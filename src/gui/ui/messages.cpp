#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/ui/messages.hpp>
#include <ui_messages.h>

#include <gui/widgets/compose.hpp>

#include <core/moneychanger.hpp>
#include <core/handlers/DBHandler.hpp>
#include <core/handlers/contacthandler.hpp>
#include <core/handlers/modelmessages.hpp>
#include <core/handlers/focuser.h>

#include <opentxs/client/OTAPI.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>
#include <opentxs/core/Log.hpp>
#include <opentxs/core/OTTransaction.hpp>
#include <opentxs/core/OTTransactionType.hpp>
#include <opentxs/client/OTRecordList.hpp>

#include <QLabel>
#include <QDebug>
#include <QToolButton>
#include <QKeyEvent>
#include <QApplication>
#include <QMessageBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMenu>
#include <QList>
#include <QSqlRecord>

#include <string>
#include <map>


MSG_TREE_ITEM Messages::make_tree_item(int nCurrentContact, QString qstrMethodType, QString qstrViaTransport)
{
    return std::make_tuple(nCurrentContact, qstrMethodType.toStdString(), qstrViaTransport.toStdString());
}


void Messages::set_inbox_msgid_for_tree_item(MSG_TREE_ITEM & theItem, int nMsgID)
{
    mapOfMsgTreeItems::iterator it = mapCurrentRows_inbox.find(theItem);
    if (mapCurrentRows_inbox.end() != it) // found it.
    {
        mapCurrentRows_inbox.erase(it);
    }
    mapCurrentRows_inbox.insert( std::pair<MSG_TREE_ITEM, int> (theItem, nMsgID) );
}

void Messages::set_outbox_msgid_for_tree_item(MSG_TREE_ITEM & theItem, int nMsgID)
{
    mapOfMsgTreeItems::iterator it = mapCurrentRows_outbox.find(theItem);
    if (mapCurrentRows_outbox.end() != it) // found it.
    {
        mapCurrentRows_outbox.erase(it);
    }
    mapCurrentRows_outbox.insert( std::pair<MSG_TREE_ITEM, int> (theItem, nMsgID) );
}

int Messages::get_inbox_msgid_for_tree_item(MSG_TREE_ITEM & theItem)
{
    mapOfMsgTreeItems::iterator it = mapCurrentRows_inbox.find(theItem);
    if (mapCurrentRows_inbox.end() != it) // found it.
    {
        return it->second;
    }
    return 0;
}

int Messages::get_outbox_msgid_for_tree_item(MSG_TREE_ITEM & theItem)
{
    mapOfMsgTreeItems::iterator it = mapCurrentRows_outbox.find(theItem);
    if (mapCurrentRows_outbox.end() != it) // found it.
    {
        return it->second;
    }
    return 0;
}

Messages::Messages(QWidget *parent) :
    QWidget(parent, Qt::Window),
    ui(new Ui::Messages)
{
    ui->setupUi(this);

    this->installEventFilter(this);

    connect(ui->toolButtonCompose, SIGNAL(clicked()), Moneychanger::It(), SLOT(mc_composemessage_slot()));
}

void setup_tableview(QTableView * pView, QAbstractItemModel * pModel)
{
    pView->setModel(pModel);
    pView->horizontalHeader()->setStretchLastSection(true);
    pView->setSortingEnabled(true);
    pView->resizeColumnsToContents();
    pView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    pView->setContextMenuPolicy(Qt::CustomContextMenu);
    pView->verticalHeader()->hide();

    pView->setAlternatingRowColors(true);

    pView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    pView->setSelectionBehavior(QAbstractItemView::SelectRows);
}


void Messages::on_tabWidget_currentChanged(int index)
{
    if (ui->tableViewSent     && ui->tableViewReceived &&
        pMsgProxyModelOutbox_ && pMsgProxyModelInbox_)
    {
        pCurrentTabTableView_  = (0 == ui->tabWidget->currentIndex()) ? ui->tableViewReceived    : ui->tableViewSent;
        pCurrentTabProxyModel_ = (0 == ui->tabWidget->currentIndex()) ? &(*pMsgProxyModelInbox_) : &(*pMsgProxyModelOutbox_);
        // -------------------------------------------------
        QModelIndex the_index  = pCurrentTabTableView_->currentIndex();

        if (the_index.isValid())
            enableButtons();
        else
            disableButtons();
        // --------------------------------------
        RefreshMessages();
    }
    else
    {
        pCurrentTabTableView_  = nullptr;
        pCurrentTabProxyModel_ = nullptr;

        disableButtons();
    }
}


void Messages::enableButtons()
{
    ui->toolButtonDelete ->setEnabled(true);
    ui->toolButtonReply  ->setEnabled(true);
    ui->toolButtonForward->setEnabled(true);

}

void Messages::disableButtons()
{
    ui->toolButtonDelete ->setEnabled(false);
    ui->toolButtonReply  ->setEnabled(false);
    ui->toolButtonForward->setEnabled(false);

}

void Messages::on_tableViewSentSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous)
{
    qDebug() << "on_tableViewSentSelectionModel_currentRowChanged WAS CALLED!";

    if (!current.isValid())
    {
        ui->headerSent->setSubject("");
        ui->headerSent->setSender("");
        ui->headerSent->setRecipient("");
        ui->headerSent->setTimestamp("");
        ui->headerSent->setFolder(tr("Sent"));
        // ----------------------------------------
        ui->plainTextEditSent->setPlainText("");
        // ----------------------------------------
        disableButtons();
        // ----------------------------------------
        MSG_TREE_ITEM theItem = make_tree_item(nCurrentContact_, qstrMethodType_, qstrViaTransport_);
        set_outbox_msgid_for_tree_item(theItem, 0);
    }
    else
    {
        enableButtons();
        // ----------------------------------------
        QPointer<ModelMessages> pModel = DBHandler::getInstance()->getMessageModel();

        if (pModel)
        {
            QModelIndex sourceIndex = pMsgProxyModelOutbox_->mapToSource(current);

            QModelIndex haveReadSourceIndex  = pMsgProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), MSG_SOURCE_COL_HAVE_READ, sourceIndex);
            QModelIndex msgIDSourceIndex     = pMsgProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), MSG_SOURCE_COL_MSG_ID,    sourceIndex);
            QModelIndex subjectSourceIndex   = pMsgProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), MSG_SOURCE_COL_SUBJECT,   sourceIndex);
            QModelIndex senderSourceIndex    = pMsgProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), MSG_SOURCE_COL_MY_NYM,    sourceIndex);
            QModelIndex recipientSourceIndex = pMsgProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), MSG_SOURCE_COL_RECIP_NYM, sourceIndex);
            QModelIndex timestampSourceIndex = pMsgProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), MSG_SOURCE_COL_TIMESTAMP, sourceIndex);

            QModelIndex subjectIndex      = pMsgProxyModelOutbox_->mapFromSource(subjectSourceIndex);
            QModelIndex senderIndex       = pMsgProxyModelOutbox_->mapFromSource(senderSourceIndex);
            QModelIndex recipientIndex    = pMsgProxyModelOutbox_->mapFromSource(recipientSourceIndex);
            QModelIndex timestampIndex    = pMsgProxyModelOutbox_->mapFromSource(timestampSourceIndex);

            QVariant varMsgID     = pModel->data(msgIDSourceIndex);
            QVariant varHaveRead  = pModel->data(haveReadSourceIndex);
            QVariant varSubject   = pMsgProxyModelOutbox_->data(subjectIndex);
            QVariant varSender    = pMsgProxyModelOutbox_->data(senderIndex);
            QVariant varRecipient = pMsgProxyModelOutbox_->data(recipientIndex);
            QVariant varTimestamp = pMsgProxyModelOutbox_->data(timestampIndex);

            QString qstrSubject   = varSubject.isValid()   ? varSubject.toString()   : "";
            QString qstrSender    = varSender.isValid()    ? varSender.toString()    : "";
            QString qstrRecipient = varRecipient.isValid() ? varRecipient.toString() : "";
            QString qstrTimestamp = varTimestamp.isValid() ? varTimestamp.toString() : "";

            ui->headerSent->setSubject(qstrSubject);
            ui->headerSent->setSender(qstrSender);
            ui->headerSent->setRecipient(qstrRecipient);
            ui->headerSent->setTimestamp(qstrTimestamp);
            ui->headerSent->setFolder(tr("Sent"));
            // ----------------------------------------------------------
            int message_id = varMsgID.isValid() ? varMsgID.toInt() : 0;
            if (message_id > 0)
            {
                QString qstrBody = MTContactHandler::getInstance()->GetMessageBody(message_id);
                if (!qstrBody.isEmpty())
                    ui->plainTextEditSent->setPlainText(qstrBody);
                // -------------------------------------------------
                MSG_TREE_ITEM theItem = make_tree_item(nCurrentContact_, qstrMethodType_, qstrViaTransport_);
                set_outbox_msgid_for_tree_item(theItem, message_id);
            }
            // ----------------------------------------------------------
            const bool bHaveRead = varHaveRead.isValid() ? varHaveRead.toBool() : false;

            if (!bHaveRead) // It's unread, so we need to set it as read.
            {
                pModel->database().transaction();

                if (pModel->setData(haveReadSourceIndex, QVariant(1))) // 1 for "true" in sqlite. "Yes, we've now read this message. Mark it as read."
                {
                    if (pModel->submitAll())
                    {
                        pModel->database().commit();
                        // ------------------------------------

                        RefreshMessages();

//                        qDebug() << "CALLING INVALIDATE\n";

//                        pMsgProxyModelOutbox_->invalidate();
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
    }
}

void Messages::on_tableViewReceivedSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous)
{
    qDebug() << "on_tableViewReceivedSelectionModel_currentRowChanged WAS CALLED!";

    if (!current.isValid())
    {
        ui->headerReceived->setSubject("");
        ui->headerReceived->setSender("");
        ui->headerReceived->setRecipient("");
        ui->headerReceived->setTimestamp("");
        ui->headerReceived->setFolder(tr("Received"));
        // ----------------------------------------
        ui->plainTextEditReceived->setPlainText("");
        // ----------------------------------------
        disableButtons();
        // ----------------------------------------
        MSG_TREE_ITEM theItem = make_tree_item(nCurrentContact_, qstrMethodType_, qstrViaTransport_);
        set_inbox_msgid_for_tree_item(theItem, 0);
    }
    else
    {
        enableButtons();
        // ----------------------------------------
        QPointer<ModelMessages> pModel = DBHandler::getInstance()->getMessageModel();

        if (pModel)
        {
            QModelIndex sourceIndex = pMsgProxyModelInbox_->mapToSource(current);

            QModelIndex haveReadSourceIndex  = pMsgProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), MSG_SOURCE_COL_HAVE_READ,  sourceIndex);
            QModelIndex msgIDSourceIndex     = pMsgProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), MSG_SOURCE_COL_MSG_ID,     sourceIndex);
            QModelIndex subjectSourceIndex   = pMsgProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), MSG_SOURCE_COL_SUBJECT,    sourceIndex);
            QModelIndex senderSourceIndex    = pMsgProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), MSG_SOURCE_COL_SENDER_NYM, sourceIndex);
            QModelIndex recipientSourceIndex = pMsgProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), MSG_SOURCE_COL_MY_NYM,     sourceIndex);
            QModelIndex timestampSourceIndex = pMsgProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), MSG_SOURCE_COL_TIMESTAMP,  sourceIndex);

            QModelIndex subjectIndex      = pMsgProxyModelInbox_->mapFromSource(subjectSourceIndex);
            QModelIndex senderIndex       = pMsgProxyModelInbox_->mapFromSource(senderSourceIndex);
            QModelIndex recipientIndex    = pMsgProxyModelInbox_->mapFromSource(recipientSourceIndex);
            QModelIndex timestampIndex    = pMsgProxyModelInbox_->mapFromSource(timestampSourceIndex);

            QVariant varMsgID     = pModel->data(msgIDSourceIndex);
            QVariant varHaveRead  = pModel->data(haveReadSourceIndex);
            QVariant varSubject   = pMsgProxyModelInbox_->data(subjectIndex);
            QVariant varSender    = pMsgProxyModelInbox_->data(senderIndex);
            QVariant varRecipient = pMsgProxyModelInbox_->data(recipientIndex);
            QVariant varTimestamp = pMsgProxyModelInbox_->data(timestampIndex);

            QString qstrSubject   = varSubject.isValid()   ? varSubject  .toString() : "";
            QString qstrSender    = varSender   .isValid() ? varSender   .toString() : "";
            QString qstrRecipient = varRecipient.isValid() ? varRecipient.toString() : "";
            QString qstrTimestamp = varTimestamp.isValid() ? varTimestamp.toString() : "";

            ui->headerReceived->setSubject  (qstrSubject);
            ui->headerReceived->setSender   (qstrSender);
            ui->headerReceived->setRecipient(qstrRecipient);
            ui->headerReceived->setTimestamp(qstrTimestamp);
            ui->headerReceived->setFolder(tr("Received"));

            int message_id = varMsgID.isValid() ? varMsgID.toInt() : 0;
            if (message_id > 0)
            {
                QString qstrBody = MTContactHandler::getInstance()->GetMessageBody(message_id);
                if (!qstrBody.isEmpty())
                    ui->plainTextEditReceived->setPlainText(qstrBody);
                // -------------------------------------------------
                MSG_TREE_ITEM theItem = make_tree_item(nCurrentContact_, qstrMethodType_, qstrViaTransport_);
                set_inbox_msgid_for_tree_item(theItem, message_id);
            }
            // ----------------------------------------------------------
            const bool bHaveRead = varHaveRead.isValid() ? varHaveRead.toBool() : false;

            if (!bHaveRead) // It's unread, so we need to set it as read.
            {
                pModel->database().transaction();

                if (pModel->setData(haveReadSourceIndex, QVariant(1))) // 1 for "true" in sqlite. "Yes, we've now read this message. Mark it as read."
                {
                    if (pModel->submitAll())
                    {
                        pModel->database().commit();
                        // ------------------------------------

                        //qDebug() << "CALLING INVALIDATE\n";


                        //pMsgProxyModelInbox_->invalidate();

                        RefreshMessages();
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
    }
}

void Messages::dialog()
{
    if (!already_init)
    {
        ui->userBar->setStyleSheet("QWidget{background-color:#c0cad4;selection-background-color:#a0aac4;}");
        // ----------------------------------
        popupMenu_.reset(new QMenu(this));
        pActionOpenNewWindow = popupMenu_->addAction(tr("Open in New Window"));
        pActionReply = popupMenu_->addAction(tr("Reply"));
        pActionForward = popupMenu_->addAction(tr("Forward"));
        popupMenu_->addSeparator();
        pActionDelete = popupMenu_->addAction(tr("Delete"));
        // ----------------------------------
        QPixmap pixmapContacts(":/icons/icons/user.png"); // This is here only for size purposes.
        QIcon   contactsButtonIcon(pixmapContacts);       // Other buttons use this to set their own size.
        // ----------------------------------------------------------------
        QPixmap pixmapCompose (":/icons/icons/pencil.png");
        QPixmap pixmapRefresh (":/icons/icons/refresh.png");
        QPixmap pixmapReply   (":/icons/icons/reply.png");
        QPixmap pixmapForward (":/icons/sendfunds");
        QPixmap pixmapDelete  (":/icons/icons/DeleteRed.png");
        // ----------------------------------------------------------------
        QIcon refreshButtonIcon (pixmapRefresh);
        QIcon composeButtonIcon (pixmapCompose);
        QIcon replyButtonIcon   (pixmapReply);
        QIcon forwardButtonIcon (pixmapForward);
        QIcon deleteButtonIcon  (pixmapDelete);
        // ----------------------------------------------------------------
        ui->toolButtonRefresh->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButtonRefresh->setAutoRaise(true);
        ui->toolButtonRefresh->setIcon(refreshButtonIcon);
//      ui->toolButtonRefresh->setIconSize(pixmapRefresh.rect().size());
        ui->toolButtonRefresh->setIconSize(pixmapContacts.rect().size());
        ui->toolButtonRefresh->setText(tr("Refresh"));
        // ----------------------------------------------------------------
        ui->toolButtonCompose->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButtonCompose->setAutoRaise(true);
        ui->toolButtonCompose->setIcon(composeButtonIcon);
//      ui->toolButtonCompose->setIconSize(pixmapCompose.rect().size());
        ui->toolButtonCompose->setIconSize(pixmapContacts.rect().size());
        ui->toolButtonCompose->setText(tr("Compose"));
        // ----------------------------------------------------------------
        ui->toolButtonDelete->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButtonDelete->setAutoRaise(true);
        ui->toolButtonDelete->setIcon(deleteButtonIcon);
//      ui->toolButtonDelete->setIconSize(pixmapDelete.rect().size());
        ui->toolButtonDelete->setIconSize(pixmapContacts.rect().size());
        ui->toolButtonDelete->setText(tr("Delete"));
        // ----------------------------------------------------------------
        ui->toolButtonReply->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButtonReply->setAutoRaise(true);
        ui->toolButtonReply->setIcon(replyButtonIcon);
//      ui->toolButtonReply->setIconSize(pixmapReply.rect().size());
        ui->toolButtonReply->setIconSize(pixmapContacts.rect().size());
        ui->toolButtonReply->setText(tr("Reply"));
        // ----------------------------------------------------------------
        ui->toolButtonForward->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButtonForward->setAutoRaise(true);
        ui->toolButtonForward->setIcon(forwardButtonIcon);
//      ui->toolButtonForward->setIconSize(pixmapForward.rect().size());
        ui->toolButtonForward->setIconSize(pixmapContacts.rect().size());
        ui->toolButtonForward->setText(tr("Forward"));
        // ******************************************************
        QPointer<ModelMessages> pModel = DBHandler::getInstance()->getMessageModel();

        if (pModel)
        {
            pMsgProxyModelInbox_  = new MessagesProxyModel;
            pMsgProxyModelOutbox_ = new MessagesProxyModel;
            pMsgProxyModelInbox_ ->setSourceModel(pModel);
            pMsgProxyModelOutbox_->setSourceModel(pModel);
            pMsgProxyModelOutbox_->setFilterFolder(0);
            pMsgProxyModelInbox_ ->setFilterFolder(1);
            // ---------------------------------
            pMsgProxyModelInbox_ ->setFilterKeyColumn(-1);
            pMsgProxyModelOutbox_->setFilterKeyColumn(-1);
            // ---------------------------------
            setup_tableview(ui->tableViewSent, pMsgProxyModelOutbox_);
            setup_tableview(ui->tableViewReceived, pMsgProxyModelInbox_);
            // ---------------------------------
            QItemSelectionModel *sm1 = ui->tableViewSent->selectionModel();
            connect(sm1, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                    this, SLOT(on_tableViewSentSelectionModel_currentRowChanged(QModelIndex,QModelIndex)));
            QItemSelectionModel *sm2 = ui->tableViewReceived->selectionModel();
            connect(sm2, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                    this, SLOT(on_tableViewReceivedSelectionModel_currentRowChanged(QModelIndex,QModelIndex)));
            // ---------------------------------
//            connect(pMsgProxyModelInbox_,  SIGNAL(modelReset()),
//                    this,                  SLOT(RefreshMessages()));
//            connect(pMsgProxyModelOutbox_, SIGNAL(modelReset()),
//                    this,                  SLOT(RefreshMessages()));
            // ---------------------------------
        }

        QWidget* pTab0 = ui->tabWidget->widget(0);
        QWidget* pTab1 = ui->tabWidget->widget(1);

        pTab0->setStyleSheet("QWidget { margin: 0 }");
        pTab1->setStyleSheet("QWidget { margin: 0 }");

        ui->splitter->setStretchFactor(0, 2);
        ui->splitter->setStretchFactor(1, 3);

        ui->splitter_3->setStretchFactor(0, 1);
        ui->splitter_3->setStretchFactor(1, 5);
        // ------------------------
        on_tabWidget_currentChanged(0);

        /** Flag Already Init **/
        already_init = true;
    }
    // -------------------------------------------
    RefreshAll();

    Focuser f(this);
    f.show();
    f.focus();
}

void Messages::ClearTree()
{
    ui->treeWidget->blockSignals(true);
    ui->treeWidget->clear();
    ui->treeWidget->blockSignals(false);
}


void Messages::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    if (nullptr != current)
    {
        nCurrentContact_  = current->data(0, Qt::UserRole).toInt();
        qstrMethodType_   = current->data(1, Qt::UserRole).isValid() ? current->data(1, Qt::UserRole).toString() : QString("");
        qstrViaTransport_ = current->data(2, Qt::UserRole).isValid() ? current->data(2, Qt::UserRole).toString() : QString("");

        if ((0 == nCurrentContact_) && qstrMethodType_.isEmpty() && qstrViaTransport_.isEmpty())
        {
            nCurrentContact_  = 0;
            qstrMethodType_   = QString("");
            qstrViaTransport_ = QString("");

            pMsgProxyModelInbox_ ->setFilterNone();
            pMsgProxyModelOutbox_->setFilterNone();
        }
        else if (qstrMethodType_.isEmpty() && qstrViaTransport_.isEmpty())
        {
            pMsgProxyModelInbox_ ->setFilterTopLevel(nCurrentContact_);
            pMsgProxyModelOutbox_->setFilterTopLevel(nCurrentContact_);
        }
        else if (0 == qstrMethodType_.compare("otserver"))
        {
            pMsgProxyModelInbox_ ->setFilterNotary(qstrViaTransport_, nCurrentContact_);
            pMsgProxyModelOutbox_->setFilterNotary(qstrViaTransport_, nCurrentContact_);
        }
        else
        {
            pMsgProxyModelInbox_ ->setFilterMethodAddress(qstrMethodType_, qstrViaTransport_);
            pMsgProxyModelOutbox_->setFilterMethodAddress(qstrMethodType_, qstrViaTransport_);
        }
    }
    else
    {
        nCurrentContact_  = 0;
        qstrMethodType_   = QString("");
        qstrViaTransport_ = QString("");

        pMsgProxyModelInbox_ ->setFilterNone();
        pMsgProxyModelOutbox_->setFilterNone();
    }
    // --------------------------------------
    RefreshMessages();
}


void Messages::RefreshMessages()
{
    qDebug() << "==== TOP of RefreshMessages ====\n";

    ui->tableViewSent->resizeColumnsToContents();
    ui->tableViewReceived->resizeColumnsToContents();

    ui->tableViewSent->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewReceived->horizontalHeader()->setStretchLastSection(true);
    // -------------------------------------------
    MSG_TREE_ITEM theItem = make_tree_item(nCurrentContact_, qstrMethodType_, qstrViaTransport_);

    bool bIsInbox = (0 == ui->tabWidget->currentIndex());
    int  nMsgID   = bIsInbox ? get_inbox_msgid_for_tree_item(theItem) : get_outbox_msgid_for_tree_item(theItem);

    if (0 == nMsgID) // There's no "current selected message ID" set for this tree item.
    {
        int nRowToSelect = -1;

        if (pCurrentTabProxyModel_->rowCount() > 0) // But there ARE rows for this tree item...
            nRowToSelect = 0;

        // So let's select the first one in the list!

        qDebug() << "Messages::RefreshMessages: ABOUT TO SELECT ROW " << nRowToSelect << " IN CURRENT TABLE VIEW...";

        QModelIndex previous = pCurrentTabTableView_->currentIndex();
        pCurrentTabTableView_->blockSignals(true);
        pCurrentTabTableView_->selectRow(nRowToSelect);
        pCurrentTabTableView_->blockSignals(false);

        if (bIsInbox)
            on_tableViewReceivedSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
        else
            on_tableViewSentSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
    }
    else // There IS a "current selected message ID" for the current tree item.
    {
        // So let's try to select that in the tree again! (If it's still there. Otherwise set it to row 0.)

        QPointer<ModelMessages> pModel = DBHandler::getInstance()->getMessageModel();

        if (pModel)
        {
            bool bFoundIt = false;

            const int nRowCount = pCurrentTabProxyModel_->rowCount();

            for (int ii = 0; ii < nRowCount; ++ii)
            {
                QModelIndex indexProxy  = pCurrentTabProxyModel_->index(ii, 0);
                QModelIndex indexSource = pCurrentTabProxyModel_->mapToSource(indexProxy);

                QSqlRecord record = pModel->record(indexSource.row());

                if (!record.isEmpty())
                {
                    QVariant the_value = record.value(MSG_SOURCE_COL_MSG_ID);
                    const int nRecordMsgID = the_value.isValid() ? the_value.toInt() : 0;

                    if (nRecordMsgID == nMsgID)
                    {
                        bFoundIt = true;

                        qDebug() << "Messages::RefreshMessages: ABOUT TO SELECT ROW " << ii << " IN CURRENT TABLE VIEW...";

                        QModelIndex previous = pCurrentTabTableView_->currentIndex();
                        pCurrentTabTableView_->blockSignals(true);
                        pCurrentTabTableView_->selectRow(ii);
                        pCurrentTabTableView_->blockSignals(false);

                        if (bIsInbox)
                            on_tableViewReceivedSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
                        else
                            on_tableViewSentSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
                        break;
                    }
                }
            }
            // ------------------------------------
            if (!bFoundIt)
            {
                int nRowToSelect = -1;

                if (nRowCount > 0)
                    nRowToSelect = 0;

                qDebug() << "Messages::RefreshMessages: ABOUT TO SELECT ROW " << nRowToSelect << " IN CURRENT TABLE VIEW...";

                QModelIndex previous = pCurrentTabTableView_->currentIndex();
                pCurrentTabTableView_->blockSignals(true);
                pCurrentTabTableView_->selectRow(nRowToSelect);
                pCurrentTabTableView_->blockSignals(false);

                if (bIsInbox)
                    on_tableViewReceivedSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
                else
                    on_tableViewSentSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
            }
        }
    }
}

void Messages::RefreshTree()
{
    ClearTree();
    // ----------------------------------------
    ui->treeWidget->blockSignals(true);
    // ----------------------------------------
    QList<QTreeWidgetItem *> items;
    // ------------------------------------
    QTreeWidgetItem * pUnfilteredItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(tr("All Messages (Unfiltered)")));
    pUnfilteredItem->setData(0, Qt::UserRole, QVariant(0));
    items.append(pUnfilteredItem);
    // ----------------------------------------
    mapIDName mapContacts;

    if (MTContactHandler::getInstance()->GetContacts(mapContacts))
    {
        for (mapIDName::iterator ii = mapContacts.begin(); ii != mapContacts.end(); ii++)
        {
            QString qstrContactID   = ii.key();
            QString qstrContactName = ii.value();
            int     nContactID      = qstrContactID.toInt();
            // ------------------------------------
            QTreeWidgetItem * pTopItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(qstrContactName));
            pTopItem->setData(0, Qt::UserRole, QVariant(nContactID));
            items.append(pTopItem);
            // ------------------------------------
            mapIDName mapMethodTypes;
            // So we can look up these names quickly without having to repeatedly hit the database
            // for the same names over and over again.
            MTContactHandler::getInstance()->GetMsgMethodTypes(mapMethodTypes);
            // ------------------------------------
            mapIDName mapTransport;

            if (MTContactHandler::getInstance()->GetMsgMethodTypesByContact(mapTransport, nContactID, true)) // True means to add the OT servers as well.
            {
                for (mapIDName::iterator it_transport = mapTransport.begin(); it_transport != mapTransport.end(); it_transport++)
                {
                    QString qstrID   = it_transport.key();
//                  QString qstrName = it_transport.value();

                    QStringList stringlist = qstrID.split("|");

                    if (stringlist.size() >= 2) // Should always be 2...
                    {
                        QString qstrViaTransport = stringlist.at(1);
                        QString qstrTransportName = qstrViaTransport;
                        QString qstrMethodType = stringlist.at(0);
                        QString qstrMethodName = qstrMethodType;

                        mapIDName::iterator it_mapMethodTypes = mapMethodTypes.find(qstrMethodType);

                        if (mapMethodTypes.end() != it_mapMethodTypes)
                        {
                            QString qstrTemp = it_mapMethodTypes.value();
                            if (!qstrTemp.isEmpty())
                                qstrMethodName = qstrTemp;
                        }
                        // ------------------------------------------------------
                        else if (0 == QString("otserver").compare(qstrMethodType))
                        {
                            qstrMethodName = tr("Notary");
                            // ------------------------------
                            QString qstrTemp = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetServer_Name(qstrViaTransport.toStdString()));
                            if (!qstrTemp.isEmpty())
                                qstrTransportName = qstrTemp;
                        }
                        // ------------------------------------------------------
                        QTreeWidgetItem * pAddressItem = new QTreeWidgetItem(pTopItem, QStringList(qstrContactName) << qstrMethodName << qstrTransportName);
                        pAddressItem->setData(0, Qt::UserRole, QVariant(nContactID));
                        pAddressItem->setData(1, Qt::UserRole, QVariant(qstrMethodType));
                        pAddressItem->setData(2, Qt::UserRole, QVariant(qstrViaTransport));
                        items.append(pAddressItem);
                    }
                }
            }
        }
        if (items.count() > 0)
        {
            ui->treeWidget->insertTopLevelItems(0, items);
            ui->treeWidget->resizeColumnToContents(0);
        }
    }
    // ----------------------------------------
    // Make sure the same item that was selected before, is selected again.
    // (If it still exists, which it probably does.)

    QTreeWidgetItem * previous = ui->treeWidget->currentItem();

    // In this case, just select the first thing on the list.
    if ( (0 == nCurrentContact_) && qstrMethodType_.isEmpty() && qstrViaTransport_.isEmpty() )
    {
        if (ui->treeWidget->topLevelItemCount() > 0)
            ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(0));
    }
    else // Find the one that was selected before the refresh.
    {
        bool bFoundIt = false;

        QTreeWidgetItemIterator it(ui->treeWidget);

        while (*it)
        {
            QVariant qvarContactID    = (*it)->data(0, Qt::UserRole);
            QVariant qvarMethodType   = (*it)->data(1, Qt::UserRole);
            QVariant qvarViaTransport = (*it)->data(2, Qt::UserRole);

            const int     nContactID       = qvarContactID   .isValid() ? qvarContactID   .toInt()    :  0;
            const QString qstrMethodType   = qvarMethodType  .isValid() ? qvarMethodType  .toString() : "";
            const QString qstrViaTransport = qvarViaTransport.isValid() ? qvarViaTransport.toString() : "";

            if ( (nContactID       == nCurrentContact_ ) &&
                 (0 == qstrMethodType.compare(qstrMethodType_)) &&
                 (0 == qstrViaTransport.compare(qstrViaTransport_)) )
            {
                bFoundIt = true;
                ui->treeWidget->setCurrentItem(*it);
                break;
            }
            // ------------------------
            ++it;
        } //while

        if (!bFoundIt)
        {
            if (ui->treeWidget->topLevelItemCount() > 0)
                ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(0));
        }
    }
    // ----------------------------------------
    ui->treeWidget->blockSignals(false);
    // ----------------------------------------
    on_treeWidget_currentItemChanged(ui->treeWidget->currentItem(), previous);
}

void Messages::on_toolButtonReply_clicked()
{
    QPointer<ModelMessages> pModel = DBHandler::getInstance()->getMessageModel();

    if (!pModel)
        return;
    // ------------------------------------
    QModelIndex proxyIndex = pCurrentTabTableView_->currentIndex();

    if (!proxyIndex.isValid())
        return;
    // ------------------------------------
    QModelIndex sourceIndex = pCurrentTabProxyModel_->mapToSource(proxyIndex);

    if (!sourceIndex.isValid())
        return;
    // ------------------------------------
    QSqlRecord record = pModel->record(sourceIndex.row());

    if (record.isEmpty())
        return;
    // ------------------------------------
    const int nMessageID = record.value(MSG_SOURCE_COL_MSG_ID).isValid() ? record.value(MSG_SOURCE_COL_MSG_ID).toInt() : 0;

    const bool bOutgoing = (0 == record.value(MSG_SOURCE_COL_FOLDER).toInt());

    const QVariant qvar_method_type = record.value(MSG_SOURCE_COL_METHOD_TYPE);
    const QString  methodType = qvar_method_type.isValid() ? qvar_method_type.toString() : "";

    const QVariant qvar_my_nym_id = record.value(MSG_SOURCE_COL_MY_NYM);
    const QString  myNymID = qvar_my_nym_id.isValid() ? qvar_my_nym_id.toString() : "";

    const QVariant qvar_my_addr = record.value(MSG_SOURCE_COL_MY_ADDR);
    const QString  myAddress = qvar_my_addr.isValid() ? qvar_my_addr.toString() : "";

    const QVariant qvar_sender_nym_id = record.value(MSG_SOURCE_COL_SENDER_NYM);
    const QString  senderNymID = qvar_sender_nym_id.isValid() ? qvar_sender_nym_id.toString() : "";

    const QVariant qvar_recipient_nym_id = record.value(MSG_SOURCE_COL_RECIP_NYM);
    const QString  recipientNymID = qvar_recipient_nym_id.isValid() ? qvar_recipient_nym_id.toString() : "";

    const QVariant qvar_notary_id = record.value(MSG_SOURCE_COL_NOTARY_ID);
    const QString  NotaryID = qvar_notary_id.isValid() ? qvar_notary_id.toString() : "";

    const QVariant qvar_sender_addr = record.value(MSG_SOURCE_COL_SENDER_ADDR);
    const QString  senderAddr = qvar_sender_addr.isValid() ? qvar_sender_addr.toString() : "";

    const QVariant qvar_recipient_addr = record.value(MSG_SOURCE_COL_RECIP_ADDR);
    const QString  recipientAddr = qvar_recipient_addr.isValid() ? qvar_recipient_addr.toString() : "";

    const QVariant qvar_subject = record.value(MSG_SOURCE_COL_SUBJECT);
    const QString  subject = qvar_subject.isValid() ? MTContactHandler::getInstance()->Decode(qvar_subject.toString()) : "";
    // --------------------------------------------------
    const QString& otherNymID = bOutgoing ? recipientNymID : senderNymID;
    const QString& otherAddress  = bOutgoing ? recipientAddr  : senderAddr;
    // --------------------------------------------------
    const bool bUsingNotary   = !NotaryID.isEmpty();
    const bool bIsSpecialMail = !bUsingNotary;
    // --------------------------------------------------
    MTCompose * compose_window = new MTCompose;
    compose_window->setAttribute(Qt::WA_DeleteOnClose);
    // --------------------------------------------------
    if (!myNymID.isEmpty()) // If there's a nym ID.
    {
        if (!myAddress.isEmpty())
            compose_window->setInitialSenderNym(myNymID, myAddress);
        else
            compose_window->setInitialSenderNym(myNymID);
    }
    else if (!myAddress.isEmpty())
        compose_window->setInitialSenderAddress(myAddress);
    // ---------------------------------------
    if (!otherNymID.isEmpty()) // If there's an "other nym ID".
    {
        if (!otherAddress.isEmpty())
            compose_window->setInitialRecipientNym(otherNymID, otherAddress);
        else
            compose_window->setInitialRecipientNym(otherNymID);
    }
    else if (!otherAddress.isEmpty())
        compose_window->setInitialRecipientAddress(otherAddress);
    // --------------------------------------------------
    if (bUsingNotary)
        compose_window->setInitialServer(NotaryID);
    // --------------------------------------------------
    compose_window->setInitialSubject(subject);
    // --------------------------------------------------
    if (nMessageID > 0)
    {
        QString body = MTContactHandler::getInstance()->GetMessageBody(nMessageID);

        if (!body.isEmpty())
            compose_window->setInitialBody(body);
    }
    // --------------------------------------------------
    compose_window->setVariousIds(
                            bOutgoing ? myNymID : senderNymID,
                            bOutgoing ? recipientNymID : myNymID,
                            bOutgoing ? myAddress : senderAddr,
                            bOutgoing ? recipientAddr : myAddress);
    // --------------------------------------------------
    compose_window->dialog();
    Focuser f(compose_window);
    f.show();
    f.focus();
}

void Messages::on_toolButtonForward_clicked()
{
    QPointer<ModelMessages> pModel = DBHandler::getInstance()->getMessageModel();

    if (!pModel)
        return;
    // ------------------------------------
    QModelIndex proxyIndex = pCurrentTabTableView_->currentIndex();

    if (!proxyIndex.isValid())
        return;
    // ------------------------------------
    QModelIndex sourceIndex = pCurrentTabProxyModel_->mapToSource(proxyIndex);

    if (!sourceIndex.isValid())
        return;
    // ------------------------------------
    QSqlRecord record = pModel->record(sourceIndex.row());

    if (record.isEmpty())
        return;
    // ------------------------------------
    const int nMessageID = record.value(MSG_SOURCE_COL_MSG_ID).isValid() ? record.value(MSG_SOURCE_COL_MSG_ID).toInt() : 0;

    const bool bOutgoing = (0 == record.value(MSG_SOURCE_COL_FOLDER).toInt());

    const QVariant qvar_method_type = record.value(MSG_SOURCE_COL_METHOD_TYPE);
    const QString  methodType = qvar_method_type.isValid() ? qvar_method_type.toString() : "";

    const QVariant qvar_my_nym_id = record.value(MSG_SOURCE_COL_MY_NYM);
    const QString  myNymID = qvar_my_nym_id.isValid() ? qvar_my_nym_id.toString() : "";

    const QVariant qvar_my_addr = record.value(MSG_SOURCE_COL_MY_ADDR);
    const QString  myAddress = qvar_my_addr.isValid() ? qvar_my_addr.toString() : "";

    const QVariant qvar_sender_nym_id = record.value(MSG_SOURCE_COL_SENDER_NYM);
    const QString  senderNymID = qvar_sender_nym_id.isValid() ? qvar_sender_nym_id.toString() : "";

    const QVariant qvar_recipient_nym_id = record.value(MSG_SOURCE_COL_RECIP_NYM);
    const QString  recipientNymID = qvar_recipient_nym_id.isValid() ? qvar_recipient_nym_id.toString() : "";

    const QVariant qvar_notary_id = record.value(MSG_SOURCE_COL_NOTARY_ID);
    const QString  NotaryID = qvar_notary_id.isValid() ? qvar_notary_id.toString() : "";

    const QVariant qvar_sender_addr = record.value(MSG_SOURCE_COL_SENDER_ADDR);
    const QString  senderAddr = qvar_sender_addr.isValid() ? qvar_sender_addr.toString() : "";

    const QVariant qvar_recipient_addr = record.value(MSG_SOURCE_COL_RECIP_ADDR);
    const QString  recipientAddr = qvar_recipient_addr.isValid() ? qvar_recipient_addr.toString() : "";

    const QVariant qvar_subject = record.value(MSG_SOURCE_COL_SUBJECT);
    const QString  subject = qvar_subject.isValid() ? MTContactHandler::getInstance()->Decode(qvar_subject.toString()) : "";
    // --------------------------------------------------
    const QString& otherNymID = bOutgoing ? recipientNymID : senderNymID;
    const QString& otherAddress  = bOutgoing ? recipientAddr  : senderAddr;
    // --------------------------------------------------
    const bool bUsingNotary   = !NotaryID.isEmpty();
    const bool bIsSpecialMail = !bUsingNotary;
    // --------------------------------------------------
    MTCompose * compose_window = new MTCompose;
    compose_window->setAttribute(Qt::WA_DeleteOnClose);
    // --------------------------------------------------
    if (!myNymID.isEmpty()) // If there's a nym ID.
    {
        if (!myAddress.isEmpty())
            compose_window->setInitialSenderNym(myNymID, myAddress);
        else
            compose_window->setInitialSenderNym(myNymID);
    }
    else if (!myAddress.isEmpty())
        compose_window->setInitialSenderAddress(myAddress);
    // ---------------------------------------
//    if (!otherNymID.isEmpty()) // If there's an "other nym ID".
//    {
//        if (!otherAddress.isEmpty())
//            compose_window->setInitialRecipientNym(otherNymID, otherAddress);
//        else
//            compose_window->setInitialRecipientNym(otherNymID);
//    }
//    else if (!otherAddress.isEmpty())
//        compose_window->setInitialRecipientAddress(otherAddress);
    // --------------------------------------------------
    if (bUsingNotary)
        compose_window->setInitialServer(NotaryID);
    // --------------------------------------------------
    compose_window->setInitialSubject(subject);
    // --------------------------------------------------
    if (nMessageID > 0)
    {
        QString body = MTContactHandler::getInstance()->GetMessageBody(nMessageID);

        if (!body.isEmpty())
            compose_window->setInitialBody(body);
    }
    // --------------------------------------------------
    compose_window->setForwarded();
    // --------------------------------------------------
    compose_window->setVariousIds(
                            bOutgoing ? myNymID : senderNymID,
                            bOutgoing ? recipientNymID : myNymID,
                            bOutgoing ? myAddress : senderAddr,
                            bOutgoing ? recipientAddr : myAddress);
    // --------------------------------------------------
    compose_window->dialog();
    Focuser f(compose_window);
    f.show();
    f.focus();
}


//QTableView         * pCurrentTabTableView_  = nullptr;
//MessagesProxyModel * pCurrentTabProxyModel_ = nullptr;
//


void Messages::on_toolButtonDelete_clicked()
{
    if ( (nullptr != pCurrentTabTableView_) &&
         (nullptr != pCurrentTabProxyModel_) )
    {
        if (!pCurrentTabTableView_->selectionModel()->hasSelection())
            return;
        // ----------------------------------------------
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, tr("Moneychanger"), QString("%1<br/><br/>%2").arg(tr("Are you sure you want to delete these messages?")).
                                      arg(tr("WARNING: This is not reversible!")),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply != QMessageBox::Yes)
            return;
        // ----------------------------------------------
        QPointer<ModelMessages> pModel = DBHandler::getInstance()->getMessageModel();

        if (pModel)
        {
            QItemSelection selection( pCurrentTabTableView_->selectionModel()->selection() );

            int nFirstProxyRowRemoved = -1;
            int nLastProxyRowRemoved  = -1;
            int nCountRowsRemoved     = 0;

            QList<int> rows, message_ids;
            foreach( const QModelIndex & index, selection.indexes() ) {
                QModelIndex sourceIndex = pCurrentTabProxyModel_->mapToSource(index);
                rows.append( sourceIndex.row() );
                // --------------------------------
                nLastProxyRowRemoved = index.row();
                if ((-1) == nFirstProxyRowRemoved)
                    nFirstProxyRowRemoved = index.row();
            }

            qSort( rows );

            bool bRemoved = false;

            int prev = -1;

            for(int ii = rows.count() - 1; ii >= 0; ii -= 1 ) {
               int current = rows[ii];
               if( current != prev ) {
                   bRemoved = true;
                   QModelIndex sourceIndexMsgID = pModel->index(current, MSG_SOURCE_COL_MSG_ID);
                   if (sourceIndexMsgID.isValid())
                       message_ids.append(pModel->data(sourceIndexMsgID).toInt());
                   pModel->removeRows( current, 1 );
                   prev = current;
                   nCountRowsRemoved++;
               }
            }

            if (bRemoved)
            {
                if (pModel->submitAll())
                {
                    pModel->database().commit();
                    // ------------------------
                    // Now we just deleted some messages; let's delete also the corresponding
                    // message bodies. (We saved the deleted IDs for this purpose.)
                    //
                    for (int ii = 0; ii < message_ids.count(); ++ii)
                    {
                        const int nMsgID = message_ids[ii];

                        if (nMsgID > 0)
                            if (!MTContactHandler::getInstance()->DeleteMessageBody(nMsgID))
                                qDebug() << "Messages::on_toolButtonDelete_clicked: Failed trying to delete message body with message_id: " << nMsgID << "\n";
                    }
                    // ------------------------
                    // We just deleted the selected rows.
                    // So now we need to choose another row to select.

                    int nRowToSelect = -1;

                    if ((nFirstProxyRowRemoved >= 0) && (nFirstProxyRowRemoved < pCurrentTabProxyModel_->rowCount()))
                        nRowToSelect = nFirstProxyRowRemoved;
                    else if (0 == nFirstProxyRowRemoved)
                        nRowToSelect = 0;
                    else if (nFirstProxyRowRemoved > 0)
                        nRowToSelect = pCurrentTabProxyModel_->rowCount() - 1;
                    else
                        nRowToSelect = 0;

                    if ((pCurrentTabProxyModel_->rowCount() > 0) && (nRowToSelect >= 0) &&
                            (nRowToSelect < pCurrentTabProxyModel_->rowCount()))
                    {
                        QModelIndex previous = pCurrentTabTableView_->currentIndex();
                        pCurrentTabTableView_->blockSignals(true);
                        pCurrentTabTableView_->selectRow(nRowToSelect);
                        pCurrentTabTableView_->blockSignals(false);

                        if (pCurrentTabTableView_ == ui->tableViewReceived)
                            on_tableViewReceivedSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
                        else
                            on_tableViewSentSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
                    }
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
}

void Messages::on_toolButtonRefresh_clicked()
{
    emit needToDownloadMail();
}

void Messages::onRecordlistPopulated()
{
    RefreshAll();
}

void Messages::RefreshAll()
{
    RefreshTree();
}

void Messages::on_pushButtonSearch_clicked()
{
    QString qstrSearchText = ui->lineEdit->text();

    this->doSearch(qstrSearchText.simplified());
}

void Messages::doSearch(QString qstrInput)
{
    if (pMsgProxyModelInbox_)
    {
        pMsgProxyModelInbox_ ->setFilterString(qstrInput);
    }
    if (pMsgProxyModelOutbox_)
    {
        pMsgProxyModelOutbox_->setFilterString(qstrInput);
    }

    RefreshMessages();
}

void Messages::on_lineEdit_textChanged(const QString &arg1)
{
    // This means someone clicked the "clear" button on the search box.
    if (arg1.isEmpty())
        doSearch(arg1);
}

void Messages::on_lineEdit_returnPressed()
{
    QString qstrSearchText = ui->lineEdit->text();

    this->doSearch(qstrSearchText.simplified());
}

Messages::~Messages()
{
    delete ui;
}

bool Messages::eventFilter(QObject *obj, QEvent *event)
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
    return QWidget::eventFilter(obj, event);
}



