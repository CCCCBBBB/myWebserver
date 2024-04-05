#include "logger.h"
#include "Timestamp.h"
#include "AsyncLogging.h"

#include <iostream>
#include <sys/time.h>

const off_t kRollSize = 500*1000*1000;

static std::once_flag oc;
static AsyncLogging *AsyncLogger_;
void once_init()
{
    AsyncLogger_ = new AsyncLogging("webserver", kRollSize);
    AsyncLogger_->start(); 
}

void output(const char* msg, int len)
{
    std::call_once(oc, once_init);
    AsyncLogger_->append(msg, len);
}
// get instance
Logger &Logger::instance()
{
    static Logger logger;
    return logger;
}
// set loglevel
void Logger::setLogLevel(int level)
{
    logLevel_ = level;
}
void Logger::formatTime()
{
    struct timeval tv;
    time_t time;
    char str_t[26] = {0};
    gettimeofday (&tv, NULL);
    time = tv.tv_sec;
    struct tm* p_time = localtime(&time);   
    strftime(str_t, 26, "%Y-%m-%d %H:%M:%S\n", p_time);
    stream_ << str_t;
}
// write log
void Logger::log(const char *fileName, int line, std::string msg)
{
    formatTime();
    switch (logLevel_)
    {
    case INFO:
        stream_ << "[INFO]";
        break;
    case ERROR:
        stream_ << "[ERROR]";
        break;
    case FATAL:
        stream_ << "[FATAL]";
        break;
    case DEBUG:
        stream_ << "[DEBUG]";
        break;
    default:
        break;
    }
    stream_ << " -- " << fileName << ":" << std::to_string(line) << "\n";
    stream_ << msg << "\n";
    const LogStream::Buffer& buf(stream_.buffer());
    output(buf.data(), buf.length());
    stream_.clear();
}