#include "btcaddpoolserver.hpp"
#include "ui_btcaddpoolserver.h"

#include "btcconnectdlg.hpp"
#include "btcpoolmanager.hpp"

#include <bitcoin/sampleescrowserver.hpp>
#include <bitcoin/sampleescrowserverzmq.hpp>
#include <bitcoin/poolmanager.hpp>

#include <core/modules.hpp>


BtcAddPoolServer::BtcAddPoolServer(QWidget *parent) :
    QWidget(parent, Qt::Window),
    ui(new Ui::BtcAddPoolServer)
{
    ui->setupUi(this);
}

BtcAddPoolServer::~BtcAddPoolServer()
{
    delete ui;
}

void BtcAddPoolServer::on_buttonConnect_clicked()
{
    if(Modules::poolManager->selectedPool.empty())
        return;

    QString url = this->ui->editHost->text();
    int port = this->ui->editPort->text().toInt();
    EscrowPoolPtr pool = Modules::poolManager->GetPoolByName(Modules::poolManager->selectedPool);

    SampleEscrowServerZmqPtr server = SampleEscrowServerZmqPtr(new SampleEscrowServerZmq(Modules::connectionManager->rpcServer, pool, url.toStdString(), port));
    pool->AddEscrowServer(server);

    this->hide();
    this->close();
}

void BtcAddPoolServer::on_buttonCreate_clicked()
{
    if(Modules::poolManager->selectedPool.empty())
        return;

    int port = this->ui->editPort->text().toInt();
    EscrowPoolPtr pool = Modules::poolManager->GetPoolByName(Modules::poolManager->selectedPool);

    SampleEscrowServerZmqPtr server = SampleEscrowServerZmqPtr(new SampleEscrowServerZmq(Modules::connectionManager->rpcServer, pool, port));
    pool->AddEscrowServer(server);

    this->hide();
    this->close();
}

void BtcAddPoolServer::on_buttonSimulate_clicked()
{
    if(Modules::poolManager->selectedPool.empty())
        return;

    EscrowPoolPtr pool = Modules::poolManager->GetPoolByName(Modules::poolManager->selectedPool);
    SampleEscrowServerPtr server = SampleEscrowServerPtr(new SampleEscrowServer(Modules::connectionManager->rpcServer, pool));
    pool->AddEscrowServer(server);

    this->hide();
    this->close();
}
