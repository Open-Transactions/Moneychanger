#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/ui/dlgpassphrasemanager.hpp>
#include <ui_dlgpassphrasemanager.h>

#include <gui/ui/dlgcreatepassphrase.hpp>

#include <core/moneychanger.hpp>
#include <core/handlers/DBHandler.hpp>
#include <core/handlers/contacthandler.hpp>
#include <core/handlers/focuser.h>

#include <opentxs/core/OTStorage.hpp>
#include <opentxs/client/OTAPI_Wrap.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>
#include <opentxs/client/OT_ME.hpp>
#include <opentxs/core/util/Assert.hpp>
#include <opentxs/client/OT_API.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>

#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QMenu>
#include <QTimer>

#include <string>

// -------------------------------------------------------------
//static
ClipboardWrapper* ClipboardWrapper::s_it = NULL;

ClipboardWrapper::ClipboardWrapper(QObject * parent/*=0*/)
    : QObject(parent), m_pTimer(new QTimer(this))
{
    QCoreApplication * pCore = QCoreApplication::instance();
    connect(m_pTimer, SIGNAL(timeout()), SLOT(clearContents()));
    connect(pCore, SIGNAL(aboutToQuit()), SLOT(onDestroy()));
    m_pTimer->setSingleShot(true);
}

//static
ClipboardWrapper * ClipboardWrapper::It()
{
    if (NULL == ClipboardWrapper::s_it)
    {
        QCoreApplication * pCore = QCoreApplication::instance();
        ClipboardWrapper::s_it = new ClipboardWrapper(pCore);
    }

    return ClipboardWrapper::s_it;
}

void ClipboardWrapper::set(const QString & strNewText)
{
    QClipboard * clipboard = QApplication::clipboard();
    OT_ASSERT(NULL != clipboard);
    clipboard->setText(strNewText, QClipboard::Clipboard);
    if (clipboard->supportsSelection()) { clipboard->setText(strNewText, QClipboard::Selection); }
    const int nTimeoutSeconds = 10; // say, 10 seconds until the passphrase times out. (TODO: hardcoding. Make this configurable.)
    m_qstrPreviousSetContents = strNewText;
    m_pTimer->start(nTimeoutSeconds * 1000); // milliseconds.
}

void ClipboardWrapper::onDestroy()
{
    if (m_pTimer->isActive())
    {
        m_pTimer->stop();
        clearContents();
    }
}

void ClipboardWrapper::clearContents()
{
    QClipboard* clipboard = QApplication::clipboard();
    OT_ASSERT(NULL != clipboard);
    if (0 == m_qstrPreviousSetContents.compare(clipboard->text(QClipboard::Clipboard)))
        clipboard->clear(QClipboard::Clipboard);
    if (clipboard->supportsSelection() &&
        (0 == m_qstrPreviousSetContents.compare(clipboard->text(QClipboard::Selection))))
        clipboard->clear(QClipboard::Selection);
    m_qstrPreviousSetContents.clear();
}

// -------------------------------------------------------------

DlgPassphraseManager::DlgPassphraseManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgPassphraseManager),
    already_init(false),
    pActionCopyUsername(NULL),
    pActionCopyPassword(NULL),
    pActionCopyURL(NULL)
{
    ui->setupUi(this);
    
    this->installEventFilter(this);
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
    ui->tableWidget->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignCenter);
    // ----------------------------------
    popupMenu_.reset(new QMenu(this));

    //popupMenu_->addAction("Menu Item 1", this, SLOT(menuItemActivated()));
    pActionCopyUsername = popupMenu_->addAction(tr("Copy username to clipboard"));
    pActionCopyPassword = popupMenu_->addAction(tr("Copy password to clipboard"));
    pActionCopyURL      = popupMenu_->addAction(tr("Copy URL to clipboard"));
}

DlgPassphraseManager::~DlgPassphraseManager()
{
    delete ui;
}

void DlgPassphraseManager::on_pushButtonAdd_clicked()
{
    // Pop up new dialog to collect Title, URL, Username, Passphrase, and Notes.
    // Then create a new Passphrase with that data and add it to the table widget, and select it.

    DlgCreatePassphrase theDlg(this);

    theDlg.setWindowTitle("Add Passphrase");

    if (QDialog::Accepted == theDlg.exec())
    {
        theDlg.m_qstrNotes.truncate(500);

        int nPassphraseID = MTContactHandler::getInstance()->CreateManagedPassphrase(theDlg.m_qstrTitle, theDlg.m_qstrUsername,
                                                                                     *(theDlg.m_pPassphrase),
                                                                                     theDlg.m_qstrURL, theDlg.m_qstrNotes);
        // If I got a passphrase ID back, then add it to the list on the screen.
        if (nPassphraseID > 0)
        {
            QString qstrTitle, qstrUsername, qstrURL, qstrNotes;
            opentxs::OTPassword thePassphrase;

            if (MTContactHandler::getInstance()->GetManagedPassphrase(nPassphraseID, qstrTitle, qstrUsername, thePassphrase, qstrURL, qstrNotes))
            {
                // ----------------------------
                ui->tableWidget->blockSignals(true);
                // ----------------------------
                int column = 0;
                // ----------------------------------
                ui->tableWidget->insertRow(0);
                // ----------------------------------
                ui->tableWidget->setItem(0, column++, new QTableWidgetItem(qstrTitle));
                ui->tableWidget->setItem(0, column++, new QTableWidgetItem(qstrURL));
                // ----------------------------------
                ui->tableWidget->item(0,0)->setData(Qt::UserRole, QVariant(nPassphraseID));
                // ----------------------------------
                ui->tableWidget->setCurrentCell(0,0);
                ui->pushButtonDelete->setEnabled(true);
                ui->pushButtonEdit  ->setEnabled(true);
                // -----------------------------------------------
                ui->tableWidget->blockSignals(false);
            } // GetManagedPassphrase
            else
                QMessageBox::warning(this, tr("Moneychanger"), tr("Strange: I just created a new "
                                                                  "passphrase entry, but then couldn't retrieve it from the local database."));
        } // Passphrase ID >0
        else
            QMessageBox::warning(this, tr("Moneychanger"), tr("An error occurred while trying to create a new passphrase entry."));
    } // Dialog accepted.
}


void DlgPassphraseManager::on_tableWidget_cellDoubleClicked(int row, int column)
{
    ui->tableWidget->setCurrentCell(row, 0);
    on_pushButtonEdit_clicked();
}

void DlgPassphraseManager::on_pushButtonEdit_clicked()
{
    int nRow = ui->tableWidget->currentRow();

    if (nRow >= 0)
    {
        // Get the Passphrase ID.
        int nPassphraseID = ui->tableWidget->item(nRow, 0)->data(Qt::UserRole).toInt();

        if (nPassphraseID > 0)
        {
            // Get the data members from the database for that Passphrase ID.
            QString qstrTitle, qstrUsername, qstrURL, qstrNotes;
            opentxs::OTPassword thePassphrase;

            if (MTContactHandler::getInstance()->GetManagedPassphrase(nPassphraseID, qstrTitle, qstrUsername, thePassphrase, qstrURL, qstrNotes))
            {
                // Now let's pop up an edit dialog and give it those data members.
                //
                DlgCreatePassphrase theDlg(qstrTitle, qstrUsername, qstrURL, qstrNotes, thePassphrase, this);

                theDlg.setWindowTitle(tr("View/Edit Passphrase"));

                if (QDialog::Accepted == theDlg.exec())
                {
                    theDlg.m_qstrNotes.truncate(500);

                    if (MTContactHandler::getInstance()->UpdateManagedPassphrase(nPassphraseID,
                                                                                 theDlg.m_qstrTitle, theDlg.m_qstrUsername,
                                                                                 *(theDlg.m_pPassphrase),
                                                                                 theDlg.m_qstrURL, theDlg.m_qstrNotes))
                    {
                        ui->tableWidget->item(nRow, 0)->setText(theDlg.m_qstrTitle);
                        ui->tableWidget->item(nRow, 1)->setText(theDlg.m_qstrURL);
                    }
                    else
                        QMessageBox::warning(this, tr("Moneychanger"),
                                             tr("An error occurred while trying to update a passphrase entry."));
                } // Dialog accepted.
            } // Found passphrase in db.
        } // passphrase ID > 0
    } // nRow is a valid row.
}

void DlgPassphraseManager::on_pushButtonDelete_clicked()
{
    // Get (index of?) current table widget item.
    int nRow = ui->tableWidget->currentRow();

    if (nRow >= 0)
    {
        // Get the Passphrase ID.
        int nPassphraseID = ui->tableWidget->item(nRow, 0)->data(Qt::UserRole).toInt();

        if (nPassphraseID > 0)
        {
            // Then ask the user "are you sure?"
            // Then delete it from the database.
            //
            if (QMessageBox::Yes == QMessageBox::question(this, tr("Moneychanger"), tr("PERMAMENT: Are you sure you want to delete this entry?"),
                                                          QMessageBox::Yes|QMessageBox::No))
            {
                // Delete it from the database.
                // Then select something else in the table widget.
                //
                if (MTContactHandler::getInstance()->DeleteManagedPassphrase(nPassphraseID))
                {
                    ui->tableWidget->blockSignals(true);
                    // ----------------------------------
                    ui->tableWidget->removeRow(nRow);  // Remove the selected row from the tableWidget.
                    // ----------------------------------
                    if (nRow < ui->tableWidget->rowCount()) // Next, select another row (since the previously-selected row was just deleted.)
                    {
                        ui->tableWidget->setCurrentCell(nRow,0);
                        ui->pushButtonDelete->setEnabled(true);
                        ui->pushButtonEdit  ->setEnabled(true);
                    }
                    else if (ui->tableWidget->rowCount() > 0)
                    {
                        ui->tableWidget->setCurrentCell(nRow-1,0);
                        ui->pushButtonDelete->setEnabled(true);
                        ui->pushButtonEdit  ->setEnabled(true);
                    }
                    else
                    {
                        ui->pushButtonDelete->setEnabled(false);
                        ui->pushButtonEdit  ->setEnabled(false);
                    }
                    // -----------------------------------------------
                    ui->tableWidget->blockSignals(false);
                }
                else
                    QMessageBox::warning(this, tr("Moneychanger"),
                                         QString("%1: %2").arg("DlgPassphraseManager::on_pushButtonDelete_clicked")
                                         .arg(tr("Error occurred while trying to delete the passphrase entry from the local database.")));
            }
        }
    }
}

// TODO: Finish out the right-click menu. (Below.)

void DlgPassphraseManager::on_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    QTableWidgetItem * pItem = ui->tableWidget->itemAt(pos);

    if (NULL != pItem)
    {
        int nRow = pItem->row();

        if (nRow >= 0)
        {
            int nPassphraseID = ui->tableWidget->item(nRow, 0)->data(Qt::UserRole).toInt();
            // ------------------------
            QPoint globalPos = ui->tableWidget->mapToGlobal(pos);
            // ------------------------
            const QAction* selectedAction = popupMenu_->exec(globalPos); // Here we popup the menu, and get the user's click.
            // ------------------------
            // Get the data members from the database for that Passphrase ID.
            QString qstrTitle, qstrUsername, qstrURL, qstrNotes;
            opentxs::OTPassword thePassphrase;

            if (MTContactHandler::getInstance()->GetManagedPassphrase(nPassphraseID, qstrTitle, qstrUsername, thePassphrase, qstrURL, qstrNotes))
            {
                if (selectedAction == pActionCopyUsername)
                {
                    // Copy the username to clipboard.
                    ClipboardWrapper::It()->set(qstrUsername);
//                  QMessageBox::information(this, QString("Passphrase Manager"), QString("Copy the username! For ID: %1").arg(nPassphraseID));
                }
                else if (selectedAction == pActionCopyPassword)
                {
                    // Copy the password to clipboard.
                    ClipboardWrapper::It()->set(QString::fromUtf8(thePassphrase.getPassword()));
//                  QMessageBox::information(this, QString("Passphrase Manager"), QString("Copy the password! For ID: %1").arg(nPassphraseID));
                }
                else if (selectedAction == pActionCopyURL)
                {
                    // Copy the URL to clipboard.
                    QClipboard *clipboard = QApplication::clipboard();

                    if (NULL != clipboard)
                    {
                        clipboard->setText(qstrURL);
                    }
//                  QMessageBox::information(this, QString("Passphrase Manager"), QString("Copy the URL! For ID: %1").arg(nPassphraseID));
                }
            } // Found managed passphrase in local db.
        } // nRow >= 0
    } // pItem not NULL.
}


// This was the only way I could figure out to do it, that would
// work even when DESELECTING an item on the tableWidget.
// Probably could adapt this trick to other parts of Moneychanger,
// where the details fields are left populated even when the list is
// deselected on the left.
void DlgPassphraseManager::on_tableWidget_itemSelectionChanged()
{
    QList<QTableWidgetItem *> theList = ui->tableWidget->selectedItems();

//  qDebug() << "Item selection changed to: " << ui->tableWidget->currentRow();
//  qDebug() << "Number of selected items: " << theList.size();
//  QTableWidgetItem * pCurrentItem = ui->tableWidget->currentItem();

    if (theList.size() > 0)
    {
        ui->pushButtonDelete->setEnabled(true);
        ui->pushButtonEdit  ->setEnabled(true);
    }
    else
    {
        ui->pushButtonDelete->setEnabled(false);
        ui->pushButtonEdit  ->setEnabled(false);
    }
}


void DlgPassphraseManager::on_lineEdit_textChanged(const QString &arg1)
{
    // This means someone clicked the "clear" button on the search box.
    if (arg1.isEmpty())
        doSearch(arg1);
}

void DlgPassphraseManager::on_lineEdit_returnPressed()
{
    QString qstrSearchText = ui->lineEdit->text();

    this->doSearch(qstrSearchText.simplified());
}

void DlgPassphraseManager::on_pushButtonSearch_clicked()
{
    QString qstrSearchText = ui->lineEdit->text();

    this->doSearch(qstrSearchText.simplified());
}

void DlgPassphraseManager::doSearch(QString qstrInput)
{
    // Retrieve all passphrases, using the input string as a
    // filter on the title, URL, username, and notes fields.
    //
    mapIDName mapTitle, mapURL;
    bool bSuccess = MTContactHandler::getInstance()->GetManagedPassphrases(mapTitle, mapURL, qstrInput);
    Q_UNUSED(bSuccess);

    // Use the results to populate the table widget.
    // We do this even if the results are empty, since it still clears the tableWidget in that case.
    PopulateTableWidget(mapTitle, mapURL);
}

void DlgPassphraseManager::PopulateTableWidget(mapIDName & mapTitle, mapIDName & mapURL)
{
    if (ui)
    {
        ui->tableWidget->blockSignals(true);
        // ----------------------------
        // Remove all items
        ui->tableWidget->clearContents();
        ui->tableWidget->setRowCount(0);
        // ----------------------------
        mapIDName::iterator it_title = mapTitle.begin();
        mapIDName::iterator it_url   = mapURL  .begin();

        for (; it_title != mapTitle.end() && it_url != mapURL.end(); ++it_title, ++it_url)
        {
            QString qstrID    = it_title.key();
            QString qstrTitle = it_title.value();
            QString qstrURL   = it_url  .value();
            // ----------------------------------
            int nPassphraseID = qstrID.toInt();
            int column        = 0;
            // ----------------------------------
            ui->tableWidget->insertRow(0);
            // ----------------------------------
            ui->tableWidget->setItem(0, column++, new QTableWidgetItem(qstrTitle));
            ui->tableWidget->setItem(0, column++, new QTableWidgetItem(qstrURL));
            // ----------------------------------
            ui->tableWidget->item(0,0)->setData(Qt::UserRole, QVariant(nPassphraseID));
        }
        // ----------------------------------
        if (ui->tableWidget->rowCount() > 0)
        {
            ui->tableWidget->setCurrentCell(0,0);
            ui->pushButtonDelete->setEnabled(true);
            ui->pushButtonEdit  ->setEnabled(true);
        }
        else
        {
            ui->pushButtonDelete->setEnabled(false);
            ui->pushButtonEdit  ->setEnabled(false);
        }
        // -----------------------------------------------
        ui->tableWidget->blockSignals(false);
    }
}


void DlgPassphraseManager::dialog()
{
    if (!already_init)
    {
        doSearch("");
        // ----------------------------
        already_init = true;
    }
    // ----------------------------
    Focuser f(this);
    f.show();
    f.focus();
    // ----------------------------
    ui->lineEdit->setFocus();
}


bool DlgPassphraseManager::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Escape)
        {
            close(); // This is caught by this same filter.
            return true;
        }
    }
    // standard event processing
    return QDialog::eventFilter(obj, event);
}



