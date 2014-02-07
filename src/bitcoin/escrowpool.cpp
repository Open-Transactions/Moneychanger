#include "escrowpool.h"

EscrowPool::EscrowPool()
{
    this->escrowServers = QList<SampleEscrowServerPtr>();

    this->poolName = "unnamed";
}

void EscrowPool::AddEscrowServer(SampleEscrowServerPtr server)
{
    if(server == NULL || this->escrowServers.contains(server))
        return;

    this->escrowServers.append(server);
}

void EscrowPool::RemoveEscrowServer(SampleEscrowServerPtr server)
{
    this->escrowServers.removeAll(server);
}
