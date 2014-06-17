#include "btctransactioninfo.hpp"
#include "ui_btctransactioninfo.h"

#include <core/modules.hpp>
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

void BtcTransactionInfo::closeEvent(QCloseEvent *event)
{
    delete this;
}

void BtcTransactionInfo::Initialize(BtcTransactionPtr tx, BtcRawTransactionPtr rawTx)
{    
    // txid
    if(tx != NULL)
        this->ui->labelTxId->setText(QString::fromStdString(tx->TxId));
    else if (rawTx != NULL)
        this->ui->labelTxId->setText(QString::fromStdString(rawTx->txId));

    if(tx != NULL)
        SetTxInfo(tx);
    else
    {
        this->ui->tableDetails->setRowCount(0);

        this->ui->labelAmount->clear();
        this->ui->labelFee->clear();

        this->ui->numberConfirms->display(0);
        this->ui->numberConfirms->setPalette(Qt::red);

        this->ui->comboConflicts->clear();

        this->ui->dateTimeSent->clear();
        this->ui->dateTimeReceived->clear();
    }

    if(rawTx != NULL)
        SetRawTxInfo(rawTx);
    else
    {
        this->ui->tableRawVin->setRowCount(0);
        this->ui->tableRawVout->setRowCount(0);
    }
}

void BtcTransactionInfo::SetTxInfo(const BtcTransactionPtr &tx)
{
    if(tx == NULL)
        return;

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
    this->ui->comboConflicts->clear();
    if(tx->walletConflicts.size() > 0)
        this->ui->comboConflicts->addItem(QString::fromStdString(tx->TxId));
    for(btc::stringList::iterator i = tx->walletConflicts.begin(); i != tx->walletConflicts.end(); i++)
    {
        this->ui->comboConflicts->addItem(QString::fromStdString(*i));
    }
    this->ui->comboConflicts->setCurrentIndex(0);

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
        int column = 0;
        this->ui->tableDetails->insertRow(0);
        this->ui->tableDetails->setItem(0, column++, new QTableWidgetItem(QString::fromStdString(detail->category)));
        this->ui->tableDetails->setItem(0, column++, new QTableWidgetItem(QString::number(BtcHelper::SatoshisToCoins(detail->amount))));
        this->ui->tableDetails->setItem(0, column++, new QTableWidgetItem(QString::fromStdString(detail->account)));
        this->ui->tableDetails->setItem(0, column++, new QTableWidgetItem(QString::fromStdString(detail->address)));
        this->ui->tableDetails->setItem(0, column++, new QTableWidgetItem(QString::number(BtcHelper::SatoshisToCoins(detail->fee))));
    }
}

void BtcTransactionInfo::SetRawTxInfo(const BtcRawTransactionPtr &rawTx)
{
    if(rawTx == NULL)
        return;

    // inputs
    this->ui->tableRawVin->setRowCount(0);
    foreach(const BtcRawTransaction::VIN &vin, rawTx->inputs)
    {
        int column = 0;
        this->ui->tableRawVin->insertRow(0);
        this->ui->tableRawVin->setItem(0, column++, new QTableWidgetItem(QString::number(vin.vout)));
        this->ui->tableRawVin->setItem(0, column++, new QTableWidgetItem(QString::fromStdString(vin.txInID)));
        this->ui->tableRawVin->setItem(0, column++, new QTableWidgetItem(QString::fromStdString(vin.ScriptSigHex)));
        this->ui->tableRawVin->setItem(0, column++, new QTableWidgetItem(QString::number(vin.Sequence)));
    }

    // outputs
    this->ui->tableRawVout->setRowCount(0);
    foreach(const BtcRawTransaction::VOUT &vout, rawTx->outputs)
    {
        int column = 0;
        this->ui->tableRawVout->insertRow(0);
        this->ui->tableRawVout->setItem(0, column++, new QTableWidgetItem(QString::number(vout.n)));
        this->ui->tableRawVout->setItem(0, column++, new QTableWidgetItem(QString::number(BtcHelper::SatoshisToCoins(vout.value))));
        this->ui->tableRawVout->setItem(0, column++, new QTableWidgetItem(QString::fromStdString(vout.addresses.size() == 1 ? vout.addresses.front() : "multiple?!?")));
        this->ui->tableRawVout->setItem(0, column++, new QTableWidgetItem(QString::fromStdString(vout.Type)));
        this->ui->tableRawVout->setItem(0, column++, new QTableWidgetItem(QString::fromStdString(vout.scriptPubKeyHex)));
        this->ui->tableRawVout->setItem(0, column++, new QTableWidgetItem(QString::number(vout.reqSigs)));
    }
}


BtcTransactionInfo::~BtcTransactionInfo()
{
    delete ui;
}

void BtcTransactionInfo::on_comboConflicts_currentIndexChanged(const QString &txId)
{
    if(txId.isEmpty())
        return;

    if(txId == this->ui->labelTxId->text())
        return;

    BtcTransactionPtr tx = Modules::btcModules->btcJson->GetTransaction(txId.toStdString());

    BtcRawTransactionPtr rawTx;
    if(tx != NULL)
        rawTx = Modules::btcModules->btcHelper->GetDecodedRawTransaction(tx);
    else
        rawTx = Modules::btcModules->btcHelper->GetDecodedRawTransaction(txId.toStdString());

    Initialize(tx, rawTx);
}
