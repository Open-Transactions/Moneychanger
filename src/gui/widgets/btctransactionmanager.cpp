#include "btctransactionmanager.hpp"
#include "ui_btctransactionmanager.h"
#include "btctransactioninfo.hpp"

#include <core/modules.hpp>
#include <bitcoin-api/btcmodules.hpp>

BtcTransactionInfo* infoWindow = NULL;

BtcTransactionManager::BtcTransactionManager(QWidget *parent) :
    QWidget(parent, Qt::Window),
    ui(new Ui::BtcTransactionManager)
{
    this->ui->setupUi(this);
}

BtcTransactionManager::~BtcTransactionManager()
{
    delete ui;
}

void BtcTransactionManager::on_buttonRefresh_clicked()
{
    RefreshBitcoinTransactions();
    RefreshPoolTransactions();
}

void BtcTransactionManager::RefreshBitcoinTransactions()
{
    // clear rows
    this->ui->tableTxBtc->setRowCount(0);

    // how many transactions to fetch
    int32_t txCount = this->ui->editTxCount->text().toInt();

    // get transactions
    BtcTransactions transactions = Modules::btcModules->btcJson->ListTransactions("*", txCount);
    foreach(BtcTransactionPtr tx, transactions)
    {
        int row = 0;    // insert at top
        int column = 0;
        this->ui->tableTxBtc->insertRow(row);
        QTableWidgetItem* item;
        if(tx->Details.size() <= 0)      // listtransactions returns one item in Details[]
        {
            item = new QTableWidgetItem("error");
            this->ui->tableTxBtc->setItem(row, column++, item);
            continue;
        }

        // type
        item = new QTableWidgetItem(QString::fromStdString(tx->Details[0]->category));
        this->ui->tableTxBtc->setItem(row, column++, item);

        // confirmations
        item = new QTableWidgetItem(QString::number(tx->Confirmations));
        this->ui->tableTxBtc->setItem(row, column++, item);

        // destination
        item = new QTableWidgetItem(QString::fromStdString(tx->Details[0]->account));
        this->ui->tableTxBtc->setItem(row, column++, item);

        // amount
        item = new QTableWidgetItem(QString::number(BtcHelper::SatoshisToCoins(tx->Amount)));
        this->ui->tableTxBtc->setItem(row, column++, item);

        // address
        item = new QTableWidgetItem(QString::fromStdString(tx->Details[0]->address));
        this->ui->tableTxBtc->setItem(row, column++, item);

        // txid
        item = new QTableWidgetItem(QString::fromStdString(tx->TxId));
        this->ui->tableTxBtc->setItem(row, column++, item);
    }
}

void BtcTransactionManager::RefreshPoolTransactions()
{

}

void BtcTransactionManager::on_buttonSearchTx_clicked()
{
    QString txId = this->ui->editSearchTx->text();
    if(txId.isEmpty())
        return;

    BtcTransactionPtr txInfo = Modules::btcModules->mtBitcoin->GetTransaction(txId.toStdString());
    BtcRawTransactionPtr txRawInfo = Modules::btcModules->btcJson->DecodeRawTransaction(txInfo->Hex);

    if(infoWindow == NULL)
        infoWindow = new BtcTransactionInfo();
    infoWindow->show();
    infoWindow->Initialize(txInfo, txRawInfo);
}

void BtcTransactionManager::on_tableTxBtc_cellChanged(int row, int column)
{
    // get txid from 6th column:
    QString txId = this->ui->tableTxBtc->itemAt(row, 5)->text();
    this->ui->editSearchTx->setText(txId);
}
