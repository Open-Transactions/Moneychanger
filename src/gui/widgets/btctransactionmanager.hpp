#ifndef BTCTRANSACTIONMANAGER_H
#define BTCTRANSACTIONMANAGER_H

#include <QWidget>

namespace Ui {
class BtcTransactionManager;
}

class QTimer;
typedef _SharedPtr<QTimer> QTimerPtr;

class BtcTransactionManager : public QWidget
{
    Q_OBJECT

public:
    explicit BtcTransactionManager(QWidget *parent = 0);
    ~BtcTransactionManager();

private slots:
    void on_buttonRefresh_clicked();

    void on_buttonSearchTx_clicked();

    void on_tableTxBtc_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    void on_tableTxPool_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

private:
    void RefreshBalances();
    void RefreshBitcoinTransactions();
    void RefreshPoolTransactions(bool refreshAll = false);

    Ui::BtcTransactionManager *ui;

    QTimerPtr updateTimer;

public slots:
    void Update();
};

#endif // BTCTRANSACTIONMANAGER_H
