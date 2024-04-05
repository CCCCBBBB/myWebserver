#include "HttpServer.h"
#include "logger.h"

HttpServer::HttpServer(EventLoop *loop, const InetAddress &addr, const std::string &name)
    : server_(loop, addr, name)
{
    server_.setConnectionCallback(std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCallback(std::bind(&HttpServer::onMessage, this, std::placeholders::_1 ,std::placeholders::_2, std::placeholders::_3));
}

void HttpServer::onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        LOG_INFO("conn UP : %s", conn->peerAddress().toIpPort().c_str());
    }
    else
    {
        LOG_INFO("conn DOWN : %s", conn->peerAddress().toIpPort().c_str());
    }
}

void HttpServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime)
{

}

void HttpServer::HttpCallback()
{

}

