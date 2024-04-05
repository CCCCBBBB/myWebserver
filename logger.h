#pragma once

#include <string>

#include "noncopyable.h"
#include "LogStream.h"

// LOG_INFO("%s %d", arg1, arg2)
#define LOG_INFO(logmsgFormat, ...)                       \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::instance();              \
        logger.setLogLevel(INFO);                         \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(__FILE__, __LINE__, buf);                                  \
    } while (0)

#define LOG_ERROR(logmsgFormat, ...)                      \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::instance();              \
        logger.setLogLevel(ERROR);                        \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(__FILE__, __LINE__, buf);                                  \
    } while (0)

#define LOG_FATAL(logmsgFormat, ...)                      \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::instance();              \
        logger.setLogLevel(FATAL);                        \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(__FILE__, __LINE__, buf);                                  \
        exit(-1);                                         \
    } while (0)

#ifdef ENABLEDEBUG
#define LOG_DEBUG(logmsgFormat, ...)                     \
    do                                                   \
    {                                                    \
        Logger &logger = Logger::instance();             \
        logger.setLogLevel(DEBUG);                       \
        char buf[1024] = {0};                            \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__) \
            logger.log(__FILE__, __LINE__, buf);                             \
    } while (0);
#else
#define LOG_DEBUG(logmsgFormat, ...)
#endif

// log level INFO DEBUG ERROR FATAL
enum LogLevel
{
    INFO,
    ERROR,
    FATAL, // core
    DEBUG,
};

class Logger : noncopyable
{
public:
    // get instance
    static Logger &instance();
    // set loglevel
    void setLogLevel(int level);
    // format time
    void formatTime();
    //
    void log(const char *fileName, int line, std::string msg);

private:
    int logLevel_;
    LogStream stream_;
};