#ifndef BTCTRANSACTIONMANAGER_H
#define BTCTRANSACTIONMANAGER_H

#include <QWidget>

namespace Ui {
class BtcTransactionManager;
}

class BtcTransactionManager : public QWidget
{
    Q_OBJECT

public:
    explicit BtcTransactionManager(QWidget *parent = 0);
    ~BtcTransactionManager();

private slots:
    void on_buttonRefresh_clicked();

private:
    void RefreshBitcoinTransactions();
    void RefreshPoolTransactions();

    Ui::BtcTransactionManager *ui;
};

#endif // BTCTRANSACTIONMANAGER_H
