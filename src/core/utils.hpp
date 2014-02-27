#ifndef UTILS_HPP
#define UTILS_HPP

#include <WinsockWrapper.h>
#include <ExportWrapper.h>

#ifdef Q_OS_WIN
#include <windows.h>    // for Sleep
#else
#include <time.h>       // for nanosleep
#endif

#include <QMutex>
#include <QWaitCondition>
#include <QCoreApplication>

// code from http://stackoverflow.com/questions/3752742/how-do-i-create-a-pause-wait-function-using-qt

// utilities
namespace utils
{
    // copy of QTest::qSleep() so we don't have to include all of QtTest module.
    // time: time to wait in ms
    void qSleep(int time);

    // this class might be better as it supposedly doesn't block GUI events.
    class SleepSimulator
    {
         QMutex localMutex;
         QWaitCondition sleepSimulator;
    public:
        SleepSimulator()
        {
            localMutex.lock();
        }
        void sleep(unsigned long sleepMS)
        {
            while(sleepMS > 10)
            {
                sleepSimulator.wait(&localMutex, 10);
                QCoreApplication::processEvents();
                sleepMS -= 10;
            }
            if(sleepMS > 0)
            {
                sleepSimulator.wait(&localMutex, sleepMS);
                QCoreApplication::processEvents();
            }

        }
        void CancelSleep()
        {
            sleepSimulator.wakeAll();
        }
    };
}

#endif // UTILS_HPP
