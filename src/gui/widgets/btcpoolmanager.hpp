#ifndef BTCPOOLMANAGER_H
#define BTCPOOLMANAGER_H

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <QWidget>

namespace Ui
{
    class BtcPoolManager;
}

class BtcPoolManager : public QWidget
{
public:
    explicit BtcPoolManager(QWidget *parent = 0);
    ~BtcPoolManager();

private:
    Q_OBJECT

    // updates the list of pools in the GUI
    void SyncPoolList();

private slots:
    void on_buttonAddSimPool_clicked();

    void on_buttonRefreshPools_clicked();

    void on_listPools_clicked(const QModelIndex &index);

private:
    Ui::BtcPoolManager *ui;
};

#endif // BTCPOOLMANAGER_H
