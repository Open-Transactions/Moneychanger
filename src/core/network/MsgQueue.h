#pragma once
//
//  MsgQueue.h

#include <queue>

#include "threading.h"


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