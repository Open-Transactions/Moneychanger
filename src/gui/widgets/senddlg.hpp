#ifndef SENDDLG_HPP
#define SENDDLG_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"
#include "core/TR1_Wrapper.hpp"

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

    QString m_hisContactId;  // To: (contact)
    QString m_hisNymId;  // To: (nym)
    QString m_myAcctId;  // From:
    QString m_memo;      // Memo:
    QString m_amount;    // Amount:

    bool m_bSent{false};
    bool canMessage_{false};
    bool lockPayee_{false};
    bool inviteMode_{false};

public:
    explicit MTSendDlg(QWidget *parent=0);
    ~MTSendDlg();

    bool inviteMode() const { return inviteMode_; }
    void setInviteMode(bool invite_mode) { inviteMode_ = invite_mode; }

    void setInitialHisContact (QString contactId, bool bUsedInternally=false); // Payment To: (contact)
    void setInitialHisNym     (QString nymId)  { m_hisNymId  = nymId;  } // Payment To: (nym)
    void setInitialMyAcct     (QString acctId) { m_myAcctId  = acctId; } // From:
    void setInitialMemo       (QString memo)   { m_memo      = memo;   } // Memo:
    void setInitialAmount     (QString amount) { m_amount    = amount; } // Amount:

    void dialog();
    // --------------------------
    int64_t rawAcctBalance();
    int64_t rawCashBalance(QString qstr_notary_id, QString qstr_asset_id, QString qstr_nym_id);

//  QString shortAcctBalance(QString qstr_acct_id, QString qstr_asset_id);
    QString cashBalance     (QString qstr_notary_id, QString qstr_asset_id, QString qstr_nym_id);
    // --------------------------
    bool sendFunds(QString memo, QString amount);

    bool sendCash           (int64_t amount, QString toNymId, QString toContactId, QString fromAcctId, QString note);
    bool sendCashierCheque  (int64_t amount, QString toNymId, QString toContactId, QString fromAcctId, QString note);
    bool sendCheque         (int64_t amount, QString toNymId, QString toContactId, QString fromAcctId, QString note);

    bool sendChequeLowLevel (int64_t amount, QString toNymId, QString toContactId, QString fromAcctId, QString note, bool isInvoice);
    bool sendChequeLowLevel (int64_t amount,
                             QString toNymId,
                             QString toContactId,
                             QString fromAcctId,
                             QString note,
                             bool isInvoice,
                             bool payeeNymIsBlank); // Meaning ANY Nym can deposit this cheque.

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
    bool already_init{false};

    Ui::MTSendDlg *ui{nullptr};
};

#endif // SENDDLG_HPP
