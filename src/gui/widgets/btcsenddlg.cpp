#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/btcsenddlg.hpp>
#include <ui_btcsenddlg.h>

#include <gui/widgets/btctxidlist.hpp>

#include <core/modules.hpp>
#include <core/handlers/focuser.h>

#include <bitcoin/poolmanager.hpp>
#include <bitcoin/sampleescrowclient.hpp>
#include <bitcoin/transactionmanager.hpp>

#include <bitcoin-api/btchelper.hpp>

#include <QStringListModel>
#include <QTimer>


BtcSendDlg::BtcSendDlg(QWidget *parent) :
    QWidget(parent, Qt::Window),
    ui(new Ui::BtcSendDlg)
{
    ui->setupUi(this);

    this->client = Modules::sampleEscrowClient;

    this->txIdList = NULL;

    this->updateTimer = _SharedPtr<QTimer>(new QTimer());
    this->updateTimer->setInterval(15000);
    this->updateTimer->start();
    connect(this->updateTimer.get(), SIGNAL(timeout()), this, SLOT(Update()));

    Update();
}

BtcSendDlg::~BtcSendDlg()
{
    delete ui;
}

void BtcSendDlg::Update()
{
    RefreshBalances();
}

void BtcSendDlg::RefreshBalances()
{
    BtcBalancesPtr balances = Modules::btcModules->btcHelper->GetBalances();
    if(balances == NULL)
        return;

    ui->scrollArea->setVisible(ui->checkBox->isChecked());
    QSize s = sizeHint(); s.setWidth(width()); resize(s);

    this->ui->labelBalanceC->setText(QString::fromStdString(btc::to_string(BtcHelper::SatoshisToCoins(balances->confirmed))));
    this->ui->labelBalanceP->setText(QString::fromStdString(btc::to_string(BtcHelper::SatoshisToCoins(balances->pending))));
    this->ui->labelBalanceWatC->setText(QString::fromStdString(btc::to_string(BtcHelper::SatoshisToCoins(balances->watchConfirmed))));
    this->ui->labelBalanceWatP->setText(QString::fromStdString(btc::to_string(BtcHelper::SatoshisToCoins(balances->watchPending))));
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
    this->ui->textOutputs->clear();
    this->ui->textVout->clear();
    this->ui->textScriptPubKeys->clear();
    this->ui->textRedeemScripts->clear();
    this->ui->labelMultisigSpendable->clear();
    this->prereqs = BtcSigningPrerequisites();
    this->ui->buttonSendRawTx->setEnabled(false);

    QStringList txIdList = this->ui->textMultisigTxids->toPlainText().split("\n");
    if(txIdList.size() <= 0)
        return;

    btc::stringList txIds;
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
}

void BtcSendDlg::on_buttonCreateRawTx_clicked()
{
    int64_t amount = BtcHelper::CoinsToSatoshis(this->ui->spinBoxAmount->value());
    int64_t fee = BtcHelper::CoinsToSatoshis(this->ui->spinBoxFee->value());

    std::string toAddress = this->ui->editAddressMultisig->text().toStdString();
    std::string changeAddress = this->ui->editChangeMultisig->text().toStdString();

    this->ui->buttonSendRawTx->setEnabled(false);
    this->ui->labelSigned->setText("Not signed");
    this->ui->textRawTx->clear();

    BtcSignedTransactionPtr tx = Modules::btcModules->btcHelper->CreateSpendTransaction(this->outputsToSpend, amount, toAddress, changeAddress, fee);
    if(tx == NULL)
        return;

    this->ui->textRawTx->setPlainText(QString::fromStdString(tx->signedTransaction));
}

void BtcSendDlg::on_buttonSignRawTx_clicked()
{
    this->ui->labelSigned->setText("Not signed");

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

    Focuser f(this->txIdList);
    f.show();
    f.focus();
}

void BtcSendDlg::on_checkBox_toggled(bool checked)
{
    ui->scrollArea->setVisible(checked);

    QSize s = sizeHint(); s.setWidth(width()); resize(s);

//    if (checked)
//    {
//    }
//    else
//    {
//    }
}
