#pragma once

#include "noncopyable.h"
#include "TcpServer.h"

#include <functional>
#include <unordered_map>
#include <memory>

class HttpServer : noncopyable
{
public:
    HttpServer(EventLoop *loop,
            const InetAddress &addr,
            const std::string &name);


    void start() { server_.start(); }
private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buf,
                 Timestamp receiveTime);

    TcpServer server_;
    char* srcDir_;

};