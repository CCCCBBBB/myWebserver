#include "AsyncLogging.h"
#include "LogFile.h"
#include "Timestamp.h"

AsyncLogging::AsyncLogging(const std::string &basename, off_t rollsize, int flushInterval)
    : flushInterval_(flushInterval)
    , running_(false)
    , ready_(false)
    , basename_(basename)
    , rollSize_(rollsize)
    , thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging")
    , currentBuffer_(new Buffer)
    , nextBuffer_(new Buffer)
    , buffers_()
    , readyCond_()
    , appendCond_()
{ 
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
}

void AsyncLogging::append(const char *msg, int len)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if(currentBuffer_->avail() > len)
    {
        currentBuffer_->append(msg, len);
    }
    else
    {
        buffers_.push_back(std::move(currentBuffer_));

        if(nextBuffer_)
        {
            currentBuffer_ = std::move(nextBuffer_);
        }
        else
        {
            currentBuffer_.reset(new Buffer);
        }
        currentBuffer_->append(msg, len);
        appendCond_.notify_one();//因为没有log消息要记录时，后端线程很可能阻塞等待log消息，当有缓冲满时，及时唤醒后端将已满缓冲数据写到磁盘上，提升性能
    }
}

void AsyncLogging::start()
{

    running_ = true;
    thread_.start();
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (!ready_)
        {
            readyCond_.wait(lock);
        }
    }
}

void AsyncLogging::stop()
{
    running_ = false;
    thread_.join();
}

void AsyncLogging::threadFunc()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        ready_ = true;
        readyCond_.notify_one();
    }
    LogFile output(basename_, rollSize_);
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();
    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);
    while (running_)
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (buffers_.empty())
            {
                appendCond_.wait_for(lock, std::chrono::seconds(flushInterval_));
            }
            buffers_.push_back(std::move(currentBuffer_));
            currentBuffer_ = std::move(newBuffer1);
            buffersToWrite.swap(buffers_);
            if (!nextBuffer_)
            {
                nextBuffer_ = std::move(newBuffer2);
            }
        }

        if (buffersToWrite.size() > 25)
        {
            char buf[256];
            snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
               Timestamp::now().toString(),
               buffersToWrite.size()-2);
            fputs(buf, stderr);      
            output.append(buf, static_cast<int>(strlen(buf)));
            buffersToWrite.erase(buffersToWrite.begin()+2, buffersToWrite.end());
        }

        for (const auto& buffer : buffersToWrite)
        {
            output.append(buffer->data(), buffer->length());
        }

        if (buffersToWrite.size() > 2)
        {
            buffersToWrite.resize(2);
        }
        if (!newBuffer1)
        {
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        if (!newBuffer2)
        {
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        buffersToWrite.clear();
        output.flush();        
    }
    output.flush();
}
