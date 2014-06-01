#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/btcsenddlg.hpp>
#include <ui_btcsenddlg.h>

#include <core/modules.hpp>

#include <bitcoin/poolmanager.hpp>
#include <bitcoin/sampleescrowclient.hpp>
#include <bitcoin/transactionmanager.hpp>

#include <QStringListModel>


BtcSendDlg::BtcSendDlg(QWidget *parent) :
    QWidget(parent, Qt::Window),
    ui(new Ui::BtcSendDlg)
{
    ui->setupUi(this);

    // update the list of escrow pools to which we can send btc
    this->OnPoolListUpdate();

    this->client = SampleEscrowClientPtr(new SampleEscrowClient());
}

BtcSendDlg::~BtcSendDlg()
{
    delete ui;
}

void BtcSendDlg::OnPoolListUpdate()
{
    // TODO: inherit from QAbstractListModel and link poolList directly with poolManager->escrowPools

    QStringList poolNames = QStringList();
    foreach(EscrowPoolPtr pool, Modules::poolManager->escrowPools)
    {
        poolNames.append(pool->poolName);
    }

    this->ui->listPools->clear();
    this->ui->listPools->addItems(poolNames);
}

void BtcSendDlg::on_sendButton_clicked()
{
    int64_t amountToSend = BtcHelper::CoinsToSatoshis(this->ui->amountSpinBox->value());
    if(amountToSend <= 0)
        return;

    std::string address = this->ui->editAddress->text().toStdString();
    std::string txId;

    if(!address.empty())
    {
        // if address is provided, send to address
        txId = Modules::btcModules->mtBitcoin->SendToAddress(address, amountToSend);
        if(txId.empty())
            this->ui->editTxid->setText(QString::fromStdString("Failed to send to address " + address));
    }
    else
    {
        // otherwise send to pool
        QString selectedPool = this->ui->listPools->currentItem()->text();
        if(selectedPool.isEmpty())
            return;

        EscrowPoolPtr pool = Modules::poolManager->GetPoolByName(selectedPool);
        if(pool == NULL)
            return;

        this->client->StartDeposit(amountToSend, pool);
        if(this->client->transactionDeposit->status == SampleEscrowTransaction::Failed)
        {
            this->ui->editTxid->setText("Failed to deposit to pool " + pool->poolName);
            return;
        }

        txId = this->client->transactionDeposit->txId;
        address = this->client->transactionDeposit->targetAddr;

        Modules::transactionManager->AddTransaction(this->client->clientName, this->client->transactionDeposit);
    }

    this->ui->editTxid->setText(QString::fromStdString(txId));
    this->ui->editAddress->setText(QString::fromStdString(address));
}

void BtcSendDlg::on_buttonRefresh_clicked()
{
    OnPoolListUpdate();
}

void BtcSendDlg::on_editAddress_textEdited(const QString &arg1)
{
    Modules::poolManager->selectedPool = QString();
}
