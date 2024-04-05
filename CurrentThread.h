#pragma once

#include <unistd.h>
#include <sys/syscall.h>

/// @brief system call to get the thread id
/// it will save the id when use it for the first time
namespace CurrentThread
{
    extern __thread int t_cachedTid;

    void cacheTid();
    
    inline int tid()
    {
        if(__builtin_expect(t_cachedTid == 0, 0)) //期望 == 0 不成立，执行cacheTid的可能性小 优化cpu执行效率
        {
            cacheTid();
        }
        return t_cachedTid;
    }
}