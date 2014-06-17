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

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_comboConflicts_currentIndexChanged(const QString &txId);

private:
    Ui::BtcTransactionInfo *ui;

    void SetTxInfo(const BtcTransactionPtr &tx);
    void SetRawTxInfo(const BtcRawTransactionPtr &rawTx);
};

#endif // BTCTRANSACTIONINFO_HPP
