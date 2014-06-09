#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin/escrowpool.hpp>

#include <bitcoin/sampleescrowserver.hpp>


EscrowPool::EscrowPool(int32_t sigsRequired)
{
    this->escrowServers = QList<SampleEscrowServerPtr>();

    this->poolName = "unnamed";

    this->sigsRequired = sigsRequired;

    this->serverNameMap = std::map<std::string, SampleEscrowServerPtr>();
}

void EscrowPool::AddEscrowServer(SampleEscrowServerPtr server)
{
    if(server == NULL || this->escrowServers.contains(server))
        return;

    this->escrowServers.append(server);
    this->serverNameMap[server->serverName] = server;
}

void EscrowPool::RemoveEscrowServer(SampleEscrowServerPtr server)
{
    this->escrowServers.removeAll(server);
}
