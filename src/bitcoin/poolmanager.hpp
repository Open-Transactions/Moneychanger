#ifndef POOLMANAGER_HPP
#define POOLMANAGER_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/MemoryWrapper.hpp>

#include <bitcoin/escrowpool.hpp>

#include <QObject>


class PoolManager : QObject
{
public:
    PoolManager();

    void AddPool(EscrowPoolPtr pool);

    void RemovePool(EscrowPoolPtr poolToRemove);

    QList<EscrowPoolPtr> escrowPools;   // available pools

private:
};

#ifndef OT_USE_TR1
    typedef std::shared_ptr<PoolManager> PoolManagerPtr;
#else
    typedef std::tr1::shared_ptr<PoolManager> PoolManagerPtr;
#endif // OT_USE_TR1

#endif // POOLMANAGER_HPP
