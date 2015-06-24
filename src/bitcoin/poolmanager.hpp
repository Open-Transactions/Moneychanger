#ifndef POOLMANAGER_HPP
#define POOLMANAGER_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"
#include "core/TR1_Wrapper.hpp" 

#include <bitcoin/escrowpool.hpp>

#include _CINTTYPES
#include _MEMORY

#include <QObject>
#include <QList>
#include <QMap>

class PoolManager : QObject
{
public:
    PoolManager();

    void AddPool(EscrowPoolPtr pool);

    void RemovePool(EscrowPoolPtr poolToRemove);

    EscrowPoolPtr GetPoolByName(const std::string &);

    QList<EscrowPoolPtr> escrowPools;   // available pools

    std::string selectedPool;

private:
    QMap<std::string, EscrowPoolPtr> poolNameMap;
};

typedef _SharedPtr<PoolManager> PoolManagerPtr;


#endif // POOLMANAGER_HPP
