#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin/sampleescrowserver.hpp>

#include <bitcoin/sampleescrowclient.hpp>


#include <core/modules.hpp>

#include <opentxs/OTLog.hpp>


SampleEscrowServer::SampleEscrowServer(BitcoinServerPtr rpcServer)
{
    this->rpcServer = rpcServer;

    this->addressForMultiSig = "";
    this->pubKeyForMultiSig = "";
    this->multiSigAddress = "";

    this->serverPool = EscrowPoolPtr();
    this->publicKeys = btc::stringList();

    //this->transactionDeposit = SampleEscrowTransactionPtr();
    //this->transactionWithdrawal = SampleEscrowTransactionPtr();

    this->clientList = std::map<std::string, SampleEscrowClientPtr>();
    this->clientBalances = BalanceMap();

    this->modules = BtcModules::staticInstance;

    this->minSignatures = 2;

    this->minConfirms = 1;

    // generate random server name
    this->serverName = "                   ";
    gen_random((char*)this->serverName.c_str(), this->serverName.size());
}

void SampleEscrowServer::OnClientConnected(SampleEscrowClient *client)
{
    SampleEscrowClientPtr localClient = SampleEscrowClientPtr(new SampleEscrowClient());
    localClient->clientName = client->clientName;

    this->clientList[localClient->clientName] = localClient;
}

std::string SampleEscrowServer::OnRequestEscrowDeposit(const std::string &sender, int64_t amount)
{
    // abort if invalid name or amount less than withdrawal fee or client not officially connected yet
    if(sender.empty() || amount <= BtcHelper::FeeMultiSig || this->clientList.find(sender) == this->clientList.end())
        return "";

    this->modules->btcRpc->ConnectToBitcoin(this->rpcServer);

    // create new address to be used for creation of the multi-sig address
    // and get its public key:
    std::string pubKey = GetMultiSigPubKey();

    // also ask the other servers for their public keys
    foreach(SampleEscrowServerPtr server, this->serverPool->escrowServers)
    {
        if(server->serverName == this->serverName)
            continue;

        // ask server for his public key
        std::string pubKey = server->OnGetPubKey(this->serverName);
        if(pubKey.empty())
            return "";
        this->AddPubKey("", pubKey);
    }

    // generate the multi sig address
    this->multiSigAddrInfo = this->modules->mtBitcoin->GetMultiSigAddressInfo(this->minSignatures, this->publicKeys, true, "multiSigDeposit");
    if(this->multiSigAddrInfo != NULL)
        this->multiSigAddress = this->multiSigAddrInfo->address;

    return this->multiSigAddress;

    //this->transactionDeposit->targetAddr = this->multiSigAddress;

    // tell client our public key and how many signatures the pool requires, so client can recreate the address himself
    // we could also just tell him the address, it doesn't really matter
    //client->OnReceivePubKey(pubKey, this->minSignatures);    // transmitted over the network
}

std::string SampleEscrowServer::GetMultiSigPubKey()
{
    // see if we already created an address to be used for multi-sig
    if(this->addressForMultiSig.empty())
    {
        this->modules->btcRpc->ConnectToBitcoin(this->rpcServer);

        // if not, create one:
        this->addressForMultiSig = this->modules->mtBitcoin->GetNewAddress();
        if(this->addressForMultiSig.empty())
            return "";

        // and get its public key
        this->pubKeyForMultiSig = this->modules->mtBitcoin->GetPublicKey(this->addressForMultiSig);
        this->publicKeys.push_back(this->pubKeyForMultiSig);
    }

    return this->pubKeyForMultiSig;
}

std::string SampleEscrowServer::OnGetPubKey(const std::string &sender)
{
    std::string pubKey = GetMultiSigPubKey();

    return pubKey;

    // send our public key to the server asking for it
    //if(sender != "")
    //    this->serverPool->serverNameMap[sender]->OnReceivePubKey(this->serverName, pubKey);
}

void SampleEscrowServer::AddPubKey(const std::string &sender, const std::string &key)
{
    btc::stringList::iterator keyRef = std::find(this->publicKeys.begin(), this->publicKeys.end(), key);
    if(keyRef != this->publicKeys.end())
        this->publicKeys.erase(keyRef);
    this->publicKeys.push_back(key);

    // sort the keys alphabetically
    std::sort(this->publicKeys.begin(), this->publicKeys.end());
}

void SampleEscrowServer::AddClientDeposit(const std::string &client, SampleEscrowTransactionPtr transaction)
{
    if(this->clientBalances.find(client) == this->clientBalances.end())
        this->clientBalances[client] = std::list<SampleEscrowTransactionPtr>();

    this->clientBalances[client].push_back(transaction);

    OTLog::vOutput(0, "Added %s\n to %s\nClient %s now has %d deposit transaction(s)\n", transaction->txId.c_str(), transaction->targetAddr.c_str(), client.c_str(), this->clientBalances[client].size());
}

void SampleEscrowServer::RemoveClientDeposit(const std::string &client, SampleEscrowTransactionPtr transaction)
{
    if(this->clientBalances.find(client) == this->clientBalances.end())
        return;

    this->clientBalances[client].remove(transaction);
}

SampleEscrowTransactionPtr SampleEscrowServer::FindClientTransaction(const std::string &targetAddress, const std::string &txId, const std::string &client)
{
    OTLog::vOutput(0, "Looking for client %s's\n transaction %s\n to %s...\n", client.c_str(), txId.c_str(), targetAddress.c_str());
    if(this->clientBalances.find(client) == this->clientBalances.end())
        return SampleEscrowTransactionPtr();

    foreach(SampleEscrowTransactionPtr tx, this->clientBalances[client])
    {
        OTLog::vOutput(0, "    checking %s\n to %s...\n", tx->txId.c_str(), tx->targetAddr.c_str());
        if(tx->txId == txId && tx->targetAddr == targetAddress)
            return tx;
    }

    return SampleEscrowTransactionPtr();
}

SampleEscrowTransactionPtr SampleEscrowServer::FindClientTransaction(const std::string &targetAddress, const std::string txId)
{
    for(BalanceMap::iterator i = this->clientBalances.begin(); i != this->clientBalances.end(); i++)
    {        
        foreach(SampleEscrowTransactionPtr tx, i->second)
        {
            if(tx->txId == txId && tx->targetAddr == targetAddress)
                return tx;
        }
    }

    return SampleEscrowTransactionPtr();
}

void SampleEscrowServer::OnIncomingDeposit(const std::string sender, const std::string txId, int64_t amount)
{
    if(sender.empty() || txId.empty() || amount <= BtcHelper::FeeMultiSig || this->clientList.find(sender) == this->clientList.end())
        return;

    // we will need to keep a list of all deposit transaction ids so we know everyone's balance
    // and because we need the tx ids to withdraw later

    // create a new object containing information about this deposit
    SampleEscrowTransactionPtr deposit = SampleEscrowTransactionPtr(new SampleEscrowTransaction(amount));
    deposit->targetAddr = this->multiSigAddress;
    deposit->txId = txId;
    deposit->status = SampleEscrowTransaction::Pending;

    // add the transaction object to this client's tx list
    this->AddClientDeposit(sender, deposit);
}

bool SampleEscrowServer::CheckTransaction(const std::string &targetAddress, const std::string txId, const std::string &sender)
{   
    OTLog::vOutput(0, "Checking %s's transactions for\n %s to %s...\n", sender.c_str(), txId.c_str(), targetAddress.c_str());
    if(this->clientBalances[sender].empty())
        return false;

    SampleEscrowTransactionPtr deposit = FindClientTransaction(targetAddress, txId, sender);
    if(deposit == NULL)
        return false;

    this->modules->btcRpc->ConnectToBitcoin(this->rpcServer);

    // check transaction for correct amount and number of confirmations
    deposit->CheckTransaction(this->minConfirms);

    if(deposit->status != SampleEscrowTransaction::Pending)
        return true;    // if transaction isn't pending anymore, we're finished

    return false;
}

std::string SampleEscrowServer::OnRequestEscrowWithdrawal(const std::string &sender, const std::string &txId, const std::string &fromAddress, const std::string &toAddress)
{
    this->modules->btcRpc->ConnectToBitcoin(this->rpcServer);

    SampleEscrowTransactionPtr deposit = FindClientTransaction(fromAddress, txId, sender);
    if(deposit == NULL)
        return "";

    // update transaction status
    deposit->CheckTransaction(this->minConfirms);

    // check if deposit is confirmed
    if(deposit->status != SampleEscrowTransaction::Successfull)
        return "";     // nope

    // create new transaction object
    SampleEscrowTransactionPtr txRelease = SampleEscrowTransactionPtr(new SampleEscrowTransaction(deposit->amountToSend));

    // create partially signed raw transaction
    txRelease->CreateWithdrawalTransaction(txId, fromAddress, toAddress);

    return txRelease->withdrawalTransaction;


    // send the partially signed transaction to the client
    //sender->OnReceiveSignedTx(this->transactionWithdrawal->withdrawalTransaction);
}

void SampleEscrowServer::ReleasingFunds(const std::string &client, const std::string &txId, const std::string &sourceTxId, const std::string &sourceAddress, const std::string &toAddress)
{
    if(client.empty() || txId.empty() || sourceAddress.empty())
        return;

    this->modules->btcRpc->ConnectToBitcoin(this->rpcServer);

    SampleEscrowTransactionPtr deposit = FindClientTransaction(sourceAddress, sourceTxId, client);
    if(deposit == NULL)
        return;

    // we will just assume that the transaction goes through because this is just an example
    this->RemoveClientDeposit(client, deposit);
}
