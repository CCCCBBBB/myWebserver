#pragma once

#include "noncopyable.h"
#include "TcpServer.h"

#include "TcpServer.h"

#include <functional>
#include <unordered_map>
#include <memory>

class HttpRequest;
class HttpResponse;

class HttpServer : noncopyable
{
public:
    using HttpCallback = std::function<void (const HttpRequest&,
                              HttpResponse*)>;
    HttpServer(EventLoop *loop,
            const InetAddress &addr,
            const std::string &name);


    void start() { server_.start(); }
    void setHttpCallback(const HttpCallback& cb)
    {
        httpCallback_ = cb;
    }
private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buf,
                 Timestamp receiveTime);
    void onRequest(const TcpConnectionPtr&, const HttpRequest&);
    TcpServer server_;
    HttpCallback httpCallback_;
};