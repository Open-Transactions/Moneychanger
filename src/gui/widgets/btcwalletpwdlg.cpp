#include "btcwalletpwdlg.hpp"
#include "ui_btcwalletpwdlg.h"

#include <core/modules.hpp>
#include <bitcoin-api/btcmodules.hpp>
#include <bitcoin-api/btcjson.hpp>

#include <QMutex>
#include <QApplication>

BtcWalletPwDlg::BtcWalletPwDlg(QWidget *parent) :
    QWidget(parent, Qt::Window | Qt::WindowStaysOnTopHint),
    ui(new Ui::BtcWalletPwDlg)
{
    this->ui->setupUi(this);

    this->password = std::string();

    this->mutex.reset(new QMutex());
    this->btcJson.reset();
    this->waitingForInput = false;
}

BtcWalletPwDlg::~BtcWalletPwDlg()
{
    delete ui;
}

std::string BtcWalletPwDlg::WaitForPassword()
{
    this->mutex->lock();

    this->show();

    this->waitingForInput = true;
    do
    {
        btc::Sleep(33);
        qApp->processEvents();
    } while(waitingForInput);


    this->mutex->unlock();

    return this->password;

}

void BtcWalletPwDlg::on_buttonUnlock_clicked()
{
    this->password = this->ui->editPassword->text().toStdString();
    this->hide();
    this->waitingForInput = false;
}
