#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/utils.hpp>


namespace utils
{
    void qSleep(int ms)
    {
        if(ms < 0) return;

    #ifdef Q_OS_WIN
        Sleep(uint(ms));
    #else
        struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
        nanosleep(&ts, NULL);
    #endif
    }
}
