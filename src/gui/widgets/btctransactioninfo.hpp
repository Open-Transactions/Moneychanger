#ifndef BTCTRANSACTIONINFO_HPP
#define BTCTRANSACTIONINFO_HPP

#include <QWidget>
#include <QObject>

// forward declare bitcoin-api objects
#include <opentxs/TR1_Wrapper.hpp>
class BtcTransaction;
class BtcRawTransaction;
typedef _SharedPtr<BtcTransaction>         BtcTransactionPtr;
typedef _SharedPtr<BtcRawTransaction>         BtcRawTransactionPtr;

namespace Ui
{
    class BtcTransactionInfo;
}

class BtcTransactionInfo : public QWidget
{
    Q_OBJECT

public:
    explicit BtcTransactionInfo(QWidget *parent = 0);
    BtcTransactionInfo(BtcTransactionPtr tx, BtcRawTransactionPtr rawTx, QWidget *parent = 0);
    ~BtcTransactionInfo();

    void Initialize(BtcTransactionPtr tx, BtcRawTransactionPtr rawTx);

private:

    Ui::BtcTransactionInfo *ui;
};

#endif // BTCTRANSACTIONINFO_HPP
