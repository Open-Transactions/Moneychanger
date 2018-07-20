#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/accountdetails.hpp>
#include <ui_accountdetails.h>

#include <gui/ui/dlgexportedtopass.hpp>
#include <gui/ui/dlgexportedcash.hpp>
#include <gui/ui/getstringdialog.hpp>
#include <gui/widgets/compose.hpp>
#include <gui/widgets/dlgchooser.hpp>

#include <gui/widgets/cashpurse.hpp>
#include <gui/widgets/home.hpp>
#include <gui/widgets/wizardaddaccount.hpp>
#include <gui/widgets/overridecursor.hpp>

#include <core/moneychanger.hpp>
#include <core/handlers/modelpayments.hpp>
#include <core/handlers/DBHandler.hpp>
#include <core/handlers/contacthandler.hpp>

#include <opentxs/opentxs.hpp>

#include <QMessageBox>
#include <QMenu>
#include <QTimer>
#include <QDebug>


static void setup_tableview(QTableView * pView, QAbstractItemModel * pProxyModel)
{
    AccountRecordsProxyModel * pPmntProxyModel = static_cast<AccountRecordsProxyModel *>(pProxyModel);
    pPmntProxyModel->setTableView(pView);

    pView->setModel(pProxyModel);
    pView->setSortingEnabled(true);
    pView->resizeColumnsToContents();
    pView->resizeRowsToContents();
    pView->horizontalHeader()->setStretchLastSection(true);
    pView->setEditTriggers(QAbstractItemView::NoEditTriggers);

//    QPointer<ModelPayments> pSourceModel = DBHandler::getInstance()->getPaymentModel();

//    if (pSourceModel)
    {
//        QModelIndex sourceIndex = pSourceModel->index(0, PMNT_SOURCE_COL_TIMESTAMP);
//        QModelIndex proxyIndex  = (static_cast<AccountRecordsProxyModel *>(pProxyModel)) -> mapFromSource(sourceIndex);
        // ----------------------------------
//        AccountRecordsProxyModel * pPmntProxyModel = static_cast<AccountRecordsProxyModel *>(pProxyModel);

      pView->sortByColumn(3, Qt::DescendingOrder); // The timestamp ends up at index 7 in all the proxy views.

//        qDebug() << "SORT COLUMN: " << proxyIndex.column() << "\n";

    }
    pView->setContextMenuPolicy(Qt::CustomContextMenu);
    pView->verticalHeader()->hide();
    pView->setAlternatingRowColors(true);
    pView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    pView->setSelectionBehavior(QAbstractItemView::SelectRows);
}


void MTAccountDetails::on_tableView_customContextMenuRequested(const QPoint &pos)
{
    if (!pPmntProxyModel_)
        return;

    QPointer<AccountRecordsProxyModel> & pProxyModel = pPmntProxyModel_;

    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

    if (!pModel)
        return;
    // ------------------------
    QTableView * pTableView = ui->tableView;

    QModelIndex indexAtRightClick = pTableView->indexAt(pos);
    if (!indexAtRightClick.isValid())
        return;
    // ------------------------
    QModelIndex sourceIndexAtRightClick = pProxyModel->mapToSource(indexAtRightClick);
    const int nRow = sourceIndexAtRightClick.row();
    // ----------------------------------
    QAction * pActionViewInPayments = nullptr;
    QAction * pActionOpenNewWindow  = nullptr;

    popupMenu_.reset(new QMenu(pTableView));
    pActionOpenNewWindow = popupMenu_->addAction(tr("View instrument(s)"));
    pActionViewInPayments = popupMenu_->addAction(tr("View in payments window"));
    popupMenu_->addSeparator();
    pActionForward = popupMenu_->addAction(tr("Forward"));
    popupMenu_->addSeparator();
    pActionDelete = popupMenu_->addAction(tr("Delete"));
    popupMenu_->addSeparator();
    pActionMarkRead = popupMenu_->addAction(tr("Mark as read"));
    pActionMarkUnread = popupMenu_->addAction(tr("Mark as unread"));
    // ----------------------------------
    pActionViewContact         = nullptr;
    pActionCreateContact       = nullptr;
    pActionExistingContact     = nullptr;
    pActionAcceptIncoming      = nullptr;
    pActionCancelOutgoing      = nullptr;
    pActionDiscardOutgoingCash = nullptr;
    pActionDiscardIncoming     = nullptr;

    int nContactId = 0;

    QString qstrSenderNymId;
    QString qstrSenderAddr;
    QString qstrRecipientNymId;
    QString qstrRecipientAddr;
    QString qstrMsgNotaryId;
    QString qstrPmntNotaryId;
    QString qstrMethodType;
//  QString qstrSubject;

    int nSenderContactByNym     = 0;
    int nSenderContactByAddr    = 0;
    int nRecipientContactByNym  = 0;
    int nRecipientContactByAddr = 0;

    ModelPayments::PaymentFlags flags = ModelPayments::NoFlags;
    // ----------------------------------------------
    // Look at the data for indexAtRightClick and see if I have a contact already in the
    // address book. If so, add the "View Contact" option to the menu. But if not, add the
    // "Create Contact" and "Add to Existing Contact" options to the menu instead.
    //
    // UPDATE: I've now also added similar functionality, for other actions specific
    // to certain payment records, based on their flags. (Pay this invoice, deposit
    // this cash, etc.)
    //
    if (nRow >= 0)
    {
        QModelIndex indexSenderNym     = pModel->index(nRow, PMNT_SOURCE_COL_SENDER_NYM);
        QModelIndex indexSenderAddr    = pModel->index(nRow, PMNT_SOURCE_COL_SENDER_ADDR);
        QModelIndex indexRecipientNym  = pModel->index(nRow, PMNT_SOURCE_COL_RECIP_NYM);
        QModelIndex indexRecipientAddr = pModel->index(nRow, PMNT_SOURCE_COL_RECIP_ADDR);
        QModelIndex indexMsgNotaryId   = pModel->index(nRow, PMNT_SOURCE_COL_MSG_NOTARY_ID);
        QModelIndex indexPmntNotaryId  = pModel->index(nRow, PMNT_SOURCE_COL_PMNT_NOTARY_ID);
        QModelIndex indexMethodType    = pModel->index(nRow, PMNT_SOURCE_COL_METHOD_TYPE);
        QModelIndex indexFlags         = pModel->index(nRow, PMNT_SOURCE_COL_FLAGS);
//      QModelIndex indexSubject       = pModel->index(nRow, PMNT_SOURCE_COL_MEMO);

        QVariant varSenderNym     = pModel->rawData(indexSenderNym);
        QVariant varSenderAddr    = pModel->rawData(indexSenderAddr);
        QVariant varRecipientNym  = pModel->rawData(indexRecipientNym);
        QVariant varRecipientAddr = pModel->rawData(indexRecipientAddr);
        QVariant varMsgNotaryId   = pModel->rawData(indexMsgNotaryId);
        QVariant varPmntNotaryId  = pModel->rawData(indexPmntNotaryId);
        QVariant varMethodType    = pModel->rawData(indexMethodType);
        QVariant varFlags         = pModel->rawData(indexFlags);
//      QVariant varSubject       = pModel->rawData(indexSubject);

        qint64 lFlags      = varFlags        .isValid() ? varFlags        .toLongLong() : 0;
        qstrSenderNymId    = varSenderNym    .isValid() ? varSenderNym    .toString()   : QString("");
        qstrSenderAddr     = varSenderAddr   .isValid() ? varSenderAddr   .toString()   : QString("");
        qstrRecipientNymId = varRecipientNym .isValid() ? varRecipientNym .toString()   : QString("");
        qstrRecipientAddr  = varRecipientAddr.isValid() ? varRecipientAddr.toString()   : QString("");
        qstrMsgNotaryId    = varMsgNotaryId  .isValid() ? varMsgNotaryId  .toString()   : QString("");
        qstrPmntNotaryId   = varPmntNotaryId .isValid() ? varPmntNotaryId .toString()   : QString("");
        qstrMethodType     = varMethodType   .isValid() ? varMethodType   .toString()   : QString("");
//      qstrSubject        = varSubject      .isValid() ? varSubject      .toString()   : QString("");

        nSenderContactByNym     = qstrSenderNymId.isEmpty()    ? 0 : MTContactHandler::getInstance()->FindContactIDByNymID(qstrSenderNymId);
        nSenderContactByAddr    = qstrSenderAddr.isEmpty()     ? 0 : MTContactHandler::getInstance()->GetContactByAddress(qstrSenderAddr);
        nRecipientContactByNym  = qstrRecipientNymId.isEmpty() ? 0 : MTContactHandler::getInstance()->FindContactIDByNymID(qstrRecipientNymId);
        nRecipientContactByAddr = qstrRecipientAddr.isEmpty()  ? 0 : MTContactHandler::getInstance()->GetContactByAddress(qstrRecipientAddr);

        nContactId = (nSenderContactByNym > 0) ? nSenderContactByNym : nSenderContactByAddr;

        if (nContactId <= 0)
            nContactId = (nRecipientContactByNym > 0) ? nRecipientContactByNym : nRecipientContactByAddr;

        flags = ModelPayments::PaymentFlag(static_cast<ModelPayments::PaymentFlag>(lFlags));
        // -------------------------------
        popupMenu_->addSeparator();
        // -------------------------------
        if (nContactId > 0) // There's a known contact for this payment.
            pActionViewContact = popupMenu_->addAction(tr("View contact in address book"));
        else // There is no known contact for this payment.
        {
            pActionCreateContact = popupMenu_->addAction(tr("Create new contact in address book"));
            pActionExistingContact = popupMenu_->addAction(tr("Add to existing contact in address book"));
        }
        // -------------------------------
        popupMenu_->addSeparator();
        // -------------------------------
        if ( flags.testFlag(ModelPayments::CanAcceptIncoming))
        {
            QString nameString;
            QString actionString;

            if ( flags.testFlag(ModelPayments::IsTransfer) )
            {
                nameString = tr("Accept this Transfer");
                actionString = tr("Accepting...");
            }
            else if ( flags.testFlag(ModelPayments::IsReceipt) )
            {
                nameString = tr("Accept this Receipt");
                actionString = tr("Accepting...");
            }
            else if ( flags.testFlag(ModelPayments::IsInvoice) )
            {
                nameString = tr("Pay this Invoice");
                actionString = tr("Paying...");
            }
            else if ( flags.testFlag(ModelPayments::IsPaymentPlan) )
            {
                nameString = tr("Activate this Payment Plan");
                actionString = tr("Activating...");
            }
            else if ( flags.testFlag(ModelPayments::IsContract) )
            {
                nameString = tr("Sign this Smart Contract");
                actionString = tr("Signing...");
            }
            else if ( flags.testFlag(ModelPayments::IsCash) )
            {
                nameString = tr("Deposit this Cash");
                actionString = tr("Depositing...");
            }
            else if ( flags.testFlag(ModelPayments::IsCheque) )
            {
                nameString = tr("Deposit this Cheque");
                actionString = tr("Depositing...");
            }
            else if ( flags.testFlag(ModelPayments::IsVoucher) )
            {
                nameString = tr("Accept this Payment");
                actionString = tr("Accepting...");
            }
            else
            {
                nameString = tr("Deposit this Payment");
                actionString = tr("Depositing...");
            }

            pActionAcceptIncoming = popupMenu_->addAction(nameString);
        }

        if (flags.testFlag(ModelPayments::CanCancelOutgoing))
        {
            QString cancelString;
            QString actionString = tr("Canceling...");
//          QString msg = tr("Cancellation Failed. Perhaps recipient had already accepted it?");

            if (flags.testFlag(ModelPayments::IsInvoice))
                cancelString = tr("Cancel this Invoice");
            else if (flags.testFlag(ModelPayments::IsPaymentPlan))
                cancelString = tr("Cancel this Payment Plan");
            else if (flags.testFlag(ModelPayments::IsContract))
                cancelString = tr("Cancel this Smart Contract");
            else if (flags.testFlag(ModelPayments::IsCash))
            {
                cancelString = tr("Recover this Cash");
                actionString = tr("Recovering...");
//              msg = tr("Recovery Failed. Perhaps recipient had already accepted it?");
            }
            else if (flags.testFlag(ModelPayments::IsCheque))
                cancelString = tr("Cancel this Cheque");
            else if (flags.testFlag(ModelPayments::IsVoucher))
                cancelString = tr("Cancel this Payment");
            else
                cancelString = tr("Cancel this Payment");

            pActionCancelOutgoing = popupMenu_->addAction(cancelString);
        }

        if (flags.testFlag(ModelPayments::CanDiscardOutgoingCash))
        {
            QString discardString = tr("Discard this Sent Cash");

            pActionDiscardOutgoingCash = popupMenu_->addAction(discardString);
        }

        if (flags.testFlag(ModelPayments::CanDiscardIncoming))
        {
            QString discardString;

            if (flags.testFlag(ModelPayments::IsInvoice))
                discardString = tr("Discard this Invoice");
            else if (flags.testFlag(ModelPayments::IsPaymentPlan))
                discardString = tr("Discard this Payment Plan");
            else if (flags.testFlag(ModelPayments::IsContract))
                discardString = tr("Discard this Smart Contract");
            else if (flags.testFlag(ModelPayments::IsCash))
                discardString = tr("Discard this Cash");
            else if (flags.testFlag(ModelPayments::IsCheque))
                discardString = tr("Discard this Cheque");
            else if (flags.testFlag(ModelPayments::IsVoucher))
                discardString = tr("Discard this Payment");
            else
                discardString = tr("Discard this Payment");

            pActionDiscardIncoming = popupMenu_->addAction(discardString);
        }
    }
    // --------------------------------------------------
    QPoint globalPos = pTableView->mapToGlobal(pos);
    const QAction* selectedAction = popupMenu_->exec(globalPos); // Here we popup the menu, and get the user's click.
    if (nullptr == selectedAction)
        return;
    // ----------------------------------
    if (selectedAction == pActionAcceptIncoming) // Only approves the current payment.
    {
        pTableView->setCurrentIndex(indexAtRightClick);
        AcceptIncoming(pModel, pProxyModel, nRow, pTableView);
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionCancelOutgoing) // Only cancels the current payment.
    {
        pTableView->setCurrentIndex(indexAtRightClick);
        CancelOutgoing(pModel, pProxyModel, nRow, pTableView);
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionDiscardOutgoingCash) // Only discards the current payment.
    {
        pTableView->setCurrentIndex(indexAtRightClick);
        DiscardOutgoingCash(pModel, pProxyModel, nRow, pTableView);
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionDiscardIncoming) // Only discards the current payment.
    {
        pTableView->setCurrentIndex(indexAtRightClick);
        DiscardIncoming(pModel, pProxyModel, nRow, pTableView);
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionReply) // Only replies to the current payment.
    {
        pTableView->setCurrentIndex(indexAtRightClick);
        on_toolButtonReply_clicked();
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionForward) // Only fowards the current payments.
    {
        pTableView->setCurrentIndex(indexAtRightClick);
        on_toolButtonForward_clicked();
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionDelete) // May delete many payments.
    {
        on_toolButtonDelete_clicked();
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionOpenNewWindow) // May open many payments.
    {
        pTableView->setCurrentIndex(indexAtRightClick);

        on_tableView_doubleClicked(indexAtRightClick); // just one for now. baby steps!
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionViewInPayments) // View this transaction in the payments window.
    {
        pTableView->setCurrentIndex(indexAtRightClick);
        // ------------------------------------
        QSqlRecord record = pModel->record(nRow);
        if (record.isEmpty())
            return;
        // ------------------------------------
        //const bool bOutgoing = (0 == record.value(PMNT_SOURCE_COL_FOLDER).toInt());
        // We emit this signal, passing the SOURCE ROW and the folder.
        const int & nSourceRow = nRow;
        const int   nFolder = record.value(PMNT_SOURCE_COL_FOLDER).isValid() ? record.value(PMNT_SOURCE_COL_FOLDER).toInt() : -1;
        if (-1 != nFolder)
            emit showPayment(nSourceRow, nFolder);
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionMarkRead) // May mark many payments.
    {
        if (!pTableView->selectionModel()->hasSelection())
            return;
        // ----------------------------------------------
        QItemSelection selection( pTableView->selectionModel()->selection() );
        QList<int> rows;
        foreach( const QModelIndex & index, selection.indexes() )
        {
            if (rows.indexOf(index.row()) != (-1)) // This row is already on the list, so skip it.
                continue;
            rows.append(index.row());
            // -----------------------
            QModelIndex sourceIndex = pProxyModel->mapToSource(index);
            QModelIndex sourceIndexHaveRead = pModel->sibling(sourceIndex.row(), PMNT_SOURCE_COL_HAVE_READ, sourceIndex);
            // --------------------------------
            if (sourceIndexHaveRead.isValid())
                listRecordsToMarkAsRead_.append(sourceIndexHaveRead);
        }
        if (listRecordsToMarkAsRead_.count() > 0)
            QTimer::singleShot(0, this, SLOT(on_MarkAsRead_timer()));
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionMarkUnread) // May mark many payments.
    {
        if (!pTableView->selectionModel()->hasSelection())
            return;
        // ----------------------------------------------
        QItemSelection selection( pTableView->selectionModel()->selection() );
        QList<int> rows;
        foreach( const QModelIndex & index, selection.indexes() )
        {
            if (rows.indexOf(index.row()) != (-1)) // This row is already on the list, so skip it.
                continue;
            rows.append(index.row());
            // -----------------------
            QModelIndex sourceIndex = pProxyModel->mapToSource(index);
            QModelIndex sourceIndexHaveRead = pModel->sibling(sourceIndex.row(), PMNT_SOURCE_COL_HAVE_READ, sourceIndex);
            // --------------------------------
            if (sourceIndexHaveRead.isValid())
                listRecordsToMarkAsUnread_.append(sourceIndexHaveRead);
        }
        if (listRecordsToMarkAsUnread_.count() > 0)
            QTimer::singleShot(0, this, SLOT(on_MarkAsUnread_timer()));
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionViewContact)
    {
        pTableView->setCurrentIndex(indexAtRightClick);

        if (nContactId > 0)
        {
            QString qstrContactId = QString::number(nContactId);
            emit showContact(qstrContactId);
        }
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionCreateContact)
    {
        pTableView->setCurrentIndex(indexAtRightClick);

        MTGetStringDialog nameDlg(this, tr("Enter a name for the new contact"));

        if (QDialog::Accepted != nameDlg.exec())
            return;
        // --------------------------------------
        QString strNewContactName = nameDlg.GetOutputString();
        // --------------------------------------------------
        // NOTE:
        // if nSenderContactByNym > 0, then the sender Nym already has a contact.
        // else if nSenderContactByNym == 0 but qstrSenderNymId exists, that means it
        // contains a NymID that could be added to an existing contact, or used to
        // create a new contact. (And the same is true for the Sender Address.)
        //
        // (And the same is also true for the recipient nymID and address.)
        //
        const QString qstrNotaryId(qstrPmntNotaryId.isEmpty() ? qstrMsgNotaryId : qstrPmntNotaryId);
        if ((0 == nSenderContactByNym) && !qstrSenderNymId.isEmpty())
            nContactId = MTContactHandler::getInstance()->CreateContactBasedOnNym(qstrSenderNymId, qstrNotaryId);
        else if ((0 == nSenderContactByAddr) && !qstrSenderAddr.isEmpty())
            nContactId = MTContactHandler::getInstance()->CreateContactBasedOnAddress(qstrSenderAddr, qstrMethodType);
        else if ((0 == nRecipientContactByNym) && !qstrRecipientNymId.isEmpty())
            nContactId = MTContactHandler::getInstance()->CreateContactBasedOnNym(qstrRecipientNymId, qstrNotaryId);
        else if ((0 == nRecipientContactByAddr) && !qstrRecipientAddr.isEmpty())
            nContactId = MTContactHandler::getInstance()->CreateContactBasedOnAddress(qstrRecipientAddr, qstrMethodType);
        // -----------------------------------------------------
        if (nContactId > 0)
        {
            MTContactHandler::getInstance()->SetContactName(nContactId, strNewContactName);
            // ---------------------------------
            QString qstrContactID = QString("%1").arg(nContactId);
            emit showContactAndRefreshHome(qstrContactID);
        }
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionExistingContact)
    {
        pTableView->setCurrentIndex(indexAtRightClick);

        // This should never happen since we wouldn't even have gotten this menu option
        // in the first place, unless contact ID had been 0.
        if (nContactId > 0)
            return;

        // (And that means no contact was found for ANY of the Nym IDs or Addresses on this payment.)
        // That means we can add the first one we find (which will probably be the only one as well.)
        // Because I'll EITHER have a SenderNymID OR SenderAddress,
        // ...OR I'll have a RecipientNymID OR RecipientAddress.
        // Thus, only one of the four IDs/Addresses will actually be found.
        // Therefore I don't care which one I find first:
        //
        QString qstrAddress, qstrNymId;

        if      (!qstrSenderNymId.isEmpty())    qstrNymId   = qstrSenderNymId;
        else if (!qstrSenderAddr.isEmpty())     qstrAddress = qstrSenderAddr;
        else if (!qstrRecipientNymId.isEmpty()) qstrNymId   = qstrRecipientNymId;
        else if (!qstrRecipientAddr.isEmpty())  qstrAddress = qstrRecipientAddr;
        // ---------------------------------------------------
        if (qstrNymId.isEmpty() && qstrAddress.isEmpty()) // Should never happen.
            return;
        // Below this point we're guaranteed that there's either a NymID or an Address.
        // ---------------------------------------------------
        if (!qstrNymId.isEmpty() && (MTContactHandler::getInstance()->FindContactIDByNymID(qstrNymId) > 0))
        {
            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                 tr("Strange: NymID %1 already belongs to an existing contact.").arg(qstrNymId));
            return;
        }
        // ---------------------------------------------------
        if (!qstrAddress.isEmpty() && MTContactHandler::getInstance()->GetContactByAddress(qstrAddress) > 0)
        {
            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                 tr("Strange: Address %1 already belongs to an existing contact.").arg(qstrAddress));
            return;
        }
        // --------------------------------------------------------------------
        // Pop up a Contact selection box. The user chooses an existing contact.
        // If OK (vs Cancel) then add the Nym / Acct to the existing contact selected.
        //
        DlgChooser theChooser(this);
        // -----------------------------------------------
        mapIDName & the_map = theChooser.m_map;
        MTContactHandler::getInstance()->GetContacts(the_map);
        // -----------------------------------------------
        theChooser.setWindowTitle(tr("Choose an Existing Contact"));
        if (theChooser.exec() != QDialog::Accepted)
            return;
        // -----------------------------------------------
        QString strContactID = theChooser.GetCurrentID();
        nContactId = strContactID.isEmpty() ? 0 : strContactID.toInt();

        if (nContactId > 0)
        {
            if (!qstrNymId.isEmpty()) // We're adding this NymID to the contact.
            {
                if (!MTContactHandler::getInstance()->AddNymToExistingContact(nContactId, qstrNymId))
                {
                    QString strContactName(MTContactHandler::getInstance()->GetContactName(nContactId));
                    QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), QString("Failed while trying to add NymID %1 to existing contact '%2' with contact ID: %3").
                                         arg(qstrNymId).arg(strContactName).arg(nContactId));
                    return;
                }
                if (!qstrPmntNotaryId.isEmpty())
                    MTContactHandler::getInstance()->NotifyOfNymServerPair(qstrNymId, qstrPmntNotaryId);
            }
            else if (!qstrAddress.isEmpty()) // We're adding this Address to the contact.
            {
                if (!MTContactHandler::getInstance()->AddMsgAddressToContact(nContactId, qstrMethodType, qstrAddress))
                {
                    QString strContactName(MTContactHandler::getInstance()->GetContactName(nContactId));
                    QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), QString("Failed while trying to add Address %1 to existing contact '%2' with contact ID: %3").
                                         arg(qstrAddress).arg(strContactName).arg(nContactId));
                    return;
                }
            }
            // ---------------------------------
            // Display the normal contacts dialog, with the new contact
            // being the one selected.
            //
            QString qstrContactID = QString("%1").arg(nContactId);
            emit showContactAndRefreshHome(qstrContactID);
            // ---------------------------------
        } // nContactID > 0
    }
}




void MTAccountDetails::on_toolButtonReply_clicked()
{
    QTableView * pTableView = ui->tableView;


    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

    if (!pModel)
        return;
    // ------------------------------------
    QModelIndex proxyIndex = pTableView->currentIndex();

    if (!proxyIndex.isValid())
        return;
    // ------------------------------------
    QModelIndex sourceIndex = pPmntProxyModel_->mapToSource(proxyIndex);

    if (!sourceIndex.isValid())
        return;

    QModelIndex haveRepliedIndex = pModel->sibling(sourceIndex.row(),
                                                   PMNT_SOURCE_COL_HAVE_REPLIED,
                                                   sourceIndex);
    // ------------------------------------
    QSqlRecord record = pModel->record(sourceIndex.row());

    if (record.isEmpty())
        return;
    // ------------------------------------
    const int nPaymentID = record.value(PMNT_SOURCE_COL_PMNT_ID).isValid() ? record.value(PMNT_SOURCE_COL_PMNT_ID).toInt() : 0;

    const bool bOutgoing = (0 == record.value(PMNT_SOURCE_COL_FOLDER).toInt());

    const QVariant qvar_method_type = record.value(PMNT_SOURCE_COL_METHOD_TYPE);
    const QString  methodType = qvar_method_type.isValid() ? qvar_method_type.toString() : "";

    const QVariant qvar_my_nym_id = record.value(PMNT_SOURCE_COL_MY_NYM);
    const QString  myNymID = qvar_my_nym_id.isValid() ? qvar_my_nym_id.toString() : "";

    const QVariant qvar_my_addr = record.value(PMNT_SOURCE_COL_MY_ADDR);
    const QString  myAddress = qvar_my_addr.isValid() ? qvar_my_addr.toString() : "";

    const QVariant qvar_sender_nym_id = record.value(PMNT_SOURCE_COL_SENDER_NYM);
    const QString  senderNymID = qvar_sender_nym_id.isValid() ? qvar_sender_nym_id.toString() : "";

    const QVariant qvar_recipient_nym_id = record.value(PMNT_SOURCE_COL_RECIP_NYM);
    const QString  recipientNymID = qvar_recipient_nym_id.isValid() ? qvar_recipient_nym_id.toString() : "";

    const QVariant qvar_msg_notary_id = record.value(PMNT_SOURCE_COL_MSG_NOTARY_ID);
    const QString  MsgNotaryID = qvar_msg_notary_id.isValid() ? qvar_msg_notary_id.toString() : "";

    const QVariant qvar_pmnt_notary_id = record.value(PMNT_SOURCE_COL_PMNT_NOTARY_ID);
    const QString  PmntNotaryID = qvar_pmnt_notary_id.isValid() ? qvar_pmnt_notary_id.toString() : "";

    const QVariant qvar_sender_addr = record.value(PMNT_SOURCE_COL_SENDER_ADDR);
    const QString  senderAddr = qvar_sender_addr.isValid() ? qvar_sender_addr.toString() : "";

    const QVariant qvar_recipient_addr = record.value(PMNT_SOURCE_COL_RECIP_ADDR);
    const QString  recipientAddr = qvar_recipient_addr.isValid() ? qvar_recipient_addr.toString() : "";

    const QVariant qvar_subject = record.value(PMNT_SOURCE_COL_MEMO);
    const QString  subject = qvar_subject.isValid() ? MTContactHandler::getInstance()->Decode(qvar_subject.toString()) : "";
    // --------------------------------------------------
    const QString& otherNymID = bOutgoing ? recipientNymID : senderNymID;
    const QString& otherAddress  = bOutgoing ? recipientAddr  : senderAddr;
    // --------------------------------------------------
    const bool bUsingNotary   = !MsgNotaryID.isEmpty();
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
        compose_window->setInitialServer(MsgNotaryID);
    // --------------------------------------------------
    compose_window->setInitialSubject(subject);
    // --------------------------------------------------
    if (nPaymentID > 0)
    {
        QString body        = MTContactHandler::getInstance()->GetPaymentBody(nPaymentID);
        QString pendingBody = MTContactHandler::getInstance()->GetPaymentPendingBody(nPaymentID);

        if (!body.isEmpty())
            compose_window->setInitialBody(body);
        else if (!pendingBody.isEmpty())
            compose_window->setInitialBody(pendingBody);
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
    // -----------------------------
    if (haveRepliedIndex.isValid())
        listRecordsToMarkAsReplied_.append(haveRepliedIndex);
    if (listRecordsToMarkAsReplied_.count() > 0)
        QTimer::singleShot(0, this, SLOT(on_MarkAsReplied_timer()));
}


// --------------------------------------------------
// TODO resume: payments::AcceptIncoming, CancelOutgoing, DiscardOutgoingCash, and DiscardIncoming:

void MTAccountDetails::AcceptIncoming(QPointer<ModelPayments> & pModel, AccountRecordsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView)
{
    emit showDashboard();
}

void MTAccountDetails::CancelOutgoing(QPointer<ModelPayments> & pModel, AccountRecordsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView)
{
    emit showDashboard();
}

void MTAccountDetails::DiscardOutgoingCash(QPointer<ModelPayments> & pModel, AccountRecordsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView)
{
    emit showDashboard();
}

void MTAccountDetails::DiscardIncoming(QPointer<ModelPayments> & pModel, AccountRecordsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView)
{
    emit showDashboard();
}
// --------------------------------------------------

void MTAccountDetails::on_toolButtonForward_clicked()
{
    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

    if (!pModel)
        return;
    // ------------------------------------
    QTableView * pTableView = ui->tableView;

    QModelIndex proxyIndex = pTableView->currentIndex();

    if (!proxyIndex.isValid())
        return;
    // ------------------------------------
    QModelIndex sourceIndex = pPmntProxyModel_->mapToSource(proxyIndex);

    if (!sourceIndex.isValid())
        return;

    QModelIndex haveForwardedIndex = pModel->sibling(sourceIndex.row(),
                                                     PMNT_SOURCE_COL_HAVE_FORWARDED,
                                                     sourceIndex);
    // ------------------------------------
    QSqlRecord record = pModel->record(sourceIndex.row());

    if (record.isEmpty())
        return;
    // ------------------------------------
    const int nPaymentID = record.value(PMNT_SOURCE_COL_PMNT_ID).isValid() ? record.value(PMNT_SOURCE_COL_PMNT_ID).toInt() : 0;

    const bool bOutgoing = (0 == record.value(PMNT_SOURCE_COL_FOLDER).toInt());

    const QVariant qvar_method_type = record.value(PMNT_SOURCE_COL_METHOD_TYPE);
    const QString  methodType = qvar_method_type.isValid() ? qvar_method_type.toString() : "";

    const QVariant qvar_my_nym_id = record.value(PMNT_SOURCE_COL_MY_NYM);
    const QString  myNymID = qvar_my_nym_id.isValid() ? qvar_my_nym_id.toString() : "";

    const QVariant qvar_my_addr = record.value(PMNT_SOURCE_COL_MY_ADDR);
    const QString  myAddress = qvar_my_addr.isValid() ? qvar_my_addr.toString() : "";

    const QVariant qvar_sender_nym_id = record.value(PMNT_SOURCE_COL_SENDER_NYM);
    const QString  senderNymID = qvar_sender_nym_id.isValid() ? qvar_sender_nym_id.toString() : "";

    const QVariant qvar_recipient_nym_id = record.value(PMNT_SOURCE_COL_RECIP_NYM);
    const QString  recipientNymID = qvar_recipient_nym_id.isValid() ? qvar_recipient_nym_id.toString() : "";

    const QVariant qvar_msg_notary_id = record.value(PMNT_SOURCE_COL_MSG_NOTARY_ID);
    const QString  MsgNotaryID = qvar_msg_notary_id.isValid() ? qvar_msg_notary_id.toString() : "";

    const QVariant qvar_pmnt_notary_id = record.value(PMNT_SOURCE_COL_PMNT_NOTARY_ID);
    const QString  PmntNotaryID = qvar_pmnt_notary_id.isValid() ? qvar_pmnt_notary_id.toString() : "";

    const QVariant qvar_sender_addr = record.value(PMNT_SOURCE_COL_SENDER_ADDR);
    const QString  senderAddr = qvar_sender_addr.isValid() ? qvar_sender_addr.toString() : "";

    const QVariant qvar_recipient_addr = record.value(PMNT_SOURCE_COL_RECIP_ADDR);
    const QString  recipientAddr = qvar_recipient_addr.isValid() ? qvar_recipient_addr.toString() : "";

    const QVariant qvar_subject = record.value(PMNT_SOURCE_COL_MEMO);
    const QString  subject = qvar_subject.isValid() ? MTContactHandler::getInstance()->Decode(qvar_subject.toString()) : "";
    // --------------------------------------------------
    const QString& otherNymID = bOutgoing ? recipientNymID : senderNymID;
    const QString& otherAddress  = bOutgoing ? recipientAddr  : senderAddr;
    // --------------------------------------------------
    const bool bUsingNotary   = !MsgNotaryID.isEmpty();
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
        compose_window->setInitialServer(MsgNotaryID);
    // --------------------------------------------------
    compose_window->setInitialSubject(subject);
    // --------------------------------------------------
    if (nPaymentID > 0)
    {
        QString body        = MTContactHandler::getInstance()->GetPaymentBody(nPaymentID);
        QString pendingBody = MTContactHandler::getInstance()->GetPaymentPendingBody(nPaymentID);

        if (!body.isEmpty())
            compose_window->setInitialBody(body);
        else if (!pendingBody.isEmpty())
            compose_window->setInitialBody(pendingBody);
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
    // -----------------------------
    if (haveForwardedIndex.isValid())
        listRecordsToMarkAsForwarded_.append(haveForwardedIndex);
    if (listRecordsToMarkAsForwarded_.count() > 0)
        QTimer::singleShot(0, this, SLOT(on_MarkAsForwarded_timer()));
}



void MTAccountDetails::on_toolButtonDelete_clicked()
{
    QTableView * pTableView = ui->tableView;

    if ( (nullptr != pTableView) &&
         (nullptr != pPmntProxyModel_) )
    {
        if (!pTableView->selectionModel()->hasSelection())
            return;
        // ----------------------------------------------
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, tr(MONEYCHANGER_APP_NAME), QString("%1<br/><br/>%2").arg(tr("Are you sure you want to delete these receipts?")).
                                      arg(tr("WARNING: This is not reversible!")),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply != QMessageBox::Yes)
            return;
        // ----------------------------------------------
        QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

        if (pModel)
        {
            QItemSelection selection( pTableView->selectionModel()->selection() );

            int nFirstProxyRowRemoved = -1;
            int nLastProxyRowRemoved  = -1;
            int nCountRowsRemoved     = 0;

            QList<int> rows, payment_ids;
            foreach( const QModelIndex & index, selection.indexes() ) {
                QModelIndex sourceIndex = pPmntProxyModel_->mapToSource(index);
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
                   QModelIndex sourceIndexpmntid = pModel->index(current, PMNT_SOURCE_COL_PMNT_ID);
                   if (sourceIndexpmntid.isValid())
                       payment_ids.append(pModel->data(sourceIndexpmntid).toInt());
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
                    // Now we just deleted some receipts; let's delete also the corresponding
                    // receipt contents. (We saved the deleted IDs for this purpose.)
                    //
                    for (int ii = 0; ii < payment_ids.count(); ++ii)
                    {
                        const int nPmntID = payment_ids[ii];

                        if (nPmntID > 0)
                            if (!MTContactHandler::getInstance()->DeletePaymentBody(nPmntID))
                                qDebug() << "AccountDetails::on_toolButtonDelete_clicked: Failed trying to delete payment body with payment_id: " << nPmntID << "\n";
                    }
                    // ------------------------
                    // We just deleted the selected rows.
                    // So now we need to choose another row to select.

                    int nRowToSelect = -1;

                    if ((nFirstProxyRowRemoved >= 0) && (nFirstProxyRowRemoved < pPmntProxyModel_->rowCount()))
                        nRowToSelect = nFirstProxyRowRemoved;
                    else if (0 == nFirstProxyRowRemoved)
                        nRowToSelect = 0;
                    else if (nFirstProxyRowRemoved > 0)
                        nRowToSelect = pPmntProxyModel_->rowCount() - 1;
                    else
                        nRowToSelect = 0;

                    if ((pPmntProxyModel_->rowCount() > 0) && (nRowToSelect >= 0) &&
                            (nRowToSelect < pPmntProxyModel_->rowCount()))
                    {
                        QModelIndex previous = pTableView->currentIndex();
                        pTableView->blockSignals(true);
                        pTableView->selectRow(nRowToSelect);
                        pTableView->blockSignals(false);

                        on_tableViewSelectionModel_currentRowChanged(pTableView->currentIndex(), previous);
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

MTAccountDetails::MTAccountDetails(QWidget *parent, MTDetailEdit & theOwner) :
    MTEditDetails(parent, theOwner),
    m_qstrID(""),
    ui(new Ui::MTAccountDetails)
{
    ui->setupUi(this);
    this->setContentsMargins(0, 0, 0, 0);
//  this->installEventFilter(this); // NOTE: Successfully tested theory that the base class has already installed this.

    // ----------------------------------
    // Note: This is a placekeeper, so later on I can just erase
    // the widget at 0 and replace it with the real header widget.
    //
    m_pHeaderWidget  = new QWidget;
    ui->verticalLayout->insertWidget(0, m_pHeaderWidget);
    // ----------------------------------
//  ui->lineEditID    ->setStyleSheet("QLineEdit { background-color: lightgray }");
    // ----------------------------------
    ui->lineEditServer->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditAsset ->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditNym   ->setStyleSheet("QLineEdit { background-color: lightgray }");
    // ----------------------------------
    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

    if (pModel)
    {
        pPmntProxyModel_  = new AccountRecordsProxyModel;
        pPmntProxyModel_->setSourceModel(pModel);
        // ---------------------------------
        setup_tableview(ui->tableView, pPmntProxyModel_);
        // ---------------------------------
        QItemSelectionModel *sm1 = ui->tableView->selectionModel();
        connect(sm1, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                this, SLOT(on_tableViewSelectionModel_currentRowChanged(QModelIndex,QModelIndex)));
    }
    connect(this, SIGNAL(showContact(QString)), Moneychanger::It(), SLOT(mc_showcontact_slot(QString)));
    // --------------------------------------------------------
    connect(this, SIGNAL(showContactAndRefreshHome(QString)), Moneychanger::It(), SLOT(onNeedToPopulateRecordlist()));
    connect(this, SIGNAL(showContactAndRefreshHome(QString)), Moneychanger::It(), SLOT(mc_showcontact_slot(QString)));
    // --------------------------------------------------------
    connect(this, SIGNAL(showDashboard()), Moneychanger::It(), SLOT(mc_overview_slot()));
    // --------------------------------------------------------
    connect(this, SIGNAL(showPayment(int,int)), Moneychanger::It(), SLOT(mc_show_payment_slot(int,int)));
}

void MTAccountDetails::on_tableViewSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous)
{
    if (!current.isValid())
    {
//        disableButtons();
//        // ----------------------------------------
//        PMNT_TREE_ITEM theItem = make_tree_item(nCurrentContact_, qstrMethodType_, qstrViaTransport_);
//        set_outbox_pmntid_for_tree_item(theItem, 0);
    }
    else
    {
//        enableButtons();
        // ----------------------------------------
        QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

        if (pModel)
        {
            QModelIndex sourceIndex = pPmntProxyModel_->mapToSource(current);

            QModelIndex haveReadSourceIndex  = pPmntProxyModel_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_HAVE_READ, sourceIndex);
            QModelIndex pmntidSourceIndex    = pPmntProxyModel_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_PMNT_ID,   sourceIndex);

            QVariant varpmntid    = pModel->data(pmntidSourceIndex);
            QVariant varHaveRead  = pModel->data(haveReadSourceIndex);
            // ----------------------------------------------------------
            int payment_id = varpmntid.isValid() ? varpmntid.toInt() : 0;
//            if (payment_id > 0)
//            {
//                PMNT_TREE_ITEM theItem = make_tree_item(nCurrentContact_, qstrMethodType_, qstrViaTransport_);
//                set_outbox_pmntid_for_tree_item(theItem, payment_id);
//            }
            // ----------------------------------------------------------
            const bool bHaveRead = varHaveRead.isValid() ? varHaveRead.toBool() : false;

            if (!bHaveRead && (payment_id > 0)) // It's unread, so we need to set it as read.
            {
                listRecordsToMarkAsRead_.append(haveReadSourceIndex);
                QTimer::singleShot(1000, this, SLOT(on_MarkAsRead_timer()));
            }
        }
    }
}


void MTAccountDetails::on_MarkAsRead_timer()
{
    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

    if (!pModel)
        return;
    // ------------------------------
    bool bEditing = false;

    while (!listRecordsToMarkAsRead_.isEmpty())
    {
        QModelIndex index = listRecordsToMarkAsRead_.front();
        listRecordsToMarkAsRead_.pop_front();
        // ------------------------------------
        if (!index.isValid())
            continue;
        // ------------------------------------
        if (!bEditing)
        {
            bEditing = true;
            pModel->database().transaction();
        }
        // ------------------------------------
        pModel->setData(index, QVariant(1)); // 1 for "true" in sqlite. "Yes, we've now read this payment. Mark it as read."
    } // while
    // ------------------------------
    if (bEditing)
    {
        if (pModel->submitAll())
        {
            pModel->database().commit();
            // ------------------------------------
            QTimer::singleShot(0, this, SLOT(RefreshPayments()));
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

void MTAccountDetails::on_MarkAsUnread_timer()
{
    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

    if (!pModel)
        return;
    // ------------------------------
    bool bEditing = false;

    while (!listRecordsToMarkAsUnread_.isEmpty())
    {
        QModelIndex index = listRecordsToMarkAsUnread_.front();
        listRecordsToMarkAsUnread_.pop_front();
        // ------------------------------------
        if (!index.isValid())
            continue;
        // ------------------------------------
        if (!bEditing)
        {
            bEditing = true;
            pModel->database().transaction();
        }
        // ------------------------------------
        pModel->setData(index, QVariant(0)); // 0 for "false" in sqlite. "This payment is now marked UNREAD."
    } // while
    // ------------------------------
    if (bEditing)
    {
        if (pModel->submitAll())
        {
            pModel->database().commit();
            // ------------------------------------
            QTimer::singleShot(0, this, SLOT(RefreshPayments()));
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

void MTAccountDetails::on_MarkAsReplied_timer()
{
    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

    if (!pModel)
        return;
    // ------------------------------
    bool bEditing = false;

    while (!listRecordsToMarkAsReplied_.isEmpty())
    {
        QModelIndex index = listRecordsToMarkAsReplied_.front();
        listRecordsToMarkAsReplied_.pop_front();
        // ------------------------------------
        if (!index.isValid())
            continue;
        // ------------------------------------
        if (!bEditing)
        {
            bEditing = true;
            pModel->database().transaction();
        }
        // ------------------------------------
        pModel->setData(index, QVariant(1)); // 1 for "true" in sqlite. "Yes, we've now replied to this payment. Mark it as replied."
    } // while
    // ------------------------------
    if (bEditing)
    {
        if (pModel->submitAll())
        {
            pModel->database().commit();
            // ------------------------------------
            QTimer::singleShot(0, this, SLOT(RefreshPayments()));
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

void MTAccountDetails::on_MarkAsForwarded_timer()
{
    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

    if (!pModel)
        return;
    // ------------------------------
    bool bEditing = false;

    while (!listRecordsToMarkAsForwarded_.isEmpty())
    {
        QModelIndex index = listRecordsToMarkAsForwarded_.front();
        listRecordsToMarkAsForwarded_.pop_front();
        // ------------------------------------
        if (!index.isValid())
            continue;
        // ------------------------------------
        if (!bEditing)
        {
            bEditing = true;
            pModel->database().transaction();
        }
        // ------------------------------------
        pModel->setData(index, QVariant(1)); // 1 for "true" in sqlite. "Yes, we've now forwarded this payment. Mark it as forwarded."
    } // while
    // ------------------------------
    if (bEditing)
    {
        if (pModel->submitAll())
        {
            pModel->database().commit();
            // ------------------------------------
            QTimer::singleShot(0, this, SLOT(RefreshPayments()));
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

MTAccountDetails::~MTAccountDetails()
{
    delete ui;
}

void MTAccountDetails::ClearContents()
{
    ui->lineEditName->setText("");
    ui->lineEditServer->setText("");
    ui->lineEditAsset->setText("");
    ui->lineEditNym->setText("");
    // ------------------------------------------
    if (m_pCashPurse)
        m_pCashPurse->ClearContents();
    // ------------------------------------------
    ui->pushButtonMakeDefault->setEnabled(false);

//    ui->tableView->setVisible(false);

    m_qstrID = QString("");
}


void MTAccountDetails::on_tableView_doubleClicked(const QModelIndex &index)
{
    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

    if (!pModel)
        return;

    if (!index.isValid())
        return;

    if (!pPmntProxyModel_)
        return;

    QPointer<AccountRecordsProxyModel> & pProxyModel = pPmntProxyModel_;

    QModelIndex sourceIndex = pProxyModel->mapToSource(index);

    if (!sourceIndex.isValid())
        return;
    // -------------------------------
    QModelIndex pmntidIndex   = pModel->sibling(sourceIndex.row(), PMNT_SOURCE_COL_PMNT_ID, sourceIndex);
    QModelIndex subjectIndex = pModel->sibling(sourceIndex.row(), PMNT_SOURCE_COL_MEMO, sourceIndex);

    QVariant qvarpmntid   = pModel->data(pmntidIndex);
    QVariant qvarSubject = pModel->data(subjectIndex);

    int     nPaymentID  = qvarpmntid.isValid() ? qvarpmntid.toInt() : 0;
    QString qstrSubject = qvarSubject.isValid() ? qvarSubject.toString() : "";
    // -------------------------------
    QString qstrPayment, qstrPending, qstrType, qstrSubtitle;
    // --------------------------------------------------
    if (nPaymentID > 0)
    {
        qstrPayment = MTContactHandler::getInstance()->GetPaymentBody(nPaymentID);
        qstrPending = MTContactHandler::getInstance()->GetPaymentPendingBody(nPaymentID);
    }
    // --------------------------------------------------
    QModelIndex myNymIndex        = pModel->sibling(sourceIndex.row(), PMNT_SOURCE_COL_MY_NYM, sourceIndex);
    QModelIndex senderNymIndex    = pModel->sibling(sourceIndex.row(), PMNT_SOURCE_COL_SENDER_NYM, sourceIndex);
    QModelIndex recipientNymIndex = pModel->sibling(sourceIndex.row(), PMNT_SOURCE_COL_RECIP_NYM, sourceIndex);

    QModelIndex myNymProxyIndex        = pProxyModel->mapFromSource(myNymIndex);
    QModelIndex senderNymProxyIndex    = pProxyModel->mapFromSource(senderNymIndex);
    QModelIndex recipientNymProxyIndex = pProxyModel->mapFromSource(recipientNymIndex);

    QVariant qvarMyNymName        = myNymProxyIndex.isValid()        ? pProxyModel->data(myNymProxyIndex) : QString("");
    QVariant qvarSenderNymName    = senderNymProxyIndex.isValid()    ? pProxyModel->data(senderNymProxyIndex) : QString("");
    QVariant qvarRecipientNymName = recipientNymProxyIndex.isValid() ? pProxyModel->data(recipientNymProxyIndex) : QString("");

    QString qstrMyNymName        = qvarMyNymName.isValid() ? qvarMyNymName.toString() : "";
    QString qstrSenderNymName    = qvarSenderNymName.isValid() ? qvarSenderNymName.toString() : "";
    QString qstrRecipientNymName = qvarRecipientNymName.isValid() ? qvarRecipientNymName.toString() : "";

    if (!qstrSenderNymName.isEmpty())
    {
        qstrType = QString("%1: %2").arg(tr("To")).arg(qstrMyNymName);
        qstrSubtitle = QString("%1: %2").arg(tr("From")).arg(qstrSenderNymName);
    }
    else if (!qstrRecipientNymName.isEmpty())
    {
        qstrType = QString("%1: %2").arg(tr("To")).arg(qstrRecipientNymName);
        qstrSubtitle = QString("%1: %2").arg(tr("From")).arg(qstrMyNymName);
    }
    else
    {
        qstrType = QString("Instrument: ");
        qstrSubtitle = QString(" ");
    }
    // -----------
    // Pop up the result dialog.
    //
    if (qstrPayment.isEmpty() || qstrPending.isEmpty())
    {
        DlgExportedToPass dlgExported(this, qstrPayment.isEmpty() ? qstrPending : qstrPayment,
                                      qstrType,
                                      qstrSubtitle, false);
        dlgExported.setWindowTitle(QString("%1: %2").arg(tr("Memo")).arg(qstrSubject));
        dlgExported.exec();
    }
    else
    {
        DlgExportedCash dlgExported(this, qstrPending, qstrPayment,
                                    tr("Receipt:"), QString(" "),
                                    qstrType,
                                    qstrSubtitle, false);
        dlgExported.setWindowTitle(QString("%1: %2").arg(tr("Memo")).arg(qstrSubject));
        dlgExported.exec();
    }
}


void MTAccountDetails::FavorLeftSideForIDs()
{
    if (NULL != ui)
    {
//      ui->lineEditID  ->home(false);
        ui->lineEditName->home(false);
        // ----------------------------------
        ui->lineEditServer->home(false);
        ui->lineEditAsset ->home(false);
        ui->lineEditNym   ->home(false);
        // ----------------------------------
    }
}

// ------------------------------------------------------
//virtual
int MTAccountDetails::GetCustomTabCount()
{
    if (Moneychanger::It()->expertMode())
        return 1;

    return 0;
}
// ----------------------------------
//virtual
QWidget * MTAccountDetails::CreateCustomTab(int nTab)
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
    case 0: // "Cash Purse" tab
        if (NULL != m_pOwner)
        {
            if (m_pCashPurse)
            {
                m_pCashPurse->setParent(NULL);
                m_pCashPurse->disconnect();
                m_pCashPurse->deleteLater();

                m_pCashPurse = NULL;
            }
            m_pCashPurse = new MTCashPurse(NULL, *m_pOwner);
            pReturnValue = m_pCashPurse;
            pReturnValue->setContentsMargins(0, 0, 0, 0);

            connect(m_pCashPurse, SIGNAL(balancesChanged(QString)),
                    m_pOwner,     SLOT(onBalancesChangedFromBelow(QString)));
        }
        break;

    default:
        qDebug() << QString("Unexpected: MTAccountDetails::CreateCustomTab was called with bad index: %1").arg(nTab);
        return NULL;
    }
    // -----------------------------
    return pReturnValue;
}

// ---------------------------------
//virtual
QString  MTAccountDetails::GetCustomTabName(int nTab)
{
    const int nCustomTabCount = this->GetCustomTabCount();
    // -----------------------------
    if ((nTab < 0) || (nTab >= nCustomTabCount))
        return tr(""); // out of bounds.
    // -----------------------------
    QString qstrReturnValue("");
    // -----------------------------
    switch (nTab)
    {
    case 0:  qstrReturnValue = tr("Cash Purse");  break;

    default:
        qDebug() << QString("Unexpected: MTAccountDetails::GetCustomTabName was called with bad index: %1").arg(nTab);
        return tr("");
    }
    // -----------------------------
    return qstrReturnValue;
}
// ------------------------------------------------------


//virtual
void MTAccountDetails::refresh(QString strID, QString strName)
{
//  qDebug() << "MTAccountDetails::refresh";

    if (!strID.isEmpty() && (NULL != ui))
    {
        ui->labelTransactions->setVisible(!Moneychanger::It()->expertMode());

        m_qstrID = strID;
        // -------------------------------------
        QString qstrAmount = MTHome::shortAcctBalance(strID);

        QWidget * pHeaderWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, strID, strName, qstrAmount, "", ":/icons/icons/vault.png", false);

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
//      ui->lineEditID  ->setText(strID);
        ui->lineEditName->setText(strName);
        // ----------------------------------
        std::string str_notary_id = opentxs::OT::App().API().Exec().GetAccountWallet_NotaryID   (strID.toStdString());
        std::string str_asset_id  = opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID(strID.toStdString());
        std::string str_nym_id    = opentxs::OT::App().API().Exec().GetAccountWallet_NymID      (strID.toStdString());
        // ----------------------------------
        QString qstr_notary_id    = QString::fromStdString(str_notary_id);
        QString qstr_asset_id     = QString::fromStdString(str_asset_id);
        QString qstr_nym_id       = QString::fromStdString(str_nym_id);
        // ----------------------------------
        QString qstr_server_name  = QString::fromStdString(opentxs::OT::App().API().Exec().GetServer_Name   (str_notary_id));
        QString qstr_asset_name   = QString::fromStdString(opentxs::OT::App().API().Exec().GetAssetType_Name(str_asset_id));
        QString qstr_nym_name     = QString::fromStdString(opentxs::OT::App().API().Exec().GetNym_Name      (str_nym_id));
        // ----------------------------------
        // MAIN TAB
        //
        if (!strID.isEmpty())
        {
            ui->lineEditServer->setText(qstr_server_name);
            ui->lineEditAsset ->setText(qstr_asset_name);
            ui->lineEditNym   ->setText(qstr_nym_name);
//            ui->tableView     ->setVisible(true);
            pPmntProxyModel_  ->setFilterAccountId(strID); // Filter records by account ID.

            ui->tableView->resizeRowsToContents();
            ui->tableView->resizeColumnsToContents();
            ui->tableView->horizontalHeader()->setStretchLastSection(true);
        }
//        else
//            ui->tableView     ->setVisible(false);
        // -----------------------------------
        // TAB: "CASH PURSE"
        //
        if (m_pCashPurse)
            m_pCashPurse->refresh(strID, strName);
        // -----------------------------------------------------------------------
        FavorLeftSideForIDs();
        // -----------------------------------------------------------------------
        QString qstr_default_acct_id = Moneychanger::It()->get_default_account_id();

        if (0 == strID.compare(qstr_default_acct_id))
            ui->pushButtonMakeDefault->setEnabled(false);
        else
            ui->pushButtonMakeDefault->setEnabled(true);
    }
}

// ------------------------------------------------------

bool MTAccountDetails::eventFilter(QObject *obj, QEvent *event)
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

void MTAccountDetails::on_pushButtonSend_clicked()
{
    if (!m_qstrID.isEmpty())
        emit SendFromAcct(m_qstrID);
}

// ------------------------------------------------------

void MTAccountDetails::on_pushButtonRequest_clicked()
{
    if (!m_qstrID.isEmpty())
        emit RequestToAcct(m_qstrID);
}

// ------------------------------------------------------

void MTAccountDetails::on_pushButtonMakeDefault_clicked()
{
    if ((NULL != m_pOwner) && !m_qstrID.isEmpty())
    {
        std::string str_acct_name = opentxs::OT::App().API().Exec().GetAccountWallet_Name(m_qstrID.toStdString());
        ui->pushButtonMakeDefault->setEnabled(false);
        // --------------------------------------------------
        QString qstrAcctName = QString::fromStdString(str_acct_name);
        // --------------------------------------------------
        emit DefaultAccountChanged(m_qstrID, qstrAcctName);
    }
}

// ------------------------------------------------------

void MTAccountDetails::on_toolButtonAsset_clicked()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        std::string str_acct_id = m_pOwner->m_qstrCurrentID.toStdString();
        // -------------------------------------------------------------------
        QString qstr_id = QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID(str_acct_id));
        // --------------------------------------------------
        emit ShowAsset(qstr_id);
    }
}

void MTAccountDetails::on_toolButtonNym_clicked()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        std::string str_acct_id = m_pOwner->m_qstrCurrentID.toStdString();
        // -------------------------------------------------------------------
        QString qstr_id = QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_NymID(str_acct_id));
        // --------------------------------------------------
        emit ShowNym(qstr_id);
    }
}

void MTAccountDetails::on_toolButtonServer_clicked()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        std::string str_acct_id = m_pOwner->m_qstrCurrentID.toStdString();
        // -------------------------------------------------------------------
        QString qstr_id = QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_NotaryID(str_acct_id));
        // --------------------------------------------------
        emit ShowServer(qstr_id);
    }
}


// ------------------------------------------------------


//virtual
void MTAccountDetails::DeleteButtonClicked()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        const std::string str_account_id   = m_pOwner->m_qstrCurrentID.toStdString();
        const std::string str_owner_nym_id = opentxs::OT::App().API().Exec().GetAccountWallet_NymID   (str_account_id);
        const std::string str_notary_id    = opentxs::OT::App().API().Exec().GetAccountWallet_NotaryID(str_account_id);
        // ----------------------------------------------------
        // Download all the intermediary files (account balance, inbox, outbox, etc)
        // to make sure we're looking at the latest inbox.
        //
        const auto theNotaryID = opentxs::Identifier::Factory(str_owner_nym_id),
                      theNymID = opentxs::Identifier::Factory(str_owner_nym_id),
                     theAcctID = opentxs::Identifier::Factory(str_account_id);
        bool bRetrieved = false;
        {
            MTSpinner theSpinner;

            bRetrieved = opentxs::OT::App().API().ServerAction().DownloadAccount(
            		theNymID, theNotaryID, theAcctID, true);
        }
        qDebug() << QString("%1 retrieving intermediary files for account %2. (Precursor to delete account.)").
                    arg(bRetrieved ? QString("Success") : QString("Failed")).arg(str_account_id.c_str());
        // -------------
        if (!bRetrieved)
        {
            Moneychanger::It()->HasUsageCredits(str_notary_id, str_owner_nym_id);
            return;
        }
        // ---------------------------------------------------------
        bool bCanRemove = opentxs::OT::App().API().Exec().Wallet_CanRemoveAccount(m_pOwner->m_qstrCurrentID.toStdString());

        if (!bCanRemove)
        {
            const auto id_acct = opentxs::Identifier::Factory(str_account_id);
            const auto account = opentxs::OT::App().Wallet().Account(id_acct);

            QString qstrMessage = QString("%1. %2")
                    .arg(tr("This Account cannot be deleted until it has a zero balance and an empty inbox"))
                    .arg((account && account.get().IsIssuer())
                         ? QString(tr("Also, issuer accounts cannot simply be deleted. The unit type itself must be un-issued first."))
                         : QString(""));
            return;
        }
        // ----------------------------------------------------
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, "", tr("Are you sure you want to delete this Account?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            int32_t nSuccess = 0;
            bool    bDeleted = false;
            {
                MTSpinner theSpinner;

                auto action = opentxs::OT::App().API().ServerAction().UnregisterAccount(theNymID, theNotaryID, theAcctID);
                std::string strResponse = action->Run();
                nSuccess                = opentxs::VerifyMessageSuccess(strResponse);
            }
            // -1 is error,
            //  0 is reply received: failure
            //  1 is reply received: success
            //
            switch (nSuccess)
            {
            case (1):
                {
                    bDeleted = true;
                    break; // SUCCESS
                }
            case (0):
                {
                    QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                        tr("Failed while trying to unregister account from Server."));
                    break;
                }
            default:
                {
                    QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                        tr("Error while trying to unregister account from Server."));
                    break;
                }
            } // switch
            // --------------------------
            if (1 != nSuccess)
            {
                Moneychanger::It()->HasUsageCredits(QString::fromStdString(str_notary_id), QString::fromStdString(str_owner_nym_id));
                return;
            }
            // --------------------------
            if (bDeleted)
            {
                // NOTE: OTClient.cpp already does this, as soon as it receives a success reply
                // from the above server message to unregisterAccount. So we don't have to do this
                // here, since it's already done by the time we reach this point.
                //
//              bool bSuccess = opentxs::OT::App().API().Exec().Wallet_RemoveAccount(m_pOwner->m_qstrCurrentID.toStdString());
                // ------------------------------------------------
                m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
                // ------------------------------------------------
                emit accountsChanged();
                // ------------------------------------------------
            }
        }
    }
}


// ------------------------------------------------------

//virtual
void MTAccountDetails::AddButtonClicked()
{
    MTWizardAddAccount theWizard(this);

    theWizard.setWindowTitle(tr("Create Account"));

    if (QDialog::Accepted == theWizard.exec())
    {
        QString qstrName     = theWizard.field("Name") .toString();
        QString qstrInstrumentDefinitionID  = theWizard.field("InstrumentDefinitionID") .toString();
        QString qstrNymID    = theWizard.field("NymID")   .toString();
        QString qstrNotaryID = theWizard.field("NotaryID").toString();
        // ---------------------------------------------------
        QString qstrAssetName  = QString::fromStdString(opentxs::OT::App().API().Exec().GetAssetType_Name(qstrInstrumentDefinitionID .toStdString()));
        QString qstrNymName    = QString::fromStdString(opentxs::OT::App().API().Exec().GetNym_Name      (qstrNymID   .toStdString()));
        QString qstrServerName = QString::fromStdString(opentxs::OT::App().API().Exec().GetServer_Name   (qstrNotaryID.toStdString()));
        // ---------------------------------------------------
        QMessageBox::information(this, tr("Confirm Create Account"),
                                 QString("%1: '%2'<br/>%3: %4<br/>%5: %6<br/>%7: %8").arg(tr("Confirm Create Account:<br/>Name")).
                                 arg(qstrName).arg(tr("Asset")).arg(qstrAssetName).arg(tr("Nym")).arg(qstrNymName).arg(tr("Server")).arg(qstrServerName));
        // ---------------------------------------------------
        // NOTE: theWizard won't allow each page to finish unless the ID is provided.
        // (Therefore we don't have to check here to see if any of the IDs are empty.)

        // ------------------------------
        // First make sure the Nym is registered at the server, and if not, register him.
        //
        bool bIsRegiseredAtServer = opentxs::OT::App().API().Exec().IsNym_RegisteredAtServer(qstrNymID.toStdString(),
                                                                         qstrNotaryID.toStdString());
        if (!bIsRegiseredAtServer)
        {
            // If the Nym's not registered at the server, then register him first.
            //
            int32_t nSuccess = 0;
            {
                MTSpinner theSpinner;

                auto strResponse = opentxs::OT::App().API().Sync().RegisterNym(opentxs::Identifier::Factory(qstrNymID.toStdString()),
                                                                               opentxs::Identifier::Factory(qstrNotaryID.toStdString()), true);

                if (false == strResponse->empty()) {
                    nSuccess = 1;
                } else {
                    nSuccess = 0;
                }
            }
            // -1 is error,
            //  0 is reply received: failure
            //  1 is reply received: success
            //
            switch (nSuccess)
            {
            case (1):
                {
                    bIsRegiseredAtServer = true;
                    MTContactHandler::getInstance()->NotifyOfNymServerPair(qstrNymID, qstrNotaryID);
                    break; // SUCCESS
                }
            case (0):
                {
                    QMessageBox::warning(this, tr("Failed Registration"),
                        tr("Failed while trying to register Nym at Server."));
                    break;
                }
            default:
                {
                    QMessageBox::warning(this, tr("Error in Registration"),
                        tr("Error while trying to register Nym at Server."));
                    break;
                }
            } // switch
            // --------------------------
            if (1 != nSuccess)
            {
                Moneychanger::It()->HasUsageCredits(qstrNotaryID, qstrNymID);
                return;
            }
        }
        // --------------------------
        // Send the request.
        // (Create Account here...)
        //
        // Send the 'create_asset_acct' message to the server.
        //
        std::string strResponse;
        {
            MTSpinner theSpinner;

            std::string nymID = qstrNymID.toStdString();
            std::string notaryID = qstrNotaryID.toStdString();
            std::string instrumentDefinitionID = qstrInstrumentDefinitionID.toStdString();
            auto action = opentxs::OT::App().API().ServerAction().RegisterAccount(
                    opentxs::Identifier::Factory(nymID), opentxs::Identifier::Factory(notaryID), opentxs::Identifier::Factory(instrumentDefinitionID));
            strResponse = action->Run();
        }
        // -1 error, 0 failure, 1 success.
        //
        if (1 != opentxs::VerifyMessageSuccess(strResponse))
        {
            const int64_t lUsageCredits = Moneychanger::It()->HasUsageCredits(qstrNotaryID, qstrNymID);

            // HasUsageCredits already pops up an error box in the cases of -2 and 0.
            //
            if (((-2) != lUsageCredits) && (0 != lUsageCredits))
                QMessageBox::warning(this, tr("Failed Creating Account"),
                    tr("Failed trying to create Account at Server."));
            return;
        }
        // ------------------------------------------------------
        // Get the ID of the new account.
        //
        QString qstrID = QString::fromStdString(opentxs::OT::App().API().Exec().Message_GetNewAcctID(strResponse));

        if (qstrID.isEmpty())
        {
            QMessageBox::warning(this, tr("Failed Getting new Account ID"),
                                 tr("Failed trying to get the new account's ID from the server response."));
            return;
        }
        // ------------------------------------------------------
        // Set the Name of the new account.
        //
        //bool bNameSet =
                opentxs::OT::App().API().Exec().SetAccountWallet_Name(qstrID   .toStdString(),
                                                  qstrNymID.toStdString(),
                                                  qstrName .toStdString());
        // -----------------------------------------------
        // Commenting out for now.
        //
//        QMessageBox::information(this, tr("Success!"), QString("%1: '%2'<br/>%3: %4").arg(tr("Success Creating Account!<br/>Name")).
//                                 arg(qstrName).arg(tr("ID")).arg(qstrID));
        // ----------
        m_pOwner->m_map.insert(qstrID, qstrName);
        m_pOwner->SetPreSelected(qstrID);
        // ------------------------------------------------
        emit newAccountAdded(qstrID);
        // ------------------------------------------------
    }
}


// ------------------------------------------------------

void MTAccountDetails::on_lineEditName_editingFinished()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        std::string str_acct_id = m_pOwner->m_qstrCurrentID.toStdString();
        std::string str_nym_id  = opentxs::OT::App().API().Exec().GetAccountWallet_NymID(str_acct_id);

        if (!str_acct_id.empty() && !str_nym_id.empty())
        {
            bool bSuccess = opentxs::OT::App().API().Exec().SetAccountWallet_Name(str_acct_id,  // Account
                                                              str_nym_id,   // Nym (Account Owner.)
                                                              ui->lineEditName->text().toStdString()); // New Name
            if (bSuccess)
            {
                m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
                m_pOwner->m_map.insert(m_pOwner->m_qstrCurrentID, ui->lineEditName->text());

                m_pOwner->SetPreSelected(m_pOwner->m_qstrCurrentID);
                // ------------------------------------------------
                emit accountsChanged();
                // ------------------------------------------------
            }
        }
    }
}


