#ifndef REQUESTDLG_HPP
#define REQUESTDLG_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/TR1_Wrapper.hpp>

#include _CINTTYPES
#include _MEMORY

#include <QPointer>
#include <QWidget>


namespace Ui {
class MTRequestDlg;
}

class MTRequestDlg : public QWidget
{
    Q_OBJECT

    QString m_hisNymId;  // From:
    QString m_myAcctId;  // To:
    QString m_memo;      // Memo:
    QString m_amount;    // Amount:

    bool m_bSent;

public:
    explicit MTRequestDlg(QWidget *parent=0);
    ~MTRequestDlg();

    void setInitialHisNym (QString nymId)  { m_hisNymId  = nymId;  } // From:
    void setInitialMyAcct (QString acctId) { m_myAcctId  = acctId; } // To:
    void setInitialMemo   (QString memo)   { m_memo      = memo;   } // Memo:
    void setInitialAmount (QString amount) { m_amount    = amount; } // Amount:

    void dialog();
    // --------------------------
    bool requestFunds(QString memo, QString amount);

    bool sendInvoice        (int64_t amount, QString toNymId, QString fromAcctId, QString note);
    bool sendChequeLowLevel (int64_t amount, QString toNymId, QString fromAcctId, QString note, bool isInvoice);

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
    void on_requestButton_clicked();

    void on_memoEdit_textChanged(const QString &arg1);

    void on_amountEdit_editingFinished();

    void on_toolButton_clicked();

    void on_toolButtonManageAccts_clicked();

private:
    bool already_init;

    Ui::MTRequestDlg *ui;
};

#endif // REQUESTDLG_HPP
