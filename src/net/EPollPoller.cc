#include "EPollPoller.h"
#include "logger.h"
#include "Channel.h"

#include <errno.h>
#include <unistd.h>
#include <string.h>

const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

EPollPoller::EPollPoller(EventLoop *loop)
    : Poller(loop)
    , epollfd_(::epoll_create1(EPOLL_CLOEXEC))
    , events_(kInitEventListSize)
{
    if (epollfd_ < 0)
    {
        LOG_FATAL("epoll_create error : %d \n", errno);
    }
}

EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
    // supposed to use DEBUG
    LOG_INFO("func=%s fd total count:%lu \n", __FUNCTION__, channels_.size());

    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;
    Timestamp now(Timestamp::now());

    if (numEvents > 0)
    {
        LOG_INFO("%d events happened \n", numEvents);

        fillActiveChannels(numEvents, activeChannels);

        if (numEvents == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numEvents == 0)
    {
        LOG_DEBUG("%s timeout \n", __FUNCTION__);
    }
    else
    {
        if(saveErrno != EINTR)
        {
            errno = saveErrno;
            LOG_ERROR("EPollPoller::poll error \n");
        }
    }
    return now;
}

void EPollPoller::updateChannel(Channel *channel)
{
    const int state = channel->state();
    LOG_INFO("func = %s fd = %d events = %d state = %d \n", __FUNCTION__, channel->fd(), channel->event(), state);
    if (state == kNew || state == kDeleted)
    {
        if (state == kNew)
        {
            int fd = channel->fd();
            channels_[fd] = channel;
        }
        channel->set_state(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        int fd = channel->fd();
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_state(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    channels_.erase(fd);
    LOG_INFO("func=%s => fd=%d\n", __FUNCTION__, fd);

    int state = channel->state();

    if (state == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }

    channel->set_state(kNew);
}

void EPollPoller::fillActiveChannels(int eventsNum, ChannelList *activerChannels) const
{
    for(int i = 0; i < eventsNum; ++i)
    {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revent(events_[i].events);
        activerChannels->push_back(channel);
    }
}

void EPollPoller::update(int operation, Channel *channel)
{
    epoll_event event;
    memset(&event, 0, sizeof event);
    int fd = channel->fd();
    event.events = channel->event();
    event.data.fd = fd;
    event.data.ptr = channel;

    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del error:%d \n", errno);
        }
        else
        {
            LOG_FATAL("epoll_ctl add or mod error:%d \n", errno);
        }
    }
}
