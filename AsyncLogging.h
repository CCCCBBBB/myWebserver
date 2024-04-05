#pragma once
#include "noncopyable.h"
#include "Thread.h"
#include "LogStream.h"


#include <string>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <memory>

class AsyncLogging : noncopyable
{
public:
    AsyncLogging(const std::string &basename, off_t rollsize, int flushInterval = 3);
    ~AsyncLogging()
    {
        if (running_)
        {
            stop();
        }
    }
    void append(const char *msg, int len);

    void start();

    void stop();


private:
    void threadFunc();


    using Buffer = FixedBuffer<kLargeBuffer>;
    using BufferVector = std::vector<std::unique_ptr<Buffer>>;
    using BufferPtr = BufferVector::value_type;
    
    Thread thread_;
    std::atomic<bool> running_;
    std::atomic<bool> ready_;
    std::condition_variable readyCond_;
    std::condition_variable appendCond_;
    std::mutex mutex_;

    const int flushInterval_;
    const std::string basename_;
    const off_t rollSize_;
    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;
};