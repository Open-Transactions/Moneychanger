#ifndef BTCCONNECTDLG_H
#define BTCCONNECTDLG_H

#include <QWidget>

#include <bitcoin-api/btcrpccurl.hpp>

namespace Ui
{
    class BtcConnectDlg;
}

class BtcConnectDlg : public QWidget
{
    Q_OBJECT

public:
    explicit BtcConnectDlg(QWidget *parent = 0);
    ~BtcConnectDlg();

    void show();

    BitcoinServerPtr rpcServer;

private slots:
    void on_buttonConnect_clicked();

    void on_buttonDisconnect_clicked();

private:
    Ui::BtcConnectDlg *ui;
};

#endif // BTCCONNECTDLG_H
