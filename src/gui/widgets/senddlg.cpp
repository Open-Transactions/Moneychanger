#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/senddlg.hpp>
#include <ui_senddlg.h>

#include <gui/ui/dlgexportedtopass.hpp>
#include <gui/widgets/home.hpp>
#include <gui/widgets/overridecursor.hpp>
#include <gui/widgets/dlgchooser.hpp>

#include <core/moneychanger.hpp>
#include <core/handlers/contacthandler.hpp>
#include <core/handlers/focuser.h>

#include <opentxs/opentxs.hpp>

#include <QDebug>
#include <QMessageBox>
#include <QKeyEvent>


// ----------------------------------------------------------------------
bool MTSendDlg::sendCash(int64_t amount, QString toNymId, QString toContactId, QString fromAcctId, QString note)
{
    // ------------------------------------------------------------
    if (fromAcctId.isEmpty()) {
        qDebug() << QString("Cannot send cash from an unknown account id, aborting.");
        return false;
    }
    if (amount <= 0) {
        qDebug() << QString("Why send 0 (or less) units? Aborting send cash.");
        return false;
    }
    // ------------------------------------------------------------
    if (toContactId.isEmpty())
    {
        qDebug() << "Cannot send cash to an empty contact id, aborting.";
        // todo: someday we will allow this, by popping up a dialog so the
        // user can copy the cash and send it out-of-band.
        //
        return false;
    }
    // --------------------------------
    if (toNymId.isEmpty())
    {
        if (!toContactId.isEmpty())
        {
            // Need to get nyms (or primary nym, or whatever) for this contact. Here.
            //
            // Note: Unlike other instruments, the "payee" really IS blank
            // when dealing with cash. However, you STILL need the recipient
            // Nym because you need to encrypt the cash tokens (in the purse)
            // to his pubkey.
            //
            // One way around that (which opentxs::Purse supports) is to
            // encrypt the cash tokens to a passphrase instead. Someday
            // that could be employed here. See "export cash" feature to
            // see this feature in action.
            //
            mapIDName theNymMap;

            if (!MTContactHandler::getInstance()->GetNyms(theNymMap, toContactId.toStdString()))
            {
                QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                     tr("Sorry, there are no NymIds associated with this opentxs contact. "));
                return false;
            }
            else
            {
                if (theNymMap.size() == 1) // This contact has exactly one Nym, so we'll go with it.
                {
                    mapIDName::iterator theNymIt = theNymMap.begin();
                    toNymId = theNymIt.key();
//                  QString qstrNymName = theNymIt.value();
                }
                else // There are multiple Nyms to choose from.
                {    // TODO: Choose one automatically from the CREDENTIALS!
                    DlgChooser theNymChooser(this);
                    theNymChooser.m_map = theNymMap;
                    theNymChooser.setWindowTitle(tr("Invoicee has multiple Nyms. (Please choose one.)"));
                    // -----------------------------------------------
                    if (theNymChooser.exec() == QDialog::Accepted)
                        toNymId = theNymChooser.m_qstrCurrentID;
                    else // User must have cancelled.
                        return  false;
                }
            }
        }
    }
    // --------------------------------
    // NOTE: the cash tokens are encrypted to the recipient's public key.
    // Only way around this is to encrypt them to a passphrase instead.
    // (Which also works, if you try the export cash feature...)
    //
    if (toNymId.isEmpty()) {
        qDebug() << QString("Cannot send cash to an empty nym id, aborting.");
        return false;
    }
    // ------------------------------------------------------------
    // todo: Fix it so you actually can put a memo onto a cash purse
    // (opentxs::Purse currently does not support this...)
    //
    if (note.isEmpty())
        note = tr("From the Qt systray app.");
    // ------------------------------------------------------------
    std::string str_toNymId   (toNymId   .toStdString());
    std::string str_fromAcctId(fromAcctId.toStdString());
    // ------------------------------------------------------------
    std::string str_fromNymId(opentxs::OT::App().API().Exec().GetAccountWallet_NymID      (str_fromAcctId));
    std::string str_NotaryID (opentxs::OT::App().API().Exec().GetAccountWallet_NotaryID   (str_fromAcctId));
    std::string str_InstrumentDefinitionID  (opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID(str_fromAcctId));
    // --------------------------------
    if (   toContactId.isEmpty()
        || str_fromNymId.empty()
        ||  (opentxs::Messagability::READY !=
             opentxs::OT::App().API().Sync()
             .CanMessage(opentxs::Identifier::Factory(str_fromNymId),
                         opentxs::Identifier::Factory(toContactId.toStdString()))))
    {
        QMessageBox::warning(this, tr("Not yet messageable"),
                             tr("This contact is not yet messageable. However, "
                                "credentials are being requested in the background, "
                                "so it's worth trying again sometime soon."));
        return false;
    }
    else
        canMessage_ = true;
    // --------------------------------
    // TODO: for security reasons, we might change the below 'if' so that
    // it ONLY checks the cash balance, and not the account balance here.
    // This would force the user to withdraw the cash by hand first, before
    // sending it (which would make it possible to preserve untraceability.)
    //
    // Otherwise, if the send_instrument happens directly after the withdrawal,
    // the server will be able to tell who the recipient is purely by timing
    // analysis, without having to break the Chaumian blinding.
    //
    int64_t theCashBalance = MTHome::rawCashBalance(QString::fromStdString(str_NotaryID),
                                                    QString::fromStdString(str_InstrumentDefinitionID),
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
#if OT_CASH
    // ------------------------------------------------------------
    int64_t SignedAmount = amount;
    qDebug() << QString("Withdrawing cash:\n Server:'%1'\n Nym:'%2'\n Acct:'%3'\n ToNym:'%4'\n Amount:'%5'\n Note:'%6'").
          arg(str_NotaryID.c_str()).arg(str_fromNymId.c_str()).arg(str_fromAcctId.c_str()).arg(toNymId).arg(SignedAmount).arg(note);
    // ------------------------------------------------------------
    // Below code is wrong because we cannot assume the server ID.
    // (That's why it's commented out)
    // We have to use the new Sync::SendPayment instead.
    //
//    bool bReturnValue = false;
//    {
//        MTSpinner theSpinner;
//        // NOTE: We don't retrieve the account files in the case of success, because the
//        // below function already does all that internally.
//        bReturnValue = opentxs::OT::App().API().Cash().withdraw_and_send_cash(str_fromAcctId, str_toNymId, SignedAmount);
//    }
//    // ------------------------------------------------------------
//    if (!bReturnValue)
//        Moneychanger::It()->HasUsageCredits(str_NotaryID, str_fromNymId);
//    // ------------------------------------------------------------
//    return bReturnValue;

    // New way:
    if (canMessage_)
    {
        // Need to get the actual purses withdrawn so I can send
        // them below.

        std::shared_ptr<const opentxs::Purse> pRecipientPurse;
        std::shared_ptr<const opentxs::Purse> pSenderPurse;

        bool bReturnValue = false;
        {
            MTSpinner theSpinner;
            // NOTE: We don't retrieve the account files in the case of success, because the
            // below function already does all that internally.
            bReturnValue = opentxs::OT::App().API().Cash().withdraw_and_export_cash(
                                str_fromAcctId,
                                str_toNymId,
                                SignedAmount,
                                pRecipientPurse,
                                pSenderPurse);
                              //bool bPasswordProtected=false
        }
        if (!bReturnValue)
        {
            qDebug() << QString("Failed to withdraw_and_export_cash from account/purse.");
            return false;
        }
        // ------------------------------------------------------------
        const auto bgthreadId
        {opentxs::OT::App().API().Sync().
            PayContactCash(opentxs::Identifier::Factory(str_fromNymId),
                       opentxs::Identifier::Factory(toContactId.toStdString()),
                       pRecipientPurse,
                       pSenderPurse
                       )};

        // Instead of MessageContact we use PayContactCash, so that it does
        // a send_user_instrument (with purses) instead of a send_user_message.
        // Note that the sender's and recipient's copies of the purse are
        // different, since each has his own copy that's encrypted to his own
        // public key. (So for example, if the recipient's copy were erroneously
        // placed in the sender's outbox, the sender wouldn't even be able to
        // read it.)
        //
        const auto status = opentxs::OT::App().API().Sync().Status(bgthreadId);

        const bool bAddToGUI = (opentxs::ThreadStatus::FINISHED_SUCCESS == status) ||
                               (opentxs::ThreadStatus::RUNNING == status);
        if (bAddToGUI) { // This means it's queued, not actually sent to the notary yet.
            //const bool bUseGrayText = (opentxs::ThreadStatus::FINISHED_SUCCESS != status);
            qDebug() << QString("Success in queueing cash for sending.");
            return true;
        }
        else
        {
            qDebug() << QString("Failed trying to queue cash for sending.");
        }
        return false;
    }
#endif // OT_CASH

    return false;
}

/*
int32_t CmdSendCash::sendCash(
    string& response,
    const string& server,
    const string& mynym,
    const string& assetType,
    const string& myacct,
    string& hisnym,
    const string& amount,
    string& indices,
    bool hasPassword) const
{
#if OT_CASH
    int64_t startAmount = "" == amount ? 0 : stoll(amount);

    // What we want to do from here is, see if we can send the cash purely using
    // cash we already have in the local purse. If so, we just package it up and
    // send it off using send_user_payment.
    //
    // But if we do NOT have the proper cash tokens in the local purse to send,
    // then we need to withdraw enough tokens until we do, and then try sending
    // again.

    int64_t remain = startAmount;
    if (!getPurseIndicesOrAmount(server, mynym, assetType, remain, indices)) {
        if ("" != indices) {
            otOut << "Error: invalid purse indices.\n";
            return -1;
        }

        // Not enough cash found in existing purse to match the amount
        CmdWithdrawCash cmd;
        if (1 != cmd.withdrawCash(myacct, remain)) {
            otOut << "Error: cannot withdraw cash.\n";
            return -1;
        }

        remain = startAmount;
        if (!getPurseIndicesOrAmount(
                server, mynym, assetType, remain, indices)) {
            otOut << "Error: cannot retrieve purse indices. "
            "(It's possible that you have enough cash, but not the right denominations for the amount requested).\n";
            return -1;
        }
    }

    CmdExportCash cmd;
    string retainedCopy = "";
    string exportedCash = cmd.exportCash(
        server, mynym, assetType, hisnym, indices, hasPassword, retainedCopy);
    if ("" == exportedCash) {
        otOut << "Error: cannot export cash.\n";
        return -1;
    }
    // By this point, exportedCash and retainedCopy should both be valid.

    std::shared_ptr<const Purse> recipientCopy(
        Purse::PurseFactory(String(exportedCash)));
    std::shared_ptr<const Purse> senderCopy(
        Purse::PurseFactory(String(retainedCopy)));

    OT_ASSERT(recipientCopy);
    OT_ASSERT(senderCopy);

    response = OT::App()
                   .API()
                   .ServerAction()
                   .SendCash(
                       Identifier(mynym),
                       Identifier(server),
                       Identifier(hisnym),
                       recipientCopy,
                       senderCopy)
                   ->Run();
    if (1 != responseStatus(response)) {
        // cannot send cash so try to re-import into sender's purse
        if (!SwigWrap::Wallet_ImportPurse(
                server, assetType, mynym, retainedCopy)) {
            otOut << "Error: cannot send cash AND failed re-importing purse."
                  << "\nServer: " << server << "\nAsset Type: " << assetType
                  << "\nNym: " << mynym
                  << "\n\nPurse (SAVE THIS SOMEWHERE!):\n\n"
                  << retainedCopy << "\n";
            return -1;
        }

        // at least re-importing succeeeded
        otOut << "Error: cannot send cash.\n";
        return -1;
    }

    return 1;
#else
    return -1;
#endif  // OT_CASH
}

*/
/*
bool Cash::withdraw_and_send_cash(
    const std::string& ACCT_ID,
    const std::string& recipientNymID,
    std::int64_t AMOUNT) const
{
    rLock lock(api_lock_);

    CmdSendCash sendCash;
    return 1 == sendCash.run(
                             "",
                             "",
                             ACCT_ID,
                             "",
                             recipientNymID,
                             std::to_string(AMOUNT),
                             "",
                             "");
}
*/


// ----------------------------------------------------------------------

bool MTSendDlg::sendCashierCheque(int64_t amount, QString toNymId, QString toContactId, QString fromAcctId, QString note)
{
    QString nsChequeType = QString("voucher");
    // ------------------------------------------------------------
    if (fromAcctId.isEmpty()) {
        qDebug() << QString("Cannot send %1 from an unknown account id, aborting.").arg(nsChequeType);
        return false;
    }
    if (amount <= 0) {
        qDebug() << QString("Why send 0 (or less) units? Aborting send %1.").arg(nsChequeType);
        return false;
    }
    if (amount > MTHome::rawAcctBalance(fromAcctId)) {
        qDebug() << QString("Aborting send %1: Amount is larger than account balance.").arg(nsChequeType);
        return false;
    }
    // ------------------------------------------------------------
    // Perhaps the sender wishes to leave the payee Nym blank, so that
    // ANYONE could deposit this voucher, and not just a specific person.
    // Fair enough.
    //
    // But even so, he still needs a Contact ID or he won't have
    // anyone to SEND the message that contains the voucher!
    //
    // Therefore by this point, we MUST have a Contact Id, even if the NymID
    // remains blank. (Alternately, we could just pop up the voucher in a dialog
    // so the user, in that case, can just copy it and send out-of-band.)
    //
    if (toContactId.isEmpty())
    {
        qDebug() << "Cannot send funds to an empty contact id, aborting.";
        // todo: someday we will allow this, by popping up a dialog so the
        // user can copy the voucher and send it out-of-band.
        //
        return false;
    }
    // --------------------------------
    if (toNymId.isEmpty())
    {
        if (!toContactId.isEmpty())
        {
            // Need to get nyms (or primary nym, or whatever) for this contact. Here.
            //
            // Todo: We really don't need to find the "to Nym" at this point.
            // Instead we could just ask the user here if he wants to leave
            // the payee field blank. Only if the user says "No" do we then
            // do the below code and choose a Nym as the payee.

            mapIDName theNymMap;

            if (!MTContactHandler::getInstance()->GetNyms(theNymMap, toContactId.toStdString()))
            {
                // Again: could choose at this point just to leave the payee
                // blank on the instrument.
                //
                QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                     tr("Sorry, there are no NymIds associated with this opentxs contact. "));
                return false;
            }
            else
            {
                if (theNymMap.size() == 1) // This contact has exactly one Nym, so we'll go with it.
                {
                    mapIDName::iterator theNymIt = theNymMap.begin();
                    toNymId = theNymIt.key();
//                  QString qstrNymName = theNymIt.value();
                }
                else // There are multiple Nyms to choose from.
                {    // TODO: Choose one automatically from the CREDENTIALS!
                    DlgChooser theNymChooser(this);
                    theNymChooser.m_map = theNymMap;
                    theNymChooser.setWindowTitle(tr("Payee has multiple Nyms. (Please choose one.)"));
                    // -----------------------------------------------
                    if (theNymChooser.exec() == QDialog::Accepted)
                        toNymId = theNymChooser.m_qstrCurrentID;
                    else // User must have cancelled.
                        return  false;
                }
            }
        }
    }
    // ------------------------------------------------------------
    // todo: someday comment this block out, allowing a blank "to nym"
    // field. (Like a cheque that any Nym can deposit).
    //
    if (toNymId.isEmpty()) {
        qDebug() << QString("Cannot send %1 to an empty nym id, aborting.").arg(nsChequeType);
        return false;
    }
    // ------------------------------------------------------------
    if (note.isEmpty())
        note = tr("From the desktop systray app.");
    // ------------------------------------------------------------
    std::string str_toNymId   (toNymId   .toStdString());
    std::string str_fromAcctId(fromAcctId.toStdString());
    // ------------------------------------------------------------
    std::string str_fromNymId(opentxs::OT::App().API().Exec().GetAccountWallet_NymID   (str_fromAcctId));
    std::string str_NotaryID (opentxs::OT::App().API().Exec().GetAccountWallet_NotaryID(str_fromAcctId));
    // ------------------------------------------------------------
    int64_t SignedAmount = amount;
//    qDebug() << QString("Sending %1:\n Server:'%2'\n Nym:'%3'\n Acct:'%4'\n ToNym:'%5'\n Amount:'%6'\n Note:'%7'").
//                arg(nsChequeType).arg(str_NotaryID.c_str()).arg(str_fromNymId.c_str()).arg(str_fromAcctId.c_str()).
//                arg(toNymId).arg(SignedAmount).arg(note);
    // ------------------------------------------------------------
    const auto notaryID = opentxs::Identifier::Factory(str_NotaryID),
                  nymID = opentxs::Identifier::Factory(str_fromNymId);
    if (!opentxs::OT::App().API().ServerAction().GetTransactionNumbers(nymID, notaryID, 1)) {
        qDebug() << QString("Failed trying to acquire a transaction number to write the %1 with.").arg(nsChequeType);
        return false;
    }
    // ------------------------------------------------------------
    // We don't want to go through the trouble of purchasing the voucher
    // if we can't even send messages to the recipient Contact. So let's
    // verify that first...
    //
    if (   toContactId.isEmpty()
        || str_fromNymId.empty()
        ||  (opentxs::Messagability::READY !=
             opentxs::OT::App().API().Sync()
             .CanMessage(opentxs::Identifier::Factory(str_fromNymId),
                         opentxs::Identifier::Factory(toContactId.toStdString()))))
    {
        QMessageBox::warning(this, tr("Not yet messageable"),
                             tr("This contact is not yet messageable. However, "
                                "credentials are being requested in the background, "
                                "so it's worth trying again sometime soon."));
        return false;
    }
    else
        canMessage_ = true;
    // --------------------------------
    const auto fromNymID = opentxs::Identifier::Factory(str_fromNymId),
            toNymID = opentxs::Identifier::Factory(str_toNymId),
            acctID = opentxs::Identifier::Factory(str_fromAcctId);
    std::string strAttempt  = "withdraw_voucher";
    std::string strResponse;
    {
        MTSpinner theSpinner;

        auto action = opentxs::OT::App().API().ServerAction().WithdrawVoucher(fromNymID, notaryID, acctID,
        		toNymID, SignedAmount, note.toStdString());
        strResponse = action->Run();
    }

    int32_t nInterpretReply = opentxs::InterpretTransactionMsgReply(str_NotaryID, str_fromNymId, str_fromAcctId,
                                                                    strAttempt, strResponse);

    if (1 != nInterpretReply) // Failure
    {
        qDebug() << QString("Failure withdrawing voucher.");

        Moneychanger::It()->HasUsageCredits(str_NotaryID, str_fromNymId);

        return false;
    }
    // ---------------------------------------------------------
    //else Success!
    std::string strLedger = opentxs::OT::App().API().Exec().Message_GetLedger(strResponse);

    if (strLedger.empty())
    {
        qDebug() << QString("Failed withdrawing voucher: strLedger is empty.");
        return false;
    }
    // ---------------------------------------------------------
    std::string strTransReply = opentxs::OT::App().API().Exec().Ledger_GetTransactionByIndex(str_NotaryID, str_fromNymId, str_fromAcctId, strLedger, 0); // index 0.

    if (strTransReply.empty())
    {
        qDebug() << QString("Error in withdraw_voucher: strTransReply is unexpectedly null, "
                            "returned by Ledger_GetTransactionByIndex, argument passed, index 0 and ledger:\n\n%s1\n").
                    arg(strLedger.c_str());
        return false;
    }
    // ---------------------------------------------------------
    std::string strVoucher = opentxs::OT::App().API().Exec().Transaction_GetVoucher(str_NotaryID, str_fromNymId, str_fromAcctId, strTransReply);

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
        std::shared_ptr<const opentxs::OTPayment> payment =
            std::make_shared<const opentxs::OTPayment>(opentxs::OT::App().Legacy().ClientDataFolder(), opentxs::String(strVoucher.c_str()));

        OT_ASSERT(payment);
        // SENDING HERE TO MYSELF (FOR OUTBOX)
        auto action = opentxs::OT::App().API().ServerAction().SendPayment(fromNymID, notaryID, fromNymID, payment);
        action->Run();
    }
    // ---------------------------------------------------------
    // Download all the intermediary files (account balance, inbox, outbox, etc)
    // since they have probably changed from this operation.
    //
    bool bRetrieved = false;
    {
        MTSpinner theSpinner;

        bRetrieved = opentxs::OT::App().API().ServerAction().DownloadAccount(
        		fromNymID, notaryID, acctID, true);
    }
    qDebug() << QString("%1 retrieving intermediary files for account %2. (After withdraw voucher.)").
                arg(bRetrieved ? QString("Success") : QString("Failed")).arg(str_fromAcctId.c_str());
    // -------------
    if (!bRetrieved)
        Moneychanger::It()->HasUsageCredits(str_NotaryID, str_fromNymId);
    // ---------------------------------------------------------
    // Below code is wrong because we cannot assume the server ID.
    // (That's why it's commented out)
    // We have to use the new Sync::SendPayment instead.
//    std::string  strSendResponse;
//    {
//        MTSpinner theSpinner;
//
//        std::shared_ptr<const opentxs::OTPayment> payment =
//            std::make_shared<const opentxs::OTPayment>(opentxs::String(strVoucher.c_str()));
//
//        OT_ASSERT(payment);
//
//        auto action = opentxs::OT::App().API().ServerAction().SendPayment(fromNymID, notaryID, toNymID, payment);
//        strSendResponse = action->Run();
//    }
//
//    int32_t nReturnVal = opentxs::VerifyMessageSuccess(strSendResponse);
//
//    if (1 != nReturnVal)
//    {
//        qDebug() << QString("send %1: Failed.").arg(nsChequeType);
//
//        Moneychanger::It()->HasUsageCredits(str_NotaryID, str_fromNymId);
//    }
//    else
//    {
//        qDebug() << QString("Success in send %1!").arg(nsChequeType);
//        return true;
//    }

    // New way:
    if (canMessage_)
    {
        const opentxs::String otstrCheque(strVoucher.c_str());

        std::shared_ptr<const opentxs::OTPayment> pPayment
        (new opentxs::OTPayment(opentxs::OT::App().Legacy().ClientDataFolder(), otstrCheque));

        const auto bgthreadId
        {opentxs::OT::App().API().Sync().
            PayContact(opentxs::Identifier::Factory(str_fromNymId),
                       opentxs::Identifier::Factory(toContactId.toStdString()),
                       pPayment
                       )};

        // Instead of MessageContact we use PayContact, so that it does
        // a send_user_instrument instead of a send_user_message.
        //
        const auto status = opentxs::OT::App().API().Sync().Status(bgthreadId);

        const bool bAddToGUI = (opentxs::ThreadStatus::FINISHED_SUCCESS == status) ||
        (opentxs::ThreadStatus::RUNNING == status);
        if (bAddToGUI) { // This means it's queued, not actually sent to the notary yet.
            //const bool bUseGrayText = (opentxs::ThreadStatus::FINISHED_SUCCESS != status);
            qDebug() << QString("Success in queueing %1 for sending.").arg(nsChequeType);
            return true;
        }
        else
        {
            qDebug() << QString("Failed trying to queue %1 for sending.").arg(nsChequeType);
        }
        return false;
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
//    string NotaryID(self.server.contractId.UTF8String);
//    string nymId(self.nym.contractId.UTF8String);
//    string fromAcctId(self.contractId.UTF8String);
//
//    NSInteger SignedAmount = amount;
//
//    qDebug() << QString("Initiating Transfer:\n Server:'%s'\n Nym:'%s'\n FromAcct:'%s'\n ToAcct:'%@'\n Amount:'%ld'\n Note:'%@'",
//          NotaryID.c_str(),nymId.c_str(),fromAcctId.c_str(),toAcctId,static_cast<long>(SignedAmount),note);
//
//    string strResponse = [OTClientObjCWrapper sharedMe]->send_transfer(NotaryID,nymId,fromAcctId,toAcctId.UTF8String,static_cast<int64_t>(SignedAmount),note.UTF8String);
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
//    bool bRetrieved = [OTClientObjCWrapper sharedMe]->retrieve_account(NotaryID, nymId, fromAcctId, true); //bForceDownload defaults to false.
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
bool MTSendDlg::sendCheque(int64_t amount, QString toNymId, QString toContactId, QString fromAcctId, QString note)
{
    // This is already verified by the calling function.
//    if (fromAcctId.isEmpty())
//    {
//        QMessageBox::warning(this, tr("Missing Payer Account"),
//            tr("You must choose which of your accounts will send the funds."));
//        return false;
//    }
    std::string str_fromAcctId(fromAcctId.toStdString());
    std::string str_fromNymId(opentxs::OT::App().API().Exec().GetAccountWallet_NymID(str_fromAcctId));
    // ------------------------------------------------------------
    // If there's a contact ID for recipient but no Nym ID, then we need to find
    // out the Nym ID, so we can WRITE him the cheque. (Unless we deliberately leave
    // the recipient blank on the cheque, to use it as an invite).
    //
    // If there's no Contact Id, then we need to find out
    // the Contact ID, so we can SEND him the cheque.
    //
    // So both are needed / preferred though I suppose it's possible to leave the field
    // blank on the invoice itself (like a blank cheque, which opentxs supports) so we can
    // still write the cheque, worst case. And it's still possible to send through the server
    // where the cheque is drawn, though there's no guarantee that the recipient has ever
    // seen that server before! (Whereas with a Contact, you definitely KNOW you are
    // sending the instrument via a method where the recipient WILL receive it).
    //
    // Already done higher up.
//    if (toContactId.isEmpty())
//    {
//        if (!toNymId.isEmpty())
//        {
//            const opentxs::Identifier toContact_Id = opentxs::OT::App().Contact()
//                .ContactID(opentxs::Identifier{toNymId.toStdString()});
//            const opentxs::String     strToContactId(toContact_Id);
//            toContactId = toContact_Id.empty() ? QString("") : QString::fromStdString(std::string(strToContactId.Get()));
//        }
//    }
    // --------------------------------
    // Perhaps the sender wishes to leave the recipient Nym blank, so that
    // ANYONE could deposit this cheque, and not just a specific recipient.
    // Fair enough.
    //
    // But even so, he still needs a Contact ID or he won't have
    // anyone to SEND the message that contains the cheque!
    //
    // Therefore by this point, we MUST have a Contact Id, even if the NymID
    // remains blank. (Alternately, we could just pop up the cheque in a dialog
    // so the user, in that case, can just copy it and send out-of-band.)
    //
    if (toContactId.isEmpty())
    {
        qDebug() << "Cannot send funds to an empty contact id, aborting.";
        // todo: someday we will allow this, by popping up a dialog so the
        // user can copy the cheque and send it out-of-band.
        //
        return false;
    }
    // --------------------------------
    if (toNymId.isEmpty())
    {
        if (!toContactId.isEmpty())
        {
            // Need to get nyms (or primary nym, or whatever) for this contact. Here.
            //
            // Todo: We really don't need to find the "to Nym" at this point.
            // Instead we could just ask the user here if he wants to leave
            // the payee field blank. Only if the user says "No" do we then
            // do the below code and choose a Nym as the recipient/payee.
            //
            mapIDName theNymMap;

            if (!MTContactHandler::getInstance()->GetNyms(theNymMap, toContactId.toStdString()))
            {
                QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                    tr("Sorry, there are no NymIds associated with this opentxs contact. "));
                return false;
            }
            else
            {
                if (theNymMap.size() == 1) // This contact has exactly one Nym, so we'll go with it.
                {
                    mapIDName::iterator theNymIt = theNymMap.begin();
                    toNymId = theNymIt.key();
//                  QString qstrNymName = theNymIt.value();
                }
                else // There are multiple Nyms to choose from.
                {    // TODO: Choose one automatically from the CREDENTIALS!
                    DlgChooser theNymChooser(this);
                    theNymChooser.m_map = theNymMap;
                    theNymChooser.setWindowTitle(tr("Payee has multiple Nyms. (Please choose one.)"));
                    // -----------------------------------------------
                    if (theNymChooser.exec() == QDialog::Accepted)
                        toNymId = theNymChooser.m_qstrCurrentID;
                    else // User must have cancelled.
                        return false;
                    // -----------------------------------------------
                }
            }
        }
    }
    // --------------------------------
    if (   toContactId.isEmpty()
        || str_fromNymId.empty()
        ||  (opentxs::Messagability::READY !=
             opentxs::OT::App().API().Sync()
             .CanMessage(opentxs::Identifier::Factory(str_fromNymId),
                         opentxs::Identifier::Factory(toContactId.toStdString()))))
    {
        QMessageBox::warning(this, tr("Not yet messageable"),
                             tr("This contact is not yet messageable. However, "
                                "credentials are being requested in the background, "
                                "so it's worth trying again sometime soon."));
        return false;
    }
    else
        canMessage_ = true;
    // --------------------------------
    return sendChequeLowLevel(amount, toNymId, toContactId, fromAcctId, note,
                              false, //isInvoice = false
                              toNymId.isEmpty()); // payeeNymIsBlank
}

// ----------------------------------------------------------------------

bool MTSendDlg::sendChequeLowLevel (int64_t amount,
                         QString toNymId,
                         QString toContactId,
                         QString fromAcctId,
                         QString note,
                         bool isInvoice,
                         bool payeeNymIsBlank) // Meaning ANY Nym can deposit this cheque.
{
    QString nsChequeType = isInvoice ? QString("invoice") : QString("cheque");
    // ------------------------------------------------------------
    if (toNymId.isEmpty() && !payeeNymIsBlank)
    {
        qDebug() << QString("Cannot send %1 to an empty nym id, aborting.").arg(nsChequeType);
        return false;
    }
    if (toContactId.isEmpty())
    {
        qDebug() << QString("Cannot send %1 to an empty contact id, aborting.").arg(nsChequeType);
        return false;
        // Todo: someday we could allow this case, and just pop up the invoice
        // on the screen in a dialog for export so the user can copy/paste it
        // out-of-band.
    }
    if (fromAcctId.isEmpty())
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
    std::string str_fromNymId(opentxs::OT::App().API().Exec().GetAccountWallet_NymID   (str_fromAcctId));
    std::string str_NotaryID (opentxs::OT::App().API().Exec().GetAccountWallet_NotaryID(str_fromAcctId));
    // ------------------------------------------------------------
    int64_t SignedAmount = amount;
    int64_t trueAmount   = isInvoice ? (SignedAmount*(-1)) : SignedAmount;
    // ------------------------------------------------------------
//    std::string str_InstrumentDefinitionID(opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID(fromAcctId.toStdString()));
//
//    if (!str_InstrumentDefinitionID.empty())
//    {
//        std::string str_formatted_amount = opentxs::OT::App().API().Exec().FormatAmount(str_InstrumentDefinitionID, amount);
//        QString     qstr_FinalAmount     = QString::fromStdString(str_formatted_amount);
//    }

    const QString qstrAmount = ui->amountEdit->text();

//    qDebug() << QString("Sending %1:\n Server:'%2'\n Nym:'%3'\n Acct:'%4'\n ToNym:'%5'\n Amount:'%6'\n Note:'%7'").
//                arg(nsChequeType).arg(QString::fromStdString(str_NotaryID)).arg(QString::fromStdString(str_fromNymId)).
//                arg(fromAcctId).arg(toNymId).arg(SignedAmount).arg(note);
    // ------------------------------------------------------------
    time64_t tFrom = opentxs::OT::App().API().Exec().GetTime();
    time64_t tTo   = tFrom + DEFAULT_CHEQUE_EXPIRATION;
    // ------------------------------------------------------------
    const auto notaryID = opentxs::Identifier::Factory(str_NotaryID),
              fromNymID = opentxs::Identifier::Factory(str_fromNymId),
                toNymID = opentxs::Identifier::Factory(str_toNymId);
    if (!opentxs::OT::App().API().ServerAction().GetTransactionNumbers(fromNymID, notaryID, 1)) {
        qDebug() << QString("Failed trying to acquire a transaction number to write the %1 with.").arg(nsChequeType);
        return false;
    }
    // ------------------------------------------------------------
    // No point even writing the cheque if we already know it will fail
    // to be sent...
    //
    if (canMessage_)
    {
        std::string strCheque = opentxs::OT::App().API().Exec().WriteCheque(
                                    str_NotaryID, trueAmount, tFrom, tTo,
                                    str_fromAcctId, str_fromNymId, note.toStdString(),
                                    str_toNymId);
        if (strCheque.empty())
        {
            qDebug() << QString("Failed creating %1.").arg(nsChequeType);
            return false;
        }
        // ----------------------------------------------------
//      Identifier PayContact(
//          const Identifier& senderNymID,
//          const Identifier& contactID,
//          std::unique_ptr<OTPayment>& payment) const = 0;
//      Identifier PayContactCash(
//         const Identifier& senderNymID,
//         const Identifier& contactID,
//         std::unique_ptr<Purse>& recipientCopy,
//         std::unique_ptr<Purse>& senderCopy) const = 0;

        const opentxs::String otstrCheque(strCheque.c_str());

        std::shared_ptr<const opentxs::OTPayment> pPayment
            (new opentxs::OTPayment(opentxs::OT::App().Legacy().ClientDataFolder(), otstrCheque));

        const auto bgthreadId
            {opentxs::OT::App().API().Sync().
                PayContact(opentxs::Identifier::Factory(str_fromNymId),
                           opentxs::Identifier::Factory(toContactId.toStdString()),
                           pPayment
                           )};

        // Instead of MessageContact we use PayContact, so that it does
        // a send_user_instrument instead of a send_user_message.
        //
        const auto status = opentxs::OT::App().API().Sync().Status(bgthreadId);

        const bool bAddToGUI = (opentxs::ThreadStatus::FINISHED_SUCCESS == status) ||
                               (opentxs::ThreadStatus::RUNNING == status);
        if (bAddToGUI) { // This means it's queued, not actually sent to the notary yet.
            //const bool bUseGrayText = (opentxs::ThreadStatus::FINISHED_SUCCESS != status);
            qDebug() << QString("Success in queueing %1 for sending.").arg(nsChequeType);
            return true;
        }
        else
        {
            qDebug() << QString("Failed trying to queue %1 for sending.").arg(nsChequeType);
        }
        return false;
    }

    // OLD CODE TO EXPORT INSTRUMENT TO A DIALOG FOR THE USER:
    // ------------------------------------------------------------
//    const QString qstrFromAccountName = ui->fromButton->text();
//    const QString qstrToContactName   = ui->toButton->text();
//    // ------------------------------------------------------------
//    opentxs::String otstr_cheque(strCheque.c_str());
//    opentxs::Armored asc_cheque(otstr_cheque);
//    opentxs::String strArmoredCheque; // TODO: Encrypt to contact's pubkey if available.
//    if (asc_cheque.WriteArmoredString(strArmoredCheque, "CHEQUE"))
//    {
//        const std::string str_armored_cheque(strArmoredCheque.Get());
//        const QString qstrPayment = QString::fromStdString(str_armored_cheque);
//
//        const QString qstrFrom = QString("%1 %2: %3").arg(qstrAmount).arg(tr("from account")).arg(qstrFromAccountName);
//        const QString qstrTo   = QString("%1: %2").arg(tr("To Contact")).arg(qstrToContactName);
//        // ------------------------------------------------------------
//        DlgExportedToPass * dlgExported = new DlgExportedToPass(NULL, qstrPayment,
//                                                                qstrFrom,
//                                                                qstrTo, false);
//        dlgExported->setAttribute(Qt::WA_DeleteOnClose);
//        // --------------------------------------------------
//        dlgExported->setWindowTitle(QString("%1: %2").arg(tr("Memo")).arg(note));
//        dlgExported->show();
//    }
//    // ------------------------------------------------------------
//    QMessageBox::StandardButton reply;
//
//    reply = QMessageBox::question(this, "", tr("The cheque has been written and placed in your outbox. Do you want to SEND IT now?"),
//                                  QMessageBox::Yes|QMessageBox::No);
//    if (reply == QMessageBox::No) {
//        return true;
//    }
//    // ------------------------------------------------------------
//    std::string  strResponse;
//    {
//        MTSpinner theSpinner;
//
//        std::unique_ptr<opentxs::OTPayment> payment =
//            std::make_unique<opentxs::OTPayment>(opentxs::String(strCheque.c_str()));
//
//        OT_ASSERT(payment);
//
//        auto action = opentxs::OT::App().API().ServerAction().SendPayment(fromNymID, notaryID, toNymID, payment);
//        strResponse = action->Run();
//    }
//
//    int32_t nReturnVal  = opentxs::VerifyMessageSuccess(strResponse);
//
//    if (1 != nReturnVal)
//    {
//        qDebug() << QString("send %1: failed.").arg(nsChequeType);
//
//        Moneychanger::It()->HasUsageCredits(str_NotaryID, str_fromNymId);
//    }
//    else
//    {
//        qDebug() << QString("Success in send %1!").arg(nsChequeType);
//        return true;
//    }
    // -------------------------------------
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
        std::string str_InstrumentDefinitionID(opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID(m_myAcctId.toStdString()));
        QString     amt = ui->amountEdit->text();

        if (!amt.isEmpty() && !str_InstrumentDefinitionID.empty())
        {
            std::string str_temp(amt.toStdString());

            if (std::string::npos == str_temp.find(".")) // not found
                str_temp += '.';

            int64_t     amount               = opentxs::OT::App().API().Exec().StringToAmount(str_InstrumentDefinitionID, str_temp);
            std::string str_formatted_amount = opentxs::OT::App().API().Exec().FormatAmount(str_InstrumentDefinitionID, static_cast<int64_t>(amount));
            QString     qstr_FinalAmount     = QString::fromStdString(str_formatted_amount);
            ui->amountEdit->setText(qstr_FinalAmount);
        }
    }
}

// ----------------------------------------------------------------------

void MTSendDlg::setInitialHisContact (QString contactId, bool bUsedInternally/*=false*/) // Payment To: (contact)
{
    canMessage_ = false;

    if (!lockPayee_)
    {
        ui->toButton->setEnabled(true);
        ui->toolButton->setEnabled(true);
        if (Moneychanger::It()->expertMode()) {
            ui->toolButton->setVisible(true);
        }
    }

    m_hisContactId = contactId;

    if (!m_myAcctId.isEmpty() && !m_hisContactId.isEmpty())
    {
        const std::string str_my_nym_id = opentxs::OT::App().API().Exec()
            .GetAccountWallet_NymID(m_myAcctId.toStdString());

        if (   !str_my_nym_id.empty()
            && (opentxs::Messagability::READY == opentxs::OT::App().API().Sync()
                .CanMessage(opentxs::Identifier::Factory(str_my_nym_id),
                            opentxs::Identifier::Factory(m_hisContactId.toStdString()))))
        {
            canMessage_ = true;

            if (!bUsedInternally)
                lockPayee_ = true;

            if (lockPayee_)
            {
                ui->toButton->setEnabled(false);
                ui->toolButton->setEnabled(false);
                ui->toolButton->setVisible(false);
            }

            const std::string str_contact_label = MTContactHandler::getInstance()->GetContactName(m_hisContactId.toStdString());
            ui->toButton->setText(QString::fromStdString(str_contact_label));
        }
    }
}

// ----------------------------------------------------------------------
bool MTSendDlg::sendFunds(QString memo, QString qstr_amount)
{
    QString & toNymId     = m_hisNymId;
    QString & toContactId = m_hisContactId;
    QString & fromAcctId  = m_myAcctId;
    // ----------------------------------------------------
    // Already done by caller function.
    //
//  if (toContactId.isEmpty() || toNymId.isEmpty())
//    if (toContactId.isEmpty())
//    {
//        if (!toNymId.isEmpty())
//        {
//            const opentxs::Identifier toContact_Id = opentxs::OT::App().Contact()
//            .ContactID(opentxs::Identifier{toNymId.toStdString()});
//            const opentxs::String strToContactId(toContact_Id);
//            toContactId = toContact_Id.empty() ? QString("") : QString::fromStdString(std::string(strToContactId.Get()));
//        }
//    }
    // ----------------------------------------------------
    if (toContactId.isEmpty())
    {
        qDebug() << "Cannot sent funds to an empty contact id, aborting.";
        // todo: someday we will allow this, by popping up a dialog so the
        // user can copy the cheque and send it out-of-band.
        //
        return false;
    }
    // ----------------------------------------------------
//    if (!inviteMode() && toNymId.isEmpty())
//    {
//        qDebug() << "Cannot send funds to an empty nym id, aborting.";
//        // todo: someday we will allow this, by leaving the payee blank on
//        // the cheque itself.
//        // NOTE: If INVITE MODE is turned ON, we would allow this.
//        return false;
//    }
    // ----------------------------------------------------
    if (fromAcctId.isEmpty())
    {
        qDebug() << "Cannot send funds from an empty acct id, aborting.";
        return false;
    }
    // ----------------------------------------------------
    if (memo.isEmpty())
        memo = tr("(Memo was empty.)");
    // ----------------------------------------------------
    if (qstr_amount.isEmpty())
        qstr_amount = QString("0");
    // ----------------------------------------------------
    int64_t     amount = 0;
    std::string str_InstrumentDefinitionID(opentxs::OT::App().API().Exec()
        .GetAccountWallet_InstrumentDefinitionID(fromAcctId.toStdString()));

    if (!str_InstrumentDefinitionID.empty())
    {
        std::string str_amount(qstr_amount.toStdString());

        if (std::string::npos == str_amount.find(".")) // not found
            str_amount += '.';

        amount = opentxs::OT::App().API().Exec().StringToAmount(str_InstrumentDefinitionID, str_amount);
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
        m_bSent = sendCashierCheque(amount, toNymId, toContactId, fromAcctId, memo);
        break;

    case (1): // Cheque
        qstrPaymentType = tr("cheque");
        m_bSent = sendCheque(amount, toNymId, toContactId, fromAcctId, memo);
        break;

    case (2): // Cash
        qstrPaymentType = tr("cash");
        m_bSent = sendCash(amount, toNymId, toContactId, fromAcctId, memo);
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
        QMessageBox::information(this, tr("Success"), QString("%1 (%2).").arg(tr("Success")).arg(qstrPaymentType));
    }
    // ---------------------------------------------------------
    return m_bSent;
}




void MTSendDlg::on_sendButton_clicked()
{
    // Send funds and then close dialog. Use progress bar.
    // -----------------------------------------------------------------
    // If the Contact ID is empty, but the NYM is set, we try to retrieve
    // the Contact ID using his Nym ID.
    //
    if (m_hisContactId.isEmpty())
    {
        if (!m_hisNymId.isEmpty())
        {
            const auto toContact_Id = opentxs::OT::App().Contact()
                .ContactID(opentxs::Identifier::Factory(m_hisNymId.toStdString()));
            const opentxs::String strToContactId(toContact_Id);
            m_hisContactId = toContact_Id->empty()
            ? QString("")
            : QString::fromStdString(std::string(strToContactId.Get()));
        }
    }
    // -----------------------------------------------------------------
    if (m_hisContactId.isEmpty()) // STILL empty?
    {
        // todo here:
        // We could just pop up the cheque in a dialog so the user, in that
        // case, can just copy/paste it out-of-band.

        QMessageBox::warning(this, tr("No Contact Selected"),
                             tr("You must select a contact for the payment to be sent to. "));
        return;
    }
    // todo: except in case of inviteMode_ of course. ^^^
    // -----------------------------------------------------------------
    // To:
    // NOTE: We may wish to allow this in the future. Just because I am
    // sending a cheque to a specific contact, doesn't mean I don't want to
    // send a cheque with the recipient set to blank. But for now, we'll
    // insist on a Nym to make the cheque out to.
    //
//    if (m_hisNymId.isEmpty())
//    {
//        QMessageBox::warning(this, tr("No Payee Selected"),
//                             tr("Please choose a payee for these funds, before sending."));
//        return;
//    }
    // NOTE: allowing this for now since a deeper function will already
    // lookup the NymID as long as the ContactID is set.
    // -----------------------------------------------------------------
    // From:
    if (m_myAcctId.isEmpty())
    {
        QMessageBox::warning(this, tr("No Payer Account Selected"),
                             tr("Please choose an account to pay from."));
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
    QString     qstr_fromNymId(QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_NymID(str_fromAcctId)));

    if (0 == qstr_fromNymId.compare(m_hisNymId))
    {
        QMessageBox::warning(this, tr("Cannot Send To Yourself"),
                             tr("Sorry, but you cannot send to yourself. "
                                "Please choose another recipient, or change "
                                "the sending account."));
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

    reply = QMessageBox::question(this, "", QString("%1 '%2'<br/>%3")
                                  .arg(tr("The amount is"))
                                  .arg(ui->amountEdit->text())
                                  .arg(tr("Continue?")),
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        // -----------------------------------------------------------------
        // Actually send the funds here.
        //
        QString memo   = ui->memoEdit  ->text();
        QString amount = ui->amountEdit->text();

        const bool bSent = this->sendFunds(memo, amount);
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

    for (const auto& [accountID, alias] : opentxs::OT::App().DB().AccountList())
    {
        //Get OT Acct ID
        QString OT_acct_id = QString::fromStdString(accountID);
        QString OT_acct_name("");
        // -----------------------------------------------
        if (!OT_acct_id.isEmpty())
        {
            if (!m_myAcctId.isEmpty() && (OT_acct_id == m_myAcctId))
                bFoundDefault = true;
            // -----------------------------------------------
            OT_acct_name = QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_Name(OT_acct_id.toStdString()));
            // -----------------------------------------------
            the_map.insert(OT_acct_id, OT_acct_name);
        }
     }
    // -----------------------------------------------
    if (bFoundDefault && !m_myAcctId.isEmpty())
        theChooser.SetPreSelected(m_myAcctId);
    // -----------------------------------------------
    theChooser.setWindowTitle(tr("Select your Payer Account"));
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
            ui->fromButton->setText(from_button_text);
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
    ui->fromButton->setText(tr("<Click to choose Account>"));

    // This might have been true before, but now we know there's no account selected,
    // meaning it's impossible to know if we "Can_Message" anymore, until we get sender
    // and recipient both set again, and we can check to answer that question.
    //
    if (canMessage_)
    {
        canMessage_ = false;

        if (!lockPayee_)
        {
            ui->toButton->setEnabled(true);
            ui->toolButton->setEnabled(true);
        }
    }
}

void MTSendDlg::on_toolButtonManageAccts_clicked()
{
    emit ShowAccount(m_myAcctId);
}

void MTSendDlg::on_toolButton_clicked()
{
    if (!m_hisContactId.isEmpty())
    {
        emit ShowContact(m_hisContactId);
        return;
    }
    // ------------------------------------------------
    if (!m_hisNymId.isEmpty())
    {
        const auto toContact_Id = opentxs::OT::App().Contact()
            .ContactID(opentxs::Identifier::Factory(m_hisNymId.toStdString()));
        const opentxs::String     strToContactId(toContact_Id);
        m_hisContactId = toContact_Id->empty() ? QString("") : QString::fromStdString(std::string(strToContactId.Get()));

        if (!m_hisContactId.isEmpty())
            emit ShowContact(m_hisContactId);
    }
}





void MTSendDlg::on_toButton_clicked()
{
    // Select recipient from the address book and convert to Nym ID.
    // -----------------------------------------------
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;
    MTContactHandler::getInstance()->GetOpentxsContacts(the_map);

//  QString m_hisContactId;  // To: (contact)
//  QString m_hisNymId;  // To: (nym)

    if (!m_hisContactId.isEmpty())
    {
        theChooser.SetPreSelected(m_hisContactId);
    }
    else if (!m_hisNymId.isEmpty())
    {
        const auto toContact_Id = opentxs::OT::App().Contact()
                .ContactID(opentxs::Identifier::Factory(m_hisNymId.toStdString()));
        const opentxs::String     strToContactId(toContact_Id);
        m_hisContactId = toContact_Id->empty()
            ? QString("")
            : QString::fromStdString(std::string(strToContactId.Get()));
        if (!m_hisContactId.isEmpty())
            theChooser.SetPreSelected(m_hisContactId);
    }
    // -----------------------------------------------
    theChooser.setWindowTitle(tr("Choose Payee"));
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
//        qDebug() << QString("SELECT was clicked for ID: %1").arg(theChooser.m_qstrCurrentID);

        // If not the same as before, then we have to choose a NymID based on the selected Contact.
        //
        QString qstrSelectedContact = theChooser.m_qstrCurrentID;

        // If they had matched, then we could have kept m_hisNymId as it was.
        // But since they are different, we have to figure out a NymID to use, based
        // on nSelectedContactID.
        //
        if (qstrSelectedContact != m_hisContactId)
        {
            QString qstrContactName;

            if (qstrSelectedContact.isEmpty()) // Should never happen.
            {
                qstrContactName  = QString("");
                m_hisNymId = QString("");
                m_hisContactId = QString("");
                canMessage_ = false;
                ui->toButton->setText(tr("<Click to choose Payee>"));
                return;
            }
            // else...
            //
            qstrContactName = theChooser.m_qstrCurrentName;

            if (qstrContactName.isEmpty())
                ui->toButton->setText(tr("(Contact has a blank name)"));
            else
                ui->toButton->setText(qstrContactName);
            // ---------------------------------------------
            // Next we need to find a Nym based on this Contact...
            //
            mapIDName theNymMap;

            if (MTContactHandler::getInstance()->GetNyms(theNymMap, qstrSelectedContact.toStdString()))
            {
                if (theNymMap.size() == 1)
                {
                    mapIDName::iterator theNymIt = theNymMap.begin();

                    if (theNymIt != theNymMap.end())
                    {
                        QString qstrNymID   = theNymIt.key();
//                      QString qstrNymName = theNymIt.value();
                        m_hisNymId = qstrNymID;
                        m_hisContactId = qstrSelectedContact;
                        ui->toButton->setText(qstrContactName);
                    }
                    else // should never happen
                    {
                        m_hisNymId = QString("");
                        m_hisContactId = QString("");
                        canMessage_ = false;
                        ui->toButton->setText(tr("<Click to choose payee>"));
                        // -------------------------------------
                        QMessageBox::warning(this, tr("Contact has no known identities"),
                                             tr("Sorry, Contact '%1' has no known NymIds (to pay funds to.)").arg(qstrContactName));
                        return;
                    }
                }
                else // There are multiple Nyms to choose from.
                {
                    DlgChooser theNymChooser(this);
                    theNymChooser.m_map = theNymMap;
                    theNymChooser.setWindowTitle(tr("Payee has multiple Nyms. (Please choose one.)"));
                    // -----------------------------------------------
                    if (theNymChooser.exec() == QDialog::Accepted)
                        m_hisNymId = theNymChooser.m_qstrCurrentID;
                    else // User must have canceled.
                    {
//                        m_hisNymId = QString("");
//                        m_hisContactId = QString("");
//                        canMessage_ = false;
//                        ui->fromButton->setText(tr("<Click to choose payee>"));
                    }
                }
            }
            else // No nyms found for this ContactID.
            {
                m_hisNymId = QString("");
                m_hisContactId = QString("");
                canMessage_ = false;
                ui->toButton->setText(tr("<Click to choose payee>"));
                // -------------------------------------
                QMessageBox::warning(this, tr("Contact has no known identities"),
                                     tr("Sorry, Contact '%1' has no known NymIDs (to pay funds to.)").arg(qstrContactName));
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




void MTSendDlg::dialog()
{
/** Send Funds Dialog **/

    if (!already_init)
    {
        if (!Moneychanger::It()->expertMode())
        {
            ui->comboBox->setVisible(false);
            ui->toolButton->setVisible(false);
            ui->toolButtonManageAccts->setVisible(false);
        }
        // ---------------------------------------
        connect(this,               SIGNAL(balancesChanged()),
                Moneychanger::It(), SLOT  (onBalancesChanged()));
        // ---------------------------------------
        this->setWindowTitle(tr("Pay Funds"));

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
            str_my_name = opentxs::OT::App().API().Exec().GetAccountWallet_Name(m_myAcctId.toStdString());

            if (str_my_name.empty())
                str_my_name = m_myAcctId.toStdString();
        }
        // -------------------------------------------
        if (str_my_name.empty())
        {
            m_myAcctId = QString("");
            ui->fromButton->setText(tr("<Click to Select Payer Account>"));
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
            str_his_name = opentxs::OT::App().API().Exec().GetNym_Name(m_hisNymId.toStdString());

            if (str_his_name.empty())
                str_his_name = m_hisNymId.toStdString();
        }
        // -------------------------------------------
        if (!lockPayee_)
        {
            if (str_his_name.empty())
            {
                m_hisNymId = QString("");
                ui->toButton->setText(tr("<Click to choose Payee>"));
            }
            else
                ui->toButton->setText(QString::fromStdString(str_his_name));
        }
        // -------------------------------------------
        if (!m_memo.isEmpty())
        {
            QString qstrTemp = m_memo;
            ui->memoEdit->setText(qstrTemp);
            // -----------------------
            this->setWindowTitle(QString("%1 %2").arg(tr("Pay Funds | Memo:")).arg(qstrTemp));
        }
        // -------------------------------------------



        // -------------------------------------------
        if (!m_amount.isEmpty())
        {
            QString qstrTemp = m_amount;
            ui->amountEdit->setText(m_amount);
        }
        // -------------------------------------------

        ui->comboBox->setCurrentIndex(1); // Cheque.

        ui->memoEdit->setFocus();

//        ui->toButton->setFocus();


        /** Flag Already Init **/
        already_init = true;
    }

    //show();
    Focuser f(this);
    f.show();
    f.focus();
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
        this->setWindowTitle(tr("Pay Funds"));
    }
    else
    {
        m_memo = arg1;
        this->setWindowTitle(tr("Pay Funds | Memo: %1").arg(arg1));
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
        return QWidget::eventFilter(obj, event);
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

