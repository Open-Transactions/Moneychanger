#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin/sampleescrowserver.hpp>
#include <bitcoin/sampleescrowclient.hpp>

#include <core/modules.hpp>

#include <opentxs/OTLog.hpp>

#include <QTimer>
#include <QMutex>

#include <cstdio>
#include <cstdlib>
#include <algorithm>

time_t prevTime = 0;

struct SampleEscrowServer::ClientRequest
{
    enum Action
    {
        Unknown,
        CreatePubKey,
        CreateMultisig,
        StartReleaseDeposit,
        SendReleaseTx
    };

    std::string client;
    std::string address;
    int64_t amount;
    Action action;
};

CheckTxDaemon::CheckTxDaemon(SampleEscrowServer *master, QObject *parent)
    :QObject(parent)
{
    this->master = master;
}

void CheckTxDaemon::StartDaemon()
{
    while(!this->master->shutDown && !Modules::shutDown)
    {
        btc::Sleep(10000);
        this->master->CheckTransactions();
    }

    this->master = NULL;
}

SampleEscrowServer::SampleEscrowServer(BitcoinServerPtr rpcServer, EscrowPoolPtr pool, QObject* parent)
    :QThread(parent)
{
    this->rpcServer = rpcServer;

    this->serverPool = pool;
    this->minSignatures = this->serverPool->sigsRequired;

    this->modules = BtcModulesPtr(new BtcModules());

    this->minConfirms = BtcHelper::WaitForConfirms;

    this->isClient = true;

    // generate random server name
    this->serverName = "                   ";
    gen_random((char*)this->serverName.c_str(), this->serverName.size());

    this->mutex = new QMutex(QMutex::Recursive);

    this->shutDown = false;

    this->modules->btcRpc->ConnectToBitcoin(this->rpcServer);
}

SampleEscrowServer::~SampleEscrowServer()
{
    this->shutDown = true;
    wait();
    delete this->mutex;
    this->mutex = NULL;
}

bool SampleEscrowServer::ClientConnected(SampleEscrowClient *client)
{
    this->mutex->lock();

    SampleEscrowClientPtr localClient = SampleEscrowClientPtr(new SampleEscrowClient());
    localClient->clientName = client->clientName;

    this->clientList[localClient->clientName] = localClient;

    this->mutex->unlock();

    return true;
}

void SampleEscrowServer::InitializeClient(const std::string &client)
{
    this->mutex->lock();

    std::printf("Clearing temporary deposit info for client %s\n", client.c_str());
    std::cout.flush();

    this->addressForMultiSig[client] = std::string();
    this->pubKeyForMultiSig[client] = std::string();
    this->multiSigAddress[client] = std::string();
    this->publicKeys[client] = btc::stringList();
    this->clientReleaseTxMap[client] = BtcSignedTransactionPtr();

    this->mutex->unlock();
}

void SampleEscrowServer::StartServerLoop()
{
    CheckTxDaemon* checkTx = new CheckTxDaemon(this);
    QThread* checkTxThread = new QThread();
    checkTx->moveToThread(checkTxThread);
    checkTxThread->start();
    QMetaObject::invokeMethod(checkTx, "StartDaemon", Qt::QueuedConnection);

    while(!this->shutDown && !Modules::shutDown)
    {
        Update();
        btc::Sleep(10);
    }
}

void SampleEscrowServer::Update()
{
    this->mutex->lock();

    foreach(ClientRequestPtr request, this->clientRequests)
    {
        switch(request->action)
        {
        case ClientRequest::CreatePubKey:
        {
            // create new address to be used for creation of the multi-sig address
            // and get its public key:
            std::string pubKey = CreatePubKey(request->client);

            break;
        }
        case ClientRequest::CreateMultisig:
        {
            // also ask the other servers for their public keys
            foreach(SampleEscrowServerPtr server, this->serverPool->escrowServers)
            {
                // skip ourselves
                if(server->serverName == this->serverName)
                    continue;

                // ask server for his public key
                std::string pubKeyOther = server->GetPubKey(request->client);

                this->AddPubKey(request->client, pubKeyOther);
            }

            // if we don't have enough keys, try again later
            if(this->publicKeys[request->client].size() < static_cast<size_t>(this->serverPool->escrowServers.size()) || this->publicKeys[request->client].size() < this->serverPool->sigsRequired)
            {
                ClientRequestPtr createMultiSig = ClientRequestPtr(new ClientRequest());
                createMultiSig->action = ClientRequest::CreateMultisig;
                createMultiSig->client = request->client;
                this->clientRequests.push_back(createMultiSig);
                break;
            }

            if(this->publicKeys[request->client].size() > static_cast<size_t>(this->serverPool->escrowServers.size()))
            {
                InitializeClient(request->client);
                break;
            }

            // generate the multisig address
            BtcMultiSigAddressPtr multiSigAddrInfo = this->modules->mtBitcoin->GetMultiSigAddressInfo(this->minSignatures, this->publicKeys[request->client], true, "multiSigDeposit");
            if(multiSigAddrInfo != NULL)
            {
                this->multiSigAddress[request->client] = multiSigAddrInfo->address;
                this->addressToClientMap[multiSigAddrInfo->address] = request->client;
                this->modules->btcJson->ImportAddress(multiSigAddrInfo->address, "multisigdeposit", false);
                if(std::find(this->multiSigAddresses.begin(), this->multiSigAddresses.end(), multiSigAddrInfo->address) == this->multiSigAddresses.end())
                    this->multiSigAddresses.push_back(multiSigAddrInfo->address);

                std::printf("server %s generated multisig address %s\n", this->serverName.c_str(), multiSigAddrInfo->address.c_str());
                std::cout.flush();
            }

            break;
        }
        case ClientRequest::StartReleaseDeposit:
        {
            // find enough outputs to cover transaction + fee
            BtcUnspentOutputs outputsToSpend = GetOutputsToSpend(request->client, request->amount + BtcHelper::FeeMultiSig);

            if (outputsToSpend.size() == 0)
            {
                std::printf("Insufficient funds.\n");
                std::cout.flush();
                break;
            }
            else if (this->multiSigAddress[request->client].empty())
            {
                // create change key
                ClientRequestPtr createPubKey = ClientRequestPtr(new ClientRequest());
                createPubKey->action = ClientRequest::CreatePubKey;
                createPubKey->client = request->client;
                this->clientRequests.push_back(createPubKey);

                // create change address
                ClientRequestPtr createMultisig = ClientRequestPtr(new ClientRequest());
                createMultisig->action = ClientRequest::CreateMultisig;
                createMultisig->client = request->client;
                this->clientRequests.push_back(createMultisig);

                ClientRequestPtr startRelease = ClientRequestPtr(new ClientRequest());
                startRelease->client = request->client;
                startRelease->action = request->action;
                startRelease->address = request->address;
                startRelease->amount = request->amount;
                this->clientRequests.push_back(startRelease);

                break;
            }

            // create unsigned transaction to send to client address and change in case there is any to change address
            BtcSignedTransactionPtr releaseTx = this->modules->btcHelper->CreateSpendTransaction(outputsToSpend, request->amount, request->address, this->multiSigAddress[request->client]);
            if(releaseTx == NULL)
                break;

            size_t txLength = releaseTx->signedTransaction.size();
            releaseTx = this->modules->btcJson->SignRawTransaction(releaseTx->signedTransaction);

            // check if signing failed
            if(releaseTx->signedTransaction.size() <= txLength)
                break;

            this->clientReleaseTxMap[request->client] = releaseTx;

            // ask other servers for signed transactions
            ClientRequestPtr sendTx = ClientRequestPtr(new ClientRequest());
            sendTx->action = ClientRequest::SendReleaseTx;
            sendTx->client = request->client;
            sendTx->address = request->address;
            sendTx->amount = request->amount;
            this->clientRequests.push_back(sendTx);

            break;
        }
        case ClientRequest::SendReleaseTx:
        {
            BtcSignedTransactionPtr releaseTx = this->clientReleaseTxMap[request->client];
            if(releaseTx == NULL || releaseTx->signedTransaction.empty())
                break;

            bool serverReturnedNull = false;
            foreach(SampleEscrowServerPtr server, this->serverPool->escrowServers)
            {
                if(server->serverName == this->serverName)
                    continue;

                std::string partiallySignedTx = server->RequestSignedWithdrawal(request->client);
                if(partiallySignedTx.empty())
                    serverReturnedNull = true;

                releaseTx->signedTransaction += partiallySignedTx;
                releaseTx = this->modules->btcJson->CombineSignedTransactions(releaseTx->signedTransaction);

                // don't add more signatures than necessary
                if(releaseTx != NULL && releaseTx->complete)
                    break;
            }

            if(releaseTx == NULL || !releaseTx->complete)
            {
                if(serverReturnedNull)
                {
                    bool failed = false;
                    foreach (SampleEscrowServerPtr server, this->serverPool->escrowServers)
                    {
                        if(server->serverName == this->serverName)
                            continue;

                        if(!server->RequestEscrowWithdrawal(request->client, request->amount, request->address))
                        {
                            failed = true;
                            InitializeClient(request->client);
                            break;
                        }
                    }

                    if(failed)
                        break;

                    ClientRequestPtr reStartRelease = ClientRequestPtr(new ClientRequest());
                    reStartRelease->client = request->client;
                    reStartRelease->action = request->action;
                    reStartRelease->address = request->address;
                    reStartRelease->amount = request->amount;
                    this->clientRequests.push_back(reStartRelease);
                }
                break;
            }

            SampleEscrowTransactionPtr tx = SampleEscrowTransactionPtr(new SampleEscrowTransaction(request->amount, this->modules));
            tx->targetAddr = request->address;
            tx->type = SampleEscrowTransaction::Release;
            this->clientHistoryMap[request->client].push_back(tx);

            tx->txId = this->modules->btcJson->SendRawTransaction(releaseTx->signedTransaction);

            break;
        }
        default:
            break;
        }

        this->clientRequests.remove(request);
        if(!(std::rand() % 5))
        {
            ClientRequestPtr request = ClientRequestPtr(new ClientRequest());
            request->action = ClientRequest::Unknown;
            this->clientRequests.push_back(request);
            request = ClientRequestPtr(new ClientRequest());
            request->action = ClientRequest::Unknown;
            this->clientRequests.push_back(request);
        }
        break;
    }
    this->mutex->unlock();
}

bool SampleEscrowServer::RequestEscrowDeposit(const std::string &client, const int64_t &amount)
{
    // abort if invalid name or amount less than withdrawal fee or client not officially connected yet
    if(client.empty() || amount <= BtcHelper::FeeMultiSig || this->clientList.find(client) == this->clientList.end())
        return false;

    this->mutex->lock();

    // create change address
    ClientRequestPtr createPubKey = ClientRequestPtr(new ClientRequest());
    createPubKey->action = ClientRequest::CreatePubKey;
    createPubKey->client = client;
    this->clientRequests.push_back(createPubKey);

    ClientRequestPtr request = ClientRequestPtr(new ClientRequest());
    request->action = ClientRequest::CreateMultisig;
    request->client = client;
    this->clientRequests.push_back(request);

    this->mutex->unlock();

    return true;


    //return this->multiSigAddress[client];
}

std::string SampleEscrowServer::RequestDepositAddress(const std::string &client)
{
    return this->multiSigAddress[client];
}

std::string SampleEscrowServer::CreatePubKey(const std::string &client)
{
    this->mutex->lock();

    // see if we already created an address to be used for multi-sig
    ClientAddressMap::iterator pubKey = this->pubKeyForMultiSig.find(client);
    if(pubKey != this->pubKeyForMultiSig.end() && !pubKey->second.empty())
    {
        this->mutex->unlock();
        return pubKey->second;
    }

    // if not, create one:
    this->addressForMultiSig[client] = this->modules->mtBitcoin->GetNewAddress();
    if(this->addressForMultiSig[client].empty())
    {
        this->mutex->unlock();
        return std::string();
    }

    // and get its public key
    this->pubKeyForMultiSig[client] = this->modules->mtBitcoin->GetPublicKey(this->addressForMultiSig[client]);
    AddPubKey(client, this->pubKeyForMultiSig[client]);

    this->mutex->unlock();
    return this->pubKeyForMultiSig[client];
}

std::string SampleEscrowServer::GetPubKey(const std::string &client)
{
    return CreatePubKey(client);
}

void SampleEscrowServer::AddPubKey(const std::string &client, const std::string &key)
{
    if(key.empty())
        return;

    this->mutex->lock();

    btc::stringList::iterator keyRef = std::find(this->publicKeys[client].begin(), this->publicKeys[client].end(), key);
    if(keyRef == this->publicKeys[client].end())
        this->publicKeys[client].push_back(key);

    // sort the keys alphabetically
    std::sort(this->publicKeys[client].begin(), this->publicKeys[client].end());

    std::printf("adding key %s\nserver %s has %lu pubkeys for client %s\n", key.c_str(), this->serverName.c_str(), this->publicKeys[client].size(), client.c_str());
    std::cout.flush();

    this->mutex->unlock();
}

void SampleEscrowServer::AddClientDeposit(const std::string &client, SampleEscrowTransactionPtr transaction, bool oldTx)
{
    this->mutex->lock();

    transaction->type = SampleEscrowTransaction::Deposit;

    foreach(SampleEscrowTransactionPtr tx, this->clientBalancesMap[client])
    {
        if(tx->txId == transaction->txId && tx->targetAddr == transaction->targetAddr)
        {
            this->mutex->unlock();
            return;
        }
    }
    this->clientBalancesMap[client].push_back(transaction);
    if(!oldTx)
        this->clientHistoryMap[client].push_back(transaction);

    OTLog::vOutput(0, "Added %s\n to %s\nClient %s now has %d deposit transaction(s)\n", transaction->txId.c_str(), transaction->targetAddr.c_str(), client.c_str(), this->clientBalancesMap[client].size());

    this->mutex->unlock();
}

void SampleEscrowServer::RemoveClientDeposit(const std::string &client, SampleEscrowTransactionPtr transaction)
{
    this->mutex->lock();

    for(SampleEscrowTransactions::iterator tx = this->clientBalancesMap[client].begin(); tx != this->clientBalancesMap[client].end(); tx++)
    {
        if((*tx)->txId == transaction->txId && (*tx)->vout == transaction->vout)
        {
            this->clientBalancesMap[client].remove((*tx));
            tx = this->clientBalancesMap[client].begin();
        }
    }

    this->mutex->unlock();
}

void SampleEscrowServer::CheckTransactions()
{
    this->mutex->lock();

    BtcUnspentOutputs unspentOutputs = BtcUnspentOutputs();
    for(ClientBalanceMap::iterator clientBalances = this->clientBalancesMap.begin(); clientBalances != this->clientBalancesMap.end(); clientBalances++)
    {
        foreach(SampleEscrowTransactionPtr tx, clientBalances->second)
        {
            BtcUnspentOutputPtr outputFromTx;
            if((outputFromTx = this->modules->btcJson->GetTxOut(tx->txId, tx->vout)) == NULL)
            {
                InitializeClient(clientBalances->first);
                RemoveClientDeposit(clientBalances->first, tx);
                continue;
            }

            if(tx->status == SampleEscrowTransaction::Pending)
                tx->CheckTransaction(this->minConfirms);

            unspentOutputs.push_back(outputFromTx);           
        }
    }

    // look for new transactions to multisig addresses
    BtcUnspentOutputs incomingTransactions = this->modules->btcHelper->ListNewOutputs(unspentOutputs, this->multiSigAddresses);

    foreach(BtcUnspentOutputPtr output, incomingTransactions)
    {
        int64_t amount = output->amount;
        SampleEscrowTransactionPtr clientTx = SampleEscrowTransactionPtr(new SampleEscrowTransaction(amount, this->modules));
        clientTx->txId = output->txId;
        clientTx->targetAddr = output->address;
        clientTx->vout = output->vout;
        clientTx->scriptPubKey = output->scriptPubKey;

        clientTx->CheckTransaction(this->minConfirms);

        std::string client = this->addressToClientMap[output->address];
        AddClientDeposit(client, clientTx, false);
        // clear temporary deposit info after new deposits so that we generate a new address next time client asks
        InitializeClient(client);

    }

    this->mutex->unlock();
}

SampleEscrowTransactionPtr SampleEscrowServer::FindClientTransaction(const std::string &targetAddress, const std::string &txId, const std::string &client)
{
    this->mutex->lock();

    if(this->clientBalancesMap.find(client) == this->clientBalancesMap.end())
    {
        this->mutex->unlock();
        return SampleEscrowTransactionPtr();
    }

    foreach(SampleEscrowTransactionPtr tx, this->clientBalancesMap[client])
    {
        if(tx->txId == txId && tx->targetAddr == targetAddress)
        {
            this->mutex->unlock();
            return tx;
        }
    }

    this->mutex->unlock();
    return SampleEscrowTransactionPtr();
}

int64_t SampleEscrowServer::GetClientBalance(const std::string &client)
{
    this->mutex->lock();

    ClientBalanceMap::iterator clientBalanceMap = this->clientBalancesMap.find(client);
    if(clientBalanceMap == this->clientBalancesMap.end())
    {
        this->mutex->unlock();
        return int64_t(0);
    }

    // iterate through all transactions associated with this client
    int64_t balance = int64_t(0);
    foreach(SampleEscrowTransactionPtr tx, clientBalanceMap->second)
    {
        if(tx->status == tx->Successfull)
            balance += tx->amountToSend;    // add value to overall balance
    }

    this->mutex->unlock();
    return balance;
}

u_int64_t SampleEscrowServer::GetClientTransactionCount(const std::string &client)
{
    this->mutex->lock();

    ClientBalanceMap::iterator clientHistory = this->clientHistoryMap.find(client);

    if(clientHistory == this->clientHistoryMap.end())
    {
        this->mutex->unlock();
        return 0;
    }

    u_int64_t size = clientHistory->second.size();
    this->mutex->unlock();
    return size;
}

SampleEscrowTransactionPtr SampleEscrowServer::GetClientTransaction(const std::string &client, u_int64_t txIndex)
{
    this->mutex->lock();

    ClientBalanceMap::iterator clientHistory = this->clientHistoryMap.find(client);
    if(clientHistory == this->clientHistoryMap.end())
    {
        this->mutex->unlock();
        return SampleEscrowTransactionPtr();
    }

    if(clientHistory->second.size() <= txIndex)
    {
        this->mutex->unlock();
        return SampleEscrowTransactionPtr();
    }

    SampleEscrowTransactions::iterator tx = clientHistory->second.begin();
    std::advance(tx, txIndex);      // silly c++03
    if(tx == clientHistory->second.end())
    {
        this->mutex->unlock();
        return SampleEscrowTransactionPtr();
    }

    this->mutex->unlock();
    return (*tx);
}

BtcUnspentOutputs SampleEscrowServer::GetOutputsToSpend(const std::string &client, const int64_t &amountToSpend)
{
    this->mutex->lock();

    BtcUnspentOutputs outputsToSpend = BtcUnspentOutputs();

    ClientBalanceMap::iterator clientBalanceMap = this->clientBalancesMap.find(client);
    if(clientBalanceMap == this->clientBalancesMap.end())
    {
        this->mutex->unlock();
        return outputsToSpend;        
    }

    int64_t currentBalance = int64_t(0);
    foreach(SampleEscrowTransactionPtr tx, clientBalanceMap->second)
    {
        if(std::find(this->multiSigAddresses.begin(), this->multiSigAddresses.end(), tx->targetAddr) == this->multiSigAddresses.end())
            continue;

        BtcUnspentOutputPtr output = BtcUnspentOutputPtr(new BtcUnspentOutput(Json::Value(Json::objectValue)));
        output->txId = tx->txId;
        output->address = tx->targetAddr;
        output->amount = tx->amountToSend;
        output->vout = tx->vout;
        output->scriptPubKey = tx->scriptPubKey;
        outputsToSpend.push_back(output);
        currentBalance += tx->amountToSend;
    }

    this->mutex->unlock();

    if(currentBalance >= amountToSpend)
        return outputsToSpend;
    else
        return BtcUnspentOutputs();
}

bool SampleEscrowServer::RequestEscrowWithdrawal(const std::string &client, const int64_t &amount, const std::string &toAddress)
{
    this->mutex->lock();

    // check client balance
    int64_t spendable = GetClientBalance(client);
    if(spendable < amount + BtcHelper::FeeMultiSig)
    {
        this->mutex->unlock();
        return false;
    }

    ClientRequestPtr request = ClientRequestPtr(new ClientRequest());
    request->client = client;
    request->amount = amount;
    request->address = toAddress;
    request->action = ClientRequest::StartReleaseDeposit;
    this->clientRequests.push_back(request);

    this->mutex->unlock();

    return true;
}

std::string SampleEscrowServer::RequestSignedWithdrawal(const std::string &client)
{
    this->mutex->lock();

    if(this->clientReleaseTxMap[client] != NULL)
    {
        this->mutex->unlock();
        return this->clientReleaseTxMap[client]->signedTransaction;
    }
    else
    {
        this->mutex->unlock();
        return std::string();
    }
}
