#pragma once

#include "noncopyable.h"
#include "CurrentThread.h"
#include "Timestamp.h"

#include <memory>
#include <functional>
#include <vector>
#include <atomic>
#include <mutex>

class Channel;
class Poller;

class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop(); // Start the loop 
    void quit(); //quit the loop

    Timestamp pollReturnTime() const { return pollReturnTime_; }

    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);

    void wakeup();
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);

    bool isInLoopThread() const { return threadId_ == CurrentThread::tid();}

private:
    void handleRead();
    void doPendingFunctors();

    using ChannelList = std::vector<Channel*>;

    std::atomic_bool looping_; 
    std::atomic_bool quit_;
    std::atomic_bool callingPendingFunctors_;
    const pid_t threadId_;
    Timestamp pollReturnTime_;
    std::unique_ptr<Poller> poller_;

    int wakeupFd_; //  wake up a sleeping thread to deat with a new channel
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;

    std::atomic_bool callingPendingFunctiors_;
    std::vector<Functor> pendingFunctors_;
    std::mutex mutex_; //protect the vector<Functor>
};
