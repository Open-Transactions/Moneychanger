#ifndef SENDDLG_HPP
#define SENDDLG_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/TR1_Wrapper.hpp>

#include _CINTTYPES
#include _MEMORY

#include <QPointer>
#include <QWidget>

namespace Ui {
class MTSendDlg;
}

class MTSendDlg : public QWidget
{
    Q_OBJECT

    QString m_hisNymId;  // To:
    QString m_myAcctId;  // From:
    QString m_memo;      // Memo:
    QString m_amount;    // Amount:

    bool m_bSent;

public:
    explicit MTSendDlg(QWidget *parent=0);
    ~MTSendDlg();

    void setInitialHisNym (QString nymId)  { m_hisNymId  = nymId;  } // To:
    void setInitialMyAcct (QString acctId) { m_myAcctId  = acctId; } // From:
    void setInitialMemo   (QString memo)   { m_memo      = memo;   } // Memo:
    void setInitialAmount (QString amount) { m_amount    = amount; } // Amount:

    void dialog();
    // --------------------------
    int64_t rawAcctBalance();
    int64_t rawCashBalance(QString qstr_server_id, QString qstr_asset_id, QString qstr_nym_id);

//  QString shortAcctBalance(QString qstr_acct_id, QString qstr_asset_id);
    QString cashBalance     (QString qstr_server_id, QString qstr_asset_id, QString qstr_nym_id);
    // --------------------------
    bool sendFunds(QString memo, QString amount);

    bool sendCash           (int64_t amount, QString toNymId, QString fromAcctId, QString note);
    bool sendCashierCheque  (int64_t amount, QString toNymId, QString fromAcctId, QString note);
    bool sendCheque         (int64_t amount, QString toNymId, QString fromAcctId, QString note);

    bool sendChequeLowLevel (int64_t amount, QString toNymId, QString fromAcctId, QString note, bool isInvoice);

//  bool sendInvoice:(NSUInteger)amount toNymId:(NSString*)toNymId withNote:(NSString*)note;

signals:
    void balancesChanged();

    void ShowContact(QString);
    void ShowAccount(QString);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void closeEvent(QCloseEvent *event);

private slots:
    void onBalancesChanged();

    void on_toButton_clicked();
    void on_fromButton_clicked();
    void on_sendButton_clicked();

    void on_memoEdit_textChanged(const QString &arg1);

    void on_amountEdit_editingFinished();

    void on_toolButton_clicked();

    void on_toolButtonManageAccts_clicked();

private:
    bool already_init;

    Ui::MTSendDlg *ui;
};

#endif // SENDDLG_HPP
