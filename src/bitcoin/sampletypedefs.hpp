#ifndef SAMPLETYPEDEFS_HPP
#define SAMPLETYPEDEFS_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/TR1_Wrapper.hpp>

#include _CINTTYPES
#include _MEMORY

// this whole header file is wrong. TODO: figure out how to circular dependency and get rid of it.

class SampleEscrowServer;
typedef _SharedPtr<SampleEscrowServer> SampleEscrowServerPtr;

class SampleEscrowClient;
typedef _SharedPtr<SampleEscrowClient> SampleEscrowClientPtr;

class SampleEscrowTransaction;
typedef _SharedPtr<SampleEscrowTransaction> SampleEscrowTransactionPtr;

class EscrowPool;
typedef _SharedPtr<EscrowPool> EscrowPoolPtr;

#endif // SAMPLETYPEDEFS_HPP
