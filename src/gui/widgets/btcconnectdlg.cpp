#include "btcconnectdlg.hpp"
#include "ui_btcconnectdlg.h"

#include <core/modules.hpp>
#include <bitcoin-api/btcmodules.hpp>

BtcConnectDlg::BtcConnectDlg(QWidget *parent) :
    QWidget(parent, Qt::Window),
    ui(new Ui::BtcConnectDlg)
{
    this->ui->setupUi(this);
}

BtcConnectDlg::~BtcConnectDlg()
{
    delete this->ui;
}

void BtcConnectDlg::on_buttonConnect_clicked()
{
    QString url = "http://" + this->ui->editHost->text();
    int port = this->ui->editPort->text().toInt();
    QString username = this->ui->editUser->text();
    QString password = this->ui->editPass->text();
    if(Modules::btcModules->btcRpc->ConnectToBitcoin(username.toStdString(), password.toStdString(), url.toStdString(), port))
        this->ui->labelStatus->setText("Connected");
    else
        this->ui->labelStatus->setText("Failed to connect");
}

void BtcConnectDlg::on_buttonDisconnect_clicked()
{
    Modules::btcModules->btcRpc->ConnectToBitcoin(BitcoinServerPtr());
    this->ui->labelStatus->setText("Disconnected");
}
