#pragma once
#include "noncopyable.h"
#include <string>

class AppendFile : noncopyable
{
public:
    explicit AppendFile(const std::string &basename);
    ~AppendFile();

    void append(const char *msg, const size_t len);
    void flush();
    off_t writtenBytes() const { return writtenBytes_; }

private:
    size_t write(const char *msg, const size_t len);
    FILE *fp_;
    char buffer_[64 * 1024];
    off_t writtenBytes_;
};