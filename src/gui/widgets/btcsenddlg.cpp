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

void BtcSendDlg::on_buttonSend_clicked()
{
    int64_t amountToSend = BtcHelper::CoinsToSatoshis(this->ui->spinBoxAmount->value());
    if(amountToSend <= 0)
        return;

    std::string address = this->ui->editAddress->text().toStdString();
    std::string txId;

    if(!address.empty())
    {
        // if address is provided, send to address
        txId = Modules::btcModules->mtBitcoin->SendToAddress(address, amountToSend);
        if(txId.empty())
        {
            this->ui->editTxid->setText(QString::fromStdString("Failed to send to address " + address));
            return;
        }
    }
    else
    {
        // otherwise send to pool
        if(this->ui->listPools->currentItem() == NULL)
            return;

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

void BtcSendDlg::on_buttonFindOutputs_clicked()
{
    QStringList txIdList = this->ui->textMultisigTxids->toPlainText().split("\n");
    if(txIdList.size() <= 0)
        return;

    btc::stringList txIds = btc::stringList();
    foreach(QString txId, txIdList)
    {
           txIds.push_back(txId.toStdString());
    }

    // find outputs
    BtcUnspentOutputs outputs = Modules::btcModules->btcHelper->FindSignableOutputs(txIds);
    int64_t spendableAmount = 0;
    for(BtcUnspentOutputs::const_iterator output = outputs.begin(); output != outputs.end(); output++)
    {
        // add output txid and vout to text edit
        this->ui->textOutputs->appendPlainText(QString::fromStdString((*output)->txId));
        this->ui->textVout->appendPlainText(QString::number((*output)->vout));
        spendableAmount += (*output)->amount;
    }
    // set spendable amount
    this->ui->labelMultisigSpendable->setText(QString::number(BtcHelper::SatoshisToCoins(spendableAmount)));

    // look up redeemScript
    BtcSigningPrerequisites prereqs = Modules::btcModules->btcHelper->GetSigningPrerequisites(outputs);
    for(BtcSigningPrerequisites::const_iterator prereq = prereqs.begin(); prereq != prereqs.end(); prereq++)
    {
        this->ui->textScriptPubKeys->appendPlainText(QString::fromStdString((*(*prereq))["scriptPubKey"].asString()));
        this->ui->textRedeemScripts->appendPlainText(QString::fromStdString((*(*prereq))["redeemScript"].asString()));
    }

    this->outputsToSpend = outputs;
    this->prereqs = prereqs;
    this->ui->buttonSendRawTx->setEnabled(false);
}

void BtcSendDlg::on_buttonCreateRawTx_clicked()
{
    int64_t amount = BtcHelper::CoinsToSatoshis(this->ui->spinBoxAmount->value());
    int64_t fee = BtcHelper::CoinsToSatoshis(this->ui->spinBoxFee->value());

    std::string toAddress = this->ui->editAddressMultisig->text().toStdString();
    std::string changeAddress = this->ui->editChangeMultisig->text().toStdString();

    BtcSignedTransactionPtr tx = Modules::btcModules->btcHelper->CreateSpendTransaction(this->outputsToSpend, amount, toAddress, changeAddress, fee);
    if(tx == NULL)
        return;

    this->ui->textRawTx->setPlainText(QString::fromStdString(tx->signedTransaction));
    this->ui->buttonSendRawTx->setEnabled(false);
    this->ui->labelMultisigSpendable->setText("Not signed");
}

void BtcSendDlg::on_buttonSignRawTx_clicked()
{
    BtcSignedTransactionPtr tx = BtcSignedTransactionPtr(new BtcSignedTransaction(Json::Value(Json::objectValue)));
    tx = Modules::btcModules->btcJson->SignRawTransaction(this->ui->textRawTx->toPlainText().toStdString(), this->prereqs);
    if(tx == NULL)
        return;

    this->ui->textRawTxSigned->setPlainText(QString::fromStdString(tx->signedTransaction));

    this->ui->buttonSendRawTx->setEnabled(tx->complete);
    this->ui->labelMultisigSpendable->setText(tx->complete ? "Signed" : "Not signed");
}

void BtcSendDlg::on_buttonSendRawTx_clicked()
{
    std::string txId = Modules::btcModules->btcJson->SendRawTransaction(this->ui->textRawTxSigned->toPlainText().toStdString());
    this->ui->editTxid->setText(QString::fromStdString(txId));

    this->ui->buttonSendRawTx->setEnabled(false);
    this->ui->labelMultisigSpendable->setText(txId.empty() ? "Not sent" : "Sent");
}
