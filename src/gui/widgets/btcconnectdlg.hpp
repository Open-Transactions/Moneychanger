#ifndef BTCCONNECTDLG_H
#define BTCCONNECTDLG_H

#include <QWidget>

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

private slots:
    void on_buttonConnect_clicked();

    void on_buttonDisconnect_clicked();

private:
    Ui::BtcConnectDlg *ui;
};

#endif // BTCCONNECTDLG_H
