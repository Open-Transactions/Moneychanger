#ifndef __TR1_WRAPPER_HPP__
#define __TR1_WRAPPER_HPP__

//  C++ Defines.
#ifdef OT_USE_TR1
#undef OT_USE_TR1
#endif
#if !defined(_MSC_VER) && defined(OPENTXS_CXX03_TR1)
#define OT_USE_TR1
#endif

#ifndef OT_USE_TR1
#ifdef __linux__
#define _CINTTYPES <tr1/cinttypes>
#define _MEMORY <tr1/memory>
#include <boost/shared_ptr.hpp>
#endif
#ifdef __APPLE__
#define _CINTTYPES <cinttypes>
#define _MEMORY <memory>
#endif
#else
#define _CINTTYPES <tr1/cinttypes>
#define _MEMORY <tr1/memory>
#endif

#ifndef OT_USE_TR1
#ifdef __linux__
#define _SharedPtr boost::shared_ptr
#define _WeakPtr boost::weak_ptr
#define _UniquePtr boost::unique_ptr
#endif
#ifdef __APPLE__
#define _SharedPtr std::shared_ptr
#define _WeakPtr std::weak_ptr
#define _UniquePtr std::unique_ptr
#endif
#else
#define _SharedPtr std::tr1::shared_ptr
#define _WeakPtr std::tr1::weak_ptr
#define _UniquePtr std::tr1::unique_ptr
#endif

#endif //__TR1_WRAPPER_HPP__

