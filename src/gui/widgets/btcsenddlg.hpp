#ifndef BTCSENDDLG_HPP
#define BTCSENDDLG_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <bitcoin/sampleescrowclient.hpp>

#include <QWidget>

class BtcTxIdList;

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
    void on_buttonSend_clicked();

    void on_buttonFindOutputs_clicked();

    void on_buttonCreateRawTx_clicked();

    void on_buttonSignRawTx_clicked();

    void on_buttonSendRawTx_clicked();

    void on_buttonShowUnspentTxids_clicked();

private:
    Ui::BtcSendDlg *ui;

    // raw transaction utxos
    BtcUnspentOutputs outputsToSpend;

    // information for offline signing
    BtcSigningPrerequisites prereqs;

    SampleEscrowClientPtr client;

    BtcTxIdList* txIdList;
};

#endif // BTCSENDDLG_HPP
