#ifndef STABLE_H
#include <core/stable.hpp>
#endif

#include "btcsenddlg.h"
#include "ui_btcsenddlg.h"
#include <QStringListModel>
#include "modules.h"

BtcSendDlg::BtcSendDlg(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BtcSendDlg)
{
    ui->setupUi(this);

    // update the list of escrow pools to which we can send btc
    this->OnPoolListUpdate();
}

BtcSendDlg::~BtcSendDlg()
{
    delete ui;
}

void BtcSendDlg::OnPoolListUpdate()
{
    // TODO: inherit from QAbstractListModel and link poolList directly with poolManager->escrowPools

    QStringList stringList = QStringList();
    foreach(EscrowPoolPtr pool, Modules::poolManager->escrowPools)
    {
        stringList.append(pool->poolName);
    }

    QStringListModel* stringListModel = new QStringListModel(stringList, NULL);

    this->ui->poolList->setModel(stringListModel);
}

void BtcSendDlg::on_sendButton_clicked()
{
    /*QString selectedPool;
    QModelIndexList selectedPools = this->ui->poolList->selectedIndexes();
    if(selectedPools.size() == 0)
        return;

    selectedPool = selectedPools.first().data().toString();

    foreach(EscrowPoolRef pool, Modules::poolManager->escrowPools)
    {

    }*/
}


