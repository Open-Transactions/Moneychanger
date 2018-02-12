
#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/proposeplandlg.hpp>
#include <ui_proposeplandlg.h>

#include <gui/widgets/home.hpp>
#include <gui/widgets/overridecursor.hpp>
#include <gui/widgets/dlgchooser.hpp>

#include <core/moneychanger.hpp>
#include <core/handlers/contacthandler.hpp>
#include <core/handlers/focuser.h>

#include <opentxs/api/Api.hpp>
#include <opentxs/api/Native.hpp>
#include <opentxs/OT.hpp>
#include <opentxs/client/OT_ME.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>

#include <QDebug>
#include <QMessageBox>
#include <QKeyEvent>

time64_t SecondsPer(const time64_t& numberOfPeriods, int nPeriodType) // nPeriod is index into a combo box on the GUI.
{
    time64_t output=OT_TIME_ZERO, multiplier=OT_TIME_ZERO;

    switch(nPeriodType)
    {
    case 0: // Seconds
        multiplier = OTTimeGetTimeFromSeconds(static_cast<int64_t>(1));
        break;
    case 1: // Minutes
        multiplier = OT_TIME_MINUTE_IN_SECONDS;
        break;
    case 2: // Hours
        multiplier = OT_TIME_HOUR_IN_SECONDS;
        break;
    case 3: // Days
        multiplier = OT_TIME_DAY_IN_SECONDS;
        break;
    case 4: // Weeks
        multiplier = OT_TIME_DAY_IN_SECONDS * static_cast<int64_t>(7); // 7 days in a week.
        break;
    case 5: // Months
        multiplier = OT_TIME_MONTH_IN_SECONDS;
        break;
    case 6: // Quarters
        multiplier = OT_TIME_THREE_MONTHS_IN_SECONDS;
        break;
    case 7: // Years
        multiplier = OT_TIME_YEAR_IN_SECONDS;
        break;
    case 8: // Decades
        multiplier = OT_TIME_YEAR_IN_SECONDS * static_cast<int64_t>(10);
        break;
    case 9: // Centuries
        multiplier = OT_TIME_YEAR_IN_SECONDS * static_cast<int64_t>(100);
        break;
    case 10: // Millennia
        multiplier = OT_TIME_YEAR_IN_SECONDS * static_cast<int64_t>(1000);
        break;
    default:
        qDebug() << "Error in proposepaymentplandlg.cpp, function: SecondsPer: Unexpected/unknown index into combo box: " << nPeriodType;
        return 0;
    }

    output = multiplier * numberOfPeriods;
    return output;
}

void ProposePlanDlg::on_dateTimeEditFrom_dateTimeChanged(const QDateTime &dateTime)
{
    ui->dateTimeEditTo->setMinimumDateTime(dateTime);
    // -------------------------
    ui->dateTimeEditInitial->setMinimumDateTime(dateTime);
    recalculateInitialStartDate();
    // -------------------------
    ui->dateTimeEditRecurring->setMinimumDateTime(dateTime);
    recalculateRecurringStartDate();
}


void ProposePlanDlg::recalculateExpirationDate()
{
    const bool bHasInitialPayment   = ui->checkBoxInitial  ->isChecked();
    const bool bHasRecurringPayment = ui->checkBoxRecurring->isChecked();
//  const bool bExpires             = ui->checkBoxExpires  ->isChecked();
    // ----------------------------------------------------
//  const QDateTime qtimeFrom    = ui->dateTimeEditFrom->dateTime();
    const QDateTime qtimeInitial = ui->dateTimeEditInitial->dateTime();
    const QDateTime qtimeFinal   = ui->dateTimeEditRecurringFinal->dateTime();

//  const time64_t timeFrom    = qtimeFrom   .toTime_t(); // "Valid from" date for the payment plan.
    const time64_t timeInitial = qtimeInitial.toTime_t(); // Date of initial one-time payment.
    const time64_t timeFinal   = qtimeFinal  .toTime_t(); // Date of final recurring payment.

    const time64_t tempInitial = bHasInitialPayment   ? timeInitial : 0;
    const time64_t tempFinal   = bHasRecurringPayment ? timeFinal   : 0;

    const time64_t timeLastPayment = (tempInitial > tempFinal) ? tempInitial : tempFinal;
    // ----------------------------------------------------
    // Here, Moneychanger gives you a day's length of lee-way after your
    // payments come due, before the instrument expires.
    //
    time64_t expirationDate = timeLastPayment + OT_TIME_DAY_IN_SECONDS;
//  time64_t validityLength = expirationDate - timeFrom;
    // ------------------------------------
    QDateTime timestamp;
    timestamp.setTime_t(expirationDate);
    // ------------------------------------
    ui->dateTimeEditTo->setDateTime(timestamp);
}

void ProposePlanDlg::on_dateTimeEditRecurringFinal_dateTimeChanged(const QDateTime &dateTime)
{
    recalculateExpirationDate();
}

void ProposePlanDlg::on_checkBoxExpires_toggled(bool checked)
{
    ui->dateTimeEditTo->setVisible(checked);
    QString qstrTitle = checked ? tr("Expires on:") : tr("Expires?");
    ui->checkBoxExpires->setText(qstrTitle);
}

// --------------------------

void ProposePlanDlg::recalculateInitialStartDate()
{
    time64_t number_of_periods = ui->initialDelayEdit->text().toLongLong();
    time64_t total_seconds = SecondsPer(number_of_periods, ui->comboBoxInitial->currentIndex());
    // ------------------------------------
    time64_t fromDate = ui->dateTimeEditFrom->dateTime().toTime_t();
    // ------------------------------------
    time64_t result = fromDate + total_seconds;
    // ------------------------------------
    QDateTime timestamp;
    timestamp.setTime_t(result);
    // ------------------------------------
    ui->dateTimeEditInitial->setDateTime(timestamp);
}

void ProposePlanDlg::on_initialDelayEdit_textChanged(const QString &arg1)
{
    recalculateInitialStartDate();
}

void ProposePlanDlg::on_comboBoxInitial_currentIndexChanged(int index)
{
    recalculateInitialStartDate();
}

// --------------------------

void ProposePlanDlg::recalculateRecurringStartDate()
{
    time64_t number_of_periods = ui->recurringDelayEdit->text().toLongLong();
    time64_t total_seconds = SecondsPer(number_of_periods, ui->comboBoxRecurringDelay->currentIndex());
    // ------------------------------------
    time64_t fromDate = ui->dateTimeEditFrom->dateTime().toTime_t();
    // ------------------------------------
    time64_t recurringStartDate = fromDate + total_seconds;
    // ------------------------------------
    QDateTime timestamp;
    timestamp.setTime_t(recurringStartDate);
    // ------------------------------------
    ui->dateTimeEditRecurring->setDateTime(timestamp);
}

void ProposePlanDlg::on_recurringDelayEdit_textChanged(const QString &arg1)
{
    recalculateRecurringStartDate();
}

void ProposePlanDlg::on_comboBoxRecurringDelay_currentIndexChanged(int index)
{
    recalculateRecurringStartDate();
}

// --------------------------

void ProposePlanDlg::recalculateRecurringNextDate()
{
    time64_t recurringStartDate = 0;
    {
        time64_t number_of_periods = ui->recurringDelayEdit->text().toLongLong();
        time64_t total_seconds     = SecondsPer(number_of_periods, ui->comboBoxRecurringDelay->currentIndex());
        // ------------------------------------
        time64_t fromDate = ui->dateTimeEditFrom->dateTime().toTime_t();
        // ------------------------------------
        recurringStartDate = fromDate + total_seconds;
    }
    // ------------------------------------
    time64_t number_of_periods = ui->recurringPeriodEdit->text().toLongLong();
    time64_t total_seconds     = SecondsPer(number_of_periods, ui->comboBoxRecurringPeriod->currentIndex());
    // ------------------------------------
    time64_t recurringNextDate = recurringStartDate + total_seconds;
    // ------------------------------------
    QDateTime timestamp;
    timestamp.setTime_t(recurringNextDate);
    // ------------------------------------
    ui->dateTimeEditRecurringPeriod->setDateTime(timestamp);
}

void ProposePlanDlg::on_dateTimeEditRecurring_dateTimeChanged(const QDateTime &dateTime)
{
    recalculateRecurringNextDate();
}

void ProposePlanDlg::on_recurringPeriodEdit_textChanged(const QString &arg1)
{
    recalculateRecurringNextDate();
}

void ProposePlanDlg::on_comboBoxRecurringPeriod_currentIndexChanged(int index)
{
    recalculateRecurringNextDate();
}

// --------------------------

void ProposePlanDlg::recalculateRecurringLastDate()
{
    const time64_t fromDate = ui->dateTimeEditFrom->dateTime().toTime_t();
    const time64_t total_recurring_payment_count = ui->comboBoxRecurringTotalCount->currentText().toLongLong();

    time64_t recurringStartDate = 0;
    {
        const time64_t number_of_periods = ui->recurringDelayEdit->text().toLongLong();
        const time64_t delay_in_seconds  = SecondsPer(number_of_periods, ui->comboBoxRecurringDelay->currentIndex());
        // ------------------------------------
        recurringStartDate = fromDate + delay_in_seconds;
    }
    // ------------------------------------
    const time64_t number_of_periods = ui->recurringPeriodEdit->text().toLongLong();
    const time64_t seconds_between_payments = SecondsPer(number_of_periods, ui->comboBoxRecurringPeriod->currentIndex());
    // ------------------------------------
//  const time64_t recurringNextDate = recurringStartDate +   seconds_between_payments;
    const time64_t recurringLastDate = recurringStartDate + ( seconds_between_payments * (total_recurring_payment_count-1));
    // NOTE: Why the -1? Because if you have 5 payments from January 3rd through May 3rd, only 4 months have passed.
    // Since the start date already includes the first payment, we must take that into account.
    // ------------------------------------
    QDateTime timestamp;
    timestamp.setTime_t(recurringLastDate);
    // ------------------------------------
    ui->dateTimeEditRecurringFinal->setDateTime(timestamp);
}


void ProposePlanDlg::on_dateTimeEditRecurringPeriod_dateTimeChanged(const QDateTime &dateTime)
{
    recalculateRecurringLastDate();
}

void ProposePlanDlg::on_comboBoxRecurringTotalCount_currentTextChanged(const QString &arg1)
{
    recalculateRecurringLastDate();
}

// --------------------------

void ProposePlanDlg::on_initialAmountEdit_textChanged(const QString &arg1)
{
    if (!m_bSent)
    {
        if (m_myAcctId.isEmpty())
        {
            ui->labelInitialAmountFormatted->setText("");
            return;
        }
        // --------------------------------------
        std::string str_InstrumentDefinitionID(opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID(m_myAcctId.toStdString()));
        QString     amt = ui->initialAmountEdit->text();

        if (!amt.isEmpty() && !str_InstrumentDefinitionID.empty())
        {
            std::string str_temp(amt.toStdString());

            if (std::string::npos == str_temp.find(".")) // not found
                str_temp += '.';

            int64_t     amount               = opentxs::OT::App().API().Exec().StringToAmount(str_InstrumentDefinitionID, str_temp);
            std::string str_formatted_amount = opentxs::OT::App().API().Exec().FormatAmount(str_InstrumentDefinitionID, static_cast<int64_t>(amount));
            QString     qstr_FinalAmount     = QString::fromStdString(str_formatted_amount);

            ui->labelInitialAmountFormatted->setText(qstr_FinalAmount);
        }
        else
            ui->labelInitialAmountFormatted->setText("");
    }
}

void ProposePlanDlg::on_recurringAmountEdit_textChanged(const QString &arg1)
{
    if (!m_bSent)
    {
        if (m_myAcctId.isEmpty())
        {
            ui->labelRecurringAmountFormatted->setText("");
            return;
        }
        // --------------------------------------
        std::string str_InstrumentDefinitionID(opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID(m_myAcctId.toStdString()));
        QString     amt = ui->recurringAmountEdit->text();

        if (!amt.isEmpty() && !str_InstrumentDefinitionID.empty())
        {
            std::string str_temp(amt.toStdString());

            if (std::string::npos == str_temp.find(".")) // not found
                str_temp += '.';

            int64_t     amount               = opentxs::OT::App().API().Exec().StringToAmount(str_InstrumentDefinitionID, str_temp);
            std::string str_formatted_amount = opentxs::OT::App().API().Exec().FormatAmount(str_InstrumentDefinitionID, static_cast<int64_t>(amount));
            QString     qstr_FinalAmount     = QString::fromStdString(str_formatted_amount);

            ui->labelRecurringAmountFormatted->setText(qstr_FinalAmount);
        }
        else
            ui->labelRecurringAmountFormatted->setText("");
    }
}

void ProposePlanDlg::on_checkBoxInitial_toggled(bool checked)
{
    ui->labelInitialAmount->setEnabled(checked);
    ui->labelInitialAmountFormatted->setEnabled(checked);
    ui->labelInitialPaymentDate->setEnabled(checked);
    ui->labelInitialTimespan->setEnabled(checked);
    ui->comboBoxInitial->setEnabled(checked);
    ui->initialAmountEdit->setEnabled(checked);
    ui->initialDelayEdit->setEnabled(checked);
    ui->dateTimeEditInitial->setEnabled(checked);
}

void ProposePlanDlg::on_checkBoxRecurring_toggled(bool checked)
{
    ui->labelRecurringAmount->setEnabled(checked);
    ui->labelRecurringAmountFormatted->setEnabled(checked);
    ui->labelRecurringStartDate->setEnabled(checked);
    ui->labelRecurringDelay->setEnabled(checked);
    ui->comboBoxRecurringDelay->setEnabled(checked);
    ui->recurringAmountEdit->setEnabled(checked);
    ui->recurringDelayEdit->setEnabled(checked);
    ui->dateTimeEditRecurring->setEnabled(checked);
    ui->labelRecurringPeriodStartDate->setEnabled(checked);
    ui->labelRecurringPeriod->setEnabled(checked);
    ui->comboBoxRecurringPeriod->setEnabled(checked);
    ui->recurringPeriodEdit->setEnabled(checked);
    ui->dateTimeEditRecurringPeriod->setEnabled(checked);
    ui->labelTotalRecurringCount->setEnabled(checked);
    ui->comboBoxRecurringTotalCount->setEnabled(checked);
    ui->labelRecurringFinalDate->setEnabled(checked);
    ui->dateTimeEditRecurringFinal->setEnabled(checked);
}


// ----------------------------------------------------------------------

void ProposePlanDlg::on_sendButton_clicked()
{
    // Send recurring payment proposal, and then close dialog. Use progress bar.
    // -----------------------------------------------------------------
    // To:
    if (m_hisNymId.isEmpty())
    {
        QMessageBox::warning(this, tr("Moneychanger"),
                             tr("No customer selected. Please choose one."));
        return;
    }
    // -----------------------------------------------------------------
    // From:
    if (m_myAcctId.isEmpty())
    {
        QMessageBox::warning(this, tr("Moneychanger"),
                             tr("No merchant account selected. Please choose an account to receive the payments."));
        return;
    }
    // -----------------------------------------------------------------
    // Memo:
    if (ui->memoEdit->text().isEmpty())
    {
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, tr("Moneychanger"), tr("The memo is blank. Are you sure you want to send?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No)
          return;
    }
    // -----------------------------------------------------------------
    // Must have at least an initial payment or a recurring payment.
    //
    if (!ui->checkBoxInitial->isChecked() && !ui->checkBoxRecurring->isChecked())
    {
        QMessageBox::warning(this, tr("Moneychanger"),
                             tr("Please enter at least an initial payment or a recurring payment. (Or both.)"));
        return;
    }
    // -----------------------------------------------------------------
    // Initial Amount:
    if (ui->checkBoxInitial->isChecked())
    {
        if (ui->initialAmountEdit->text().isEmpty())
        {
            QMessageBox::warning(this, tr("Moneychanger"),
                                 tr("Please enter the amount for the initial payment."));
            return;
        }
        if (ui->initialDelayEdit->text().isEmpty())
        {
            QMessageBox::warning(this, tr("Moneychanger"),
                                 tr("When will the one-time payment occur? (The field is empty.)"));
            return;
        }
    }
    // -----------------------------------------------------------------
    // Recurring Amount:
    if (ui->checkBoxRecurring->isChecked())
    {
        if (ui->recurringAmountEdit->text().isEmpty())
        {
            QMessageBox::warning(this, tr("Moneychanger"),
                                 tr("Please enter the amount for the recurring payment."));
            return;
        }
        if (ui->recurringDelayEdit->text().isEmpty())
        {
            QMessageBox::warning(this, tr("Moneychanger"),
                                 tr("When do recurring payments begin? (The field is empty.)"));
            return;
        }
        if (ui->recurringPeriodEdit->text().isEmpty())
        {
            QMessageBox::warning(this, tr("Moneychanger"),
                                 tr("How often do recurring payments repeat? (The field is empty.)"));
            return;
        }
    }
    // -----------------------------------------------------------------
    // Make sure I'm not sending to myself (since that will fail...)
    //
    std::string str_fromAcctId(m_myAcctId.toStdString());
    QString     qstr_fromNymId(QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_NymID(str_fromAcctId)));

    if (0 == qstr_fromNymId.compare(m_hisNymId))
    {
        QMessageBox::warning(this, tr("Cannot Send To Yourself"),
                             tr("Sorry, but you cannot send this to yourself. Please choose another customer, or change the merchant account."));
        return;
    }
    // -----------------------------------------------------------------

    on_initialAmountEdit_editingFinished();
    on_recurringAmountEdit_editingFinished();

    // -----------------------------------------------------------------
    // TODO: We want an extra "ARE YOU SURE?" step to go right here, but likely it will
    // just be the passphrase dialog being FORCED to come up. But still, that means here
    // we'll have to set some kind of flag, probably, to force it to do that.
    //
    // NOTE: We'll want the "Are you sure" for the AMOUNT to display on that dialog.
    // (That is for security purposes.)
    //
    QMessageBox::StandardButton reply;

    const bool bWeHaveBoth = (ui->checkBoxInitial->isChecked() && ui->checkBoxRecurring->isChecked());
    QString qstrAnd(tr(" and "));
    const int32_t total_recurring_payment_count = ui->checkBoxRecurring->isChecked() ?
                ui->comboBoxRecurringTotalCount->currentText().toInt() : 0;
    // -----------------------------------------------------------------
    QString qstrInitialWarning = QString("%1 '%2'").
            arg(tr("Initial one-time payment is")).arg(ui->initialAmountEdit->text());
    if (!ui->checkBoxInitial->isChecked())
        qstrInitialWarning = QString("");

    QString qstrRecurringWarning = QString("%1%2 '%3'").arg(bWeHaveBoth ? qstrAnd : QString(" ")).
            arg(tr("Recurring amount is")).arg(ui->recurringAmountEdit->text());
    if (!ui->checkBoxRecurring->isChecked())
        qstrRecurringWarning = QString("");

    QString qstrNumberWarning = QString(". %1 %2").
            arg(tr("The total number of recurring payments is")).arg(total_recurring_payment_count);
    if (!ui->checkBoxRecurring->isChecked())
        qstrNumberWarning = QString("");
    // -----------------------------------------------------------------
    QString qstrWarning = QString("%1%2%3.<br/>%4").
            arg(qstrInitialWarning).
            arg(qstrRecurringWarning).
            arg(qstrNumberWarning).
            arg(tr("Send to customer?"));

    reply = QMessageBox::question(this, tr("Moneychanger"), qstrWarning, QMessageBox::Yes|QMessageBox::No);
    if (reply != QMessageBox::Yes)
        return;
    // -----------------------------------------------------------------
    QString memo                  = ui->memoEdit->text();
    QString qstr_initial_amount   = ui->initialAmountEdit->text();
    QString qstr_recurring_amount = ui->recurringAmountEdit->text();
    // -----------------------------------------------------------------
    if (memo.isEmpty())
        memo = tr("(Memo was empty.)");
    // ----------------------------------------------------
    if (qstr_initial_amount.isEmpty())
        qstr_initial_amount = QString("0");
    if (qstr_recurring_amount.isEmpty())
        qstr_recurring_amount = QString("0");
    // ----------------------------------------------------
    int64_t     initial_amount   = 0;
    int64_t     recurring_amount = 0;

    std::string str_InstrumentDefinitionID(opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID(m_myAcctId.toStdString()));

    if (!str_InstrumentDefinitionID.empty())
    {
        std::string str_initial_amount(qstr_initial_amount.toStdString());

        if (std::string::npos == str_initial_amount.find(".")) // not found
            str_initial_amount += '.';

        initial_amount = opentxs::OT::App().API().Exec().StringToAmount(str_InstrumentDefinitionID, str_initial_amount);
        // ----------------------------------------------------
        std::string str_recurring_amount(qstr_recurring_amount.toStdString());

        if (std::string::npos == str_recurring_amount.find(".")) // not found
            str_recurring_amount += '.';

        recurring_amount = opentxs::OT::App().API().Exec().StringToAmount(str_InstrumentDefinitionID, str_recurring_amount);
    }
    // ----------------------------------------------------
    if (initial_amount < 0)
    {
        QMessageBox::warning(this, tr("Moneychanger"),
                             tr("Cannot use a negative initial amount."));
        return;
    }
    if (recurring_amount < 0)
    {
        QMessageBox::warning(this, tr("Moneychanger"),
                             tr("Cannot use a negative recurring amount."));
        return;
    }
    // ----------------------------------------------------
    m_bSent = this->proposePlan(memo, initial_amount, recurring_amount, total_recurring_payment_count);
    // ----------------------------------------------------
    QString qstrPaymentType("proposal for recurring payment plan");

    if (!m_bSent)
    {
        qDebug() << "Recurring payments proposal: Failed.";
        QMessageBox::warning(this, tr("Failure"), QString("%1 %2.").arg(tr("Failure trying to send")).arg(qstrPaymentType));
    }
    else
    {
        qDebug() << "Success in sending proposal for recurring payments!";
        QMessageBox::information(this, tr("Success"), QString("%1 %2.").arg(tr("Success sending")).arg(qstrPaymentType));
        // --------------
        this->close();
    }
}


bool ProposePlanDlg::proposePlan(QString memo, int64_t initial_amount, int64_t recurring_amount, const int32_t total_recurring_payment_count)
{
    const std::string myAcctId = m_myAcctId.toStdString();
    const std::string myNymId  = opentxs::OT::App().API().Exec().GetAccountWallet_NymID(myAcctId);
    const std::string notaryID = opentxs::OT::App().API().Exec().GetAccountWallet_NotaryID(myAcctId);
    const std::string hisNymId = m_hisNymId.toStdString();
    // ----------------------------------------------------
    const QDateTime qtimeFrom    = ui->dateTimeEditFrom->dateTime();
    const QDateTime qtimeTo      = ui->dateTimeEditTo->dateTime();
    const QDateTime qtimeOneTime = ui->dateTimeEditInitial->dateTime();
    const QDateTime qtimeFirstRecurring = ui->dateTimeEditRecurring->dateTime();
    const QDateTime qtimeNextRecurring  = ui->dateTimeEditRecurringPeriod->dateTime();
//  const QDateTime qtimeFinal   = ui->dateTimeEditRecurringFinal->dateTime();

    const time64_t timeFrom    = qtimeFrom   .toTime_t(); // "Valid from" date for the payment plan.
    const time64_t timeTo      = qtimeTo     .toTime_t(); // "Valid to" date for the payment plan. (Expiration date.)
    const time64_t timeOneTime = qtimeOneTime.toTime_t(); // Date of initial one-time payment.
    const time64_t timeFirstRecurring = qtimeFirstRecurring.toTime_t(); // Date of first recurring payment.
    const time64_t timeNextRecurring  = qtimeNextRecurring.toTime_t();  // Date of second recurring payment.
//  const time64_t timeFinal   = qtimeFinal  .toTime_t(); // Date of final recurring payment.
    // ----------------------------------------------------
    const bool bHasInitialPayment   = ui->checkBoxInitial  ->isChecked();
    const bool bHasRecurringPayment = ui->checkBoxRecurring->isChecked();
    const bool bExpires             = ui->checkBoxExpires  ->isChecked();
    // ----------------------------------------------------
    {
        if (!opentxs::OT::App().API().OTME().make_sure_enough_trans_nums(2, notaryID, myNymId))
        {
            const QString qstrErr("Failed trying to acquire 2 transaction numbers (to write the recurring payment with.)");
            qDebug() << qstrErr;
            emit showLog(qstrErr);
            return false;
        }
    }
    // ------------------------------------------------------------
    const time64_t validityLength        = bExpires ? 0 : (timeTo - timeFrom);
    const time64_t oneTimePaymentDelay   = timeOneTime - timeFrom;
    const time64_t recurringPaymentDelay = timeFirstRecurring - timeFrom;
    const time64_t recurringPeriod       = timeNextRecurring  - timeFirstRecurring;
    // ------------------------------------------------------------
    const std::string str_plan = opentxs::OT::App().API().Exec().ProposePaymentPlan(
        notaryID,
        timeFrom,
        validityLength, //"valid to" is apparently ADDED to "valid from" so it's a bit misnamed.
        "", // This is the customer's asset account, but we don't know that yet.
        hisNymId, // The customer's Nym ID.
        memo.toStdString(),
        myAcctId,
        myNymId,
        bHasInitialPayment ? initial_amount : 0,
        bHasInitialPayment ? oneTimePaymentDelay : 0,
        bHasRecurringPayment ? recurring_amount : 0,
        bHasRecurringPayment ? recurringPaymentDelay : 0,
        bHasRecurringPayment ? recurringPeriod : 0,
        0, // No need for length in seconds, since the validFrom (2nd arg) expires it already.
        bHasRecurringPayment ? total_recurring_payment_count : 0);
    // ----------------------------------------------------
    // SwigWrap::EasyProposePlan is a version of ProposePaymentPlan that
    // compresses it into a fewer number of arguments. (Then it expands them
    // and calls ProposePaymentPlan.)
    // Basically this version has ALL the same parameters, but it stuffs two or
    // three at a time into a single parameter, as a comma-separated list in
    // string form. See details for each parameter, in the comment below.
    //
//    otOut << "date_range (from,to): " << daterange << "\n";
//    otOut << "consideration: " << memo << "\n";
//    otOut << "initial_payment (amount,delay): " << initialpayment << "\n";
//    otOut << "payment_plan (amount,delay,period): " << paymentplan << "\n";
//    otOut << "plan_expiry (length,number): " << planexpiry << "\n";

//    string plan = SwigWrap::EasyProposePlan(
//        server, daterange, hisacct, hisnym, memo, myacct, mynym, initialpayment,
//        paymentplan, planexpiry);
    // ------------------------------------------------------------
    if (str_plan.empty())
    {
        const QString qstrErr("Failed trying to create proposed recurring payment plan.");
        qDebug() << qstrErr;
        emit showLog(qstrErr);
        return false;
    }
    // ------------------------------------------------------------
    std::string  strResponse;
    {
        MTSpinner      theSpinner;
        strResponse  = opentxs::OT::App().API().OTME().send_user_payment(notaryID, myNymId, hisNymId, str_plan);
    }
    // ------------------------------------------------------------
    const int32_t nReturnVal = opentxs::OT::App().API().OTME().VerifyMessageSuccess(strResponse);

    if (1 != nReturnVal)
    {
        // We can't forget to do this, or we'll leak transaction numbers here.
        opentxs::OT::App().API().Exec().Msg_HarvestTransactionNumbers(str_plan, myNymId, false, false, false, false, false);
        // ------------------------------------
        const QString qstrErr("Failed trying to send proposed recurring payment plan to recipient. Perhaps network is down? (Or notary is down?)");
        qDebug() << qstrErr;
        Moneychanger::It()->HasUsageCredits(notaryID, myNymId);
        emit showLog(qstrErr);
    }
    else
    {
        // NOTE: We do not retrieve the account files here, in the case of success.
        // That's because none of them have changed yet from this operation -- not
        // until the recipient confirms the recurring payment plan.
        // However, the send_user_payment DOES put something in our outpayments box,
        // So we can still go ahead and reload the record list.
        //
        emit needToPopulateRecordlist();
        return true;
    }

    return false;
}

// ----------------------------------------------------------------------

void ProposePlanDlg::on_initialAmountEdit_editingFinished()
{
    if (!m_myAcctId.isEmpty() && !m_bSent)
    {
        std::string str_InstrumentDefinitionID(opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID(m_myAcctId.toStdString()));
        QString     amt = ui->initialAmountEdit->text();

        if (!amt.isEmpty() && !str_InstrumentDefinitionID.empty())
        {
            std::string str_temp(amt.toStdString());

            if (std::string::npos == str_temp.find(".")) // not found
                str_temp += '.';

            int64_t     amount               = opentxs::OT::App().API().Exec().StringToAmount(str_InstrumentDefinitionID, str_temp);
            std::string str_formatted_amount = opentxs::OT::App().API().Exec().FormatAmount(str_InstrumentDefinitionID, static_cast<int64_t>(amount));
            QString     qstr_FinalAmount     = QString::fromStdString(str_formatted_amount);

            ui->initialAmountEdit->setText(qstr_FinalAmount);
        }
    }
}
// ----------------------------------------------------------------------

void ProposePlanDlg::on_recurringAmountEdit_editingFinished()
{
    if (!m_myAcctId.isEmpty() && !m_bSent)
    {
        std::string str_InstrumentDefinitionID(opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID(m_myAcctId.toStdString()));
        QString     amt = ui->recurringAmountEdit->text();

        if (!amt.isEmpty() && !str_InstrumentDefinitionID.empty())
        {
            std::string str_temp(amt.toStdString());

            if (std::string::npos == str_temp.find(".")) // not found
                str_temp += '.';

            int64_t     amount               = opentxs::OT::App().API().Exec().StringToAmount(str_InstrumentDefinitionID, str_temp);
            std::string str_formatted_amount = opentxs::OT::App().API().Exec().FormatAmount(str_InstrumentDefinitionID, static_cast<int64_t>(amount));
            QString     qstr_FinalAmount     = QString::fromStdString(str_formatted_amount);

            ui->recurringAmountEdit->setText(qstr_FinalAmount);
        }
    }
}

// ----------------------------------------------------------------------


void ProposePlanDlg::on_merchantButton_clicked()
{
    // Select from Accounts in local wallet.
    //
    DlgChooser theChooser(this);
    theChooser.SetIsAccounts();
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;

    bool bFoundDefault = false;
    // -----------------------------------------------
    const int32_t acct_count = opentxs::OT::App().API().Exec().GetAccountCount();
    // -----------------------------------------------
    for(int32_t ii = 0; ii < acct_count; ++ii)
    {
        //Get OT Acct ID
        QString OT_acct_id = QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_ID(ii));
        QString OT_acct_name("");
        // -----------------------------------------------
        if (!OT_acct_id.isEmpty())
        {
            if (!m_myAcctId.isEmpty() && (OT_acct_id == m_myAcctId))
                bFoundDefault = true;
            // -----------------------------------------------
            MTNameLookupQT theLookup;

            OT_acct_name = QString::fromStdString(theLookup.GetAcctName(OT_acct_id.toStdString(), "", "", ""));
            // -----------------------------------------------
            the_map.insert(OT_acct_id, OT_acct_name);
        }
     }
    // -----------------------------------------------
    if (bFoundDefault && !m_myAcctId.isEmpty())
        theChooser.SetPreSelected(m_myAcctId);
    // -----------------------------------------------
    theChooser.setWindowTitle(tr("Select the Source Account"));
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
//      qDebug() << QString("SELECT was clicked for AcctID: %1").arg(theChooser.m_qstrCurrentID);

        if (!theChooser.m_qstrCurrentID.isEmpty())
        {
            QString display_name("");
            QString from_button_text("");
            // -----------------------------------------
            m_myAcctId = theChooser.m_qstrCurrentID;
            // -----------------------------------------
            if (theChooser.m_qstrCurrentName.isEmpty())
                display_name = QString("");
            else
                display_name = theChooser.m_qstrCurrentName;
            // -----------------------------------------
            from_button_text = MTHome::FormDisplayLabelForAcctButton(m_myAcctId, display_name);
            // -----------------------------------------
            ui->merchantButton->setText(from_button_text);
            // -----------------------------------------
            return;
        }
    }
    else
    {
//      qDebug() << "CANCEL was clicked";
    }
    // -----------------------------------------------
    m_myAcctId = QString("");
    ui->merchantButton->setText(tr("<Click to choose merchant account>"));
}





void ProposePlanDlg::on_toolButtonManageAccts_clicked()
{
    emit ShowAccount(m_myAcctId);
}

void ProposePlanDlg::on_toolButton_clicked()
{
    QString qstrContactID("");
    // ------------------------------------------------
    if (!m_hisNymId.isEmpty())
    {
        int nContactID = MTContactHandler::getInstance()->FindContactIDByNymID(m_hisNymId);

        if (nContactID > 0)
            qstrContactID = QString("%1").arg(nContactID);
    }
    // ------------------------------------------------
    emit ShowContact(qstrContactID);
}


void ProposePlanDlg::on_customerButton_clicked()
{
    // Select recipient from the address book and convert to Nym ID.
    // -----------------------------------------------
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;

    MTContactHandler::getInstance()->GetContacts(the_map);

    if (!m_hisNymId.isEmpty())
    {
        int nContactID = MTContactHandler::getInstance()->FindContactIDByNymID(m_hisNymId);

        if (nContactID > 0)
        {
            QString strTempID = QString("%1").arg(nContactID);
            theChooser.SetPreSelected(strTempID);
        }
    }
    // -----------------------------------------------
    theChooser.setWindowTitle(tr("Choose the Recipient"));
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
//        qDebug() << QString("SELECT was clicked for ID: %1").arg(theChooser.m_qstrCurrentID);

        // If not the same as before, then we have to choose a NymID based on the selected Contact.
        //
        int nSelectedContactID = theChooser.m_qstrCurrentID.toInt();
        int nOldNymContactID   = MTContactHandler::getInstance()->FindContactIDByNymID(m_hisNymId);

        // If they had matched, then we could have kept m_hisNymId as it was.
        // But since they are different, we have to figure out a NymID to use, based
        // on nSelectedContactID.
        //
        if (nSelectedContactID != nOldNymContactID)
        {
            QString qstrContactName;

            if (nSelectedContactID <= 0) // Should never happen.
            {
                qstrContactName  = QString("");
                m_hisNymId = QString("");
                ui->customerButton->setText(tr("<Click to choose customer>"));
                return;
            }
            // else...
            //
            qstrContactName = MTContactHandler::getInstance()->GetContactName(nSelectedContactID);

            if (qstrContactName.isEmpty())
                ui->customerButton->setText(tr("(Contact has a blank name)"));
            else
                ui->customerButton->setText(qstrContactName);
            // ---------------------------------------------
            // Next we need to find a Nym based on this Contact...
            //
            mapIDName theNymMap;

            if (MTContactHandler::getInstance()->GetNyms(theNymMap, nSelectedContactID))
            {
                if (theNymMap.size() == 1)
                {
                    mapIDName::iterator theNymIt = theNymMap.begin();

                    if (theNymIt != theNymMap.end())
                    {
                        QString qstrNymID   = theNymIt.key();
                        QString qstrNymName = theNymIt.value();

                        m_hisNymId = qstrNymID;
                    }
                    else
                    {
                        m_hisNymId = QString("");
                        ui->customerButton->setText(tr("<Click to choose customer>"));
                        // -------------------------------------
                        QMessageBox::warning(this, tr("Contact has no known identities"),
                                             tr("Sorry, Contact '%1' has no known NymIDs (to propose a recurring payment to.)").arg(qstrContactName));
                        return;
                    }
                }
                else // There are multiple Nyms to choose from.
                {
                    DlgChooser theNymChooser(this);
                    theNymChooser.m_map = theNymMap;
                    theNymChooser.setWindowTitle(tr("Recipient has multiple Nyms. (Please choose one.)"));
                    // -----------------------------------------------
                    if (theNymChooser.exec() == QDialog::Accepted)
                        m_hisNymId = theNymChooser.m_qstrCurrentID;
                    else // User must have cancelled.
                    {
                        m_hisNymId = QString("");
                        ui->customerButton->setText(tr("<Click to choose customer>"));
                    }
                }
            }
            else // No nyms found for this ContactID.
            {
                m_hisNymId = QString("");
                ui->customerButton->setText(tr("<Click to choose customer>"));
                // -------------------------------------
                QMessageBox::warning(this, tr("Contact has no known identities"),
                                     tr("Sorry, Contact '%1' has no known NymIDs (to propose a recurring payment to.)").arg(qstrContactName));
                return;
            }
            // --------------------------------
        }
    }
    else
    {
//      qDebug() << "CANCEL was clicked";
    }
    // -----------------------------------------------
}




void ProposePlanDlg::dialog()
{
/** Recurring Payment Proposal Dialog **/

    if (!already_init)
    {
        if (!Moneychanger::It()->expertMode())
        {
            ui->toolButton->setVisible(false);
            ui->toolButtonManageAccts->setVisible(false);
        }
        // ---------------------------------------
        connect(this, SIGNAL(showLog(QString)),           Moneychanger::It(), SLOT  (mc_showlog_slot(QString)));
        connect(this, SIGNAL(needToPopulateRecordlist()), Moneychanger::It(), SLOT  (onNeedToPopulateRecordlist()));
        // ---------------------------------------
        QString style_sheet = "QPushButton{border: none; border-style: outset; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa);}"
                "QPushButton:pressed {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }"
                "QPushButton:hover {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }";

        ui->merchantButton->setStyleSheet(style_sheet);
        ui->customerButton->setStyleSheet(style_sheet);

        // Here if there is pre-set data for the subject, contents, to, from, server, etc
        // then we set it here.
        //
        // -------------------------------------------
        std::string str_my_name;
        // -------------------------------------------
        if (!m_myAcctId.isEmpty()) // myAcct was provided.
        {
            MTNameLookupQT theLookup;

            str_my_name = theLookup.GetAcctName(m_myAcctId.toStdString(), "", "", "");

            if (str_my_name.empty())
                str_my_name = m_myAcctId.toStdString();
        }
        // -------------------------------------------
        if (str_my_name.empty())
        {
            m_myAcctId = QString("");
            ui->merchantButton->setText(tr("<Click to choose merchant account>"));
        }
        else
        {
            QString from_button_text = MTHome::FormDisplayLabelForAcctButton(m_myAcctId, QString::fromStdString(str_my_name));

            ui->merchantButton->setText(from_button_text);
        }
        // -------------------------------------------
        std::string str_his_name;
        // -------------------------------------------
        if (!m_hisNymId.isEmpty()) // hisNym was provided.
        {
            MTNameLookupQT theLookup;

            str_his_name = theLookup.GetNymName(m_hisNymId.toStdString(), "");

            if (str_his_name.empty())
                str_his_name = m_hisNymId.toStdString();
        }
        // -------------------------------------------
        if (str_his_name.empty())
        {
            m_hisNymId = QString("");
            ui->customerButton->setText(tr("<Click to choose customer>"));
        }
        else
            ui->customerButton->setText(QString::fromStdString(str_his_name));
        // -------------------------------------------
        if (!m_memo.isEmpty())
        {
            QString qstrTemp = m_memo;
            ui->memoEdit->setText(qstrTemp);
            // -----------------------
            this->setWindowTitle(tr("Recurring payment | Memo: %1").arg(qstrTemp));
        }
        // -------------------------------------------
        if (!m_initialAmount.isEmpty())
        {
            ui->initialAmountEdit->setText(m_initialAmount);
        }
        // -------------------------------------------
        if (!m_recurringAmount.isEmpty())
        {
            ui->recurringAmountEdit->setText(m_recurringAmount);
        }
        // -------------------------------------------
        ui->initialAmountEdit->setText("1");
        on_initialAmountEdit_editingFinished();
        ui->recurringAmountEdit->setText("2");
        on_recurringAmountEdit_editingFinished();
        // -------------------------------------------
        ui->dateTimeEditFrom->setMinimumDateTime(QDateTime::currentDateTime());
        ui->dateTimeEditTo  ->setMinimumDateTime(QDateTime::currentDateTime());
        ui->dateTimeEditInitial->setMinimumDateTime(QDateTime::currentDateTime());
        ui->dateTimeEditRecurring->setMinimumDateTime(QDateTime::currentDateTime());
        ui->dateTimeEditRecurringPeriod->setMinimumDateTime(QDateTime::currentDateTime());
        ui->dateTimeEditRecurringFinal->setMinimumDateTime(QDateTime::currentDateTime());

        ui->dateTimeEditTo  ->setVisible(false);
        // -------------------------------------------
        ui->dateTimeEditInitial->setStyleSheet("QDateTimeEdit { background-color: lightgray }");
        ui->dateTimeEditRecurring->setStyleSheet("QDateTimeEdit { background-color: lightgray }");
        ui->dateTimeEditRecurringPeriod->setStyleSheet("QDateTimeEdit { background-color: lightgray }");
        ui->dateTimeEditRecurringFinal->setStyleSheet("QDateTimeEdit { background-color: lightgray }");
        ui->dateTimeEditTo->setStyleSheet("QDateTimeEdit { background-color: lightgray }");
        // -------------------------------------------
        const int nComboStartingIndex = 3;  // seconds, minutes, hours, days, weeks, months, years.
        ui->comboBoxInitial         ->setCurrentIndex(nComboStartingIndex);
        ui->comboBoxRecurringDelay  ->setCurrentIndex(nComboStartingIndex);
        ui->comboBoxRecurringPeriod ->setCurrentIndex(nComboStartingIndex);

        ui->comboBoxRecurringTotalCount->setCurrentIndex(11); // index 11 contains "12" so it's for 12 payments total.
        // -------------------------------------------
        ui->customerButton->setFocus();


        /** Flag Already Init **/
        already_init = true;
    }

    //show();
    Focuser f(this);
    f.show();
    f.focus();
}




ProposePlanDlg::ProposePlanDlg(QWidget *parent) :
    QWidget(parent, Qt::Window),
    ui(new Ui::ProposePlanDlg)
{
    ui->setupUi(this);

    this->installEventFilter(this);

    connect(this, SIGNAL(ShowContact(QString)), Moneychanger::It(), SLOT(mc_showcontact_slot(QString)));
    connect(this, SIGNAL(ShowAccount(QString)), Moneychanger::It(), SLOT(mc_show_account_slot(QString)));
}


ProposePlanDlg::~ProposePlanDlg()
{
    delete ui;
}


void ProposePlanDlg::on_memoEdit_textChanged(const QString &arg1)
{
    if (arg1.isEmpty())
    {
        m_memo = QString("");
        this->setWindowTitle(tr("Recurring Payment"));
    }
    else
    {
        m_memo = arg1;
        this->setWindowTitle(tr("Recurring Payment | Memo: %1").arg(arg1));
    }
}


bool ProposePlanDlg::eventFilter(QObject *obj, QEvent *event){

    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Escape)
        {
            close(); // This is caught by this same filter.
            return true;
        }
        return true;
    }
    else
    {
        // standard event processing
        return QWidget::eventFilter(obj, event);
    }
}

void ProposePlanDlg::closeEvent(QCloseEvent *event)
{
    // Pop up a Yes/No dialog to confirm the cancellation of this payment.
    // (ONLY if the memo field contains text.)
    //
    if (!m_bSent && !ui->memoEdit->text().isEmpty())
    {
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, "", tr("Close without sending?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply != QMessageBox::Yes)
        {
          event->ignore();
          return;
        }
    }
    // -------------------------------------------
    QWidget::closeEvent(event);
}




