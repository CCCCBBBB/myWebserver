#pragma once
#include "noncopyable.h"

#include <string>
#include "string.h"

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template <int SIZE>
class FixedBuffer : noncopyable
{
public:
    FixedBuffer() : cur_(data_) {}
    ~FixedBuffer() {}

    void append(const char *buf, size_t len)
    {
        if (static_cast<size_t>(avail()) > len)
        {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }

    int avail() const { return static_cast<int>(end() - cur_); }
    const char *data() const { return data_; }
    int length() const { return static_cast<int>(cur_ - data_); } 
    void bzero() { ::bzero(data_, sizeof data_); }
    void reset() { bzero(); cur_ = data_; }
private:
    const char *end() const { return data_ + sizeof data_; }
    char data_[SIZE];
    char *cur_;
};

class LogStream : noncopyable
{
    using self = LogStream;

public:
    using Buffer = FixedBuffer<kSmallBuffer>;

    self &operator<<(const std::string &s)
    {
        buffer_.append(s.c_str(), s.size());
        return *this;
    }
    self &operator<<(const char *str)
    {
        if (str)
        {
            buffer_.append(str, strlen(str));
        }
        else
        {
            buffer_.append("(null)", 6);
        }
        return *this;
    }
    const Buffer &buffer() const { return buffer_; }
    void clear() { buffer_.reset(); }
private:
    Buffer buffer_;
};