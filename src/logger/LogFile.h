#pragma once
#include "noncopyable.h"
#include "FileUtil.h"

#include <string>
#include <mutex>
#include <memory>

class LogFile : noncopyable
{
public:
    LogFile(const std::string &basename,
            off_t rollSize,
            int flushInterval = 3, 
            int flushEveryN = 1024);
    ~LogFile() = default;

    void append(const char *msg, int len);
    void flush();
    bool rollFile();

private:
    static std::string getLogFileName(const std::string &basename, time_t *now);
    const std::string basename_;
    const off_t rollSize_;
    const int flushInterval_;
    const int flushEveryN_;
    std::mutex mutex_;
    std::unique_ptr<AppendFile> file_;

    int count_;
    const static int kRollPerSeconds_ = 60 * 60 * 24;
    time_t startOfPeriod_;
    time_t lastRoll_;
    time_t lastFlush_;
};
