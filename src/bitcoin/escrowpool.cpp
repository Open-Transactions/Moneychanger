#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin/escrowpool.hpp>

#include <bitcoin/sampleescrowserver.hpp>

#include <QThread>


EscrowPool::EscrowPool(uint32_t sigsRequired)
{
    this->escrowServers = QList<SampleEscrowServerPtr>();

    this->poolName = "unnamed";

    this->sigsRequired = sigsRequired;

    this->containsHostedServer = false;

    this->serverNameMap = std::map<std::string, SampleEscrowServerPtr>();
}

void EscrowPool::AddEscrowServer(SampleEscrowServerPtr server)
{
    if(server == NULL || this->escrowServers.contains(server))
        return;

    this->escrowServers.append(server);
    this->serverNameMap[server->serverName] = server;

    if(!server->isClient)
        this->containsHostedServer = true;

    QThread* serverThread = new QThread();
    server->moveToThread(serverThread);
    serverThread->start();
    this->serverThreadMap[server->serverName] = serverThread;

    if(!server->isClient)
    {
        QMetaObject::invokeMethod(server.get(), "StartServerLoop", Qt::QueuedConnection);
    }
}

void EscrowPool::RemoveEscrowServer(SampleEscrowServerPtr server)
{
    QThread* serverThread = this->serverThreadMap[server->serverName];
    this->serverThreadMap[server->serverName] = NULL;
    delete serverThread;
    serverThread = NULL;
    this->escrowServers.removeAll(server);
}
