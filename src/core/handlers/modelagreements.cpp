#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/moneychanger.hpp>
#include <core/handlers/DBHandler.hpp>
#include <core/handlers/contacthandler.hpp>
#include <gui/widgets/homedetail.hpp>

#include <core/handlers/modelagreements.hpp>

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


//ModelAgreementMySigners::ModelAgreementMySigners(QObject *parent/*=0*/, QSqlDatabase db/*=QSqlDatabase()*/)
//{

//}

//QVariant ModelAgreementMySigners::data ( const QModelIndex & index, int role/*=Qt::DisplayRole*/) const
//{

//}

//QVariant ModelAgreementMySigners::rawData ( const QModelIndex & index, int role/*=Qt::DisplayRole*/) const
//{

//}

//QVariant ModelAgreementMySigners::headerData(int section, Qt::Orientation orientation, int role/*=Qt::DisplayRole*/) const
//{

//}

//bool ModelAgreementMySigners::setData(const QModelIndex & index, const QVariant & value, int role/*=Qt::EditRole*/)
//{

//}

//// ----------------------------------------------



//AgreementMySignersProxyModel::AgreementMySignersProxyModel(QObject *parent/*=0*/)
//{

//}

//void AgreementMySignersProxyModel::setFilterString(QString qstrFilter)
//{

//}

//void AgreementMySignersProxyModel::setFilterAgreementId(int nFilter)
//{

//}

//QVariant AgreementMySignersProxyModel::headerData(int section, Qt::Orientation orientation, int role/*=Qt::DisplayRole*/) const
//{

//}

//QVariant AgreementMySignersProxyModel::data    ( const QModelIndex & index, int role/*=Qt::DisplayRole*/) const
//{

//}

//QVariant AgreementMySignersProxyModel::rawData ( const QModelIndex & index, int role/*=Qt::DisplayRole*/) const
//{

//}

//bool AgreementMySignersProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
//{

//}

//bool AgreementMySignersProxyModel::filterAcceptsColumn(int source_column, const QModelIndex & source_parent) const
//{

//}






// ------------------------------------------------------------


ModelAgreements::ModelAgreements(QObject *parent/*=0*/, QSqlDatabase db/*=QSqlDatabase()*/)
    : QSqlTableModel(parent, db) {}

QVariant ModelAgreements::data ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
    if (role == Qt::TextAlignmentRole) // I don't want to center the memo column.
    {
        if (index.column() == AGRMT_SOURCE_COL_MEMO)
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
//        else if (index.column() == AGRMT_RECEIPT_COL_DESCRIPTION)
//            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
    }
    // ----------------------------------------
    if (role==Qt::DisplayRole && QSqlTableModel::data(index,role).isValid())
    {
//        if (index.column() == PMNT_SOURCE_COL_RECORD_NAME) // record_name
//        {
//            QVariant qvarData    = QSqlTableModel::data(index,role);
//            QString  qstrData    = qvarData.isValid() ? qvarData.toString() : "";
//            QString  qstrDecoded = qstrData.isEmpty() ? "" : MTContactHandler::Decode(qstrData);
//            return QVariant(qstrDecoded);
//        }
//        if (index.column() == PMNT_SOURCE_COL_DESCRIPTION) // description
//        {
//            QVariant qvarData    = QSqlTableModel::data(index,role);
//            QString  qstrData    = qvarData.isValid() ? qvarData.toString() : "";
//            QString  qstrDecoded = qstrData.isEmpty() ? "" : MTContactHandler::Decode(qstrData);
//            return QVariant(qstrDecoded);
//        }
        if (index.column() == AGRMT_SOURCE_COL_MEMO) // memo
        {
            QVariant qvarData    = QSqlTableModel::data(index,role);
            QString  qstrData    = qvarData.isValid() ? qvarData.toString() : "";
            QString  qstrDecoded = qstrData.isEmpty() ? "" : MTContactHandler::Decode(qstrData);
            return QVariant(qstrDecoded);
        }
    }

    return QSqlTableModel::data(index,role);
}

QVariant ModelAgreements::rawData ( const QModelIndex & index, int role/*=Qt::DisplayRole*/) const
{
    return QSqlTableModel::data(index,role);
}

// I'm overriding this so I can return the ACTUAL row or column back (depending on orientation) from the source
// model. This way, the proxy model can call this to find out the actual column or row, whenever it needs to.
//
QVariant ModelAgreements::headerData(int section, Qt::Orientation orientation, int role/*=Qt::DisplayRole*/) const
{
    if (role==Qt::UserRole)
    {
        return section;
    }

    return QSqlTableModel::headerData(section, orientation, role);
}

bool ModelAgreements::setData(const QModelIndex & index, const QVariant & value, int role/*=Qt::EditRole*/)
{
    bool success = false;

    if (role==Qt::DisplayRole && value.isValid())
    {
//        if (index.column() == PMNT_SOURCE_COL_RECORD_NAME) // record_name
//        {
//            success = QSqlTableModel::setData(index,QVariant(MTContactHandler::Encode(value.toString())),role);
//        }
//        if (index.column() == PMNT_SOURCE_COL_DESCRIPTION) // description
//        {
//            success = QSqlTableModel::setData(index,QVariant(MTContactHandler::Encode(value.toString())),role);
//        }
        if (index.column() == AGRMT_SOURCE_COL_MEMO) // memo
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

// ------------------------------------------------------------


AgreementsProxyModel::AgreementsProxyModel(QObject *parent/*=0*/)
    : QSortFilterProxyModel(parent) { }

void AgreementsProxyModel::setFilterFolder(int nFolder)
{
    nFolder_ = nFolder; // 0 for outbox, 1 for inbox, and 2+ for all other future custom boxes.
    invalidateFilter();
}

// ------------------------------------------------------------
QVariant AgreementsProxyModel::data ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
//  if (role == Qt::TextAlignmentRole)
//      return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
    // ----------------------------------------
    if ( role==Qt::FontRole && index.isValid())
    {
        const int nSourceRow    = headerData(index.row(),    Qt::Vertical,   Qt::UserRole).toInt();
        const int nSourceColumn = AGRMT_SOURCE_COL_HAVE_READ; // If you haven't read this Agreement, we set it bold.

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

        if (nSourceColumn == AGRMT_SOURCE_COL_NOTARY_ID) // notary_id
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
        else if (nSourceColumn == AGRMT_SOURCE_COL_TIMESTAMP) // timestamp
        {
            time64_t the_time = sourceData.isValid() ? sourceData.toLongLong() : 0;
            QDateTime timestamp;
            timestamp.setTime_t(the_time);
            return QVariant(QString(timestamp.toString(Qt::SystemLocaleShortDate)));
        }
        else if (nSourceColumn == AGRMT_SOURCE_COL_NEWEST_KNOWN_STATE) // newest known state
        {
            int nState = sourceData.isValid() ? sourceData.toInt() : 0;

            QString qstrState;

            switch(nState)
            {
            case 1:
                qstrState = tr("Paid");
                break;
            case 2:
                qstrState = tr("Payment Failed");
                break;
            case 3:
                qstrState = tr("No longer active");
                break;
            default:
                qstrState = tr("Error");
                break;
            }

            return QVariant(qstrState);
        }
    }
    // -------------------------------
    return QSortFilterProxyModel::data(index,role);
}


QVariant AgreementsProxyModel::headerData(int section, Qt::Orientation orientation, int role/*=Qt::DisplayRole*/) const
{
    return QSortFilterProxyModel::headerData(section, orientation, role);
}

// --------------------------------------------


bool AgreementsProxyModel::filterAcceptsColumn(int source_column, const QModelIndex & source_parent) const
{
    bool bReturn = true;

    switch (source_column)
    {
    case AGRMT_SOURCE_COL_AGRMT_ID:   bReturn = false;  break;
    case AGRMT_SOURCE_COL_HAVE_READ:   bReturn = false;  break;
    case AGRMT_SOURCE_COL_TXN_ID_DISPLAY:  bReturn = true;  break;
    case AGRMT_SOURCE_COL_NOTARY_ID:  bReturn = true;  break;
    case AGRMT_SOURCE_COL_CONTRACT_ID:  bReturn = true;  break;
    case AGRMT_SOURCE_COL_NEWEST_RECEIPT_ID:  bReturn = true;  break;
    case AGRMT_SOURCE_COL_NEWEST_KNOWN_STATE:  bReturn = true;  break;
    case AGRMT_SOURCE_COL_TIMESTAMP:  bReturn = true;  break;
    case AGRMT_SOURCE_COL_MEMO:  bReturn = true;  break;
    case AGRMT_SOURCE_COL_FOLDER:  bReturn = false;  break;
    default:  bReturn = true;  break;
    }
    return bReturn;
}

bool AgreementsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex indexFolder = sourceModel()->index(sourceRow, AGRMT_SOURCE_COL_FOLDER, sourceParent); // folder

    QAbstractItemModel * pAbstractModel = sourceModel();
    ModelAgreements    * pSourceModel   = dynamic_cast<ModelAgreements*>(pAbstractModel);

    if (nullptr != pSourceModel)
    {
        // Check the folder here.
        QVariant dataFolder = pSourceModel->data(indexFolder);
        const int nFolder = dataFolder.isValid() ? dataFolder.toInt() : (-1);

        if ((nFolder != -1)  && (nFolder != nFolder_))
            return false;
    }
    return true;
}


// --------------------------------------------

QVariant AgreementsProxyModel::rawData ( const QModelIndex & index, int role/*=Qt::DisplayRole*/ ) const
{
    return QSortFilterProxyModel::data(index,role);

}

// ------------------------------------------------------------

ModelAgreementReceipts::ModelAgreementReceipts(QObject *parent/*=0*/, QSqlDatabase db/*=QSqlDatabase()*/)
    : QSqlTableModel(parent, db) {}


QVariant ModelAgreementReceipts::rawData ( const QModelIndex & index, int role/*=Qt::DisplayRole*/ ) const
{
    return QSqlTableModel::data(index,role);
}

QVariant ModelAgreementReceipts::headerData(int section, Qt::Orientation orientation, int role/*=Qt::DisplayRole*/) const
{
    if (role==Qt::UserRole)
    {
        return section;
    }

    return QSqlTableModel::headerData(section, orientation, role);
}

QVariant ModelAgreementReceipts::data ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
    if (role == Qt::TextAlignmentRole) // I don't want to center the memo column.
    {
        if (index.column() == AGRMT_RECEIPT_COL_MEMO)
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        else if (index.column() == AGRMT_RECEIPT_COL_DESCRIPTION)
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
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
        if (index.column() == AGRMT_RECEIPT_COL_RECORD_NAME) // record_name
        {
            QVariant qvarData    = QSqlTableModel::data(index,role);
            QString  qstrData    = qvarData.isValid() ? qvarData.toString() : "";
            QString  qstrDecoded = qstrData.isEmpty() ? "" : MTContactHandler::Decode(qstrData);
            return QVariant(qstrDecoded);
        }
        if (index.column() == AGRMT_RECEIPT_COL_DESCRIPTION) // description
        {
            QVariant qvarData    = QSqlTableModel::data(index,role);
            QString  qstrData    = qvarData.isValid() ? qvarData.toString() : "";
            QString  qstrDecoded = qstrData.isEmpty() ? "" : MTContactHandler::Decode(qstrData);
            return QVariant(qstrDecoded);
        }
        if (index.column() == AGRMT_RECEIPT_COL_MEMO) // memo
        {
            QVariant qvarData    = QSqlTableModel::data(index,role);
            QString  qstrData    = qvarData.isValid() ? qvarData.toString() : "";
            QString  qstrDecoded = qstrData.isEmpty() ? "" : MTContactHandler::Decode(qstrData);
            return QVariant(qstrDecoded);
        }
    }

    return QSqlTableModel::data(index,role);
}

bool ModelAgreementReceipts::setData(const QModelIndex & index, const QVariant & value, int role/*=Qt::EditRole*/)
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
        if (index.column() == AGRMT_RECEIPT_COL_RECORD_NAME) // record_name
        {
            success = QSqlTableModel::setData(index,QVariant(MTContactHandler::Encode(value.toString())),role);
        }
        if (index.column() == AGRMT_RECEIPT_COL_DESCRIPTION) // description
        {
            success = QSqlTableModel::setData(index,QVariant(MTContactHandler::Encode(value.toString())),role);
        }
        if (index.column() == AGRMT_RECEIPT_COL_MEMO) // memo
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

// -----------------------------------------------------------

AgreementReceiptsProxyModel::AgreementReceiptsProxyModel(QObject *parent/*=0*/)
    : QSortFilterProxyModel(parent) {}

void AgreementReceiptsProxyModel::setFilterSent()
{
    filterType_ = FilterSent;
    invalidateFilter();
}

void AgreementReceiptsProxyModel::setFilterReceived()
{
    filterType_ = FilterReceived;
    invalidateFilter();
}

void AgreementReceiptsProxyModel::setFilterNone()
{
    clearFilterType();
    invalidateFilter();
}

void AgreementReceiptsProxyModel::clearFilterType()
{
    filterType_ = FilterNone;
}

void AgreementReceiptsProxyModel::setFilterString(QString qstrFilter)
{
    filterString_ = qstrFilter;
    invalidateFilter();
}

void AgreementReceiptsProxyModel::setFilterAccountId(QString qstrFilter)
{
    filterAccountId_ = qstrFilter;
    invalidateFilter();
}

void AgreementReceiptsProxyModel::setFilterMySignerId(QString qstrFilter)
{
    filterMySignerId_ = qstrFilter;
    invalidateFilter();
}

void AgreementReceiptsProxyModel::setFilterAgreementId(int nFilter)
{
    filterAgreementId_ = nFilter;
    invalidateFilter();
}

QVariant AgreementReceiptsProxyModel::rawData ( const QModelIndex & index, int role/*=Qt::DisplayRole*/ ) const
{
    return QSortFilterProxyModel::data(index,role);
}

QWidget * AgreementReceiptsProxyModel::CreateDetailHeaderWidget(const int nSourceRow, bool bExternal/*=true*/) const
{
    QModelIndex sourceIndex_Folder         = sourceModel()->index(nSourceRow, AGRMT_RECEIPT_COL_FOLDER);
    QModelIndex sourceIndex_AssetId        = sourceModel()->index(nSourceRow, AGRMT_RECEIPT_COL_MY_ASSET_TYPE);
    QModelIndex sourceIndex_Date           = sourceModel()->index(nSourceRow, AGRMT_RECEIPT_COL_TIMESTAMP);
    QModelIndex sourceIndex_Desc           = sourceModel()->index(nSourceRow, AGRMT_RECEIPT_COL_DESCRIPTION);
    QModelIndex sourceIndex_RecordName     = sourceModel()->index(nSourceRow, AGRMT_RECEIPT_COL_RECORD_NAME);
    QModelIndex sourceIndex_InstrumentType = sourceModel()->index(nSourceRow, AGRMT_RECEIPT_COL_INSTRUMENT_TYPE);
    QModelIndex sourceIndex_Amount         = sourceModel()->index(nSourceRow, AGRMT_RECEIPT_COL_AMOUNT);
    QModelIndex sourceIndex_Flags          = sourceModel()->index(nSourceRow, AGRMT_RECEIPT_COL_FLAGS);

    QVariant    sourceData_Folder     = sourceModel()->data(sourceIndex_Folder,  Qt::DisplayRole);
    QVariant    sourceData_AssetId    = sourceModel()->data(sourceIndex_AssetId, Qt::DisplayRole);
    QVariant    sourceData_Date       = sourceModel()->data(sourceIndex_Date,    Qt::DisplayRole);
    QVariant    sourceData_Desc       = sourceModel()->data(sourceIndex_Desc,    Qt::DisplayRole);
    QVariant    sourceData_RecordName = sourceModel()->data(sourceIndex_RecordName,    Qt::DisplayRole);
    QVariant    sourceData_InstrumentType = sourceModel()->data(sourceIndex_InstrumentType,Qt::DisplayRole);
    QVariant    sourceData_Amount  = sourceModel()->data(sourceIndex_Amount,   Qt::DisplayRole);
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


// ------------------------------------------------------------

QVariant AgreementReceiptsProxyModel::data ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
//    if (role == Qt::TextAlignmentRole)
//        return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
    // ----------------------------------------
    if ( role==Qt::SizeHintRole && index.isValid())
    {
//      const int nSourceRow    = headerData(index.row(),    Qt::Vertical,   Qt::UserRole).toInt();
        const int nSourceColumn = headerData(index.column(), Qt::Horizontal, Qt::UserRole).toInt();

        if (AGRMT_RECEIPT_COL_AGRMT_RECEIPT_KEY == nSourceColumn)
        {
            return QVariant::fromValue(QSize(300, 60));
        }
    }
    // ----------------------------------------
    if ( role==Qt::FontRole && index.isValid())
    {
        const int nSourceRow    = headerData(index.row(),    Qt::Vertical,   Qt::UserRole).toInt();
        const int nSourceColumn = AGRMT_RECEIPT_COL_HAVE_READ; // If you haven't read this Payment, we set it bold.

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

        if (nSourceColumn == AGRMT_RECEIPT_COL_AGRMT_RECEIPT_KEY) // agreement_receipt_key (used only behind the scenes as a primary key.)
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
        if (nSourceColumn == AGRMT_RECEIPT_COL_MY_NYM) // my_nym_id
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
        else if (nSourceColumn == AGRMT_RECEIPT_COL_MY_ACCT) // my_acct_id
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
        else if (nSourceColumn == AGRMT_RECEIPT_COL_SENDER_NYM) // sender_nym_id
        {
            QString qstrID = sourceData.isValid() ? sourceData.toString().trimmed() : "";
            const std::string str_id = qstrID.isEmpty() ? "" : qstrID.toStdString();
            MTNameLookupQT theLookup;
            const std::string str_name = str_id.empty() ? "" : theLookup.GetNymName(str_id, "");
            // ------------------------
            if (!str_name.empty())
                return QVariant(QString::fromStdString(str_name));

            QModelIndex sibling = sourceIndex.sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_SENDER_ADDR);
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
        else if (nSourceColumn == AGRMT_RECEIPT_COL_RECIP_NYM) // recipient_nym_id
        {
            QString qstrID = sourceData.isValid() ? sourceData.toString().trimmed() : "";
            const std::string str_id = qstrID.isEmpty() ? "" : qstrID.toStdString();
            MTNameLookupQT theLookup;
            const std::string str_name = str_id.empty() ? "" : theLookup.GetNymName(str_id, "");
            // ------------------------
            if (!str_name.empty())
                return QVariant(QString::fromStdString(str_name));

            QModelIndex sibling = sourceIndex.sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_RECIP_ADDR);
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
        else if (nSourceColumn == AGRMT_RECEIPT_COL_NOTARY_ID) // notary_id
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
        else if (nSourceColumn == AGRMT_RECEIPT_COL_AMOUNT) // amount
        {
            QModelIndex sourceIndex_Amount         = sourceModel()->index(nSourceRow, AGRMT_RECEIPT_COL_AMOUNT);

            QVariant qvarAmount = sourceIndex_Amount.isValid() ? sourceModel()->data(sourceIndex_Amount) : QVariant();

            //int64_t lAmount = sourceData.isValid() ? sourceData.toLongLong() : 0;
            int64_t lAmount = qvarAmount.isValid() ? qvarAmount.toLongLong() : 0;

            QModelIndex sibling   = sourceIndex.sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_MY_ASSET_TYPE);
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
        else if (nSourceColumn == AGRMT_RECEIPT_COL_MY_ASSET_TYPE) // my_asset_type_id
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
        else if (nSourceColumn == AGRMT_RECEIPT_COL_METHOD_TYPE_DISP) // Method type display (if blank, we see if there's a notary ID. If so, then transport shows "otserver".)
        {
            QString qstrType = sourceData.isValid() ? sourceData.toString() : "";
            // ------------------------
            if (!qstrType.isEmpty())
                return QVariant(qstrType);
            return QVariant(QString("opentxs"));
        }
        else if (nSourceColumn == AGRMT_RECEIPT_COL_TIMESTAMP) // timestamp
        {
            time64_t the_time = sourceData.isValid() ? sourceData.toLongLong() : 0;
            QDateTime timestamp;
            timestamp.setTime_t(the_time);
            return QVariant(QString(timestamp.toString(Qt::SystemLocaleShortDate)));
        }
        else if (nSourceColumn == AGRMT_RECEIPT_COL_FOLDER) // folder
        {
            int nFolder = sourceData.isValid() ? sourceData.toInt() : 0;
            return QVariant(tr(0 == nFolder ? "Sent" : "Receieved"));
        }
    }
    // --------------------------------------------
    return QSortFilterProxyModel::data(index,role);
}

QVariant AgreementReceiptsProxyModel::headerData(int section, Qt::Orientation orientation, int role/*=Qt::DisplayRole*/) const
{
    QVariant varHeaderData = QSortFilterProxyModel::headerData(section, orientation, role);

    if (Qt::Horizontal == orientation && Qt::DisplayRole == role)
    {
        QString qstrHeader = varHeaderData.toString();

        if (0 == qstrHeader.compare(tr("Me")))
        {
            if (FilterSent == filterType_) // sent box
                return QVariant(tr("From"));
            else // received box
                return QVariant(tr("To"));
        }
        else if (0 == qstrHeader.compare(tr("Details")))
        {
            if (FilterSent == filterType_) // sent box
                return QVariant(tr("To"));
            else // received box
                return QVariant(tr("From"));
        }
    }

    return varHeaderData;
}

// --------------------------------------------

bool AgreementReceiptsProxyModel::filterAcceptsColumn(int source_column, const QModelIndex & source_parent) const
{
    bool bReturn = true;

    switch (source_column)
    {
    case AGRMT_RECEIPT_COL_AGRMT_RECEIPT_KEY:   bReturn = true;  break; // agreement_receipt_key
    case AGRMT_RECEIPT_COL_AGRMT_ID:   bReturn = false;  break; // agreement_id
    case AGRMT_RECEIPT_COL_MY_NYM:  // my_nym_id
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

    case AGRMT_RECEIPT_COL_MY_ACCT:
    {
        if (filterAccountId_.isEmpty()) // If the acct is empty, we aren't filtering on it. So show the account ID.
            bReturn = true;
        else
            bReturn = false; // But if we're already filtering based on a certain account ID, we don't need to show the acct ID column.
    }
        break;
    case AGRMT_RECEIPT_COL_MY_ASSET_TYPE:
    {
        bReturn = true;
    }
        break;
    case AGRMT_RECEIPT_COL_MY_ADDR:
    {
        bReturn = false;
//        if (FilterNone == filterType_)
//            bReturn = true;
//        else
//            bReturn = false;
    }
        break;
    case AGRMT_RECEIPT_COL_SENDER_NYM:
    {
        if (FilterSent == filterType_) // 0 is the outbox. (In the outbox, you don't show the sender, but the recipient. So we hide the sender column.)
            return false;
        else
            bReturn = true;
    }
        break;
    case AGRMT_RECEIPT_COL_SENDER_ACCT:
    {
        bReturn = false;
    }
        break;
    case AGRMT_RECEIPT_COL_SENDER_ADDR:
    {
        if (FilterSent == filterType_) // 0 is the outbox. (In the outbox, you don't show the sender, but the recipient. So we hide the sender column.)
            return false;
        else
            bReturn = false; // Current expected behavior.
    }
        break;
    case AGRMT_RECEIPT_COL_RECIP_NYM:
    {
        if (FilterSent != filterType_) // All other boxes except outbox, hide recipient. (We normally see a recipient in the outbox, instead of a sender.)
            return false;
        else
            bReturn = true;
    }
        break;
    case AGRMT_RECEIPT_COL_RECIP_ACCT:
    {
        bReturn = false;
    }
        break;

    case AGRMT_RECEIPT_COL_RECIP_ADDR:
    {
        if (FilterSent != filterType_) // All other boxes except outbox, hide recipient. (We normally see a recipient in the outbox, instead of a sender.)
            return false;
        else
            bReturn = false; // Current expected behavior.
    }
        break;

    case AGRMT_RECEIPT_COL_METHOD_TYPE:
    case AGRMT_RECEIPT_COL_METHOD_TYPE_DISP: bReturn = false;  break;

    case AGRMT_RECEIPT_COL_EVENT_ID:        bReturn = false; break;

    case AGRMT_RECEIPT_COL_NOTARY_ID:       bReturn = true;  break;

    case AGRMT_RECEIPT_COL_TXN_ID_DISPLAY:  bReturn = false;  break;
    case AGRMT_RECEIPT_COL_AMOUNT:          bReturn = true;   break;
    case AGRMT_RECEIPT_COL_TIMESTAMP:       bReturn = true;   break;
    case AGRMT_RECEIPT_COL_HAVE_READ:       bReturn = false;  break;
    case AGRMT_RECEIPT_COL_MEMO:            bReturn = false;  break;
    case AGRMT_RECEIPT_COL_DESCRIPTION:     bReturn = false;  break;
    case AGRMT_RECEIPT_COL_RECORD_NAME:     bReturn = false;  break;
    case AGRMT_RECEIPT_COL_FOLDER:          bReturn = false;  break;
    case AGRMT_RECEIPT_COL_FLAGS:           bReturn = false;  break;
    default:  bReturn = true;  break;
    }
    return bReturn;
}


bool AgreementReceiptsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex indexAgreementId  = sourceModel()->index(sourceRow, AGRMT_RECEIPT_COL_AGRMT_ID,       sourceParent); // agreement_id
    QModelIndex indexReceiptId    = sourceModel()->index(sourceRow, AGRMT_RECEIPT_COL_RECEIPT_ID,     sourceParent); // receipt_id
    QModelIndex indexTxnIdDisplay = sourceModel()->index(sourceRow, AGRMT_RECEIPT_COL_TXN_ID_DISPLAY, sourceParent); // txn_id_display
    QModelIndex indexMyNym        = sourceModel()->index(sourceRow, AGRMT_RECEIPT_COL_MY_NYM,         sourceParent); // my_nym_id
    QModelIndex indexMyAcct       = sourceModel()->index(sourceRow, AGRMT_RECEIPT_COL_MY_ACCT,        sourceParent); // my_acct_id
    QModelIndex indexAssetType    = sourceModel()->index(sourceRow, AGRMT_RECEIPT_COL_MY_ASSET_TYPE,  sourceParent); // my_asset_type_id
    QModelIndex indexSenderNym    = sourceModel()->index(sourceRow, AGRMT_RECEIPT_COL_SENDER_NYM,     sourceParent); // sender_nym_id
    QModelIndex indexSenderAcct   = sourceModel()->index(sourceRow, AGRMT_RECEIPT_COL_SENDER_ACCT,    sourceParent); // sender_acct_id
    QModelIndex indexSenderAddr   = sourceModel()->index(sourceRow, AGRMT_RECEIPT_COL_SENDER_ADDR,    sourceParent); // sender_address
    QModelIndex indexRecipNym     = sourceModel()->index(sourceRow, AGRMT_RECEIPT_COL_RECIP_NYM,      sourceParent); // recipient_nym_id
    QModelIndex indexRecipAcct    = sourceModel()->index(sourceRow, AGRMT_RECEIPT_COL_RECIP_ACCT,     sourceParent); // recipient_acct_id
    QModelIndex indexRecipAddr    = sourceModel()->index(sourceRow, AGRMT_RECEIPT_COL_RECIP_ADDR,     sourceParent); // recipient_address
    QModelIndex indexMethodType   = sourceModel()->index(sourceRow, AGRMT_RECEIPT_COL_METHOD_TYPE,    sourceParent); // method_type
    QModelIndex indexNotary       = sourceModel()->index(sourceRow, AGRMT_RECEIPT_COL_NOTARY_ID,      sourceParent); // notary_id
    QModelIndex indexFolder       = sourceModel()->index(sourceRow, AGRMT_RECEIPT_COL_FOLDER,         sourceParent); // folder
    QModelIndex indexDescription  = sourceModel()->index(sourceRow, AGRMT_RECEIPT_COL_DESCRIPTION,    sourceParent); // description
    QModelIndex indexFlags        = sourceModel()->index(sourceRow, AGRMT_RECEIPT_COL_FLAGS,          sourceParent); // flags

    QAbstractItemModel     * pModel    = sourceModel();
    ModelAgreementReceipts * pMsgModel = dynamic_cast<ModelAgreementReceipts*>(pModel);

    if (nullptr != pMsgModel)
    {
        // Check the folder here.
        QVariant dataFolder = pMsgModel->data(indexFolder);
        const int nFolder = dataFolder.isValid() ? dataFolder.toInt() : (-1);

        if ((nFolder != -1) && (FilterNone != filterType_))
        {
            if ( (FilterSent == filterType_) && (0 != nFolder) )
                return false;
            else if ( (FilterReceived == filterType_) && !(nFolder > 0) )
                return false;
        }
        // ------------------------------------
        // Grab the data for the current row.
        //
        ModelPayments::PaymentFlags flags = ModelPayments::NoFlags;

        const QVariant dataAgreementId      = pMsgModel->data(indexAgreementId);
        const QVariant dataReceiptId        = pMsgModel->data(indexReceiptId);
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
        const QVariant dataDescription      = pMsgModel->data(indexDescription);
        const QVariant varFlags             = pMsgModel->rawData(indexFlags);

//        const QVariant dataSenderName       = this->data(mapFromSource(indexSenderNym));
//        const QVariant dataRecipientName    = this->data(mapFromSource(indexRecipNym));
//        const QVariant dataNotaryName       = this->data(mapFromSource(indexNotary));

        qint64 lFlags = varFlags.isValid() ? varFlags.toLongLong() : 0;

        flags = ModelPayments::PaymentFlag(static_cast<ModelPayments::PaymentFlag>(lFlags)); // Note: isn't this line redundant??

        if ( flags.testFlag(ModelPayments::IsFinalReceipt))
            return false;
        // -----------------------------------------------
        const int64_t lAgreementId         = dataAgreementId.isValid() ? dataAgreementId.toLongLong() : 0;
        const int64_t lReceiptId           = dataReceiptId.isValid() ? dataReceiptId.toLongLong() : 0;
        const int64_t lTxnIdDisplay        = dataTxnIdDisplay.isValid() ? dataTxnIdDisplay.toLongLong() : 0;
        const QString qstrAgreementId      = lAgreementId > 0 ? QString::fromStdString(opentxs::OTAPI_Wrap::It()->LongToString(lAgreementId)) : "";
        const QString qstrReceiptId        = lReceiptId > 0 ? QString::fromStdString(opentxs::OTAPI_Wrap::It()->LongToString(lReceiptId)) : "";
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

            if (!qstrAgreementId.contains(filterString_) &&
                !qstrReceiptId.contains(filterString_) &&
                !qstrTxnIdDisplay.contains(filterString_) &&
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
        if (!filterAccountId_.isEmpty() && !qstrMyAcct.isEmpty())
        {
            if (0 != qstrMyAcct.compare(filterAccountId_))
                return false;
        }
        // ------------------------------------
        if (!filterMySignerId_.isEmpty() && !qstrMyNym.isEmpty())
        {
            if (0 != qstrMyNym.compare(filterMySignerId_))
                return false;
        }
        // ------------------------------------
        if ((filterAgreementId_ > 0) && (lAgreementId > 0))
        {
            if (lAgreementId != filterAgreementId_)
                return false;
        }
        // ------------------------------------
    }

    return true;
}

