#include "LogFile.h"

#include <time.h>

LogFile::LogFile(const std::string &basename,
            off_t rollSize,
            int flushInterval, 
            int flushEveryN)
    : basename_(basename)
    , rollSize_(rollSize)
    , flushInterval_(flushInterval)
    , flushEveryN_(flushEveryN)
    , count_(0)
    , startOfPeriod_(0)
    , lastRoll_(0)
    , lastFlush_(0)
{
    rollFile();
}



void LogFile::append(const char *msg, int len)
{
    file_->append(msg, len);
    if(file_->writtenBytes() > rollSize_)
    {
        rollFile();
    }
    else
    {
        ++count_;
        if (count_ >= flushEveryN_)
        {
            count_ = 0;
            time_t now = ::time(NULL);
            time_t thisPeriod = now / kRollPerSeconds_ * kRollPerSeconds_;
            if (thisPeriod != startOfPeriod_)
            {
                rollFile();
            }
            else if (now - lastFlush_ > flushInterval_)
            {
                lastFlush_ = now;
                file_->flush();
            }
        }
    }
}

void LogFile::flush()
{
    file_->flush();
}

bool LogFile::rollFile()
{
    time_t now = 0;
    std::string filename = getLogFileName(basename_, &now);
    time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

    if (now > lastRoll_)
    {
        lastRoll_ = now;
        lastFlush_ = now;
        startOfPeriod_ = start;
        file_.reset(new AppendFile(filename));
        return true;
    }
    return false;
}

std::string LogFile::getLogFileName(const std::string &basename, time_t *now)
{
    std::string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;

    char timebuf[32];
    struct tm tm;
    *now = time(NULL);
    gmtime_r(now, &tm); // FIXME: localtime_r ?
    strftime(timebuf, sizeof timebuf, "_%Y%m%d-%H%M%S", &tm);
    filename += timebuf;

    filename += ".log";

    return filename;
}
