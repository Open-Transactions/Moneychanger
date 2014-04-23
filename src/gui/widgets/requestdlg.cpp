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

#include <opentxs/OTAPI.hpp>
#include <opentxs/OTAPI_Exec.hpp>
#include <opentxs/OT_ME.hpp>

#include <QDebug>
#include <QMessageBox>
#include <QKeyEvent>


// ----------------------------------------------------------------------
bool MTRequestDlg::sendInvoice(int64_t amount, QString toNymId, QString fromAcctId, QString note)
{
    return sendChequeLowLevel(amount, toNymId, fromAcctId, note, true); //isInvoice = true
}

// -------------------------------------------------------------------

bool MTRequestDlg::sendChequeLowLevel(int64_t amount, QString toNymId, QString fromAcctId, QString note, bool isInvoice)
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
        note = QString("From the Qt systray app.");
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

void MTRequestDlg::on_amountEdit_editingFinished()
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

bool MTRequestDlg::requestFunds(QString memo, QString qstr_amount)
{
    // Send INVOICE TO NYM, FROM ACCOUNT.
    //
    QString & toNymId     = m_hisNymId;
    QString & fromAcctId  = m_myAcctId;
    // ----------------------------------------------------
    if (toNymId.isEmpty())
    {
        qDebug() << "Cannot request funds from an empty nym id, aborting.";
        return false;
    }
    // ----------------------------------------------------
    if (fromAcctId.isEmpty())
    {
        qDebug() << "Cannot request funds to an empty acct id, aborting.";
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
        qDebug() << "Cannot request a negative or zero amount.";
        return false;
    }
    // ----------------------------------------------------
    m_bSent = sendInvoice(amount, toNymId, fromAcctId, memo);
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
    // From:
    if (m_hisNymId.isEmpty())
    {
        QMessageBox::warning(this, tr("No Invoicee"),
                             tr("Please choose an invoicee for the request."));
        return;
    }
    // -----------------------------------------------------------------
    // To:
    if (m_myAcctId.isEmpty())
    {
        QMessageBox::warning(this, tr("No Payee Account"),
                             tr("Please choose an account to receive the funds into."));
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

    on_amountEdit_editingFinished();

    // -----------------------------------------------------------------

    // TODO: We want an extra "ARE YOU SURE?" step to go right here, but likely it will
    // just be the passphrase dialog being FORCED to come up. But still, that means here
    // we'll have to set some kind of flag, probably, to force it to do that.
    //
    // NOTE: We'll want the "Are you sure" for the AMOUNT to display on that dialog.
    // (That is for security purposes.)

    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(this, "", QString("%1 '%2'<br/>%3").
                                  arg(tr("The amount is")).arg(ui->amountEdit->text()).arg(tr("Send Invoice?")),
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        // -----------------------------------------------------------------
        // Actually send the invoice here.
        //
        QString memo   = ui->memoEdit  ->text();
        QString amount = ui->amountEdit->text();

        bool bSent = this->requestFunds(memo, amount);
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
    theChooser.setWindowTitle(tr("Select your Payee Account"));
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
        qDebug() << QString("SELECT was clicked for AcctID: %1").arg(theChooser.m_qstrCurrentID);

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
            return;
        }
    }
    else
    {
      qDebug() << "CANCEL was clicked";
    }
    // -----------------------------------------------
    m_myAcctId = QString("");
    ui->toButton->setText(tr("<Click to choose Account>"));
}



void MTRequestDlg::on_toolButton_clicked()
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



void MTRequestDlg::on_toolButtonManageAccts_clicked()
{
    emit ShowAccount(m_myAcctId);
}


void MTRequestDlg::on_fromButton_clicked()
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
    theChooser.setWindowTitle(tr("Choose the Invoicee"));
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
                ui->fromButton->setText(tr("<Click to choose Invoicee>"));
                return;
            }
            // else...
            //
            qstrContactName = MTContactHandler::getInstance()->GetContactName(nSelectedContactID);

            if (qstrContactName.isEmpty())
                ui->fromButton->setText(tr("(Contact has a blank name)"));
            else
                ui->fromButton->setText(qstrContactName);
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
                        ui->fromButton->setText(tr("<Click to choose Invoicee>"));
                        // -------------------------------------
                        QMessageBox::warning(this, tr("Contact has no known identities"),
                                             tr("Sorry, Contact '%1' has no known NymIDs (to request funds from.)").arg(qstrContactName));
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
                    else // User must have cancelled.
                    {
                        m_hisNymId = QString("");
                        ui->fromButton->setText(tr("<Click to choose Invoicee>"));
                    }
                }
            }
            else // No nyms found for this ContactID.
            {
                m_hisNymId = QString("");
                ui->fromButton->setText(tr("<Click to choose Invoicee>"));
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
      qDebug() << "CANCEL was clicked";
    }
    // -----------------------------------------------
}




void MTRequestDlg::dialog()
{
/** Request Funds Dialog **/

    if (!already_init)
    {
        connect(this,               SIGNAL(balancesChanged()),
                Moneychanger::It(), SLOT  (onBalancesChanged()));
        // ---------------------------------------
        this->setWindowTitle(tr("Request Funds"));

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
            ui->toButton->setText(tr("<Click to Select Payee Account>"));
        }
        else
        {
            QString from_button_text = MTHome::FormDisplayLabelForAcctButton(m_myAcctId, QString::fromStdString(str_my_name));

            ui->toButton->setText(from_button_text);
        }
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
            ui->fromButton->setText(tr("<Click to choose Invoicee>"));
        }
        else
            ui->fromButton->setText(QString::fromStdString(str_his_name));
        // -------------------------------------------



        // -------------------------------------------
        if (!m_memo.isEmpty())
        {
            QString qstrTemp = m_memo;
            ui->memoEdit->setText(qstrTemp);
            // -----------------------
            this->setWindowTitle(QString("%1 %2").arg(tr("Request Funds | Memo:")).arg(qstrTemp));
        }
        // -------------------------------------------



        // -------------------------------------------
        if (!m_amount.isEmpty())
        {
            QString qstrTemp = m_amount;
            ui->amountEdit->setText(m_amount);
        }
        // -------------------------------------------


        ui->fromButton->setFocus();


        /** Flag Already Init **/
        already_init = true;
    }

    show();
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

    connect(this, SIGNAL(ShowContact(QString)), Moneychanger::It(), SLOT(mc_showcontact_slot(QString)));
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
        this->setWindowTitle(tr("Request Funds"));
    }
    else
    {
        m_memo = arg1;
        this->setWindowTitle(QString("%1 %2").arg(tr("Request Funds | Memo:")).arg(arg1));
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
        return QObject::eventFilter(obj, event);
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

