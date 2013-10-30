
#include <QDebug>
#include <QMessageBox>

#include "nymdetails.h"
#include "ui_nymdetails.h"

#include "detailedit.h"

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>


// ------------------------------------------------------

MTNymDetails::MTNymDetails(QWidget *parent) :
    MTEditDetails(parent),
    ui(new Ui::MTNymDetails)
{
    ui->setupUi(this);
    this->setContentsMargins(0, 0, 0, 0);
//  this->installEventFilter(this); // NOTE: Successfully tested theory that the base class has already installed this.

    ui->lineEditID->setStyleSheet("QLineEdit { background-color: lightgray }");
}

// ------------------------------------------------------

MTNymDetails::~MTNymDetails()
{
    delete ui;
}

// ------------------------------------------------------

void MTNymDetails::FavorLeftSideForIDs()
{
    if (NULL != ui)
    {
        ui->lineEditID  ->home(false);
        ui->lineEditName->home(false);
    }
}

// ------------------------------------------------------

bool MTNymDetails::eventFilter(QObject *obj, QEvent *event)
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
void MTNymDetails::DeleteButtonClicked()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        // ----------------------------------------------------
        bool bCanRemove = OTAPI_Wrap::Wallet_CanRemoveNym(m_pOwner->m_qstrCurrentID.toStdString());

        if (!bCanRemove)
        {
            QMessageBox::warning(this, QString("Nym Cannot Be Deleted"),
                                 QString("This Nym cannot be deleted yet, since it's already been registered on at least one "
                                         "server, and perhaps even owns an asset account or two. (This is where, in the future, "
                                         "you will be given the option to automatically delete all that stuff and thus delete "
                                         "this Nym.)"));
            return;
        }
        // ----------------------------------------------------
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, "", "Are you sure you want to delete this Nym?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            bool bSuccess = OTAPI_Wrap::Wallet_RemoveNym(m_pOwner->m_qstrCurrentID.toStdString());

            if (bSuccess)
            {
                m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
                m_pOwner->RefreshRecords();
            }
            else
                QMessageBox::warning(this, QString("Failure Deleting Nym"),
                                     QString("Failed trying to delete this Nym."));
        }
    }
    // ----------------------------------------------------
}


// ------------------------------------------------------

//virtual
void MTNymDetails::AddButtonClicked()
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
void MTNymDetails::refresh(QString strID, QString strName)
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

void MTNymDetails::on_lineEditName_editingFinished()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        bool bSuccess = OTAPI_Wrap::SetNym_Name(m_pOwner->m_qstrCurrentID.toStdString(), // Nym
                                                m_pOwner->m_qstrCurrentID.toStdString(), // Signer
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










