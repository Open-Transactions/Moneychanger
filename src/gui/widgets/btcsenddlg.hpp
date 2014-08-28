#ifndef BTCSENDDLG_HPP
#define BTCSENDDLG_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <bitcoin/sampleescrowclient.hpp>

#include <QWidget>

class BtcTxIdList;
class QTimer;

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
    void Update();

    void on_buttonSend_clicked();

    void on_buttonFindOutputs_clicked();

    void on_buttonCreateRawTx_clicked();

    void on_buttonSignRawTx_clicked();

    void on_buttonSendRawTx_clicked();

    void on_buttonShowUnspentTxids_clicked();

    void on_checkBox_toggled(bool checked);

private:
    void RefreshBalances();

    Ui::BtcSendDlg *ui;

    // raw transaction utxos
    BtcUnspentOutputs outputsToSpend;

    // information for offline signing
    BtcSigningPrerequisites prereqs;

    SampleEscrowClientPtr client;

    BtcTxIdList* txIdList;

    _SharedPtr<QTimer> updateTimer;
};

#endif // BTCSENDDLG_HPP
