#ifndef BTCSENDDLG_HPP
#define BTCSENDDLG_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <bitcoin/sampleescrowclient.hpp>

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

    void on_buttonRefresh_clicked();

    void on_editAddress_textEdited(const QString &arg1);

private:
    Ui::BtcSendDlg *ui;

    SampleEscrowClientPtr client;
};

#endif // BTCSENDDLG_HPP
