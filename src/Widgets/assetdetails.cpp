
#include <QDebug>
#include <QMessageBox>

#include "assetdetails.h"
#include "ui_assetdetails.h"

#include "detailedit.h"

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>

MTAssetDetails::MTAssetDetails(QWidget *parent) :
    MTEditDetails(parent),
    ui(new Ui::MTAssetDetails)
{
    ui->setupUi(this);
    this->setContentsMargins(0, 0, 0, 0);
//  this->installEventFilter(this); // NOTE: Successfully tested theory that the base class has already installed this.

    ui->lineEditID->setStyleSheet("QLineEdit { background-color: lightgray }");
}

MTAssetDetails::~MTAssetDetails()
{
    delete ui;
}

void MTAssetDetails::FavorLeftSideForIDs()
{
    if (NULL != ui)
    {
        ui->lineEditID  ->home(false);
        ui->lineEditName->home(false);
    }
}


// ------------------------------------------------------

bool MTAssetDetails::eventFilter(QObject *obj, QEvent *event)
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
void MTAssetDetails::DeleteButtonClicked()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        // ----------------------------------------------------
        bool bCanRemove = OTAPI_Wrap::Wallet_CanRemoveAssetType(m_pOwner->m_qstrCurrentID.toStdString());

        if (!bCanRemove)
        {
            QMessageBox::warning(this, QString("Asset Contract Cannot Be Removed"),
                                 QString("This Asset Contract cannot be removed, since you probably have already created accounts "
                                         "using this asset type. (This is where, in the future, you would be given the option "
                                         "to automatically delete those accounts, and remove this asset contract along with them.)"));
            return;
        }
        // ----------------------------------------------------
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, "", "Are you sure you want to delete this Asset Contract?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            bool bSuccess = OTAPI_Wrap::Wallet_RemoveAssetType(m_pOwner->m_qstrCurrentID.toStdString());

            if (bSuccess)
            {
                m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
                m_pOwner->RefreshRecords();
            }
            else
                QMessageBox::warning(this, QString("Failure Removing Asset Contract"),
                                     QString("Failed trying to remove this Asset Contract."));
        }
    }
    // ----------------------------------------------------
}


// ------------------------------------------------------

//virtual
void MTAssetDetails::AddButtonClicked()
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

//virtual
void MTAssetDetails::refresh(QString strID, QString strName)
{
//  qDebug() << "MTNymDetails::refresh";

    if (NULL != ui)
    {
        ui->lineEditID  ->setText(strID);
        ui->lineEditName->setText(strName);

        FavorLeftSideForIDs();
    }
}

// ------------------------------------------------------

void MTAssetDetails::on_lineEditName_editingFinished()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        bool bSuccess = OTAPI_Wrap::SetAssetType_Name(m_pOwner->m_qstrCurrentID.toStdString(),  // Asset Type
                                                      ui->lineEditName->text(). toStdString()); // New Name
        if (bSuccess)
        {
            m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
            m_pOwner->m_map.insert(m_pOwner->m_qstrCurrentID, ui->lineEditName->text());

            m_pOwner->SetPreSelected(m_pOwner->m_qstrCurrentID);

            m_pOwner->RefreshRecords();
        }
    }
}

// ------------------------------------------------------

