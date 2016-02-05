#ifndef PROPOSEPLANDLG_HPP
#define PROPOSEPLANDLG_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"
#include "core/TR1_Wrapper.hpp"

#include _CINTTYPES
#include _MEMORY

#include <QPointer>
#include <QWidget>

namespace Ui {
class ProposePlanDlg;
}

class ProposePlanDlg : public QWidget
{
    Q_OBJECT

    QString m_hisNymId;  // To:
    QString m_myAcctId;  // From:
    QString m_memo;      // Memo:
    QString m_initialAmount;   // Amount of initial payment.
    QString m_recurringAmount; // Amount of recurring payment.

    bool m_bSent=false;

public:
    explicit ProposePlanDlg(QWidget *parent=0);
    ~ProposePlanDlg();

    void setInitialHisNym  (QString nymId)  { m_hisNymId        = nymId;  } // To:
    void setInitialMyAcct  (QString acctId) { m_myAcctId        = acctId; } // From:
    void setInitialMemo    (QString memo)   { m_memo            = memo;   } // Memo:
    void setInitialAmount  (QString amount) { m_initialAmount   = amount; } // Initial Amount:
    void setRecurringAmount(QString amount) { m_recurringAmount = amount; } // Recurring Amount:

    void dialog();
    // --------------------------
    bool proposePlan(QString memo, int64_t initial_amount, int64_t recurring_amount, const int32_t total_recurring_payment_count);
    // --------------------------
    void recalculateInitialStartDate();
    void recalculateRecurringStartDate();
    void recalculateRecurringNextDate();
    void recalculateRecurringLastDate();
    void recalculateExpirationDate();

signals:
    void ShowContact(QString);
    void ShowAccount(QString);
    void showLog(QString);
    void needToPopulateRecordlist();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void closeEvent(QCloseEvent *event);

private slots:
    void on_customerButton_clicked();
    void on_merchantButton_clicked();
    void on_sendButton_clicked();
    void on_memoEdit_textChanged(const QString &arg1);
    void on_initialAmountEdit_editingFinished();
    void on_recurringAmountEdit_editingFinished();
    void on_toolButton_clicked();
    void on_toolButtonManageAccts_clicked();
    void on_dateTimeEditFrom_dateTimeChanged(const QDateTime &dateTime);
    void on_checkBoxExpires_toggled(bool checked);
    void on_checkBoxInitial_toggled(bool checked);
    void on_checkBoxRecurring_toggled(bool checked);
    void on_initialDelayEdit_textChanged(const QString &arg1);
    void on_comboBoxInitial_currentIndexChanged(int index);
    void on_recurringDelayEdit_textChanged(const QString &arg1);
    void on_comboBoxRecurringDelay_currentIndexChanged(int index);
    void on_recurringPeriodEdit_textChanged(const QString &arg1);
    void on_comboBoxRecurringPeriod_currentIndexChanged(int index);
    void on_comboBoxRecurringTotalCount_currentTextChanged(const QString &arg1);
    void on_initialAmountEdit_textChanged(const QString &arg1);
    void on_recurringAmountEdit_textChanged(const QString &arg1);
    void on_dateTimeEditRecurring_dateTimeChanged(const QDateTime &dateTime);
    void on_dateTimeEditRecurringPeriod_dateTimeChanged(const QDateTime &dateTime);
    void on_dateTimeEditRecurringFinal_dateTimeChanged(const QDateTime &dateTime);

private:
    bool already_init=false;

    Ui::ProposePlanDlg *ui;
};

#endif // PROPOSEPLANDLG_HPP
