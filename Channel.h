#pragma once

#include "noncopyable.h"
#include "Timestamp.h"

#include <functional>
#include <memory>

class Timestamp;
class EventLoop;

/// A IO channel which includes the file descriptor and the events,
/// but it doesn't really own the descriptor
class Channel : noncopyable
{
public:

    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    void handleEvent(Timestamp receiveTime);

    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    void tie(const std::shared_ptr<void> &);

    int fd() const { return fd_; }
    int event() const { return event_; }
    void set_revent(int revt) { revent_ = revt; }

    bool isNoneEvent() const { return event_ == kNoneEvent_; }
    bool isReadEvent() const { return event_ & kReadEvent_; }
    bool isWriteEvent() const { return event_ & kWriteEvent_; }

    void enableReading() { event_ |= kReadEvent_; update(); }
    void disableReading() { event_ &= ~kReadEvent_;  update(); }
    void enableWriting() { event_ |= kWriteEvent_;  update(); }
    void disableWriting() { event_ &= ~kWriteEvent_;  update(); }
    void disableAll() { event_ = kNoneEvent_; update(); }

    int state() { return state_; }
    void set_state(int sta) { state_ = sta; }

    EventLoop *ownerLoop() { return loop_; }
    void remove();

private:

    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent_;
    static const int kReadEvent_;
    static const int kWriteEvent_;

    EventLoop *loop_; // epoller loop
    const int fd_;    // file descriptor
    int event_;       // interested event
    int revent_;      // received event
    int state_;       // used by epoller
    
    std::weak_ptr<void> tie_;
    bool tied_;

    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;

};
