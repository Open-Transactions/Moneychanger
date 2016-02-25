#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/serverdetails.hpp>
#include <ui_serverdetails.h>

#include <gui/widgets/wizardaddcontract.hpp>

#include <opentxs/client/OpenTransactions.hpp>
#include <opentxs/client/OTAPI.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>
#include <opentxs/client/OTWallet.hpp>
#include <opentxs/core/OTStorage.hpp>
#include <opentxs/core/contract/ServerContract.hpp>
#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/crypto/OTCachedKey.hpp>
#include <opentxs/core/util/OTFolders.hpp>

#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QDebug>
#include <QClipboard>
#include <QFileDialog>


MTServerDetails::MTServerDetails(QWidget *parent, MTDetailEdit & theOwner) :
    MTEditDetails(parent, theOwner),
    ui(new Ui::MTServerDetails)
{
    ui->setupUi(this);
    this->setContentsMargins(0, 0, 0, 0);
//  this->installEventFilter(this); // NOTE: Successfully tested theory that the base class has already installed this.

    ui->lineEditID   ->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditNymID->setStyleSheet("QLineEdit { background-color: lightgray }");
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




void MTServerDetails::on_toolButtonNotary_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();

    if (NULL != clipboard)
    {
        clipboard->setText(ui->lineEditID->text());

        QMessageBox::information(this, tr("ID copied"), QString("%1:<br/>%2").
                                 arg(tr("Copied Notary ID to the clipboard")).
                                 arg(ui->lineEditID->text()));
    }
}


void MTServerDetails::on_toolButtonNym_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();

    if (NULL != clipboard)
    {
        clipboard->setText(ui->lineEditNymID->text());

        QMessageBox::information(this, tr("ID copied"), QString("%1:<br/>%2").
                                 arg(tr("Copied Signer Nym ID to the clipboard")).
                                 arg(ui->lineEditNymID->text()));
    }
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

        QLabel * pLabelContents = new QLabel(tr("Server contract:"));

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
        ui->lineEditID   ->home(false);
        ui->lineEditName ->home(false);
        ui->lineEditNymID->home(false);
    }
}

void MTServerDetails::ClearContents()
{
    ui->lineEditID   ->setText("");
    ui->lineEditName ->setText("");
    ui->lineEditNymID->setText("");

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
void MTServerDetails::DeleteButtonClicked()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        // ----------------------------------------------------
        bool bCanRemove = opentxs::OTAPI_Wrap::It()->Wallet_CanRemoveServer(m_pOwner->m_qstrCurrentID.toStdString());

        if (!bCanRemove)
        {
            QMessageBox::warning(this, tr("Moneychanger"),
                                 tr("This server contract cannot be removed, because there are still accounts and/or nyms in the wallet that are registered there. Please unregister the relevant nyms, and/or delete the relevant accounts, first."));
            return;
        }
        // ----------------------------------------------------
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, "", tr("Are you sure you want to remove this Server Contract?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            bool bSuccess = opentxs::OTAPI_Wrap::It()->Wallet_RemoveServer(m_pOwner->m_qstrCurrentID.toStdString());

            if (bSuccess)
            {
                m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
                // ------------------------------------------------
                emit serversChanged();
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
    QString qstrContractID = QString::fromStdString(opentxs::OTAPI_Wrap::It()->CalculateServerContractID(qstrContents.toStdString()));

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
//        std::string str_Contract = opentxs::OTAPI_Wrap::It()->LoadServerContract(qstrContractID.toStdString());
//
//        if (!str_Contract.empty())
//        {
//            QMessageBox::warning(this, tr("Contract Already in Wallet"),
//                tr("Failed importing this contract, since it's already in the wallet."));
//            return;
//        }
        // ---------------------------------------------------
        int32_t nAdded = opentxs::OTAPI_Wrap::It()->AddServerContract(qstrContents.toStdString());

        if (1 != nAdded)
        {
            QMessageBox::warning(this, tr("Failed Importing Server Contract"),
                tr("Failed trying to import contract. Is it already in the wallet?"));
            return;
        }
        // -----------------------------------------------
        QString qstrContractName = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetServer_Name(qstrContractID.toStdString()));
        // -----------------------------------------------
        // Commenting this out for now.
        //
//      QMessageBox::information(this, tr("Success!"), QString("%1: '%2' %3: %4").arg(tr("Success Importing Server Contract! Name")).
//                               arg(qstrContractName).arg(tr("ID")).arg(qstrContractID));
        // ----------
        m_pOwner->m_map.insert(qstrContractID, qstrContractName);
        m_pOwner->SetPreSelected(qstrContractID);
        // ------------------------------------------------
        emit newServerAdded(qstrContractID);
        // ------------------------------------------------
    } // if (!qstrContractID.isEmpty())
}

// ------------------------------------------------------
/*
// Source: http://stackoverflow.com/questions/2598117/zipping-a-folder-file-using-qt
static bool extract(const QString & filePath, const QString & extDirPath, const QString & singleFileName = QString(""))
{
    QuaZip zip(filePath);

    if (!zip.open(QuaZip::mdUnzip)) {
        qWarning("testRead(): zip.open(): %d", zip.getZipError());
        return false;
    }

    zip.setFileNameCodec("IBM866");

    qWarning("%d entries\n", zip.getEntriesCount());
    qWarning("Global comment: %s\n", zip.getComment().toLocal8Bit().constData());

    QuaZipFileInfo info;

    QuaZipFile file(&zip);

    QFile out;
    QString name;
    char c;
    for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile())
    {
        if (!zip.getCurrentFileInfo(&info)) {
            qWarning("testRead(): getCurrentFileInfo(): %d\n", zip.getZipError());
            return false;
        }

        if (!singleFileName.isEmpty())
            if (!info.name.contains(singleFileName))
                continue;

        if (!file.open(QIODevice::ReadOnly)) {
            qWarning("testRead(): file.open(): %d", file.getZipError());
            return false;
        }

        name = QString("%1/%2").arg(extDirPath).arg(file.getActualFileName());

        if (file.getZipError() != UNZ_OK) {
            qWarning("testRead(): file.getFileName(): %d", file.getZipError());
            return false;
        }

        //out.setFileName("out/" + name);
        out.setFileName(name);

        // this will fail if "name" contains subdirectories, but we don't mind that
        out.open(QIODevice::WriteOnly);
        // Slow like hell (on GNU/Linux at least), but it is not my fault.
        // Not ZIP/UNZIP package's fault either.
        // The slowest thing here is out.putChar(c).
        while (file.getChar(&c)) out.putChar(c);

        out.close();

        if (file.getZipError() != UNZ_OK) {
            qWarning("testRead(): file.getFileName(): %d", file.getZipError());
            return false;
        }

        if (!file.atEnd()) {
            qWarning("testRead(): read all but not EOF");
            return false;
        }

        file.close();

        if (file.getZipError() != UNZ_OK) {
            qWarning("testRead(): file.close(): %d", file.getZipError());
            return false;
        }
    } // for

    zip.close();

    if (zip.getZipError() != UNZ_OK) {
        qWarning("testRead(): zip.close(): %d", zip.getZipError());
        return false;
    }

    return true;
}
*/


static void recurseAddDir(QDir d, QStringList & list)
{
    QStringList qsl = d.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);

    foreach (QString file, qsl)
    {
        QFileInfo finfo(QString("%1/%2").arg(d.path()).arg(file));

        if (finfo.isSymLink())
            return;

        if (finfo.isDir()) {

            QString dirname = finfo.fileName();
            QDir sd(finfo.filePath());

            recurseAddDir(sd, list);

        } else
            list << QDir::toNativeSeparators(finfo.filePath());
    }
}


bool MTServerDetails::archive(const QString & filePath, const QDir & dir, const QString & comment/*=QString("")*/)
{
    QuaZip zip(filePath);
    zip.setFileNameCodec("IBM866");

    if (!zip.open(QuaZip::mdCreate))
    {
        QMessageBox::information(this, tr("Failure Creating Zipfile"), QString("archive(): zip.open(): %1").arg(zip.getZipError()));
        return false;
    }

    if (!dir.exists())
    {
        QMessageBox::information(this, tr("Directory to be Zipped Doesn't Exist"), QString("dir.exists(%1)=FALSE").arg(dir.absolutePath()));
        return false;
    }

    QFile inFile;

    // Получаем список файлов и папок рекурсивно
    QStringList sl;
    recurseAddDir(dir, sl);

    // Создаем массив состоящий из QFileInfo объектов
    QFileInfoList files;
    foreach (QString fn, sl) files << QFileInfo(fn);

    QuaZipFile outFile(&zip);

    char c;
    foreach(QFileInfo fileInfo, files)
    {
        if (!fileInfo.isFile())
            continue;

        // Если файл в поддиректории, то добавляем имя этой поддиректории к именам файлов
        // например: fileInfo.filePath() = "D:\Work\Sources\SAGO\svn\sago\Release\tmp_DOCSWIN\Folder\123.opn"
        // тогда после удаления части строки fileNameWithSubFolders будет равен "Folder\123.opn" и т.д.
        QString fileNameWithRelativePath = fileInfo.filePath().remove(0, dir.absolutePath().length() + 1);

        inFile.setFileName(fileInfo.filePath());

        if (!inFile.open(QIODevice::ReadOnly)) {
            QMessageBox::information(this, tr("Failure opening an Input File"),
                                     QString("archive(): inFile.open(): %1").arg(inFile.errorString().toLocal8Bit().constData()));
            return false;
        }

        if (!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileNameWithRelativePath, fileInfo.filePath()))) {
            QMessageBox::information(this, tr("Failure opening Output File"),
                                     QString("archive(): outFile.open(): %1").arg(outFile.getZipError()));
            return false;
        }

        while (inFile.getChar(&c) && outFile.putChar(c)) {
            // blank on purpose.
        }

        if (outFile.getZipError() != UNZ_OK) {
            QMessageBox::information(this, tr("Error writing to Output File"),
                                     QString("archive(): outFile.putChar(): %1").arg(outFile.getZipError()));
            return false;
        }

        outFile.close();

        if (outFile.getZipError() != UNZ_OK) {
            QMessageBox::information(this, tr("Error after closing Output File"),
                                     QString("archive(): outFile.close(): %1").arg(outFile.getZipError()));
            return false;
        }

        inFile.close();
    }

    // + комментарий
    if (!comment.isEmpty())
        zip.setComment(comment);

    zip.close();

    if (zip.getZipError() != 0)
    {
        QMessageBox::information(this, tr("Error after closing ZipFile"),
                                 QString("archive(): zip.close(): %1").arg(zip.getZipError()));
        return false;
    }

    return true;
}



//virtual
void MTServerDetails::AddButtonClicked()
{
    MTWizardAddContract theWizard(this);

    theWizard.setWindowTitle(tr("Add Server Contract"));

    QString qstrDefaultValue("https://raw.github.com/FellowTraveler/Open-Transactions/master/sample-data/sample-contracts/cloud.otc");
    QVariant varDefault(qstrDefaultValue);

    theWizard.setField(QString("URL"), varDefault);
    theWizard.setField(QString("contractType"), QString("server")); // So the wizard knows it's creating a server contract.

    QString qstrDefaultContract(
                "<notaryProviderContract version=\"2.0\">\n"
                "\n"
                "<entity shortname=\"localhost\"\n"
                " longname=\"Localhost Test Contract\"\n"
                " email=\"serverfarm@blahcloudcomputing.com\"\n"
                " serverURL=\"https://blahtransactions.com/vers/1/\"/>\n"
                           "\n"
                "<notaryServer hostname=\"localhost\"\n"
                " port=\"7085\"\n"
                " URL=\"https://blahtransactions.com/vers/1/\" />\n"
                      "\n"
                "</notaryProviderContract>\n"
    );

    theWizard.setField(QString("contractXML"), qstrDefaultContract);

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
            QString qstrXMLContents = theWizard.field("contractXML").toString();
            QString qstrNymID       = theWizard.field("NymID").toString();

            std::string strContractID;

            if (strContractID.empty() || "" == strContractID) {
                QMessageBox::warning(this, tr("Failed Creating Contract"),
                                     tr("Unable to create contract. Perhaps the XML contents were bad?"));
                return;
            }
        } // bIsCreating is true.
    } // Wizard "OK" was clicked.
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
        opentxs::OTWallet* pWallet =
            pWallet = opentxs::OTAPI_Wrap::OTAPI()->GetWallet(__FUNCTION__);

        if (nullptr == pWallet) { return; }

        opentxs::ServerContract* contract =
        pWallet->GetServerContract(strID.toStdString());

        if (nullptr == contract) {return; }

        QString qstrNymID("");
        qstrNymID =
            QString::fromStdString(opentxs::String(contract->ID()).Get());

        QString qstrContents = QString::fromStdString(contract->Terms().Get());

        if (m_pPlainTextEdit)
            m_pPlainTextEdit->setPlainText(qstrContents);
        // ----------------------------------

        // ----------------------------------
        ui->lineEditID   ->setText(strID);
        ui->lineEditName ->setText(strName);
        ui->lineEditNymID->setText(qstrNymID);

        FavorLeftSideForIDs();
    }
}

// ------------------------------------------------------

void MTServerDetails::on_lineEditName_editingFinished()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        bool bSuccess = opentxs::OTAPI_Wrap::It()->SetServer_Name(m_pOwner->m_qstrCurrentID.toStdString(),  // Server
                                                   ui->lineEditName->text(). toStdString()); // New Name
        if (bSuccess)
        {
            m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
            m_pOwner->m_map.insert(m_pOwner->m_qstrCurrentID, ui->lineEditName->text());

            m_pOwner->SetPreSelected(m_pOwner->m_qstrCurrentID);
            // ------------------------------------------------
            emit serversChanged();
            // ------------------------------------------------
        }
    }
}

// ------------------------------------------------------

