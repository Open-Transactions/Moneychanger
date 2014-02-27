#ifndef BTCSENDDLG_HPP
#define BTCSENDDLG_HPP

#include <WinsockWrapper.h>
#include <ExportWrapper.h>



#include <QWidget>

namespace Ui {
class BtcSendDlg;
}

class BtcSendDlg : public QWidget
{
    Q_OBJECT

public:
    explicit BtcSendDlg(QWidget *parent = 0);
    ~BtcSendDlg();

private slots:
    void on_sendButton_clicked();

    void OnPoolListUpdate();

private:
    Ui::BtcSendDlg *ui;
};

#endif // BTCSENDDLG_HPP
