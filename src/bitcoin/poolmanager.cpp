#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin/poolmanager.hpp>


PoolManager::PoolManager()
{
    this->escrowPools = QList<EscrowPoolPtr>();
}

void PoolManager::AddPool(EscrowPoolPtr pool)
{
    if(pool == NULL || this->escrowPools.contains(pool))
        return;

    this->escrowPools.append(pool);
}

void PoolManager::RemovePool(EscrowPoolPtr poolToRemove)
{
    // better safe than sorry
    this->escrowPools.removeAll(poolToRemove);
}
