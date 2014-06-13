#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/btcsenddlg.hpp>
#include <ui_btcsenddlg.h>

#include <gui/widgets/btctxidlist.hpp>

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

    this->client = Modules::sampleEscrowClient;

    this->txIdList = NULL;
}

BtcSendDlg::~BtcSendDlg()
{
    delete ui;
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

    this->ui->editTxid->setText(QString::fromStdString(txId));
    this->ui->editAddress->setText(QString::fromStdString(address));
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
    BtcUnspentOutputs outputs;
    if(this->ui->checkExcludeUnsignable->isChecked())
        outputs = Modules::btcModules->btcHelper->FindUnspentSignableOutputs(txIds);
    else
        outputs = Modules::btcModules->btcHelper->FindUnspentOutputs(txIds);

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

    if(txId.empty())
        return;

    this->ui->buttonSendRawTx->setEnabled(false);
    this->ui->labelMultisigSpendable->setText(txId.empty() ? "Not sent" : "Sent");
}

void BtcSendDlg::on_buttonShowUnspentTxids_clicked()
{
    if(this->txIdList == NULL)
        this->txIdList = new BtcTxIdList();

    this->txIdList->Update();
    this->txIdList->show();
}
