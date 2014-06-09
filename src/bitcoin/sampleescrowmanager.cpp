#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin/sampleescrowmanager.hpp>

#include <gui/widgets/btcguitest.hpp>

#include <bitcoin/poolmanager.hpp>
#include <bitcoin/transactionmanager.hpp>

#include <bitcoin/sampleescrowserver.hpp>

#include <core/modules.hpp>
#include <core/utils.hpp>


SampleEscrowManager::SampleEscrowManager()
{
    this->escrowPool = EscrowPoolPtr();
    this->client = SampleEscrowClientPtr();
}

void SampleEscrowManager::OnSimulateEscrowServers()
{
    // simulate a new pool
    this->escrowPool = EscrowPoolPtr(new EscrowPool());

    // give pool a name
    this->escrowPool->poolName = "pool #" + btc::to_string(Modules::poolManager->escrowPools.size());

    // add pool to global pool list
    Modules::poolManager->AddPool(this->escrowPool);

    // simulate servers in pool, each using its own instance of bitcoind/bitcoin-qt
    BitcoinServerPtr rpcServer;
    for(int i = 1; i < 4; i++)
    {
        // admin2..4, rpc port 19011, 19021, 19031
        rpcServer = BitcoinServerPtr(new BitcoinServer(QString("admin"+QString::number(i+1)).toStdString(), "123", "http://127.0.0.1", 19001 + i * 10));

        SampleEscrowServerPtr server = SampleEscrowServerPtr(new SampleEscrowServer(rpcServer));
        server->serverPool = this->escrowPool;
        this->escrowPool->AddEscrowServer(server);
    }
}

void SampleEscrowManager::OnInitializeEscrow(BtcGuiTest* btcGuiTest)
{
    // create a new client
    if(client == NULL)
        this->client = SampleEscrowClientPtr(new SampleEscrowClient(BitcoinServerPtr(new BitcoinServer("admin1", "123", "http://127.0.0.1", 19001))));

    // connect events to update GUI
    QObject::connect(client.get(), SIGNAL(SetMultiSigAddress(const std::string&)), btcGuiTest, SLOT(SetMultiSigAddress(const std::string&)));
    QObject::connect(client.get(), SIGNAL(SetTxIdDeposit(const std::string&)), btcGuiTest, SLOT(SetTxIdDeposit(const std::string&)));
    QObject::connect(client.get(), SIGNAL(SetConfirmationsDeposit(int)), btcGuiTest, SLOT(OnSetConfirmationsDeposit(int)));
    QObject::connect(client.get(), SIGNAL(SetStatusDeposit(SampleEscrowTransaction::SUCCESS)), btcGuiTest, SLOT(SetStatusDeposit(SampleEscrowTransaction::SUCCESS)));
    QObject::connect(client.get(), SIGNAL(SetWithdrawalAddress(const std::string&)), btcGuiTest, SLOT(SetWithdrawalAddress(const std::string&)));
    QObject::connect(client.get(), SIGNAL(SetTxIdWithdrawal(const std::string&)), btcGuiTest, SLOT(SetTxIdWithdrawal(const std::string&)));
    QObject::connect(client.get(), SIGNAL(SetConfirmationsWithdrawal(int)), btcGuiTest, SLOT(OnSetConfirmationsWithdrawal(int)));
    QObject::connect(client.get(), SIGNAL(SetStatusWithdrawal(SampleEscrowTransaction::SUCCESS)), btcGuiTest, SLOT(SetStatusWithdrawal(SampleEscrowTransaction::SUCCESS)));

    double amountToSend = btcGuiTest->GetAmountToSend();
    int64_t amountSatoshis = BtcHelper::CoinsToSatoshis(amountToSend);

    // instruct client to start sending bitcoin to pool
    client->StartDeposit(amountSatoshis, this->escrowPool); 
}

void SampleEscrowManager::OnRequestWithdrawal(BtcGuiTest *btcGuiTest)
{
    std::string receiveAddress = Modules::btcModules->mtBitcoin->GetNewAddress("from pool");
    client->StartWithdrawal(BtcHelper::CoinsToSatoshis(btcGuiTest->GetAmountToSend()) - BtcHelper::FeeMultiSig, receiveAddress, this->escrowPool);
}
