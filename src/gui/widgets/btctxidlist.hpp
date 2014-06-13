#ifndef BTCTXIDLIST_H
#define BTCTXIDLIST_H

#include <QWidget>

#include <bitcoin-api/btcobjects.hpp>

namespace Ui {
class BtcTxIdList;
}

class BtcTxIdList : public QWidget
{
    Q_OBJECT

public:
    explicit BtcTxIdList(QWidget *parent = 0);
    ~BtcTxIdList();

    void Update(BtcUnspentOutputs outputs = BtcUnspentOutputs());

private:
    Ui::BtcTxIdList *ui;
};

#endif // BTCTXIDLIST_H
