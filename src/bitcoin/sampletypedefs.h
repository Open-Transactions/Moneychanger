#ifndef SAMPLETYPEDEFS_H
#define SAMPLETYPEDEFS_H

#include <tr1/memory>

// this whole header file is wrong. TODO: figure out how to circular dependency and get rid of it.

class SampleEscrowServer;
#ifndef OT_USE_TR1
    typedef std::shared_ptr<SampleEscrowServer> SampleEscrowServerPtr;
#else
    typedef std::tr1::shared_ptr<SampleEscrowServer> SampleEscrowServerPtr;
#endif // OT_USE_TR1

class SampleEscrowClient;
#ifndef OT_USE_TR1
    typedef std::shared_ptr<SampleEscrowClient> SampleEscrowClientPtr;
#else
    typedef std::tr1::shared_ptr<SampleEscrowClient> SampleEscrowClientPtr;
#endif // OT_USE_TR1

class SampleEscrowTransaction;
#ifndef OT_USE_TR1
    typedef std::shared_ptr<SampleEscrowTransaction> SampleEscrowTransactionPtr;
#else
    typedef std::tr1::shared_ptr<SampleEscrowTransaction> SampleEscrowTransactionPtr;
#endif // OT_USE_TR1

class EscrowPool;
#ifndef OT_USE_TR1
    typedef std::shared_ptr<EscrowPool> EscrowPoolPtr;
#else
    typedef std::tr1::shared_ptr<EscrowPool> EscrowPoolPtr;
#endif // OT_USE_TR1


#endif // SAMPLETYPEDEFS_H
