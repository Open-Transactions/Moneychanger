#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin/poolmanager.hpp>
#include <bitcoin/sampleescrowserver.hpp>


PoolManager::PoolManager()
{
    this->escrowPools = QList<EscrowPoolPtr>();
    this->selectedPool = std::string();
    this->poolNameMap = QMap<std::string, EscrowPoolPtr>();
}

void PoolManager::AddPool(EscrowPoolPtr pool)
{
    if(pool == NULL || this->escrowPools.contains(pool))
        return;

    EscrowPoolPtr oldPool = this->GetPoolByName(pool->poolName);
    if(oldPool != NULL)
        this->RemovePool(oldPool);

    this->escrowPools.append(pool);
    this->poolNameMap[pool->poolName] = pool;

    // TODO: emit signal so the GUI can update or use model based lists that update automatically.
}

void PoolManager::RemovePool(EscrowPoolPtr poolToRemove)
{
    foreach(SampleEscrowServerPtr server, poolToRemove->escrowServers)
    {
        server->shutDown = true;
        server->serverPool = EscrowPoolPtr();
        server = SampleEscrowServerPtr();
    }

    this->escrowPools.removeOne(poolToRemove);
    this->poolNameMap.remove(poolToRemove->poolName);
    if(this->selectedPool == poolToRemove->poolName)
        this->selectedPool = std::string();

    // TODO: emit signal
}

EscrowPoolPtr PoolManager::GetPoolByName(const std::string& name)
{
    QMap<std::string, EscrowPoolPtr>::const_iterator poolIter = this->poolNameMap.find(name);
    if(poolIter != this->poolNameMap.end())
        return poolIter.value();

    return EscrowPoolPtr();
}
