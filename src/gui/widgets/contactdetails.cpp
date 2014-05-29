#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/contactdetails.hpp>
#include <ui_contactdetails.h>

#include <gui/widgets/credentials.hpp>
#include <gui/ui/dlgnewcontact.hpp>

#include <gui/widgets/compose.hpp>

#include <core/moneychanger.hpp>
#include <core/handlers/contacthandler.hpp>
#include <core/mtcomms.h>

#include <QComboBox>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QMessageBox>
#include <QDebug>



MTContactDetails::MTContactDetails(QWidget *parent, MTDetailEdit & theOwner) :
    MTEditDetails(parent, theOwner),
    ui(new Ui::MTContactDetails)
{
    ui->setupUi(this);
    this->setContentsMargins(0, 0, 0, 0);

    ui->lineEditID->setStyleSheet("QLineEdit { background-color: lightgray }");

    // ----------------------------------
    // Note: This is a placekeeper, so later on I can just erase
    // the widget at 0 and replace it with the real header widget.
    //
    m_pHeaderWidget  = new QWidget;
    ui->verticalLayout->insertWidget(0, m_pHeaderWidget);
    // ----------------------------------
}

MTContactDetails::~MTContactDetails()
{
    delete ui;
}


// ----------------------------------
//virtual
int MTContactDetails::GetCustomTabCount()
{
    return 2;
}
// ----------------------------------
//virtual
QWidget * MTContactDetails::CreateCustomTab(int nTab)
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
    case 0: // "Credentials" tab
        if (m_pOwner)
        {
            if (m_pCredentials)
            {
                m_pCredentials->setParent(NULL);
                m_pCredentials->disconnect();
                m_pCredentials->deleteLater();

                m_pCredentials = NULL;
            }
            m_pCredentials = new MTCredentials(NULL, *m_pOwner);
            pReturnValue = m_pCredentials;
            pReturnValue->setContentsMargins(0, 0, 0, 0);
        }
        break;

    case 1: // "Known IDs" tab
    {
        if (m_pPlainTextEdit)
        {
            m_pPlainTextEdit->setParent(NULL);
            m_pPlainTextEdit->disconnect();
            m_pPlainTextEdit->deleteLater();

            m_pPlainTextEdit = NULL;
        }
        m_pPlainTextEdit = new QPlainTextEdit;

        m_pPlainTextEdit->setReadOnly(true);
        m_pPlainTextEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        // -------------------------------
        QVBoxLayout * pvBox = new QVBoxLayout;

        QLabel * pLabelContents = new QLabel(tr("Known IDs:"));

        pvBox->setAlignment(Qt::AlignTop);
        pvBox->addWidget   (pLabelContents);
        pvBox->addWidget   (m_pPlainTextEdit);
        // -------------------------------
        pReturnValue = new QWidget;
        pReturnValue->setContentsMargins(0, 0, 0, 0);
        pReturnValue->setLayout(pvBox);
    }
        break;

    default:
        qDebug() << QString("Unexpected: MTContactDetails::CreateCustomTab was called with bad index: %1").arg(nTab);
        return NULL;
    }
    // -----------------------------
    return pReturnValue;
}
// ---------------------------------
//virtual
QString  MTContactDetails::GetCustomTabName(int nTab)
{
    const int nCustomTabCount = this->GetCustomTabCount();
    // -----------------------------
    if ((nTab < 0) || (nTab >= nCustomTabCount))
        return QString(""); // out of bounds.
    // -----------------------------
    QString qstrReturnValue("");
    // -----------------------------
    switch (nTab)
    {
    case 0:  qstrReturnValue = "Credentials";  break;
    case 1:  qstrReturnValue = "Known IDs";    break;

    default:
        qDebug() << QString("Unexpected: MTContactDetails::GetCustomTabName was called with bad index: %1").arg(nTab);
        return QString("");
    }
    // -----------------------------
    return qstrReturnValue;
}
// ------------------------------------------------------



//virtual
void MTContactDetails::DeleteButtonClicked()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        bool bSuccess = MTContactHandler::getInstance()->DeleteContact(m_pOwner->m_qstrCurrentID.toInt());

        if (bSuccess)
        {
            m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
            // ------------------------------------------------
            emit RefreshRecordsAndUpdateMenu();
            // ------------------------------------------------
        }
    }
}

//virtual
void MTContactDetails::AddButtonClicked()
{
    // -----------------------------------------------
    MTDlgNewContact theNewContact(this);
    // -----------------------------------------------
    theNewContact.setWindowTitle(tr("Create New Contact"));
    // -----------------------------------------------
    if (theNewContact.exec() == QDialog::Accepted)
    {
        QString nymID = theNewContact.GetId();

//      qDebug() << QString("MTContactDetails::AddButtonClicked: OKAY was clicked. Value: %1").arg(nymID);

        if (!nymID.isEmpty())
        {
            int nExisting = MTContactHandler::getInstance()->FindContactIDByNymID(nymID);

            if (nExisting > 0)
            {
                QString contactName = MTContactHandler::getInstance()->GetContactName(nExisting);

                QMessageBox::warning(this, tr("Contact Already Exists"),
                                     tr("Contact '%1' already exists with NymID: %2").arg(contactName).arg(nymID));
                return;
            }
            // -------------------------------------------------------
            //else (a contact doesn't already exist for that NymID)
            //
            int nContact  = MTContactHandler::getInstance()->CreateContactBasedOnNym(nymID);

            if (nContact <= 0)
            {
                QMessageBox::warning(this, tr("Failed creating contact"),
                                     tr("Failed trying to create contact for NymID: %1").arg(nymID));
                return;
            }
            // -------------------------------------------------------
            // else (Successfully created the new Contact...)
            // Now let's add this contact to the Map, and refresh the dialog,
            // and then set the new contact as the current one.
            //
            QString qstrContactID = QString("%1").arg(nContact);

            m_pOwner->m_map.insert(qstrContactID, QString("")); // Blank name. (To start.)
            m_pOwner->SetPreSelected(qstrContactID);
            // ------------------------------------------------
            emit RefreshRecordsAndUpdateMenu();
            // ------------------------------------------------
        }
    }
//    else
//    {
//      qDebug() << "MTContactDetails::AddButtonClicked: CANCEL was clicked";
//    }
    // -----------------------------------------------
}


void MTContactDetails::ClearContents()
{
    ui->lineEditID  ->setText("");
    ui->lineEditName->setText("");
    // ------------------------------------------
    if (m_pCredentials)
        m_pCredentials->ClearContents();
    // ------------------------------------------
    if (m_pPlainTextEdit)
        m_pPlainTextEdit->setPlainText("");
    // ------------------------------------------
    ui->pushButtonMsg->setEnabled(false);
    ui->pushButtonMsg->setProperty("contactid", 0);
    // ------------------------------------------
    if (m_pAddresses)
    {
        ui->verticalLayout->removeWidget(m_pAddresses);

        m_pAddresses->setParent(NULL);
        m_pAddresses->disconnect();
        m_pAddresses->deleteLater();

        m_pAddresses = NULL;
    }
}


QGroupBox * MTContactDetails::createAddressGroupBox(QString strContactID)
{
    QGroupBox   * pBox = new QGroupBox(tr("P2P Addresses"));
    QVBoxLayout * vbox = new QVBoxLayout;
    // -----------------------------------------------------------------
    // Loop through all known transport methods (communications addresses)
    // known for this Nym,
    mapIDName theMap;

    qDebug() << QString("CONTACT ID: %1").arg(strContactID);

    int nContactID = strContactID.isEmpty() ? 0 : strContactID.toInt();

    if ((nContactID > 0) && MTContactHandler::getInstance()->GetAddressesByContact(theMap, nContactID, QString("")))
    {
        for (mapIDName::iterator it = theMap.begin(); it != theMap.end(); ++it)
        {
            QString qstrID          = it.key();   // QString("%1|%2").arg(qstrType).arg(qstrAddress)
            QString qstrDisplayAddr = it.value(); // QString("%1: %2").arg(qstrTypeDisplay).arg(qstrAddress);

            QStringList stringlist = qstrID.split("|");

            if (stringlist.size() >= 2) // Should always be 2...
            {
                QString qstrType     = stringlist.at(0);
                QString qstrAddress  = stringlist.at(1);
                // --------------------------------------
                std::string strTypeDisplay = MTComms::displayName(qstrType.toStdString());
                QString    qstrTypeDisplay = QString::fromStdString(strTypeDisplay);

                QWidget * pWidget = this->createSingleAddressWidget(nContactID, qstrType, qstrTypeDisplay, qstrAddress);

                if (NULL != pWidget)
                    vbox->addWidget(pWidget);
            }
        }
    }
    // -----------------------------------------------------------------
    QWidget * pWidget = (nContactID > 0) ? this->createNewAddressWidget(nContactID) : NULL;

    if (NULL != pWidget)
        vbox->addWidget(pWidget);
    // -----------------------------------------------------------------
    pBox->setLayout(vbox);

    return pBox;
}



QWidget * MTContactDetails::createSingleAddressWidget(int nContactID, QString qstrType, QString qstrTypeDisplay, QString qstrAddress)
{
    QWidget     * pWidget    = new QWidget;
    QLineEdit   * pType      = new QLineEdit(qstrTypeDisplay);
    QLabel      * pLabel     = new QLabel(tr("Address:"));
//  QLineEdit   * pAddress   = new QLineEdit(qstrDisplayAddr);
    QLineEdit   * pAddress   = new QLineEdit(qstrAddress);
    QPushButton * pBtnDelete = new QPushButton(tr("Delete"));
    // ----------------------------------------------------------
    pType   ->setMinimumWidth(60);
    pLabel  ->setMinimumWidth(55);
    pLabel  ->setMaximumWidth(55);
    pAddress->setMinimumWidth(60);

    pType   ->setReadOnly(true);
    pAddress->setReadOnly(true);

    pType   ->setStyleSheet("QLineEdit { background-color: lightgray }");
    pAddress->setStyleSheet("QLineEdit { background-color: lightgray }");

    pBtnDelete->setProperty("contactid",    nContactID);
    pBtnDelete->setProperty("methodtype",   qstrType);
    pBtnDelete->setProperty("methodaddr",   qstrAddress);
    pBtnDelete->setProperty("methodwidget", VPtr<QWidget>::asQVariant(pWidget));
    // ----------------------------------------------------------
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(pType);
    layout->addWidget(pLabel);
    layout->addWidget(pAddress);
    layout->addWidget(pBtnDelete);
    // ----------------------------------------------------------
    pWidget->setLayout(layout);

    connect(pBtnDelete, SIGNAL(clicked()), this, SLOT(on_btnAddressDelete_clicked()));
    // ----------------------------------------------------------
    layout->setStretch(0,  0);
    layout->setStretch(1, -1);
    layout->setStretch(2,  0);
    layout->setStretch(3,  1);
    // ----------------------------------------------------------
    pType   ->home(false);
    pAddress->home(false);
    // ----------------------------------------------------------
    return pWidget;
}


QWidget * MTContactDetails::createNewAddressWidget(int nContactID)
{
    QWidget     * pWidget = new QWidget;
    QPushButton * pBtnAdd = new QPushButton(tr("Add"));
    /*
    QString create_msg_method = "CREATE TABLE msg_method"
            " (method_id INTEGER PRIMARY KEY,"   // 1, 2, etc.
            "  method_display_name TEXT,"        // "Localhost"
            "  method_type TEXT,"                // "bitmessage"
            "  method_type_display TEXT,"        // "Bitmessage"
            "  method_connect TEXT)";            // "http://username:password@http://127.0.0.1:8332/"
    */
  //QString create_nym_method
  // = "CREATE TABLE nym_method(nym_id TEXT, method_id INTEGER, address TEXT, PRIMARY KEY(nym_id, method_id, address))";
  //QString create_contact_method
  // = "CREATE TABLE contact_method(contact_id INTEGER, method_type TEXT, address TEXT, PRIMARY KEY(contact_id, method_type, address))";

    QComboBox   * pCombo  = new QComboBox;
    mapIDName     mapMethodTypes;
    MTContactHandler::getInstance()->GetMsgMethodTypes(mapMethodTypes);
    // -----------------------------------------------
    int nIndex = -1;
    for (mapIDName::iterator ii = mapMethodTypes.begin(); ii != mapMethodTypes.end(); ++ii)
    {
        ++nIndex; // 0 on first iteration.
        // ------------------------------
        QString method_type         = ii.key();
        QString method_type_display = ii.value();
        // ------------------------------
        pCombo->insertItem(nIndex, method_type_display, method_type);
    }
    // -----------------------------------------------
    if (mapMethodTypes.size() > 0)
        pCombo->setCurrentIndex(0);
    else
        pBtnAdd->setEnabled(false);
    // -----------------------------------------------
    QLabel      * pLabel   = new QLabel(tr("Address:"));
    QLineEdit   * pAddress = new QLineEdit;
    QHBoxLayout * layout   = new QHBoxLayout;
    // -----------------------------------------------
    pCombo   ->setMinimumWidth(60);
    pLabel   ->setMinimumWidth(55);
    pLabel   ->setMaximumWidth(55);
    pAddress ->setMinimumWidth(60);

    pBtnAdd->setProperty("contactid",    nContactID);
    pBtnAdd->setProperty("methodcombo",  VPtr<QWidget>::asQVariant(pCombo));
    pBtnAdd->setProperty("addressedit",  VPtr<QWidget>::asQVariant(pAddress));
    pBtnAdd->setProperty("methodwidget", VPtr<QWidget>::asQVariant(pWidget));
    // -----------------------------------------------
    layout->addWidget(pCombo);
    layout->addWidget(pLabel);
    layout->addWidget(pAddress);
    layout->addWidget(pBtnAdd);
    // -----------------------------------------------
    pWidget->setLayout(layout);
    // -----------------------------------------------
    layout->setStretch(0,  0);
    layout->setStretch(1, -1);
    layout->setStretch(2,  0);
    layout->setStretch(3,  1);
    // -----------------------------------------------
    connect(pBtnAdd, SIGNAL(clicked()), this, SLOT(on_btnAddressAdd_clicked()));
    // -----------------------------------------------
    return pWidget;
}

void MTContactDetails::on_btnAddressAdd_clicked()
{
    QObject * pqobjSender = QObject::sender();

    if (NULL != pqobjSender)
    {
        QPushButton * pBtnAdd = dynamic_cast<QPushButton *>(pqobjSender);

        if (m_pAddresses && (NULL != pBtnAdd))
        {
            QVariant    varContactID   = pBtnAdd->property("contactid");
            QVariant    varMethodCombo = pBtnAdd->property("methodcombo");
            QVariant    varAddressEdit = pBtnAdd->property("addressedit");
            int         nContactID     = varContactID.toInt();
            QComboBox * pCombo         = VPtr<QComboBox>::asPtr(varMethodCombo);
            QLineEdit * pAddressEdit   = VPtr<QLineEdit>::asPtr(varAddressEdit);
            QWidget   * pWidget        = VPtr<QWidget>::asPtr(pBtnAdd->property("methodwidget"));

            if ((nContactID > 0) && (NULL != pCombo) && (NULL != pAddressEdit) && (NULL != pWidget))
            {
                QString qstrMethodType  = QString("");
                QString qstrAddress     = QString("");
                // --------------------------------------------------
                if (pCombo->currentIndex() < 0)
                    return;
                // --------------------------------------------------
                QVariant varMethodType = pCombo->itemData(pCombo->currentIndex());
                qstrMethodType = varMethodType.toString();

                if (qstrMethodType.isEmpty())
                    return;
                // --------------------------------------------------
                qstrAddress = pAddressEdit->text();

                if (qstrAddress.isEmpty())
                    return;
                // --------------------------------------------------
                bool bAdded = MTContactHandler::getInstance()->AddMsgAddressToContact(nContactID, qstrMethodType, qstrAddress);

                if (bAdded) // Let's add it to the GUI, too, then.
                {
                    QString qstrTypeDisplay = pCombo->currentText();
                    // --------------------------------------------------
                    QLayout     * pLayout = m_pAddresses->layout();
                    QVBoxLayout * pVBox   = (NULL == pLayout) ? NULL : dynamic_cast<QVBoxLayout *>(pLayout);

                    if (NULL != pVBox)
                    {
                        QWidget * pNewWidget = this->createSingleAddressWidget(nContactID, qstrMethodType, qstrTypeDisplay, qstrAddress);

                        if (NULL != pNewWidget)
                            pVBox->insertWidget(pVBox->count()-1, pNewWidget);
                    }
                }
            }
        }
    }
}


void MTContactDetails::on_btnAddressDelete_clicked()
{
    QObject * pqobjSender = QObject::sender();

    if (NULL != pqobjSender)
    {
        QPushButton * pBtnDelete = dynamic_cast<QPushButton *>(pqobjSender);

        if (m_pAddresses && (NULL != pBtnDelete))
        {
            QVariant  varContactID   = pBtnDelete->property("contactid");
            QVariant  varMethodType  = pBtnDelete->property("methodtype");
            QVariant  varMethodAddr  = pBtnDelete->property("methodaddr");
            int       nContactID     = varContactID .toInt();
            QString   qstrMethodType = varMethodType.toString();
            QString   qstrAddress    = varMethodAddr.toString();
            QWidget * pWidget        = VPtr<QWidget>::asPtr(pBtnDelete->property("methodwidget"));

            if (NULL != pWidget)
            {
                bool bRemoved = MTContactHandler::getInstance()->RemoveMsgAddressFromContact(nContactID, qstrMethodType, qstrAddress);

                if (bRemoved) // Let's remove it from the GUI, too, then.
                {
                    QLayout * pLayout = m_pAddresses->layout();

                    if (NULL != pLayout)
                    {
                        pLayout->removeWidget(pWidget);

                        pWidget->setParent(NULL);
                        pWidget->disconnect();
                        pWidget->deleteLater();

                        pWidget = NULL;
                    }
                }
            }
        }
    }
}


void MTContactDetails::on_pushButtonMsg_clicked()
{
    QVariant varContactID = ui->pushButtonMsg->property("contactid");
    int      nContactID   = varContactID.toInt();

    if (nContactID > 0)
    {
        // --------------------------------------------------
        MTCompose * compose_window = new MTCompose;
        compose_window->setAttribute(Qt::WA_DeleteOnClose);
        // --------------------------------------------------
        // If Moneychanger has a default Nym set, we use that for the Sender.
        // (User can always change it.)
        //
        QString qstrDefaultNym = Moneychanger::It()->get_default_nym_id();

        if (!qstrDefaultNym.isEmpty())
            compose_window->setInitialSenderNym(qstrDefaultNym);

        compose_window->setInitialRecipientContactID(nContactID); // We definitely know this, since we're on the Contacts page.
        // --------------------------------------------------
        if (!qstrDefaultNym.isEmpty()) // Sender Nym is set.
        {
            QString qstrTransportType("");
            mapOfCommTypes mapTypes;

            if (MTComms::types(mapTypes) && (mapTypes.size() > 0))
            {
                mapOfCommTypes::iterator it = mapTypes.begin();
                qstrTransportType = QString::fromStdString(it->first);
            } // Likely now qstrTransportType contains "bitmessage"
            // --------------------------------------------------------
            // Do they both support Bitmessage?
            //
            mapIDName mapSenderAddresses, mapRecipientAddresses;

            if (!qstrTransportType.isEmpty() && compose_window->CheckPotentialCommonMsgMethod(qstrTransportType, &mapSenderAddresses, &mapRecipientAddresses))
            {
                compose_window->setInitialMsgType(qstrTransportType);
                // ----------------------------------------------------
                std::string strTypeDisplay = MTComms::displayName(qstrTransportType.toStdString());
                QString    qstrTypeDisplay = QString::fromStdString(strTypeDisplay);

                compose_window->chooseSenderAddress   (mapSenderAddresses,    qstrTypeDisplay, true); //bForce=false by default.
                compose_window->chooseRecipientAddress(mapRecipientAddresses, qstrTypeDisplay);
            }
            // --------------------------------------------------------
            // No? Okay then let's try the default OT server, if one is available.
            else
            {
                QString qstrDefaultServer = Moneychanger::It()->get_default_server_id();

                if (!qstrDefaultServer.isEmpty() && compose_window->setRecipientNymBasedOnContact() &&
                    compose_window->verifySenderAgainstServer   (false, qstrDefaultServer) &&
                    compose_window->verifyRecipientAgainstServer(false, qstrDefaultServer)) // Notice the false? That's so it doesn't pop up a dialog asking questions.
                    // ---------------------------------------------------
                    compose_window->setInitialServer(qstrDefaultServer);
            }
        }
        // --------------------------------------------------
        compose_window->dialog();
        compose_window->show();
        // --------------------------------------------------
        // Recipient has just changed. Does Sender exist? If so, make sure he is
        // compatible with msgtype or find a new one that matches both. We call this
        // here to force the compose dialog to find a matching transport method
        // between sender and recipient, if one isn't already set by now.
        //
        compose_window->FindSenderMsgMethod();
    }
}

//virtual
void MTContactDetails::refresh(QString strID, QString strName)
{
    qDebug() << "MTContactDetails::refresh";

    if ((NULL == ui) || strID.isEmpty())
    {
        ui->pushButtonMsg->setEnabled(false);
        ui->pushButtonMsg->setProperty("contactid", 0);
        return;
    }

    QWidget * pHeaderWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, strID, strName, "", "", ":/icons/icons/user.png", false);

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
    ui->lineEditID  ->setText(strID);
    ui->lineEditName->setText(strName);
    // --------------------------------------------
    QGroupBox * pAddresses = this->createAddressGroupBox(strID);

    if (m_pAddresses)
    {
        ui->verticalLayout->removeWidget(m_pAddresses);

        m_pAddresses->setParent(NULL);
        m_pAddresses->disconnect();
        m_pAddresses->deleteLater();

        m_pAddresses = NULL;
    }
    ui->verticalLayout->addWidget(pAddresses);

    m_pAddresses = pAddresses;
    // ----------------------------------
    QString     strDetails;
    QStringList qstrlistNymIDs;
    // --------------------------------------------
    int nContactID = strID.toInt();
    // ------------------------------------------
    if (nContactID > 0)
    {
        ui->pushButtonMsg->setProperty("contactid", nContactID);
        ui->pushButtonMsg->setEnabled(true);
    }
    else
    {
        ui->pushButtonMsg->setProperty("contactid", 0);
        ui->pushButtonMsg->setEnabled(false);
    }
    // ------------------------------------------
    {
        mapIDName theNymMap;

        if (MTContactHandler::getInstance()->GetNyms(theNymMap, nContactID))
        {
            strDetails += tr("Nyms:\n");

            for (mapIDName::iterator ii = theNymMap.begin(); ii != theNymMap.end(); ii++)
            {
                QString qstrNymID    = ii.key();
                QString qstrNymValue = ii.value();
                // -------------------------------------
                qstrlistNymIDs.append(qstrNymID);
                // -------------------------------------
                strDetails += QString("%1\n").arg(qstrNymID);
                // -------------------------------------
                mapIDName theServerMap;

                if (MTContactHandler::getInstance()->GetServers(theServerMap, qstrNymID))
                {
                    strDetails += tr("Found on servers:\n");

                    for (mapIDName::iterator iii = theServerMap.begin(); iii != theServerMap.end(); iii++)
                    {
                        QString qstrServerID    = iii.key();
                        QString qstrServerValue = iii.value();
                        // -------------------------------------
                        strDetails += QString("%1\n").arg(qstrServerID);
                        // -------------------------------------
                        mapIDName theAccountMap;

                        if (MTContactHandler::getInstance()->GetAccounts(theAccountMap, qstrNymID, qstrServerID, QString("")))
                        {
                            strDetails += tr("Where he owns the accounts:\n");

                            for (mapIDName::iterator iiii = theAccountMap.begin(); iiii != theAccountMap.end(); iiii++)
                            {
                                QString qstrAcctID    = iiii.key();
                                QString qstrAcctValue = iiii.value();
                                // -------------------------------------
                                strDetails += QString("%1\n").arg(qstrAcctID);
                                // -------------------------------------
                            } // for (accounts)
                        } // got accounts
                    } // for (servers)
                    strDetails += QString("\n");
                } // got servers
            } // for (nyms)
            strDetails += QString("\n");
        } // got nyms
    }
    // --------------------------------------------
//    ui->plainTextEdit->setPlainText(strDetails);
    // --------------------------------------------
    // TAB: "Known IDs"
    //
    if (m_pPlainTextEdit)
        m_pPlainTextEdit->setPlainText(strDetails);
    // -----------------------------------
    // TAB: "CREDENTIALS"
    //
    if (m_pCredentials)
        m_pCredentials->refresh(qstrlistNymIDs);
    // -----------------------------------------------------------------------
}


// Add a "delete contact" function that the owner dialog can use.
// It will use this to delete the contact from the SQL db:
//
//         bool MTContactHandler::DeleteContact(int nContactID);

void MTContactDetails::on_lineEditName_editingFinished()
{
    int nContactID = m_pOwner->m_qstrCurrentID.toInt();

    if (nContactID > 0)
    {
        bool bSuccess = MTContactHandler::getInstance()->SetContactName(nContactID, ui->lineEditName->text());

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

