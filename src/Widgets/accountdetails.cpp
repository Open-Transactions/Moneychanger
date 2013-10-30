
#include <QDebug>
#include <QMessageBox>

#include "accountdetails.h"
#include "ui_accountdetails.h"

#include "detailedit.h"

#include "home.h"

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>


MTAccountDetails::MTAccountDetails(QWidget *parent) :
    MTEditDetails(parent),
    m_pLineEdit_Acct_ID(NULL),
    m_pLineEdit_Nym_ID(NULL),
    m_pLineEdit_Server_ID(NULL),
    m_pLineEdit_AssetType_ID(NULL),
    m_pLineEdit_Acct_Name(NULL),
    m_pLineEdit_Nym_Name(NULL),
    m_pLineEdit_Server_Name(NULL),
    m_pLineEdit_AssetType_Name(NULL),
    m_pHeaderWidget(NULL),
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
//    ui->lineEditID    ->setStyleSheet("QLineEdit { background-color: lightgray }");
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
        // TAB: "IDs"
        //
        if (NULL != m_pLineEdit_Acct_ID)
        {
            m_pLineEdit_Acct_ID->home(false);
            m_pLineEdit_Acct_Name->home(false);
            // ------------------------------------
            m_pLineEdit_Nym_ID->home(false);
            m_pLineEdit_Nym_Name->home(false);
            // ------------------------------------
            m_pLineEdit_Server_ID->home(false);
            m_pLineEdit_Server_Name->home(false);
            // ------------------------------------
            m_pLineEdit_AssetType_ID->home(false);
            m_pLineEdit_AssetType_Name->home(false);
        }
        // ------------------------------------
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
    case 0: // "IDs" tab
    {
        QGridLayout * pGridLayout = new QGridLayout;
        int nGridRow = 0;

        pGridLayout->setColumnMinimumWidth(0, 75);
        pGridLayout->setColumnMinimumWidth(1, 75);

        pGridLayout->setColumnStretch(1, 2);
        pGridLayout->setColumnStretch(2, 5);

        pGridLayout->setAlignment(Qt::AlignTop);
        // -----------------------------------------------------------
        {
            QLabel    * pLabel          = new QLabel(tr("Account Name: "));

            m_pLineEdit_Acct_ID         = new QLineEdit;
            m_pLineEdit_Acct_Name       = new QLineEdit;

            m_pLineEdit_Acct_ID->setReadOnly(true);
            m_pLineEdit_Acct_Name->setReadOnly(true);

            m_pLineEdit_Acct_ID->setStyleSheet("QLineEdit { background-color: lightgray }");
            m_pLineEdit_Acct_Name->setStyleSheet("QLineEdit { background-color: lightgray }");

            pGridLayout->addWidget(pLabel,    nGridRow,   0);
            pGridLayout->addWidget(m_pLineEdit_Acct_Name,    nGridRow,   1);
            pGridLayout->addWidget(m_pLineEdit_Acct_ID, nGridRow++, 2);
        }
        // -----------------------------------------------------------
        {
            QLabel    * pLabel          = new QLabel(tr("Asset Type: "));

            m_pLineEdit_AssetType_ID    = new QLineEdit;
            m_pLineEdit_AssetType_Name  = new QLineEdit;

            m_pLineEdit_AssetType_ID->setReadOnly(true);
            m_pLineEdit_AssetType_Name->setReadOnly(true);

            m_pLineEdit_AssetType_ID->setStyleSheet("QLineEdit { background-color: lightgray }");
            m_pLineEdit_AssetType_Name->setStyleSheet("QLineEdit { background-color: lightgray }");

            pGridLayout->addWidget(pLabel, nGridRow, 0);
            pGridLayout->addWidget(m_pLineEdit_AssetType_Name, nGridRow, 1);
            pGridLayout->addWidget(m_pLineEdit_AssetType_ID, nGridRow++, 2);
        }
        // -----------------------------------------------------------
        {
            QLabel    * pLabel          = new QLabel(tr("Owner Nym: "));

            m_pLineEdit_Nym_ID          = new QLineEdit;
            m_pLineEdit_Nym_Name        = new QLineEdit;

            m_pLineEdit_Nym_ID->setReadOnly(true);
            m_pLineEdit_Nym_Name->setReadOnly(true);

            m_pLineEdit_Nym_ID->setStyleSheet("QLineEdit { background-color: lightgray }");
            m_pLineEdit_Nym_Name->setStyleSheet("QLineEdit { background-color: lightgray }");

            pGridLayout->addWidget(pLabel, nGridRow,   0);
            pGridLayout->addWidget(m_pLineEdit_Nym_Name, nGridRow,   1);
            pGridLayout->addWidget(m_pLineEdit_Nym_ID, nGridRow++, 2);
        }
        // -----------------------------------------------------------
        {
            QLabel    * pLabel          = new QLabel(tr("Server: "));

            m_pLineEdit_Server_ID       = new QLineEdit;
            m_pLineEdit_Server_Name     = new QLineEdit;

            m_pLineEdit_Server_ID->setReadOnly(true);
            m_pLineEdit_Server_Name->setReadOnly(true);

            m_pLineEdit_Server_ID->setStyleSheet("QLineEdit { background-color: lightgray }");
            m_pLineEdit_Server_Name->setStyleSheet("QLineEdit { background-color: lightgray }");

            pGridLayout->addWidget(pLabel,    nGridRow,   0);
            pGridLayout->addWidget(m_pLineEdit_Server_Name,    nGridRow,   1);
            pGridLayout->addWidget(m_pLineEdit_Server_ID, nGridRow++, 2);
        }
        // -----------------------------------------------------------
        pReturnValue = new QWidget;
        pReturnValue->setContentsMargins(0, 0, 0, 0);
        pReturnValue->setLayout(pGridLayout);
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
    case 0:  qstrReturnValue = "IDs";  break;

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

    if (NULL != ui)
    {
        QString qstrAmount = MTHome::shortAcctBalance(strID);

        QWidget * pHeaderWidget  = MTEditDetails::CreateDetailHeaderWidget(strID, strName, qstrAmount);

        if (NULL != m_pHeaderWidget)
        {
            ui->verticalLayout->removeWidget(m_pHeaderWidget);
            delete m_pHeaderWidget;
            m_pHeaderWidget = NULL;
        }
        ui->verticalLayout->insertWidget(0, pHeaderWidget);
        m_pHeaderWidget = pHeaderWidget;
        // ----------------------------------
//      ui->lineEditID  ->setText(strID);
        ui->lineEditName->setText(strName);
        // ----------------------------------
        std::string str_server_id = OTAPI_Wrap::GetAccountWallet_ServerID   (strID.toStdString());
        std::string str_asset_id  = OTAPI_Wrap::GetAccountWallet_AssetTypeID(strID.toStdString());
        std::string str_nym_id    = OTAPI_Wrap::GetAccountWallet_NymID      (strID.toStdString());
        // ----------------------------------
        QString qstr_server_id    = QString::fromStdString(str_server_id);
        QString qstr_asset_id     = QString::fromStdString(str_asset_id);
        QString qstr_nym_id       = QString::fromStdString(str_nym_id);
        // ----------------------------------
        QString qstr_server_name  = QString::fromStdString(OTAPI_Wrap::GetServer_Name   (str_server_id));
        QString qstr_asset_name   = QString::fromStdString(OTAPI_Wrap::GetAssetType_Name(str_asset_id));
        QString qstr_nym_name     = QString::fromStdString(OTAPI_Wrap::GetNym_Name      (str_nym_id));
        // ----------------------------------
        // MAIN TAB
        //
        if (!strID.isEmpty())
        {
            ui->lineEditServer->setText(qstr_server_name);
            ui->lineEditAsset ->setText(qstr_asset_name);
            ui->lineEditNym   ->setText(qstr_nym_name);
        }
        // ----------------------------------
        // TAB: "IDs"
        //
        if (NULL != m_pLineEdit_Acct_ID)
        {
            m_pLineEdit_Acct_ID->setText(strID);
            m_pLineEdit_Acct_Name->setText(strName);
            // ------------------------------------
            m_pLineEdit_Nym_ID->setText(qstr_nym_id);
            m_pLineEdit_Nym_Name->setText(qstr_nym_name);
            // ------------------------------------
            m_pLineEdit_Server_ID->setText(qstr_server_id);
            m_pLineEdit_Server_Name->setText(qstr_server_name);
            // ------------------------------------
            m_pLineEdit_AssetType_ID->setText(qstr_asset_id);
            m_pLineEdit_AssetType_Name->setText(qstr_asset_name);
        }
        // ------------------------------------
        FavorLeftSideForIDs();
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


//virtual
void MTAccountDetails::DeleteButtonClicked()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        // ----------------------------------------------------
        bool bCanRemove = OTAPI_Wrap::Wallet_CanRemoveAccount(m_pOwner->m_qstrCurrentID.toStdString());

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

//            bool bSuccess = OTAPI_Wrap::Wallet_RemoveAccount(m_pOwner->m_qstrCurrentID.toStdString());

            bool bSuccess = false;

            if (bSuccess)
            {
                m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
                m_pOwner->RefreshRecords();
            }
            else
                QMessageBox::warning(this, tr("Failure Deleting Account"),
                                     tr("Failed trying to delete this Account."));
        }
    }
    // ----------------------------------------------------
}


// ------------------------------------------------------

//virtual
void MTAccountDetails::AddButtonClicked()
{
    // TODO:

//    // -----------------------------------------------
//    MTDlgNewContact theNewContact(this);
//    // -----------------------------------------------
//    theNewContact.setWindowTitle("Create New Nym");
//    // -----------------------------------------------
//    if (theNewContact.exec() == QDialog::Accepted)
//    {
//        QString nymID = theNewContact.GetId();
//
//        qDebug() << QString("MTContactDetails::AddButtonClicked: OKAY was clicked. Value: %1").arg(nymID);
//
//        //resume
//        // TODO: Use the NymID we just obtained (theNewContact.GetId()) to create a new Contact.
//
//        if (!nymID.isEmpty())
//        {
//            int nExisting = MTContactHandler::getInstance()->FindContactIDByNymID(nymID);
//
//            if (nExisting > 0)
//            {
//                QString contactName = MTContactHandler::getInstance()->GetContactName(nExisting);
//
//                QMessageBox::warning(this, QString("Contact Already Exists"),
//                                     QString("Contact '%1' already exists with NymID: %2").arg(contactName).arg(nymID));
//                return;
//            }
//            // -------------------------------------------------------
//            //else (a contact doesn't already exist for that NymID)
//            //
//            int nContact  = MTContactHandler::getInstance()->CreateContactBasedOnNym(nymID);
//
//            if (nContact <= 0)
//            {
//                QMessageBox::warning(this, QString("Failed creating contact"),
//                                     QString("Failed trying to create contact for NymID: %1").arg(nymID));
//                return;
//            }
//            // -------------------------------------------------------
//            // else (Successfully created the new Contact...)
//            // Now let's add this contact to the Map, and refresh the dialog,
//            // and then set the new contact as the current one.
//            //
//            QString qstrContactID = QString("%1").arg(nContact);
//
//            m_pOwner->m_map.insert(qstrContactID, QString("")); // Blank name. (To start.)
//            m_pOwner->SetPreSelected(qstrContactID);
//            m_pOwner->RefreshRecords();
//        }
//    }


//    else
//    {
//        qDebug() << "MTContactDetails::AddButtonClicked: CANCEL was clicked";
//    }
}


// ------------------------------------------------------

void MTAccountDetails::on_lineEditName_editingFinished()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        std::string str_acct_id = m_pOwner->m_qstrCurrentID.toStdString();
        std::string str_nym_id  = OTAPI_Wrap::GetAccountWallet_NymID(str_acct_id);

        if (!str_acct_id.empty() && !str_nym_id.empty())
        {
            bool bSuccess = OTAPI_Wrap::SetAccountWallet_Name(str_acct_id,  // Account
                                                              str_nym_id,   // Nym (Account Owner.)
                                                              ui->lineEditName->text().toStdString()); // New Name
            if (bSuccess)
            {
                m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
                m_pOwner->m_map.insert(m_pOwner->m_qstrCurrentID, ui->lineEditName->text());

                m_pOwner->SetPreSelected(m_pOwner->m_qstrCurrentID);

                m_pOwner->RefreshRecords();
            }
        }
    }
}

// ------------------------------------------------------

