#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin/sampleescrowclient.hpp>

#include <bitcoin/sampleescrowserver.hpp>


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

SampleEscrowClient::SampleEscrowClient(QObject* parent)
    :QObject(parent)
{   
    this->rpcServer = BitcoinServerPtr(new BitcoinServer("admin1", "123", "http://127.0.0.1", 19001));

    this->minSignatures = 0;    // will be set later by server pool

    this->minConfirms = BtcHelper::WaitForConfirms;      // wait for one confirmation by default

    this->targetPool = EscrowPoolPtr();
    this->pubKeyList = btc::stringList();

    this->transactionDeposit = SampleEscrowTransactionPtr();
    this->transactionWithdrawal = SampleEscrowTransactionPtr();

    this->modules = BtcModules::staticInstance;

    // generate random client name
    this->clientName = "                   ";
    gen_random((char*)this->clientName.c_str(), this->clientName.size());
}

SampleEscrowClient::~SampleEscrowClient()
{
    this->rpcServer.reset();
    this->targetPool.reset();
    this->pubKeyList.clear();
    this->transactionDeposit.reset();
    this->transactionWithdrawal.reset();
}

void SampleEscrowClient::StartDeposit(int64_t amountToSend, EscrowPoolPtr targetPool)
{
    this->modules->btcRpc->ConnectToBitcoin(this->rpcServer);

    this->targetPool = targetPool;

    this->transactionDeposit = SampleEscrowTransactionPtr(new SampleEscrowTransaction(amountToSend));

    // ask the servers for an address to send money to
    // assuming no BIP32 and assuming servers don't constantly monitor the blockchain for incoming transactions
    // in reality this call would have to be forwarded through the net
    std::string depositAddress;
    foreach(SampleEscrowServerPtr server, this->targetPool->escrowServers)
    {
        server->OnClientConnected(this);

        depositAddress = server->OnRequestEscrowDeposit(this->clientName, amountToSend);
        if(depositAddress.empty())
            break;  // error
    }

    this->transactionDeposit->targetAddr = depositAddress;

    SendToEscrow();
}

void SampleEscrowClient::OnReceivePubKey(const std::string &publicKey, int minSignatures)
{
    if(publicKey == "")
        return;

    this->pubKeyList.push_back(publicKey);         // add server's key to keylist
    std::sort(this->pubKeyList.begin(), this->pubKeyList.end());                    // somehow arrange all keys in the same order.

    if(this->minSignatures == 0)                // set number of required signatures
        this->minSignatures = minSignatures;
    if(this->minSignatures != minSignatures)    // see if all servers agree
        return;     // OMG HACKS!!!11

    // if we received all public keys we can start sending the bitcoins
    if(this->pubKeyList.size() == (uint)this->targetPool->escrowServers.size())
    {
        // create the multi-sig address of the server pool
        this->transactionDeposit->targetAddr = this->modules->mtBitcoin->GetMultiSigAddress(this->minSignatures, this->pubKeyList);

        SendToEscrow();
    }
}

void SampleEscrowClient::SendToEscrow()
{
    this->modules->btcRpc->ConnectToBitcoin(this->rpcServer);

    // set multi-sig address in GUI
    emit SetMultiSigAddress(this->transactionDeposit->targetAddr);

    this->transactionDeposit->SendToTarget();

    // set txid in GUI
    emit SetTxIdDeposit(this->transactionDeposit->txId);

    // notify server of incoming transaction
    foreach(SampleEscrowServerPtr server, this->targetPool->escrowServers)
    {
        server->OnIncomingDeposit(this->clientName, this->transactionDeposit->txId, this->transactionDeposit->amountToSend);
    }
}

bool SampleEscrowClient::CheckDepositFinished()
{
    bool finished = CheckTransactionFinished(this->transactionDeposit);

    // set confirmations in GUI
    emit SetConfirmationsDeposit(this->transactionDeposit->confirmations);
    emit SetStatusDeposit(this->transactionDeposit->status);

    return finished;
}

void SampleEscrowClient::StartWithdrawal()
{
    this->modules->btcRpc->ConnectToBitcoin(this->rpcServer);

    // create new transaction object. substract fee from the amount we expect
    this->transactionWithdrawal = SampleEscrowTransactionPtr(
                new SampleEscrowTransaction(this->transactionDeposit->amountToSend - BtcHelper::FeeMultiSig));

    // set the txId from which the funds should be released
    this->transactionWithdrawal->sourceTxId = this->transactionDeposit->txId;

    // set the address to which the funds should be sent
    this->transactionWithdrawal->targetAddr = this->modules->mtBitcoin->GetNewAddress("ReceivedFromPool");
    if(this->transactionWithdrawal->targetAddr.empty())
        return;

    // set address in GUI
    emit SetWithdrawalAddress(this->transactionWithdrawal->targetAddr);

    foreach(SampleEscrowServerPtr server, this->targetPool->escrowServers)
    {
        std::string partiallySignedTx = server->OnRequestEscrowWithdrawal(this->clientName, this->transactionWithdrawal->sourceTxId, this->transactionDeposit->targetAddr, this->transactionWithdrawal->targetAddr);

        // break when enough signatures were collected
        if(this->transactionWithdrawal->AddWithdrawalTransaction(partiallySignedTx))
            break;
    }

    // if enough signatures were collected, broadcast the transaction immediately
    // adding more signatures would do nothing but increase fees
    this->transactionWithdrawal->SendWithdrawalTransaction();

    // set txid in GUI
    emit SetTxIdWithdrawal(this->transactionWithdrawal->txId);

    foreach (SampleEscrowServerPtr server, this->targetPool->escrowServers)
    {
        // notify servers about the transaction
        server->ReleasingFunds(this->clientName, this->transactionWithdrawal->txId, this->transactionDeposit->txId, this->transactionWithdrawal->sourceTxId, this->transactionWithdrawal->targetAddr);
    }
}

bool SampleEscrowClient::CheckWithdrawalFinished()
{  
    bool finished = CheckTransactionFinished(this->transactionWithdrawal);

    // set confirmations in GUI
    emit SetConfirmationsWithdrawal(this->transactionWithdrawal->confirmations);
    emit SetStatusWithdrawal(this->transactionWithdrawal->status);

    return finished;
}

bool SampleEscrowClient::CheckTransactionFinished(SampleEscrowTransactionPtr transaction)
{
    this->modules->btcRpc->ConnectToBitcoin(this->rpcServer);

    transaction->CheckTransaction(this->minConfirms);

    if(transaction->status == SampleEscrowTransaction::Pending)
        return false;   // if transaction is pending, return false

    return true;        // if transaction is successfull or failed, return true
}








