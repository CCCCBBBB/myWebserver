#include "HttpServer.h"
#include "HttpResponse.h"
#include "HttpRequest.h"
#include "logger.h"

#include <fstream>

HttpServer::HttpServer(EventLoop *loop, const InetAddress &addr, const std::string &name)
    : server_(loop, addr, name)
{
    server_.setConnectionCallback(std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCallback(std::bind(&HttpServer::onMessage, this, std::placeholders::_1 ,std::placeholders::_2, std::placeholders::_3));
    server_.setThreadNum(4);
    srcDir_ = getcwd(nullptr, 256);
    strncat(srcDir_, "/resources/", 16);
}

void HttpServer::onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        LOG_INFO("http connected : %s", conn->peerAddress().toIpPort().c_str());
    }
    else
    {
        LOG_INFO("http disconnected : %s", conn->peerAddress().toIpPort().c_str());
    }
}

void HttpServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime)
{
    HttpRequest request;
    HttpResponse response;
    request.init();
    if(buf->readableBytes() <= 0)
    {
        LOG_ERROR("invalid none HttpRequest");
    }
    else if(request.parse(*buf))
    {
        // std::string s = "/index.html";
        // response.Init(srcDir_, s, request.isKeepAlive(), 200);    
        response.Init(srcDir_, request.path(), request.isKeepAlive(), 200);    
        Buffer buffer;
        response.MakeResponse(buffer);

        conn->send(&buffer);
        if(!request.isKeepAlive())
        {
            conn->shutdown();
        }
    }
    else
    {
        response.Init(srcDir_, request.path(), false, 400);
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }
}

