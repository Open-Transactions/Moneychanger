#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/senddlg.hpp>
#include <ui_senddlg.h>

#include <gui/widgets/home.hpp>
#include <gui/widgets/overridecursor.hpp>
#include <gui/widgets/dlgchooser.hpp>

#include <core/moneychanger.hpp>
#include <core/handlers/contacthandler.hpp>

#include <opentxs/OTAPI.hpp>
#include <opentxs/OTAPI_Exec.hpp>
#include <opentxs/OT_ME.hpp>

#include <QDebug>
#include <QMessageBox>
#include <QKeyEvent>


// ----------------------------------------------------------------------
bool MTSendDlg::sendCash(int64_t amount, QString toNymId, QString fromAcctId, QString note)
{
    if (toNymId.size() == 0) {
        qDebug() << QString("Cannot send cash to an empty nym id, aborting.");
        return false;
    }
    if (fromAcctId.size() == 0) {
        qDebug() << QString("Cannot send cash from an unknown account id, aborting.");
        return false;
    }
    if (amount <= 0) {
        qDebug() << QString("Why send 0 (or less) units? Aborting send cash.");
        return false;
    }
    // ------------------------------------------------------------
    if (note.isEmpty())
        note = tr("From the Qt systray app.");
    // ------------------------------------------------------------
    std::string str_toNymId   (toNymId   .toStdString());
    std::string str_fromAcctId(fromAcctId.toStdString());
    // ------------------------------------------------------------
    std::string str_fromNymId(OTAPI_Wrap::It()->GetAccountWallet_NymID      (str_fromAcctId));
    std::string str_serverId (OTAPI_Wrap::It()->GetAccountWallet_ServerID   (str_fromAcctId));
    std::string str_assetId  (OTAPI_Wrap::It()->GetAccountWallet_AssetTypeID(str_fromAcctId));
    // ------------------------------------------------------------
    // TODO: for security reasons, we might change the below 'if' so that
    // it ONLY checks the cash balance, and not the account balance here.
    // This would force the user to withdraw the cash by hand first, before
    // sending it (which would make it possible to preserve untraceability.)
    //
    // Otherwise, if the send_instrument happens directly after the withdrawal,
    // the server will be able to tell who the recipient is purely by timing
    // analysis, without having to break the Chaumian blinding.
    //
    int64_t theCashBalance = MTHome::rawCashBalance(QString::fromStdString(str_serverId),
                                                    QString::fromStdString(str_assetId),
                                                    QString::fromStdString(str_fromNymId));
    int64_t theAcctBalance = MTHome::rawAcctBalance(this->m_myAcctId);

    if ((amount > theCashBalance) && (amount > (theAcctBalance + theCashBalance)))
    {
        qDebug() << QString("Aborting send cash: Amount is larger than cash balance, and is also "
              "larger than combined cash + account balance.");
        return false;
        // Note: you may be asking why doesn't the amount only have to be equal
        // to the SUM of the two balances? Why must it be available in FULL from
        // one or the other? The answer is, because if there is not enough available
        // in the purse, or if the tokens there have enough units, but not the right
        // denominations to create the amount exactly, then we have to withdraw the
        // amount from the account instead. In that case we will want to withdraw the
        // full amount, so that we can guarantee that we have tokens of the right
        // denominations for that amount.
        // NOTE: apparently the high-level API only withdraws the difference, which
        // means it could still end up with the right "amount" of cash, but the wrong
        // denominations necessary to "make change" for the exact amount.
    }
    // ------------------------------------------------------------
    int64_t SignedAmount = amount;
    qDebug() << QString("Sending cash:\n Server:'%1'\n Nym:'%2'\n Acct:'%3'\n ToNym:'%4'\n Amount:'%5'\n Note:'%6'").
          arg(str_serverId.c_str()).arg(str_fromNymId.c_str()).arg(str_fromAcctId.c_str()).arg(toNymId).arg(SignedAmount).arg(note);
    // ------------------------------------------------------------
    OT_ME madeEasy;

    bool bReturnValue = false;
    {
        MTSpinner theSpinner;

        bReturnValue = madeEasy.withdraw_and_send_cash(str_fromAcctId, str_toNymId, note.toStdString(), SignedAmount);
    }
    // ------------------------------------------------------------
    if (!bReturnValue)
        Moneychanger::HasUsageCredits(this, str_serverId, str_fromNymId);
    // ------------------------------------------------------------
    return bReturnValue;

    // NOTE: We don't retrieve the account files in the case of success, because the
    // above function already does all that internally.
}

// ----------------------------------------------------------------------

bool MTSendDlg::sendCashierCheque(int64_t amount, QString toNymId, QString fromAcctId, QString note)
{
    QString nsChequeType = QString("voucher");
    // ------------------------------------------------------------
    if (toNymId.size() == 0) {
        qDebug() << QString("Cannot send %1 to an empty nym id, aborting.").arg(nsChequeType);
        return false;
    }
    if (fromAcctId.size() == 0) {
        qDebug() << QString("Cannot send %1 from an unknown account id, aborting.").arg(nsChequeType);
        return false;
    }
    if (amount <= 0) {
        qDebug() << QString("Why send 0 (or less) units? Aborting send %1.").arg(nsChequeType);
        return false;
    }
    if (amount > MTHome::rawAcctBalance(m_myAcctId)) {
        qDebug() << QString("Aborting send %1: Amount is larger than account balance.").arg(nsChequeType);
        return false;
    }
    if (note.isEmpty())
        note = tr("From the desktop systray app.");
    // ------------------------------------------------------------
    std::string str_toNymId   (toNymId   .toStdString());
    std::string str_fromAcctId(fromAcctId.toStdString());
    // ------------------------------------------------------------
    std::string str_fromNymId(OTAPI_Wrap::It()->GetAccountWallet_NymID   (str_fromAcctId));
    std::string str_serverId (OTAPI_Wrap::It()->GetAccountWallet_ServerID(str_fromAcctId));
    // ------------------------------------------------------------
    int64_t SignedAmount = amount;
    qDebug() << QString("Sending %1:\n Server:'%2'\n Nym:'%3'\n Acct:'%4'\n ToNym:'%5'\n Amount:'%6'\n Note:'%7'").
                arg(nsChequeType).arg(str_serverId.c_str()).arg(str_fromNymId.c_str()).arg(str_fromAcctId.c_str()).
                arg(toNymId).arg(SignedAmount).arg(note);
    // ------------------------------------------------------------
    OT_ME madeEasy;

    std::string strAttempt  = "withdraw_voucher";
    std::string strResponse;
    {
        MTSpinner theSpinner;

        strResponse = madeEasy.withdraw_voucher(str_serverId, str_fromNymId, str_fromAcctId,
                                                str_toNymId, note.toStdString(), SignedAmount);
    }

    int32_t nInterpretReply = madeEasy.InterpretTransactionMsgReply(str_serverId, str_fromNymId, str_fromAcctId,
                                                                    strAttempt, strResponse);

    if (1 != nInterpretReply) // Failure
    {
        qDebug() << QString("Failure withdrawing voucher.");

        Moneychanger::HasUsageCredits(this, str_serverId, str_fromNymId);

        return false;
    }
    // ---------------------------------------------------------
    //else Success!
    std::string strLedger = OTAPI_Wrap::It()->Message_GetLedger(strResponse);

    if (strLedger.empty())
    {
        qDebug() << QString("Failed withdrawing voucher: strLedger is empty.");
        return false;
    }
    // ---------------------------------------------------------
    std::string strTransReply = OTAPI_Wrap::It()->Ledger_GetTransactionByIndex(str_serverId, str_fromNymId, str_fromAcctId, strLedger, 0); // index 0.

    if (strTransReply.empty())
    {
        qDebug() << QString("Error in withdraw_voucher: strTransReply is unexpectedly null, "
                            "returned by Ledger_GetTransactionByIndex, argument passed, index 0 and ledger:\n\n%s1\n").
                    arg(strLedger.c_str());
        return false;
    }
    // ---------------------------------------------------------
    std::string strVoucher = OTAPI_Wrap::It()->Transaction_GetVoucher(str_serverId, str_fromNymId, str_fromAcctId, strTransReply);

    if (strVoucher.empty())
    {
        qDebug() << QString("Error in withdraw_voucher: Voucher is unexpectedly empty, returned by Transaction_GetVoucher "
                            "with strTransReply set to:\n\n%1\n").arg(strTransReply.c_str());
        return false;
    }
    else
    {
        // Save a copy in my own outpayments box. I don't want to lose this voucher since it uses
        // one of my own transaction numbers. (If I later send the voucher to someone, OT is smart
        // enough to remove the first copy from outpayments, when adding the second copy.)
        //
        // Notice how I can send an instrument to myself. This doesn't actually send anything --
        // it just puts a copy into my outpayments box for safe-keeping.
        //
        OT_ME sendToSelf;
        sendToSelf.send_user_payment(str_serverId, str_fromNymId, str_fromNymId, strVoucher);
    }
    // ---------------------------------------------------------
    // Download all the intermediary files (account balance, inbox, outbox, etc)
    // since they have probably changed from this operation.
    //
    OT_ME retrieveAcct;
    bool bRetrieved = false;
    {
        MTSpinner theSpinner;

        bRetrieved = retrieveAcct.retrieve_account(str_serverId, str_fromNymId, str_fromAcctId, true); //bForceDownload defaults to false.
    }
    qDebug() << QString("%1 retrieving intermediary files for account %2. (After withdraw voucher.)").
                arg(bRetrieved ? QString("Success") : QString("Failed")).arg(str_fromAcctId.c_str());
    // -------------
    if (!bRetrieved)
        Moneychanger::HasUsageCredits(this, str_serverId, str_fromNymId);
    // ---------------------------------------------------------
    // We try to send it, even if the retrieve_account failed.
    // That way we insure that a copy of the voucher is stored
    // in the outpayment box. (Even if it fails to send.)
    // That way the user can later cancel or re-send it.
    //

    //OTLog::vOutput(0, "Sending payment to NymID: %s\n", str_toNymId.c_str());

    OT_ME sendPayment;

    std::string  strSendResponse;
    {
        MTSpinner theSpinner;

        strSendResponse = sendPayment.send_user_payment(str_serverId, str_fromNymId, str_toNymId, strVoucher);
    }

    int32_t nReturnVal = sendPayment.VerifyMessageSuccess(strSendResponse);

    if (1 != nReturnVal)
    {
        qDebug() << QString("send %1: Failed.").arg(nsChequeType);

        Moneychanger::HasUsageCredits(this, str_serverId, str_fromNymId);
    }
    else
    {
        qDebug() << QString("Success in send %1!").arg(nsChequeType);
        return true;
    }

    return false;
}

// ----------------------------------------------------------------------

//- (bool) transferAmount:(NSUInteger)amount toAcctId:(NSString*)toAcctId withNote:(NSString*)note {
//    if (toAcctId.length == 0) {
//        qDebug() << QString("Cannot send a transfer to an empty account id, aborting.");
//        return false;
//    }
//    if (amount <= 0) {
//        qDebug() << QString("Why send 0 units? Aborting.");
//        return false;
//    }
//    if (amount > [self rawAcctBalance]) {
//        qDebug() << QString("Aborting send transfer: Amount is larger than account balance.");
//        return false;
//    }
//
//    if (!note)
//        note = @"From the iOS Client.";
//
//    string serverId(self.server.contractId.UTF8String);
//    string nymId(self.nym.contractId.UTF8String);
//    string fromAcctId(self.contractId.UTF8String);
//
//    NSInteger SignedAmount = amount;
//
//    qDebug() << QString("Initiating Transfer:\n Server:'%s'\n Nym:'%s'\n FromAcct:'%s'\n ToAcct:'%@'\n Amount:'%ld'\n Note:'%@'",
//          serverId.c_str(),nymId.c_str(),fromAcctId.c_str(),toAcctId,static_cast<long>(SignedAmount),note);
//
//    string strResponse = [OTClientObjCWrapper sharedMe]->send_transfer(serverId,nymId,fromAcctId,toAcctId.UTF8String,static_cast<int64_t>(SignedAmount),note.UTF8String);
//    int32_t nReturnVal = [OTClientObjCWrapper sharedMe]->VerifyMessageSuccess(strResponse);
//
//    if (1 != nReturnVal)
//    {
//        qDebug() << QString("send_transfer: Failed.");
//        return false;
//    }
//    qDebug() << QString("Success in send_transfer!");
//    // ---------------------------------------------------------
//    // Download all the intermediary files (account balance, inbox, outbox, etc)
//    // since they have probably changed from this operation.
//    //
//    bool bRetrieved = [OTClientObjCWrapper sharedMe]->retrieve_account(serverId, nymId, fromAcctId, true); //bForceDownload defaults to false.
//    qDebug() << QString("%@ retrieving intermediary files for account %s. (After send transfer.)",
//          (bRetrieved ? @"Success" : @"Failed"), fromAcctId.c_str());
//    // ---------------------------------------------------------
//    return true;
//}

// ----------------------------------------------------------------------
//bool MTSendDlg::sendInvoice(int64_t amount, QString toNymId, QString fromAcctId, QString note)
//{
//    return sendChequeLowLevel(amount, toNymId, fromAcctId, note, true);
//}

// -------------------------------------------------------------------
bool MTSendDlg::sendCheque(int64_t amount, QString toNymId, QString fromAcctId, QString note)
{
    return sendChequeLowLevel(amount, toNymId, fromAcctId, note, false);
}

// ----------------------------------------------------------------------

bool MTSendDlg::sendChequeLowLevel(int64_t amount, QString toNymId, QString fromAcctId, QString note, bool isInvoice)
{
    QString nsChequeType = isInvoice ? QString("invoice") : QString("cheque");
    // ------------------------------------------------------------
    if (toNymId.size() == 0)
    {
        qDebug() << QString("Cannot send %1 to an empty nym id, aborting.").arg(nsChequeType);
        return false;
    }
    if (fromAcctId.size() == 0)
    {
        qDebug() << QString("Cannot send %1 from a non-existent account id, aborting.").arg(nsChequeType);
        return false;
    }
    if (amount <= 0)
    {
        qDebug() << QString("Why send 0 (or less) units? Aborting send %1.").arg(nsChequeType);
        return false;
    }
    // Note: in the case of cheques we don't have to see if the amount exceeds the
    // account balance here, since the money doesn't come out of the account until
    // the cheque is deposited by the recipient. Technically you could write a bad
    // cheque. Also, your balance will not change right away either -- not until the
    // recipient deposits the cheque.
    // Also of course, in the case of invoices, your account balance is irrelevant
    // since an invoice can only increase your balance, not decrease it.
    if (note.isEmpty())
        note = tr("From the Qt systray app.");
    // ------------------------------------------------------------
    std::string str_toNymId   (toNymId   .toStdString());
    std::string str_fromAcctId(fromAcctId.toStdString());
    // ------------------------------------------------------------
    std::string str_fromNymId(OTAPI_Wrap::It()->GetAccountWallet_NymID   (str_fromAcctId));
    std::string str_serverId (OTAPI_Wrap::It()->GetAccountWallet_ServerID(str_fromAcctId));
    // ------------------------------------------------------------
    int64_t SignedAmount = amount;
    int64_t trueAmount   = isInvoice ? (SignedAmount*(-1)) : SignedAmount;
    // ------------------------------------------------------------
    qDebug() << QString("Sending %1:\n Server:'%2'\n Nym:'%3'\n Acct:'%4'\n ToNym:'%5'\n Amount:'%6'\n Note:'%7'").
                arg(nsChequeType).arg(QString::fromStdString(str_serverId)).arg(QString::fromStdString(str_fromNymId)).
                arg(fromAcctId).arg(toNymId).arg(SignedAmount).arg(note);
    // ------------------------------------------------------------
    time_t tFrom = OTAPI_Wrap::It()->GetTime();
    time_t tTo   = tFrom + DEFAULT_CHEQUE_EXPIRATION;
    // ------------------------------------------------------------
    std::string strCheque = OTAPI_Wrap::It()->WriteCheque(str_serverId, trueAmount, tFrom, tTo,
                                                    str_fromAcctId, str_fromNymId, note.toStdString(),
                                                    str_toNymId);
    if (strCheque.empty())
    {
        qDebug() << QString("Failed creating %1.").arg(nsChequeType);
        return false;
    }
    // ------------------------------------------------------------
    OT_ME madeEasy;

    std::string  strResponse;
    {
        MTSpinner theSpinner;

        strResponse = madeEasy.send_user_payment(str_serverId, str_fromNymId, str_toNymId, strCheque);
    }

    int32_t nReturnVal  = madeEasy.VerifyMessageSuccess(strResponse);

    if (1 != nReturnVal)
    {
        qDebug() << QString("send %1: failed.").arg(nsChequeType);

        Moneychanger::HasUsageCredits(this, str_serverId, str_fromNymId);
    }
    else
    {
        qDebug() << QString("Success in send %1!").arg(nsChequeType);
        return true;
    }
    // NOTE: We do not retrieve the account files here, in the case of success.
    // That's because none of them have changed yet from this operation -- not
    // until the recipient deposits the cheque.

    return false;
}

// ----------------------------------------------------------------------

void MTSendDlg::on_amountEdit_editingFinished()
{
    if (!m_myAcctId.isEmpty() && !m_bSent)
    {
        std::string str_assetId(OTAPI_Wrap::It()->GetAccountWallet_AssetTypeID(m_myAcctId.toStdString()));
        QString     amt = ui->amountEdit->text();

        if (!amt.isEmpty() && !str_assetId.empty())
        {
            std::string str_temp(amt.toStdString());

            if (std::string::npos == str_temp.find(".")) // not found
                str_temp += '.';

            int64_t     amount               = OTAPI_Wrap::It()->StringToAmount(str_assetId, str_temp);
            std::string str_formatted_amount = OTAPI_Wrap::It()->FormatAmount(str_assetId, static_cast<int64_t>(amount));
            QString     qstr_FinalAmount     = QString::fromStdString(str_formatted_amount);

            ui->amountEdit->setText(qstr_FinalAmount);
        }
    }
}

// ----------------------------------------------------------------------
bool MTSendDlg::sendFunds(QString memo, QString qstr_amount)
{
    QString & toNymId     = m_hisNymId;
    QString & fromAcctId  = m_myAcctId;
    // ----------------------------------------------------
    if (toNymId.isEmpty())
    {
        qDebug() << "Cannot send funds to an empty nym id, aborting.";
        return false;
    }
    // ----------------------------------------------------
    if (fromAcctId.isEmpty())
    {
        qDebug() << "Cannot send funds from an empty acct id, aborting.";
        return false;
    }
    // ----------------------------------------------------
    if (memo.isEmpty())
        memo = tr("From the desktop client. (Empty memo.)");
    // ----------------------------------------------------
    if (qstr_amount.isEmpty())
        qstr_amount = QString("0");
    // ----------------------------------------------------
    int64_t     amount = 0;
    std::string str_assetId(OTAPI_Wrap::It()->GetAccountWallet_AssetTypeID(fromAcctId.toStdString()));

    if (!str_assetId.empty())
    {
        std::string str_amount(qstr_amount.toStdString());

        if (std::string::npos == str_amount.find(".")) // not found
            str_amount += '.';

        amount = OTAPI_Wrap::It()->StringToAmount(str_assetId, str_amount);
    }
    // ----------------------------------------------------
    if (amount <= 0)
    {
        qDebug() << "Cannot send a negative or zero amount.";
        return false;
    }
    // ----------------------------------------------------
    QString qstrPaymentType("");

    switch (ui->comboBox->currentIndex())
    {
    case (0): // Payment ("Cashier's Cheque")
        qstrPaymentType = tr("payment");
        m_bSent = sendCashierCheque(amount, toNymId, fromAcctId, memo);
        break;

    case (1): // Cheque
        qstrPaymentType = tr("cheque");
        m_bSent = sendCheque(amount, toNymId, fromAcctId, memo);
        break;

    case (2): // Cash
        qstrPaymentType = tr("cash");
        m_bSent = sendCash(amount, toNymId, fromAcctId, memo);
        break;

    default:  // ERROR
        qDebug() << "Error in MTSendDlg::sendFunds -- the comboBox has a bad index selected.";
        return false;
    }
    // ----------------------------------------------------
    if (!m_bSent)
    {
        qDebug() << "send funds: Failed.";
        QMessageBox::warning(this, tr("Failure"), QString("%1 %2.").arg(tr("Failure trying to send")).arg(qstrPaymentType));
    }
    else
    {
        qDebug() << "Success in send funds!";
        QMessageBox::information(this, tr("Success"), QString("%1 %2.").arg(tr("Success sending")).arg(qstrPaymentType));
    }
    // ---------------------------------------------------------
    return m_bSent;
}




void MTSendDlg::on_sendButton_clicked()
{
    // Send funds and then close dialog. Use progress bar.
    // -----------------------------------------------------------------
    // To:
    if (m_hisNymId.isEmpty())
    {
        QMessageBox::warning(this, tr("No Recipient Selected"),
                             tr("Please choose a recipient for these funds, before sending."));
        return;
    }
    // -----------------------------------------------------------------
    // From:
    if (m_myAcctId.isEmpty())
    {
        QMessageBox::warning(this, tr("No Sender Account Selected"),
                             tr("Please choose an account where the funds will come out of."));
        return;
    }
    // -----------------------------------------------------------------
    // Memo:
    if (ui->memoEdit->text().isEmpty())
    {
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, "", tr("The memo is blank. Are you sure you want to send?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No)
          return;
    }
    // -----------------------------------------------------------------
    // Amount:
    if (ui->amountEdit->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Amount is Empty"),
                             tr("Please enter the amount you wish to send."));
        return;
    }
    // -----------------------------------------------------------------
    // Make sure I'm not sending to myself (since that will fail...)
    //
    std::string str_fromAcctId(m_myAcctId.toStdString());
    QString     qstr_fromNymId(QString::fromStdString(OTAPI_Wrap::It()->GetAccountWallet_NymID(str_fromAcctId)));

    if (m_hisNymId == qstr_fromNymId)
    {
        QMessageBox::warning(this, tr("Cannot Send To Yourself"),
                             tr("Sorry, but you cannot send to yourself. Please choose another recipient, or change the sending account."));
        return;
    }
    // -----------------------------------------------------------------

    on_amountEdit_editingFinished();

    // -----------------------------------------------------------------
    // TODO: We want an extra "ARE YOU SURE?" step to go right here, but likely it will
    // just be the passphrase dialog being FORCED to come up. But still, that means here
    // we'll have to set some kind of flag, probably, to force it to do that.
    //
    // NOTE: We'll want the "Are you sure" for the AMOUNT to display on that dialog.
    // (That is for security purposes.)
    //
    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(this, "", QString("%1 '%2'<br/>%3").
                                  arg(tr("The amount is")).arg(ui->amountEdit->text()).arg(tr("Send?")),
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        // -----------------------------------------------------------------
        // Actually send the funds here.
        //
        QString memo   = ui->memoEdit  ->text();
        QString amount = ui->amountEdit->text();

        bool bSent = this->sendFunds(memo, amount);
        // -----------------------------------------------------------------
        if (bSent)
            emit balancesChanged();
        // -----------------------------------------------------------------
    }
}


void MTSendDlg::onBalancesChanged()
{
    this->close();
}



void MTSendDlg::on_fromButton_clicked()
{
    // Select from Accounts in local wallet.
    //
    DlgChooser theChooser(this);
    theChooser.SetIsAccounts();
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;

    bool bFoundDefault = false;
    // -----------------------------------------------
    const int32_t acct_count = OTAPI_Wrap::It()->GetAccountCount();
    // -----------------------------------------------
    for(int32_t ii = 0; ii < acct_count; ++ii)
    {
        //Get OT Acct ID
        QString OT_acct_id = QString::fromStdString(OTAPI_Wrap::It()->GetAccountWallet_ID(ii));
        QString OT_acct_name("");
        // -----------------------------------------------
        if (!OT_acct_id.isEmpty())
        {
            if (!m_myAcctId.isEmpty() && (OT_acct_id == m_myAcctId))
                bFoundDefault = true;
            // -----------------------------------------------
            MTNameLookupQT theLookup;

            OT_acct_name = QString::fromStdString(theLookup.GetAcctName(OT_acct_id.toStdString()));
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
        qDebug() << QString("SELECT was clicked for AcctID: %1").arg(theChooser.m_qstrCurrentID);

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
            ui->fromButton->setText(from_button_text);
            // -----------------------------------------
            return;
        }
    }
    else
    {
      qDebug() << "CANCEL was clicked";
    }
    // -----------------------------------------------
    m_myAcctId = QString("");
    ui->fromButton->setText(tr("<Click to choose Account>"));
}





void MTSendDlg::on_toolButtonManageAccts_clicked()
{
    emit ShowAccount(m_myAcctId);
}

void MTSendDlg::on_toolButton_clicked()
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


void MTSendDlg::on_toButton_clicked()
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
        qDebug() << QString("SELECT was clicked for ID: %1").arg(theChooser.m_qstrCurrentID);

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
                ui->toButton->setText(tr("<Click to choose Recipient>"));
                return;
            }
            // else...
            //
            qstrContactName = MTContactHandler::getInstance()->GetContactName(nSelectedContactID);

            if (qstrContactName.isEmpty())
                ui->toButton->setText(tr("(Contact has a blank name)"));
            else
                ui->toButton->setText(qstrContactName);
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
                        ui->toButton->setText(tr("<Click to choose Recipient>"));
                        // -------------------------------------
                        QMessageBox::warning(this, tr("Contact has no known identities"),
                                             tr("Sorry, Contact '%1' has no known NymIDs (to send funds to.)").arg(qstrContactName));
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
                        ui->toButton->setText(tr("<Click to choose Recipient>"));
                    }
                }
            }
            else // No nyms found for this ContactID.
            {
                m_hisNymId = QString("");
                ui->toButton->setText(tr("<Click to choose Recipient>"));
                // -------------------------------------
                QMessageBox::warning(this, tr("Contact has no known identities"),
                                     tr("Sorry, Contact '%1' has no known NymIDs (to send funds to.)").arg(qstrContactName));
                return;
            }
            // --------------------------------
        }
    }
    else
    {
      qDebug() << "CANCEL was clicked";
    }
    // -----------------------------------------------
}




void MTSendDlg::dialog()
{
/** Send Funds Dialog **/

    if (!already_init)
    {
        connect(this,               SIGNAL(balancesChanged()),
                Moneychanger::It(), SLOT  (onBalancesChanged()));
        // ---------------------------------------
        this->setWindowTitle(tr("Send Funds"));

        QString style_sheet = "QPushButton{border: none; border-style: outset; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa);}"
                "QPushButton:pressed {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }"
                "QPushButton:hover {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }";

        ui->fromButton->setStyleSheet(style_sheet);
        ui->toButton->setStyleSheet(style_sheet);

        // Here if there is pre-set data for the subject, contents, to, from, server, etc
        // then we set it here.
        //
        // -------------------------------------------
        std::string str_my_name;
        // -------------------------------------------
        if (!m_myAcctId.isEmpty()) // myAcct was provided.
        {
            MTNameLookupQT theLookup;

            str_my_name = theLookup.GetAcctName(m_myAcctId.toStdString());

            if (str_my_name.empty())
                str_my_name = m_myAcctId.toStdString();
        }
        // -------------------------------------------
        if (str_my_name.empty())
        {
            m_myAcctId = QString("");
            ui->fromButton->setText(tr("<Click to Select Account>"));
        }
        else
        {
            QString from_button_text = MTHome::FormDisplayLabelForAcctButton(m_myAcctId, QString::fromStdString(str_my_name));

            ui->fromButton->setText(from_button_text);
        }
        // -------------------------------------------


        // -------------------------------------------
        std::string str_his_name;
        // -------------------------------------------
        if (!m_hisNymId.isEmpty()) // hisNym was provided.
        {
            MTNameLookupQT theLookup;

            str_his_name = theLookup.GetNymName(m_hisNymId.toStdString());

            if (str_his_name.empty())
                str_his_name = m_hisNymId.toStdString();
        }
        // -------------------------------------------
        if (str_his_name.empty())
        {
            m_hisNymId = QString("");
            ui->toButton->setText(tr("<Click to choose Recipient>"));
        }
        else
            ui->toButton->setText(QString::fromStdString(str_his_name));
        // -------------------------------------------



        // -------------------------------------------
        if (!m_memo.isEmpty())
        {
            QString qstrTemp = m_memo;
            ui->memoEdit->setText(qstrTemp);
            // -----------------------
            this->setWindowTitle(tr("Send Funds | Memo: %1").arg(qstrTemp));
        }
        // -------------------------------------------



        // -------------------------------------------
        if (!m_amount.isEmpty())
        {
            QString qstrTemp = m_amount;
            ui->amountEdit->setText(m_amount);
        }
        // -------------------------------------------

        ui->toButton->setFocus();


        /** Flag Already Init **/
        already_init = true;
    }

    show();
}










MTSendDlg::MTSendDlg(QWidget *parent) :
    QWidget(parent, Qt::Window),
    m_bSent(false),
    already_init(false),
    ui(new Ui::MTSendDlg)
{
    ui->setupUi(this);

    this->installEventFilter(this);

    connect(this, SIGNAL(balancesChanged()), this, SLOT(onBalancesChanged()));

    connect(this, SIGNAL(ShowContact(QString)), Moneychanger::It(), SLOT(mc_showcontact_slot(QString)));
    connect(this, SIGNAL(ShowAccount(QString)), Moneychanger::It(), SLOT(mc_show_account_slot(QString)));
}


MTSendDlg::~MTSendDlg()
{
    delete ui;
}


void MTSendDlg::on_memoEdit_textChanged(const QString &arg1)
{
    if (arg1.isEmpty())
    {
        m_memo = QString("");
        this->setWindowTitle(tr("Send Funds"));
    }
    else
    {
        m_memo = arg1;
        this->setWindowTitle(tr("Send Funds | Memo: %1").arg(arg1));
    }
}


bool MTSendDlg::eventFilter(QObject *obj, QEvent *event){

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
        return QObject::eventFilter(obj, event);
    }
}

void MTSendDlg::closeEvent(QCloseEvent *event)
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

