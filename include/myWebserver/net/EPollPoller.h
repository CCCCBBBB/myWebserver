#pragma once

#include "Poller.h"
#include "Timestamp.h"

#include <vector>
#include <sys/epoll.h>
class channel;
class EventLoop;

class EPollPoller : public Poller
{
public:
    EPollPoller(EventLoop *loop);
    ~EPollPoller() override;

    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;
    virtual void updateChannel(Channel *channel) override;
    virtual void removeChannel(Channel *channel) override;

private:
    static const int kInitEventListSize = 4096;

    void fillActiveChannels(int eventsNum, ChannelList *activerChannels) const;
    void update(int operation, Channel *channel);

    using EventList = std::vector<epoll_event>;

    int epollfd_;
    EventList events_;
};