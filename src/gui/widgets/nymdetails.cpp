#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/nymdetails.hpp>
#include <ui_nymdetails.h>

#include <gui/widgets/credentials.hpp>
#include <gui/widgets/wizardaddnym.hpp>
#include <gui/widgets/overridecursor.hpp>

#include <core/handlers/contacthandler.hpp>
#include <core/mtcomms.h>
#include <core/moneychanger.hpp>

#include <namecoin/Namecoin.hpp>

#include <opentxs/client/OTAPI.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>
#include <opentxs/client/OT_ME.hpp>

#include <QMessageBox>
#include <QStringList>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QPlainTextEdit>
#include <QClipboard>
#include <QDebug>


// ------------------------------------------------------
MTNymDetails::MTNymDetails(QWidget *parent, MTDetailEdit & theOwner) :
    MTEditDetails(parent, theOwner),
    ui(new Ui::MTNymDetails),
    pActionRegister_(nullptr),
    pActionUnregister_(nullptr)
{
    ui->setupUi(this);
    this->setContentsMargins(0, 0, 0, 0);
//  this->installEventFilter(this); // NOTE: Successfully tested theory that the base class has already installed this.

    ui->lineEditID->setStyleSheet("QLineEdit { background-color: lightgray }");
    // ----------------------------------
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    // ----------------------------------
    ui->tableWidget->verticalHeader()->hide();
    // ----------------------------------
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    // ----------------------------------
    ui->tableWidget->setSelectionMode    (QAbstractItemView::SingleSelection);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    // ----------------------------------
    ui->tableWidget->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
    ui->tableWidget->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
    // ----------------------------------
    // Note: This is a placekeeper, so later on I can just erase
    // the widget at 0 and replace it with the real header widget.
    //
    m_pHeaderWidget  = new QWidget;
    ui->verticalLayout->insertWidget(0, m_pHeaderWidget);
    // ----------------------------------
    popupMenu_.reset(new QMenu(this));

    //popupMenu_->addAction("Menu Item 1", this, SLOT(menuItemActivated()));
    pActionRegister_ = popupMenu_->addAction(tr("Register"));
    pActionUnregister_ = popupMenu_->addAction(tr("Unregister"));
    // ----------------------------------
}

// ------------------------------------------------------

MTNymDetails::~MTNymDetails()
{
    delete ui;
}


void MTNymDetails::on_toolButton_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();

    if (NULL != clipboard)
    {
        clipboard->setText(ui->lineEditID->text());

        QMessageBox::information(this, tr("ID copied"), QString("%1:<br/>%2").
                                 arg(tr("Copied Nym ID to the clipboard")).
                                 arg(ui->lineEditID->text()));
    }
}


// ----------------------------------
//virtual
int MTNymDetails::GetCustomTabCount()
{
    return 2;
}
// ----------------------------------
//virtual
QWidget * MTNymDetails::CreateCustomTab(int nTab)
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

    case 1: // "State of Nym" tab
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

        QLabel * pLabelContents = new QLabel(tr("Raw State of Nym:"));

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
        qDebug() << QString("Unexpected: MTNymDetails::CreateCustomTab was called with bad index: %1").arg(nTab);
        return NULL;
    }
    // -----------------------------
    return pReturnValue;
}
// ---------------------------------
//virtual
QString  MTNymDetails::GetCustomTabName(int nTab)
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
    case 1:  qstrReturnValue = "State";        break;

    default:
        qDebug() << QString("Unexpected: MTNymDetails::GetCustomTabName was called with bad index: %1").arg(nTab);
        return QString("");
    }
    // -----------------------------
    return qstrReturnValue;
}
// ------------------------------------------------------

QWidget * MTNymDetails::createNewAddressWidget(QString strNymID)
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
    mapIDName     mapMethods;
    MTContactHandler::getInstance()->GetMsgMethods(mapMethods);
    // -----------------------------------------------
    int nIndex = -1;
    for (mapIDName::iterator ii = mapMethods.begin(); ii != mapMethods.end(); ++ii)
    {
        ++nIndex; // 0 on first iteration.
        // ------------------------------
        QString method_id   = ii.key();
        QString method_name = ii.value();
        // ------------------------------
        pCombo->insertItem(nIndex, method_name, method_id);
    }
    // -----------------------------------------------
    if (mapMethods.size() > 0)
    {
        pWidget->setVisible(true);
        pCombo->setCurrentIndex(0);
    }
    else
    {
        pWidget->setVisible(false);
        pBtnAdd->setEnabled(false);
    }
    // -----------------------------------------------
    QLabel      * pLabel   = new QLabel(tr("Address:"));
    QLineEdit   * pAddress = new QLineEdit;
    QHBoxLayout * layout   = new QHBoxLayout;
    // -----------------------------------------------
    pCombo   ->setMinimumWidth(60);
    pLabel   ->setMinimumWidth(55);
    pLabel   ->setMaximumWidth(55);
    pAddress ->setMinimumWidth(60);

    pBtnAdd->setProperty("nymid",        strNymID);
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

void MTNymDetails::on_btnAddressAdd_clicked()
{
    QObject * pqobjSender = QObject::sender();

    if (NULL != pqobjSender)
    {
        QPushButton * pBtnAdd = dynamic_cast<QPushButton *>(pqobjSender);

        if (m_pAddresses && (NULL != pBtnAdd))
        {
            QVariant    varNymID       = pBtnAdd->property("nymid");
            QVariant    varMethodCombo = pBtnAdd->property("methodcombo");
            QVariant    varAddressEdit = pBtnAdd->property("addressedit");
            QString     qstrNymID      = varNymID.toString();
            QComboBox * pCombo         = VPtr<QComboBox>::asPtr(varMethodCombo);
            QLineEdit * pAddressEdit   = VPtr<QLineEdit>::asPtr(varAddressEdit);
            QWidget   * pWidget        = VPtr<QWidget>::asPtr(pBtnAdd->property("methodwidget"));

            if (!qstrNymID.isEmpty() && (NULL != pCombo) && (NULL != pAddressEdit) && (NULL != pWidget))
            {
                int     nMethodID       = 0;
                QString qstrAddress     = QString("");
                QString qstrDisplayAddr = QString("");
                // --------------------------------------------------
                if (pCombo->currentIndex() < 0)
                    return;
                // --------------------------------------------------
                QVariant varMethodID = pCombo->itemData(pCombo->currentIndex());
                nMethodID = varMethodID.toInt();

                if (nMethodID <= 0)
                    return;
                // --------------------------------------------------
                qstrAddress = pAddressEdit->text();

                if (qstrAddress.isEmpty())
                    return;
                // --------------------------------------------------
                bool bAdded = MTContactHandler::getInstance()->AddMsgAddressToNym(qstrNymID, nMethodID, qstrAddress);

                if (bAdded) // Let's add it to the GUI, too, then.
                {
                    QString qstrTypeDisplay = MTContactHandler::getInstance()->GetMethodTypeDisplay(nMethodID);
                    qstrDisplayAddr = QString("%1: %2").arg(qstrTypeDisplay).arg(qstrAddress);
                    // --------------------------------------------------
                    QLayout     * pLayout = m_pAddresses->layout();
                    QVBoxLayout * pVBox   = (NULL == pLayout) ? NULL : dynamic_cast<QVBoxLayout *>(pLayout);

                    if (NULL != pVBox)
                    {
                        QWidget * pNewWidget = this->createSingleAddressWidget(qstrNymID, nMethodID, qstrAddress, qstrAddress);

                        if (NULL != pNewWidget)
                            pVBox->insertWidget(pVBox->count()-1, pNewWidget);
                    }
                }
            }
        }
    }
}

void MTNymDetails::on_btnAddressDelete_clicked()
{
    QObject * pqobjSender = QObject::sender();

    if (NULL != pqobjSender)
    {
        QPushButton * pBtnDelete = dynamic_cast<QPushButton *>(pqobjSender);

        if (m_pAddresses && (NULL != pBtnDelete))
        {
            QVariant  varNymID      = pBtnDelete->property("nymid");
            QVariant  varMethodID   = pBtnDelete->property("methodid");
            QVariant  varMethodAddr = pBtnDelete->property("methodaddr");
            QString   qstrNymID     = varNymID     .toString();
            int       nMethodID     = varMethodID  .toInt();
            QString   qstrAddress   = varMethodAddr.toString();
            QWidget * pWidget       = VPtr<QWidget>::asPtr(pBtnDelete->property("methodwidget"));

            if (NULL != pWidget)
            {
                bool bRemoved = MTContactHandler::getInstance()->RemoveMsgAddressFromNym(qstrNymID, nMethodID, qstrAddress);

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


QWidget * MTNymDetails::createSingleAddressWidget(QString strNymID, int nMethodID, QString qstrAddress, QString qstrDisplayAddr)
{
    QString qstrMethodName = (nMethodID > 0) ? MTContactHandler::getInstance()->GetMethodDisplayName(nMethodID) : QString(tr("Unknown"));

    QWidget     * pWidget    = new QWidget;
    QLineEdit   * pMethod    = new QLineEdit(qstrMethodName);
    QLabel      * pLabel     = new QLabel(tr("Address:"));
    QLineEdit   * pAddress   = new QLineEdit(qstrDisplayAddr);
    QPushButton * pBtnDelete = new QPushButton(tr("Delete"));
    // ----------------------------------------------------------
    pMethod ->setMinimumWidth(60);
    pLabel  ->setMinimumWidth(55);
    pLabel  ->setMaximumWidth(55);
    pAddress->setMinimumWidth(60);

    pMethod ->setReadOnly(true);
    pAddress->setReadOnly(true);

    pMethod ->setStyleSheet("QLineEdit { background-color: lightgray }");
    pAddress->setStyleSheet("QLineEdit { background-color: lightgray }");

    pBtnDelete->setProperty("nymid",        strNymID);
    pBtnDelete->setProperty("methodid",     nMethodID);
    pBtnDelete->setProperty("methodaddr",   qstrAddress);
    pBtnDelete->setProperty("methodwidget", VPtr<QWidget>::asQVariant(pWidget));
    // ----------------------------------------------------------
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(pMethod);
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
    pMethod ->home(false);
    pAddress->home(false);
    // ----------------------------------------------------------
    return pWidget;
}

QGroupBox * MTNymDetails::createAddressGroupBox(QString strNymID)
{
    QGroupBox   * pBox = new QGroupBox(tr("P2P Addresses"));
    QVBoxLayout * vbox = new QVBoxLayout;
    // -----------------------------------------------------------------
    int nAddressesAdded = 0;
    // -----------------------------------------------------------------
    // Loop through all known transport methods (communications addresses)
    // known for this Nym,
    mapIDName theMap;

    if (!strNymID.isEmpty() && MTContactHandler::getInstance()->GetMethodsAndAddrByNym(theMap, strNymID))
    {
        for (mapIDName::iterator it = theMap.begin(); it != theMap.end(); ++it)
        {
            QString qstrID          = it.key();   // QString("%1|%2").arg(qstrAddress).arg(nMethodID);
            QString qstrDisplayAddr = it.value(); // QString("%1: %2").arg(qstrTypeDisplay).arg(qstrAddress);

            QStringList stringlist = qstrID.split("|");

            if (stringlist.size() >= 2) // Should always be 2...
            {
                QString qstrAddress  = stringlist.at(0);
                QString qstrMethodID = stringlist.at(1);
                const int  nMethodID = qstrMethodID.isEmpty() ? 0 : qstrMethodID.toInt();
                // --------------------------------------
                QWidget * pWidget = this->createSingleAddressWidget(strNymID, nMethodID, qstrAddress, qstrAddress);

                if (NULL != pWidget)
                {
                    nAddressesAdded++;
                    vbox->addWidget(pWidget);
                }
            }
        }
    }
    // -----------------------------------------------------------------
    QWidget * pWidget = this->createNewAddressWidget(strNymID);

    if (NULL != pWidget)
        vbox->addWidget(pWidget);
    // -----------------------------------------------------------------
    pBox->setLayout(vbox);
    // -----------------------------------------------------------------
    if (pWidget->isHidden() && (0 == nAddressesAdded))
        pBox->hide();
    // -----------------------------------------------------------------
    return pBox;
}

//virtual
void MTNymDetails::refresh(QString strID, QString strName)
{
//  qDebug() << "MTNymDetails::refresh";

    if ((NULL != ui) && !strID.isEmpty())
    {
        QWidget * pHeaderWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, strID, strName,
                                                                           "", "", ":/icons/icons/identity_BW.png",
                                                                           false);

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

        FavorLeftSideForIDs();
        // ----------------------------------------------------------------
        clearNotaryTable();
        // ----------------------------------------------------------------
        ui->tableWidget->blockSignals(true);
        // ----------------------------
        std::string nymId   = strID.toStdString();
        const int32_t serverCount = opentxs::OTAPI_Wrap::It()->GetServerCount();

        for (int32_t serverIndex = 0; serverIndex < serverCount; ++serverIndex)
        {
            std::string NotaryID   = opentxs::OTAPI_Wrap::It()->GetServer_ID(serverIndex);
            QString qstrNotaryID   = QString::fromStdString(NotaryID);
            QString qstrNotaryName = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetServer_Name(NotaryID));
            bool    bStatus        = opentxs::OTAPI_Wrap::It()->IsNym_RegisteredAtServer(nymId, NotaryID);
            QString qstrStatus     = bStatus ? tr("Registered") : tr("Not registered");
            // ----------------------------------
            int column = 0;
            // ----------------------------------
            ui->tableWidget->insertRow(0);
            QTableWidgetItem * item = nullptr;
            // ----------------------------------
            item = new QTableWidgetItem(qstrNotaryName);
            item->setFlags(item->flags() &  ~Qt::ItemIsEditable);
            ui->tableWidget->setItem(0, column++, item);
            // ----------------------------------
            item = new QTableWidgetItem(qstrStatus);
            item->setFlags(item->flags() &  ~Qt::ItemIsEditable);
            ui->tableWidget->setItem(0, column++, item);
            // ----------------------------------
            item = new QTableWidgetItem(qstrNotaryID);
            item->setFlags(item->flags() &  ~Qt::ItemIsEditable);
            ui->tableWidget->setItem(0, column++, item);
            // ----------------------------------
            ui->tableWidget->item(0,0)->setData(Qt::UserRole, QVariant(qstrNotaryID));
        }
        if (serverCount < 1)
            ui->tableWidget->setCurrentCell(-1,0);
        else
            ui->tableWidget->setCurrentCell(0,0);
        // ----------------------------
        ui->tableWidget->blockSignals(false);
        // ----------------------------------------------------------------
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
        // TAB: "Nym State"
        //
        if (m_pPlainTextEdit)
        {
            QString strContents = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetNym_Stats(strID.toStdString()));
            m_pPlainTextEdit->setPlainText(strContents);
        }
        // -----------------------------------
        // TAB: "CREDENTIALS"
        //
        if (m_pCredentials)
        {
            QStringList qstrlistNymIDs;
            qstrlistNymIDs.append(strID);

            m_pCredentials->refresh(qstrlistNymIDs);
        }
        // -----------------------------------------------------------------------
    }
}

void MTNymDetails::clearNotaryTable()
{
    ui->tableWidget->blockSignals(true);
    // -----------------------------------
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount (0);
    // -----------------------------------
    ui->tableWidget->setCurrentCell(-1, 0);
    ui->tableWidget->blockSignals(false);
}

void MTNymDetails::ClearContents()
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
    clearNotaryTable();
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
//        return QWidget::eventFilter(obj, event);
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
        bool bCanRemove = opentxs::OTAPI_Wrap::It()->Wallet_CanRemoveNym(m_pOwner->m_qstrCurrentID.toStdString());

        if (!bCanRemove)
        {
            QMessageBox::warning(this, tr("Moneychanger"),
                                 tr("For your protection, Nyms already registered on a notary cannot be summarily deleted. "
                                    "Please unregister first. (You may also delete need to any accounts you may have registered "
                                    "at that same notary using the same Nym.)"));
            return;
        }
        // ----------------------------------------------------
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, tr("Moneychanger"), tr("Are you sure you want to delete this Nym?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            bool bSuccess = opentxs::OTAPI_Wrap::It()->Wallet_RemoveNym(m_pOwner->m_qstrCurrentID.toStdString());

            if (bSuccess)
            {
                m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
                // ------------------------------------------------
                emit RefreshRecordsAndUpdateMenu();
                // ------------------------------------------------
            }
            else
                QMessageBox::warning(this, tr("Moneychanger"),
                                     tr("Failed trying to delete this Nym."));
        }
    }
}


// ------------------------------------------------------

//virtual
void MTNymDetails::AddButtonClicked()
{
    MTWizardAddNym theWizard(this);

    theWizard.setWindowTitle(tr("Create Nym (a.k.a. Create Identity)"));

    if (QDialog::Accepted == theWizard.exec())
    {
        QString qstrName        = theWizard.field("Name")     .toString();
        int     nAuthorityIndex = theWizard.field("Authority").toInt();
        int     nAlgorithmIndex = theWizard.field("Algorithm").toInt();
        QString qstrSource      = theWizard.field("Source")   .toString();
        QString qstrLocation    = theWizard.field("Location") .toString();
        // ---------------------------------------------------
        // NOTE: theWizard won't allow each page to finish unless the data is provided.
        // (Therefore we don't have to check here to see if any of the data is empty.)

        // -------------------------------------------
        std::string NYM_ID_SOURCE("");

        if (0 != nAuthorityIndex) // Zero would be Self-Signed, which needs no source.
            NYM_ID_SOURCE = qstrSource.toStdString();
        // -------------------------------------------
        std::string ALT_LOCATION("");

        if (!qstrLocation.isEmpty())
            ALT_LOCATION = qstrLocation.toStdString();
        // -------------------------------------------
        // Create Nym here...
        //
        opentxs::OT_ME madeEasy;
        std::string    str_id;

        switch (nAlgorithmIndex)
        {
            case 0:  // ECDSA
                str_id = madeEasy.create_nym_ecdsa(NYM_ID_SOURCE, ALT_LOCATION);
                break;
            case 1: // 1024-bit RSA
                str_id = madeEasy.create_nym_legacy(1024, NYM_ID_SOURCE, ALT_LOCATION);
                break;
//            case 2: // 2048-bit RSA
//                str_id = madeEasy.create_nym_legacy(2048, NYM_ID_SOURCE, ALT_LOCATION);
//                break;
//            case 3: // 4096-bit RSA
//                str_id = madeEasy.create_nym_legacy(4096, NYM_ID_SOURCE, ALT_LOCATION);
//                break;
//            case 4: // 8192-bit RSA
//                str_id = madeEasy.create_nym_legacy(8192, NYM_ID_SOURCE, ALT_LOCATION);
//                break;
            default:
                QMessageBox::warning(this, tr("Moneychanger"),
                    tr("Unexpected key type."));
                return;
        }
        // --------------------------------------------------
        if (str_id.empty())
        {
            QMessageBox::warning(this, tr("Moneychanger"),
                tr("Failed trying to create Nym."));
            return;
        }
        // ------------------------------------------------------
        // Get the ID of the new nym.
        //
        QString qstrID = QString::fromStdString(str_id);
        // ------------------------------------------------------
        // Register the Namecoin name.
        if (nAuthorityIndex == 1)
        {
            const unsigned cnt = opentxs::OTAPI_Wrap::It()->GetNym_MasterCredentialCount (str_id);
            if (cnt != 1)
            {
                qDebug () << "Expected one master credential, got " << cnt
                          << ".  Skipping Namecoin registration.";
            }
            else
            {
                const std::string cred = opentxs::OTAPI_Wrap::It()->GetNym_MasterCredentialID (str_id, 0);
                const QString qCred = QString::fromStdString (cred);
                NMC_NameManager& nmc = NMC_NameManager::getInstance ();
                nmc.startRegistration (qstrID, qCred);
            }
        }

        // ------------------------------------------------------
        // Set the Name of the new Nym.
        //
        //bool bNameSet =
                opentxs::OTAPI_Wrap::It()->SetNym_Name(qstrID.toStdString(), qstrID.toStdString(), qstrName.toStdString());
        // -----------------------------------------------
        // Commenting this out for now.
        //
//      QMessageBox::information(this, tr("Success!"), QString("%1: '%2' %3: %4").arg(tr("Success Creating Nym! Name")).
//                               arg(qstrName).arg(tr("ID")).arg(qstrID));
        // ----------
        m_pOwner->m_map.insert(qstrID, qstrName);
        m_pOwner->SetPreSelected(qstrID);
        // ------------------------------------------------
        emit newNymAdded(qstrID);
        // ------------------------------------------------
    }
}

// ------------------------------------------------------

void MTNymDetails::on_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        QString qstrNymID(m_pOwner->m_qstrCurrentID);
        QString qstrNymName(m_pOwner->m_qstrCurrentName);
        const std::string str_nym_id = qstrNymID.toStdString();
        // ----------------------------------------------------
        QTableWidgetItem * pItem = ui->tableWidget->itemAt(pos);

        if (NULL != pItem)
        {
            int nRow = pItem->row();

            if (nRow >= 0)
            {
                QString qstrNotaryID = ui->tableWidget->item(nRow, 0)->data(Qt::UserRole).toString();
                std::string str_notary_id = qstrNotaryID.toStdString();
                QString qstrNotaryName = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetServer_Name(str_notary_id));
                // ------------------------
                QPoint globalPos = ui->tableWidget->mapToGlobal(pos);
                // ------------------------
                const QAction* selectedAction = popupMenu_->exec(globalPos); // Here we popup the menu, and get the user's click.
                // ------------------------
                if (selectedAction == pActionRegister_)
                {
                    if (opentxs::OTAPI_Wrap::It()->IsNym_RegisteredAtServer(str_nym_id, str_notary_id))
                    {
                        QMessageBox::information(this, tr("Moneychanger"), QString("%1 '%2' %3 '%4'.").arg(tr("The Nym")).
                                                 arg(qstrNymName).arg(tr("is already registered on notary")).arg(qstrNotaryName));
                        return;
                    }
                    else
                    {
                        opentxs::OT_ME madeEasy;

                        int32_t nSuccess = 0;
                        bool    bRegistered = false;
                        {
                            MTSpinner theSpinner;

                            std::string strResponse = madeEasy.register_nym(str_notary_id, str_nym_id);
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
                                bRegistered = true;

                                QMessageBox::information(this, tr("Moneychanger"),
                                    tr("Success!"));

                                break; // SUCCESS
                            }
                        case (0):
                            {
                                QMessageBox::warning(this, tr("Moneychanger"),
                                    tr("Failed while trying to register nym on Server."));
                                break;
                            }
                        default:
                            {
                                QMessageBox::warning(this, tr("Moneychanger"),
                                    tr("Error while trying to register nym on Server."));
                                break;
                            }
                        } // switch
                        // --------------------------
                        if (1 != nSuccess)
                        {
                            Moneychanger::It()->HasUsageCredits(QString::fromStdString(str_notary_id), QString::fromStdString(str_nym_id));
                            return;
                        }
                    }
                }
                // ------------------------
                else if (selectedAction == pActionUnregister_)
                {
                    if (!opentxs::OTAPI_Wrap::It()->IsNym_RegisteredAtServer(str_nym_id, str_notary_id))
                    {
                        QMessageBox::information(this, tr("Moneychanger"), QString("%1 '%2' %3 '%4'.").arg(tr("The Nym")).
                                                 arg(qstrNymName).arg(tr("is already not registered on notary")).arg(qstrNotaryName));
                        return;
                    }
                    else
                    {
                        // See if Nym even CAN unregister.
                        // (Not if he has any accounts there -- he must delete those accounts first.)
                        //
                        mapIDName accountMap;
                        if (MTContactHandler::getInstance()->GetAccounts(accountMap, qstrNymID, qstrNotaryID, QString("")))
                        {
                            QMessageBox::information(this, tr("Moneychanger"), QString("%1 '%2' %3 '%4'. %5.").arg(tr("The Nym")).
                                                     arg(qstrNymName).arg(tr("still has asset accounts on the notary")).arg(qstrNotaryName).arg(tr("Please delete those first")));
                            return;
                        }
                        // -----------------------------------
                        QMessageBox::StandardButton reply;
                        reply = QMessageBox::question(this, tr("Moneychanger"),
                                                      tr("Are you sure you want to unregister your Nym from this notary?"),
                                                      QMessageBox::Yes|QMessageBox::No);
                        if (reply == QMessageBox::Yes)
                        {
                            opentxs::OT_ME madeEasy;

                            int32_t nSuccess = 0;
                            bool    bUnregistered = false;
                            {
                                MTSpinner theSpinner;

                                std::string strResponse = madeEasy.unregister_nym(str_notary_id, str_nym_id);
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
                                    bUnregistered = true;

                                    QMessageBox::information(this, tr("Moneychanger"),
                                        tr("Success!"));

                                    break; // SUCCESS
                                }
                            case (0):
                                {
                                    QMessageBox::warning(this, tr("Moneychanger"),
                                        tr("Failed while trying to unregister nym from Server."));
                                    break;
                                }
                            default:
                                {
                                    QMessageBox::warning(this, tr("Moneychanger"),
                                        tr("Error while trying to unregister nym from Server."));
                                    break;
                                }
                            } // switch
                            // --------------------------
                            if (1 != nSuccess)
                            {
                                Moneychanger::It()->HasUsageCredits(QString::fromStdString(str_notary_id), QString::fromStdString(str_nym_id));
                                return;
                            }
                        }
                    }
                }
            } // nRow >= 0
        } // pItem not NULL.
    }
}

// ------------------------------------------------------

void MTNymDetails::on_lineEditName_editingFinished()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        bool bSuccess = opentxs::OTAPI_Wrap::It()->SetNym_Name(m_pOwner->m_qstrCurrentID.toStdString(), // Nym
                                                m_pOwner->m_qstrCurrentID.toStdString(), // Signer
                                                ui->lineEditName->text(). toStdString()); // New Name
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

// ------------------------------------------------------









