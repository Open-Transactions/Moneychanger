#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/serverdetails.hpp>
#include <ui_serverdetails.h>

#include <gui/widgets/wizardaddcontract.hpp>

#include <opentxs/OTAPI.hpp>
#include <opentxs/OTAPI_Exec.hpp>

#include <QFile>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QDebug>


MTServerDetails::MTServerDetails(QWidget *parent, MTDetailEdit & theOwner) :
    MTEditDetails(parent, theOwner),
    ui(new Ui::MTServerDetails)
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

MTServerDetails::~MTServerDetails()
{
    delete ui;
}


// ----------------------------------
//virtual
int MTServerDetails::GetCustomTabCount()
{
    return 1;
}
// ----------------------------------
//virtual
QWidget * MTServerDetails::CreateCustomTab(int nTab)
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
    case 0:
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

        QLabel * pLabelContents = new QLabel(tr("Server Contract:"));

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
        qDebug() << QString("Unexpected: MTServerDetails::CreateCustomTab was called with bad index: %1").arg(nTab);
        return NULL;
    }
    // -----------------------------
    return pReturnValue;
}
// ---------------------------------
//virtual
QString  MTServerDetails::GetCustomTabName(int nTab)
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
    case 0:  qstrReturnValue = "Server Contract";  break;

    default:
        qDebug() << QString("Unexpected: MTServerDetails::GetCustomTabName was called with bad index: %1").arg(nTab);
        return QString("");
    }
    // -----------------------------
    return qstrReturnValue;
}
// ------------------------------------------------------

void MTServerDetails::FavorLeftSideForIDs()
{
    if (NULL != ui)
    {
        ui->lineEditID  ->home(false);
        ui->lineEditName->home(false);
    }
}

void MTServerDetails::ClearContents()
{
    ui->lineEditID  ->setText("");
    ui->lineEditName->setText("");

    if (m_pPlainTextEdit)
        m_pPlainTextEdit->setPlainText("");
}


// ------------------------------------------------------

bool MTServerDetails::eventFilter(QObject *obj, QEvent *event)
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
void MTServerDetails::DeleteButtonClicked()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        // ----------------------------------------------------
        bool bCanRemove = OTAPI_Wrap::It()->Wallet_CanRemoveServer(m_pOwner->m_qstrCurrentID.toStdString());

        if (!bCanRemove)
        {
            QMessageBox::warning(this, tr("Server Cannot Be Deleted"),
                                 tr("This Server cannot be deleted yet, since you probably have Nyms registered there. "
                                         "(This is where, in the future, you'd be given the option to automatically delete those Nyms.)"));
            return;
        }
        // ----------------------------------------------------
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, "", tr("Are you sure you want to remove this Server Contract?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            bool bSuccess = OTAPI_Wrap::It()->Wallet_RemoveServer(m_pOwner->m_qstrCurrentID.toStdString());

            if (bSuccess)
            {
                m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
                // ------------------------------------------------
                emit RefreshRecordsAndUpdateMenu();
                // ------------------------------------------------
            }
            else
                QMessageBox::warning(this, tr("Failure Removing Server Contract"),
                                     tr("Failed trying to remove this Server Contract."));
        }
    }
    // ----------------------------------------------------
}


// ------------------------------------------------------

void MTServerDetails::DownloadedURL()
{
    QString qstrContents(m_pDownloader->downloadedData());
    // ----------------------------
    if (qstrContents.isEmpty())
    {
        QMessageBox::warning(this, tr("File at URL Was Empty"),
                             tr("File at specified URL was apparently empty"));
        return;
    }
    // ----------------------------
    ImportContract(qstrContents);
}

// ------------------------------------------------------

void MTServerDetails::ImportContract(QString qstrContents)
{
    if (qstrContents.isEmpty())
    {
        QMessageBox::warning(this, tr("Contract is Empty"),
            tr("Failed importing: contract is empty."));
        return;
    }
    // ------------------------------------------------------
    QString qstrContractID = QString::fromStdString(OTAPI_Wrap::It()->CalculateServerContractID(qstrContents.toStdString()));

    if (qstrContractID.isEmpty())
    {
        QMessageBox::warning(this, tr("Failed Calculating Contract ID"),
                             tr("Failed trying to calculate this contract's ID. Perhaps the 'contract' is malformed?"));
        return;
    }
    // ------------------------------------------------------
    else
    {
        // Already in the wallet?
        //
//        std::string str_Contract = OTAPI_Wrap::It()->LoadServerContract(qstrContractID.toStdString());
//
//        if (!str_Contract.empty())
//        {
//            QMessageBox::warning(this, tr("Contract Already in Wallet"),
//                tr("Failed importing this contract, since it's already in the wallet."));
//            return;
//        }
        // ---------------------------------------------------
        int32_t nAdded = OTAPI_Wrap::It()->AddServerContract(qstrContents.toStdString());

        if (1 != nAdded)
        {
            QMessageBox::warning(this, tr("Failed Importing Server Contract"),
                tr("Failed trying to import contract. Is it already in the wallet?"));
            return;
        }
        // -----------------------------------------------
        QString qstrContractName = QString::fromStdString(OTAPI_Wrap::It()->GetServer_Name(qstrContractID.toStdString()));
        // -----------------------------------------------
        // Commenting this out for now.
        //
//      QMessageBox::information(this, tr("Success!"), QString("%1: '%2' %3: %4").arg(tr("Success Importing Server Contract! Name")).
//                               arg(qstrContractName).arg(tr("ID")).arg(qstrContractID));
        // ----------
        m_pOwner->m_map.insert(qstrContractID, qstrContractName);
        m_pOwner->SetPreSelected(qstrContractID);
        // ------------------------------------------------
        emit RefreshRecordsAndUpdateMenu();
        // ------------------------------------------------
    } // if (!qstrContractID.isEmpty())
}

// ------------------------------------------------------

//virtual
void MTServerDetails::AddButtonClicked()
{
    MTWizardAddContract theWizard(this);

    theWizard.setWindowTitle(tr("Add Server Contract"));

    QString qstrDefaultValue("https://raw.github.com/FellowTraveler/Open-Transactions/master/sample-data/sample-contracts/cloud.otc");
    QVariant varDefault(qstrDefaultValue);

    theWizard.setField(QString("URL"), varDefault);

    if (QDialog::Accepted == theWizard.exec())
    {
        bool bIsImporting = theWizard.field("isImporting").toBool();
        bool bIsCreating  = theWizard.field("isCreating").toBool();

        if (bIsImporting)
        {
            bool bIsURL      = theWizard.field("isURL").toBool();
            bool bIsFilename = theWizard.field("isFilename").toBool();
            bool bIsContents = theWizard.field("isContents").toBool();

            if (bIsURL)
            {
                QString qstrURL = theWizard.field("URL").toString();
                // --------------------------------
                if (qstrURL.isEmpty())
                {
                    QMessageBox::warning(this, tr("URL is Empty"),
                        tr("No URL was provided."));

                    return;
                }

                QUrl theURL(qstrURL);
                // --------------------------------
                if (m_pDownloader)
                {
                    m_pDownloader->setParent(NULL);
                    m_pDownloader->disconnect();
                    m_pDownloader->deleteLater();

                    m_pDownloader = NULL;
                }
                // --------------------------------
                m_pDownloader = new FileDownloader(theURL, this);

                connect(m_pDownloader, SIGNAL(downloaded()), SLOT(DownloadedURL()));
            }
            // --------------------------------
            else if (bIsFilename)
            {
                QString fileName = theWizard.field("Filename").toString();

                if (fileName.isEmpty())
                {
                    QMessageBox::warning(this, tr("Filename is Empty"),
                        tr("No filename was provided."));

                    return;
                }
                // -----------------------------------------------
                QString qstrContents;
                QFile plainFile(fileName);

                if (plainFile.open(QIODevice::ReadOnly))//| QIODevice::Text)) // Text flag translates /n/r to /n
                {
                    QTextStream in(&plainFile); // Todo security: check filesize here and place a maximum size.
                    qstrContents = in.readAll();

                    plainFile.close();
                    // ----------------------------
                    if (qstrContents.isEmpty())
                    {
                        QMessageBox::warning(this, tr("File Was Empty"),
                                             QString("%1: %2").arg(tr("File was apparently empty")).arg(fileName));
                        return;
                    }
                    // ----------------------------
                }
                else
                {
                    QMessageBox::warning(this, tr("Failed Reading File"),
                                         QString("%1: %2").arg(tr("Failed trying to read file")).arg(fileName));
                    return;
                }
                // -----------------------------------------------
                ImportContract(qstrContents);
            }
            // --------------------------------
            else if (bIsContents)
            {
                QString qstrContents = theWizard.getContents();

                if (qstrContents.isEmpty())
                {
                    QMessageBox::warning(this, tr("Empty Contract"),
                        tr("Failure Importing: Contract is Empty."));
                    return;
                }
                // -------------------------
                ImportContract(qstrContents);
            }
        }
        // --------------------------------
        else if (bIsCreating)
        {

        }
    }
}

// ------------------------------------------------------

//virtual
void MTServerDetails::refresh(QString strID, QString strName)
{
    if (!strID.isEmpty() && (NULL != ui))
    {
        QWidget * pHeaderWidget = MTEditDetails::CreateDetailHeaderWidget(m_Type, strID, strName, "", "", ":/icons/server", false);

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
        if (m_pPlainTextEdit)
        {
            QString strContents = QString::fromStdString(OTAPI_Wrap::It()->LoadServerContract(strID.toStdString()));
            m_pPlainTextEdit->setPlainText(strContents);
        }
        // --------------------------
    }
}

// ------------------------------------------------------

void MTServerDetails::on_lineEditName_editingFinished()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        bool bSuccess = OTAPI_Wrap::It()->SetServer_Name(m_pOwner->m_qstrCurrentID.toStdString(),  // Server
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

