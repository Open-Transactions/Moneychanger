
#include <QObject>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QGridLayout>
#include <QToolButton>
#include <QIcon>
#include <QPixmap>
#include <QDateTime>
#include <QLabel>
#include <QString>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>
#include <opentxs/OTLog.h>

#include "homedetail.h"
#include "ui_homedetail.h"

#include "compose.h"
#include "overridecursor.h"

#include "home.h"

#include "UI/getstringdialog.h"
#include "Widgets/dlgchooser.h"

#include "Handlers/contacthandler.h"

#include "moneychanger.h"


void MTHomeDetail::SetHomePointer(MTHome & theHome)
{
    m_pHome = &theHome;
}



MTHomeDetail::MTHomeDetail(QWidget *parent) :
    QWidget(parent),
    m_nContactID(0),
    m_pDetailLayout(NULL),
    m_pHome(NULL),
    ui(new Ui::MTHomeDetail)
{
    ui->setupUi(this);

    this->installEventFilter(this);

    this->setContentsMargins(0, 0, 0, 0);
}

MTHomeDetail::~MTHomeDetail()
{
    // --------------------------------------------------
    if (NULL != m_pDetailLayout)
    {
//      this->clearLayout(m_pDetailLayout);

        // The layout will already be cleared when it gets deleted.
        // Therefore we only use clearLayout when replacing it with
        // a new layout -- we don't have to clear it on destruction.
        // (It already clears itself in that case.)

        delete m_pDetailLayout;
        m_pDetailLayout = NULL;
    }
    // --------------------------------------------------
    delete ui;
}

bool MTHomeDetail::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape){
            close(); // This is caught by this same filter.
            return true;
        }
        return true;
    }else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}


void MTHomeDetail::on_viewContactButton_clicked(bool checked /*=false*/)
{
    qDebug() << "View Existing Contact button clicked.";

    if (m_record && (NULL != m_pHome) && (m_nContactID > 0))
    {
//      MTRecord & recordmt = *m_record;
        ((Moneychanger *)(m_pHome->parentWidget()))->mc_addressbook_show(QString("%1").arg(m_nContactID));
    }
}

void MTHomeDetail::on_addContactButton_clicked(bool checked /*=false*/)
{
    qDebug() << "Add New Contact button clicked.";

    if (m_record)
    {
        MTRecord & recordmt = *m_record;
        // --------------------------------------------------
        MTGetStringDialog nameDlg(this);

        if (QDialog::Accepted == nameDlg.exec())
        {
            QString strNewContactName = nameDlg.GetOutputString();
            // --------------------------------------------------
            const std::string str_acct_id    = recordmt.GetOtherAccountID();
            const std::string str_nym_id     = recordmt.GetOtherNymID();
            const std::string str_server_id  = recordmt.GetServerID();
            const std::string str_asset_id   = recordmt.GetAssetID();
            // --------------------------------------------------
            int nContactID = 0;

            if (!str_nym_id.empty())
            {
                QString nymID     = QString::fromStdString(str_nym_id);
                QString serverID  = QString::fromStdString(str_server_id);
                QString assetID   = QString::fromStdString(str_asset_id);
                QString accountID = QString::fromStdString(str_acct_id);
                // --------------------------------------------------
                nContactID = MTContactHandler::getInstance()->CreateContactBasedOnNym(nymID, serverID);
                // --------------------------------------------------
                if (!str_acct_id.empty())
                {
                    int nAcctContactID = MTContactHandler::getInstance()->FindContactIDByAcctID(accountID, nymID, serverID, assetID);

                    if (!(nContactID > 0))
                        nContactID = nAcctContactID;
                }
                else if (!str_server_id.empty())
                    MTContactHandler::getInstance()->NotifyOfNymServerPair(nymID, serverID);
                // --------------------------------------------------
                if (nContactID > 0)
                {
                    MTContactHandler::getInstance()->SetContactName(nContactID, strNewContactName);
                    // ---------------------------------
                    m_nContactID = nContactID;
                    // ---------------------------------
                    // Refresh the detail page.
                    //
                    refresh(recordmt);
                    // ---------------------------------
                    // Display the normal contacts dialog, with the new contact
                    // being the one selected.
                    //
                    if (NULL != m_pHome)
                    {
                        m_pHome->SetNeedRefresh();

                        ((Moneychanger *)(m_pHome->parentWidget()))->mc_addressbook_show(QString("%1").arg(m_nContactID));
                    }
                }
            }
            else
                qDebug() << "Warning: Failed adding a contact, since there was no NymID for this record.";
        } // accepted. ("OK" clicked on dialog.)
        // --------------------------------------------------
    }
}

void MTHomeDetail::on_existingContactButton_clicked(bool checked /*=false*/)
{
    qDebug() << "Add to Existing Contact button clicked.";

    if (m_record)
    {
        MTRecord & recordmt = *m_record;

        const std::string str_acct_id    = recordmt.GetOtherAccountID();
        const std::string str_nym_id     = recordmt.GetOtherNymID();
        const std::string str_server_id  = recordmt.GetServerID();
        const std::string str_asset_id   = recordmt.GetAssetID();
        // --------------------------------------------------
        if (str_nym_id.empty())
        {
            QMessageBox::warning(this, QString("Failure"), QString("Sorry, but this record has no NymID."));
            return;
        }
        // else...
        //
        QString nymID     = QString::fromStdString(str_nym_id);
        QString serverID  = QString::fromStdString(str_server_id);
        QString assetID   = QString::fromStdString(str_asset_id);
        QString accountID = QString::fromStdString(str_acct_id);

        if (MTContactHandler::getInstance()->ContactExists(nymID.toInt()))
        {
            QMessageBox::warning(this, QString("Strange"),
                                 QString("Strange: NymID %1 already belongs to an existing contact.").arg(nymID));
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
        theChooser.setWindowTitle("Choose an Existing Contact");
        // -----------------------------------------------
        if (theChooser.exec() == QDialog::Accepted)
        {
            QString strContactID = theChooser.GetCurrentID();

            qDebug() << QString("SELECT was clicked for ID: %1").arg(strContactID);

            int nContactID = strContactID.isEmpty() ? 0 : strContactID.toInt();

            if (nContactID > 0)
            {
                bool bAdded = MTContactHandler::getInstance()->AddNymToExistingContact(nContactID, nymID);

                if (!bAdded)
                {
                    QString strContactName(MTContactHandler::getInstance()->GetContactName(nContactID));
                    QMessageBox::warning(this, QString("Failure"), QString("Failed while trying to add NymID %1 to existing contact '%2' with contact ID: %3").
                                         arg(nymID).arg(strContactName).arg(nContactID));
                    return;
                }
                // --------------------------------------
                // else...
                //
                if (!str_acct_id.empty())
                    //int nAcctContactID =
                    MTContactHandler::getInstance()->FindContactIDByAcctID(accountID, nymID, serverID, assetID);
                else if (!str_server_id.empty())
                    MTContactHandler::getInstance()->NotifyOfNymServerPair(nymID, serverID);
                // --------------------------------------------------
                m_nContactID = nContactID;
                // ---------------------------------
                // Refresh the detail page.
                //
                refresh(recordmt);
                // ---------------------------------
                // Display the normal contacts dialog, with the new contact
                // being the one selected.
                //
                if (NULL != m_pHome)
                {
                    m_pHome->SetNeedRefresh();

                    ((Moneychanger *)(m_pHome->parentWidget()))->mc_addressbook_show(QString("%1").arg(m_nContactID));
                }
                // ---------------------------------
            } // nContactID > 0
        }
        else
        {
          qDebug() << "CANCEL was clicked";
        }
    }
}


void MTHomeDetail::on_deleteButton_clicked(bool checked /*=false*/)
{
    qDebug() << "Delete button clicked.";

    if (m_record)
    {
        MTRecord & recordmt = *m_record;
        // -----------------------------------
        bool bSuccess = recordmt.DeleteRecord();

        if (bSuccess)
        {
            if (NULL != m_pHome)
                m_pHome->OnDeletedRecord();
            else
                qDebug() << QString("Error: m_pHome was NULL.");
        }
    }
}
















void MTHomeDetail::on_acceptButton_clicked(bool checked /*=false*/)
{
    qDebug() << "Accept button clicked.";

    if (m_record)
    {
        MTRecord & recordmt = *m_record;
        // -------------------------------------------------
        const bool bIsTransfer = (recordmt.GetRecordType() == MTRecord::Transfer);
        const bool bIsReceipt  = (recordmt.GetRecordType() == MTRecord::Receipt);
        // -------------------------------------------------
        if (bIsTransfer)
        {
            bool bSuccess = false;
            {
                MTOverrideCursor theSpinner;

                bSuccess = recordmt.AcceptIncomingTransfer();
            }

            if (!bSuccess)
            {
                QMessageBox::warning(this, QString("Transaction failure"), QString("Failed accepting this transfer."));
            }
            else
            {
                // todo: refresh the main list, or at least change the color of the refresh button.

                if (NULL != m_pHome)
                    m_pHome->SetNeedRefresh();
            }
        }
        // -------------------------------------------------

    }

    /*
    [actions addObject:[SectionAction actionWithName:nameString icon:nil block:^(UIViewController* vc)
    {
        bool (^actionBlock)() = ^bool{return false;};

        NSString *msg = QString("Transaction Failed.");
        UIAlertView *fail = [[UIAlertView alloc] initWithTitle:nil message:msg delegate:nil
                                             cancelButtonTitle:QString("OK") otherButtonTitles:nil];

        if (bIsTransfer)
        {
            actionBlock = ^bool {
                bool bSuccess = record->AcceptIncomingTransfer();

                if (bSuccess)
                {
                    self.navigationController.navigationItem.rightBarButtonItem.tintColor = [UIColor redColor];
                    [self.navigationController popViewControllerAnimated:YES];
                }
                return bSuccess;
            };
            [LoadingView fallibleReloadAction:actionBlock inView:self.view withText:actionString withFailureAlert:fail];
        }
        else if (bIsReceipt)
        {
            actionBlock = ^bool {
                bool bSuccess = record->AcceptIncomingReceipt();

                if (bSuccess)
                {
                    self.navigationController.navigationItem.rightBarButtonItem.tintColor = [UIColor redColor];
                    [self.navigationController popViewControllerAnimated:YES];
                }
                return bSuccess;
            };
            [LoadingView fallibleReloadAction:actionBlock inView:self.view withText:actionString withFailureAlert:fail];
        }
        else if (record->GetRecordType() == MTRecord::Instrument) {  // TODO: filter these accounts by serverID and asset type ID.
            ContractPickerViewController *picker = [ContractPickerViewController pickerWithTitle:QString("Account")
                                                                                      dataSource:[AccountContractList instance]];
            picker.cancelButtonEnabled = YES;
            picker.callback = ^(ContractPickerViewController* vc, id<ContractWrapper> contract) {

                bool (^actionBlock)() = ^bool{return false;};

                if (contract) {
                    actionBlock = ^bool {
                        bool bSuccess = record->AcceptIncomingInstrument(contract.contractId.UTF8String);

                        if (bSuccess)
                        {
                            self.navigationController.navigationItem.rightBarButtonItem.tintColor = [UIColor redColor];
                            [self.navigationController popViewControllerAnimated:YES];
                        }
                        return bSuccess;
                    };
                    [LoadingView fallibleReloadAction:actionBlock inView:self.view withText:actionString withFailureAlert:fail];
                }
                [vc dismissModalViewControllerAnimated:YES];
            };
            [vc presentModalViewController:picker animated:YES];
        }
        //TODO do a refresh vs a pop

    }]];
    */

}

void MTHomeDetail::on_cancelButton_clicked(bool checked /*=false*/)
{
    qDebug() << "Cancel button clicked.";


    if (m_record)
    {
        MTRecord & recordmt = *m_record;

    }


    /*
    [actions addObject:[SectionAction actionWithName:cancelString icon:nil block:^(UIViewController* vc) {
        bool (^actionBlock)() = ^bool{return false;};

        actionBlock = ^bool {

            if (record->IsCash())
            {
                ContractPickerViewController *picker = [ContractPickerViewController pickerWithTitle:QString("Account") // TODO: filter these accounts by serverID and asset type ID.
                                                                                          dataSource:[AccountContractList instance]];
                picker.cancelButtonEnabled = YES;
                picker.callback = ^(ContractPickerViewController* vc, id<ContractWrapper> contract) {

                    bool (^actionBlock)() = ^bool{return false;};

                    if (contract) {
                        actionBlock = ^bool {
                            OT_ME madeEasy;
                            bool bInnerSuccess = false;
                            if (1 == madeEasy.deposit_cash(record->GetServerID(), record->GetNymID(), contract.contractId.UTF8String, record->GetContents()))
                            {
                                bInnerSuccess = true;
                                record->DiscardOutgoingCash();
                                self.navigationController.navigationItem.rightBarButtonItem.tintColor = [UIColor redColor];
                                [self.navigationController popViewControllerAnimated:YES];
                            }
                            return bInnerSuccess;
                        };
                        [LoadingView fallibleReloadAction:actionBlock inView:self.view withText:actionString withFailureAlert:fail];
                    }
                    [vc dismissModalViewControllerAnimated:YES];
                };
                [vc presentModalViewController:picker animated:YES];
            }
            else
            {
                bool bSuccess = record->CancelOutgoing(record->GetAccountID());

                if (bSuccess)
                {
                    self.navigationController.navigationItem.rightBarButtonItem.tintColor = [UIColor redColor];
                    [self.navigationController popViewControllerAnimated:YES];
                }
                return bSuccess;
            }

            return true;
        };

        [LoadingView fallibleReloadAction:actionBlock inView:self.view withText:actionString withFailureAlert:fail];

    }]];
    */

}

void MTHomeDetail::on_discardOutgoingButton_clicked(bool checked /*=false*/)
{
    qDebug() << "Discard Outgoing button clicked.";

    if (m_record)
    {
        MTRecord & recordmt = *m_record;

    }
    /*
    [actions addObject:[SectionAction actionWithName:discardString icon:nil block:^(UIViewController* vc) {
        record->DiscardOutgoingCash();

        self.navigationController.navigationItem.rightBarButtonItem.tintColor = [UIColor redColor];
        [self.navigationController popViewControllerAnimated:YES];
    }]];
    */

}

void MTHomeDetail::on_discardIncomingButton_clicked(bool checked /*=false*/)
{
    qDebug() << "Discard Incoming button clicked.";

    if (m_record)
    {
        MTRecord & recordmt = *m_record;

    }

    /*
    [actions addObject:[SectionAction actionWithName:discardString icon:nil block:^(UIViewController* vc) {
        record->DiscardIncoming();

        self.navigationController.navigationItem.rightBarButtonItem.tintColor = [UIColor redColor];
        [self.navigationController popViewControllerAnimated:YES];
    }]];
    */

}

void MTHomeDetail::on_msgButton_clicked(bool checked /*=false*/)
{
    if (m_record)
    {
        MTRecord & recordmt = *m_record;
        // --------------------------------------------------
        const std::string str_my_nym_id    = recordmt.GetNymID();
        const std::string str_other_nym_id = recordmt.GetOtherNymID();
        const std::string str_server_id    = recordmt.GetServerID();
        // --------------------------------------------------
        QString myNymID    = QString::fromStdString(str_my_nym_id);
        QString otherNymID = QString::fromStdString(str_other_nym_id);
        QString serverID   = QString::fromStdString(str_server_id);
        // --------------------------------------------------
        MTCompose * compose_window = new MTCompose;
        compose_window->setAttribute(Qt::WA_DeleteOnClose);
        // --------------------------------------------------
        compose_window->setInitialSenderNym   (myNymID);
        compose_window->setInitialRecipientNym(otherNymID);
        compose_window->setInitialServer      (serverID);
        // ---------------------------------------
        // Set subject, if one is available.
        std::string str_desc;

        if (recordmt.IsMail())
            recordmt.FormatShortMailDescription(str_desc);
        else
            recordmt.FormatDescription(str_desc);
        // ---------------------------------------
        compose_window->setInitialSubject(QString::fromStdString(str_desc));
        // --------------------------------------------------
        compose_window->dialog();
        compose_window->show();
        // --------------------------------------------------
    }

    /*
        // Display the SendMailViewController here.
        //
        SendMailViewController *send_mail_control = [[SendMailViewController alloc] initWithNibName:nil bundle:nil];

        // Pre-fill the recipient here.
//            send_mail_control.record = record;
        send_mail_control.nymID = QString(record->GetOtherNymID().c_str());


        // NOTE: You will need to lookup the CONTACT for the appropriate recipient,
        // since the sendMail page uses a Contact for a recipient.
        // (And what if there IS no contact for that Nym??)

        // Use this call to get the Nym Name:
        //std::string MTNameLookupIPhone::GetNymName(const std::string & str_id) const

        // But what if I don't need the Nym Name? What if I need the actual Contact?

        // Anyway, once you are able to lookup the contact for the appropriate recipient,
        // that means you should also be able to lookup the contact when displaying any
        // transaction detail, so you can remove the "Add as contact" button IN CASES
        // WHERE THE CONTACT ALREADY EXISTS.

        [self.navigationController pushViewController:send_mail_control animated:YES];
    }]];
    */

}


//static
void MTHomeDetail::clearLayout(QLayout* pLayout)
{
    if ( NULL == pLayout)
        return;
    // -----------------------------------------------
    QLayoutItem * pItemAt = NULL;

    while ( ( pItemAt = pLayout->takeAt( 0 ) ) != NULL )
    {
        if (QWidget * childWidget = pItemAt->widget())
            delete childWidget;
        else if (QLayout* childLayout = pItemAt->layout())
            clearLayout(childLayout);
        delete pItemAt;
    }
}




//static
QWidget * MTHomeDetail::CreateDetailHeaderWidget(MTRecord & recordmt, bool bExternal/*=true*/)
{
    TransactionTableViewCellType cellType = (recordmt.IsOutgoing() ?
                                                 // -------------------------------------------------
                                                 (recordmt.IsPending() ?
                                                      TransactionTableViewCellTypeOutgoing :  // outgoing
                                                      TransactionTableViewCellTypeSent) :     // sent
                                                 // -------------------------------------------------
                                                 (recordmt.IsPending() ?
                                                      TransactionTableViewCellTypeIncoming :  // incoming
                                                      TransactionTableViewCellTypeReceived)); // received
    // --------------------------------------------------------------------------------------------
    // For invoices and invoice receipts.
    //
    if (recordmt.IsInvoice() || recordmt.IsPaymentPlan() ||
        ((0 == recordmt.GetInstrumentType().compare("chequeReceipt")) &&
         (( recordmt.IsOutgoing() && (OTAPI_Wrap::It()->StringToLong(recordmt.GetAmount()) > 0)) ||
          (!recordmt.IsOutgoing() && (OTAPI_Wrap::It()->StringToLong(recordmt.GetAmount()) < 0)))
         ))
        cellType = (recordmt.IsOutgoing() ?
                    (recordmt.IsPending() ?
                     TransactionTableViewCellTypeIncoming  : // outgoing
                     TransactionTableViewCellTypeReceived) : // sent
                    // -------------------------------------------------
                    (recordmt.IsPending() ?
                     TransactionTableViewCellTypeOutgoing  : // incoming
                     TransactionTableViewCellTypeSent));     // received
    // --------------------------------------------------------------------------------------------
    std::string str_desc;
    // ---------------------------------------
    if (recordmt.IsMail())
        recordmt.FormatShortMailDescription(str_desc);
    else
        recordmt.FormatDescription(str_desc);
    // ---------------------------------------
    QString strColor("black");

    switch (cellType)
    {
        case TransactionTableViewCellTypeReceived:
            strColor = QString("green");
            break;
        case TransactionTableViewCellTypeSent:
            strColor = QString("red");
            break;
        case TransactionTableViewCellTypeIncoming:
            strColor = QString("LightGreen");
            break;
        case TransactionTableViewCellTypeOutgoing:
            strColor = QString("Crimson");
            break;
        default:
            qDebug() << "CELL TYPE: " << cellType;
            assert("Expected all cell types to be handled for color.");
            break;
    }
    // --------------------------------------------------------------------------------------------
    //Append to transactions list in overview dialog.
    QWidget * row_widget = new QWidget;
    QGridLayout * row_widget_layout = new QGridLayout;

    row_widget_layout->setSpacing(4);
    row_widget_layout->setContentsMargins(10, 4, 10, 4);

    row_widget->setLayout(row_widget_layout);
    row_widget->setStyleSheet("QWidget{background-color:#c0cad4;selection-background-color:#a0aac4;}");
    // -------------------------------------------
    //Render row.
    //Header of row
    QString tx_name = QString(QString::fromStdString(recordmt.GetName()));

    if(tx_name.trimmed() == "")
    {
        //Tx has no name
        tx_name.clear();
        tx_name = "Transaction";
    }

    QLabel * header_of_row = new QLabel;
    QString header_of_row_string = QString("");
    header_of_row_string.append(tx_name);

    header_of_row->setText(header_of_row_string);

    //Append header to layout
    row_widget_layout->addWidget(header_of_row, 0, 0, 1,1, Qt::AlignLeft);
    // -------------------------------------------
    // Amount (with currency tla)
    QLabel * currency_amount_label = new QLabel;
    QString currency_amount;

    currency_amount_label->setStyleSheet(QString("QLabel { color : %1; }").arg(strColor));
    // ----------------------------------------------------------------
    bool bLabelAdded = false;

    std::string str_formatted;
    bool bFormatted = false;

    if (!recordmt.IsMail())
        bFormatted = recordmt.FormatAmount(str_formatted);
    // ----------------------------------------
    if (bFormatted && !str_formatted.empty())
        currency_amount = QString::fromStdString(str_formatted);
    else if (recordmt.IsMail())
    {
        if (recordmt.IsOutgoing())
            currency_amount = QString("sent msg");
        else
            currency_amount = QString("message");
        // ------------------------------------------
        if (!bExternal)
        {
//            QToolButton *buttonLock  = new QToolButton;
            // ----------------------------------------------------------------
            QPixmap pixmapLock    (":/icons/icons/lock.png");
            // ----------------------------------------------------------------
            QLabel * pLockLabel = new QLabel;
            pLockLabel->setPixmap(pixmapLock);

            QHBoxLayout * pLabelLayout = new QHBoxLayout;

            pLabelLayout->addWidget(pLockLabel);
            pLabelLayout->addWidget(currency_amount_label);

            row_widget_layout->addLayout(pLabelLayout, 0, 1, 1,1, Qt::AlignRight);

            bLabelAdded = true;
        }
    }
    else
        currency_amount = QString("");
    // ----------------------------------------------------------------
    currency_amount_label->setText(currency_amount);
    // ----------------------------------------------------------------
    if (!bLabelAdded)
        row_widget_layout->addWidget(currency_amount_label, 0, 1, 1,1, Qt::AlignRight);
    // -------------------------------------------
    //Sub-info
    QWidget * row_content_container = new QWidget;
    QGridLayout * row_content_grid = new QGridLayout;

    // left top right bottom

    row_content_grid->setSpacing(4);
    row_content_grid->setContentsMargins(3, 4, 3, 4);

    row_content_container->setLayout(row_content_grid);

    row_widget_layout->addWidget(row_content_container, 1,0, 1,2);
    // -------------------------------------------
    // Column one
    //Date (sub-info)
    //Calc/convert date/times
    QDateTime timestamp;

    long lDate = OTAPI_Wrap::StringToLong(recordmt.GetDate());

    timestamp.setTime_t(lDate);

    QLabel * row_content_date_label = new QLabel;
    QString row_content_date_label_string;
    row_content_date_label_string.append(QString(timestamp.toString(Qt::SystemLocaleShortDate)));

    row_content_date_label->setStyleSheet("QLabel { color : grey; font-size:11pt;}");
    row_content_date_label->setText(row_content_date_label_string);

    row_content_grid->addWidget(row_content_date_label, 0,0, 1,1, Qt::AlignLeft);
    // -------------------------------------------
    // Column two
    //Status
    QLabel * row_content_status_label = new QLabel;
    QString row_content_status_string;

    row_content_status_string.append(QString::fromStdString(str_desc));
    // -------------------------------------------
    //add string to label
    row_content_status_label->setStyleSheet("QLabel { color : grey; font-size:11pt;}");
    row_content_status_label->setWordWrap(false);
    row_content_status_string.replace("\r\n"," ");
    row_content_status_string.replace("\n\r"," ");
    row_content_status_string.replace("\n",  " ");
    row_content_status_label->setText(row_content_status_string);

    //add to row_content grid
    row_content_grid->addWidget(row_content_status_label, 0,1, 1,1, Qt::AlignRight);
    // -------------------------------------------
    return row_widget;
}


void SetHeight (QPlainTextEdit* edit, int nRows)
{
  QFontMetrics m (edit -> font()) ;
  int RowHeight = m.lineSpacing() ;
  edit -> setFixedHeight  (nRows * RowHeight) ;
}


void increment_cell(int & nCurrentRow, int & nCurrentColumn)
{
    if (nCurrentColumn == 1)
    {
        nCurrentColumn++;
        return;
    }
    // ------------------------
    else
    {
        nCurrentColumn--;
        nCurrentRow++;
    }
}






void MTHomeDetail::refresh(int nRow, MTRecordList & theList)
{
//    qDebug() << QString("MTHomeDetail::refresh: nRow: %1").arg(nRow);

    if ((nRow >= 0) && (nRow < theList.size()))
    {
        weak_ptr_MTRecord   weakRecord = theList.GetRecord(nRow);
        shared_ptr_MTRecord record     = weakRecord.lock();

        if (weakRecord.expired())
        {
            this->setLayout(m_pDetailLayout);
            return;
        }
        // --------------------------------------------------
        m_record = record;
        MTRecord & recordmt = *record;
        // --------------------------------------------------
        refresh(recordmt);
    }
//    else
//        qDebug() << QString("MTHomeDetail::refresh: nRow %1 is out of bounds. (Max size is %2.)").arg(nRow).arg(theList.size());
}















void MTHomeDetail::refresh(MTRecord & recordmt)
{
    m_nContactID = 0;
    // --------------------------------------------------
    if (NULL != m_pDetailLayout)
    {
        MTHomeDetail::clearLayout(m_pDetailLayout);
        delete m_pDetailLayout;
        m_pDetailLayout = NULL;
    }
    // --------------------------------------------------
    m_pDetailLayout = new QGridLayout;
    m_pDetailLayout->setAlignment(Qt::AlignTop);
    m_pDetailLayout->setContentsMargins(0, 0, 0, 0);
    // --------------------------------------------------
    int nCurrentRow = 0;
    int nCurrentColumn = 1;
    // --------------------------------------------------
    // Add the header widget for this detail.
    //
    QWidget * pHeader = MTHomeDetail::CreateDetailHeaderWidget(recordmt, false);

    if (NULL != pHeader)
    {            
        pHeader->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        m_pDetailLayout->addWidget(pHeader, nCurrentRow, nCurrentColumn, 1, 2);
        m_pDetailLayout->setAlignment(pHeader, Qt::AlignTop);
        nCurrentRow++;
    }
    // --------------------------------------------------
    if (recordmt.HasMemo())
    {
        QPlainTextEdit *sec = new QPlainTextEdit;

//        const std::string str_acct_id    = recordmt.GetOtherAccountID();
//        const std::string str_nym_id     = recordmt.GetOtherNymID();
//        const std::string str_server_id  = recordmt.GetServerID();
//        const std::string str_asset_id   = recordmt.GetAssetID();
//
//        QString strMemo = QString("AcctID: %1 NymID: %2 ServerID: %3 AssetID: %4").
//                arg(QString::fromStdString(str_acct_id)).
//                arg(QString::fromStdString(str_nym_id)).
//                arg(QString::fromStdString(str_server_id)).
//                arg(QString::fromStdString(str_asset_id));


        QString strMemo = QString(recordmt.GetMemo().c_str());

        sec->setPlainText(strMemo);
        sec->setReadOnly(true);
        // -----------------------------------------
        QHBoxLayout * pHLayout = new QHBoxLayout;
        QLabel * labelMemo = new QLabel(QString("Memo: "));

        pHLayout->addWidget(labelMemo);
        pHLayout->addWidget(sec);

        pHLayout->setAlignment(labelMemo, Qt::AlignLeft);
        pHLayout->setAlignment(sec, Qt::AlignTop);
        // -----------------------------------------
        m_pDetailLayout->addLayout(pHLayout, nCurrentRow, nCurrentColumn, 1, 2);
        m_pDetailLayout->setAlignment(pHLayout, Qt::AlignTop);

        SetHeight (sec, 2);

        nCurrentRow++;
    }
    // --------------------------------------------------





    // *************************************************************
    QString viewDetails = QString("");

    if (recordmt.IsReceipt() || recordmt.IsOutgoing())
        viewDetails = QString("View Recipient Details");
    else
        viewDetails = QString("View Sender Details");
    // --------------------------------------------------
    const std::string str_acct_id    = recordmt.GetOtherAccountID();
    const std::string str_nym_id     = recordmt.GetOtherNymID();
    const std::string str_server_id  = recordmt.GetServerID();
    const std::string str_asset_id   = recordmt.GetAssetID();

    if (!str_nym_id.empty() && !str_server_id.empty())
        MTContactHandler::getInstance()->NotifyOfNymServerPair(QString::fromStdString(str_nym_id),
                                                               QString::fromStdString(str_server_id));
    // --------------------------------------------------
    int nContactID = 0;

    if (!str_acct_id.empty())
        nContactID = MTContactHandler::getInstance()->FindContactIDByAcctID(QString::fromStdString(str_acct_id),
                                                                            QString::fromStdString(str_nym_id),
                                                                            QString::fromStdString(str_server_id),
                                                                            QString::fromStdString(str_asset_id));
    if (!(nContactID > 0) && !str_nym_id.empty())
        nContactID = MTContactHandler::getInstance()->FindContactIDByNymID (QString::fromStdString(str_nym_id));
    // --------------------------------------------------
    bool bExistingContact = (nContactID > 0);

    if (bExistingContact)
    {
        m_nContactID = nContactID;
        // --------------------------------------------------
        QPushButton * viewContactButton = new QPushButton(viewDetails);

        m_pDetailLayout->addWidget(viewContactButton, nCurrentRow, nCurrentColumn,1,1);
        m_pDetailLayout->setAlignment(viewContactButton, Qt::AlignTop);

        increment_cell(nCurrentRow, nCurrentColumn);
        // -------------------------------------------
        connect(viewContactButton, SIGNAL(clicked()), this, SLOT(on_viewContactButton_clicked()));
    }
    else if (!str_nym_id.empty())
    {
        QPushButton * addContactButton = new QPushButton(QString("Add as Contact"));

        m_pDetailLayout->addWidget(addContactButton, nCurrentRow, nCurrentColumn,1,1);
        m_pDetailLayout->setAlignment(addContactButton, Qt::AlignTop);

        increment_cell(nCurrentRow, nCurrentColumn);
        // -------------------------------------------
        connect(addContactButton, SIGNAL(clicked()), this, SLOT(on_addContactButton_clicked()));
        // --------------------------------------------------
        // If the contact didn't already exist, we don't just have "add new contact"
        // but also "add to existing contact."
        //
        QPushButton * existingContactButton = new QPushButton(QString("Add to an Existing Contact"));

        m_pDetailLayout->addWidget(existingContactButton, nCurrentRow, nCurrentColumn,1,1);
        m_pDetailLayout->setAlignment(existingContactButton, Qt::AlignTop);

        increment_cell(nCurrentRow, nCurrentColumn);
        // ----------------------------------
        connect(existingContactButton, SIGNAL(clicked()), this, SLOT(on_existingContactButton_clicked()));
    }
    // *************************************************************
    if (recordmt.CanDeleteRecord())
    {
        QString deleteActionName = recordmt.IsMail() ? QString("Archive this Message") : QString("Archive this Record");
        QPushButton * deleteButton = new QPushButton(deleteActionName);

        m_pDetailLayout->addWidget(deleteButton, nCurrentRow, nCurrentColumn,1,1);
        m_pDetailLayout->setAlignment(deleteButton, Qt::AlignTop);

        increment_cell(nCurrentRow, nCurrentColumn);
        // -------------------------------------------
        connect(deleteButton, SIGNAL(clicked()), this, SLOT(on_deleteButton_clicked()));
    }
    // --------------------------------------------------

    if (recordmt.CanAcceptIncoming())
    {
        const bool bIsTransfer = (recordmt.GetRecordType() == MTRecord::Transfer);
        const bool bIsReceipt  = (recordmt.GetRecordType() == MTRecord::Receipt);

        QString nameString;
        QString actionString;

        if (recordmt.IsTransfer())
        {
            nameString = QString("Accept this Transfer");
            actionString = QString("Accepting...");
        }
        else if (recordmt.IsReceipt())
        {
            nameString = QString("Accept this Receipt");
            actionString = QString("Accepting...");
        }
        else if (recordmt.IsInvoice())
        {
            nameString = QString("Pay this Invoice");
            actionString = QString("Paying...");
        }
        else if (recordmt.IsPaymentPlan())
        {
            nameString = QString("Activate this Payment Plan");
            actionString = QString("Activating...");
        }
        else if (recordmt.IsContract())
        {
            nameString = QString("Sign this Smart Contract");
            actionString = QString("Signing...");
        }
        else if (recordmt.IsCash())
        {
            nameString = QString("Deposit this Cash");
            actionString = QString("Depositing...");
        }
        else if (recordmt.IsCheque())
        {
            nameString = QString("Deposit this Cheque");
            actionString = QString("Depositing...");
        }
        else if (recordmt.IsVoucher())
        {
            nameString = QString("Accept this Payment");
            actionString = QString("Accepting...");
        }
        else
        {
            nameString = QString("Deposit this Payment");
            actionString = QString("Depositing...");
        }

        QPushButton * acceptButton = new QPushButton(nameString);

        m_pDetailLayout->addWidget(acceptButton, nCurrentRow, nCurrentColumn,1,1);
        m_pDetailLayout->setAlignment(acceptButton, Qt::AlignTop);

        increment_cell(nCurrentRow, nCurrentColumn);

        connect(acceptButton, SIGNAL(clicked()), this, SLOT(on_acceptButton_clicked()));

    }

    if (recordmt.CanCancelOutgoing())
    {
        QString msg = QString("Cancellation Failed. Perhaps recipient had already accepted it?");
//        UIAlertView *fail = [[UIAlertView alloc] initWithTitle:nil message:msg delegate:nil
//                                             cancelButtonTitle:QString("OK") otherButtonTitles:nil];

        QString cancelString;
        QString actionString = QString("Canceling...");

        if (recordmt.IsInvoice())
            cancelString = QString("Cancel this Invoice");
        else if (recordmt.IsPaymentPlan())
            cancelString = QString("Cancel this Payment Plan");
        else if (recordmt.IsContract())
            cancelString = QString("Cancel this Smart Contract");
        else if (recordmt.IsCash())
        {
            cancelString = QString("Recover this Cash");
            actionString = QString("Recovering...");
            msg = QString("Recovery Failed. Perhaps recipient had already accepted it?");
        }
        else if (recordmt.IsCheque())
            cancelString = QString("Cancel this Cheque");
        else if (recordmt.IsVoucher())
            cancelString = QString("Cancel this Payment");
        else
            cancelString = QString("Cancel this Payment");


        QPushButton * cancelButton = new QPushButton(cancelString);

        m_pDetailLayout->addWidget(cancelButton, nCurrentRow, nCurrentColumn,1,1);
        m_pDetailLayout->setAlignment(cancelButton, Qt::AlignTop);

        increment_cell(nCurrentRow, nCurrentColumn);

        connect(cancelButton, SIGNAL(clicked()), this, SLOT(on_cancelButton_clicked()));

    }

    if (recordmt.CanDiscardOutgoingCash())
    {
        QString discardString = QString("Discard this Sent Cash");

        QPushButton * discardOutgoingButton = new QPushButton(discardString);

        m_pDetailLayout->addWidget(discardOutgoingButton, nCurrentRow, nCurrentColumn,1,1);
        m_pDetailLayout->setAlignment(discardOutgoingButton, Qt::AlignTop);

        increment_cell(nCurrentRow, nCurrentColumn);

        connect(discardOutgoingButton, SIGNAL(clicked()), this, SLOT(on_discardOutgoingButton_clicked()));


    }

    if (recordmt.CanDiscardIncoming())
    {
        QString discardString;

        if (recordmt.IsInvoice())
            discardString = QString("Discard this Invoice");
        else if (recordmt.IsPaymentPlan())
            discardString = QString("Discard this Payment Plan");
        else if (recordmt.IsContract())
            discardString = QString("Discard this Smart Contract");
        else if (recordmt.IsCash())
            discardString = QString("Discard this Cash");
        else if (recordmt.IsCheque())
            discardString = QString("Discard this Cheque");
        else if (recordmt.IsVoucher())
            discardString = QString("Discard this Payment");
        else
            discardString = QString("Discard this Payment");


        QPushButton * discardIncomingButton = new QPushButton(discardString);

        m_pDetailLayout->addWidget(discardIncomingButton, nCurrentRow, nCurrentColumn,1,1);
        m_pDetailLayout->setAlignment(discardIncomingButton, Qt::AlignTop);

        increment_cell(nCurrentRow, nCurrentColumn);

        connect(discardIncomingButton, SIGNAL(clicked()), this, SLOT(on_discardIncomingButton_clicked()));

    }

    if (!(recordmt.GetOtherNymID().empty()))
    {
        QString msgUser;

        if (recordmt.IsReceipt() || recordmt.IsOutgoing())
            msgUser = QString("Message the Recipient");
        else
            msgUser = QString("Message the Sender");


        QPushButton * msgButton = new QPushButton(((recordmt.IsMail() && !recordmt.IsOutgoing()) ? QString("Reply to this Message") : msgUser));

        m_pDetailLayout->addWidget(msgButton, nCurrentRow, nCurrentColumn,1,1);
        m_pDetailLayout->setAlignment(msgButton, Qt::AlignTop);

        increment_cell(nCurrentRow, nCurrentColumn);

        connect(msgButton, SIGNAL(clicked()), this, SLOT(on_msgButton_clicked()));

    }

    // ----------------------------------
    if (nCurrentColumn > 1)
    {
        nCurrentColumn = 1;
        nCurrentRow++;
    }
    // ----------------------------------

    // TRANSACTION IDs DISPLAYED HERE

//    ActionSection *act = [ActionSection sectionWithName:QString("Actions") andActions:actions];
//    act.defaultAlignment = (UITextAlignment) UITextAlignmentLeft;

//    TransactionIdSection* idSec = [TransactionIdSection sectionWithRecord:record];
    // ----------------------------------
//    NSMutableArray *sections = [NSMutableArray arrayWithArray:QString()[
//                                [TransactionSummarySection sectionWithRecord:record],
//                                act,
//                                idSec,
//                                ]];
    // ----------------------------------
    QTabWidget * pTabWidget  = new QTabWidget;
    QWidget    * pTab1Widget = new QWidget;
    QWidget    * pTab2Widget = NULL;

    pTabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pTab1Widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    pTabWidget->setContentsMargins(5, 5, 5, 5);
    pTab1Widget->setContentsMargins(5, 5, 5, 5);

    pTabWidget->addTab(pTab1Widget, QString("Details"));
    // ----------------------------------
    if (recordmt.HasContents())
    {
        QPlainTextEdit *sec = new QPlainTextEdit;

        QString strContents = QString(recordmt.GetContents().c_str());

        sec->setPlainText(strContents);
        sec->setReadOnly(true);
        sec->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        // -------------------------------

        // -------------------------------
        QVBoxLayout * pvBox = NULL;

        if (recordmt.IsMail())
        {
            m_pDetailLayout->addWidget(sec, nCurrentRow++, nCurrentColumn, 1, 2);
        }
        else
        {
            pvBox       = new QVBoxLayout;
            pTab2Widget = new QWidget;
            QLabel * pLabelContents = new QLabel(QString("Raw Contents:"));

            pvBox->setAlignment(Qt::AlignTop);
            pvBox->addWidget   (pLabelContents);
            pvBox->addWidget   (sec);

            pTab2Widget->setContentsMargins(0, 0, 0, 0);
            pTab2Widget->setLayout(pvBox);

            pTabWidget->addTab(pTab2Widget, QString("Contents"));
        }
        // -------------------------------
    }    


    pTab1Widget->setLayout(m_pDetailLayout);

    // -----------------------------------------------
    this->m_pDetailLayout = new QGridLayout;
    this->m_pDetailLayout->addWidget(pTabWidget);

    m_pDetailLayout->setContentsMargins(0,0,0,0);
    pTabWidget->setTabPosition(QTabWidget::South);

    this->setLayout(m_pDetailLayout);

    // ----------------------------------

/*
    if (recordmt.IsPaymentPlan())
    {
        if (recordmt.HasInitialPayment() || recordmt.HasPaymentPlan())
        {
            std::string str_asset_name = OTAPI_Wrap::GetAssetType_Name(recordmt.GetAssetID().c_str());
            // ---------------------------------
            std::stringstream sss;
            sss << "Payments use the currency: " << str_asset_name << "\n";
            // ---------------------------------
            NSDateFormatter * formatter  = nil;
            NSString        * dateString = nil;
            NSDate          * date       = nil;

            formatter = [[NSDateFormatter alloc] init];
            [formatter setDateFormat:QString("dd-MM-yyyy HH:mm")];

            if (recordmt.HasInitialPayment())
            {
                date = [NSDate dateWithTimeIntervalSince1970:recordmt.GetInitialPaymentDate()];
                dateString = [formatter stringFromDate:[NSDate date]];

                long        lAmount    = recordmt.GetInitialPaymentAmount();
                std::string str_output = OTAPI_Wrap::It()->FormatAmount(recordmt.GetAssetID().c_str(),
                                                                        static_cast<int64_t>(lAmount));
                sss << "Initial payment of " << str_output << " due: " << dateString.UTF8String << "\n";
            }
            // -----------------------------------------------
            if (recordmt.HasPaymentPlan())
            {
                // ----------------------------------------------------------------
                date = [NSDate dateWithTimeIntervalSince1970:recordmt.GetPaymentPlanStartDate()];
                dateString = [formatter stringFromDate:[NSDate date]];

                long        lAmount    = recordmt.GetPaymentPlanAmount();
                std::string str_output = OTAPI_Wrap::It()->FormatAmount(recordmt.GetAssetID().c_str(),
                                                                        static_cast<int64_t>(lAmount));
                sss << "Recurring payments of " << str_output << " begin: " << dateString.UTF8String << " ";
                // ----------------------------------------------------------------
                NSDate *date2 = [NSDate dateWithTimeIntervalSince1970:(recordmt.GetPaymentPlanStartDate() + recordmt.GetTimeBetweenPayments())];
                NSCalendar *calendar = [NSCalendar currentCalendar];
                NSUInteger calendarUnits = NSDayCalendarUnit;
                NSDateComponents *dateComponents = [calendar components:calendarUnits fromDate:date toDate:date2 options:0];

                std::string str_regular = (([dateComponents day] == 1) ?
                                           "and repeat daily." :
                                           "and repeat every %i days.");
                // -----------------------------------------------------------------
                NSString * strInterval = nil;

                if ([dateComponents day] == 1)
                    strInterval = QString(str_regular.c_str());
                else
                    strInterval = [NSString stringWithFormat:QString(str_regular.c_str()),[dateComponents day]];
                // -----------------------------------------------------------------
                sss << strInterval.UTF8String << "\n";
                // -----------------------------------------------------------------
                if (recordmt.GetMaximumNoPayments() > 0)
                    sss << "The maximum number of payments is: " << recordmt.GetMaximumNoPayments() << "\n";
                // -----------------------------------------------------------------
//todo:         inline const time_t &	GetPaymentPlanLength()	 const	{ return m_tPaymentPlanLength; }
            }
            // -----------------------------------------------
            std::string str_details(sss.str());
            TextViewSection *sec = [TextViewSection sectionWithName:QString("Recurring Payment Terms")
                                                      initialString:QString(str_details.c_str()) andStringAction:nil];
            sec.textView.editable = NO;
            sec.customHeight = 80;

            [sections insertObject:sec atIndex:1]; // So the recurring payment terms appears just under the memo.
        }
    }
*/
}















