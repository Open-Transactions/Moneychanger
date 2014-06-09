#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin/sampleescrowserver.hpp>
#include <bitcoin/sampleescrowclient.hpp>

#include <core/modules.hpp>

#include <opentxs/OTLog.hpp>

#include <QTimer>
#include <QMutex>


struct SampleEscrowServer::ClientRequest
{
    enum Action
    {
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

SampleEscrowServer::SampleEscrowServer(BitcoinServerPtr rpcServer, EscrowPoolPtr pool, QObject* parent)
    :QObject(parent)
{
    this->rpcServer = rpcServer;

    this->addressForMultiSig = ClientAddressMap();
    this->pubKeyForMultiSig = ClientKeyMap();
    this->multiSigAddress = ClientMultiSigMap();
    this->publicKeys = ClientKeyListMap();
    this->clientBalancesMap = ClientBalanceMap();
    this->clientReleaseTxMap = ClientReleaseTxMap();
    this->addressToClientMap = AddressClientMap();

    this->multiSigAddresses = btc::stringList();

    this->serverPool = pool;
    this->minSignatures = this->serverPool->sigsRequired;

    this->clientList = std::map<std::string, SampleEscrowClientPtr>();

    this->modules = BtcModulesPtr(new BtcModules());

    this->minConfirms = BtcHelper::WaitForConfirms;

    // generate random server name
    this->serverName = "                   ";
    gen_random((char*)this->serverName.c_str(), this->serverName.size());

    this->updateTimer = new QTimer(parent);
    updateTimer->setInterval(1000);
    updateTimer->start();
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(Update()));

    this->mutex = new QMutex(QMutex::Recursive);

    this->modules->btcRpc->ConnectToBitcoin(this->rpcServer);
}

SampleEscrowServer::~SampleEscrowServer()
{
    this->updateTimer->stop();
    delete this->updateTimer;
    this->updateTimer = NULL;

    this->mutex->unlock();
    delete this->mutex;
    this->mutex = NULL;
}

void SampleEscrowServer::ClientConnected(SampleEscrowClient *client)
{
    this->mutex->lock();

    SampleEscrowClientPtr localClient = SampleEscrowClientPtr(new SampleEscrowClient());
    localClient->clientName = client->clientName;

    this->clientList[localClient->clientName] = localClient;

    this->mutex->unlock();
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

int transactionCount = 30;
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
                if(pubKeyOther.empty())
                    continue;
                this->AddPubKey(request->client, pubKeyOther);
            }

            // if we have don't enough keys, try again later
            if(this->publicKeys[request->client].size() < static_cast<size_t>(this->serverPool->escrowServers.size()))
            {
                ClientRequestPtr createMultiSig = ClientRequestPtr(new ClientRequest());
                createMultiSig->client = request->client;
                this->clientRequests.push_back(createMultiSig);
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

                ClientRequestPtr startRelease = ClientRequestPtr(new ClientRequest());
                startRelease->client = request->client;
                startRelease->action = request->action;
                startRelease->address = request->address;
                startRelease->amount = request->amount;
                this->clientRequests.push_back(startRelease);
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
                releaseTx = BtcSignedTransactionPtr(new BtcSignedTransaction(Json::Value(Json::objectValue)));
            releaseTx = this->modules->btcJson->SignRawTransaction(releaseTx->signedTransaction);

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
            if(releaseTx == NULL)
                releaseTx = BtcSignedTransactionPtr(new BtcSignedTransaction(Json::Value(Json::objectValue)));

            foreach(SampleEscrowServerPtr server, this->serverPool->escrowServers)
            {
                if(server->serverName == this->serverName)
                    continue;

                std::string partiallySignedTx = server->RequestSignedWithdrawal(request->client);
                releaseTx->signedTransaction += partiallySignedTx;
                releaseTx = this->modules->btcJson->CombineSignedTransactions(releaseTx->signedTransaction);

                if(releaseTx != NULL && releaseTx->complete)
                    break;
            }

            if(releaseTx == NULL || !releaseTx->complete)
                break;

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
        break;
    }
    this->mutex->unlock();

    if(transactionCount-- == 0)
    {
        // this lags so let's not do it too often
        transactionCount = 30;
        CheckTransactions();
    }
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
    this->mutex->lock();

    return this->multiSigAddress[client];

    this->mutex->unlock();
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

    this->mutex->lock();

    return this->pubKeyForMultiSig[client];
}

std::string SampleEscrowServer::GetPubKey(const std::string &client)
{
    return CreatePubKey(client);
}

void SampleEscrowServer::AddPubKey(const std::string &client, const std::string &key)
{
    this->mutex->lock();

    btc::stringList::iterator keyRef = std::find(this->publicKeys[client].begin(), this->publicKeys[client].end(), key);
    if(keyRef == this->publicKeys[client].end())
        this->publicKeys[client].push_back(key);

    // sort the keys alphabetically
    std::sort(this->publicKeys[client].begin(), this->publicKeys[client].end());

    this->mutex->unlock();
}

void SampleEscrowServer::AddClientDeposit(const std::string &client, SampleEscrowTransactionPtr transaction)
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
    this->clientHistoryMap[client].push_back(transaction);

    OTLog::vOutput(0, "Added %s\n to %s\nClient %s now has %d deposit transaction(s)\n", transaction->txId.c_str(), transaction->targetAddr.c_str(), client.c_str(), this->clientBalancesMap[client].size());

    this->mutex->unlock();
}

void SampleEscrowServer::RemoveClientDeposit(const std::string &client, SampleEscrowTransactionPtr transaction)
{
    this->mutex->lock();

    for(SampleEscrowTransactions::iterator tx = this->clientBalancesMap[client].begin(); tx != this->clientBalancesMap[client].end(); tx++)
    {
        if((*tx)->txId == transaction->txId && (*tx)->targetAddr == transaction->targetAddr)
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

    // look for new transactions
    BtcUnspentOutputs balances = this->modules->btcJson->ListUnspent(0, BtcHelper::MaxConfirms, this->multiSigAddresses); //ListReceivedByAddress(int32_t(0), false, true);
    foreach(BtcUnspentOutputPtr balance, balances)
    {
        AddressClientMap::iterator client = this->addressToClientMap.find(balance->address);
        if(client == this->addressToClientMap.end())
            continue;

        // see if tx is already known
        SampleEscrowTransactionPtr clientTx = FindClientTransaction(balance->address, balance->txId, client->second);
        if(clientTx != NULL)
            continue;

        BtcRawTransactionPtr rawTx = this->modules->btcHelper->GetDecodedRawTransaction(balance->txId);
        int64_t amount = this->modules->btcHelper->GetTotalOutput(rawTx, balance->address);
        clientTx = SampleEscrowTransactionPtr(new SampleEscrowTransaction(amount, this->modules));
        clientTx->txId = balance->txId;
        clientTx->targetAddr = balance->address;
        AddClientDeposit(client->second, clientTx);

        // clear temporary deposit info
        InitializeClient(client->second);

    }

    for(ClientBalanceMap::iterator clientBalanceMap = this->clientBalancesMap.begin(); clientBalanceMap != this->clientBalancesMap.end(); clientBalanceMap++)
    {
        foreach(SampleEscrowTransactionPtr tx, clientBalanceMap->second)
        {
            // check if transaction isn't spent yet
            bool spent = true;
            BtcUnspentOutputs outputs = this->modules->btcHelper->FindUnspentSignableOutputs(btc::stringList { tx->txId });
            foreach(BtcUnspentOutputPtr output, outputs)
            {
                // found it in 'listunspent'
                if(output->address == tx->targetAddr)
                {
                    spent = false;
                    tx->amountToSend = output->amount;
                }
            }
            if(spent)
                tx->status = SampleEscrowTransaction::Spent;

            if(tx->status == SampleEscrowTransaction::Successfull)
                continue;

            if(tx->status == SampleEscrowTransaction::Pending)
            {
                tx->CheckTransaction(this->minConfirms);
            }

            if(tx->status == SampleEscrowTransaction::Conflicted)
            {
                foreach (std::string txId, this->modules->btcHelper->GetDoubleSpends(tx->txId))
                {
                    if(this->modules->btcHelper->TransactionConfirmed(txId, this->minConfirms))
                        tx->status = SampleEscrowTransaction::Failed;

                    break;
                }
            }

            if(tx->status == SampleEscrowTransaction::Failed || tx->status == SampleEscrowTransaction::Spent)
            {
                std::printf("removing tx %s\n to address %s\n from client %s\n", tx->txId.c_str(), tx->targetAddr.c_str(), clientBalanceMap->first.c_str());
                RemoveClientDeposit(clientBalanceMap->first, tx);
                break;
            }
        }
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
        return 0;
    }

    // iterate through all transactions associated with this client
    int64_t balance = 0;
    foreach(SampleEscrowTransactionPtr tx, clientBalanceMap->second)
    {
        // recheck transaction status
        tx->CheckTransaction(this->minConfirms);
        if(tx->status == tx->Successfull)
            balance += tx->amountToSend;    // add value to overall balance
    }

    this->mutex->unlock();
    return balance;
}

int32_t SampleEscrowServer::GetClientTransactionCount(const std::string &client)
{
    this->mutex->lock();

    ClientBalanceMap::iterator clientHistory = this->clientHistoryMap.find(client);

    this->mutex->unlock();

    if(clientHistory == this->clientHistoryMap.end())
        return 0;

    return clientHistory->second.size();
}

SampleEscrowTransactionPtr SampleEscrowServer::GetClientTransaction(const std::string &client, u_int32_t txIndex)
{
    this->mutex->lock();

    ClientBalanceMap::iterator clientHistory = this->clientHistoryMap.find(client);
    if(clientHistory == this->clientHistoryMap.end())
    {
        this->mutex->unlock();
        return SampleEscrowTransactionPtr();
    }

    if(static_cast<uint32_t>(clientHistory->second.size()) <= txIndex)
        return SampleEscrowTransactionPtr();

    SampleEscrowTransactions::iterator tx = std::next(clientHistory->second.begin(), txIndex);
    if(tx == clientHistory->second.end())
        return SampleEscrowTransactionPtr();

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

    int64_t currentBalance = 0;
    foreach(SampleEscrowTransactionPtr tx, clientBalanceMap->second)
    {
        BtcUnspentOutputs txOutputs = this->modules->btcHelper->FindUnspentSignableOutputs(btc::stringList {tx->txId});
        foreach(BtcUnspentOutputPtr output, txOutputs)
        {
            // one transaction could be depositing into different addresses for different accounts
            // only pick the transaction for this client
            if(output->address != tx->targetAddr)
                continue;

            outputsToSpend.push_back(output);
            currentBalance += output->amount;
            break;  // there can only be one output per address per transaction
        }
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

    this->modules->btcRpc->ConnectToBitcoin(this->rpcServer);

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
