#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/nymdetails.hpp>
#include <ui_nymdetails.h>

#include <gui/widgets/credentials.hpp>
#include <gui/widgets/wizardaddnym.hpp>

#include <namecoin/Namecoin.hpp>

#include <opentxs/OTAPI.hpp>
#include <opentxs/OTAPI_Exec.hpp>
#include <opentxs/OT_ME.hpp>

#include <QMessageBox>
#include <QPlainTextEdit>
#include <QClipboard>
#include <QDebug>


// ------------------------------------------------------
MTNymDetails::MTNymDetails(QWidget *parent, MTDetailEdit & theOwner) :
    MTEditDetails(parent, theOwner),
    ui(new Ui::MTNymDetails)
{
    ui->setupUi(this);
    this->setContentsMargins(0, 0, 0, 0);
//  this->installEventFilter(this); // NOTE: Successfully tested theory that the base class has already installed this.

    ui->lineEditID->setStyleSheet("QLineEdit { background-color: lightgray }");
    // ----------------------------------
    // Note: This is a placekeeper, so later on I can just erase
    // the widget at 0 and replace it with the real header widget.
    //
    m_pHeaderWidget  = new QWidget;
    ui->verticalLayout->insertWidget(0, m_pHeaderWidget);
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

//virtual
void MTNymDetails::refresh(QString strID, QString strName)
{
//  qDebug() << "MTNymDetails::refresh";

    if ((NULL != ui) && !strID.isEmpty())
    {
        QWidget * pHeaderWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, strID, strName, "", "", ":/icons/icons/identity_BW.png", false);

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
        // --------------------------
        // TAB: "Nym State"
        //
        if (m_pPlainTextEdit)
        {
            QString strContents = QString::fromStdString(OTAPI_Wrap::It()->GetNym_Stats(strID.toStdString()));
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
        bool bCanRemove = OTAPI_Wrap::It()->Wallet_CanRemoveNym(m_pOwner->m_qstrCurrentID.toStdString());

        if (!bCanRemove)
        {
            QMessageBox::warning(this, tr("Nym Cannot Be Deleted"),
                                 tr("This Nym cannot be deleted yet, since it's already been registered on at least one "
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
            bool bSuccess = OTAPI_Wrap::It()->Wallet_RemoveNym(m_pOwner->m_qstrCurrentID.toStdString());

            if (bSuccess)
            {
                m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
                // ------------------------------------------------
                emit RefreshRecordsAndUpdateMenu();
                // ------------------------------------------------
            }
            else
                QMessageBox::warning(this, tr("Failure Deleting Nym"),
                                     tr("Failed trying to delete this Nym."));
        }
    }
    // ----------------------------------------------------
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
        int     nKeysizeIndex   = theWizard.field("Keysize")  .toInt();
        int     nAuthorityIndex = theWizard.field("Authority").toInt();
        QString qstrSource      = theWizard.field("Source")   .toString();
        QString qstrLocation    = theWizard.field("Location") .toString();
        // ---------------------------------------------------
        // NOTE: theWizard won't allow each page to finish unless the data is provided.
        // (Therefore we don't have to check here to see if any of the data is empty.)

        int32_t nKeybits = 1024;

        switch (nKeysizeIndex)
        {
        case 0: // 1024
            nKeybits = 1024;
            break;

        case 1: // 2048
            nKeybits = 2048;
            break;

        case 2: // 4096
            nKeybits = 4096;
            break;

        default:
            qDebug() << QString("%1: %2").arg(tr("Error in keysize selection. Using default")).arg(nKeybits);
            break;
        }
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
        OT_ME madeEasy;

        std::string str_id = madeEasy.create_pseudonym(nKeybits, NYM_ID_SOURCE, ALT_LOCATION);

        if (str_id.empty())
        {
            QMessageBox::warning(this, tr("Failed Creating Nym"),
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
            const unsigned cnt = OTAPI_Wrap::It()->GetNym_CredentialCount (str_id);
            if (cnt != 1)
            {
                qDebug () << "Expected one master credential, got " << cnt
                          << ".  Skipping Namecoin registration.";
            }
            else
            {
                const std::string cred = OTAPI_Wrap::It()->GetNym_CredentialID (str_id, 0);
                const QString qCred = QString::fromStdString (cred);
                NMC_NameManager& nmc = NMC_NameManager::getInstance ();
                nmc.startRegistration (qstrID, qCred);
            }
        }

        // ------------------------------------------------------
        // Set the Name of the new Nym.
        //
        //bool bNameSet =
                OTAPI_Wrap::It()->SetNym_Name(qstrID.toStdString(), qstrID.toStdString(), qstrName.toStdString());
        // -----------------------------------------------
        // Commenting this out for now.
        //
//        QMessageBox::information(this, tr("Success!"), QString("%1: '%2' %3: %4").arg(tr("Success Creating Nym! Name")).
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

void MTNymDetails::on_lineEditName_editingFinished()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        bool bSuccess = OTAPI_Wrap::It()->SetNym_Name(m_pOwner->m_qstrCurrentID.toStdString(), // Nym
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








