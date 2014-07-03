#ifndef ESCROWPOOL_HPP
#define ESCROWPOOL_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/TR1_Wrapper.hpp>

#include _CINTTYPES
#include _MEMORY

#include <QList>
#include <QString>
#include <map>

class SampleEscrowServer;
class QThread;

typedef _SharedPtr<SampleEscrowServer> SampleEscrowServerPtr;

class EscrowPool
{
public:
    EscrowPool(uint32_t sigsRequired);

    // add server to pool
    void AddEscrowServer(SampleEscrowServerPtr server);

    void RemoveEscrowServer(SampleEscrowServerPtr server);

    QList<SampleEscrowServerPtr> escrowServers;     // servers that are part of this pool

    std::string poolName;

    std::map<std::string, SampleEscrowServerPtr> serverNameMap;
    std::map<std::string, QThread*> serverThreadMap;

    uint32_t sigsRequired;

    bool containsHostedServer;

private:

};

typedef _SharedPtr<EscrowPool> EscrowPoolPtr;

#endif // ESCROWPOOL_HPP
