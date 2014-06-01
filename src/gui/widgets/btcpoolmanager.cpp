#include "btcpoolmanager.hpp"
#include "ui_btcpoolmanager.h"

#include <core/modules.hpp>

#include <bitcoin/poolmanager.hpp>
#include <bitcoin/sampleescrowserver.hpp>


BtcPoolManager::BtcPoolManager(QWidget *parent) :
    QWidget(parent, Qt::Window),
    ui(new Ui::BtcPoolManager)
{
    this->ui->setupUi(this);
}

BtcPoolManager::~BtcPoolManager()
{
    delete ui;
}

void BtcPoolManager::SyncPoolList()
{
    QStringList poolNames = QStringList();
    foreach(EscrowPoolPtr pool, Modules::poolManager->escrowPools)
    {
        poolNames.push_back(pool->poolName);
    }

    this->ui->listPools->clear();
    this->ui->listPools->addItems(poolNames);

    if(!Modules::poolManager->selectedPool.isEmpty() && Modules::poolManager->selectedPool != this->ui->labelPoolname->text())
    {
        QStringList serverNames;
        EscrowPoolPtr pool = Modules::poolManager->GetPoolByName(Modules::poolManager->selectedPool);

        foreach(SampleEscrowServerPtr escrowServer, pool->escrowServers)
        {
            serverNames.push_back(QString::fromStdString(escrowServer->serverName));
        }

        this->ui->listServers->clear();
        this->ui->listServers->addItems(serverNames);
        this->ui->labelPoolname->setText(Modules::poolManager->selectedPool);
    }
}

void BtcPoolManager::on_buttonAddSimPool_clicked()
{
    // simulate a new pool
    EscrowPoolPtr escrowPool = EscrowPoolPtr(new EscrowPool());

    // give pool a name
    escrowPool->poolName = "pool #" + QString::number(Modules::poolManager->escrowPools.size());

    // add pool to global pool list
    Modules::poolManager->AddPool(escrowPool);

    // simulate servers in pool, each using its own instance of bitcoind/bitcoin-qt
    BitcoinServerPtr rpcServer;
    for(int i = 1; i < 4; i++)
    {
        // admin2..4, rpc port 19011, 19021, 19031
        rpcServer = BitcoinServerPtr(new BitcoinServer(QString("admin"+QString::number(i+1)).toStdString(), "123", "http://127.0.0.1", 19001 + i * 10));

        SampleEscrowServerPtr server = SampleEscrowServerPtr(new SampleEscrowServer(rpcServer));
        server->serverPool = escrowPool;
        escrowPool->AddEscrowServer(server);
    }

    SyncPoolList();
}

void BtcPoolManager::on_buttonRefreshPools_clicked()
{
    SyncPoolList();
}

void BtcPoolManager::on_listPools_clicked(const QModelIndex &index)
{
    Modules::poolManager->selectedPool = this->ui->listPools->currentItem()->text();
    SyncPoolList();
}
