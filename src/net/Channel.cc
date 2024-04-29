#include "Channel.h"
#include "logger.h"
#include "EventLoop.h"

#include <sys/epoll.h>

const int Channel::kNoneEvent_ = 0;
const int Channel::kReadEvent_ = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent_ = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), fd_(fd), event_(0), revent_(0), state_(-1), tied_(false)
{
}

Channel::~Channel()
{
}

void Channel::handleEvent(Timestamp receiveTime)
{
    if (tied_)
    {
        std::shared_ptr<void> guard = tie_.lock();
        if (guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}

//tie a tcpconnection object, in case the tcpconnention is destroyed before the channel run the callback
void Channel::tie(const std::shared_ptr<void> &obj) 
{
    tie_ = obj;
    tied_ = true;
}

void Channel::update()
{
    loop_->updateChannel(this);

}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    LOG_INFO("channel handleEvent revent : %d\n", revent_);

    if ((revent_ & EPOLLHUP) && !(revent_ & EPOLLIN))
    {
        if (closeCallback_)
        {
            closeCallback_();
        }
    }

    if (revent_ & EPOLLERR)
    {
        if (errorCallback_)
        {
            errorCallback_();
        }
    }

    if (revent_ & (EPOLLIN | EPOLLPRI))
    {
        if (readCallback_)
        {
            readCallback_(receiveTime);
        }
    }

    if (revent_ & EPOLLOUT)
    {
        if (writeCallback_)
        {
            writeCallback_();
        }
    }
}

void Channel::remove()
{
    loop_->removeChannel(this);
}
