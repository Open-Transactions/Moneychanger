#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin/sampleescrowclient.hpp>
#include <bitcoin/sampleescrowserver.hpp>

#include <core/modules.hpp>

#include <QTimer>

#include <cstdio>  // printf
#include <cstdlib> // find

// random name generator
void gen_random(char *s, const int len)
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = 0;
}

struct SampleEscrowClient::Action
{
    enum ActionType
    {
        ContactServer,
        StartDeposit,
        RequestMultisigAddr,
        RequestRelease,
        CheckBalance,
        GetTxCount,
        FetchTransaction
    };

    ActionType type;
    EscrowPoolPtr pool;
    int64_t amount;
    std::string address;
};

SampleEscrowClient::SampleEscrowClient(QObject* parent)
    :QObject(parent)
{   
    Initialize();
}

SampleEscrowClient::SampleEscrowClient(BitcoinServerPtr rpcServer, QObject *parent)
    :QObject(parent)
{
    Initialize();

    this->rpcServer = rpcServer;
}

SampleEscrowClient::SampleEscrowClient(BtcModulesPtr modules, QObject *parent)
    :QObject(parent)
{
    Initialize();

    this->modules = modules;
}

SampleEscrowClient::~SampleEscrowClient()
{
    this->rpcServer.reset();
}

void SampleEscrowClient::Initialize()
{
    this->modules = BtcModulesPtr(new BtcModules());

    // generate random client name
    this->clientName = "                   ";
    gen_random((char*)this->clientName.c_str(), this->clientName.size());

    this->updateTimer = _SharedPtr<QTimer>(new QTimer());
    this->updateTimer->setInterval(100);
    this->updateTimer->start();
    connect(this->updateTimer.get(), SIGNAL(timeout()), this, SLOT(Update()));

    Reset();
}

void SampleEscrowClient::Reset()
{
    this->minConfirms = BtcHelper::WaitForConfirms;      // wait for one confirmation by default

    this->poolAddressMap = PoolAddressMap();
    this->poolBalanceMap = PoolBalanceMap();
    this->poolTxMap = PoolTxMap();
    this->poolTxCountMap = PoolTxCountMap();

    this->actionsToDo = Actions();
}

void SampleEscrowClient::Update()
{
    if(this->actionsToDo.size() <= 0)
        return;

    this->updateTimer->stop();

    ActionPtr action = this->actionsToDo.front();
    switch((int)action->type)
    {
    case Action::ContactServer:
    {
        ContactServer(action);
        break;
    }
    case Action::StartDeposit:
    {
        RequestDeposit(action);
        break;
    }
    case Action::RequestMultisigAddr:
    {
        AskForDepositAddress(action);
        break;
    }
    case Action::RequestRelease:
    {
        RequestRelease(action);
        break;
    }
    case Action::CheckBalance:
    {
        CheckBalance(action);
        break;
    }
    case Action::GetTxCount:
    {
        GetPoolTxCount(action);
        break;
    }
    case Action::FetchTransaction:
    {
        FetchPoolTx(action);
        break;
    }
    }

    this->actionsToDo.remove(action);

    this->updateTimer->start();
}

void SampleEscrowClient::StartDeposit(int64_t amountToSend, EscrowPoolPtr targetPool)
{
    if(this->rpcServer != NULL)
        this->modules->btcRpc->ConnectToBitcoin(this->rpcServer);

    if(targetPool == NULL || targetPool->containsHostedServer || amountToSend <= 0)
        return;

    InitializePool(targetPool);

    ActionPtr contact = ActionPtr(new Action());
    contact->type = Action::ContactServer;
    contact->pool = targetPool;

    ActionPtr start = ActionPtr(new Action());
    start->type = Action::StartDeposit;
    start->amount = amountToSend;
    start->pool = targetPool;

    this->actionsToDo.push_back(contact);
    this->actionsToDo.push_back(start);   
}

void SampleEscrowClient::StartWithdrawal(const int64_t &amountToWithdraw, const std::string &toAddress, EscrowPoolPtr fromPool)
{
    if(fromPool == NULL || fromPool->containsHostedServer || amountToWithdraw <= 0 || toAddress.empty())
        return;

    if(this->rpcServer != NULL)
        this->modules->btcRpc->ConnectToBitcoin(this->rpcServer);

    InitializePool(fromPool);

    ActionPtr request = ActionPtr(new Action());
    request->type = Action::RequestRelease;
    request->amount = amountToWithdraw;
    request->address = toAddress;
    request->pool = fromPool;
    this->actionsToDo.push_back(request);
}

void SampleEscrowClient::CheckPoolBalance(EscrowPoolPtr pool)
{
    std::cout.flush();
    if(pool == NULL || pool->containsHostedServer)
        return;

    InitializePool(pool);

    ActionPtr check = ActionPtr(new Action());
    check->type = Action::CheckBalance;
    check->pool = pool;
    this->actionsToDo.push_back(check);
}

void SampleEscrowClient::CheckPoolTransactions(EscrowPoolPtr pool)
{
    if(pool == NULL || pool->containsHostedServer)
        return;

    InitializePool(pool);

    foreach (ActionPtr action, this->actionsToDo)
    {
        if(action->type == Action::FetchTransaction && action->pool->poolName == pool->poolName)
            return;
    }

    ActionPtr getTxCount = ActionPtr(new Action());
    getTxCount->type = Action::GetTxCount;
    getTxCount->pool = pool;
    this->actionsToDo.push_back(getTxCount);

    ActionPtr getTransactions = ActionPtr(new Action());
    getTransactions->type = Action::FetchTransaction;
    getTransactions->pool = pool;
    this->actionsToDo.push_back(getTransactions);
}

void SampleEscrowClient::ContactServer(ActionPtr action)
{
    // connect to each server
    foreach(SampleEscrowServerPtr server, action->pool->escrowServers)
    {
        if(server->isClient)
            server->ClientConnected(this);
    }
}

void SampleEscrowClient::RequestDeposit(ActionPtr action)
{
    // tell each server we'd like to deposit something
    foreach(SampleEscrowServerPtr server, action->pool->escrowServers)
    {
        if(!server->isClient)
            continue;

        bool accepted = server->RequestEscrowDeposit(this->clientName, action->amount);
        //if(!accepted)
        //    return;   currently doesn't work yet
    }

    ActionPtr requestAddr = ActionPtr(new Action());
    requestAddr->type = Action::RequestMultisigAddr;
    requestAddr->pool = action->pool;
    this->actionsToDo.push_back(requestAddr);
}

void SampleEscrowClient::AskForDepositAddress(ActionPtr action)
{
    this->poolAddressMap[action->pool->poolName] = std::string();

    // ask the servers for an address to send money to
    std::string depositAddress = std::string();
    foreach(SampleEscrowServerPtr server, action->pool->escrowServers)
    {
        if(!server->isClient)
            continue;

        std::string serverMultisig = server->RequestDepositAddress(this->clientName);

        if(depositAddress.empty())
            depositAddress = serverMultisig;
        else if(depositAddress != serverMultisig)
        {
            std::printf("received deposit address %s, expected %s\n", serverMultisig.c_str(), depositAddress.c_str());
            std::cout.flush();
            depositAddress = std::string();
            break;
        }
    }

    if(depositAddress.empty())
    {
        ActionPtr askForAddr = ActionPtr(new Action());
        askForAddr->type = Action::RequestMultisigAddr;
        askForAddr->pool = action->pool;
        this->actionsToDo.push_back(askForAddr);
    }

    this->poolAddressMap[action->pool->poolName] = depositAddress;
}

void SampleEscrowClient::SendToEscrow(ActionPtr action)
{

}

void SampleEscrowClient::RequestRelease(ActionPtr action)
{
    foreach(SampleEscrowServerPtr server, action->pool->escrowServers)
    {
        if(!server->isClient)
            continue;

        bool accepted = server->RequestEscrowWithdrawal(this->clientName, action->amount, action->address);
        //if(!accepted)
        //    return;   doesn't work yet
    }
}

void SampleEscrowClient::CheckBalance(ActionPtr action)
{
    if(action->pool->escrowServers.isEmpty())
        return;

    if(!action->pool->escrowServers.first()->isClient)
        return;

    SampleEscrowServerPtr server = action->pool->escrowServers.first();
    this->poolBalanceMap[action->pool->poolName] = server->GetClientBalance(this->clientName);
}

void SampleEscrowClient::GetPoolTxCount(ActionPtr action)
{
    if(action->pool->escrowServers.isEmpty())
        return;

    if(!action->pool->escrowServers.first()->isClient)
        return;

    SampleEscrowServerPtr server = action->pool->escrowServers.first();
    this->poolTxCountMap[action->pool->poolName] = server->GetClientTransactionCount(this->clientName);

    std::printf("Pool Tx count: %lu\n", this->poolTxCountMap[action->pool->poolName]);
    std::cout.flush();
}

void SampleEscrowClient::FetchPoolTx(ActionPtr action)
{
    if(this->poolTxCountMap[action->pool->poolName] == this->poolTxMap[action->pool->poolName].size())
        return;

    if(action->pool->escrowServers.isEmpty())
        return;

    if(!action->pool->escrowServers.first()->isClient)
        return;

    SampleEscrowServerPtr server = action->pool->escrowServers.first();
    SampleEscrowTransactionPtr tx = server->GetClientTransaction(this->clientName, this->poolTxMap[action->pool->poolName].size());
    if(tx == NULL)
        return;

    tx->modules = this->modules;
    this->poolTxMap[action->pool->poolName].push_back(tx);

    ActionPtr fetchNextTx = ActionPtr(new Action());
    fetchNextTx->type = Action::FetchTransaction;
    fetchNextTx->pool = action->pool;
}

void SampleEscrowClient::InitializePool(EscrowPoolPtr pool)
{
    PoolAddressMap::iterator poolAddress = this->poolAddressMap.find(pool->poolName);
    PoolBalanceMap::iterator poolBalance = this->poolBalanceMap.find(pool->poolName);
    PoolTxCountMap::iterator poolTxCount = this->poolTxCountMap.find(pool->poolName);
    PoolTxMap::iterator poolTransactions = this->poolTxMap.find(pool->poolName);

    if(poolAddress == this->poolAddressMap.end())
        this->poolAddressMap[pool->poolName] = std::string();
    if(poolBalance == this->poolBalanceMap.end())
        this->poolBalanceMap[pool->poolName] = 0;
    if(poolTxCount == this->poolTxCountMap.end())
        this->poolTxCountMap[pool->poolName] = 0;
    if(poolTransactions == this->poolTxMap.end())
        this->poolTxMap[pool->poolName] = SampleEscrowTransactions();
}








