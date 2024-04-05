#include "TcpConnection.h"
#include "logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

#include <memory>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/tcp.h>

static EventLoop *CheckLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d TcpConnectionLoop is null! \n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}



TcpConnection::TcpConnection(EventLoop *loop, const std::string &name, int sockfd, const InetAddress &localAddr, const InetAddress &peerAddr)
    : loop_(CheckLoopNotNull(loop))
    , name_(name)
    , state_(kConnecting)
    , reading_(true)
    , socket_(new Socket(sockfd))
    , channel_(new Channel(loop, sockfd))
    , localAddr_(localAddr)
    , peerAddr_(peerAddr)
    , highWaterMark_(64*1024*1024)
{
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

    LOG_INFO("TcpConnection::ctor[%s] at fd = %d \n", name_.c_str(), sockfd);
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    LOG_INFO("TcpConnection::dtor[%s] at fd = %d state = %d\n", name_.c_str(), socket_->fd(), (int)state_);
}

void TcpConnection::send(const std::string &buf)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(buf.c_str(), buf.size());
        }
        else
        {
            // loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));// why not just using the runInloop? 
            void (TcpConnection::*fp)(const std::string& message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(
                std::bind(fp, this, buf));
        }
    }
}

void TcpConnection::send(Buffer *buf)
{
  if (state_ == kConnected)
  {
    if (loop_->isInLoopThread())
    {
      sendInLoop(buf->peek(), buf->readableBytes());
      buf->retrieveAll();
    }
    else
    {
      void (TcpConnection::*fp)(const std::string& message) = &TcpConnection::sendInLoop;
      loop_->runInLoop(
          std::bind(fp,
                    this,     // FIXME
                    buf->retrieveAllAsString()));
                    //std::forward<string>(message)));
    }
  }
}
void TcpConnection::sendInLoop(const std::string& message)
{
    sendInLoop(message.data(), message.size());
}
void TcpConnection::sendInLoop(const void *message, size_t len)
{
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;

    if(state_ == kDisconnected)
    {
        LOG_ERROR("disconnected, give up writing");
        return;
    }
    
    if (!channel_->isWriteEvent() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = ::write(channel_->fd(), message, len);
        if (nwrote >= 0)
        {
            remaining = len - nwrote;
            if(remaining == 0 && writeCompleteCallback_)
            {
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
    }
    else
    {
        nwrote = 0;
        if(errno != EWOULDBLOCK)
        {
            LOG_ERROR("TcpConnection::sendInLoop");
            if(errno == EPIPE || errno == ECONNRESET)
            {
                faultError = true;
            }
        }
    }

    if( !faultError && remaining > 0) //Not all of the data was sent
    {
        size_t oldLen = outputBuffer_.readableBytes();
        if(oldLen + remaining >= highWaterMark_ 
            && oldLen < highWaterMark_
            && highWaterMarkCallback_)
        {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }
        outputBuffer_.append((char*)message + nwrote, remaining);
        if(!channel_->isWriteEvent())
        {
            channel_->enableWriting();// register the writing event
        }
    }

}

void TcpConnection::shutdown()
{
    if(state_ == kConnected)
    {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop()
{
    if(!channel_->isWriteEvent())
    {
        socket_->shutdownWrite(); //head to handleClose callback
    }
}


void TcpConnection::connectEstablished()
{
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    if(state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(socket_->fd(), &savedErrno);
    if(n > 0)
    {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if (n == 0)
    {
        handleClose();
    }
    else
    {
        errno = savedErrno;
        LOG_ERROR("TcpConnection::handleRead");
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    if(channel_->isWriteEvent())
    {
        int savedErrno = 0;
        ssize_t n = outputBuffer_.writeFd(socket_->fd(), &savedErrno);
        if(n > 0)
        {
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableBytes() == 0)
            {
                channel_->disableWriting();
                if(writeCompleteCallback_)
                {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));// no need to use queueInLoop, now is in the localThread
                    // writeCompleteCallback_(shared_from_this());
                }
            }
            if (state_ == kDisconnecting)
            {
                shutdownInLoop();
            }

        }
        else
        {
            LOG_ERROR("TcpConnection::handleWrite");
        }
    }
    else
    {
        LOG_ERROR("TcpConnection fd=%d is down, no more writing \n", channel_->fd());
    }
}

void TcpConnection::handleClose()
{
    LOG_INFO("TcpConnection::handleClose fd=%d state=%d \n", channel_->fd(), (int)state_);

    setState(kDisconnecting);
    channel_->disableAll();

    TcpConnectionPtr connPtr(shared_from_this());
    if(connectionCallback_) connectionCallback_(connPtr);
    if(closeCallback_) closeCallback_(connPtr);
}

void TcpConnection::handleError()
{
    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if(::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
    LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d \n", name_.c_str(), err);
}
