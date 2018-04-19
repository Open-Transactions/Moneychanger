#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/ui/dlgtradearchive.hpp>
#include <ui_dlgtradearchive.h>

#include <gui/ui/dlgexportedtopass.hpp>

#include <core/handlers/modeltradearchive.hpp>

#include <core/moneychanger.hpp>
#include <core/handlers/DBHandler.hpp>
#include <core/handlers/contacthandler.hpp>
#include <core/handlers/focuser.h>

#include <opentxs/opentxs.hpp>

#include <QKeyEvent>
#include <QApplication>
#include <QMessageBox>
#include <QMenu>
#include <QtAlgorithms>
#include <QItemSelection>

#include <string>



DlgTradeArchive::DlgTradeArchive(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgTradeArchive),
    already_init(false),
    pActionDelete(NULL),
    pActionShowAssetReceipt(NULL),
    pActionShowCurrencyReceipt(NULL),
    pActionShowFinalReceipt(NULL)

{
    ui->setupUi(this);

    this->installEventFilter(this);
    // ----------------------------------
    popupMenu_.reset(new QMenu(this));
    pActionShowAssetReceipt = popupMenu_->addAction(tr("Show asset receipt"));
    pActionShowCurrencyReceipt = popupMenu_->addAction(tr("Show currency receipt"));
    pActionShowFinalReceipt = popupMenu_->addAction(tr("Show final receipt"));
    popupMenu_->addSeparator();
    pActionDelete = popupMenu_->addAction(tr("Delete this record"));
    // ----------------------------------
    QPointer<ModelTradeArchive> pModel = DBHandler::getInstance()->getTradeArchiveModel();

    if (pModel)
    {
        ui->tableView->setModel(pModel);
        ui->tableView->setSortingEnabled(true);
        ui->tableView->resizeColumnsToContents();
        ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

        ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->tableView->verticalHeader()->hide();

        ui->tableView->setAlternatingRowColors(true);

        ui->tableView->horizontalHeader()->setStretchLastSection(true);
        ui->tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);

        //ui->tableView->setSelectionMode    (QAbstractItemView::SingleSelection);
        ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    }
}

DlgTradeArchive::~DlgTradeArchive()
{
    delete ui;
}


void DlgTradeArchive::on_tableView_customContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = ui->tableView->indexAt(pos);
    const int nRow    = index.isValid() ? index.row() : -1;

    if (index.isValid() && (nRow >= 0))
    {
        ui->tableView->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        // ------------------------
        QPoint globalPos = ui->tableView->mapToGlobal(pos);
        // ------------------------
        const QAction* selectedAction = popupMenu_->exec(globalPos); // Here we popup the menu, and get the user's click.
        // ------------------------
        bool bShowOutputDialog = false;
        QPointer<ModelTradeArchive> pModel = DBHandler::getInstance()->getTradeArchiveModel();
        QString qstrReceipt, qstrType, qstrSubTitle;
        // ------------------------
        if (selectedAction == pActionDelete)
            on_pushButton_clicked();
        else if (selectedAction == pActionShowAssetReceipt)
        {
            QModelIndex index14 = pModel->index(nRow, 14); // Asset receipt
            qstrReceipt = pModel->rawData(index14).toString();

            if (qstrReceipt.isEmpty())
                QMessageBox::information(ui->tableView, tr(MONEYCHANGER_APP_NAME), tr("Sorry, no asset receipt is available for this record."));
            else
            {
                bShowOutputDialog = true;
                qstrType = tr("Market receipt of trade:");
                qstrSubTitle = tr("(For asset account)");
            }
        }
        else if (selectedAction == pActionShowCurrencyReceipt)
        {
            QModelIndex index15 = pModel->index(nRow, 15); // Currency receipt
            qstrReceipt = pModel->rawData(index15).toString();

            if (qstrReceipt.isEmpty())
                QMessageBox::information(ui->tableView, tr(MONEYCHANGER_APP_NAME), tr("Sorry, no currency receipt is available for this record."));
            else
            {
                bShowOutputDialog = true;
                qstrType = tr("Market receipt of trade:");
                qstrSubTitle = tr("(For currency account)");
            }
        }
        else if (selectedAction == pActionShowFinalReceipt)
        {
            QModelIndex index16 = pModel->index(nRow, 16); // Final receipt
            qstrReceipt = pModel->rawData(index16).toString();

            if (qstrReceipt.isEmpty())
                QMessageBox::information(ui->tableView, tr(MONEYCHANGER_APP_NAME), tr("Sorry, no final (closing) receipt is available for this record. (Maybe the offer is still live on the market?)"));
            else
            {
                bShowOutputDialog = true;
                qstrType = tr("Final (closing) receipt:");
                qstrSubTitle = tr("No more trades can happen against this offer.");
            }
        }
        // -------------------------------
        if (bShowOutputDialog)
        {
            // -----------
            // Pop up the result dialog.
            //
            DlgExportedToPass dlgExported(this, qstrReceipt,
                                          qstrType,
                                          qstrSubTitle, false);
            dlgExported.exec();
        }

    } // nRow >= 0
}



void DlgTradeArchive::dialog()
{
    if (!already_init)
    {
        connect(ui->tableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                    this, SLOT(tableViewSelectionChanged(const QItemSelection &, const QItemSelection &)));
        // ----------------------------
        already_init = true;
    }
    // ----------------------------
    Focuser f(this);
    f.show();
    f.focus();
    // ----------------------------
//    ui->lineEdit->setFocus();

    QPointer<ModelTradeArchive> pModel = DBHandler::getInstance()->getTradeArchiveModel();

    if (pModel)
    {
        pModel->updateDBFromOT();

        QItemSelectionModel *select = ui->tableView->selectionModel();

        if (select->hasSelection())
            ui->pushButton->setEnabled(true);
        else
            ui->pushButton->setEnabled(false);
    }
}


bool DlgTradeArchive::eventFilter(QObject *obj, QEvent *event)
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



void DlgTradeArchive::on_pushButton_clicked()
{
    if (!ui->tableView->selectionModel()->hasSelection())
        return;
    // ----------------------------------------------
    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(this, tr(MONEYCHANGER_APP_NAME), QString("%1<br/><br/>%2").arg(tr("Are you sure you want to delete these records?")).
                                  arg(tr("WARNING: This is not reversible!")),
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply != QMessageBox::Yes)
        return;
    // ----------------------------------------------
    QPointer<ModelTradeArchive> pModel = DBHandler::getInstance()->getTradeArchiveModel();

    if (pModel)
    {
        QItemSelection selection( ui->tableView->selectionModel()->selection() );

        QList<int> rows;
        foreach( const QModelIndex & index, selection.indexes() ) {
           rows.append( index.row() );
        }

        qSort( rows );

        bool bRemoved = false;

        int prev = -1;
        for( int i = rows.count() - 1; i >= 0; i -= 1 ) {
           int current = rows[i];
           if( current != prev ) {
               bRemoved = true;
               pModel->removeRows( current, 1 );
               prev = current;
           }
        }

        if (bRemoved)
        {
            if (pModel->submitAll())
            {
                pModel->database().commit();
            }
            else
            {
                pModel->database().rollback();
                qDebug() << "Database Write Error" <<
                           "The database reported an error: " <<
                           pModel->lastError().text();
            }
        }
    }
}

// This was the only way I could figure out to do it, that would
// work even when DESELECTING an item on the tableWidget.
// Probably could adapt this trick to other parts of Moneychanger,
// where the details fields are left populated even when the list is
// deselected on the left.
void DlgTradeArchive::tableViewSelectionChanged(const QItemSelection &, const QItemSelection &)
{
    QItemSelectionModel *select = ui->tableView->selectionModel();

    if (select->hasSelection())
    {
        ui->pushButton->setEnabled(true);
    }
    else
    {
        ui->pushButton->setEnabled(false);
    }
}

