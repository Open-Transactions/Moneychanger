#pragma once
//
//  MsgQueue.h
//

#include <queue>

#ifndef OT_USE_TR1
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <function>
#define OT_THREAD std::thread
#define OT_MUTEX(MT) std::mutex MT
#define INSTANTIATE_MLOCK(MT) std::unique_lock<std::mutex>mlock(MT)
#define CONDITION_VARIABLE(VAR) std::condition_variable VAR
#define OT_ATOMIC(THE_ATOM) std::atomic<bool> THE_ATOM
#define OT_ATOMIC_TRUE true
#define OT_ATOMIC_FALSE false
#define OT_ATOMIC_ISTRUE(THE_VAL) (true == THE_VAL)
#define OT_ATOMIC_ISFALSE(THE_VAL) (false == THE_VAL)
#define OT_STD_FUNCTION(FUNC_TYPE) std::function< FUNC_TYPE >
#define OT_STD_BIND std::bind
#else
//#include <QMutex>
//#include <QMutexLocker>
//#include <QWaitCondition>
//#include <QThread>
//#include <QAtomicInt>
//#define OT_MUTEX(MT) QMutex MT
//#define INSTANTIATE_MLOCK(MT) QMutexLocker mlock(&MT)
//#define CONDITION_VARIABLE(VAR) QWaitCondition VAR
//#define notify_one wakeOne
//#define OT_ATOMIC(THE_ATOM) QAtomicInt THE_ATOM
//#define OT_ATOMIC_ISTRUE(THE_VAL) (1 == THE_VAL.load())
//#define OT_ATOMIC_ISFALSE(THE_VAL) (0 == THE_VAL.load())
#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>
#include <tr1/functional>
#define OT_THREAD boost::thread
#define OT_MUTEX(MT) boost::mutex MT
#define INSTANTIATE_MLOCK(MT) boost::unique_lock<boost::mutex>mlock(MT)
#define CONDITION_VARIABLE(VAR) boost::condition_variable VAR
#define OT_ATOMIC(THE_ATOM) boost::atomic<bool> THE_ATOM
#define OT_ATOMIC_TRUE 1
#define OT_ATOMIC_FALSE 0
#define OT_ATOMIC_ISTRUE(THE_VAL) (true == THE_VAL)
#define OT_ATOMIC_ISFALSE(THE_VAL) (false == THE_VAL)
#define OT_STD_FUNCTION(FUNC_TYPE) std::tr1::function< FUNC_TYPE >
#define OT_STD_BIND std::tr1::bind
#ifndef nullptr
#define nullptr NULL
#endif
#endif



template <typename T>
class MsgQueue
{
public:
    
    T pop()
    {
        INSTANTIATE_MLOCK(mutex_);
        while (queue_.empty())
        {
            cond_.wait(mlock);
        }
        auto item = queue_.front();
        queue_.pop();
        return item;
    }

    void pop(T& item)
    {
        INSTANTIATE_MLOCK(mutex_);
        while (queue_.empty())
        {
            cond_.wait(mlock);
        }
        item = queue_.front();
        queue_.pop();
    }
    
    void push(const T& item)
    {
        INSTANTIATE_MLOCK(mutex_);
        queue_.push(item);
        mlock.unlock();
        cond_.notify_one();
    }
    
#ifndef OT_USE_TR1
    void push(T&& item)
    {
        INSTANTIATE_MLOCK(mutex_);
        queue_.push(std::move(item));
        mlock.unlock();
        cond_.notify_one();
    }
#endif
    
    int size()
    {
        INSTANTIATE_MLOCK(mutex_);
        int size = queue_.size();
        mlock.unlock();
        return size;
    }
    
    void clear()
    {
        INSTANTIATE_MLOCK(mutex_);
        while(!queue_.empty())
            queue_.pop();
        mlock.unlock();
    }
    
private:
    std::queue<T> queue_;
    OT_MUTEX(mutex_);
    CONDITION_VARIABLE(cond_);
};