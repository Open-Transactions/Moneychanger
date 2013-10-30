
#include <QObject>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QGridLayout>
#include <QToolButton>
#include <QIcon>
#include <QPixmap>
#include <QDateTime>
#include <QLabel>
#include <QString>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>
#include <opentxs/OTLog.h>

#include "homedetail.h"
#include "ui_homedetail.h"

#include "compose.h"
#include "overridecursor.h"

#include "home.h"

#include "UI/getstringdialog.h"
#include "Widgets/dlgchooser.h"

#include "Handlers/contacthandler.h"

#include "moneychanger.h"


void MTHomeDetail::SetHomePointer(MTHome & theHome)
{
    m_pHome = &theHome;
}



MTHomeDetail::MTHomeDetail(QWidget *parent) :
    QWidget(parent),
    m_nContactID(0),
    m_pDetailLayout(NULL),
    m_pHome(NULL),
    m_pLineEdit_Nym_ID(NULL),
    m_pLineEdit_OtherNym_ID(NULL),
    m_pLineEdit_Acct_ID(NULL),
    m_pLineEdit_OtherAcct_ID(NULL),
    m_pLineEdit_Server_ID(NULL),
    m_pLineEdit_AssetType_ID(NULL),
    m_pLineEdit_Nym_Name(NULL),
    m_pLineEdit_OtherNym_Name(NULL),
    m_pLineEdit_Acct_Name(NULL),
    m_pLineEdit_OtherAcct_Name(NULL),
    m_pLineEdit_Server_Name(NULL),
    m_pLineEdit_AssetType_Name(NULL),
    ui(new Ui::MTHomeDetail)
{
    ui->setupUi(this);

    this->installEventFilter(this);

    this->setContentsMargins(0, 0, 0, 0);
}

MTHomeDetail::~MTHomeDetail()
{
    // --------------------------------------------------
    if (NULL != m_pDetailLayout)
    {
//      this->clearLayout(m_pDetailLayout);

        // The layout will already be cleared when it gets deleted.
        // Therefore we only use clearLayout when replacing it with
        // a new layout -- we don't have to clear it on destruction.
        // (It already clears itself in that case.)

        delete m_pDetailLayout;
        m_pDetailLayout = NULL;
    }
    // --------------------------------------------------
    delete ui;
}

void MTHomeDetail::FavorLeftSideForIDs()
{
    if (NULL != m_pLineEdit_Nym_ID)         m_pLineEdit_Nym_ID->home(false);
    if (NULL != m_pLineEdit_OtherNym_ID)    m_pLineEdit_OtherNym_ID->home(false);
    if (NULL != m_pLineEdit_Acct_ID)        m_pLineEdit_Acct_ID->home(false);
    if (NULL != m_pLineEdit_OtherAcct_ID)   m_pLineEdit_OtherAcct_ID->home(false);
    if (NULL != m_pLineEdit_Server_ID)      m_pLineEdit_Server_ID->home(false);
    if (NULL != m_pLineEdit_AssetType_ID)   m_pLineEdit_AssetType_ID->home(false);
    // --------------------------------------------------------------------
    if (NULL != m_pLineEdit_Nym_Name)       m_pLineEdit_Nym_Name->home(false);
    if (NULL != m_pLineEdit_OtherNym_Name)  m_pLineEdit_OtherNym_Name->home(false);
    if (NULL != m_pLineEdit_Acct_Name)      m_pLineEdit_Acct_Name->home(false);
    if (NULL != m_pLineEdit_OtherAcct_Name) m_pLineEdit_OtherAcct_Name->home(false);
    if (NULL != m_pLineEdit_Server_Name)    m_pLineEdit_Server_Name->home(false);
    if (NULL != m_pLineEdit_AssetType_Name) m_pLineEdit_AssetType_Name->home(false);
}

//if (QResizeEvent *resizeEvent = static_cast<QResizeEvent*>(event))
//somesize.setWidth(width);

bool MTHomeDetail::eventFilter(QObject *obj, QEvent *event)
{
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
    else if (event->type() == QEvent::Resize)
    {
        // This insures that the left-most part of the IDs and Names
        // remains visible during all resize events.
        //
        FavorLeftSideForIDs();
    }
    else
    {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}


void MTHomeDetail::on_viewContactButton_clicked(bool checked /*=false*/)
{
    qDebug() << "View Existing Contact button clicked.";

    if (m_record && (NULL != m_pHome) && (m_nContactID > 0))
        ((Moneychanger *)(m_pHome->parentWidget()))->mc_addressbook_show(QString("%1").arg(m_nContactID));
}

void MTHomeDetail::on_addContactButton_clicked(bool checked /*=false*/)
{
    qDebug() << "Add New Contact button clicked.";

    if (m_record)
    {
        MTRecord & recordmt = *m_record;
        // --------------------------------------------------
        MTGetStringDialog nameDlg(this);

        if (QDialog::Accepted == nameDlg.exec())
        {
            QString strNewContactName = nameDlg.GetOutputString();
            // --------------------------------------------------
            const std::string str_acct_id    = recordmt.GetOtherAccountID();
            const std::string str_nym_id     = recordmt.GetOtherNymID();
            const std::string str_server_id  = recordmt.GetServerID();
            const std::string str_asset_id   = recordmt.GetAssetID();
            // --------------------------------------------------
            int nContactID = 0;

            if (!str_nym_id.empty())
            {
                QString nymID     = QString::fromStdString(str_nym_id);
                QString serverID  = QString::fromStdString(str_server_id);
                QString assetID   = QString::fromStdString(str_asset_id);
                QString accountID = QString::fromStdString(str_acct_id);
                // --------------------------------------------------
                nContactID = MTContactHandler::getInstance()->CreateContactBasedOnNym(nymID, serverID);
                // --------------------------------------------------
                if (!str_acct_id.empty())
                {
                    int nAcctContactID = MTContactHandler::getInstance()->FindContactIDByAcctID(accountID, nymID, serverID, assetID);

                    if (!(nContactID > 0))
                        nContactID = nAcctContactID;
                }
                else if (!str_server_id.empty())
                    MTContactHandler::getInstance()->NotifyOfNymServerPair(nymID, serverID);
                // --------------------------------------------------
                if (nContactID > 0)
                {
                    MTContactHandler::getInstance()->SetContactName(nContactID, strNewContactName);
                    // ---------------------------------
                    m_nContactID = nContactID;
                    // ---------------------------------
                    // Refresh the detail page.
                    //
                    refresh(recordmt);
                    // ---------------------------------
                    // Display the normal contacts dialog, with the new contact
                    // being the one selected.
                    //
                    if (NULL != m_pHome)
                    {
                        m_pHome->SetNeedRefresh();

                        ((Moneychanger *)(m_pHome->parentWidget()))->mc_addressbook_show(QString("%1").arg(m_nContactID));
                    }
                }
            }
            else
                qDebug() << "Warning: Failed adding a contact, since there was no NymID for this record.";
        } // accepted. ("OK" clicked on dialog.)
        // --------------------------------------------------
    }
}

void MTHomeDetail::on_existingContactButton_clicked(bool checked /*=false*/)
{
    qDebug() << "Add to Existing Contact button clicked.";

    if (m_record)
    {
        MTRecord & recordmt = *m_record;

        const std::string str_acct_id    = recordmt.GetOtherAccountID();
        const std::string str_nym_id     = recordmt.GetOtherNymID();
        const std::string str_server_id  = recordmt.GetServerID();
        const std::string str_asset_id   = recordmt.GetAssetID();
        // --------------------------------------------------
        if (str_nym_id.empty())
        {
            QMessageBox::warning(this, QString("Failure"), QString("Sorry, but this record has no NymID."));
            return;
        }
        // else...
        //
        QString nymID     = QString::fromStdString(str_nym_id);
        QString serverID  = QString::fromStdString(str_server_id);
        QString assetID   = QString::fromStdString(str_asset_id);
        QString accountID = QString::fromStdString(str_acct_id);

        if (MTContactHandler::getInstance()->ContactExists(nymID.toInt()))
        {
            QMessageBox::warning(this, QString("Strange"),
                                 QString("Strange: NymID %1 already belongs to an existing contact.").arg(nymID));
            return;
        }
        // --------------------------------------------------------------------
        // Pop up a Contact selection box. The user chooses an existing contact.
        // If OK (vs Cancel) then add the Nym / Acct to the existing contact selected.
        //
        DlgChooser theChooser(this);
        // -----------------------------------------------
        mapIDName & the_map = theChooser.m_map;
        MTContactHandler::getInstance()->GetContacts(the_map);
        // -----------------------------------------------
        theChooser.setWindowTitle("Choose an Existing Contact");
        // -----------------------------------------------
        if (theChooser.exec() == QDialog::Accepted)
        {
            QString strContactID = theChooser.GetCurrentID();

            qDebug() << QString("SELECT was clicked for ID: %1").arg(strContactID);

            int nContactID = strContactID.isEmpty() ? 0 : strContactID.toInt();

            if (nContactID > 0)
            {
                bool bAdded = MTContactHandler::getInstance()->AddNymToExistingContact(nContactID, nymID);

                if (!bAdded)
                {
                    QString strContactName(MTContactHandler::getInstance()->GetContactName(nContactID));
                    QMessageBox::warning(this, QString("Failure"), QString("Failed while trying to add NymID %1 to existing contact '%2' with contact ID: %3").
                                         arg(nymID).arg(strContactName).arg(nContactID));
                    return;
                }
                // --------------------------------------
                // else...
                //
                if (!str_acct_id.empty())
                    //int nAcctContactID =
                    MTContactHandler::getInstance()->FindContactIDByAcctID(accountID, nymID, serverID, assetID);
                else if (!str_server_id.empty())
                    MTContactHandler::getInstance()->NotifyOfNymServerPair(nymID, serverID);
                // --------------------------------------------------
                m_nContactID = nContactID;
                // ---------------------------------
                // Refresh the detail page.
                //
                refresh(recordmt);
                // ---------------------------------
                // Display the normal contacts dialog, with the new contact
                // being the one selected.
                //
                if (NULL != m_pHome)
                {
                    m_pHome->SetNeedRefresh();

                    ((Moneychanger *)(m_pHome->parentWidget()))->mc_addressbook_show(QString("%1").arg(m_nContactID));
                }
                // ---------------------------------
            } // nContactID > 0
        }
        else
        {
          qDebug() << "CANCEL was clicked";
        }
    }
}


void MTHomeDetail::on_deleteButton_clicked(bool checked /*=false*/)
{
    qDebug() << "Delete button clicked.";
    // --------------------------------
    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(this, "", "Are you sure you want to archive this record?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::No)
      return;
    // --------------------------------
    if (m_record)
    {
        MTRecord & recordmt = *m_record;
        // -----------------------------------
        bool bSuccess = recordmt.DeleteRecord();

        if (bSuccess)
        {
            if (NULL != m_pHome)
                m_pHome->OnDeletedRecord();
            else
                qDebug() << QString("Error: m_pHome was NULL.");
        }
    }
}



QString MTHomeDetail::FindAppropriateDepositAccount(MTRecord & recordmt)
{
    // -----------------------------------------
    const std::string str_record_asset  = recordmt.GetAssetID();
    const std::string str_record_nym    = recordmt.GetNymID();
    const std::string str_record_server = recordmt.GetServerID();
    // -----------------------------------------
    QString qstr_record_asset  = QString::fromStdString(str_record_asset);
    QString qstr_record_nym    = QString::fromStdString(str_record_nym);
    QString qstr_record_server = QString::fromStdString(str_record_server);
    // -----------------------------------------
    std::string str_acct_id;
    std::string str_acct_nym;
    std::string str_acct_server;
    std::string str_acct_asset;

    QString qstr_acct_id,
            qstr_acct_nym,
            qstr_acct_server,
            qstr_acct_asset;

    if (NULL != m_pHome)
        qstr_acct_id = ((Moneychanger *)(m_pHome->parentWidget()))->get_default_account_id();
    // -----------------------------------
    // If there's a default account, and it has the same asset ID
    // as the record (and server ID and NymID) then accept the instrument
    // using the default account.
    //
    // BUT -- if the default account has the wrong asset ID, or
    // if there is no default account, then we need to pop up a
    // list of accounts owned by the same Nym, and filtered by
    // the server and asset ID from this record.
    //
    // We'll also need some sort of option to create an account
    // on the spot, if at least one doesn't exist matching that
    // criteria.
    //
    if (!qstr_acct_id.isEmpty()) // There IS a default account...
    {
        // Need to make sure the default account has the right server ID, NymID,
        // asset ID, to accept the current record. Otherwise we need to ask the
        // user to choose an account (the same as if there had been no default
        // account in the first place.)
        // -----------------------------------
        str_acct_id      = qstr_acct_id.toStdString();
        str_acct_nym     = OTAPI_Wrap::It()->GetAccountWallet_NymID      (str_acct_id);
        str_acct_server  = OTAPI_Wrap::It()->GetAccountWallet_ServerID   (str_acct_id);
        str_acct_asset   = OTAPI_Wrap::It()->GetAccountWallet_AssetTypeID(str_acct_id);
        // -----------------------------------
        qstr_acct_nym    = QString::fromStdString(str_acct_nym);
        qstr_acct_server = QString::fromStdString(str_acct_server);
        qstr_acct_asset  = QString::fromStdString(str_acct_asset);
        // -----------------------------------
        if ((qstr_record_nym    != qstr_acct_nym)    ||
            (qstr_record_server != qstr_acct_server) ||
            (qstr_record_asset  != qstr_acct_asset) )
        {
            // There's a default account, but it's got the wrong Asset type, or the
            // wrong server, etc, for it to accept this record. Therefore we have to
            // ask the user to select an account, just the same as if no default account
            // had been set at all.
            //
            str_acct_id      = "";
            str_acct_nym     = "";
            str_acct_server  = "";
            str_acct_asset   = "";
            // ------------------
            qstr_acct_id     = QString("");
            qstr_acct_nym    = QString("");
            qstr_acct_server = QString("");
            qstr_acct_asset  = QString("");
            // ------------------
        }
    }
    // -----------------------------------
    // By this point, there's definitely no default account, or if it
    // was, its IDs were all wrong. Therefore we have to ask the user
    // to choose an account by hand.
    //
    if (qstr_acct_id.isEmpty())
    {
        DlgChooser theChooser(this);
        // -----------------------------------------------
        mapIDName & the_map = theChooser.m_map;
        // -----------------------------------------------
        const int32_t acct_count = OTAPI_Wrap::GetAccountCount();
        // -----------------------------------------------
        for (int32_t ii = 0; ii < acct_count; ++ii)
        {
            //Get OT Acct ID
            QString OT_acct_id = QString::fromStdString(OTAPI_Wrap::GetAccountWallet_ID(ii));
            QString OT_acct_name("");
            // -----------------------------------------------
            if (!OT_acct_id.isEmpty()) // Should never be empty.
            {
                qstr_acct_id     = OT_acct_id;
                // -----------------------------------
                str_acct_id      = qstr_acct_id.toStdString();
                str_acct_nym     = OTAPI_Wrap::It()->GetAccountWallet_NymID      (str_acct_id);
                str_acct_server  = OTAPI_Wrap::It()->GetAccountWallet_ServerID   (str_acct_id);
                str_acct_asset   = OTAPI_Wrap::It()->GetAccountWallet_AssetTypeID(str_acct_id);
                // -----------------------------------
                qstr_acct_nym    = QString::fromStdString(str_acct_nym);
                qstr_acct_server = QString::fromStdString(str_acct_server);
                qstr_acct_asset  = QString::fromStdString(str_acct_asset);
                // -----------------------------------------------
                if ((qstr_record_nym    == qstr_acct_nym)    &&
                    (qstr_record_server == qstr_acct_server) &&
                    (qstr_record_asset  == qstr_acct_asset) )
                {
                    MTNameLookupQT theLookup;

                    OT_acct_name = QString::fromStdString(theLookup.GetAcctName(OT_acct_id.toStdString()));
                    // -----------------------------------------------
                    the_map.insert(OT_acct_id, OT_acct_name);
                }
                // -----------------------------------------------
                str_acct_id      = "";
                str_acct_nym     = "";
                str_acct_server  = "";
                str_acct_asset   = "";
                // ------------------
                qstr_acct_id     = QString("");
                qstr_acct_nym    = QString("");
                qstr_acct_server = QString("");
                qstr_acct_asset  = QString("");
                // -----------------------------------------------
            }
        } // for
        // -----------------------------------------------
        // At this point, the_map contains a list of accounts that could be
        // used to accept the record. At this point we could just pop up the
        // chooser and let the user pick one of those accounts.
        //
        // BUT -- what if the list is empty? In that case, the user doesn't
        // have an appropriate account. In which case one needs to be created...
        //
        //
        if (0 == the_map.size())
        {
            QMessageBox::warning(this, QString("No Matching Accounts"),
                                 QString("There are no existing accounts with the proper asset type, server, and nym. "
                                         "In the future, this is where you would be given the option to create "
                                         "one. (Someday.) In the meantime, just create one and then try again."));
            return QString("");
        }
        else if (1 == the_map.size())
        {
            // If there's only one account that could be appropriate, then we just
            // go with that account.
            //
            mapIDName::iterator it_map = the_map.begin();
            qstr_acct_id = it_map.key();
        }
        else // There are multiple matching accounts, so we ask the user to choose one.
        {
            // -----------------------------------------------
            theChooser.setWindowTitle("Select an Asset Account");
            // -----------------------------------------------
            if (theChooser.exec() == QDialog::Accepted)
            {
                qDebug() << QString("SELECT was clicked for AcctID: %1").arg(theChooser.m_qstrCurrentID);

                if (!theChooser.m_qstrCurrentID.isEmpty())
                    qstr_acct_id = theChooser.m_qstrCurrentID;
            }
            // (else the user cancelled.)
        }
    }
    // ----------------------------------
    return qstr_acct_id;
}




void MTHomeDetail::on_acceptButton_clicked(bool checked /*=false*/)
{
    qDebug() << "Accept button clicked.";

    if (m_record)
    {
        MTRecord & recordmt = *m_record;
        // -------------------------------------------------
        const bool bIsTransfer = (recordmt.GetRecordType() == MTRecord::Transfer);
        const bool bIsReceipt  = (recordmt.GetRecordType() == MTRecord::Receipt);
        // -------------------------------------------------
        if (bIsTransfer)
        {
            bool bSuccess = false;
            {
                MTOverrideCursor theSpinner;
                // -----------------------------------------
                bSuccess = recordmt.AcceptIncomingTransfer();
            }
            // -----------------------------------------
            if (!bSuccess)
            {
                QMessageBox::warning(this, QString("Transaction failure"), QString("Failed accepting this transfer."));
            }
            else
            {
                // Refresh the main list, or at least change the color of the refresh button.
                //
                if (NULL != m_pHome)
                    m_pHome->SetNeedRefresh();
            }
        }
        // -------------------------------------------------
        else if (bIsReceipt)
        {
            bool bSuccess = false;
            {
                MTOverrideCursor theSpinner;
                // -----------------------------------------
                bSuccess = recordmt.AcceptIncomingReceipt();
            }
            // -----------------------------------------
            if (!bSuccess)
            {
                QMessageBox::warning(this, QString("Transaction failure"), QString("Failed accepting this receipt."));
            }
            else
            {
                // Refresh the main list, or at least change the color of the refresh button.
                //
                if (NULL != m_pHome)
                    m_pHome->SetNeedRefresh();
            }
        }
        // -------------------------------------------------
        else if (MTRecord::Instrument == recordmt.GetRecordType())
        {
            // --------------------------------
            if (recordmt.IsInvoice())
            {
                QMessageBox::StandardButton reply;

                reply = QMessageBox::question(this, "", "Are you sure you want to pay this invoice?",
                                              QMessageBox::Yes|QMessageBox::No);
                if (reply == QMessageBox::No)
                    return;
            }
            // --------------------------------
            QString qstr_acct_id = FindAppropriateDepositAccount(recordmt);
            // -----------------------------------
            // At this point we likely have the account ID, and if so, it's definitely
            // got the right asset type, server, etc. Therefore it's time to accept the
            // instrument.
            //
            if (!qstr_acct_id.isEmpty())
            {
                bool bSuccess = false;
                {
                    MTOverrideCursor theSpinner;
                    // -----------------------------------------
                    bSuccess = recordmt.AcceptIncomingInstrument(qstr_acct_id.toStdString());
                }
                // -----------------------------------------
                if (!bSuccess)
                {
                    QMessageBox::warning(this, QString("Transaction failure"), QString("Failed accepting this instrument."));
                }
                else
                {
                    // Refresh the main list, or at least change the color of the refresh button.
                    //
                    if (NULL != m_pHome)
                        m_pHome->SetNeedRefresh();
                }
            }
        } // record type == instrument.
        // -------------------------------------------------
    }
}


void MTHomeDetail::on_cancelButton_clicked(bool checked /*=false*/)
{
    qDebug() << "Cancel button clicked.";

    if (m_record)
    {
        MTRecord & recordmt = *m_record;
        // ---------------------------------------
        if (recordmt.IsCash())
        {
            QMessageBox::StandardButton reply;

            reply = QMessageBox::question(this, "",
                                          "This will prevent the original recipient from depositing the cash. "
                                          "(And FYI, this action will fail, if he has already deposited it.) "
                                          "Are you sure you want to recover this cash?",
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::No)
                return;
            // -----------------------------------
            QString qstr_acct_id = FindAppropriateDepositAccount(recordmt);
            // -----------------------------------
            // At this point we likely have the account ID, and if so, it's definitely
            // got the right asset type, server, etc. Therefore it's time to accept the
            // instrument.
            //
            if (!qstr_acct_id.isEmpty())
            {
                bool bSuccess = false;
                {
                    MTOverrideCursor theSpinner;
                    // -----------------------------------------
                    OT_ME madeEasy;

                    if (1 == madeEasy.deposit_cash(recordmt.GetServerID(), recordmt.GetNymID(),
                                                   qstr_acct_id.toStdString(), recordmt.GetContents()))
                    {
                        bSuccess = true;
                        recordmt.DiscardOutgoingCash();
                    }
                }
                // -----------------------------------------
                if (!bSuccess)
                {
                    QMessageBox::warning(this, QString("Recovery failure"),
                                         QString("Failed recovering this outgoing cash. "
                                                 "(Perhaps the recipient already deposited it?)"));
                }
                else
                {
                    // Refresh the main list, or at least change the color of the refresh button.
                    //
                    if (NULL != m_pHome)
                    {
                        m_pHome->SetNeedRefresh();
                        m_pHome->OnDeletedRecord();
                    }
                }
            } // qstr_acct_id not empty.
        } // record is cash
        // ----------------------------------------
        else // record is not cash (it's some other outgoing instrument, such as a cheque.)
        {
            QMessageBox::StandardButton reply;

            reply = QMessageBox::question(this, "",
                                          "This will prevent the original recipient from exercising this instrument. "
                                          "(And FYI, this action will fail if he's already done so.) "
                                          "Are you sure you want to cancel?",
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::No)
                return;
            // ----------------------------------------
            bool bSuccess = false;
            {
                MTOverrideCursor theSpinner;
                // -----------------------------------------
                bSuccess = recordmt.CancelOutgoing(recordmt.GetAccountID());
            }
            // -----------------------------------------
            if (!bSuccess)
            {
                QMessageBox::warning(this, QString("Cancellation failure"),
                                     QString("Failed canceling this outgoing instrument."));
            }
            else
            {
                // Refresh the main list, or at least change the color of the refresh button.
                //
                if (NULL != m_pHome)
                    m_pHome->SetNeedRefresh();
            }
        } // not cash
        // ----------------------------------------
    } // if m_record not NULL.
}



void MTHomeDetail::on_discardOutgoingButton_clicked(bool checked /*=false*/)
{
    qDebug() << "Discard Outgoing button clicked.";

    if (m_record)
    {
        MTRecord & recordmt = *m_record;
        // ---------------------------------------
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, "",
                                      "This will prevent you from recovering this cash in the future. "
                                      "(And FYI, once the cash expires, this record will be discarded automatically anyway.) "
                                      "Are you sure you want to discard this record of sent cash?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No)
            return;
        // ----------------------------------------
        bool bSuccess = false;
        {
            MTOverrideCursor theSpinner;
            // -----------------------------------------
            bSuccess = recordmt.DiscardOutgoingCash();
        }
        // -----------------------------------------
        if (!bSuccess)
        {
            QMessageBox::warning(this, QString("Discard failure"),
                                 QString("Failed discarding this sent cash."));
        }
        else
        {
            // Refresh the main list, or at least change the color of the refresh button.
            //
            if (NULL != m_pHome)
            {
                m_pHome->SetNeedRefresh();
                m_pHome->OnDeletedRecord();
            }
        }
        // ----------------------------------
    }
}



void MTHomeDetail::on_discardIncomingButton_clicked(bool checked /*=false*/)
{
    qDebug() << "Discard Incoming button clicked.";

    if (m_record)
    {
        MTRecord & recordmt = *m_record;
        // ---------------------------------------
        QMessageBox::StandardButton reply;

        QString qstr_instrument = recordmt.IsInvoice() ? QString("invoice") : QString("instrument");

        reply = QMessageBox::question(this, "",
                                      QString("Are you sure you want to discard this incoming %1?").arg(qstr_instrument),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No)
            return;
        // ----------------------------------------
        bool bSuccess = false;
        {
            MTOverrideCursor theSpinner;
            // -------------------------------------
            bSuccess = recordmt.DiscardIncoming();
        }
        // -----------------------------------------
        if (!bSuccess)
        {
            QMessageBox::warning(this, QString("Discard failure"),
                                 QString("Failed discarding this incoming %1.").arg(qstr_instrument));
        }
        else
        {
            // Refresh the main list, or at least change the color of the refresh button.
            //
            if (NULL != m_pHome)
            {
                m_pHome->SetNeedRefresh();
                m_pHome->OnDeletedRecord();
            }
        }
        // ----------------------------------
    }
}



void MTHomeDetail::on_msgButton_clicked(bool checked /*=false*/)
{
    if (m_record)
    {
        MTRecord & recordmt = *m_record;
        // --------------------------------------------------
        const std::string str_my_nym_id    = recordmt.GetNymID();
        const std::string str_other_nym_id = recordmt.GetOtherNymID();
        const std::string str_server_id    = recordmt.GetServerID();
        // --------------------------------------------------
        QString myNymID    = QString::fromStdString(str_my_nym_id);
        QString otherNymID = QString::fromStdString(str_other_nym_id);
        QString serverID   = QString::fromStdString(str_server_id);
        // --------------------------------------------------
        MTCompose * compose_window = new MTCompose;
        compose_window->setAttribute(Qt::WA_DeleteOnClose);
        // --------------------------------------------------
        compose_window->setInitialSenderNym   (myNymID);
        compose_window->setInitialRecipientNym(otherNymID);
        compose_window->setInitialServer      (serverID);
        // ---------------------------------------
        // Set subject, if one is available.
        std::string str_desc;

        if (recordmt.IsMail())
            recordmt.FormatShortMailDescription(str_desc);
        else
            recordmt.FormatDescription(str_desc);
        // ---------------------------------------
        compose_window->setInitialSubject(QString::fromStdString(str_desc));
        // --------------------------------------------------
        compose_window->dialog();
        compose_window->show();
        // --------------------------------------------------
    }
}


//static
void MTHomeDetail::clearLayout(QLayout* pLayout)
{
    if ( NULL == pLayout)
        return;
    // -----------------------------------------------
    QLayoutItem * pItemAt = NULL;

    while ( ( pItemAt = pLayout->takeAt( 0 ) ) != NULL )
    {
        if (QWidget * childWidget = pItemAt->widget())
            delete childWidget;
        else if (QLayout* childLayout = pItemAt->layout())
            clearLayout(childLayout);
        delete pItemAt;
    }
}


//static
QWidget * MTHomeDetail::CreateDetailHeaderWidget(MTRecord & recordmt, bool bExternal/*=true*/)
{
    TransactionTableViewCellType cellType = (recordmt.IsOutgoing() ?
                                                 // -------------------------------------------------
                                                 (recordmt.IsPending() ?
                                                      TransactionTableViewCellTypeOutgoing :  // outgoing
                                                      TransactionTableViewCellTypeSent) :     // sent
                                                 // -------------------------------------------------
                                                 (recordmt.IsPending() ?
                                                      TransactionTableViewCellTypeIncoming :  // incoming
                                                      TransactionTableViewCellTypeReceived)); // received
    // --------------------------------------------------------------------------------------------
    // For invoices and invoice receipts.
    //
    if (recordmt.IsInvoice() || recordmt.IsPaymentPlan() ||
        ((0 == recordmt.GetInstrumentType().compare("chequeReceipt")) &&
         (( recordmt.IsOutgoing() && (OTAPI_Wrap::It()->StringToLong(recordmt.GetAmount()) > 0)) ||
          (!recordmt.IsOutgoing() && (OTAPI_Wrap::It()->StringToLong(recordmt.GetAmount()) < 0)))
         ))
        cellType = (recordmt.IsOutgoing() ?
                    (recordmt.IsPending() ?
                     TransactionTableViewCellTypeIncoming  : // outgoing
                     TransactionTableViewCellTypeReceived) : // sent
                    // -------------------------------------------------
                    (recordmt.IsPending() ?
                     TransactionTableViewCellTypeOutgoing  : // incoming
                     TransactionTableViewCellTypeSent));     // received
    // --------------------------------------------------------------------------------------------
    std::string str_desc;
    // ---------------------------------------
    if (recordmt.IsMail())
        recordmt.FormatShortMailDescription(str_desc);
    else
        recordmt.FormatDescription(str_desc);
    // ---------------------------------------
    QString strColor("black");

    switch (cellType)
    {
        case TransactionTableViewCellTypeReceived:
            strColor = QString("green");
            break;
        case TransactionTableViewCellTypeSent:
            strColor = QString("red");
            break;
        case TransactionTableViewCellTypeIncoming:
            strColor = QString("LightGreen");
            break;
        case TransactionTableViewCellTypeOutgoing:
            strColor = QString("Crimson");
            break;
        default:
            qDebug() << "CELL TYPE: " << cellType;
            assert("Expected all cell types to be handled for color.");
            break;
    }
    // --------------------------------------------------------------------------------------------
    //Append to transactions list in overview dialog.
    QWidget * row_widget = new QWidget;
    QGridLayout * row_widget_layout = new QGridLayout;

    row_widget_layout->setSpacing(4);
    row_widget_layout->setContentsMargins(10, 4, 10, 4);

    row_widget->setLayout(row_widget_layout);
    row_widget->setStyleSheet("QWidget{background-color:#c0cad4;selection-background-color:#a0aac4;}");
    // -------------------------------------------
    //Render row.
    //Header of row
    QString tx_name = QString(QString::fromStdString(recordmt.GetName()));

    if(tx_name.trimmed() == "")
    {
        //Tx has no name
        tx_name.clear();
        tx_name = "Transaction";
    }

    QLabel * header_of_row = new QLabel;
    QString header_of_row_string = QString("");
    header_of_row_string.append(tx_name);

    header_of_row->setText(header_of_row_string);

    //Append header to layout
    row_widget_layout->addWidget(header_of_row, 0, 0, 1,1, Qt::AlignLeft);
    // -------------------------------------------
    // Amount (with currency tla)
    QLabel * currency_amount_label = new QLabel;
    QString currency_amount;

    currency_amount_label->setStyleSheet(QString("QLabel { color : %1; }").arg(strColor));
    // ----------------------------------------------------------------
    bool bLabelAdded = false;

    std::string str_formatted;
    bool bFormatted = false;

    if (!recordmt.IsMail())
        bFormatted = recordmt.FormatAmount(str_formatted);
    // ----------------------------------------
    if (bFormatted && !str_formatted.empty())
        currency_amount = QString::fromStdString(str_formatted);
    else if (recordmt.IsMail())
    {
        if (recordmt.IsOutgoing())
            currency_amount = QString("sent msg");
        else
            currency_amount = QString("message");
        // ------------------------------------------
        if (!bExternal)
        {
//            QToolButton *buttonLock  = new QToolButton;
            // ----------------------------------------------------------------
            QPixmap pixmapLock    (":/icons/icons/lock.png");
            // ----------------------------------------------------------------
            QLabel * pLockLabel = new QLabel;
            pLockLabel->setPixmap(pixmapLock);

            QHBoxLayout * pLabelLayout = new QHBoxLayout;

            pLabelLayout->addWidget(pLockLabel);
            pLabelLayout->addWidget(currency_amount_label);

            row_widget_layout->addLayout(pLabelLayout, 0, 1, 1,1, Qt::AlignRight);

            bLabelAdded = true;
        }
    }
    else
        currency_amount = QString("");
    // ----------------------------------------------------------------
    currency_amount_label->setText(currency_amount);
    // ----------------------------------------------------------------
    if (!bLabelAdded)
        row_widget_layout->addWidget(currency_amount_label, 0, 1, 1,1, Qt::AlignRight);
    // -------------------------------------------
    //Sub-info
    QWidget * row_content_container = new QWidget;
    QGridLayout * row_content_grid = new QGridLayout;

    // left top right bottom

    row_content_grid->setSpacing(4);
    row_content_grid->setContentsMargins(3, 4, 3, 4);

    row_content_container->setLayout(row_content_grid);

    row_widget_layout->addWidget(row_content_container, 1,0, 1,2);
    // -------------------------------------------
    // Column one
    //Date (sub-info)
    //Calc/convert date/times
    QDateTime timestamp;

    long lDate = OTAPI_Wrap::StringToLong(recordmt.GetDate());

    timestamp.setTime_t(lDate);

    QLabel * row_content_date_label = new QLabel;
    QString row_content_date_label_string;
    row_content_date_label_string.append(QString(timestamp.toString(Qt::SystemLocaleShortDate)));

    row_content_date_label->setStyleSheet("QLabel { color : grey; font-size:11pt;}");
    row_content_date_label->setText(row_content_date_label_string);

    row_content_grid->addWidget(row_content_date_label, 0,0, 1,1, Qt::AlignLeft);
    // -------------------------------------------
    // Column two
    //Status
    QLabel * row_content_status_label = new QLabel;
    QString row_content_status_string;

    row_content_status_string.append(QString::fromStdString(str_desc));
    // -------------------------------------------
    //add string to label
    row_content_status_label->setStyleSheet("QLabel { color : grey; font-size:11pt;}");
    row_content_status_label->setWordWrap(false);
    row_content_status_string.replace("\r\n"," ");
    row_content_status_string.replace("\n\r"," ");
    row_content_status_string.replace("\n",  " ");
    row_content_status_label->setText(row_content_status_string);

    //add to row_content grid
    row_content_grid->addWidget(row_content_status_label, 0,1, 1,1, Qt::AlignRight);
    // -------------------------------------------
    return row_widget;
}


void SetHeight (QPlainTextEdit* edit, int nRows)
{
    QFontMetrics m (edit -> font()) ;
    int RowHeight = m.lineSpacing() ;
    edit -> setFixedHeight  (nRows * RowHeight) ;
}


void increment_cell(int & nCurrentRow, int & nCurrentColumn)
{
    if (nCurrentColumn == 1)
    {
        nCurrentColumn++;
        return;
    }
    // ------------------------
    else
    {
        nCurrentColumn--;
        nCurrentRow++;
    }
}


void MTHomeDetail::refresh(int nRow, MTRecordList & theList)
{
//  qDebug() << QString("MTHomeDetail::refresh: nRow: %1").arg(nRow);

    if ((nRow >= 0) && (nRow < theList.size()))
    {
        weak_ptr_MTRecord   weakRecord = theList.GetRecord(nRow);
        shared_ptr_MTRecord record     = weakRecord.lock();

        if (weakRecord.expired())
        {
            this->setLayout(m_pDetailLayout);
            return;
        }
        // --------------------------------------------------
        m_record = record;
        MTRecord & recordmt = *record;
        // --------------------------------------------------
        refresh(recordmt);
    }
//  else
//      qDebug() << QString("MTHomeDetail::refresh: nRow %1 is out of bounds. (Max size is %2.)").arg(nRow).arg(theList.size());
    else
        RecreateLayout(); // This blanks out the detail side.
}


void MTHomeDetail::RecreateLayout()
{
    m_nContactID = 0;
    // --------------------------------------------------
    if (NULL != m_pDetailLayout)
    {
        MTHomeDetail::clearLayout(m_pDetailLayout);
        delete m_pDetailLayout;
        m_pDetailLayout = NULL;
    }
    // --------------------------------------------------
    m_pDetailLayout = new QGridLayout;
    m_pDetailLayout->setAlignment(Qt::AlignTop);
    m_pDetailLayout->setContentsMargins(0, 0, 0, 0);
    // --------------------------------------------------
    m_pLineEdit_Nym_ID          = NULL;
    m_pLineEdit_OtherNym_ID     = NULL;
    m_pLineEdit_Acct_ID         = NULL;
    m_pLineEdit_OtherAcct_ID    = NULL;
    m_pLineEdit_Server_ID       = NULL;
    m_pLineEdit_AssetType_ID    = NULL;
    m_pLineEdit_Nym_Name        = NULL;
    m_pLineEdit_OtherNym_Name   = NULL;
    m_pLineEdit_Acct_Name       = NULL;
    m_pLineEdit_OtherAcct_Name  = NULL;
    m_pLineEdit_Server_Name     = NULL;
    m_pLineEdit_AssetType_Name  = NULL;
}


void MTHomeDetail::refresh(MTRecord & recordmt)
{
    // --------------------------------------------------
    RecreateLayout();
    // --------------------------------------------------
    int nCurrentRow = 0;
    int nCurrentColumn = 1;
    // --------------------------------------------------
    // Add the header widget for this detail.
    //
    QWidget * pHeader = MTHomeDetail::CreateDetailHeaderWidget(recordmt, false);

    if (NULL != pHeader)
    {            
        pHeader->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        m_pDetailLayout->addWidget(pHeader, nCurrentRow, nCurrentColumn, 1, 2);
        m_pDetailLayout->setAlignment(pHeader, Qt::AlignTop);
        nCurrentRow++;
    }
    // --------------------------------------------------
    if (recordmt.HasMemo())
    {
        QPlainTextEdit *sec = new QPlainTextEdit;

//        const std::string str_acct_id    = recordmt.GetOtherAccountID();
//        const std::string str_nym_id     = recordmt.GetOtherNymID();
//        const std::string str_server_id  = recordmt.GetServerID();
//        const std::string str_asset_id   = recordmt.GetAssetID();
//
//        QString strMemo = QString("AcctID: %1 NymID: %2 ServerID: %3 AssetID: %4").
//                arg(QString::fromStdString(str_acct_id)).
//                arg(QString::fromStdString(str_nym_id)).
//                arg(QString::fromStdString(str_server_id)).
//                arg(QString::fromStdString(str_asset_id));


        QString strMemo = QString(recordmt.GetMemo().c_str());

        sec->setPlainText(strMemo);
        sec->setReadOnly(true);
        // -----------------------------------------
        QHBoxLayout * pHLayout = new QHBoxLayout;
        QLabel * labelMemo = new QLabel(QString("Memo: "));

        pHLayout->addWidget(labelMemo);
        pHLayout->addWidget(sec);

        pHLayout->setAlignment(labelMemo, Qt::AlignLeft);
        pHLayout->setAlignment(sec, Qt::AlignTop);
        // -----------------------------------------
        m_pDetailLayout->addLayout(pHLayout, nCurrentRow, nCurrentColumn, 1, 2);
        m_pDetailLayout->setAlignment(pHLayout, Qt::AlignTop);

        SetHeight (sec, 2);

        nCurrentRow++;
    }
    // --------------------------------------------------





    // *************************************************************
    QString viewDetails = QString("");

    if (recordmt.IsReceipt() || recordmt.IsOutgoing())
        viewDetails = QString("View Recipient Details");
    else
        viewDetails = QString("View Sender Details");
    // --------------------------------------------------
    const std::string str_acct_id    = recordmt.GetOtherAccountID();
    const std::string str_nym_id     = recordmt.GetOtherNymID();
    const std::string str_server_id  = recordmt.GetServerID();
    const std::string str_asset_id   = recordmt.GetAssetID();

    if (!str_nym_id.empty() && !str_server_id.empty())
        MTContactHandler::getInstance()->NotifyOfNymServerPair(QString::fromStdString(str_nym_id),
                                                               QString::fromStdString(str_server_id));
    // --------------------------------------------------
    int nContactID = 0;

    if (!str_acct_id.empty())
        nContactID = MTContactHandler::getInstance()->FindContactIDByAcctID(QString::fromStdString(str_acct_id),
                                                                            QString::fromStdString(str_nym_id),
                                                                            QString::fromStdString(str_server_id),
                                                                            QString::fromStdString(str_asset_id));
    if (!(nContactID > 0) && !str_nym_id.empty())
        nContactID = MTContactHandler::getInstance()->FindContactIDByNymID (QString::fromStdString(str_nym_id));
    // --------------------------------------------------
    bool bExistingContact = (nContactID > 0);

    if (bExistingContact)
    {
        m_nContactID = nContactID;
        // --------------------------------------------------
        QPushButton * viewContactButton = new QPushButton(viewDetails);

        m_pDetailLayout->addWidget(viewContactButton, nCurrentRow, nCurrentColumn,1,1);
        m_pDetailLayout->setAlignment(viewContactButton, Qt::AlignTop);

        increment_cell(nCurrentRow, nCurrentColumn);
        // -------------------------------------------
        connect(viewContactButton, SIGNAL(clicked()), this, SLOT(on_viewContactButton_clicked()));
    }
    else if (!str_nym_id.empty())
    {
        QPushButton * addContactButton = new QPushButton(QString("Add as Contact"));

        m_pDetailLayout->addWidget(addContactButton, nCurrentRow, nCurrentColumn,1,1);
        m_pDetailLayout->setAlignment(addContactButton, Qt::AlignTop);

        increment_cell(nCurrentRow, nCurrentColumn);
        // -------------------------------------------
        connect(addContactButton, SIGNAL(clicked()), this, SLOT(on_addContactButton_clicked()));
        // --------------------------------------------------
        // If the contact didn't already exist, we don't just have "add new contact"
        // but also "add to existing contact."
        //
        QPushButton * existingContactButton = new QPushButton(QString("Add to an Existing Contact"));

        m_pDetailLayout->addWidget(existingContactButton, nCurrentRow, nCurrentColumn,1,1);
        m_pDetailLayout->setAlignment(existingContactButton, Qt::AlignTop);

        increment_cell(nCurrentRow, nCurrentColumn);
        // ----------------------------------
        connect(existingContactButton, SIGNAL(clicked()), this, SLOT(on_existingContactButton_clicked()));
    }
    // *************************************************************
    if (recordmt.CanDeleteRecord())
    {
        QString deleteActionName = recordmt.IsMail() ? QString("Archive this Message") : QString("Archive this Record");
        QPushButton * deleteButton = new QPushButton(deleteActionName);

        m_pDetailLayout->addWidget(deleteButton, nCurrentRow, nCurrentColumn,1,1);
        m_pDetailLayout->setAlignment(deleteButton, Qt::AlignTop);

        increment_cell(nCurrentRow, nCurrentColumn);
        // -------------------------------------------
        connect(deleteButton, SIGNAL(clicked()), this, SLOT(on_deleteButton_clicked()));
    }
    // --------------------------------------------------

    if (recordmt.CanAcceptIncoming())
    {
        const bool bIsTransfer = (recordmt.GetRecordType() == MTRecord::Transfer);
        const bool bIsReceipt  = (recordmt.GetRecordType() == MTRecord::Receipt);

        QString nameString;
        QString actionString;

        if (recordmt.IsTransfer())
        {
            nameString = QString("Accept this Transfer");
            actionString = QString("Accepting...");
        }
        else if (recordmt.IsReceipt())
        {
            nameString = QString("Accept this Receipt");
            actionString = QString("Accepting...");
        }
        else if (recordmt.IsInvoice())
        {
            nameString = QString("Pay this Invoice");
            actionString = QString("Paying...");
        }
        else if (recordmt.IsPaymentPlan())
        {
            nameString = QString("Activate this Payment Plan");
            actionString = QString("Activating...");
        }
        else if (recordmt.IsContract())
        {
            nameString = QString("Sign this Smart Contract");
            actionString = QString("Signing...");
        }
        else if (recordmt.IsCash())
        {
            nameString = QString("Deposit this Cash");
            actionString = QString("Depositing...");
        }
        else if (recordmt.IsCheque())
        {
            nameString = QString("Deposit this Cheque");
            actionString = QString("Depositing...");
        }
        else if (recordmt.IsVoucher())
        {
            nameString = QString("Accept this Payment");
            actionString = QString("Accepting...");
        }
        else
        {
            nameString = QString("Deposit this Payment");
            actionString = QString("Depositing...");
        }

        QPushButton * acceptButton = new QPushButton(nameString);

        m_pDetailLayout->addWidget(acceptButton, nCurrentRow, nCurrentColumn,1,1);
        m_pDetailLayout->setAlignment(acceptButton, Qt::AlignTop);

        increment_cell(nCurrentRow, nCurrentColumn);

        connect(acceptButton, SIGNAL(clicked()), this, SLOT(on_acceptButton_clicked()));

    }

    if (recordmt.CanCancelOutgoing())
    {
        QString msg = QString("Cancellation Failed. Perhaps recipient had already accepted it?");
//        UIAlertView *fail = [[UIAlertView alloc] initWithTitle:nil message:msg delegate:nil
//                                             cancelButtonTitle:QString("OK") otherButtonTitles:nil];

        QString cancelString;
        QString actionString = QString("Canceling...");

        if (recordmt.IsInvoice())
            cancelString = QString("Cancel this Invoice");
        else if (recordmt.IsPaymentPlan())
            cancelString = QString("Cancel this Payment Plan");
        else if (recordmt.IsContract())
            cancelString = QString("Cancel this Smart Contract");
        else if (recordmt.IsCash())
        {
            cancelString = QString("Recover this Cash");
            actionString = QString("Recovering...");
            msg = QString("Recovery Failed. Perhaps recipient had already accepted it?");
        }
        else if (recordmt.IsCheque())
            cancelString = QString("Cancel this Cheque");
        else if (recordmt.IsVoucher())
            cancelString = QString("Cancel this Payment");
        else
            cancelString = QString("Cancel this Payment");


        QPushButton * cancelButton = new QPushButton(cancelString);

        m_pDetailLayout->addWidget(cancelButton, nCurrentRow, nCurrentColumn,1,1);
        m_pDetailLayout->setAlignment(cancelButton, Qt::AlignTop);

        increment_cell(nCurrentRow, nCurrentColumn);

        connect(cancelButton, SIGNAL(clicked()), this, SLOT(on_cancelButton_clicked()));

    }

    if (recordmt.CanDiscardOutgoingCash())
    {
        QString discardString = QString("Discard this Sent Cash");

        QPushButton * discardOutgoingButton = new QPushButton(discardString);

        m_pDetailLayout->addWidget(discardOutgoingButton, nCurrentRow, nCurrentColumn,1,1);
        m_pDetailLayout->setAlignment(discardOutgoingButton, Qt::AlignTop);

        increment_cell(nCurrentRow, nCurrentColumn);

        connect(discardOutgoingButton, SIGNAL(clicked()), this, SLOT(on_discardOutgoingButton_clicked()));


    }

    if (recordmt.CanDiscardIncoming())
    {
        QString discardString;

        if (recordmt.IsInvoice())
            discardString = QString("Discard this Invoice");
        else if (recordmt.IsPaymentPlan())
            discardString = QString("Discard this Payment Plan");
        else if (recordmt.IsContract())
            discardString = QString("Discard this Smart Contract");
        else if (recordmt.IsCash())
            discardString = QString("Discard this Cash");
        else if (recordmt.IsCheque())
            discardString = QString("Discard this Cheque");
        else if (recordmt.IsVoucher())
            discardString = QString("Discard this Payment");
        else
            discardString = QString("Discard this Payment");


        QPushButton * discardIncomingButton = new QPushButton(discardString);

        m_pDetailLayout->addWidget(discardIncomingButton, nCurrentRow, nCurrentColumn,1,1);
        m_pDetailLayout->setAlignment(discardIncomingButton, Qt::AlignTop);

        increment_cell(nCurrentRow, nCurrentColumn);

        connect(discardIncomingButton, SIGNAL(clicked()), this, SLOT(on_discardIncomingButton_clicked()));

    }

    if (!(recordmt.GetOtherNymID().empty()))
    {
        QString msgUser;

        if (recordmt.IsReceipt() || recordmt.IsOutgoing())
            msgUser = QString("Message the Recipient");
        else
            msgUser = QString("Message the Sender");


        QPushButton * msgButton = new QPushButton(((recordmt.IsMail() && !recordmt.IsOutgoing()) ? QString("Reply to this Message") : msgUser));

        m_pDetailLayout->addWidget(msgButton, nCurrentRow, nCurrentColumn,1,1);
        m_pDetailLayout->setAlignment(msgButton, Qt::AlignTop);

        increment_cell(nCurrentRow, nCurrentColumn);

        connect(msgButton, SIGNAL(clicked()), this, SLOT(on_msgButton_clicked()));

    }

    // ----------------------------------
    if (nCurrentColumn > 1)
    {
        nCurrentColumn = 1;
        nCurrentRow++;
    }
    // ----------------------------------
    // TRANSACTION IDs DISPLAYED HERE

    QString qstr_NymID       = QString::fromStdString(recordmt.GetNymID());
    QString qstr_OtherNymID  = QString::fromStdString(recordmt.GetOtherNymID());
    QString qstr_AccountID   = QString::fromStdString(recordmt.GetAccountID());
    QString qstr_OtherAcctID = QString::fromStdString(recordmt.GetOtherAccountID());
    QString qstr_ServerID    = QString::fromStdString(recordmt.GetServerID());
    QString qstr_AssetTypeID = QString::fromStdString(recordmt.GetAssetID());

    QString qstr_OtherType;

    if (recordmt.IsReceipt() || recordmt.IsOutgoing())
        qstr_OtherType = QString("Recipient");
    else
        qstr_OtherType = QString("Sender");

    QGridLayout * pGridLayout = new QGridLayout;
    int nGridRow = 0;

    pGridLayout->setColumnMinimumWidth(0, 75);
    pGridLayout->setColumnMinimumWidth(1, 75);

    pGridLayout->setColumnStretch(1, 2);
    pGridLayout->setColumnStretch(2, 5);

    pGridLayout->setAlignment(Qt::AlignTop);

    if (!qstr_NymID.isEmpty())
    {
        QLabel    * pLabel    = new QLabel(QString("My Nym: "));

        MTNameLookupQT theLookup;
        QString qstr_name = QString::fromStdString(theLookup.GetNymName(qstr_NymID.toStdString()));

        m_pLineEdit_Nym_ID          = new QLineEdit(qstr_NymID);
        m_pLineEdit_Nym_Name        = new QLineEdit(qstr_name);

        m_pLineEdit_Nym_ID->setReadOnly(true);
        m_pLineEdit_Nym_Name->setReadOnly(true);

        m_pLineEdit_Nym_ID->setStyleSheet("QLineEdit { background-color: lightgray }");
        m_pLineEdit_Nym_Name->setStyleSheet("QLineEdit { background-color: lightgray }");

        pGridLayout->addWidget(pLabel, nGridRow,   0);
        pGridLayout->addWidget(m_pLineEdit_Nym_Name, nGridRow,   1);
        pGridLayout->addWidget(m_pLineEdit_Nym_ID, nGridRow++, 2);
    }

    if (!qstr_OtherNymID.isEmpty())
    {
        QLabel    * pLabel    = new QLabel(QString("%1 Nym: ").arg(qstr_OtherType));

        MTNameLookupQT theLookup;
        QString qstr_name = QString::fromStdString(theLookup.GetNymName(qstr_OtherNymID.toStdString()));

        m_pLineEdit_OtherNym_ID     = new QLineEdit(qstr_OtherNymID);
        m_pLineEdit_OtherNym_Name   = new QLineEdit(qstr_name);

        m_pLineEdit_OtherNym_ID->setReadOnly(true);
        m_pLineEdit_OtherNym_Name->setReadOnly(true);

        m_pLineEdit_OtherNym_ID->setStyleSheet("QLineEdit { background-color: lightgray }");
        m_pLineEdit_OtherNym_Name->setStyleSheet("QLineEdit { background-color: lightgray }");

        pGridLayout->addWidget(pLabel,    nGridRow,   0);
        pGridLayout->addWidget(m_pLineEdit_OtherNym_Name,    nGridRow,   1);
        pGridLayout->addWidget(m_pLineEdit_OtherNym_ID, nGridRow++, 2);
    }

    if (!qstr_AccountID.isEmpty())
    {
        QLabel    * pLabel    = new QLabel(QString("My Account: "));

        MTNameLookupQT theLookup;
        QString qstr_name = QString::fromStdString(theLookup.GetAcctName(qstr_AccountID.toStdString()));

        m_pLineEdit_Acct_ID         = new QLineEdit(qstr_AccountID);
        m_pLineEdit_Acct_Name       = new QLineEdit(qstr_name);

        m_pLineEdit_Acct_ID->setReadOnly(true);
        m_pLineEdit_Acct_Name->setReadOnly(true);

        m_pLineEdit_Acct_ID->setStyleSheet("QLineEdit { background-color: lightgray }");
        m_pLineEdit_Acct_Name->setStyleSheet("QLineEdit { background-color: lightgray }");

        pGridLayout->addWidget(pLabel,    nGridRow,   0);
        pGridLayout->addWidget(m_pLineEdit_Acct_Name,    nGridRow,   1);
        pGridLayout->addWidget(m_pLineEdit_Acct_ID, nGridRow++, 2);
    }

    if (!qstr_OtherAcctID.isEmpty())
    {
        QLabel    * pLabel    = new QLabel(QString("%1 Account: ").arg(qstr_OtherType));

        MTNameLookupQT theLookup;
        QString qstr_name = QString::fromStdString(theLookup.GetAcctName(qstr_OtherAcctID.toStdString()));

        m_pLineEdit_OtherAcct_ID    = new QLineEdit(qstr_OtherAcctID);
        m_pLineEdit_OtherAcct_Name  = new QLineEdit(qstr_name);

        m_pLineEdit_OtherAcct_ID->setReadOnly(true);
        m_pLineEdit_OtherAcct_Name->setReadOnly(true);

        m_pLineEdit_OtherAcct_ID->setStyleSheet("QLineEdit { background-color: lightgray }");
        m_pLineEdit_OtherAcct_Name->setStyleSheet("QLineEdit { background-color: lightgray }");

        pGridLayout->addWidget(pLabel,    nGridRow,   0);
        pGridLayout->addWidget(m_pLineEdit_OtherAcct_Name,    nGridRow,   1);
        pGridLayout->addWidget(m_pLineEdit_OtherAcct_ID, nGridRow++, 2);
    }

    if (!qstr_ServerID.isEmpty())
    {
        QLabel    * pLabel    = new QLabel(QString("Server: "));

        QString qstr_name = QString::fromStdString(OTAPI_Wrap::GetServer_Name(qstr_ServerID.toStdString()));

        m_pLineEdit_Server_ID       = new QLineEdit(qstr_ServerID);
        m_pLineEdit_Server_Name     = new QLineEdit(qstr_name);

        m_pLineEdit_Server_ID->setReadOnly(true);
        m_pLineEdit_Server_Name->setReadOnly(true);

        m_pLineEdit_Server_ID->setStyleSheet("QLineEdit { background-color: lightgray }");
        m_pLineEdit_Server_Name->setStyleSheet("QLineEdit { background-color: lightgray }");

        pGridLayout->addWidget(pLabel,    nGridRow,   0);
        pGridLayout->addWidget(m_pLineEdit_Server_Name,    nGridRow,   1);
        pGridLayout->addWidget(m_pLineEdit_Server_ID, nGridRow++, 2);
    }

    if (!qstr_AssetTypeID.isEmpty())
    {
        QLabel    * pLabel    = new QLabel(QString("Asset Type: "));

        QString qstr_name = QString::fromStdString(OTAPI_Wrap::GetAssetType_Name(qstr_AssetTypeID.toStdString()));

        m_pLineEdit_AssetType_ID    = new QLineEdit(qstr_AssetTypeID);
        m_pLineEdit_AssetType_Name  = new QLineEdit(qstr_name);

        m_pLineEdit_AssetType_ID->setReadOnly(true);
        m_pLineEdit_AssetType_Name->setReadOnly(true);

        m_pLineEdit_AssetType_ID->setStyleSheet("QLineEdit { background-color: lightgray }");
        m_pLineEdit_AssetType_Name->setStyleSheet("QLineEdit { background-color: lightgray }");

        pGridLayout->addWidget(pLabel, nGridRow, 0);
        pGridLayout->addWidget(m_pLineEdit_AssetType_Name, nGridRow, 1);
        pGridLayout->addWidget(m_pLineEdit_AssetType_ID, nGridRow++, 2);
    }
    // ----------------------------------
    FavorLeftSideForIDs();
    // ----------------------------------
    QTabWidget * pTabWidget  = new QTabWidget;
    QWidget    * pTab1Widget = new QWidget;
    QWidget    * pTab2Widget = NULL;
    QWidget    * pTab3Widget = NULL;

    pTabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pTab1Widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    pTabWidget->setContentsMargins(5, 5, 5, 5);
    pTab1Widget->setContentsMargins(5, 5, 5, 5);

    pTabWidget->addTab(pTab1Widget, QString("Details"));
    // ----------------------------------
    if (pGridLayout->count() > 0)
    {
        pTab3Widget = new QWidget;

        pTab3Widget->setContentsMargins(0, 0, 0, 0);
        pTab3Widget->setLayout(pGridLayout);

        pTabWidget->addTab(pTab3Widget, QString("IDs"));
//      m_pDetailLayout->addLayout(pGridLayout, nCurrentRow++, nCurrentColumn, 1, 2, Qt::AlignBottom);
    }
    else
    {
        delete pGridLayout;
        pGridLayout = NULL;
    }

//    ActionSection *act = [ActionSection sectionWithName:QString("Actions") andActions:actions];
//    act.defaultAlignment = (UITextAlignment) UITextAlignmentLeft;

//    TransactionIdSection* idSec = [TransactionIdSection sectionWithRecord:record];
    // ----------------------------------
//    NSMutableArray *sections = [NSMutableArray arrayWithArray:QString()[
//                                [TransactionSummarySection sectionWithRecord:record],
//                                act,
//                                idSec,
//                                ]];
    // ----------------------------------
    if (recordmt.HasContents())
    {
        QPlainTextEdit *sec = new QPlainTextEdit;

        QString strContents = QString(recordmt.GetContents().c_str());

        sec->setPlainText(strContents);
        sec->setReadOnly(true);
        sec->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        // -------------------------------

        // -------------------------------
        QVBoxLayout * pvBox = NULL;

        if (recordmt.IsMail())
        {
            m_pDetailLayout->addWidget(sec, nCurrentRow++, nCurrentColumn, 1, 2);
        }
        else
        {
            pvBox       = new QVBoxLayout;
            pTab2Widget = new QWidget;
            QLabel * pLabelContents = new QLabel(QString("Raw Contents:"));

            pvBox->setAlignment(Qt::AlignTop);
            pvBox->addWidget   (pLabelContents);
            pvBox->addWidget   (sec);

            pTab2Widget->setContentsMargins(0, 0, 0, 0);
            pTab2Widget->setLayout(pvBox);

            pTabWidget->addTab(pTab2Widget, QString("Contents"));
        }
        // -------------------------------
    }    


    pTab1Widget->setLayout(m_pDetailLayout);

    // -----------------------------------------------
    this->m_pDetailLayout = new QGridLayout;
    this->m_pDetailLayout->addWidget(pTabWidget);

    m_pDetailLayout->setContentsMargins(0,0,0,0);
    pTabWidget->setTabPosition(QTabWidget::South);

    this->setLayout(m_pDetailLayout);
    // ----------------------------------

/*
    if (recordmt.IsPaymentPlan())
    {
        if (recordmt.HasInitialPayment() || recordmt.HasPaymentPlan())
        {
            std::string str_asset_name = OTAPI_Wrap::GetAssetType_Name(recordmt.GetAssetID().c_str());
            // ---------------------------------
            std::stringstream sss;
            sss << "Payments use the currency: " << str_asset_name << "\n";
            // ---------------------------------
            NSDateFormatter * formatter  = nil;
            NSString        * dateString = nil;
            NSDate          * date       = nil;

            formatter = [[NSDateFormatter alloc] init];
            [formatter setDateFormat:QString("dd-MM-yyyy HH:mm")];

            if (recordmt.HasInitialPayment())
            {
                date = [NSDate dateWithTimeIntervalSince1970:recordmt.GetInitialPaymentDate()];
                dateString = [formatter stringFromDate:[NSDate date]];

                long        lAmount    = recordmt.GetInitialPaymentAmount();
                std::string str_output = OTAPI_Wrap::It()->FormatAmount(recordmt.GetAssetID().c_str(),
                                                                        static_cast<int64_t>(lAmount));
                sss << "Initial payment of " << str_output << " due: " << dateString.UTF8String << "\n";
            }
            // -----------------------------------------------
            if (recordmt.HasPaymentPlan())
            {
                // ----------------------------------------------------------------
                date = [NSDate dateWithTimeIntervalSince1970:recordmt.GetPaymentPlanStartDate()];
                dateString = [formatter stringFromDate:[NSDate date]];

                long        lAmount    = recordmt.GetPaymentPlanAmount();
                std::string str_output = OTAPI_Wrap::It()->FormatAmount(recordmt.GetAssetID().c_str(),
                                                                        static_cast<int64_t>(lAmount));
                sss << "Recurring payments of " << str_output << " begin: " << dateString.UTF8String << " ";
                // ----------------------------------------------------------------
                NSDate *date2 = [NSDate dateWithTimeIntervalSince1970:(recordmt.GetPaymentPlanStartDate() + recordmt.GetTimeBetweenPayments())];
                NSCalendar *calendar = [NSCalendar currentCalendar];
                NSUInteger calendarUnits = NSDayCalendarUnit;
                NSDateComponents *dateComponents = [calendar components:calendarUnits fromDate:date toDate:date2 options:0];

                std::string str_regular = (([dateComponents day] == 1) ?
                                           "and repeat daily." :
                                           "and repeat every %i days.");
                // -----------------------------------------------------------------
                NSString * strInterval = nil;

                if ([dateComponents day] == 1)
                    strInterval = QString(str_regular.c_str());
                else
                    strInterval = [NSString stringWithFormat:QString(str_regular.c_str()),[dateComponents day]];
                // -----------------------------------------------------------------
                sss << strInterval.UTF8String << "\n";
                // -----------------------------------------------------------------
                if (recordmt.GetMaximumNoPayments() > 0)
                    sss << "The maximum number of payments is: " << recordmt.GetMaximumNoPayments() << "\n";
                // -----------------------------------------------------------------
//todo:         inline const time_t &	GetPaymentPlanLength()	 const	{ return m_tPaymentPlanLength; }
            }
            // -----------------------------------------------
            std::string str_details(sss.str());
            TextViewSection *sec = [TextViewSection sectionWithName:QString("Recurring Payment Terms")
                                                      initialString:QString(str_details.c_str()) andStringAction:nil];
            sec.textView.editable = NO;
            sec.customHeight = 80;

            [sections insertObject:sec atIndex:1]; // So the recurring payment terms appears just under the memo.
        }
    }
*/
}















