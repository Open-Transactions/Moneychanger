#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/requestdlg.hpp>
#include <ui_requestdlg.h>

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
// Note that on the request dialog, we are sending invoices.
// Therefore the "from" account is payee's account where funds will
// flow in the event that the invoice is processed by its recipient.
//
bool MTRequestDlg::sendInvoice(int64_t amount, QString toNymId, QString toContactId, QString fromAcctId, QString note)
{
    // This is already verified by the calling function.
//    if (fromAcctId.isEmpty())
//    {
//        QMessageBox::warning(this, tr("Missing Payee Account"),
//            tr("You must choose which of your accounts will receive the funds."));
//        return false;
//    }
    std::string str_fromAcctId(fromAcctId.toStdString());
    std::string str_fromNymId(opentxs::OT::App().API().Exec().GetAccountWallet_NymID(str_fromAcctId));
    // ------------------------------------------------------------
    // If there's a contact ID for recipient but no Nym ID, then we need to find
    // out the Nym ID, so we can WRITE him the cheque (invoice). (Unless we deliberately
    // leave the recipient blank on the invoice, so we can give it to anyone).
    //
    // If there's no Contact Id, then we need to find out
    // the Contact ID, so we can SEND him the cheque (invoice).
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
//                    .ContactID(opentxs::Identifier{toNymId.toStdString()});
//            const opentxs::String strToContactId(toContact_Id);
//            toContactId = toContact_Id.empty() ? QString("") : QString::fromStdString(std::string(strToContactId.Get()));
//        }
//    }
    // --------------------------------
    // Perhaps the payee wishes to leave the payer Nym blank, so that
    // ANYONE could pay this invoice, and not just a specific person.
    // Fair enough.
    //
    // But even so, he still needs a Contact ID or he won't have
    // anyone to SEND the message that contains the invoice!
    //
    // Therefore by this point, we MUST have a Contact Id, even if the NymID
    // remains blank. (Alternately, we could just pop up the invoice in a dialog
    // so the user, in that case, can just copy it and send out-of-band.)
    //
    if (toContactId.isEmpty())
    {
        qDebug() << "Cannot request funds from an empty contact id, aborting.";
        // todo: someday we will allow this, by popping up a dialog so the
        // user can copy the invoice and send it out-of-band.
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
            // the invoicee field blank. Only if the user says "No" do we then
            // do the below code and choose a Nym as the payer.

            mapIDName theNymMap;

            if (!MTContactHandler::getInstance()->GetNyms(theNymMap, toContactId.toStdString()))
            {
                // Again: could choose at this point just to leave the invoicee
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
    //              QString qstrNymName = theNymIt.value();
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
                              true, //isInvoice = true
                              toNymId.isEmpty()); // payerNymIsBlank
}

// -------------------------------------------------------------------

bool MTRequestDlg::sendChequeLowLevel(
    int64_t amount,
    QString toNymId,
    QString toContactId,
    QString fromAcctId,
    QString note,
    bool isInvoice,
    bool payerNymIsBlank)  // Meaning ANY Nym can pay this invoice.
{
    QString nsChequeType = isInvoice ? QString("invoice") : QString("cheque");
    // ------------------------------------------------------------
    if (toNymId.isEmpty() && !payerNymIsBlank)
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
        qDebug() << QString("Why invoice an amount of 0 (or less) units? Aborting send %1.").arg(nsChequeType);
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
        note = QString("Request for payment.");
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
//    qDebug() << QString("Sending %1:\n Server:'%2'\n Nym:'%3'\n Acct:'%4'\n ToNym:'%5'\n Amount:'%6'\n Note:'%7'").
//                arg(nsChequeType).arg(QString::fromStdString(str_NotaryID)).arg(QString::fromStdString(str_fromNymId)).
//                arg(fromAcctId).arg(toNymId).arg(SignedAmount).arg(note);
    // ------------------------------------------------------------
    time64_t tFrom = opentxs::OT::App().API().Exec().GetTime();
    time64_t tTo   = tFrom + DEFAULT_CHEQUE_EXPIRATION;
    // ------------------------------------------------------------
    const auto notaryID = opentxs::Identifier::Factory(str_NotaryID),
               nymID    = opentxs::Identifier::Factory(str_fromNymId);
    if (!opentxs::OT::App().API().ServerAction().GetTransactionNumbers(nymID, notaryID, 1)) {
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
    // ------------------------------------------------------------
    // NOTE: We do not retrieve the account files here, in the case of success.
    // That's because none of them have changed yet from this operation -- not
    // until the recipient processes the invoice.
    return false;
}

// ----------------------------------------------------------------------

void MTRequestDlg::on_amountEdit_editingFinished()
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

void MTRequestDlg::setInitialHisContact (QString contactId, bool bUsedInternally/*=false*/) // Payment From: (contact)
{
    canMessage_ = false;

    if (!lockInvoicee_)
    {
        ui->fromButton->setEnabled(true);
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
                lockInvoicee_ = true;

            if (lockInvoicee_)
            {
                ui->fromButton->setEnabled(false);
                ui->toolButton->setEnabled(false);
                ui->toolButton->setVisible(false);
            }

            const std::string str_contact_label = MTContactHandler::getInstance()->GetContactName(m_hisContactId.toStdString());
            ui->fromButton->setText(QString::fromStdString(str_contact_label));
        }
    }
}


bool MTRequestDlg::requestFunds(QString memo, QString qstr_amount)
{
    // Send invoice TO Contact, drawn on MY account.
    //
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
        qDebug() << "Cannot request funds from an empty contact id, aborting.";
        // todo: someday we will allow this, by popping up a dialog so the
        // user can copy the invoice and send it out-of-band.
        //
        return false;
    }
    // ----------------------------------------------------
    if (fromAcctId.isEmpty())
    {
        qDebug() << "Cannot request funds to an empty payee account; aborting.";
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

        amount = opentxs::OT::App().API().Exec()
            .StringToAmount(str_InstrumentDefinitionID, str_amount);
    }
    // ----------------------------------------------------
    if (amount <= 0)
    {
        qDebug() << "Cannot request a negative or zero amount.";
        return false;
    }
    // ----------------------------------------------------
    m_bSent = sendInvoice(amount, toNymId, toContactId, fromAcctId, memo);
    // ----------------------------------------------------
    if (!m_bSent)
    {
        qDebug() << "request funds: Failed.";
        QMessageBox::warning(this, tr("Failure"), tr("Failure trying to request payment"));
    }
    else
    {
        qDebug() << "Success in request funds! (Invoice sent.)";
        QMessageBox::information(this, tr("Success"), tr("Success sending invoice."));
    }
    // ---------------------------------------------------------
    return m_bSent;
}


void MTRequestDlg::on_requestButton_clicked()
{
    // Send invoice and then close dialog. Use progress bar.
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
    // --------------------------------
    // Perhaps the payee wishes to leave the payer Nym blank, so that
    // ANYONE could pay this invoice, and not just a specific person.
    // Fair enough.
    //
    // But even so, he still needs a Contact ID or he won't have
    // anyone to SEND the message that contains the invoice!
    //
    // Therefore by this point, we MUST have a Contact Id, even if the NymID
    // remains blank.
    //
    if (m_hisContactId.isEmpty()) // STILL empty?
    {
        // todo here:
        // We could just pop up the inoice in a dialog so the user, in that
        // case, can just copy/paste it out-of-band.

        QMessageBox::warning(this, tr("No Contact Selected"),
                             tr("You must select a contact for the invoice to be sent to. "
                                "(Even if the invoice is made out to a blank payer)."));
        return;
    }
    // -----------------------------------------------------------------
    // Explicitly allowing the payer's Nym ID to be empty, separate from whether
    // or not we have a Contact to send the invoice to.
    // This way, ANYONE could pay this invoice, not just the person it was
    // sent to.
    //
//    if (m_hisNymId.isEmpty())
//    {
//        QMessageBox::warning(this, tr("No Invoicee"),
//                             tr("Please choose an invoicee."));
//        return;
//    }
    // -----------------------------------------------------------------
    // Upon payment, funds to be received into my account:
    if (m_myAcctId.isEmpty())
    {
        QMessageBox::warning(this, tr("No Payee Account"),
                             tr("Please choose which of your accounts will receive the funds."));
        return;
    }
    // -----------------------------------------------------------------
    // Memo:
    if (ui->memoEdit->text().isEmpty())
    {
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, "", tr("The memo is blank. Are you sure you want to request?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No)
          return;
    }
    // -----------------------------------------------------------------
    // Amount:
    if (ui->amountEdit->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Amount is Empty"),
                             tr("Please enter the amount you wish to request."));
        return;
    }
    // -----------------------------------------------------------------
    // Make sure I'm not sending to myself (since that will fail...)
    //
    std::string str_fromAcctId(m_myAcctId.toStdString());
    QString     qstr_fromNymId(QString::fromStdString(
        opentxs::OT::App().API().Exec().GetAccountWallet_NymID(str_fromAcctId)));

    if (0 == qstr_fromNymId.compare(m_hisNymId))
    {
        QMessageBox::warning(this, tr("Cannot Invoice Yourself"),
                             tr("Sorry, but you cannot invoice yourself. "
                                "Please choose another invoicee, or change "
                                "the payee account."));
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

    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(this, "", QString("%1 '%2'<br/>%3")
                                  .arg(tr("The amount is"))
                                  .arg(ui->amountEdit->text())
                                  .arg(tr("Send Invoice?")),
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        // -----------------------------------------------------------------
        // Actually send the invoice here.
        //
        QString memo   = ui->memoEdit  ->text();
        QString amount = ui->amountEdit->text();

        const bool bSent = this->requestFunds(memo, amount);
        // -----------------------------------------------------------------
        if (bSent)
            emit balancesChanged();
        // -----------------------------------------------------------------
    }
}



void MTRequestDlg::onBalancesChanged()
{
    this->close();
}


void MTRequestDlg::on_toButton_clicked()
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
    theChooser.setWindowTitle(tr("Select your Payee Account"));
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
//      qDebug() << QString("SELECT was clicked for AcctID: %1").arg(theChooser.m_qstrCurrentID);

        if (!theChooser.m_qstrCurrentID.isEmpty())
        {
            QString display_name("");
            QString button_text("");
            // -----------------------------------------
            m_myAcctId = theChooser.m_qstrCurrentID;
            // -----------------------------------------
            if (theChooser.m_qstrCurrentName.isEmpty())
                display_name = QString("");
            else
                display_name = theChooser.m_qstrCurrentName;
            // -----------------------------------------
            button_text = MTHome::FormDisplayLabelForAcctButton(m_myAcctId, display_name);
            // -----------------------------------------
            ui->toButton->setText(button_text);
            // -----------------------------------------
            if (!m_myAcctId.isEmpty() && !m_hisContactId.isEmpty())
            {
                setInitialHisContact(m_hisContactId, true);
            }
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
    ui->toButton->setText(tr("<Click to choose Account>"));

    // This might have been true before, but now we know there's no account selected,
    // meaning it's impossible to know if we "Can_Message" anymore, until we get sender
    // and recipient both set again, and we can check to answer that question.
    //
    if (canMessage_)
    {
        canMessage_ = false;

        if (!lockInvoicee_)
        {
            ui->fromButton->setEnabled(true);
            ui->toolButton->setEnabled(true);
        }
    }
}


void MTRequestDlg::on_toolButtonManageAccts_clicked()
{
    emit ShowAccount(m_myAcctId);
}


void MTRequestDlg::on_toolButton_clicked()
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

void MTRequestDlg::on_fromButton_clicked()
{
    // Select recipient from the address book and convert to Nym ID.
    // -----------------------------------------------
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;
    MTContactHandler::getInstance()->GetOpentxsContacts(the_map);

//  QString m_hisContactId;  // From: (contact)
//  QString m_hisNymId;  // From: (nym)

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
    theChooser.setWindowTitle(tr("Choose invoicee"));
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
//      qDebug() << QString("SELECT was clicked for ID: %1").arg(theChooser.m_qstrCurrentID);

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
                ui->fromButton->setText(tr("<Click to choose invoicee>"));
                return;
            }
            // else...
            //
            qstrContactName = theChooser.m_qstrCurrentName;

            if (qstrContactName.isEmpty())
                ui->fromButton->setText(tr("(Contact has a blank name)"));
            else
                ui->fromButton->setText(qstrContactName);
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
                        ui->fromButton->setText(qstrContactName);
                    }
                    else // should never happen
                    {
                        m_hisNymId = QString("");
                        m_hisContactId = QString("");
                        canMessage_ = false;
                        ui->fromButton->setText(tr("<Click to choose invoicee>"));
                        // -------------------------------------
                        QMessageBox::warning(this, tr("Contact has no known identities"),
                                             tr("Sorry, Contact '%1' has no known NymIds (to request funds from.)").arg(qstrContactName));
                        return;
                    }
                }
                else // There are multiple Nyms to choose from.
                {
                    DlgChooser theNymChooser(this);
                    theNymChooser.m_map = theNymMap;
                    theNymChooser.setWindowTitle(tr("Invoicee has multiple Nyms. (Please choose one.)"));
                    // -----------------------------------------------
                    if (theNymChooser.exec() == QDialog::Accepted)
                        m_hisNymId = theNymChooser.m_qstrCurrentID;
                    else // User must have canceled.
                    {
//                        m_hisNymId = QString("");
//                        m_hisContactId = QString("");
//                        canMessage_ = false;
//                        ui->fromButton->setText(tr("<Click to choose invoicee>"));
                    }
                }
            }
            else // No nyms found for this ContactID.
            {
                m_hisNymId = QString("");
                m_hisContactId = QString("");
                canMessage_ = false;
                ui->fromButton->setText(tr("<Click to choose invoicee>"));
                // -------------------------------------
                QMessageBox::warning(this, tr("Contact has no known identities"),
                                     tr("Sorry, Contact '%1' has no known NymIDs (to request funds from.)").arg(qstrContactName));
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




void MTRequestDlg::dialog()
{
/** Request Funds Dialog **/

    if (!already_init)
    {
        if (!Moneychanger::It()->expertMode())
        {
            ui->toolButton->setVisible(false);
            ui->toolButtonManageAccts->setVisible(false);
        }
        // ---------------------------------------
        connect(this,               SIGNAL(balancesChanged()),
                Moneychanger::It(), SLOT  (onBalancesChanged()));
        // ---------------------------------------
        this->setWindowTitle(tr("Invoice (Request Funds)"));

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
            ui->toButton->setText(tr("<Click to Select Payee Account>"));
        }
        else
        {
            QString to_button_text = MTHome::FormDisplayLabelForAcctButton(m_myAcctId,
                                        QString::fromStdString(str_my_name));

            ui->toButton->setText(to_button_text);
        }
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
        if (!lockInvoicee_)
        {
            if (str_his_name.empty())
            {
                m_hisNymId = QString("");
                ui->fromButton->setText(tr("<Click to choose Invoicee>"));
            }
            else
                ui->fromButton->setText(QString::fromStdString(str_his_name));
        }
        // -------------------------------------------
        if (!m_memo.isEmpty())
        {
            QString qstrTemp = m_memo;
            ui->memoEdit->setText(qstrTemp);
            // -----------------------
            this->setWindowTitle(QString("%1 %2").arg(tr("Invoice (Request Funds) | Memo:")).arg(qstrTemp));
        }
        // -------------------------------------------



        // -------------------------------------------
        if (!m_amount.isEmpty())
        {
            QString qstrTemp = m_amount;
            ui->amountEdit->setText(m_amount);
        }
        // -------------------------------------------
        ui->memoEdit->setFocus();

        /** Flag Already Init **/
        already_init = true;
    }

    Focuser f(this);
    f.show();
    f.focus();
}





MTRequestDlg::MTRequestDlg(QWidget *parent) :
    QWidget(parent, Qt::Window),
    m_bSent(false),
    already_init(false),
    ui(new Ui::MTRequestDlg)
{
    ui->setupUi(this);

    this->installEventFilter(this);

    connect(this, SIGNAL(balancesChanged()), this, SLOT(onBalancesChanged()));

    connect(this, SIGNAL(ShowContact(QString)), Moneychanger::It(), SLOT(mc_show_opentxs_contact_slot(QString)));
    connect(this, SIGNAL(ShowAccount(QString)), Moneychanger::It(), SLOT(mc_show_account_slot(QString)));
}

MTRequestDlg::~MTRequestDlg()
{
    delete ui;
}


void MTRequestDlg::on_memoEdit_textChanged(const QString &arg1)
{
    if (arg1.isEmpty())
    {
        m_memo = QString("");
        this->setWindowTitle(tr("Invoice (Request Funds)"));
    }
    else
    {
        m_memo = arg1;
        this->setWindowTitle(QString("%1 %2").arg(tr("Invoice (Request Funds) | Memo:")).arg(arg1));
    }
}


bool MTRequestDlg::eventFilter(QObject *obj, QEvent *event){

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

void MTRequestDlg::closeEvent(QCloseEvent *event)
{
    // Pop up a Yes/No dialog to confirm the cancellation of this payment.
    // (ONLY if the memo field contains text.)
    //
    if (!m_bSent && !ui->memoEdit->text().isEmpty())
    {
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, "", tr("Close without requesting funds?"),
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

