#ifndef UTILS_HPP
#define UTILS_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <opentxs/Timer.hpp>

#include <QMutex>
#include <QWaitCondition>
#include <QCoreApplication>



#define LENGTH_OF_YEAR_IN_SECONDS 31536000
#define LENGTH_OF_SIX_MONTHS_IN_SECONDS 15552000
#define LENGTH_OF_THREE_MONTHS_IN_SECONDS 7776000
#define LENGTH_OF_MONTH_IN_SECONDS 2592000
#define LENGTH_OF_DAY_IN_SECONDS 86400
#define LENGTH_OF_HOUR_IN_SECONDS 3600
#define LENGTH_OF_MINUTE_IN_SECONDS 60


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
