#include "btcpoolmanager.hpp"
#include "ui_btcpoolmanager.h"

#include "btcaddpoolserver.hpp"

#include <core/modules.hpp>

#include <bitcoin/poolmanager.hpp>
#include <bitcoin/sampleescrowserver.hpp>
#include <bitcoin/sampleescrowclient.hpp>

#include <QTimer>


BtcPoolManager::BtcPoolManager(QWidget *parent) :
    QWidget(parent, Qt::Window),
    ui(new Ui::BtcPoolManager)
{
    this->ui->setupUi(this);
    this->ui->labelClientName->setText(QString::fromStdString(Modules::sampleEscrowClient->clientName));

    this->lastBalance = 0;
    this->lastDepositAddress = std::string();

    this->updateTimer = QTimerPtr(new QTimer(parent));
    updateTimer->setInterval(500);
    updateTimer->start();
    connect(updateTimer.get(), SIGNAL(timeout()), this, SLOT(Update()));
}

BtcPoolManager::~BtcPoolManager()
{
    delete ui;
}

int balanceCounter = 10;
void BtcPoolManager::Update()
{
    std::string poolName = Modules::poolManager->selectedPool;
    SampleEscrowClientPtr client = Modules::sampleEscrowClient;

    if(balanceCounter == 10)
        SyncPoolList(true);

    if(poolName.empty())
        return;

    if(client->poolBalanceMap[poolName] != this->lastBalance)
    {
        this->lastBalance = client->poolBalanceMap[poolName];
        this->ui->labelBalance->setText(QString::number(BtcHelper::SatoshisToCoins(this->lastBalance)));
    }
    if(client->poolAddressMap[poolName] != this->lastDepositAddress)
    {
        this->lastDepositAddress = client->poolAddressMap[poolName];
        this->ui->editDepositAddr->setText(QString::fromStdString(this->lastDepositAddress));
    }

    if(balanceCounter-- == 0)
    {
        balanceCounter = 10;
        client->CheckPoolBalance(Modules::poolManager->GetPoolByName(poolName));
    }
}

void BtcPoolManager::SyncPoolList(bool refreshAll)
{
    btc::stringList poolNames = btc::stringList();
    foreach(EscrowPoolPtr pool, Modules::poolManager->escrowPools)
    {
        poolNames.push_back(pool->poolName);
    }

    this->ui->listPools->clear();
    foreach(std::string poolName, poolNames)
    {
        this->ui->listPools->addItem(QString::fromStdString(poolName));
    }

    if(!Modules::poolManager->selectedPool.empty()) // && Modules::poolManager->selectedPool != this->ui->labelPoolname->text().toStdString())
    {
        QStringList serverNames;
        EscrowPoolPtr pool = Modules::poolManager->GetPoolByName(Modules::poolManager->selectedPool);

        foreach(SampleEscrowServerPtr escrowServer, pool->escrowServers)
        {
            serverNames.push_back(QString::fromStdString(escrowServer->serverName));
        }

        this->ui->listServers->clear();
        this->ui->listServers->addItems(serverNames);
        this->ui->labelPoolname->setText(QString::fromStdString(Modules::poolManager->selectedPool));
    }   
}

void BtcPoolManager::on_buttonAddSimPool_clicked()
{  
    // simulate a new pool
    int32_t sigsRequired = this->ui->spinBoxSignatures->value();
    EscrowPoolPtr escrowPool = EscrowPoolPtr(new EscrowPool(sigsRequired));

    // give pool a name
    escrowPool->poolName = "pool #" + btc::to_string(Modules::poolManager->escrowPools.size());

    // add pool to global pool list
    Modules::poolManager->AddPool(escrowPool);

    SyncPoolList();
}

void BtcPoolManager::on_buttonRefreshPools_clicked()
{
    SyncPoolList();

    foreach (EscrowPoolPtr pool, Modules::poolManager->escrowPools)
    {
        Modules::sampleEscrowClient->CheckPoolBalance(pool);
        Modules::sampleEscrowClient->CheckPoolTransactions(pool);
    }
}

void BtcPoolManager::on_listPools_clicked(const QModelIndex &index)
{
    Modules::poolManager->selectedPool = this->ui->listPools->currentItem()->text().toStdString();

    SyncPoolList();
}

void BtcPoolManager::on_buttonRequestDeposit_clicked()
{
    // send to pool
    if(Modules::poolManager->selectedPool.empty())
       return;

    EscrowPoolPtr pool = Modules::poolManager->GetPoolByName(Modules::poolManager->selectedPool);
    if(pool == NULL)
        return;

    Modules::sampleEscrowClient->StartDeposit(BtcHelper::FeeMultiSig*2, pool);     // dummy value

    this->ui->editDepositAddr->setText(QString::fromStdString("Waiting for deposit address to pool " + pool->poolName));
    return;
}

void BtcPoolManager::on_buttonRequestPayout_clicked()
{
    int64_t amount = BtcHelper::CoinsToSatoshis(this->ui->spinAmount->value());
    std::string toAddress = this->ui->editToAddress->text().toStdString();

    if(Modules::poolManager->selectedPool.empty() || amount <= 0 || toAddress.empty())
        return;

    EscrowPoolPtr pool = Modules::poolManager->GetPoolByName(Modules::poolManager->selectedPool);
    if(pool == NULL)
        return;

    Modules::sampleEscrowClient->StartWithdrawal(amount, toAddress, pool);
}

void BtcPoolManager::on_buttonAddServer_clicked()
{
   BtcAddPoolServer* addServerDlg = new BtcAddPoolServer();
    addServerDlg->show();
}
