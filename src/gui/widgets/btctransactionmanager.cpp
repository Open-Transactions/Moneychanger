#include "btctransactionmanager.hpp"
#include "ui_btctransactionmanager.h"

#include <core/modules.hpp>
#include <bitcoin-api/btcmodules.hpp>

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
        int row = 0;
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

        row++;
    }
}

void BtcTransactionManager::RefreshPoolTransactions()
{

}
