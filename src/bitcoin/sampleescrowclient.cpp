#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin/sampleescrowclient.hpp>
#include <bitcoin/sampleescrowserver.hpp>

#include <core/modules.hpp>

#include <QTimer>


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
    this->minSignatures = 0;    // will be set later by server pool

    this->minConfirms = BtcHelper::WaitForConfirms;      // wait for one confirmation by default

    this->targetPool = EscrowPoolPtr();
    this->pubKeyList = btc::stringList();

    this->transactionDeposit = SampleEscrowTransactionPtr();
    this->transactionWithdrawal = SampleEscrowTransactionPtr();

    this->modules = BtcModulesPtr(new BtcModules());

    // generate random client name
    this->clientName = "                   ";
    gen_random((char*)this->clientName.c_str(), this->clientName.size());

    this->updateTimer = new QTimer(parent);
    updateTimer->setInterval(5000);
    updateTimer->start();
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(Update()));
}

SampleEscrowClient::SampleEscrowClient(BitcoinServerPtr rpcServer, QObject *parent)
    : SampleEscrowClient(parent)
{
    this->rpcServer = rpcServer;
}

SampleEscrowClient::~SampleEscrowClient()
{
    this->rpcServer.reset();
    this->targetPool.reset();
    this->pubKeyList.clear();
    this->transactionDeposit.reset();
    this->transactionWithdrawal.reset();

    delete this->updateTimer;
}

void SampleEscrowClient::Update()
{
    if(this->transactionDeposit != NULL && this->transactionDeposit->status != SampleEscrowTransaction::Successfull)
    {
        if(this->transactionDeposit->targetAddr.empty())
        {
            AskForDepositAddress();
        }
        else if(this->transactionDeposit->txId.empty())
        {
            SendToEscrow();
        }
        else
        {
            CheckDepositFinished();
        }
    }
    if(this->transactionWithdrawal != NULL && transactionWithdrawal->status != SampleEscrowTransaction::Successfull)
    {
        CheckWithdrawalFinished();
    }
}

void SampleEscrowClient::StartDeposit(int64_t amountToSend, EscrowPoolPtr targetPool)
{
    if(this->rpcServer != NULL)
        this->modules->btcRpc->ConnectToBitcoin(this->rpcServer);

    this->targetPool = targetPool;

    this->transactionDeposit = SampleEscrowTransactionPtr(new SampleEscrowTransaction(amountToSend, this->modules));

    // connect to each server
    foreach(SampleEscrowServerPtr server, this->targetPool->escrowServers)
    {
        server->ClientConnected(this);
    }

    // tell each server we'd like to deposit something
    foreach(SampleEscrowServerPtr server, this->targetPool->escrowServers)
    {
        bool accepted = server->RequestEscrowDeposit(this->clientName, amountToSend);
        if(!accepted)
            return;
    }
}

void SampleEscrowClient::AskForDepositAddress()
{
    // ask the servers for an address to send money to
    std::string depositAddress = std::string();
    foreach(SampleEscrowServerPtr server, this->targetPool->escrowServers)
    {
        std::string serverMultisig = server->RequestDepositAddress(this->clientName);

        if(depositAddress.empty())
            depositAddress = serverMultisig;
        else if(depositAddress != serverMultisig)
            return;     // error
    }

    this->transactionDeposit->targetAddr = depositAddress;

    // set multi-sig address in GUI
    emit SetMultiSigAddress(this->transactionDeposit->targetAddr);
}

void SampleEscrowClient::SendToEscrow()
{
    this->transactionDeposit->SendToTarget();

    // set txid in GUI
    emit SetTxIdDeposit(this->transactionDeposit->txId);
}

bool SampleEscrowClient::CheckDepositFinished()
{
    bool finished = CheckTransactionFinished(this->transactionDeposit);

    // set confirmations in GUI
    emit SetConfirmationsDeposit(this->transactionDeposit->confirmations);
    emit SetStatusDeposit(this->transactionDeposit->status);

    return finished;
}

void SampleEscrowClient::StartWithdrawal(const int64_t &amountToWithdraw)
{
    if(this->rpcServer != NULL)
        this->modules->btcRpc->ConnectToBitcoin(this->rpcServer);

    // create new transaction object. substract fee from the amount we expect
    this->transactionWithdrawal = SampleEscrowTransactionPtr(
                new SampleEscrowTransaction(amountToWithdraw, this->modules));

    // set the address to which the funds should be sent
    this->transactionWithdrawal->targetAddr = this->modules->mtBitcoin->GetNewAddress("ReceivedFromPool");
    if(this->transactionWithdrawal->targetAddr.empty())
        return;

    // set address in GUI
    emit SetWithdrawalAddress(this->transactionWithdrawal->targetAddr);

    foreach(SampleEscrowServerPtr server, this->targetPool->escrowServers)
    {
        bool accepted = server->RequestEscrowWithdrawal(this->clientName, this->transactionWithdrawal->amountToSend, this->transactionWithdrawal->targetAddr);
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
    transaction->CheckTransaction(this->minConfirms);

    if(transaction->status == SampleEscrowTransaction::Pending)
        return false;   // if transaction is pending, return false

    return true;        // if transaction is successfull or failed, return true
}








