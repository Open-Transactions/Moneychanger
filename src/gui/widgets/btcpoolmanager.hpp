#ifndef BTCPOOLMANAGER_H
#define BTCPOOLMANAGER_H

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/TR1_Wrapper.hpp>

#include <QWidget>

class QTimer;
typedef _SharedPtr<QTimer> QTimerPtr;

namespace Ui
{
    class BtcPoolManager;
}

class BtcPoolManager : public QWidget
{
    Q_OBJECT
public:
    explicit BtcPoolManager(QWidget *parent = 0);
    ~BtcPoolManager();

    // updates the list of pools in the GUI
    void SyncPoolList(bool refreshAll = false);

    QTimerPtr updateTimer;

public slots:
    void Update();

private slots:
    void on_buttonAddSimPool_clicked();

    void on_buttonRefreshPools_clicked();

    void on_listPools_clicked(const QModelIndex &index);

    void on_buttonRequestPayout_clicked();

    void on_buttonRequestDeposit_clicked();

    void on_buttonAddServer_clicked();

private:
    Ui::BtcPoolManager *ui;
    std::string lastDepositAddress;
    int64_t lastBalance;
};

#endif // BTCPOOLMANAGER_H
