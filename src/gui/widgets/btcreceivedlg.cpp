#include "btcreceivedlg.hpp"
#include "ui_btcreceivedlg.h"

#include <core/modules.hpp>
#include <bitcoin-api/btcmodules.hpp>

BtcReceiveDlg::BtcReceiveDlg(QWidget *parent) :
    QWidget(parent, Qt::Window),
    ui(new Ui::BtcReceiveDlg)
{
    ui->setupUi(this);

    UpdateAddressList();
}

BtcReceiveDlg::~BtcReceiveDlg()
{
    delete ui;
}

void BtcReceiveDlg::UpdateAddressList()
{
    bool showEmpty = this->ui->checkShowEmpty->isChecked();
    bool showWatchonly = this->ui->checkShowWatchonly->isChecked();
    BtcAddressBalances balances = Modules::btcModules->btcJson->ListReceivedByAddress(0, showEmpty, showWatchonly);

    this->ui->tableAddresses->setRowCount(0);

    ui->groupBox->setVisible(ui->checkBox->isChecked());
    QSize s = sizeHint(); s.setWidth(width()); resize(s);

//    resize(sizeHint());

    if(balances.size() <= 0)
        return;

    foreach(BtcAddressBalancePtr balance, balances)
    {
        int column = 0;
        this->ui->tableAddresses->insertRow(0);

        this->ui->tableAddresses->setItem(0, column++, new QTableWidgetItem(QString::fromStdString(balance->account)));
        this->ui->tableAddresses->setItem(0, column++, new QTableWidgetItem(QString::number(BtcHelper::SatoshisToCoins(balance->amount))));
        this->ui->tableAddresses->setItem(0, column++, new QTableWidgetItem(QString::fromStdString(balance->address)));
        this->ui->tableAddresses->setItem(0, column++, new QTableWidgetItem(balance->involvesWatchonly ? "yes" : ""));
    }
    this->ui->tableAddresses->sortItems(0, Qt::AscendingOrder);
}

void BtcReceiveDlg::on_buttonNewAddress_clicked()
{
    QString account = this->ui->editAccount->text();
    std::string address = Modules::btcModules->mtBitcoin->GetNewAddress(account.toStdString());
    this->ui->editNewAddress->setText(QString::fromStdString(address));

    if(address.empty())
        return;

    BtcAddressInfoPtr addrInfo = Modules::btcModules->btcJson->ValidateAddress(address);
    if(addrInfo == NULL)
        return;

    this->ui->editPubKey->setText(QString::fromStdString(addrInfo->pubkey));

    UpdateAddressList();
}

void BtcReceiveDlg::on_checkShowEmpty_stateChanged(int arg1)
{
    UpdateAddressList();
}

void BtcReceiveDlg::on_checkShowWatchonly_stateChanged(int arg1)
{
    UpdateAddressList();
}

void BtcReceiveDlg::on_buttonImport_clicked()
{
    QString address = this->ui->editImportAddress->text();
    QString account = this->ui->editImportAccount->text();
    bool rescan = this->ui->checkImportRescan->isChecked();

    Modules::btcModules->mtBitcoin->ImportAddress(address.toStdString(), account.toStdString(), rescan);

    UpdateAddressList();
}

void BtcReceiveDlg::on_buttonCreateMultisig_clicked()
{
    int32_t sigs = this->ui->spinSigsRequired->value();
    if(sigs <= 0)
        return;

    QString pubKeysText = ui->textPublicKeys->toPlainText();
    QStringList lines = pubKeysText.split("\n");
    if(lines.count() == 0)
        return;
    btc::stringList pubKeys;
    foreach(QString line, lines)
    {
        pubKeys.push_back(line.toStdString());
    }

    QString account = this->ui->editMultisAccount->text();

    std::string multiSigAddr = Modules::btcModules->mtBitcoin->GetMultiSigAddress(sigs, pubKeys, true, account.toStdString());
    Modules::btcModules->mtBitcoin->ImportAddress(multiSigAddr, account.toStdString(), false);

    this->ui->editNewAddress->setText(QString::fromStdString(multiSigAddr));

    UpdateAddressList();
}

void BtcReceiveDlg::on_tableAddresses_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    this->ui->editPubKey->clear();

    QTableWidgetItem* item = this->ui->tableAddresses->item(currentRow, currentColumn);
    if(item == NULL)
        return;

    std::string address = item->text().toStdString();
    if(address.empty())
        return;

    BtcAddressInfoPtr addrInfo = Modules::btcModules->btcJson->ValidateAddress(address);
    if(addrInfo == NULL)
        return;

    this->ui->editNewAddress->setText(QString::fromStdString(address));
    this->ui->editPubKey->setText(QString::fromStdString(addrInfo->pubkey));

}

void BtcReceiveDlg::on_checkBox_toggled(bool checked)
{
    ui->groupBox->setVisible(checked);
//    resize(sizeHint());

    QSize s = sizeHint(); s.setWidth(width());

    if (!checked)
       s.setHeight(380);

    resize(s);
}


