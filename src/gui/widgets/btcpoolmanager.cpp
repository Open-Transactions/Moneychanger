#include "btcpoolmanager.hpp"
#include "ui_btcpoolmanager.h"

BtcPoolManager::BtcPoolManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BtcPoolManager)
{
    ui->setupUi(this);
}

BtcPoolManager::~BtcPoolManager()
{
    delete ui;
}
