#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/moneychanger.hpp>
#include <core/handlers/DBHandler.hpp>
#include <core/handlers/contacthandler.hpp>
#include <gui/widgets/homedetail.hpp>

#include <core/handlers/modelpayments.hpp>

#include <opentxs/core/OTStorage.hpp>
#include <opentxs/client/OTAPI.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>
#include <opentxs/client/OT_ME.hpp>
#include <opentxs/core/util/OTPaths.hpp>

#include <opentxs/client/OTRecordList.hpp>

#include <QDebug>
#include <QtGlobal>
#include <QDateTime>
#include <Qt>
#include <QLabel>
#include <QHBoxLayout>
#include <QTableView>


// ------------------------------------------------------------

AccountRecordsProxyModel::AccountRecordsProxyModel(QObject *parent /*=0*/)
: QSortFilterProxyModel(parent)
{
}

void AccountRecordsProxyModel::clearFilterType()
{
    filterType_ = FilterNone;
}

void AccountRecordsProxyModel::setFilterNone()
{
    clearFilterType();
    invalidateFilter();
}

void AccountRecordsProxyModel::setFilterSent()
{
    filterType_ = FilterSent;
    invalidateFilter();
}

void AccountRecordsProxyModel::setFilterReceived()
{
    filterType_ = FilterReceived;
    invalidateFilter();
}

void AccountRecordsProxyModel::setFilterString(QString qstrFilter)
{
    filterString_ = qstrFilter;
    invalidateFilter();
}

void AccountRecordsProxyModel::setFilterAccountId(QString qstrFilter)
{
    filterAccount_ = qstrFilter;
    invalidateFilter();
}



QWidget * AccountRecordsProxyModel::CreateDetailHeaderWidget(const int nSourceRow, bool bExternal/*=true*/) const
{
    QModelIndex sourceIndex_Folder         = sourceModel()->index(nSourceRow, PMNT_SOURCE_COL_FOLDER);
    QModelIndex sourceIndex_AssetId        = sourceModel()->index(nSourceRow, PMNT_SOURCE_COL_MY_ASSET_TYPE);
    QModelIndex sourceIndex_Date           = sourceModel()->index(nSourceRow, PMNT_SOURCE_COL_TIMESTAMP);
    QModelIndex sourceIndex_Desc           = sourceModel()->index(nSourceRow, PMNT_SOURCE_COL_DESCRIPTION);
    QModelIndex sourceIndex_RecordName     = sourceModel()->index(nSourceRow, PMNT_SOURCE_COL_RECORD_NAME);
    QModelIndex sourceIndex_InstrumentType = sourceModel()->index(nSourceRow, PMNT_SOURCE_COL_INSTRUMENT_TYPE);
    QModelIndex sourceIndex_Amount         = sourceModel()->index(nSourceRow, PMNT_SOURCE_COL_AMOUNT);
    QModelIndex sourceIndex_PendingFound   = sourceModel()->index(nSourceRow, PMNT_SOURCE_COL_PENDING_FOUND);
    QModelIndex sourceIndex_CompletedFound = sourceModel()->index(nSourceRow, PMNT_SOURCE_COL_COMPLETED_FOUND);
    QModelIndex sourceIndex_Flags          = sourceModel()->index(nSourceRow, PMNT_SOURCE_COL_FLAGS);

    QVariant    sourceData_Folder     = sourceModel()->data(sourceIndex_Folder,  Qt::DisplayRole);
    QVariant    sourceData_AssetId    = sourceModel()->data(sourceIndex_AssetId, Qt::DisplayRole);
    QVariant    sourceData_Date       = sourceModel()->data(sourceIndex_Date,    Qt::DisplayRole);
    QVariant    sourceData_Desc       = sourceModel()->data(sourceIndex_Desc,    Qt::DisplayRole);
    QVariant    sourceData_RecordName = sourceModel()->data(sourceIndex_RecordName,    Qt::DisplayRole);
    QVariant    sourceData_InstrumentType = sourceModel()->data(sourceIndex_InstrumentType,Qt::DisplayRole);
    QVariant    sourceData_Amount  = sourceModel()->data(sourceIndex_Amount,   Qt::DisplayRole);
    QVariant    sourceData_PendingFound  = sourceModel()->data(sourceIndex_PendingFound, Qt::DisplayRole);
    QVariant    sourceData_CompletedFound  = sourceModel()->data(sourceIndex_CompletedFound, Qt::DisplayRole);
    QVariant    sourceData_Flags      = sourceModel()->data(sourceIndex_Flags,  Qt::DisplayRole);

    QString     qstrAssetId     = sourceData_AssetId.isValid() ? sourceData_AssetId.toString() : "";
    QString     qstrDescription = sourceData_Desc   .isValid() ? sourceData_Desc   .toString() : "";
    QString     qstrRecordName  = sourceData_RecordName       .isValid() ? sourceData_RecordName    .toString() : "";
    QString     qstrInstrumentType = sourceData_InstrumentType.isValid() ? sourceData_InstrumentType.toString() : "";

    ModelPayments::PaymentFlags flags = sourceData_Flags.isValid() ? (ModelPayments::PaymentFlag)sourceData_Flags.toLongLong()
                                                                  : ModelPayments::NoFlags;

    const int64_t lAmount = sourceData_Amount.isValid() ? sourceData_Amount.toLongLong() : 0;

    time64_t the_time = sourceData_Date.isValid() ? sourceData_Date.toLongLong() : 0;
    QDateTime timestamp;
    timestamp.setTime_t(the_time);
    QString qstrTimestamp = QString(timestamp.toString(Qt::SystemLocaleShortDate));

    const bool bIsOutgoing     = flags.testFlag(ModelPayments::IsOutgoing);
    const bool bIsPending      = flags.testFlag(ModelPayments::IsPending);

//  const bool bIsOutgoing     = sourceData_Folder.isValid()         ? (sourceData_Folder.toInt() == 0)   : false;
//  const bool bPendingFound   = sourceData_PendingFound.isValid()   ? sourceData_PendingFound.toBool()   : false;
//  const bool bCompletedFound = sourceData_CompletedFound.isValid() ? sourceData_CompletedFound.toBool() : false;
//  const bool bIsPending = (bPendingFound && !bCompletedFound);


//    qDebug() << "bIsOutgoing: " << (bIsOutgoing ? QString("true") :  QString("false"))
//    << " bPendingFound: " << (bPendingFound ? QString("true") :  QString("false")) <<
//    " bCompletedFound: " << (bCompletedFound ? QString("true") :  QString("false")) <<
//    " bIsPending: " << (bIsPending ? QString("true") :  QString("false")) << "\n";


    TransactionTableViewCellType cellType = (bIsOutgoing ?
                                                 // -------------------------------------------------
                                                 (bIsPending ?
                                                      TransactionTableViewCellTypeOutgoing :  // outgoing
                                                      TransactionTableViewCellTypeSent) :     // sent
                                                 // -------------------------------------------------
                                                 (bIsPending ?
                                                      TransactionTableViewCellTypeIncoming :  // incoming
                                                      TransactionTableViewCellTypeReceived)); // received
    // --------------------------------------------------------------------------------------------
    // For invoices and invoice receipts.
    //

    const bool bIsInvoice       = flags.testFlag(ModelPayments::IsInvoice);
    const bool bIsPlan          = flags.testFlag(ModelPayments::IsPaymentPlan);
    const bool bIsNotice        = flags.testFlag(ModelPayments::IsNotice);
    const bool bIsChequeReceipt = (0 == qstrInstrumentType.compare("chequeReceipt"));
    const bool bIsMarketReceipt = (0 == qstrInstrumentType.compare("marketReceipt"));

    if (bIsInvoice || bIsPlan || bIsNotice ||
        ( bIsChequeReceipt &&
          (( bIsOutgoing && lAmount > 0) ||
           (!bIsOutgoing && lAmount < 0))
        ) )
        cellType = (bIsOutgoing ?
                    (bIsPending ?
                     TransactionTableViewCellTypeIncoming  : // outgoing
                     TransactionTableViewCellTypeReceived) : // sent
                    // -------------------------------------------------
                    (bIsPending ?
                     TransactionTableViewCellTypeOutgoing  : // incoming
                     TransactionTableViewCellTypeSent));     // received
    // --------------------------------------------------------------------------------------------
    if (bIsMarketReceipt)
    {
        cellType = (lAmount > 0) ? TransactionTableViewCellTypeReceived : TransactionTableViewCellTypeSent;
    }
    // --------------------------------------------------------------------------------------------
    std::string str_desc = qstrDescription.toStdString();
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
            OT_FAIL_MSG("Expected all cell types to be handled for color.");
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
    QString tx_name = qstrRecordName;

    if (tx_name.trimmed() == "")
    {
        //Tx has no name
        tx_name.clear();
        tx_name = "Receipt";
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

    if ( !qstrAssetId.isEmpty() )
    {
        str_formatted = opentxs::OTAPI_Wrap::It()->FormatAmount(qstrAssetId.toStdString(), lAmount);
        bFormatted = !str_formatted.empty();
    }
    // ----------------------------------------
    if (bFormatted)
        currency_amount = QString::fromStdString(str_formatted);
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
    QLabel * row_content_date_label = new QLabel;
    QString row_content_date_label_string;
    row_content_date_label_string.append(qstrTimestamp);

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
    row_widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    return row_widget;
}

QVariant AccountRecordsProxyModel::data ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
//    if (role == Qt::TextAlignmentRole)
//        return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
    // ----------------------------------------
    if ( role==Qt::SizeHintRole && index.isValid())
    {
//      const int nSourceRow    = headerData(index.row(),    Qt::Vertical,   Qt::UserRole).toInt();
        const int nSourceColumn = headerData(index.column(), Qt::Horizontal, Qt::UserRole).toInt();

        if (PMNT_SOURCE_COL_PMNT_ID == nSourceColumn)
        {
            return QVariant::fromValue(QSize(300, 60));
        }
    }
    // ----------------------------------------
    if ( role==Qt::FontRole && index.isValid())
    {
        const int nSourceRow    = headerData(index.row(),    Qt::Vertical,   Qt::UserRole).toInt();
        const int nSourceColumn = PMNT_SOURCE_COL_HAVE_READ; // If you haven't read this Payment, we set it bold.

        QModelIndex sourceIndex = sourceModel()->index(nSourceRow, nSourceColumn);
        QVariant    sourceData  = sourceModel()->data(sourceIndex, Qt::DisplayRole);

        if (sourceData.isValid() && (!sourceData.toBool()))
        {
            QFont boldFont;
            boldFont.setBold(true);
            return boldFont;
        }
    }
    // ----------------------------------------
    else if ( role==Qt::DisplayRole && index.isValid() &&
        QSortFilterProxyModel::data(index,role).isValid())
    {
        const int nSourceRow    = headerData(index.row(),    Qt::Vertical,   Qt::UserRole).toInt();
        const int nSourceColumn = headerData(index.column(), Qt::Horizontal, Qt::UserRole).toInt();

        QModelIndex sourceIndex = sourceModel()->index(nSourceRow, nSourceColumn);
        QVariant    sourceData  = sourceModel()->data(sourceIndex, role);

        if (nSourceColumn == PMNT_SOURCE_COL_PMNT_ID) // payment_id
        {
            if (nullptr != pTableView_)
            {
                if (nullptr == pTableView_->indexWidget(index))
                {
                    QWidget * pWidget = CreateDetailHeaderWidget(nSourceRow);
                    pTableView_->setIndexWidget(index, pWidget);
                }
            }
            return QVariant();
        }
        else
        if (nSourceColumn == PMNT_SOURCE_COL_MY_NYM) // my_nym_id
        {
            QString qstrID = sourceData.isValid() ? sourceData.toString() : "";

            std::string str_name;

            if (!qstrID.isEmpty())
            {
                const std::string str_id = qstrID.trimmed().toStdString();
                str_name = str_id.empty() ? "" : opentxs::OTAPI_Wrap::It()->GetNym_Name(str_id);
            }
            // ------------------------
            if (str_name.empty() && !qstrID.isEmpty())
                return QVariant(qstrID);
            return QVariant(QString::fromStdString(str_name));
        }
        else if (nSourceColumn == PMNT_SOURCE_COL_MY_ACCT) // my_acct_id
        {
            QString qstrID = sourceData.isValid() ? sourceData.toString() : "";

            std::string str_name;

            if (!qstrID.isEmpty())
            {
                const std::string str_id = qstrID.trimmed().toStdString();
                str_name = str_id.empty() ? "" : opentxs::OTAPI_Wrap::It()->GetAccountWallet_Name(str_id);
            }
            // ------------------------
            if (str_name.empty() && !qstrID.isEmpty())
                return QVariant(qstrID);
            return QVariant(QString::fromStdString(str_name));
        }
        else if (nSourceColumn == PMNT_SOURCE_COL_TXN_ID) // txn_id
        {
            QModelIndex sibling = sourceIndex.sibling(sourceIndex.row(), PMNT_SOURCE_COL_TXN_ID_DISPLAY);

            int64_t lID         = sourceData.isValid() ? sourceData.toLongLong() : 0;
            int64_t lDisplayID  = sourceModel()->data(sibling,role).isValid() ? sourceModel()->data(sibling,role).toLongLong() : 0;
            // ------------------------
            if (lDisplayID > 0)
                return QVariant::fromValue(lDisplayID);
            return QVariant::fromValue(lID);
        }
        else if (nSourceColumn == PMNT_SOURCE_COL_SENDER_NYM) // sender_nym_id
        {
            QString qstrID = sourceData.isValid() ? sourceData.toString().trimmed() : "";
            const std::string str_id = qstrID.isEmpty() ? "" : qstrID.toStdString();
            MTNameLookupQT theLookup;
            const std::string str_name = str_id.empty() ? "" : theLookup.GetNymName(str_id, "");
            // ------------------------
            if (!str_name.empty())
                return QVariant(QString::fromStdString(str_name));

            QModelIndex sibling = sourceIndex.sibling(sourceIndex.row(), PMNT_SOURCE_COL_SENDER_ADDR);
            QString qstrAddress = sourceModel()->data(sibling,role).isValid() ? sourceModel()->data(sibling,role).toString() : QString("");

            if (!qstrAddress.isEmpty())
            {
                const int nContactID = MTContactHandler::getInstance()->GetContactByAddress(qstrAddress);

                if (nContactID > 0)
                {
                    QString qstrContactName = MTContactHandler::getInstance()->GetContactName(nContactID);

                    if (!qstrContactName.isEmpty())
                        return QVariant(qstrContactName);
                }
            }

            if (!qstrID.isEmpty())
                return QVariant(qstrID);

            if (!qstrAddress.isEmpty())
                return QVariant(qstrAddress);

            return sourceData;
        }
        else if (nSourceColumn == PMNT_SOURCE_COL_RECIP_NYM) // recipient_nym_id
        {
            QString qstrID = sourceData.isValid() ? sourceData.toString().trimmed() : "";
            const std::string str_id = qstrID.isEmpty() ? "" : qstrID.toStdString();
            MTNameLookupQT theLookup;
            const std::string str_name = str_id.empty() ? "" : theLookup.GetNymName(str_id, "");
            // ------------------------
            if (!str_name.empty())
                return QVariant(QString::fromStdString(str_name));

            QModelIndex sibling = sourceIndex.sibling(sourceIndex.row(), PMNT_SOURCE_COL_RECIP_ADDR);
            QString qstrAddress = sourceModel()->data(sibling,role).isValid() ? sourceModel()->data(sibling,role).toString() : QString("");

            if (!qstrAddress.isEmpty())
            {
                const int nContactID = MTContactHandler::getInstance()->GetContactByAddress(qstrAddress);

                if (nContactID > 0)
                {
                    QString qstrContactName = MTContactHandler::getInstance()->GetContactName(nContactID);

                    if (!qstrContactName.isEmpty())
                        return QVariant(qstrContactName);
                }
            }

            if (!qstrID.isEmpty())
                return QVariant(qstrID);

            if (!qstrAddress.isEmpty())
                return QVariant(qstrAddress);

            return sourceData;
        }
        else if (nSourceColumn == PMNT_SOURCE_COL_NOTARY_ID) // notary_id
        {
            QString qstrID = sourceData.isValid() ? sourceData.toString() : "";
            const std::string str_id = qstrID.toStdString();
            const std::string str_name = str_id.empty() ? "" : opentxs::OTAPI_Wrap::It()->GetServer_Name(str_id);
            // ------------------------
            if (!str_name.empty())
                return QVariant(QString::fromStdString(str_name));

            if (!qstrID.isEmpty())
                return QVariant(qstrID);

            return sourceData;
        }
        else if (nSourceColumn == PMNT_SOURCE_COL_AMOUNT) // amount
        {
            QModelIndex sourceIndex_Amount         = sourceModel()->index(nSourceRow, PMNT_SOURCE_COL_AMOUNT);

            QVariant qvarAmount = sourceIndex_Amount.isValid() ? sourceModel()->data(sourceIndex_Amount) : QVariant();

            //int64_t lAmount = sourceData.isValid() ? sourceData.toLongLong() : 0;
            int64_t lAmount = qvarAmount.isValid() ? qvarAmount.toLongLong() : 0;

            QModelIndex sibling   = sourceIndex.sibling(sourceIndex.row(), PMNT_SOURCE_COL_MY_ASSET_TYPE);
            QString qstrAssetType = sourceModel()->data(sibling,role).isValid() ? sourceModel()->data(sibling,role).toString() : QString("");

            QString qstrAmount = QString::fromStdString(opentxs::OTAPI_Wrap::It()->LongToString(lAmount));

            if (!qstrAssetType.isEmpty())
            {
                QString qstrTemp = QString::fromStdString(opentxs::OTAPI_Wrap::It()->FormatAmount(qstrAssetType.toStdString(), lAmount));
                if (!qstrTemp.isEmpty())
                    qstrAmount = qstrTemp;
            }
            // ----------------------------------------------
            return QVariant(qstrAmount);
        }
        else if (nSourceColumn == PMNT_SOURCE_COL_MY_ASSET_TYPE) // my_asset_type_id
        {
            QString qstrID = sourceData.isValid() ? sourceData.toString() : "";
            const std::string str_id = qstrID.toStdString();
            const std::string str_name = str_id.empty() ? "" : opentxs::OTAPI_Wrap::It()->GetAssetType_Name(str_id);
            // ------------------------
            if (!str_name.empty())
                return QVariant(QString::fromStdString(str_name));

            if (!qstrID.isEmpty())
                return QVariant(qstrID);

            return sourceData;
        }
        else if (nSourceColumn == PMNT_SOURCE_COL_METHOD_TYPE_DISP) // Method type display (if blank, we see if there's a notary ID. If so, then transport shows "otserver".)
        {
            QString qstrType = sourceData.isValid() ? sourceData.toString() : "";
            // ------------------------
            if (!qstrType.isEmpty())
                return QVariant(qstrType);
            return QVariant(QString("opentxs"));
        }
        else if (nSourceColumn == PMNT_SOURCE_COL_TIMESTAMP) // timestamp
        {
            time64_t the_time = sourceData.isValid() ? sourceData.toLongLong() : 0;
            QDateTime timestamp;
            timestamp.setTime_t(the_time);
            return QVariant(QString(timestamp.toString(Qt::SystemLocaleShortDate)));
        }
        else if (nSourceColumn == PMNT_SOURCE_COL_FOLDER) // folder
        {
            int nFolder = sourceData.isValid() ? sourceData.toInt() : 0;
            return QVariant(tr(0 == nFolder ? "Sent" : "Receieved"));
        }
    }
    // -------------------------------
    else if (role==Qt::DecorationRole && index.isValid())
    {
        const int nSourceRow             = headerData(index.row(),    Qt::Vertical,   Qt::UserRole).toInt();
        const int nSourceColumn          = headerData(index.column(), Qt::Horizontal, Qt::UserRole).toInt();

        const int nSourceColumnReplied   = PMNT_SOURCE_COL_HAVE_REPLIED;
        const int nSourceColumnForwarded = PMNT_SOURCE_COL_HAVE_FORWARDED;

        QModelIndex sourceIndexReplied   = sourceModel()->index(nSourceRow, nSourceColumnReplied);
        QModelIndex sourceIndexForwarded = sourceModel()->index(nSourceRow, nSourceColumnForwarded);

        QVariant    sourceDataReplied    = sourceModel()->data(sourceIndexReplied,   Qt::DisplayRole);
        QVariant    sourceDataForwarded  = sourceModel()->data(sourceIndexForwarded, Qt::DisplayRole);

        if (nSourceColumn == PMNT_SOURCE_COL_MEMO)
        {
            const bool bHaveReplied   = sourceDataReplied.isValid() ? sourceDataReplied.toBool() : false;
            const bool bHaveForwarded = sourceDataForwarded.isValid() ? sourceDataForwarded.toBool() : false;
            const bool bHaveBoth      = (bHaveReplied && bHaveForwarded);

            if (bHaveBoth)
            {
                QPixmap pixmap(":/icons/icons/replied_forwarded.png");
                return pixmap;
            }
            else if (bHaveReplied)
            {
                QPixmap pixmap(":/icons/icons/replied.png");
                return pixmap;
            }
            if (bHaveForwarded)
            {
                QPixmap pixmap(":/icons/icons/forwarded.png");
                return pixmap;
            }
        }
    }
    // --------------------------------------------
    return QSortFilterProxyModel::data(index,role);
}


QVariant AccountRecordsProxyModel::headerData(int section, Qt::Orientation orientation, int role/*=Qt::DisplayRole*/) const
{
    QVariant varHeaderData = QSortFilterProxyModel::headerData(section, orientation, role);

    if (Qt::Horizontal == orientation && Qt::DisplayRole == role)
    {
        QString qstrHeader = varHeaderData.toString();

        if (0 == qstrHeader.compare(tr("Me")))
        {
            if (FilterSent == filterType_) // sent box
                return QVariant(tr("From"));
            else if (FilterReceived == filterType_) // received box
                return QVariant(tr("To"));
            else
                return QVariant(tr("Me"));
        }
        else if (0 == qstrHeader.compare(tr("Details")))
        {
            if (FilterSent == filterType_) // sent box
                return QVariant(tr("To"));
            else if (FilterReceived == filterType_) // received box
                return QVariant(tr("From"));
            else
                return QVariant(tr("Details"));
        }
    }

    return varHeaderData;
}

// --------------------------------------------

bool AccountRecordsProxyModel::filterAcceptsColumn(int source_column, const QModelIndex & source_parent) const
{
    bool bReturn = true;

    switch (source_column)
    {
    case PMNT_SOURCE_COL_PMNT_ID:   bReturn = true;  break; // payment_id
    case PMNT_SOURCE_COL_MY_NYM:  // my_nym_id
    {
        bReturn = false;
    }
        break;
    case PMNT_SOURCE_COL_MY_ACCT:
    {
        bReturn = false;
    }
        break;
    case PMNT_SOURCE_COL_MY_ASSET_TYPE:
    {
        bReturn = false;
    }
        break;
    case PMNT_SOURCE_COL_MY_ADDR:
    {
        bReturn = false;
    }
        break;
    case PMNT_SOURCE_COL_SENDER_NYM:
    {
        if (FilterSent == filterType_)
            return false;
        else if (FilterNone == filterType_)
            return false;
        else
            bReturn = true;
    }
        break;
    case PMNT_SOURCE_COL_SENDER_ACCT:
    {
        bReturn = false;
    }
        break;

    case PMNT_SOURCE_COL_SENDER_ADDR:
    {
        bReturn = false;
    }
        break;
    case PMNT_SOURCE_COL_RECIP_NYM:
    {
        if (FilterReceived == filterType_)
            return false;
        else if (FilterNone == filterType_)
            return false;
        else
            bReturn = true;
    }
        break;
    case PMNT_SOURCE_COL_RECIP_ACCT:
    {
        bReturn = false;
    }
        break;

    case PMNT_SOURCE_COL_RECIP_ADDR:
    {
        bReturn = false;
    }
        break;
    case PMNT_SOURCE_COL_METHOD_TYPE:      bReturn = false;  break;
    case PMNT_SOURCE_COL_METHOD_TYPE_DISP:
    {
        bReturn = false;
    }
        break;
    case PMNT_SOURCE_COL_NOTARY_ID:
    {
        bReturn = false;
    }
        break;
    case PMNT_SOURCE_COL_TXN_ID:          bReturn = true;   break;
    case PMNT_SOURCE_COL_TXN_ID_DISPLAY:  bReturn = false;  break;
    case PMNT_SOURCE_COL_AMOUNT:          bReturn = true;   break;
    case PMNT_SOURCE_COL_PENDING_FOUND:   bReturn = false;  break;
    case PMNT_SOURCE_COL_COMPLETED_FOUND: bReturn = false;  break;
    case PMNT_SOURCE_COL_TIMESTAMP:       bReturn = true;   break;
    case PMNT_SOURCE_COL_HAVE_READ:       bReturn = false;  break;
    case PMNT_SOURCE_COL_HAVE_REPLIED:    bReturn = false;  break;
    case PMNT_SOURCE_COL_HAVE_FORWARDED:  bReturn = false;  break;
    case PMNT_SOURCE_COL_MEMO:            bReturn = true;   break;
    case PMNT_SOURCE_COL_DESCRIPTION:     bReturn = false;  break;
    case PMNT_SOURCE_COL_RECORD_NAME:     bReturn = false;  break;
    case PMNT_SOURCE_COL_FOLDER:          bReturn = false;  break;
    case PMNT_SOURCE_COL_FLAGS:           bReturn = false;  break;
    default:  bReturn = true;  break;
    }
    return bReturn;
}


// What do we filter on here?
// Well, the Payment table doesn't contain a Contact ID.
// Instead, it contains a NymID and/or a Bitmessage address and/or a notary address.
//
// When the user clicks on a top-level Contact ID, we need to filter on ALL known Nyms AND Bitmessage addresses for that Contact.
// Any single one of them could be the one on the Payment, that causes the filter to approve it.
// Notice in that case they're not ALL required -- just ONE of them.
//
// Next let's say a user clicks on a contact's Bitmessage address. In that case it's easy -- we can just filter
// based on that Bitmessage address.
//
// Next let's say a user clicks on a contact's Notary. In that case we need to filter on ALL known Nyms for that contact
// AND we need to filter on the notary--and the notary is part of the filter.
//
// That means there are 3 main filter types:
// 1. otserver    -- must pass NotaryID as well as ContactID (that's so we can look up the Nym list for that contact.) Requirement is "any Nym from that list, plus the notary."
// 2. method_type -- must pass the method type and the address. That's the requirement, too (both.)
// 3. top-level   -- contact ID is only thing passed. Look up ALL Nyms and Addresses for that contact. Any of them is good enough for a match.
//
bool AccountRecordsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex indexTxnId        = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_TXN_ID,         sourceParent); // txn_id
    QModelIndex indexTxnIdDisplay = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_TXN_ID_DISPLAY, sourceParent); // txn_id_display
    QModelIndex indexMyNym        = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_MY_NYM,         sourceParent); // my_nym_id
    QModelIndex indexMyAcct       = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_MY_ACCT,        sourceParent); // my_acct_id
    QModelIndex indexAssetType    = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_MY_ASSET_TYPE,  sourceParent); // my_asset_type_id
    QModelIndex indexSenderNym    = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_SENDER_NYM,     sourceParent); // sender_nym_id
    QModelIndex indexSenderAcct   = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_SENDER_ACCT,    sourceParent); // sender_acct_id
    QModelIndex indexSenderAddr   = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_SENDER_ADDR,    sourceParent); // sender_address
    QModelIndex indexRecipNym     = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_RECIP_NYM,      sourceParent); // recipient_nym_id
    QModelIndex indexRecipAcct    = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_RECIP_ACCT,     sourceParent); // recipient_acct_id
    QModelIndex indexRecipAddr    = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_RECIP_ADDR,     sourceParent); // recipient_address
    QModelIndex indexMethodType   = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_METHOD_TYPE,    sourceParent); // method_type
    QModelIndex indexNotary       = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_NOTARY_ID,      sourceParent); // notary_id
    QModelIndex indexFolder       = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_FOLDER,         sourceParent); // folder
    QModelIndex indexMemo         = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_MEMO,           sourceParent); // memo
    QModelIndex indexDescription  = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_DESCRIPTION,    sourceParent); // description

    QAbstractItemModel * pModel    = sourceModel();
    ModelPayments      * pMsgModel = dynamic_cast<ModelPayments*>(pModel);

    if (nullptr != pMsgModel)
    {
        // Check the folder here.
        QVariant dataFolder = pMsgModel->data(indexFolder);
        const int nFolder = dataFolder.isValid() ? dataFolder.toInt() : (-1);

        if (-1 != nFolder)
        {
            if (FilterSent == filterType_ && nFolder != 0)
                return false;
            else if (FilterReceived == filterType_ && !(nFolder > 0) )
                return false;
        }
        // ------------------------------------
        // Grab the data for the current row.
        //
        const QVariant dataTxnId            = pMsgModel->data(indexTxnId);
        const QVariant dataTxnIdDisplay     = pMsgModel->data(indexTxnIdDisplay);
        const QVariant dataMyNym            = pMsgModel->data(indexMyNym);
        const QVariant dataMyAcct           = pMsgModel->data(indexMyAcct);
        const QVariant dataAssetType        = pMsgModel->data(indexAssetType);
        const QVariant dataMethodType       = pMsgModel->data(indexMethodType);
        const QVariant dataSenderNym        = pMsgModel->data(indexSenderNym);
        const QVariant dataRecipientNym     = pMsgModel->data(indexRecipNym);
        const QVariant dataSenderAcct       = pMsgModel->data(indexSenderAcct);
        const QVariant dataRecipientAcct    = pMsgModel->data(indexRecipAcct);
        const QVariant dataSenderAddress    = pMsgModel->data(indexSenderAddr);
        const QVariant dataRecipientAddress = pMsgModel->data(indexRecipAddr);
        const QVariant dataNotaryID         = pMsgModel->data(indexNotary);
        const QVariant dataMemo             = pMsgModel->data(indexMemo);
        const QVariant dataDescription      = pMsgModel->data(indexDescription);

//        const QVariant dataSenderName       = this->data(mapFromSource(indexSenderNym));
//        const QVariant dataRecipientName    = this->data(mapFromSource(indexRecipNym));
//        const QVariant dataNotaryName       = this->data(mapFromSource(indexNotary));

        const int64_t lTxnId               = dataTxnId.isValid() ? dataTxnId.toLongLong() : 0;
        const int64_t lTxnIdDisplay        = dataTxnIdDisplay.isValid() ? dataTxnIdDisplay.toLongLong() : 0;
        const QString qstrTxnId            = lTxnId        > 0 ? QString::fromStdString(opentxs::OTAPI_Wrap::It()->LongToString(lTxnId       )) : "";
        const QString qstrTxnIdDisplay     = lTxnIdDisplay > 0 ? QString::fromStdString(opentxs::OTAPI_Wrap::It()->LongToString(lTxnIdDisplay)) : "";
        const QString qstrMyNym            = dataMyNym.isValid() ? dataMyNym.toString() : "";
        const QString qstrMyAcct           = dataMyAcct.isValid() ? dataMyAcct.toString() : "";
        const QString qstrAssetType        = dataAssetType.isValid() ? dataAssetType.toString() : "";
        const QString qstrMethodType       = dataMethodType.isValid() ? dataMethodType.toString() : "";
        const QString qstrSenderNym        = dataSenderNym.isValid() ? dataSenderNym.toString() : "";
        const QString qstrRecipientNym     = dataRecipientNym.isValid() ? dataRecipientNym.toString() : "";
        const QString qstrSenderAcct       = dataSenderAcct.isValid() ? dataSenderAcct.toString() : "";
        const QString qstrRecipientAcct    = dataRecipientAcct.isValid() ? dataRecipientAcct.toString() : "";
        const QString qstrSenderAddress    = dataSenderAddress.isValid() ? dataSenderAddress.toString() : "";
        const QString qstrRecipientAddress = dataRecipientAddress.isValid() ? dataRecipientAddress.toString() : "";
        const QString qstrNotaryID         = dataNotaryID.isValid() ? dataNotaryID.toString() : "";
        // ------------------------------------
        // Here we check filterAccount_, which is for the account details screen.
        // So we want to filter for records that match filterAccount_ to qstrMyAcct.
        if (!qstrMyAcct.contains(filterAccount_))
            return false;
        // ------------------------------------
        // Here we check the filterString (optional string the user can type.)
        //
        if (!filterString_.isEmpty())
        {
            const QString qstrMemo        = dataMemo       .isValid() ? dataMemo       .toString() : "";
            const QString qstrDescription = dataDescription.isValid() ? dataDescription.toString() : "";

            const QString qstrNotaryName = qstrNotaryID.isEmpty() ? QString("") :
                                           QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetServer_Name(qstrNotaryID.toStdString()));

            const QString qstrMyAcctName = qstrMyAcct.isEmpty() ? QString("") :
                                           QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAccountWallet_Name(qstrMyAcct.toStdString()));
            const QString qstrAssetName = qstrAssetType.isEmpty() ? QString("") :
                                           QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAssetType_Name(qstrAssetType.toStdString()));

            MTNameLookupQT theLookup;
            QString qstrMyName        = qstrMyNym       .isEmpty() ? "" : QString::fromStdString(theLookup.GetNymName(qstrMyNym       .toStdString(), ""));
            QString qstrSenderName    = qstrSenderNym   .isEmpty() ? "" : QString::fromStdString(theLookup.GetNymName(qstrSenderNym   .toStdString(), ""));
            QString qstrRecipientName = qstrRecipientNym.isEmpty() ? "" : QString::fromStdString(theLookup.GetNymName(qstrRecipientNym.toStdString(), ""));

            if (qstrSenderName.isEmpty() && !qstrSenderAddress.isEmpty())
                qstrSenderName = QString::fromStdString(theLookup.GetAddressName(qstrSenderAddress.toStdString()));
            if (qstrRecipientName.isEmpty() && !qstrRecipientAddress.isEmpty())
                qstrRecipientName = QString::fromStdString(theLookup.GetAddressName(qstrRecipientAddress.toStdString()));

            if (qstrSenderName.isEmpty() && !qstrSenderAcct.isEmpty())
                qstrSenderName = QString::fromStdString(theLookup.GetAcctName(qstrSenderAcct.toStdString(), "", "", ""));
            if (qstrRecipientName.isEmpty() && !qstrRecipientAcct.isEmpty())
                qstrRecipientName = QString::fromStdString(theLookup.GetAcctName(qstrRecipientAcct.toStdString(), "", "", ""));

            if (qstrMyName.isEmpty() && !qstrMyAcctName.isEmpty())
                qstrMyName = qstrMyAcctName;

            if (!qstrTxnId.contains(filterString_) &&
                !qstrTxnIdDisplay.contains(filterString_) &&
                !qstrMemo.contains(filterString_) &&
                !qstrDescription.contains(filterString_) &&
                !qstrMethodType.contains(filterString_) &&
                !qstrMyNym.contains(filterString_) &&
                !qstrSenderNym.contains(filterString_) &&
                !qstrRecipientNym.contains(filterString_) &&
                !qstrMyAcct.contains(filterString_) &&
                !qstrSenderAcct.contains(filterString_) &&
                !qstrRecipientAcct.contains(filterString_) &&
                !qstrSenderAddress.contains(filterString_) &&
                !qstrRecipientAddress.contains(filterString_) &&
                !qstrAssetType.contains(filterString_) &&
                !qstrNotaryID.contains(filterString_) &&
                !qstrMyName.contains(filterString_) &&
                !qstrMyAcctName.contains(filterString_) &&
                !qstrSenderName.contains(filterString_) &&
                !qstrRecipientName.contains(filterString_) &&
                !qstrAssetName.contains(filterString_) &&
                !qstrNotaryName.contains(filterString_) )
                return false;
        }
    }

    return true;
}

// --------------------------------------------

QVariant AccountRecordsProxyModel::rawData ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
    return QSortFilterProxyModel::data(index,role);
}

// ------------------------------------------------------------



// ------------------------------------------------------------

PaymentsProxyModel::PaymentsProxyModel(QObject *parent /*=0*/)
: QSortFilterProxyModel(parent)
{
}

void PaymentsProxyModel::setFilterFolder(int nFolder)
{
    nFolder_ = nFolder; // 0 for outbox, 1 for inbox, and 2+ for all other future custom boxes.
    invalidateFilter();
}

void PaymentsProxyModel::setFilterNone()
{
    clearFilterType();
    filterType_ = FilterNone;
    // --------------------------------
    invalidateFilter();
}

void PaymentsProxyModel::setFilterTopLevel(int nContactID)
{
    clearFilterType();
    filterType_ = FilterTopLevel;
    // --------------------------------
    MTContactHandler::getInstance()->GetNyms(mapNymIds_, nContactID);
    MTContactHandler::getInstance()->GetAddressesByContact(mapAddresses_, nContactID, "", false);
    // --------------------------------
    invalidateFilter();
}

void PaymentsProxyModel::setFilterNotary(QString qstrNotaryId, int nContactID)
{
    clearFilterType();
    filterType_ = FilterNotary;
    // --------------------------------
    notaryId_ = qstrNotaryId;
    MTContactHandler::getInstance()->GetNyms(mapNymIds_, nContactID);
    // --------------------------------
    invalidateFilter();
}

void PaymentsProxyModel::setFilterMethodAddress(QString qstrMethodType, QString qstrAddress)
{
    clearFilterType();
    filterType_ = FilterMethodAddress;
    // --------------------------------
    singleMethodType_ = qstrMethodType;
    singleAddress_    = qstrAddress;
    // --------------------------------
    invalidateFilter();
}

void PaymentsProxyModel::setFilterString(QString qstrFilter)
{
    filterString_ = qstrFilter;
    invalidateFilter();
}

void PaymentsProxyModel::clearFilterType()
{
    filterType_       = FilterNone;
    // --------------------------------
    notaryId_         = QString("");
    singleMethodType_ = QString("");
    singleAddress_    = QString("");
    // --------------------------------
    mapNymIds_.clear();
    mapAddresses_.clear();
}


QWidget * PaymentsProxyModel::CreateDetailHeaderWidget(const int nSourceRow, bool bExternal/*=true*/) const
{
    QModelIndex sourceIndex_Folder         = sourceModel()->index(nSourceRow, PMNT_SOURCE_COL_FOLDER);
    QModelIndex sourceIndex_AssetId        = sourceModel()->index(nSourceRow, PMNT_SOURCE_COL_MY_ASSET_TYPE);
    QModelIndex sourceIndex_Date           = sourceModel()->index(nSourceRow, PMNT_SOURCE_COL_TIMESTAMP);
    QModelIndex sourceIndex_Desc           = sourceModel()->index(nSourceRow, PMNT_SOURCE_COL_DESCRIPTION);
    QModelIndex sourceIndex_RecordName     = sourceModel()->index(nSourceRow, PMNT_SOURCE_COL_RECORD_NAME);
    QModelIndex sourceIndex_InstrumentType = sourceModel()->index(nSourceRow, PMNT_SOURCE_COL_INSTRUMENT_TYPE);
    QModelIndex sourceIndex_Amount         = sourceModel()->index(nSourceRow, PMNT_SOURCE_COL_AMOUNT);
    QModelIndex sourceIndex_PendingFound   = sourceModel()->index(nSourceRow, PMNT_SOURCE_COL_PENDING_FOUND);
    QModelIndex sourceIndex_CompletedFound = sourceModel()->index(nSourceRow, PMNT_SOURCE_COL_COMPLETED_FOUND);
    QModelIndex sourceIndex_Flags          = sourceModel()->index(nSourceRow, PMNT_SOURCE_COL_FLAGS);

    QVariant    sourceData_Folder     = sourceModel()->data(sourceIndex_Folder,  Qt::DisplayRole);
    QVariant    sourceData_AssetId    = sourceModel()->data(sourceIndex_AssetId, Qt::DisplayRole);
    QVariant    sourceData_Date       = sourceModel()->data(sourceIndex_Date,    Qt::DisplayRole);
    QVariant    sourceData_Desc       = sourceModel()->data(sourceIndex_Desc,    Qt::DisplayRole);
    QVariant    sourceData_RecordName = sourceModel()->data(sourceIndex_RecordName,    Qt::DisplayRole);
    QVariant    sourceData_InstrumentType = sourceModel()->data(sourceIndex_InstrumentType,Qt::DisplayRole);
    QVariant    sourceData_Amount  = sourceModel()->data(sourceIndex_Amount,   Qt::DisplayRole);
    QVariant    sourceData_PendingFound  = sourceModel()->data(sourceIndex_PendingFound, Qt::DisplayRole);
    QVariant    sourceData_CompletedFound  = sourceModel()->data(sourceIndex_CompletedFound, Qt::DisplayRole);
    QVariant    sourceData_Flags      = sourceModel()->data(sourceIndex_Flags,  Qt::DisplayRole);

    QString     qstrAssetId     = sourceData_AssetId.isValid() ? sourceData_AssetId.toString() : "";
    QString     qstrDescription = sourceData_Desc   .isValid() ? sourceData_Desc   .toString() : "";
    QString     qstrRecordName  = sourceData_RecordName       .isValid() ? sourceData_RecordName    .toString() : "";
    QString     qstrInstrumentType = sourceData_InstrumentType.isValid() ? sourceData_InstrumentType.toString() : "";

    ModelPayments::PaymentFlags flags = sourceData_Flags.isValid() ? (ModelPayments::PaymentFlag)sourceData_Flags.toLongLong()
                                                                  : ModelPayments::NoFlags;

    const int64_t lAmount = sourceData_Amount.isValid() ? sourceData_Amount.toLongLong() : 0;

    time64_t the_time = sourceData_Date.isValid() ? sourceData_Date.toLongLong() : 0;
    QDateTime timestamp;
    timestamp.setTime_t(the_time);
    QString qstrTimestamp = QString(timestamp.toString(Qt::SystemLocaleShortDate));

    const bool bIsOutgoing     = flags.testFlag(ModelPayments::IsOutgoing);
    const bool bIsPending      = flags.testFlag(ModelPayments::IsPending);

//  const bool bIsOutgoing     = sourceData_Folder.isValid()         ? (sourceData_Folder.toInt() == 0)   : false;
//  const bool bPendingFound   = sourceData_PendingFound.isValid()   ? sourceData_PendingFound.toBool()   : false;
//  const bool bCompletedFound = sourceData_CompletedFound.isValid() ? sourceData_CompletedFound.toBool() : false;
//  const bool bIsPending = (bPendingFound && !bCompletedFound);

    
//    qDebug() << "bIsOutgoing: " << (bIsOutgoing ? QString("true") :  QString("false"))
//    << " bPendingFound: " << (bPendingFound ? QString("true") :  QString("false")) <<
//    " bCompletedFound: " << (bCompletedFound ? QString("true") :  QString("false")) <<
//    " bIsPending: " << (bIsPending ? QString("true") :  QString("false")) << "\n";
    
    
    TransactionTableViewCellType cellType = (bIsOutgoing ?
                                                 // -------------------------------------------------
                                                 (bIsPending ?
                                                      TransactionTableViewCellTypeOutgoing :  // outgoing
                                                      TransactionTableViewCellTypeSent) :     // sent
                                                 // -------------------------------------------------
                                                 (bIsPending ?
                                                      TransactionTableViewCellTypeIncoming :  // incoming
                                                      TransactionTableViewCellTypeReceived)); // received
    // --------------------------------------------------------------------------------------------
    // For invoices and invoice receipts.
    //

    const bool bIsInvoice       = flags.testFlag(ModelPayments::IsInvoice);
    const bool bIsPlan          = flags.testFlag(ModelPayments::IsPaymentPlan);
    const bool bIsNotice        = flags.testFlag(ModelPayments::IsNotice);
    const bool bIsChequeReceipt = (0 == qstrInstrumentType.compare("chequeReceipt"));
    const bool bIsMarketReceipt = (0 == qstrInstrumentType.compare("marketReceipt"));

    if (bIsInvoice || bIsPlan || bIsNotice ||
        ( bIsChequeReceipt &&
          (( bIsOutgoing && lAmount > 0) ||
           (!bIsOutgoing && lAmount < 0))
        ) )
        cellType = (bIsOutgoing ?
                    (bIsPending ?
                     TransactionTableViewCellTypeIncoming  : // outgoing
                     TransactionTableViewCellTypeReceived) : // sent
                    // -------------------------------------------------
                    (bIsPending ?
                     TransactionTableViewCellTypeOutgoing  : // incoming
                     TransactionTableViewCellTypeSent));     // received
    // --------------------------------------------------------------------------------------------
    if (bIsMarketReceipt)
    {
        cellType = (lAmount > 0) ? TransactionTableViewCellTypeReceived : TransactionTableViewCellTypeSent;
    }
    // --------------------------------------------------------------------------------------------
    std::string str_desc = qstrDescription.toStdString();
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
            OT_FAIL_MSG("Expected all cell types to be handled for color.");
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
    QString tx_name = qstrRecordName;

    if (tx_name.trimmed() == "")
    {
        //Tx has no name
        tx_name.clear();
        tx_name = "Receipt";
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

    if ( !qstrAssetId.isEmpty() )
    {
        str_formatted = opentxs::OTAPI_Wrap::It()->FormatAmount(qstrAssetId.toStdString(), lAmount);
        bFormatted = !str_formatted.empty();
    }
    // ----------------------------------------
    if (bFormatted)
        currency_amount = QString::fromStdString(str_formatted);
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
    QLabel * row_content_date_label = new QLabel;
    QString row_content_date_label_string;
    row_content_date_label_string.append(qstrTimestamp);

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
    row_widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    return row_widget;
}

QVariant PaymentsProxyModel::data ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
//    if (role == Qt::TextAlignmentRole)
//        return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
    // ----------------------------------------
    if ( role==Qt::SizeHintRole && index.isValid())
    {
//      const int nSourceRow    = headerData(index.row(),    Qt::Vertical,   Qt::UserRole).toInt();
        const int nSourceColumn = headerData(index.column(), Qt::Horizontal, Qt::UserRole).toInt();

        if (PMNT_SOURCE_COL_PMNT_ID == nSourceColumn)
        {
            return QVariant::fromValue(QSize(300, 60));
        }
    }
    // ----------------------------------------
    if ( role==Qt::FontRole && index.isValid())
    {
        const int nSourceRow    = headerData(index.row(),    Qt::Vertical,   Qt::UserRole).toInt();
        const int nSourceColumn = PMNT_SOURCE_COL_HAVE_READ; // If you haven't read this Payment, we set it bold.

        QModelIndex sourceIndex = sourceModel()->index(nSourceRow, nSourceColumn);
        QVariant    sourceData  = sourceModel()->data(sourceIndex, Qt::DisplayRole);

        if (sourceData.isValid() && (!sourceData.toBool()))
        {
            QFont boldFont;
            boldFont.setBold(true);
            return boldFont;
        }
    }
    // ----------------------------------------
    else if ( role==Qt::DisplayRole && index.isValid() &&
        QSortFilterProxyModel::data(index,role).isValid())
    {
        const int nSourceRow    = headerData(index.row(),    Qt::Vertical,   Qt::UserRole).toInt();
        const int nSourceColumn = headerData(index.column(), Qt::Horizontal, Qt::UserRole).toInt();

        QModelIndex sourceIndex = sourceModel()->index(nSourceRow, nSourceColumn);
        QVariant    sourceData  = sourceModel()->data(sourceIndex, role);

        if (nSourceColumn == PMNT_SOURCE_COL_PMNT_ID) // payment_id
        {
            if (nullptr != pTableView_)
            {
                if (nullptr == pTableView_->indexWidget(index))
                {
                    QWidget * pWidget = CreateDetailHeaderWidget(nSourceRow);
                    pTableView_->setIndexWidget(index, pWidget);
                }
            }
            return QVariant();
        }
        else
        if (nSourceColumn == PMNT_SOURCE_COL_MY_NYM) // my_nym_id
        {
            QString qstrID = sourceData.isValid() ? sourceData.toString() : "";

            std::string str_name;

            if (!qstrID.isEmpty())
            {
                const std::string str_id = qstrID.trimmed().toStdString();
                str_name = str_id.empty() ? "" : opentxs::OTAPI_Wrap::It()->GetNym_Name(str_id);
            }
            // ------------------------
            if (str_name.empty() && !qstrID.isEmpty())
                return QVariant(qstrID);
            return QVariant(QString::fromStdString(str_name));
        }
        else if (nSourceColumn == PMNT_SOURCE_COL_MY_ACCT) // my_acct_id
        {
            QString qstrID = sourceData.isValid() ? sourceData.toString() : "";

            std::string str_name;

            if (!qstrID.isEmpty())
            {
                const std::string str_id = qstrID.trimmed().toStdString();
                str_name = str_id.empty() ? "" : opentxs::OTAPI_Wrap::It()->GetAccountWallet_Name(str_id);
            }
            // ------------------------
            if (str_name.empty() && !qstrID.isEmpty())
                return QVariant(qstrID);
            return QVariant(QString::fromStdString(str_name));
        }
        else if (nSourceColumn == PMNT_SOURCE_COL_TXN_ID) // txn_id
        {
            QModelIndex sibling = sourceIndex.sibling(sourceIndex.row(), PMNT_SOURCE_COL_TXN_ID_DISPLAY);

            int64_t lID         = sourceData.isValid() ? sourceData.toLongLong() : 0;
            int64_t lDisplayID  = sourceModel()->data(sibling,role).isValid() ? sourceModel()->data(sibling,role).toLongLong() : 0;
            // ------------------------
            if (lDisplayID > 0)
                return QVariant::fromValue(lDisplayID);
            return QVariant::fromValue(lID);
        }
        else if (nSourceColumn == PMNT_SOURCE_COL_SENDER_NYM) // sender_nym_id
        {
            QString qstrID = sourceData.isValid() ? sourceData.toString().trimmed() : "";
            const std::string str_id = qstrID.isEmpty() ? "" : qstrID.toStdString();
            MTNameLookupQT theLookup;
            const std::string str_name = str_id.empty() ? "" : theLookup.GetNymName(str_id, "");
            // ------------------------
            if (!str_name.empty())
                return QVariant(QString::fromStdString(str_name));

            QModelIndex sibling = sourceIndex.sibling(sourceIndex.row(), PMNT_SOURCE_COL_SENDER_ADDR);
            QString qstrAddress = sourceModel()->data(sibling,role).isValid() ? sourceModel()->data(sibling,role).toString() : QString("");

            if (!qstrAddress.isEmpty())
            {
                const int nContactID = MTContactHandler::getInstance()->GetContactByAddress(qstrAddress);

                if (nContactID > 0)
                {
                    QString qstrContactName = MTContactHandler::getInstance()->GetContactName(nContactID);

                    if (!qstrContactName.isEmpty())
                        return QVariant(qstrContactName);
                }
            }

            if (!qstrID.isEmpty())
                return QVariant(qstrID);

            if (!qstrAddress.isEmpty())
                return QVariant(qstrAddress);

            return sourceData;
        }
        else if (nSourceColumn == PMNT_SOURCE_COL_RECIP_NYM) // recipient_nym_id
        {
            QString qstrID = sourceData.isValid() ? sourceData.toString().trimmed() : "";
            const std::string str_id = qstrID.isEmpty() ? "" : qstrID.toStdString();
            MTNameLookupQT theLookup;
            const std::string str_name = str_id.empty() ? "" : theLookup.GetNymName(str_id, "");
            // ------------------------
            if (!str_name.empty())
                return QVariant(QString::fromStdString(str_name));

            QModelIndex sibling = sourceIndex.sibling(sourceIndex.row(), PMNT_SOURCE_COL_RECIP_ADDR);
            QString qstrAddress = sourceModel()->data(sibling,role).isValid() ? sourceModel()->data(sibling,role).toString() : QString("");

            if (!qstrAddress.isEmpty())
            {
                const int nContactID = MTContactHandler::getInstance()->GetContactByAddress(qstrAddress);

                if (nContactID > 0)
                {
                    QString qstrContactName = MTContactHandler::getInstance()->GetContactName(nContactID);

                    if (!qstrContactName.isEmpty())
                        return QVariant(qstrContactName);
                }
            }

            if (!qstrID.isEmpty())
                return QVariant(qstrID);

            if (!qstrAddress.isEmpty())
                return QVariant(qstrAddress);

            return sourceData;
        }
        else if (nSourceColumn == PMNT_SOURCE_COL_NOTARY_ID) // notary_id
        {
            QString qstrID = sourceData.isValid() ? sourceData.toString() : "";
            const std::string str_id = qstrID.toStdString();
            const std::string str_name = str_id.empty() ? "" : opentxs::OTAPI_Wrap::It()->GetServer_Name(str_id);
            // ------------------------
            if (!str_name.empty())
                return QVariant(QString::fromStdString(str_name));

            if (!qstrID.isEmpty())
                return QVariant(qstrID);

            return sourceData;
        }
        else if (nSourceColumn == PMNT_SOURCE_COL_AMOUNT) // amount
        {
            QModelIndex sourceIndex_Amount         = sourceModel()->index(nSourceRow, PMNT_SOURCE_COL_AMOUNT);

            QVariant qvarAmount = sourceIndex_Amount.isValid() ? sourceModel()->data(sourceIndex_Amount) : QVariant();

            //int64_t lAmount = sourceData.isValid() ? sourceData.toLongLong() : 0;
            int64_t lAmount = qvarAmount.isValid() ? qvarAmount.toLongLong() : 0;

            QModelIndex sibling   = sourceIndex.sibling(sourceIndex.row(), PMNT_SOURCE_COL_MY_ASSET_TYPE);
            QString qstrAssetType = sourceModel()->data(sibling,role).isValid() ? sourceModel()->data(sibling,role).toString() : QString("");

            QString qstrAmount = QString::fromStdString(opentxs::OTAPI_Wrap::It()->LongToString(lAmount));

            if (!qstrAssetType.isEmpty())
            {
                QString qstrTemp = QString::fromStdString(opentxs::OTAPI_Wrap::It()->FormatAmount(qstrAssetType.toStdString(), lAmount));
                if (!qstrTemp.isEmpty())
                    qstrAmount = qstrTemp;
            }
            // ----------------------------------------------
            return QVariant(qstrAmount);
        }
        else if (nSourceColumn == PMNT_SOURCE_COL_MY_ASSET_TYPE) // my_asset_type_id
        {
            QString qstrID = sourceData.isValid() ? sourceData.toString() : "";
            const std::string str_id = qstrID.toStdString();
            const std::string str_name = str_id.empty() ? "" : opentxs::OTAPI_Wrap::It()->GetAssetType_Name(str_id);
            // ------------------------
            if (!str_name.empty())
                return QVariant(QString::fromStdString(str_name));

            if (!qstrID.isEmpty())
                return QVariant(qstrID);

            return sourceData;
        }
        else if (nSourceColumn == PMNT_SOURCE_COL_METHOD_TYPE_DISP) // Method type display (if blank, we see if there's a notary ID. If so, then transport shows "otserver".)
        {
            QString qstrType = sourceData.isValid() ? sourceData.toString() : "";
            // ------------------------
            if (!qstrType.isEmpty())
                return QVariant(qstrType);
            return QVariant(QString("opentxs"));
        }
        else if (nSourceColumn == PMNT_SOURCE_COL_TIMESTAMP) // timestamp
        {
            time64_t the_time = sourceData.isValid() ? sourceData.toLongLong() : 0;
            QDateTime timestamp;
            timestamp.setTime_t(the_time);
            return QVariant(QString(timestamp.toString(Qt::SystemLocaleShortDate)));
        }
        else if (nSourceColumn == PMNT_SOURCE_COL_FOLDER) // folder
        {
            int nFolder = sourceData.isValid() ? sourceData.toInt() : 0;
            return QVariant(tr(0 == nFolder ? "Sent" : "Receieved"));
        }
    }
    // -------------------------------
    else if (role==Qt::DecorationRole && index.isValid())
    {
        const int nSourceRow             = headerData(index.row(),    Qt::Vertical,   Qt::UserRole).toInt();
        const int nSourceColumn          = headerData(index.column(), Qt::Horizontal, Qt::UserRole).toInt();

        const int nSourceColumnReplied   = PMNT_SOURCE_COL_HAVE_REPLIED;
        const int nSourceColumnForwarded = PMNT_SOURCE_COL_HAVE_FORWARDED;

        QModelIndex sourceIndexReplied   = sourceModel()->index(nSourceRow, nSourceColumnReplied);
        QModelIndex sourceIndexForwarded = sourceModel()->index(nSourceRow, nSourceColumnForwarded);

        QVariant    sourceDataReplied    = sourceModel()->data(sourceIndexReplied,   Qt::DisplayRole);
        QVariant    sourceDataForwarded  = sourceModel()->data(sourceIndexForwarded, Qt::DisplayRole);

        if (nSourceColumn == PMNT_SOURCE_COL_MEMO)
        {
            const bool bHaveReplied   = sourceDataReplied.isValid() ? sourceDataReplied.toBool() : false;
            const bool bHaveForwarded = sourceDataForwarded.isValid() ? sourceDataForwarded.toBool() : false;
            const bool bHaveBoth      = (bHaveReplied && bHaveForwarded);

            if (bHaveBoth)
            {
                QPixmap pixmap(":/icons/icons/replied_forwarded.png");
                return pixmap;
            }
            else if (bHaveReplied)
            {
                QPixmap pixmap(":/icons/icons/replied.png");
                return pixmap;
            }
            if (bHaveForwarded)
            {
                QPixmap pixmap(":/icons/icons/forwarded.png");
                return pixmap;
            }
        }
    }
    // --------------------------------------------
    return QSortFilterProxyModel::data(index,role);
}


QVariant PaymentsProxyModel::headerData(int section, Qt::Orientation orientation, int role/*=Qt::DisplayRole*/) const
{
    QVariant varHeaderData = QSortFilterProxyModel::headerData(section, orientation, role);

    if (Qt::Horizontal == orientation && Qt::DisplayRole == role)
    {
        QString qstrHeader = varHeaderData.toString();

        if (0 == qstrHeader.compare(tr("Me")))
        {
            if (0 == nFolder_) // sent box
                return QVariant(tr("From"));
            else // received box
                return QVariant(tr("To"));
        }
        else if (0 == qstrHeader.compare(tr("Details")))
        {
            if (0 == nFolder_) // sent box
                return QVariant(tr("To"));
            else // received box
                return QVariant(tr("From"));
        }
    }

    return varHeaderData;
}

// --------------------------------------------

bool PaymentsProxyModel::filterAcceptsColumn(int source_column, const QModelIndex & source_parent) const
{
    bool bReturn = true;

    switch (source_column)
    {
    case PMNT_SOURCE_COL_PMNT_ID:   bReturn = true;  break; // payment_id
    case PMNT_SOURCE_COL_MY_NYM:  // my_nym_id
    {
        // NOTE: We'll hide this column, where necessary, using the tableView.
        // That way we still have access to the data when we need it.
        // So as far as the model is concerned, this column is always shown.
        bReturn = true;
//        if (FilterNone == filterType_)
//           bReturn = true;
//        else
//           bReturn = false;
    }
        break;
    case PMNT_SOURCE_COL_MY_ACCT:
    {
        bReturn = true;
    }
        break;
    case PMNT_SOURCE_COL_MY_ASSET_TYPE:
    {
        bReturn = true;
    }
        break;
    case PMNT_SOURCE_COL_MY_ADDR:
    {
        bReturn = false;
//        if (FilterNotary == filterType_)
//            bReturn = false;
//        else if (FilterNone == filterType_)
//            bReturn = true;
//        else
//            bReturn = false;
    }
        break;
    case PMNT_SOURCE_COL_SENDER_NYM:
    {
        if (nFolder_ == 0) // 0 is the outbox. (In the outbox, you don't show the sender, but the recipient. So we hide the sender column.)
            return false;
        else
            bReturn = true;
    }
        break;
    case PMNT_SOURCE_COL_SENDER_ACCT:
    {
        bReturn = false;
//        if (nFolder_ == 0) // 0 is the outbox. (In the outbox, you don't show the sender, but the recipient. So we hide the sender column.)
//            return false;
//        else
//            bReturn = true;
    }
        break;

    case PMNT_SOURCE_COL_SENDER_ADDR:
    {
        if (FilterNotary == filterType_) // If Payment was sent through a notary, then it DEFINITELY won't have a Bitmessage address.
            bReturn = false;
        else if (nFolder_ == 0) // 0 is the outbox. (In the outbox, you don't show the sender, but the recipient. So we hide the sender column.)
            return false;
        else
            bReturn = false; // Current expected behavior.
    }
        break;
    case PMNT_SOURCE_COL_RECIP_NYM:
    {
        if (nFolder_ > 0) // All other boxes except outbox, hide recipient. (We normally see a recipient in the outbox, instead of a sender.)
            return false;
        else
            bReturn = true;
    }
        break;
    case PMNT_SOURCE_COL_RECIP_ACCT:
    {
        bReturn = false;
//        if (nFolder_ > 0) // All other boxes except outbox, hide recipient. (We normally see a recipient in the outbox, instead of a sender.)
//            return false;
//        else
//            bReturn = true;
    }
        break;

    case PMNT_SOURCE_COL_RECIP_ADDR:
    {
        if (FilterNotary == filterType_) // If Payment was sent through a notary, then it DEFINITELY won't have a Bitmessage address.
            bReturn = false;
        else if (nFolder_ > 0) // All other boxes except outbox, hide recipient. (We normally see a recipient in the outbox, instead of a sender.)
            return false;
        else
            bReturn = false; // Current expected behavior.
    }
        break;
    case PMNT_SOURCE_COL_METHOD_TYPE:      bReturn = false;  break;
    case PMNT_SOURCE_COL_METHOD_TYPE_DISP:
    {
        if (FilterNone == filterType_)
            bReturn = true;
        else if (FilterTopLevel == filterType_)
            bReturn = true;
        else
            bReturn = false;
    }
        break;
    case PMNT_SOURCE_COL_NOTARY_ID:
    {
        if (FilterMethodAddress == filterType_)
            bReturn = false;
        else if (FilterNotary == filterType_)
            bReturn = false;
        else
            bReturn = true;
    }
        break;
    case PMNT_SOURCE_COL_TXN_ID:          bReturn = true;   break;
    case PMNT_SOURCE_COL_TXN_ID_DISPLAY:  bReturn = false;  break;
    case PMNT_SOURCE_COL_AMOUNT:          bReturn = true;   break;
    case PMNT_SOURCE_COL_PENDING_FOUND:   bReturn = false;  break;
    case PMNT_SOURCE_COL_COMPLETED_FOUND: bReturn = false;  break;
    case PMNT_SOURCE_COL_TIMESTAMP:       bReturn = true;   break;
    case PMNT_SOURCE_COL_HAVE_READ:       bReturn = false;  break;
    case PMNT_SOURCE_COL_HAVE_REPLIED:    bReturn = false;  break;
    case PMNT_SOURCE_COL_HAVE_FORWARDED:  bReturn = false;  break;
    case PMNT_SOURCE_COL_MEMO:            bReturn = true;   break;
    case PMNT_SOURCE_COL_DESCRIPTION:     bReturn = false;  break;
    case PMNT_SOURCE_COL_RECORD_NAME:     bReturn = false;  break;
    case PMNT_SOURCE_COL_FOLDER:          bReturn = false;  break;
    case PMNT_SOURCE_COL_FLAGS:           bReturn = false;  break;
    default:  bReturn = true;  break;
    }
    return bReturn;
}


// What do we filter on here?
// Well, the Payment table doesn't contain a Contact ID.
// Instead, it contains a NymID and/or a Bitmessage address and/or a notary address.
//
// When the user clicks on a top-level Contact ID, we need to filter on ALL known Nyms AND Bitmessage addresses for that Contact.
// Any single one of them could be the one on the Payment, that causes the filter to approve it.
// Notice in that case they're not ALL required -- just ONE of them.
//
// Next let's say a user clicks on a contact's Bitmessage address. In that case it's easy -- we can just filter
// based on that Bitmessage address.
//
// Next let's say a user clicks on a contact's Notary. In that case we need to filter on ALL known Nyms for that contact
// AND we need to filter on the notary--and the notary is part of the filter.
//
// That means there are 3 main filter types:
// 1. otserver    -- must pass NotaryID as well as ContactID (that's so we can look up the Nym list for that contact.) Requirement is "any Nym from that list, plus the notary."
// 2. method_type -- must pass the method type and the address. That's the requirement, too (both.)
// 3. top-level   -- contact ID is only thing passed. Look up ALL Nyms and Addresses for that contact. Any of them is good enough for a match.
//
bool PaymentsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex indexTxnId        = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_TXN_ID,         sourceParent); // txn_id
    QModelIndex indexTxnIdDisplay = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_TXN_ID_DISPLAY, sourceParent); // txn_id_display
    QModelIndex indexMyNym        = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_MY_NYM,         sourceParent); // my_nym_id
    QModelIndex indexMyAcct       = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_MY_ACCT,        sourceParent); // my_acct_id
    QModelIndex indexAssetType    = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_MY_ASSET_TYPE,  sourceParent); // my_asset_type_id
    QModelIndex indexSenderNym    = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_SENDER_NYM,     sourceParent); // sender_nym_id
    QModelIndex indexSenderAcct   = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_SENDER_ACCT,    sourceParent); // sender_acct_id
    QModelIndex indexSenderAddr   = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_SENDER_ADDR,    sourceParent); // sender_address
    QModelIndex indexRecipNym     = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_RECIP_NYM,      sourceParent); // recipient_nym_id
    QModelIndex indexRecipAcct    = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_RECIP_ACCT,     sourceParent); // recipient_acct_id
    QModelIndex indexRecipAddr    = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_RECIP_ADDR,     sourceParent); // recipient_address
    QModelIndex indexMethodType   = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_METHOD_TYPE,    sourceParent); // method_type
    QModelIndex indexNotary       = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_NOTARY_ID,      sourceParent); // notary_id
    QModelIndex indexFolder       = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_FOLDER,         sourceParent); // folder
    QModelIndex indexMemo         = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_MEMO,           sourceParent); // memo
    QModelIndex indexDescription  = sourceModel()->index(sourceRow, PMNT_SOURCE_COL_DESCRIPTION,    sourceParent); // description

    QAbstractItemModel * pModel    = sourceModel();
    ModelPayments      * pMsgModel = dynamic_cast<ModelPayments*>(pModel);

    if (nullptr != pMsgModel)
    {
        // Check the folder here.
        QVariant dataFolder = pMsgModel->data(indexFolder);
        const int nFolder = dataFolder.isValid() ? dataFolder.toInt() : (-1);

        if ((nFolder != -1)  && (nFolder != nFolder_))
            return false;
        // ------------------------------------
        // Grab the data for the current row.
        //
        const QVariant dataTxnId            = pMsgModel->data(indexTxnId);
        const QVariant dataTxnIdDisplay     = pMsgModel->data(indexTxnIdDisplay);
        const QVariant dataMyNym            = pMsgModel->data(indexMyNym);
        const QVariant dataMyAcct           = pMsgModel->data(indexMyAcct);
        const QVariant dataAssetType        = pMsgModel->data(indexAssetType);
        const QVariant dataMethodType       = pMsgModel->data(indexMethodType);
        const QVariant dataSenderNym        = pMsgModel->data(indexSenderNym);
        const QVariant dataRecipientNym     = pMsgModel->data(indexRecipNym);
        const QVariant dataSenderAcct       = pMsgModel->data(indexSenderAcct);
        const QVariant dataRecipientAcct    = pMsgModel->data(indexRecipAcct);
        const QVariant dataSenderAddress    = pMsgModel->data(indexSenderAddr);
        const QVariant dataRecipientAddress = pMsgModel->data(indexRecipAddr);
        const QVariant dataNotaryID         = pMsgModel->data(indexNotary);
        const QVariant dataMemo             = pMsgModel->data(indexMemo);
        const QVariant dataDescription      = pMsgModel->data(indexDescription);

//        const QVariant dataSenderName       = this->data(mapFromSource(indexSenderNym));
//        const QVariant dataRecipientName    = this->data(mapFromSource(indexRecipNym));
//        const QVariant dataNotaryName       = this->data(mapFromSource(indexNotary));

        const int64_t lTxnId               = dataTxnId.isValid() ? dataTxnId.toLongLong() : 0;
        const int64_t lTxnIdDisplay        = dataTxnIdDisplay.isValid() ? dataTxnIdDisplay.toLongLong() : 0;
        const QString qstrTxnId            = lTxnId        > 0 ? QString::fromStdString(opentxs::OTAPI_Wrap::It()->LongToString(lTxnId       )) : "";
        const QString qstrTxnIdDisplay     = lTxnIdDisplay > 0 ? QString::fromStdString(opentxs::OTAPI_Wrap::It()->LongToString(lTxnIdDisplay)) : "";
        const QString qstrMyNym            = dataMyNym.isValid() ? dataMyNym.toString() : "";
        const QString qstrMyAcct           = dataMyAcct.isValid() ? dataMyAcct.toString() : "";
        const QString qstrAssetType        = dataAssetType.isValid() ? dataAssetType.toString() : "";
        const QString qstrMethodType       = dataMethodType.isValid() ? dataMethodType.toString() : "";
        const QString qstrSenderNym        = dataSenderNym.isValid() ? dataSenderNym.toString() : "";
        const QString qstrRecipientNym     = dataRecipientNym.isValid() ? dataRecipientNym.toString() : "";
        const QString qstrSenderAcct       = dataSenderAcct.isValid() ? dataSenderAcct.toString() : "";
        const QString qstrRecipientAcct    = dataRecipientAcct.isValid() ? dataRecipientAcct.toString() : "";
        const QString qstrSenderAddress    = dataSenderAddress.isValid() ? dataSenderAddress.toString() : "";
        const QString qstrRecipientAddress = dataRecipientAddress.isValid() ? dataRecipientAddress.toString() : "";
        const QString qstrNotaryID         = dataNotaryID.isValid() ? dataNotaryID.toString() : "";
        // ------------------------------------
        // Here we check the filterString (optional string the user can type.)
        //
        if (!filterString_.isEmpty())
        {
            const QString qstrMemo        = dataMemo       .isValid() ? dataMemo       .toString() : "";
            const QString qstrDescription = dataDescription.isValid() ? dataDescription.toString() : "";

            const QString qstrNotaryName = qstrNotaryID.isEmpty() ? QString("") :
                                           QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetServer_Name(qstrNotaryID.toStdString()));

            const QString qstrMyAcctName = qstrMyAcct.isEmpty() ? QString("") :
                                           QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAccountWallet_Name(qstrMyAcct.toStdString()));
            const QString qstrAssetName = qstrAssetType.isEmpty() ? QString("") :
                                           QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAssetType_Name(qstrAssetType.toStdString()));

            MTNameLookupQT theLookup;
            QString qstrMyName        = qstrMyNym       .isEmpty() ? "" : QString::fromStdString(theLookup.GetNymName(qstrMyNym       .toStdString(), ""));
            QString qstrSenderName    = qstrSenderNym   .isEmpty() ? "" : QString::fromStdString(theLookup.GetNymName(qstrSenderNym   .toStdString(), ""));
            QString qstrRecipientName = qstrRecipientNym.isEmpty() ? "" : QString::fromStdString(theLookup.GetNymName(qstrRecipientNym.toStdString(), ""));

            if (qstrSenderName.isEmpty() && !qstrSenderAddress.isEmpty())
                qstrSenderName = QString::fromStdString(theLookup.GetAddressName(qstrSenderAddress.toStdString()));
            if (qstrRecipientName.isEmpty() && !qstrRecipientAddress.isEmpty())
                qstrRecipientName = QString::fromStdString(theLookup.GetAddressName(qstrRecipientAddress.toStdString()));

            if (qstrSenderName.isEmpty() && !qstrSenderAcct.isEmpty())
                qstrSenderName = QString::fromStdString(theLookup.GetAcctName(qstrSenderAcct.toStdString(), "", "", ""));
            if (qstrRecipientName.isEmpty() && !qstrRecipientAcct.isEmpty())
                qstrRecipientName = QString::fromStdString(theLookup.GetAcctName(qstrRecipientAcct.toStdString(), "", "", ""));

            if (qstrMyName.isEmpty() && !qstrMyAcctName.isEmpty())
                qstrMyName = qstrMyAcctName;

            if (!qstrTxnId.contains(filterString_) &&
                !qstrTxnIdDisplay.contains(filterString_) &&
                !qstrMemo.contains(filterString_) &&
                !qstrDescription.contains(filterString_) &&
                !qstrMethodType.contains(filterString_) &&
                !qstrMyNym.contains(filterString_) &&
                !qstrSenderNym.contains(filterString_) &&
                !qstrRecipientNym.contains(filterString_) &&
                !qstrMyAcct.contains(filterString_) &&
                !qstrSenderAcct.contains(filterString_) &&
                !qstrRecipientAcct.contains(filterString_) &&
                !qstrSenderAddress.contains(filterString_) &&
                !qstrRecipientAddress.contains(filterString_) &&
                !qstrAssetType.contains(filterString_) &&
                !qstrNotaryID.contains(filterString_) &&
                !qstrMyName.contains(filterString_) &&
                !qstrMyAcctName.contains(filterString_) &&
                !qstrSenderName.contains(filterString_) &&
                !qstrRecipientName.contains(filterString_) &&
                !qstrAssetName.contains(filterString_) &&
                !qstrNotaryName.contains(filterString_) )
                return false;
        }
        // ------------------------------------
        // Then check the other stuff:
        switch (filterType_)
        {
        case FilterNone:
            return true;

        case FilterTopLevel:
        {
//            const QVariant dataMethodType       = pMsgModel->data(indexMethodType);
//            const QVariant dataSenderNym        = pMsgModel->data(indexSenderNym);
//            const QVariant dataRecipientNym     = pMsgModel->data(indexRecipNym);
//            const QVariant dataSenderAddress    = pMsgModel->data(indexSenderAddr);
//            const QVariant dataRecipientAddress = pMsgModel->data(indexRecipAddr);

//            const QString qstrMethodType       = dataMethodType.isValid() ? dataMethodType.toString() : "";
//            const QString qstrSenderNym        = dataSenderNym.isValid() ? dataSenderNym.toString() : "";
//            const QString qstrRecipientNym     = dataRecipientNym.isValid() ? dataRecipientNym.toString() : "";
//            const QString qstrSenderAddress    = dataSenderAddress.isValid() ? dataSenderAddress.toString() : "";
//            const QString qstrRecipientAddress = dataRecipientAddress.isValid() ? dataRecipientAddress.toString() : "";

            for (mapIDName::const_iterator ii = mapNymIds_.begin(); ii != mapNymIds_.end(); ii++)
            {
                const QString qstrNymID = ii.key();

                if ( (0 == qstrNymID.compare(qstrSenderNym)) ||
                     (0 == qstrNymID.compare(qstrRecipientNym)) ) // Match!
                    return true;
            }
            for (mapIDName::const_iterator ii = mapAddresses_.begin(); ii != mapAddresses_.end(); ii++)
            {
                QString qstrAddress = ii.key();

                if ( (0 == qstrAddress.compare(qstrSenderAddress)) ||
                     (0 == qstrAddress.compare(qstrRecipientAddress)) ) // Match!
                    return true;
            }
            return false;
        }
            break;

        case FilterNotary:
        {
//            const QVariant dataNotaryID     = pMsgModel->data(indexNotary);
//            const QVariant dataSenderNym    = pMsgModel->data(indexSenderNym);
//            const QVariant dataRecipientNym = pMsgModel->data(indexRecipNym);

//            const QString  qstrNotaryID     = dataNotaryID.isValid() ? dataNotaryID.toString() : "";
//            const QString  qstrSenderNym    = dataSenderNym.isValid() ? dataSenderNym.toString() : "";
//            const QString  qstrRecipientNym = dataRecipientNym.isValid() ? dataRecipientNym.toString() : "";

            if (!notaryId_.isEmpty() && (qstrNotaryID != notaryId_)) // The row doesn't match our filter.
                return false;

            for (mapIDName::const_iterator ii = mapNymIds_.begin(); ii != mapNymIds_.end(); ii++)
            {
                const QString qstrNymID = ii.key();

                if ( (0 == qstrNymID.compare(qstrSenderNym)) ||
                     (0 == qstrNymID.compare(qstrRecipientNym)) ) // Match!
                    return true;
            }
            return false;
        }
            break;

        case FilterMethodAddress:
        {
//            const QVariant dataMethodType       = pMsgModel->data(indexMethodType);
//            const QVariant dataSenderAddress    = pMsgModel->data(indexSenderAddr);
//            const QVariant dataRecipientAddress = pMsgModel->data(indexRecipAddr);

//            const QString qstrMethodType       = dataMethodType.isValid() ? dataMethodType.toString() : "";
//            const QString qstrSenderAddress    = dataSenderAddress.isValid() ? dataSenderAddress.toString() : "";
//            const QString qstrRecipientAddress = dataRecipientAddress.isValid() ? dataRecipientAddress.toString() : "";

            if (!singleMethodType_.isEmpty() && (qstrMethodType != singleMethodType_)) // The row doesn't match our filter.
                return false;
            if (!singleAddress_.isEmpty())
            {
                if ((qstrSenderAddress    != singleAddress_) &&
                    (qstrRecipientAddress != singleAddress_))
                return false;
            }
        }
            break;

        default:
            qDebug() << "Unexpected filter type in PaymentsProxyModel::filterAcceptsRow.\n";
            return false;
        }
    }

    return true;
}

// --------------------------------------------

QVariant PaymentsProxyModel::rawData ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
    return QSortFilterProxyModel::data(index,role);
}

// ------------------------------------------------------------

ModelPayments::ModelPayments(QObject * parent /*= 0*/, QSqlDatabase db /*=QSqlDatabase()*/)
: QSqlTableModel(parent, db) {}


// I'm overriding this so I can return the ACTUAL row or column back (depending on orientation) from the source
// model. This way, the proxy model can call this to find out the actual column or row, whenever it needs to.
//
QVariant ModelPayments::headerData(int section, Qt::Orientation orientation, int role /*=Qt::DisplayRole*/) const
{
    if (role==Qt::UserRole)
    {
        return section;
    }

    return QSqlTableModel::headerData(section, orientation, role);
}


QVariant ModelPayments::rawData ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
    return QSqlTableModel::data(index,role);
}


bool ModelPayments::setData(const QModelIndex & index, const QVariant & value, int role /*=Qt::EditRole*/)
{
    bool success = false;

    if (role==Qt::DisplayRole && value.isValid())
    {
//        if (index.column() == PMNT_SOURCE_COL_MY_ADDR) // my_address
//        {
//            success = QSqlTableModel::setData(index,QVariant(MTContactHandler::Encode(value.toString())),role);
//        }
//        else if (index.column() == PMNT_SOURCE_COL_SENDER_ADDR) // sender_address
//        {
//            success = QSqlTableModel::setData(index,QVariant(MTContactHandler::Encode(value.toString())),role);
//        }
//        else if (index.column() == PMNT_SOURCE_COL_RECIP_ADDR) // recipient_address
//        {
//            success = QSqlTableModel::setData(index,QVariant(MTContactHandler::Encode(value.toString())),role);
//        }
//        else if (index.column() == PMNT_SOURCE_COL_METHOD_TYPE) // method_type
//        {
//            success = QSqlTableModel::setData(index,QVariant(MTContactHandler::Encode(value.toString())),role);
//        }
//        else if (index.column() == PMNT_SOURCE_COL_METHOD_TYPE_DISP) // method_type_display
//        {
//            success = QSqlTableModel::setData(index,QVariant(MTContactHandler::Encode(value.toString())),role);
//        }
        if (index.column() == PMNT_SOURCE_COL_RECORD_NAME) // record_name
        {
            success = QSqlTableModel::setData(index,QVariant(MTContactHandler::Encode(value.toString())),role);
        }
        if (index.column() == PMNT_SOURCE_COL_MEMO) // memo
        {
            success = QSqlTableModel::setData(index,QVariant(MTContactHandler::Encode(value.toString())),role);
        }
        if (index.column() == PMNT_SOURCE_COL_DESCRIPTION) // description
        {
            success = QSqlTableModel::setData(index,QVariant(MTContactHandler::Encode(value.toString())),role);
        }
    }

    if (!success)
        success = QSqlTableModel::setData(index,QVariant(value),role);

    if (success)
        emit dataChanged(index,index);

    return success;
}

QVariant ModelPayments::data ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
    if (role == Qt::TextAlignmentRole) // I don't want to center the memo column.
    {
        if (index.column() == PMNT_SOURCE_COL_MEMO)
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        else if (index.column() == PMNT_SOURCE_COL_DESCRIPTION)
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
//        if (index.column() == PMNT_SOURCE_COL_HAVE_REPLIED ||
//            index.column() == PMNT_SOURCE_COL_HAVE_FORWARDED)
//            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
    }
    // ----------------------------------------
    if (role==Qt::DisplayRole && QSqlTableModel::data(index,role).isValid())
    {
//        if (index.column() == PMNT_SOURCE_COL_MY_ADDR) // my_address
//        {
//            return QVariant(MTContactHandler::Decode(QSqlTableModel::data(index,role).toString()));
//        }
//        else if (index.column() == PMNT_SOURCE_COL_SENDER_ADDR) // sender_address
//        {
//            return QVariant( MTContactHandler::Decode(QSqlTableModel::data(index,role).toString()));
//        }
//        else if (index.column() == PMNT_SOURCE_COL_RECIP_ADDR) // recipient_address
//        {
//            return QVariant( MTContactHandler::Decode(QSqlTableModel::data(index,role).toString()));
//        }
//        else if (index.column() == PMNT_SOURCE_COL_METHOD_TYPE) // method_type
//        {
//            return QVariant( MTContactHandler::Decode(QSqlTableModel::data(index,role).toString()));
//        }
//        else if (index.column() == PMNT_SOURCE_COL_METHOD_TYPE_DISP) // method_type_display
//        {
//            return QVariant( MTContactHandler::Decode(QSqlTableModel::data(index,role).toString()));
//        }
        if (index.column() == PMNT_SOURCE_COL_RECORD_NAME) // record_name
        {
            QVariant qvarData    = QSqlTableModel::data(index,role);
            QString  qstrData    = qvarData.isValid() ? qvarData.toString() : "";
            QString  qstrDecoded = qstrData.isEmpty() ? "" : MTContactHandler::Decode(qstrData);
            return QVariant(qstrDecoded);
        }
        if (index.column() == PMNT_SOURCE_COL_MEMO) // memo
        {
            QVariant qvarData    = QSqlTableModel::data(index,role);
            QString  qstrData    = qvarData.isValid() ? qvarData.toString() : "";
            QString  qstrDecoded = qstrData.isEmpty() ? "" : MTContactHandler::Decode(qstrData);
            return QVariant(qstrDecoded);
        }
        if (index.column() == PMNT_SOURCE_COL_DESCRIPTION) // description
        {
            QVariant qvarData    = QSqlTableModel::data(index,role);
            QString  qstrData    = qvarData.isValid() ? qvarData.toString() : "";
            QString  qstrDecoded = qstrData.isEmpty() ? "" : MTContactHandler::Decode(qstrData);
            return QVariant(qstrDecoded);
        }
    }

    return QSqlTableModel::data(index,role);
}
