#include "btctxidlist.hpp"
#include "ui_btctxidlist.h"

#include <core/modules.hpp>
#include <bitcoin-api/btcmodules.hpp>

BtcTxIdList::BtcTxIdList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BtcTxIdList)
{
    ui->setupUi(this);
}

BtcTxIdList::~BtcTxIdList()
{
    delete ui;
}

void BtcTxIdList::Update(BtcUnspentOutputs outputs)
{
    if(outputs.empty())
        outputs = Modules::btcModules->mtBitcoin->ListUnspentOutputs();

    this->ui->editTxIds->clear();

    foreach (BtcUnspentOutputPtr output, outputs)
    {
        this->ui->editTxIds->appendPlainText(QString::fromStdString(output->txId));
    }
}
