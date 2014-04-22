#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/accountdetails.hpp>
#include <ui_accountdetails.h>

#include <gui/widgets/cashpurse.hpp>
#include <gui/widgets/home.hpp>
#include <gui/widgets/wizardaddaccount.hpp>
#include <gui/widgets/overridecursor.hpp>

#include <core/moneychanger.hpp>

#include <opentxs/OTAPI.hpp>
#include <opentxs/OTAPI_Exec.hpp>
#include <opentxs/OT_ME.hpp>

#include <QMessageBox>
#include <QDebug>




MTAccountDetails::MTAccountDetails(QWidget *parent, MTDetailEdit & theOwner) :
    MTEditDetails(parent, theOwner),
    m_qstrID(""),
    ui(new Ui::MTAccountDetails)
{
    ui->setupUi(this);
    this->setContentsMargins(0, 0, 0, 0);
//  this->installEventFilter(this); // NOTE: Successfully tested theory that the base class has already installed this.

    // ----------------------------------
    // Note: This is a placekeeper, so later on I can just erase
    // the widget at 0 and replace it with the real header widget.
    //
    m_pHeaderWidget  = new QWidget;
    ui->verticalLayout->insertWidget(0, m_pHeaderWidget);
    // ----------------------------------
//  ui->lineEditID    ->setStyleSheet("QLineEdit { background-color: lightgray }");
    // ----------------------------------
    ui->lineEditServer->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditAsset ->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditNym   ->setStyleSheet("QLineEdit { background-color: lightgray }");
    // ----------------------------------
}

MTAccountDetails::~MTAccountDetails()
{
    delete ui;
}

void MTAccountDetails::ClearContents()
{
    ui->lineEditName->setText("");
    ui->lineEditServer->setText("");
    ui->lineEditAsset->setText("");
    ui->lineEditNym->setText("");
    // ------------------------------------------
    if (m_pCashPurse)
        m_pCashPurse->ClearContents();
    // ------------------------------------------
    ui->pushButtonMakeDefault->setEnabled(false);

    m_qstrID = QString("");
}


void MTAccountDetails::FavorLeftSideForIDs()
{
    if (NULL != ui)
    {
//      ui->lineEditID  ->home(false);
        ui->lineEditName->home(false);
        // ----------------------------------
        ui->lineEditServer->home(false);
        ui->lineEditAsset ->home(false);
        ui->lineEditNym   ->home(false);
        // ----------------------------------
    }
}

// ------------------------------------------------------
//virtual
int MTAccountDetails::GetCustomTabCount()
{
    return 1;
}
// ----------------------------------
//virtual
QWidget * MTAccountDetails::CreateCustomTab(int nTab)
{
    const int nCustomTabCount = this->GetCustomTabCount();
    // -----------------------------
    if ((nTab < 0) || (nTab >= nCustomTabCount))
        return NULL; // out of bounds.
    // -----------------------------
    QWidget * pReturnValue = NULL;
    // -----------------------------
    switch (nTab)
    {
    case 0: // "Cash Purse" tab
        if (NULL != m_pOwner)
        {
            if (m_pCashPurse)
            {
                m_pCashPurse->setParent(NULL);
                m_pCashPurse->disconnect();
                m_pCashPurse->deleteLater();

                m_pCashPurse = NULL;
            }
            m_pCashPurse = new MTCashPurse(NULL, *m_pOwner);
            pReturnValue = m_pCashPurse;
            pReturnValue->setContentsMargins(0, 0, 0, 0);

            connect(m_pCashPurse, SIGNAL(balancesChanged(QString)),
                    m_pOwner,     SLOT(onBalancesChangedFromBelow(QString)));
        }
        break;

    default:
        qDebug() << QString("Unexpected: MTAccountDetails::CreateCustomTab was called with bad index: %1").arg(nTab);
        return NULL;
    }
    // -----------------------------
    return pReturnValue;
}

// ---------------------------------
//virtual
QString  MTAccountDetails::GetCustomTabName(int nTab)
{
    const int nCustomTabCount = this->GetCustomTabCount();
    // -----------------------------
    if ((nTab < 0) || (nTab >= nCustomTabCount))
        return tr(""); // out of bounds.
    // -----------------------------
    QString qstrReturnValue("");
    // -----------------------------
    switch (nTab)
    {
    case 0:  qstrReturnValue = tr("Cash Purse");  break;

    default:
        qDebug() << QString("Unexpected: MTAccountDetails::GetCustomTabName was called with bad index: %1").arg(nTab);
        return tr("");
    }
    // -----------------------------
    return qstrReturnValue;
}
// ------------------------------------------------------


//virtual
void MTAccountDetails::refresh(QString strID, QString strName)
{
//  qDebug() << "MTAccountDetails::refresh";

    if (!strID.isEmpty() && (NULL != ui))
    {
        m_qstrID = strID;
        // -------------------------------------
        QString qstrAmount = MTHome::shortAcctBalance(strID);

        QWidget * pHeaderWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, strID, strName, qstrAmount, "", ":/icons/icons/vault.png", false);

        pHeaderWidget->setObjectName(QString("DetailHeader")); // So the stylesheet doesn't get applied to all its sub-widgets.

        if (m_pHeaderWidget)
        {
            ui->verticalLayout->removeWidget(m_pHeaderWidget);

            m_pHeaderWidget->setParent(NULL);
            m_pHeaderWidget->disconnect();
            m_pHeaderWidget->deleteLater();

            m_pHeaderWidget = NULL;
        }
        ui->verticalLayout->insertWidget(0, pHeaderWidget);
        m_pHeaderWidget = pHeaderWidget;
        // ----------------------------------
//      ui->lineEditID  ->setText(strID);
        ui->lineEditName->setText(strName);
        // ----------------------------------
        std::string str_server_id = OTAPI_Wrap::It()->GetAccountWallet_ServerID   (strID.toStdString());
        std::string str_asset_id  = OTAPI_Wrap::It()->GetAccountWallet_AssetTypeID(strID.toStdString());
        std::string str_nym_id    = OTAPI_Wrap::It()->GetAccountWallet_NymID      (strID.toStdString());
        // ----------------------------------
        QString qstr_server_id    = QString::fromStdString(str_server_id);
        QString qstr_asset_id     = QString::fromStdString(str_asset_id);
        QString qstr_nym_id       = QString::fromStdString(str_nym_id);
        // ----------------------------------
        QString qstr_server_name  = QString::fromStdString(OTAPI_Wrap::It()->GetServer_Name   (str_server_id));
        QString qstr_asset_name   = QString::fromStdString(OTAPI_Wrap::It()->GetAssetType_Name(str_asset_id));
        QString qstr_nym_name     = QString::fromStdString(OTAPI_Wrap::It()->GetNym_Name      (str_nym_id));
        // ----------------------------------
        // MAIN TAB
        //
        if (!strID.isEmpty())
        {
            ui->lineEditServer->setText(qstr_server_name);
            ui->lineEditAsset ->setText(qstr_asset_name);
            ui->lineEditNym   ->setText(qstr_nym_name);
        }
        // -----------------------------------
        // TAB: "CASH PURSE"
        //
        if (m_pCashPurse)
            m_pCashPurse->refresh(strID, strName);
        // -----------------------------------------------------------------------
        FavorLeftSideForIDs();
        // -----------------------------------------------------------------------
        QString qstr_default_acct_id = Moneychanger::It()->get_default_account_id();

        if (strID == qstr_default_acct_id)
            ui->pushButtonMakeDefault->setEnabled(false);
        else
            ui->pushButtonMakeDefault->setEnabled(true);
    }
}

// ------------------------------------------------------

bool MTAccountDetails::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Resize)
    {
        // This insures that the left-most part of the IDs and Names
        // remains visible during all resize events.
        //
        FavorLeftSideForIDs();
    }
//    else
//    {
        // standard event processing
//        return QObject::eventFilter(obj, event);
        return MTEditDetails::eventFilter(obj, event);

        // NOTE: Since the base class has definitely already installed this
        // function as the event filter, I must assume that this version
        // is overriding the version in the base class.
        //
        // Therefore I call the base class version here, since as it's overridden,
        // I don't expect it will otherwise ever get called.
//    }
}



// ------------------------------------------------------

void MTAccountDetails::on_pushButtonSend_clicked()
{
    if (!m_qstrID.isEmpty())
        emit SendFromAcct(m_qstrID);
}

// ------------------------------------------------------

void MTAccountDetails::on_pushButtonRequest_clicked()
{
    if (!m_qstrID.isEmpty())
        emit RequestToAcct(m_qstrID);
}

// ------------------------------------------------------

void MTAccountDetails::on_pushButtonMakeDefault_clicked()
{
    if ((NULL != m_pOwner) && !m_qstrID.isEmpty())
    {
        std::string str_acct_name = OTAPI_Wrap::It()->GetAccountWallet_Name(m_qstrID.toStdString());
        ui->pushButtonMakeDefault->setEnabled(false);
        // --------------------------------------------------
        QString qstrAcctName = QString::fromStdString(str_acct_name);
        // --------------------------------------------------
        emit DefaultAccountChanged(m_qstrID, qstrAcctName);
    }
}

// ------------------------------------------------------

void MTAccountDetails::on_toolButtonAsset_clicked()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        std::string str_acct_id = m_pOwner->m_qstrCurrentID.toStdString();
        // -------------------------------------------------------------------
        QString qstr_id = QString::fromStdString(OTAPI_Wrap::It()->GetAccountWallet_AssetTypeID(str_acct_id));
        // --------------------------------------------------
        emit ShowAsset(qstr_id);
    }
}

void MTAccountDetails::on_toolButtonNym_clicked()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        std::string str_acct_id = m_pOwner->m_qstrCurrentID.toStdString();
        // -------------------------------------------------------------------
        QString qstr_id = QString::fromStdString(OTAPI_Wrap::It()->GetAccountWallet_NymID(str_acct_id));
        // --------------------------------------------------
        emit ShowNym(qstr_id);
    }
}

void MTAccountDetails::on_toolButtonServer_clicked()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        std::string str_acct_id = m_pOwner->m_qstrCurrentID.toStdString();
        // -------------------------------------------------------------------
        QString qstr_id = QString::fromStdString(OTAPI_Wrap::It()->GetAccountWallet_ServerID(str_acct_id));
        // --------------------------------------------------
        emit ShowServer(qstr_id);
    }
}


// ------------------------------------------------------


//virtual
void MTAccountDetails::DeleteButtonClicked()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        // ----------------------------------------------------
        bool bCanRemove = OTAPI_Wrap::It()->Wallet_CanRemoveAccount(m_pOwner->m_qstrCurrentID.toStdString());

        if (!bCanRemove)
        {
            QMessageBox::warning(this, tr("Account Cannot Be Deleted"),
                                 tr("This Account cannot be deleted yet, since it probably doesn't have a zero balance, "
                                         "and probably still has outstanding receipts. (This is where, in the future, "
                                         "you will be given the option to automatically close-out all that stuff and thus delete "
                                         "this Account.)"));
            return;
        }
        // ----------------------------------------------------
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, "", tr("Are you sure you want to delete this Account?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            // TODO: Need to use OT_ME to send a "delete account" message to the server.
            // Only if that is successful, do we set bSuccess here to true.

//          bool bSuccess = OTAPI_Wrap::It()->Wallet_RemoveAccount(m_pOwner->m_qstrCurrentID.toStdString());

            bool bSuccess = false;

            if (!bSuccess)
                QMessageBox::warning(this, tr("Failure Deleting Account"),
                                     tr("Failed trying to delete this Account."));
            else
            {
                m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
                // ------------------------------------------------
                emit RefreshRecordsAndUpdateMenu();
                // ------------------------------------------------
            }
        }
    }
    // ----------------------------------------------------
}


// ------------------------------------------------------

//virtual
void MTAccountDetails::AddButtonClicked()
{
    MTWizardAddAccount theWizard(this);

    theWizard.setWindowTitle(tr("Create Account"));

    if (QDialog::Accepted == theWizard.exec())
    {
        QString qstrName     = theWizard.field("Name") .toString();
        QString qstrAssetID  = theWizard.field("AssetID") .toString();
        QString qstrNymID    = theWizard.field("NymID")   .toString();
        QString qstrServerID = theWizard.field("ServerID").toString();
        // ---------------------------------------------------
        QString qstrAssetName  = QString::fromStdString(OTAPI_Wrap::It()->GetAssetType_Name(qstrAssetID .toStdString()));
        QString qstrNymName    = QString::fromStdString(OTAPI_Wrap::It()->GetNym_Name      (qstrNymID   .toStdString()));
        QString qstrServerName = QString::fromStdString(OTAPI_Wrap::It()->GetServer_Name   (qstrServerID.toStdString()));
        // ---------------------------------------------------
        QMessageBox::information(this, tr("Confirm Create Account"),
                                 QString("%1: '%2'<br/>%3: %4<br/>%5: %6<br/>%7: %8").arg(tr("Confirm Create Account:<br/>Name")).
                                 arg(qstrName).arg(tr("Asset")).arg(qstrAssetName).arg(tr("Nym")).arg(qstrNymName).arg(tr("Server")).arg(qstrServerName));
        // ---------------------------------------------------
        // NOTE: theWizard won't allow each page to finish unless the ID is provided.
        // (Therefore we don't have to check here to see if any of the IDs are empty.)

        // ------------------------------
        // First make sure the Nym is registered at the server, and if not, register him.
        //
        bool bIsRegiseredAtServer = OTAPI_Wrap::It()->IsNym_RegisteredAtServer(qstrNymID.toStdString(),
                                                                         qstrServerID.toStdString());
        if (!bIsRegiseredAtServer)
        {
            OT_ME madeEasy;

            // If the Nym's not registered at the server, then register him first.
            //
            int32_t nSuccess = 0;
            {
                MTSpinner theSpinner;

                std::string strResponse = madeEasy.register_nym(qstrServerID.toStdString(),
                                                                qstrNymID   .toStdString()); // This also does getRequest internally, if success.
                nSuccess                = madeEasy.VerifyMessageSuccess(strResponse);
            }
            // -1 is error,
            //  0 is reply received: failure
            //  1 is reply received: success
            //
            switch (nSuccess)
            {
            case (1):
                {
                    bIsRegiseredAtServer = true;
                    break; // SUCCESS
                }
            case (0):
                {
                    QMessageBox::warning(this, tr("Failed Registration"),
                        tr("Failed while trying to register Nym at Server."));
                    break;
                }
            default:
                {
                    QMessageBox::warning(this, tr("Error in Registration"),
                        tr("Error while trying to register Nym at Server."));
                    break;
                }
            } // switch
            // --------------------------
            if (1 != nSuccess)
            {
                Moneychanger::HasUsageCredits(this, qstrServerID, qstrNymID);
                return;
            }
        }
        // --------------------------
        // Send the request.
        // (Create Account here...)
        //
        OT_ME madeEasy;

        // Send the 'create_asset_acct' message to the server.
        //
        std::string strResponse;
        {
            MTSpinner theSpinner;

            strResponse = madeEasy.create_asset_acct(qstrServerID.toStdString(),
                                                     qstrNymID   .toStdString(),
                                                     qstrAssetID .toStdString());
        }
        // -1 error, 0 failure, 1 success.
        //
        if (1 != madeEasy.VerifyMessageSuccess(strResponse))
        {
            const int64_t lUsageCredits = Moneychanger::HasUsageCredits(this, qstrServerID, qstrNymID);

            // HasUsageCredits already pops up an error box in the cases of -2 and 0.
            //
            if (((-2) != lUsageCredits) && (0 != lUsageCredits))
                QMessageBox::warning(this, tr("Failed Creating Account"),
                    tr("Failed trying to create Account at Server."));
            return;
        }
        // ------------------------------------------------------
        // Get the ID of the new account.
        //
        QString qstrID = QString::fromStdString(OTAPI_Wrap::It()->Message_GetNewAcctID(strResponse));

        if (qstrID.isEmpty())
        {
            QMessageBox::warning(this, tr("Failed Getting new Account ID"),
                                 tr("Failed trying to get the new account's ID from the server response."));
            return;
        }
        // ------------------------------------------------------
        // Set the Name of the new account.
        //
        //bool bNameSet =
                OTAPI_Wrap::It()->SetAccountWallet_Name(qstrID   .toStdString(),
                                                  qstrNymID.toStdString(),
                                                  qstrName .toStdString());
        // -----------------------------------------------
        // Commenting out for now.
        //
//        QMessageBox::information(this, tr("Success!"), QString("%1: '%2'<br/>%3: %4").arg(tr("Success Creating Account!<br/>Name")).
//                                 arg(qstrName).arg(tr("ID")).arg(qstrID));
        // ----------
        m_pOwner->m_map.insert(qstrID, qstrName);
        m_pOwner->SetPreSelected(qstrID);
        // ------------------------------------------------
        emit RefreshRecordsAndUpdateMenu();
        // ------------------------------------------------
    }
}


// ------------------------------------------------------

void MTAccountDetails::on_lineEditName_editingFinished()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        std::string str_acct_id = m_pOwner->m_qstrCurrentID.toStdString();
        std::string str_nym_id  = OTAPI_Wrap::It()->GetAccountWallet_NymID(str_acct_id);

        if (!str_acct_id.empty() && !str_nym_id.empty())
        {
            bool bSuccess = OTAPI_Wrap::It()->SetAccountWallet_Name(str_acct_id,  // Account
                                                              str_nym_id,   // Nym (Account Owner.)
                                                              ui->lineEditName->text().toStdString()); // New Name
            if (bSuccess)
            {
                m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
                m_pOwner->m_map.insert(m_pOwner->m_qstrCurrentID, ui->lineEditName->text());

                m_pOwner->SetPreSelected(m_pOwner->m_qstrCurrentID);
                // ------------------------------------------------
                emit RefreshRecordsAndUpdateMenu();
                // ------------------------------------------------
            }
        }
    }
}

