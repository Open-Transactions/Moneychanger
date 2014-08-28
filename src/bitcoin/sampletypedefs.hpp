#ifndef SAMPLETYPEDEFS_HPP
#define SAMPLETYPEDEFS_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/TR1_Wrapper.hpp>

#include <list>

#include _CINTTYPES
#include _MEMORY

class SampleEscrowServer;
typedef _SharedPtr<SampleEscrowServer> SampleEscrowServerPtr;

class SampleEscrowClient;
typedef _SharedPtr<SampleEscrowClient> SampleEscrowClientPtr;

class SampleEscrowTransaction;
typedef _SharedPtr<SampleEscrowTransaction> SampleEscrowTransactionPtr;
typedef std::list<SampleEscrowTransactionPtr> EscrowTransactionList;

class EscrowPool;
typedef _SharedPtr<EscrowPool> EscrowPoolPtr;

#endif // SAMPLETYPEDEFS_HPP
