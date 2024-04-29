#include "FileUtil.h"


#include <errno.h>
#include <sys/stat.h>
#include <string.h>

AppendFile::AppendFile(const std::string &basename)
    : fp_(fopen(basename.c_str(), "ae")) // 'e' for O_CLOEXEC
    , writtenBytes_(0)
{
    ::setbuffer(fp_, buffer_, sizeof buffer_);
}

AppendFile::~AppendFile()
{
    ::fclose(fp_);
}

void AppendFile::append(const char *msg, const size_t len)
{
    size_t written = 0;

    while (written != len)
    {
        size_t remain = len - written;
        size_t n = write(msg + written, remain);
        if (n != remain)
        {
            int err = ferror(fp_);
            if (err)
            {
                fprintf(stderr, "AppendFile::append() failed \n");
                break;
            }
        }
        written += n;
    }

    writtenBytes_ += written;
}

void AppendFile::flush()
{
    fflush(fp_); 
}

size_t AppendFile::write(const char *msg, const size_t len)
{
    return fwrite_unlocked(msg, 1, len, fp_);
}
