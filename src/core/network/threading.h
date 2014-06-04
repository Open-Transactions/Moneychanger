#pragma once
//
//  threading.h
//
//
//  Created by Jonathan Rumion on 5/29/14.
//
//

#ifndef OT_USE_TR1
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
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
