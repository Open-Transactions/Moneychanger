#ifndef BTCRECEIVEDLG_H
#define BTCRECEIVEDLG_H

#include <QWidget>

namespace Ui {
class BtcReceiveDlg;
}

class BtcReceiveDlg : public QWidget
{
    Q_OBJECT

public:
    explicit BtcReceiveDlg(QWidget *parent = 0);
    ~BtcReceiveDlg();

private slots:
    void on_buttonNewAddress_clicked();

    void on_checkShowEmpty_stateChanged(int arg1);

    void on_checkShowWatchonly_stateChanged(int arg1);

    void on_buttonImport_clicked();

    void on_buttonCreateMultisig_clicked();

private:
    void UpdateAddressList();

    Ui::BtcReceiveDlg *ui;
};

#endif // BTCRECEIVEDLG_H
