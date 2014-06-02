#include "btctransactioninfo.hpp"
#include "ui_btctransactioninfo.h"

#include <bitcoin-api/btcobjects.hpp>
#include <bitcoin-api/btcmodules.hpp>

BtcTransactionInfo::BtcTransactionInfo(QWidget *parent) :
    QWidget(parent, Qt::Window),
    ui(new Ui::BtcTransactionInfo)
{
    ui->setupUi(this);
}

BtcTransactionInfo::BtcTransactionInfo(BtcTransactionPtr tx, BtcRawTransactionPtr rawTx, QWidget *parent)
    : QWidget(parent, Qt::Window), ui(new Ui::BtcTransactionInfo)
{
    ui->setupUi(this);

    Initialize(tx, rawTx);
}

void BtcTransactionInfo::Initialize(BtcTransactionPtr tx, BtcRawTransactionPtr rawTx)
{
    // txid
    this->ui->labelTxId->setText(QString::fromStdString(tx->TxId));

    // amount, fee
    this->ui->labelAmount->setText(QString::number(BtcHelper::SatoshisToCoins(tx->Amount)));
    this->ui->labelFee->setText(QString::number(BtcHelper::SatoshisToCoins(tx->Fee)));

    // confirmations
    this->ui->numberConfirms->display(tx->Confirmations);
    Qt::GlobalColor lcdColor = Qt::red;
    if(tx->Confirmations > 1)
        lcdColor = Qt::yellow;
    else if(tx->Confirmations > 4)
        lcdColor = Qt::green;
    this->ui->numberConfirms->setPalette(lcdColor);

    // conflicts
    for(btc::stringList::iterator i = tx->walletConflicts.begin(); i != tx->walletConflicts.end(); i++)
    {
        this->ui->comboConflicts->addItem(QString::fromStdString(*i));
    }

    // time
    QDateTime time;
    time.setTime_t(tx->Time);
    this->ui->dateTimeSent->setDateTime(time);
    time.setTime_t(tx->TimeReceived);
    this->ui->dateTimeReceived->setDateTime(time);

    // details
    this->ui->tableDetails->setRowCount(0);
    foreach(BtcTxDetailPtr detail, tx->Details)
    {
        int row = 0;
        int column = 0;
        this->ui->tableDetails->insertRow(row);
        this->ui->tableDetails->setItem(row, column++, new QTableWidgetItem(QString::fromStdString(detail->category)));
        this->ui->tableDetails->setItem(row, column++, new QTableWidgetItem(QString::fromStdString(detail->account)));
        this->ui->tableDetails->setItem(row, column++, new QTableWidgetItem(QString::fromStdString(detail->address)));
        this->ui->tableDetails->setItem(row, column++, new QTableWidgetItem(QString::number(BtcHelper::SatoshisToCoins(detail->amount))));
        this->ui->tableDetails->setItem(row, column++, new QTableWidgetItem(QString::number(BtcHelper::SatoshisToCoins(detail->fee))));
    }
}


BtcTransactionInfo::~BtcTransactionInfo()
{
    delete ui;
}
