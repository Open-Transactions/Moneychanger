#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/moneychanger.hpp>
#include <core/handlers/DBHandler.hpp>
#include <core/handlers/contacthandler.hpp>
#include <core/handlers/modelmessages.hpp>

#include <opentxs/opentxs.hpp>

#include <QDebug>
#include <QtGlobal>
#include <QDateTime>
#include <Qt>

#include <QGridLayout>
#include <QLabel>


// ------------------------------------------------------------


//#define CONV_SOURCE_COL_MSG_ID 0
//#define CONV_SOURCE_COL_MY_NYM 1
//#define CONV_SOURCE_COL_THREAD_ID 2
//#define CONV_SOURCE_COL_ITEM_ID 3
//#define CONV_SOURCE_COL_TIMESTAMP 4
//#define CONV_SOURCE_COL_FOLDER 5
//#define CONV_SOURCE_COL_BODY 6

// class QSqlQueryMessages : public QSqlQueryModel

QSqlQueryMessages::QSqlQueryMessages(QObject *parent/*=0*/) : QSqlQueryModel(parent)
{
}

QVariant QSqlQueryMessages::data ( const QModelIndex & index, int role/*=Qt::DisplayRole*/) const
{
//    if (role==Qt::DisplayRole && QSqlQueryModel::data(index,role).isValid())
//    {
//        if (index.column() == CONV_SOURCE_COL_BODY) // message body
//        {
//            QVariant qvarData      = QSqlQueryModel::data(index,role);
//            QString  qstrData      = qvarData.isValid() ? qvarData.toString() : "";
//            QString  qstrDecrypted = qstrData.isEmpty() ? "" : MTContactHandler::Decrypt(qstrData);
//            return QVariant(qstrDecrypted);
//        }
//    }
    return QSqlQueryModel::data(index,role);
}

QVariant QSqlQueryMessages::rawData ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
    return QSqlQueryModel::data(index,role);
}

QVariant QSqlQueryMessages::headerData(int section, Qt::Orientation orientation, int role /*=Qt::DisplayRole*/) const
{
    if (role==Qt::UserRole)
    {
        return section;
    }

    return QSqlQueryModel::headerData(section, orientation, role);
}

// ------------------------------------------------------------

ConvMsgsProxyModel::ConvMsgsProxyModel(QObject *parent /*=0*/)
: QSortFilterProxyModel(parent)
{
}

void ConvMsgsProxyModel::setFilterFolder(int nFolder)
{
    nFolder_ = nFolder; // 0 for outbox, 1 for inbox, and 2 for both.
    invalidateFilter();
}

void ConvMsgsProxyModel::setFilterNone()
{
    clearFilterType();
    filterType_ = FilterNone;
    // --------------------------------
    invalidateFilter();
}

void ConvMsgsProxyModel::setFilterString(QString qstrFilter)
{
    filterString_ = qstrFilter;
    invalidateFilter();
}

void ConvMsgsProxyModel::clearFilterType()
{
    // Notice we DON'T clear filterString_,
    // since that may be in play (or not) regardless
    // of which conversation you may or may not be clicked on.
    // Similarly the "folder" isn't affected by this function either.
    // Basically this function is purely about which NOTARY or which
    // CONVERSATION you're currently clicked on.
    //
    filterType_       = FilterNone;
    // --------------------------------
    myNymId_          = QString("");
    notaryId_         = QString("");
    conversationId_   = QString("");
    hisContactId_     = QString("");
    // --------------------------------
    mapNymIds_.clear();
    mapAddresses_.clear();
}


void ConvMsgsProxyModel::setFilterTopLevel_Contact(const std::string & strMyNymID, const std::string & strContactID)
{
    clearFilterType();
    filterType_ = FilterTopLevel;
    // --------------------------------
    myNymId_ = QString::fromStdString(strMyNymID);
    hisContactId_ = QString::fromStdString(strContactID);
    conversationId_ = QString("");
    // --------------------------------
    MTContactHandler::getInstance()->GetNyms(mapNymIds_, strContactID);
    MTContactHandler::getInstance()->GetAddressesByContact(mapAddresses_, strContactID, "", false);
    // --------------------------------
    invalidateFilter();
}

void ConvMsgsProxyModel::setFilterTopLevel_Convo(const std::string & strMyNymID, const std::string & strConversationID)
{
    clearFilterType();
    filterType_ = FilterTopLevel;
    // --------------------------------
    myNymId_ = QString::fromStdString(strMyNymID);
    hisContactId_ = QString("");
    conversationId_ = QString::fromStdString(strConversationID);
    // --------------------------------
    // Note and TODO: until we have group convos, ConvoId IS the Contact ID.
    // But once we have real group convos, this will have to be fixed.
    //
    const std::string & strContactID = strConversationID;
    // --------------------------------
    MTContactHandler::getInstance()->GetNyms(mapNymIds_, strContactID);
    MTContactHandler::getInstance()->GetAddressesByContact(mapAddresses_, strContactID, "", false);
    // --------------------------------
    invalidateFilter();
}

//void ConvMsgsProxyModel::setFilterTopLevel(const std::string & strContactID)
//{
//    clearFilterType();
//    filterType_ = FilterTopLevel;
//    // --------------------------------
//    MTContactHandler::getInstance()->GetNyms(mapNymIds_, strContactID);
//    MTContactHandler::getInstance()->GetAddressesByContact(mapAddresses_, strContactID, "", false);
//    // --------------------------------
//    invalidateFilter();
//}


void ConvMsgsProxyModel::setFilterNotary_Contact(QString qstrNotaryId, const std::string & strMyNymID, const std::string & strContactID)
{
    clearFilterType();
    filterType_ = FilterNotary;
    // --------------------------------
    notaryId_ = qstrNotaryId;
    myNymId_ = QString::fromStdString(strMyNymID);
    hisContactId_ = QString::fromStdString(strContactID);
    conversationId_ = QString("");
    // --------------------------------
    MTContactHandler::getInstance()->GetNyms(mapNymIds_, strContactID);
    // --------------------------------
    invalidateFilter();

}

void ConvMsgsProxyModel::setFilterNotary_Convo(
    QString qstrNotaryId,
    const std::string & strMyNymID,
    const std::string & strConversationID) // Note: until we have group convos, ConvoId IS the Contact ID.
{
    clearFilterType();
    filterType_ = FilterNotary;
    // --------------------------------
    notaryId_ = qstrNotaryId;
    myNymId_ = QString::fromStdString(strMyNymID);
    hisContactId_ = QString("");
    conversationId_ = QString::fromStdString(strConversationID);
    // --------------------------------
    // Note and TODO: until we have group convos, ConvoId IS the Contact ID.
    // But once we have real group convos, this will have to be fixed.
    //
    const std::string & strContactID = strConversationID;
    // --------------------------------
    MTContactHandler::getInstance()->GetNyms(mapNymIds_, strContactID);
    // --------------------------------
    invalidateFilter();

}

//void ConvMsgsProxyModel::setFilterNotary(QString qstrNotaryId, const std::string & strContactID)
//{
//    clearFilterType();
//    filterType_ = FilterNotary;
//    // --------------------------------
//    notaryId_ = qstrNotaryId;
//    MTContactHandler::getInstance()->GetNyms(mapNymIds_, strContactID);
//    // --------------------------------
//    invalidateFilter();
//}


// --------------------------------------------

QVariant ConvMsgsProxyModel::headerData(int section, Qt::Orientation orientation, int role/*=Qt::DisplayRole*/) const
{
    return QSortFilterProxyModel::headerData(section, orientation, role);

//    QVariant varHeaderData = QSortFilterProxyModel::headerData(section, orientation, role);

//    if (Qt::Horizontal == orientation && Qt::DisplayRole == role)
//    {
//        QString qstrHeader = varHeaderData.toString();

//        if (0 == qstrHeader.compare(tr("Me"))) // If the actual column header is "Me", substitute it with "From" or "To" as appropriate.
//        {
//            if (0 == nFolder_) // sent box
//                return QVariant(tr("From"));
//            else // received box
//                return QVariant(tr("To"));
//        }
//    }

//    return varHeaderData;
}

// --------------------------------------------

QVariant ConvMsgsProxyModel::rawData ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
    return QSortFilterProxyModel::data(index,role);
}

// ------------------------------------------------------------

//#define CONV_SOURCE_COL_MSG_ID 0
//#define CONV_SOURCE_COL_MY_NYM 1
//#define CONV_SOURCE_COL_THREAD_ID 2
//#define CONV_SOURCE_COL_ITEM_ID 3
//#define CONV_SOURCE_COL_TIMESTAMP 4
//#define CONV_SOURCE_COL_FOLDER 5
//#define CONV_SOURCE_COL_BODY 6

QVariant ConvMsgsProxyModel::data ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
//    if (role == Qt::TextAlignmentRole)
//        return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
    // ----------------------------------------
    if ( role==Qt::SizeHintRole && index.isValid())
    {
//      const int nSourceRow    = headerData(index.row(),    Qt::Vertical,   Qt::UserRole).toInt();

//        const int nSourceColumn = headerData(index.column(), Qt::Horizontal, Qt::UserRole).toInt();

//        if (CONV_SOURCE_COL_BODY == nSourceColumn)
//        {
//            if (nullptr != pTableView_)
//            {
//                QWidget * pWidget = pTableView_->indexWidget(index);

//                if (nullptr != pWidget)
//                {
//                    return QVariant::fromValue(pWidget->sizeHint());
//                }
//            }
//        }
    }
//    else
    // ----------------------------------------
//    if ( role==Qt::FontRole && index.isValid())
//    {
//        const int nSourceRow    = headerData(index.row(),    Qt::Vertical,   Qt::UserRole).toInt();
//        const int nSourceColumn = MSG_SOURCE_COL_HAVE_READ; // If you haven't read this message, we set it bold.

//        QModelIndex sourceIndex = sourceModel()->index(nSourceRow, nSourceColumn);
//        QVariant    sourceData  = sourceModel()->data(sourceIndex, Qt::DisplayRole);

//        if (sourceData.isValid() && (!sourceData.toBool()))
//        {
//            QFont boldFont;
//            boldFont.setBold(true);
//            return boldFont;
//        }
//    }
//    // ----------------------------------------
    else
    if ( role==Qt::DisplayRole && index.isValid())
//      && QSortFilterProxyModel::data(index,role).isValid())
    {
        const int nSourceRow    = headerData(index.row(),    Qt::Vertical,   Qt::UserRole).toInt();
        const int nSourceColumn = headerData(index.column(), Qt::Horizontal, Qt::UserRole).toInt();
        QModelIndex sourceIndex = sourceModel()->index(nSourceRow, nSourceColumn);
        QVariant    sourceData  = sourceModel()->data(sourceIndex, role);

        if (nSourceColumn == CONV_SOURCE_COL_BODY) // message body
        {
            if (nullptr != pTableView_)
            {
                if (   (nullptr == pTableView_->indexWidget(index))
                    && sourceData.isValid())
                {
                    QWidget * pWidget = CreateDetailHeaderWidget(nSourceRow);
                    pWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Ignored);
                    pTableView_->setIndexWidget(index, pWidget);
//                  pTableView_->setRowHeight(index.row(), pWidget->height()*1.2);
                }
            }
            return QVariant();
        }
        else
        if (nSourceColumn == CONV_SOURCE_COL_MY_NYM) // my_nym_id
        {
            QString qstrID = sourceData.isValid() ? sourceData.toString() : "";

            std::string str_name;

            if (!qstrID.isEmpty())
            {
                const std::string str_id = qstrID.trimmed().toStdString();
                str_name = str_id.empty() ? "" : opentxs::OT::App().Client().Exec().GetNym_Name(str_id);
            }
            // ------------------------
            if (str_name.empty() && !qstrID.isEmpty())
                return QVariant(qstrID);
            return QVariant(QString::fromStdString(str_name));
        }
        else
        if (nSourceColumn == CONV_SOURCE_COL_TIMESTAMP) // timestamp
        {
            time64_t the_time = sourceData.isValid() ? sourceData.toLongLong() : 0;
            QDateTime timestamp;
            timestamp.setTime_t(the_time);
            return QVariant(QString(timestamp.toString(Qt::SystemLocaleShortDate)));
        }
    }
    // --------------------------------------------
    return QSortFilterProxyModel::data(index,role);
}



//#define MSG_SOURCE_COL_MSG_ID 0
//#define MSG_SOURCE_COL_HAVE_READ 1
//#define MSG_SOURCE_COL_HAVE_REPLIED 2
//#define MSG_SOURCE_COL_HAVE_FORWARDED 3
//#define MSG_SOURCE_COL_SUBJECT 4
//#define MSG_SOURCE_COL_SENDER_NYM 5
//#define MSG_SOURCE_COL_SENDER_ADDR 6
//#define MSG_SOURCE_COL_RECIP_NYM 7
//#define MSG_SOURCE_COL_RECIP_ADDR 8
//#define MSG_SOURCE_COL_TIMESTAMP 9
//#define MSG_SOURCE_COL_METHOD_TYPE 10
//#define MSG_SOURCE_COL_METHOD_TYPE_DISP 11
//#define MSG_SOURCE_COL_NOTARY_ID 12
//#define MSG_SOURCE_COL_MY_NYM 13
//#define MSG_SOURCE_COL_MY_ADDR 14
//#define MSG_SOURCE_COL_FOLDER 15
//#define MSG_SOURCE_COL_THREAD_ITEM_ID 16
//#define MSG_SOURCE_COL_ARCHIVED 17
//#define MSG_SOURCE_COL_HAS_SUBJECT 18


//#define CONV_SOURCE_COL_MSG_ID 0
//#define CONV_SOURCE_COL_MY_NYM 1
//#define CONV_SOURCE_COL_THREAD_ID 2
//#define CONV_SOURCE_COL_ITEM_ID 3
//#define CONV_SOURCE_COL_TIMESTAMP 4
//#define CONV_SOURCE_COL_FOLDER 5
//#define CONV_SOURCE_COL_BODY 6

// TODO: Make this appear as a bubble chat style conversation.
QWidget * ConvMsgsProxyModel::CreateDetailHeaderWidget(const int nSourceRow, bool bExternal/*=false*/) const
{
//  QModelIndex sourceIndex_MsgId          = sourceModel()->index(nSourceRow, CONV_SOURCE_COL_MSG_ID);
//  QModelIndex sourceIndex_MyNymId        = sourceModel()->index(nSourceRow, CONV_SOURCE_COL_MY_NYM);
//  QModelIndex sourceIndex_ThreadId       = sourceModel()->index(nSourceRow, CONV_SOURCE_COL_THREAD_ID);
//  QModelIndex sourceIndex_ThreadItemId   = sourceModel()->index(nSourceRow, CONV_SOURCE_COL_ITEM_ID);
//  QModelIndex sourceIndex_Timestamp      = sourceModel()->index(nSourceRow, CONV_SOURCE_COL_TIMESTAMP);
    QModelIndex sourceIndex_Folder         = sourceModel()->index(nSourceRow, CONV_SOURCE_COL_FOLDER);
    QModelIndex sourceIndex_Body           = sourceModel()->index(nSourceRow, CONV_SOURCE_COL_BODY);

//  QVariant sourceData_MsgId          = sourceModel()->data(sourceIndex_MsgId,        Qt::DisplayRole);
//  QVariant sourceData_MyNymId        = sourceModel()->data(sourceIndex_MyNymId,      Qt::DisplayRole);
//  QVariant sourceData_ThreadId       = sourceModel()->data(sourceIndex_ThreadId,     Qt::DisplayRole);
//  QVariant sourceData_ThreadItemId   = sourceModel()->data(sourceIndex_ThreadItemId, Qt::DisplayRole);
//  QVariant sourceData_Timestamp      = sourceModel()->data(sourceIndex_Timestamp,    Qt::DisplayRole);
    QVariant sourceData_Folder         = sourceModel()->data(sourceIndex_Folder,       Qt::DisplayRole);
    QVariant sourceData_Body           = sourceModel()->data(sourceIndex_Body,         Qt::DisplayRole);

//  const int64_t lMsgId  = sourceData_MsgId.isValid() ? sourceData_MsgId.toLongLong() : 0;
    const int     nFolder = sourceData_Folder.isValid() ? sourceData_Folder.toInt() : -1;

//  QString     qstrMyNymId     = sourceData_MyNymId.isValid() ? sourceData_MyNymId.toString() : "";
//  QString     qstrThreadId = sourceData_ThreadId.isValid() ? sourceData_ThreadId   .toString() : "";
//  QString     qstrThreadItemId  = sourceData_ThreadItemId.isValid() ? sourceData_ThreadItemId.toString() : "";

    //const QString qstrEncryptedBody = sourceData_Body.isValid() ? sourceData_Body.toString() : "";
    QString qstrBody = sourceData_Body.isValid() ? sourceData_Body.toString() : "";
//    QString qstrBody = (!qstrEncryptedBody.isEmpty() && bExternal)
//            ? MTContactHandler::Decrypt(qstrEncryptedBody)
//            : qstrEncryptedBody;

//  time64_t the_time = sourceData_Timestamp.isValid() ? sourceData_Timestamp.toLongLong() : 0;
//  QDateTime timestamp;
//  timestamp.setTime_t(the_time);
//  QString qstrTimestamp = QString(timestamp.toString(Qt::SystemLocaleShortDate));
    // -------------------------------------------------
    const bool bIsOutgoing     = (nFolder == 0);
    // --------------------------------------------------------------------------------------------
//  std::string str_desc = qstrDescription.toStdString();
    // --------------------------------------------------------------------------------------------
    QWidget * row_widget = new QWidget;
    QGridLayout * row_widget_layout = new QGridLayout;

    row_widget_layout->setSpacing(4);
    row_widget_layout->setContentsMargins(10, 4, 10, 4);

    row_widget->setLayout(row_widget_layout);
    row_widget->setStyleSheet("QWidget{background-color:#585858;selection-background-color:#a0aac4;}");
//    // -------------------------------------------
//    //Render row.
//    //Header of row
//    QString tx_name = qstrRecordName;

//    if (tx_name.trimmed() == "")
//    {
//        //Tx has no name
//        tx_name.clear();
//        tx_name = "Receipt";
//    }

//    QLabel * header_of_row = new QLabel;
//    QString header_of_row_string = QString("");
//    header_of_row_string.append(tx_name);

//    header_of_row->setText(header_of_row_string);

//    //Append header to layout
//    row_widget_layout->addWidget(header_of_row, 0, 0, 1,1, Qt::AlignLeft);
//    // -------------------------------------------
//    // Amount (with currency tla)
//    QLabel * currency_amount_label = new QLabel;
//    QString currency_amount;

//    currency_amount_label->setStyleSheet(QString("QLabel { color : %1; }").arg(strColor));
//    // ----------------------------------------------------------------
//    bool bLabelAdded = false;

//    std::string str_formatted;
//    bool bFormatted = false;

//    if ( !qstrAssetId.isEmpty() )
//    {
//        str_formatted = opentxs::OT::App().Client().Exec().FormatAmount(qstrAssetId.toStdString(), lAmount);
//        bFormatted = !str_formatted.empty();
//    }
//    // ----------------------------------------
//    if (bFormatted)
//        currency_amount = QString::fromStdString(str_formatted);
//    else
//        currency_amount = QString("");
//    // ----------------------------------------------------------------
//    currency_amount_label->setText(currency_amount);
//    // ----------------------------------------------------------------
//    if (!bLabelAdded)
//        row_widget_layout->addWidget(currency_amount_label, 0, 1, 1,1, Qt::AlignRight);
//    // -------------------------------------------
//    //Sub-info
//    QWidget * row_content_container = new QWidget;
//    QGridLayout * row_content_grid = new QGridLayout;

//    // left top right bottom

//    row_content_grid->setSpacing(4);
//    row_content_grid->setContentsMargins(3, 4, 3, 4);

//    row_content_container->setLayout(row_content_grid);

//    row_widget_layout->addWidget(row_content_container, 1,0, 1,2);
//    // -------------------------------------------
//    // Column one
//    QLabel * row_content_date_label = new QLabel;
//    QString row_content_date_label_string;
//    row_content_date_label_string.append(qstrTimestamp);

//    row_content_date_label->setStyleSheet("QLabel { color : grey; font-size:11pt;}");
//    row_content_date_label->setText(row_content_date_label_string);

//    row_content_grid->addWidget(row_content_date_label, 0,0, 1,1, Qt::AlignLeft);
//    // -------------------------------------------
//    // Column two
//    //Status
    QLabel * row_content_body_label = new QLabel;
    QString & row_content_body_string = qstrBody;
    // -------------------------------------------
    //add string to label
    const QString strTextColor       (bIsOutgoing ? "#ededed" : "Black");
    const QString strBackgroundColor (bIsOutgoing ? "#787878"  : "#9e9e9e");

    const QString qstrStyleSheet = QString("QLabel { background-color : %1; color : %2; font-size:11pt;}").arg(strBackgroundColor).arg(strTextColor);

//  row_content_body_label->setStyleSheet("QLabel { color : grey; font-size:11pt;}");
    row_content_body_label->setStyleSheet(qstrStyleSheet);
    row_content_body_label->setWordWrap(true);
    row_content_body_label->setMargin(2);
    row_content_body_label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Ignored);
    row_content_body_string.replace("\r\n"," ");
    row_content_body_string.replace("\n\r"," ");
    row_content_body_string.replace("\n",  " ");
    row_content_body_string.replace("\r",  " ");
    row_content_body_label->setText(row_content_body_string);

    if(bIsOutgoing)
        row_widget_layout->addWidget(row_content_body_label, 0,1, 1,1, Qt::AlignRight);
    else
        row_widget_layout->addWidget(row_content_body_label, 0,1, 1,1, Qt::AlignLeft);

//    //add to row_content grid
//    row_content_grid->addWidget(row_content_status_label, 0,1, 1,1, Qt::AlignRight);
//    // -------------------------------------------
    row_widget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Ignored);
    //row_widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    return row_widget;
}


//#define CONV_SOURCE_COL_MSG_ID 0
//#define CONV_SOURCE_COL_MY_NYM 1
//#define CONV_SOURCE_COL_THREAD_ID 2
//#define CONV_SOURCE_COL_ITEM_ID 3
//#define CONV_SOURCE_COL_TIMESTAMP 4
//#define CONV_SOURCE_COL_FOLDER 5
//#define CONV_SOURCE_COL_BODY 6

bool ConvMsgsProxyModel::filterAcceptsColumn(int source_column, const QModelIndex & source_parent) const
{
    bool bReturn = true;

    switch (source_column)
    {
    case CONV_SOURCE_COL_MSG_ID:   bReturn = false;  break; // message_id
    case CONV_SOURCE_COL_MY_NYM:  // my_nym_id
    {
        // NOTE: We'll hide this column, where necessary, using the tableView.
        // That way we still have access to the data when we need it.
        // So as far as the model is concerned, this column is always shown.
        bReturn = false;
//        if (FilterNone == filterType_)
//           bReturn = true;
//        else
//           bReturn = false;
    }
        break;
    case CONV_SOURCE_COL_THREAD_ID:
    {
        bReturn = false;
    }
        break;
    case CONV_SOURCE_COL_ITEM_ID:
    {
        bReturn = false;
    }
        break;

    case CONV_SOURCE_COL_TIMESTAMP:       bReturn = true;   break;
    case CONV_SOURCE_COL_FOLDER:          bReturn = false;  break;
    case CONV_SOURCE_COL_BODY:            bReturn = true;   break;

    default:  bReturn = true;  break;
    }
    return bReturn;
}



// What do we filter on here?
// Well, the message table doesn't contain a Contact ID.
// Instead, it contains a NymID and/or a Bitmessage address and/or a notary address.
//
// When the user clicks on a top-level Contact ID, we need to filter on ALL known Nyms AND Bitmessage addresses for that Contact.
// Any single one of them could be the one on the message, that causes the filter to approve it.
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

//#define CONV_SOURCE_COL_MSG_ID 0
//#define CONV_SOURCE_COL_MY_NYM 1
//#define CONV_SOURCE_COL_THREAD_ID 2
//#define CONV_SOURCE_COL_ITEM_ID 3
//#define CONV_SOURCE_COL_TIMESTAMP 4
//#define CONV_SOURCE_COL_FOLDER 5
//#define CONV_SOURCE_COL_BODY 6

bool ConvMsgsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    return true;

//    QModelIndex indexSenderNym  = sourceModel()->index(sourceRow, MSG_SOURCE_COL_SENDER_NYM,  sourceParent); // sender_nym_id
//    QModelIndex indexSenderAddr = sourceModel()->index(sourceRow, MSG_SOURCE_COL_SENDER_ADDR, sourceParent); // sender_address
//    QModelIndex indexRecipNym   = sourceModel()->index(sourceRow, MSG_SOURCE_COL_RECIP_NYM,   sourceParent); // recipient_nym_id
//    QModelIndex indexRecipAddr  = sourceModel()->index(sourceRow, MSG_SOURCE_COL_RECIP_ADDR,  sourceParent); // recipient_address
//    QModelIndex indexMethodType = sourceModel()->index(sourceRow, MSG_SOURCE_COL_METHOD_TYPE, sourceParent); // method_type
//    QModelIndex indexNotary     = sourceModel()->index(sourceRow, MSG_SOURCE_COL_NOTARY_ID,   sourceParent); // notary_id
//    QModelIndex indexFolder     = sourceModel()->index(sourceRow, MSG_SOURCE_COL_FOLDER,      sourceParent); // folder
////  QModelIndex indexSubject    = sourceModel()->index(sourceRow, MSG_SOURCE_COL_SUBJECT,     sourceParent); // subject
////  QModelIndex indexArchived   = sourceModel()->index(sourceRow, MSG_SOURCE_COL_ARCHIVED,    sourceParent); // is archived?
//    QModelIndex indexHasSubject = sourceModel()->index(sourceRow, MSG_SOURCE_COL_HAS_SUBJECT, sourceParent); // has a real subject (vs. a "display subject" containing intro words from the msg body)

//    QAbstractItemModel * pModel    = sourceModel();
//    ModelMessages      * pMsgModel = dynamic_cast<ModelMessages*>(pModel);

//    if (nullptr != pMsgModel)
//    {
//        // Check the folder here.
//        QVariant dataFolder = pMsgModel->data(indexFolder);
//        const int nFolder = dataFolder.isValid() ? dataFolder.toInt() : (-1);

//        if ((nFolder != -1)  && (nFolder != nFolder_))
//            return false;
//        // ------------------------------------
//        // Grab the data for the current row.
//        //
//        const QVariant dataMethodType       = pMsgModel->data(indexMethodType);
//        const QVariant dataSenderNym        = pMsgModel->data(indexSenderNym);
//        const QVariant dataRecipientNym     = pMsgModel->data(indexRecipNym);
//        const QVariant dataSenderAddress    = pMsgModel->data(indexSenderAddr);
//        const QVariant dataRecipientAddress = pMsgModel->data(indexRecipAddr);
//        const QVariant dataNotaryID         = pMsgModel->data(indexNotary);
////      const QVariant dataSubject          = pMsgModel->data(indexSubject);
//        const QVariant dataHasSubject       = pMsgModel->data(indexHasSubject);

////      const QVariant dataSenderName       = this->data(mapFromSource(indexSenderNym));
////      const QVariant dataRecipientName    = this->data(mapFromSource(indexRecipNym));
////      const QVariant dataNotaryName       = this->data(mapFromSource(indexNotary));

//        const QString qstrMethodType        = dataMethodType.isValid() ? dataMethodType.toString() : "";
//        const QString qstrSenderNym         = dataSenderNym.isValid() ? dataSenderNym.toString() : "";
//        const QString qstrRecipientNym      = dataRecipientNym.isValid() ? dataRecipientNym.toString() : "";
//        const QString qstrSenderAddress     = dataSenderAddress.isValid() ? dataSenderAddress.toString() : "";
//        const QString qstrRecipientAddress  = dataRecipientAddress.isValid() ? dataRecipientAddress.toString() : "";
//        const QString qstrNotaryID          = dataNotaryID.isValid() ? dataNotaryID.toString() : "";
////      const QString qstrSubject           = dataSubject.isValid() ? dataSubject.toString() : "";
//        const bool    bHasSubject           = dataHasSubject.isValid() ? dataHasSubject.toBool() : false;
//        // ------------------------------------
//        // This is because the conversations page should ignore all mail-style messages
//        // (which are the ones that have a real subject).
//        //
//        if (bHasSubject)
//        {
//            return false;
//        }
//        // ------------------------------------
//        // Here we check the filterString (optional string the user can type.)
//        //
//        if (!filterString_.isEmpty())
//        {
////            QModelIndex    indexSubject   = sourceModel()->index(sourceRow, MSG_SOURCE_COL_SUBJECT, sourceParent); // subject
////            const QVariant dataSubject    = pMsgModel->data(indexSubject);
////            const QString  qstrSubject    = dataSubject.isValid()  ? dataSubject.toString() : "";
//            const QString  qstrNotaryName = qstrNotaryID.isEmpty() ? QString("") :
//                                                QString::fromStdString(opentxs::OT::App().Client().Exec().GetServer_Name(qstrNotaryID.toStdString()));
//            MTNameLookupQT theLookup;
//            QString qstrSenderName    = qstrSenderNym   .isEmpty() ? "" : QString::fromStdString(theLookup.GetNymName(qstrSenderNym   .toStdString(), ""));
//            QString qstrRecipientName = qstrRecipientNym.isEmpty() ? "" : QString::fromStdString(theLookup.GetNymName(qstrRecipientNym.toStdString(), ""));

//            if (qstrSenderName.isEmpty() && !qstrSenderAddress.isEmpty())
//                qstrSenderName = QString::fromStdString(theLookup.GetAddressName(qstrSenderAddress.toStdString()));
//            if (qstrRecipientName.isEmpty() && !qstrRecipientAddress.isEmpty())
//                qstrRecipientName = QString::fromStdString(theLookup.GetAddressName(qstrRecipientAddress.toStdString()));

//            if (!qstrMethodType.contains(filterString_) &&
//                !qstrSenderNym.contains(filterString_) &&
//                !qstrRecipientNym.contains(filterString_) &&
//                !qstrSenderAddress.contains(filterString_) &&
//                !qstrRecipientAddress.contains(filterString_) &&
//                !qstrNotaryID.contains(filterString_) &&
//                !qstrSenderName.contains(filterString_) &&
//                !qstrRecipientName.contains(filterString_) &&
//                !qstrNotaryName.contains(filterString_) )
//                return false;
//        }
//        // ------------------------------------
//        // Then check the other stuff:
//        switch (filterType_)
//        {
//        case FilterNone:
//            return false; // If there's no filter (no conversation selected) then the window containing individual chat msgs should be empty.

//        case FilterTopLevel:
//        {
////            const QVariant dataMethodType       = pMsgModel->data(indexMethodType);
////            const QVariant dataSenderNym        = pMsgModel->data(indexSenderNym);
////            const QVariant dataRecipientNym     = pMsgModel->data(indexRecipNym);
////            const QVariant dataSenderAddress    = pMsgModel->data(indexSenderAddr);
////            const QVariant dataRecipientAddress = pMsgModel->data(indexRecipAddr);

////            const QString qstrMethodType       = dataMethodType.isValid() ? dataMethodType.toString() : "";
////            const QString qstrSenderNym        = dataSenderNym.isValid() ? dataSenderNym.toString() : "";
////            const QString qstrRecipientNym     = dataRecipientNym.isValid() ? dataRecipientNym.toString() : "";
////            const QString qstrSenderAddress    = dataSenderAddress.isValid() ? dataSenderAddress.toString() : "";
////            const QString qstrRecipientAddress = dataRecipientAddress.isValid() ? dataRecipientAddress.toString() : "";

//            for (mapIDName::const_iterator ii = mapNymIds_.begin(); ii != mapNymIds_.end(); ii++)
//            {
//                const QString qstrNymID = ii.key();

//                if ( (0 == qstrNymID.compare(qstrSenderNym)) ||
//                     (0 == qstrNymID.compare(qstrRecipientNym)) ) // Match!
//                    return true;
//            }
//            for (mapIDName::const_iterator ii = mapAddresses_.begin(); ii != mapAddresses_.end(); ii++)
//            {
//                QString qstrAddress = ii.key();

//                if ( (0 == qstrAddress.compare(qstrSenderAddress)) ||
//                     (0 == qstrAddress.compare(qstrRecipientAddress)) ) // Match!
//                    return true;
//            }
//            return false;
//        }
//            break;

//        case FilterNotary:
//        {
////            const QVariant dataNotaryID     = pMsgModel->data(indexNotary);
////            const QVariant dataSenderNym    = pMsgModel->data(indexSenderNym);
////            const QVariant dataRecipientNym = pMsgModel->data(indexRecipNym);

////            const QString  qstrNotaryID     = dataNotaryID.isValid() ? dataNotaryID.toString() : "";
////            const QString  qstrSenderNym    = dataSenderNym.isValid() ? dataSenderNym.toString() : "";
////            const QString  qstrRecipientNym = dataRecipientNym.isValid() ? dataRecipientNym.toString() : "";

//            if (!notaryId_.isEmpty() && (qstrNotaryID != notaryId_)) // The row doesn't match our filter.
//                return false;

//            for (mapIDName::const_iterator ii = mapNymIds_.begin(); ii != mapNymIds_.end(); ii++)
//            {
//                const QString qstrNymID = ii.key();

//                if ( (0 == qstrNymID.compare(qstrSenderNym)) ||
//                     (0 == qstrNymID.compare(qstrRecipientNym)) ) // Match!
//                    return true;
//            }
//            return false;
//        }
//            break;

//        default:
//            qDebug() << "Unexpected filter type in ConvMsgsProxyModel::filterAcceptsRow.\n";
//            return false;
//        }
//    }

//    return true;
}

// --------------------------------------------









// ------------------------------------------------------------

MessagesProxyModel::MessagesProxyModel(QObject *parent /*=0*/)
: QSortFilterProxyModel(parent)
{
}

void MessagesProxyModel::setFilterFolder(int nFolder)
{
    nFolder_ = nFolder; // 0 for outbox, 1 for inbox, and 2+ for all other future custom boxes.
    invalidateFilter();
}

void MessagesProxyModel::setFilterNone()
{
    clearFilterType();
    filterType_ = FilterNone;
    // --------------------------------
    invalidateFilter();
}

void MessagesProxyModel::setFilterTopLevel(QString qstrContactID)
{
    clearFilterType();
    filterType_ = FilterTopLevel;
    // --------------------------------
    MTContactHandler::getInstance()->GetNyms(mapNymIds_, qstrContactID.toStdString());
//  MTContactHandler::getInstance()->GetAddressesByContact(mapAddresses_, nContactID, "", false);
    // --------------------------------
    invalidateFilter();
}

void MessagesProxyModel::setFilterTopLevel(int nContactID)
{
    clearFilterType();
    filterType_ = FilterTopLevel;
    // --------------------------------
    MTContactHandler::getInstance()->GetNyms(mapNymIds_, nContactID);
    MTContactHandler::getInstance()->GetAddressesByContact(mapAddresses_, nContactID, "", false);
    // --------------------------------
    invalidateFilter();
}

void MessagesProxyModel::setFilterNotary(QString qstrNotaryId, int nContactID)
{
    clearFilterType();
    filterType_ = FilterNotary;
    // --------------------------------
    notaryId_ = qstrNotaryId;
    MTContactHandler::getInstance()->GetNyms(mapNymIds_, nContactID);
    // --------------------------------
    invalidateFilter();
}

void MessagesProxyModel::setFilterNotary(QString qstrNotaryId, QString qstrContactID)
{
    clearFilterType();
    filterType_ = FilterNotary;
    // --------------------------------
    notaryId_ = qstrNotaryId;
    MTContactHandler::getInstance()->GetNyms(mapNymIds_, qstrContactID.toStdString());
    // --------------------------------
    invalidateFilter();
}

void MessagesProxyModel::setFilterMethodAddress(QString qstrMethodType, QString qstrAddress)
{
    clearFilterType();
    filterType_ = FilterMethodAddress;
    // --------------------------------
    singleMethodType_ = qstrMethodType;
    singleAddress_    = qstrAddress;
    // --------------------------------
    invalidateFilter();
}

void MessagesProxyModel::setFilterString(QString qstrFilter)
{
    filterString_ = qstrFilter;
    invalidateFilter();
}

void MessagesProxyModel::clearFilterType()
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

QVariant MessagesProxyModel::data ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
//    if (role == Qt::TextAlignmentRole)
//        return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
    // ----------------------------------------
    if ( role==Qt::FontRole && index.isValid())
    {
        const int nSourceRow    = headerData(index.row(),    Qt::Vertical,   Qt::UserRole).toInt();
        const int nSourceColumn = MSG_SOURCE_COL_HAVE_READ; // If you haven't read this message, we set it bold.

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

        if (nSourceColumn == MSG_SOURCE_COL_MY_NYM) // my_nym_id
        {
            QString qstrID = sourceData.isValid() ? sourceData.toString() : "";

            std::string str_name;

            if (!qstrID.isEmpty())
            {
                const std::string str_id = qstrID.trimmed().toStdString();
                str_name = str_id.empty() ? "" : opentxs::OT::App().Client().Exec().GetNym_Name(str_id);
            }
            // ------------------------
            if (str_name.empty() && !qstrID.isEmpty())
                return QVariant(qstrID);
            return QVariant(QString::fromStdString(str_name));
        }
//        else if (nSourceColumn == 2) // my_address   (People already see their Nym ID from the above column. They might WANT to see this address.)
//        {
//            QString qstrAddress = QSortFilterProxyModel::data(index,role).toString();
//            QString qstrNymID   = MTContactHandler::getInstance()->GetNymByAddress(qstrAddress);

//            if (!qstrNymID.isEmpty())
//            {
//                const std::string str_id = qstrNymID.toStdString();
//                const std::string str_name = str_id.empty() ? "" : opentxs::OT::App().Client().Exec().GetNym_Name(str_id);
//                // ------------------------
//                if (!str_name.empty())
//                    return QVariant(QString::fromStdString(str_name));
//            }
//            return QSortFilterProxyModel::data(index,role);
//        }
        else if (nSourceColumn == MSG_SOURCE_COL_SENDER_NYM) // sender_nym_id
        {
            QString qstrID = sourceData.isValid() ? sourceData.toString().trimmed() : "";
            const std::string str_id = qstrID.isEmpty() ? "" : qstrID.toStdString();
            const std::string str_name = str_id.empty() ? "" : opentxs::OT::App().Client().Exec().GetNym_Name(str_id);
            // ------------------------
            if (!str_name.empty())
                return QVariant(QString::fromStdString(str_name));

            QModelIndex sibling = sourceIndex.sibling(sourceIndex.row(), MSG_SOURCE_COL_SENDER_ADDR);
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
        else if (nSourceColumn == MSG_SOURCE_COL_RECIP_NYM) // recipient_nym_id
        {
            QString qstrID = sourceData.isValid() ? sourceData.toString().trimmed() : "";
            const std::string str_id = qstrID.isEmpty() ? "" : qstrID.toStdString();
            const std::string str_name = str_id.empty() ? "" : opentxs::OT::App().Client().Exec().GetNym_Name(str_id);
            // ------------------------
            if (!str_name.empty())
                return QVariant(QString::fromStdString(str_name));

            QModelIndex sibling = sourceIndex.sibling(sourceIndex.row(), MSG_SOURCE_COL_RECIP_ADDR);
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
        else if (nSourceColumn == MSG_SOURCE_COL_NOTARY_ID) // notary_id
        {
            // If method type is otserver, we put the notary Name here.
            // Else if the method t
            QString qstrID = sourceData.isValid() ? sourceData.toString() : "";
            const std::string str_id = qstrID.toStdString();
            const std::string str_name = str_id.empty() ? "" : opentxs::OT::App().Client().Exec().GetServer_Name(str_id);
            // ------------------------
            if (!str_name.empty())
                return QVariant(QString::fromStdString(str_name));

            if (!qstrID.isEmpty())
                return QVariant(qstrID);

            return sourceData;
        }
        else if (nSourceColumn == MSG_SOURCE_COL_METHOD_TYPE_DISP) // Method type display (if blank, we see if there's a notary ID. If so, then transport shows "otserver".)
        {
            QString qstrType = sourceData.isValid() ? sourceData.toString() : "";
            // ------------------------
            if (!qstrType.isEmpty())
                return QVariant(qstrType);
            return QVariant(QString("opentxs"));
        }
        else if (nSourceColumn == MSG_SOURCE_COL_TIMESTAMP) // timestamp
        {
            time64_t the_time = sourceData.isValid() ? sourceData.toLongLong() : 0;
            QDateTime timestamp;
            timestamp.setTime_t(the_time);
            return QVariant(QString(timestamp.toString(Qt::SystemLocaleShortDate)));
        }
        else if (nSourceColumn == MSG_SOURCE_COL_FOLDER) // folder
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

        const int nSourceColumnReplied   = MSG_SOURCE_COL_HAVE_REPLIED;
        const int nSourceColumnForwarded = MSG_SOURCE_COL_HAVE_FORWARDED;

        QModelIndex sourceIndexReplied   = sourceModel()->index(nSourceRow, nSourceColumnReplied);
        QModelIndex sourceIndexForwarded = sourceModel()->index(nSourceRow, nSourceColumnForwarded);

        QVariant    sourceDataReplied    = sourceModel()->data(sourceIndexReplied,   Qt::DisplayRole);
        QVariant    sourceDataForwarded  = sourceModel()->data(sourceIndexForwarded, Qt::DisplayRole);

        if (nSourceColumn == MSG_SOURCE_COL_SUBJECT)
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


QVariant MessagesProxyModel::headerData(int section, Qt::Orientation orientation, int role/*=Qt::DisplayRole*/) const
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
    }

    return varHeaderData;
}

// --------------------------------------------

bool MessagesProxyModel::filterAcceptsColumn(int source_column, const QModelIndex & source_parent) const
{
    bool bReturn = true;

    switch (source_column)
    {
    case MSG_SOURCE_COL_MSG_ID:   bReturn = false;  break; // message_id
    case MSG_SOURCE_COL_MY_NYM:  // my_nym_id
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
    case MSG_SOURCE_COL_MY_ADDR:
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
    case MSG_SOURCE_COL_SENDER_NYM:
    {
        if (nFolder_ == 0) // 0 is the outbox. (In the outbox, you don't show the sender, but the recipient. So we hide the sender column.)
            return false;
        else
            bReturn = true;
    }
        break;
    case MSG_SOURCE_COL_SENDER_ADDR:
    {
        if (FilterNotary == filterType_) // If message was sent through a notary, then it DEFINITELY won't have a Bitmessage address.
            bReturn = false;
        else if (nFolder_ == 0) // 0 is the outbox. (In the outbox, you don't show the sender, but the recipient. So we hide the sender column.)
            return false;
        else
            bReturn = false; // Current expected behavior.
    }
        break;
    case MSG_SOURCE_COL_RECIP_NYM:
    {
        if (nFolder_ > 0) // All other boxes except outbox, hide recipient. (We normally see a recipient in the outbox, instead of a sender.)
            return false;
        else
            bReturn = true;
    }
        break;
    case MSG_SOURCE_COL_RECIP_ADDR:
    {
        if (FilterNotary == filterType_) // If message was sent through a notary, then it DEFINITELY won't have a Bitmessage address.
            bReturn = false;
        else if (nFolder_ > 0) // All other boxes except outbox, hide recipient. (We normally see a recipient in the outbox, instead of a sender.)
            return false;
        else
            bReturn = false; // Current expected behavior.
    }
        break;
    case MSG_SOURCE_COL_METHOD_TYPE:      bReturn = false;  break;
    case MSG_SOURCE_COL_METHOD_TYPE_DISP:
    {
        if (FilterNone == filterType_)
            bReturn = true;
        else if (FilterTopLevel == filterType_)
            bReturn = true;
        else
            bReturn = false;
    }
        break;
    case MSG_SOURCE_COL_NOTARY_ID:
    {
        if (FilterMethodAddress == filterType_)
            bReturn = false;
        else if (FilterNotary == filterType_)
            bReturn = false;
        else
            bReturn = true;
    }
        break;
    case MSG_SOURCE_COL_TIMESTAMP:       bReturn = true;   break;
    case MSG_SOURCE_COL_HAVE_READ:       bReturn = false;  break;
    case MSG_SOURCE_COL_HAVE_REPLIED:    bReturn = false;  break;
    case MSG_SOURCE_COL_HAVE_FORWARDED:  bReturn = false;  break;
    case MSG_SOURCE_COL_SUBJECT:         bReturn = true;   break;
    case MSG_SOURCE_COL_FOLDER:
    case MSG_SOURCE_COL_ARCHIVED:
    case MSG_SOURCE_COL_HAS_SUBJECT:
    {
        bReturn = false;
    }
        break;
    default:  bReturn = true;  break;
    }
    return bReturn;
}

/*
QString create_message_body_table = "CREATE TABLE IF NOT EXISTS message_body"
       "(message_id INTEGER PRIMARY KEY,"
       " body TEXT"
       ")";
*/


// What do we filter on here?
// Well, the message table doesn't contain a Contact ID.
// Instead, it contains a NymID and/or a Bitmessage address and/or a notary address.
//
// When the user clicks on a top-level Contact ID, we need to filter on ALL known Nyms AND Bitmessage addresses for that Contact.
// Any single one of them could be the one on the message, that causes the filter to approve it.
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
bool MessagesProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex indexSenderNym  = sourceModel()->index(sourceRow, MSG_SOURCE_COL_SENDER_NYM,  sourceParent); // sender_nym_id
    QModelIndex indexSenderAddr = sourceModel()->index(sourceRow, MSG_SOURCE_COL_SENDER_ADDR, sourceParent); // sender_address
    QModelIndex indexRecipNym   = sourceModel()->index(sourceRow, MSG_SOURCE_COL_RECIP_NYM,   sourceParent); // recipient_nym_id
    QModelIndex indexRecipAddr  = sourceModel()->index(sourceRow, MSG_SOURCE_COL_RECIP_ADDR,  sourceParent); // recipient_address
    QModelIndex indexMethodType = sourceModel()->index(sourceRow, MSG_SOURCE_COL_METHOD_TYPE, sourceParent); // method_type
    QModelIndex indexNotary     = sourceModel()->index(sourceRow, MSG_SOURCE_COL_NOTARY_ID,   sourceParent); // notary_id
    QModelIndex indexFolder     = sourceModel()->index(sourceRow, MSG_SOURCE_COL_FOLDER,      sourceParent); // folder
//  QModelIndex indexSubject    = sourceModel()->index(sourceRow, MSG_SOURCE_COL_SUBJECT,     sourceParent); // subject
    QModelIndex indexArchived   = sourceModel()->index(sourceRow, MSG_SOURCE_COL_ARCHIVED,    sourceParent); // is archived?
    QModelIndex indexHasSubject = sourceModel()->index(sourceRow, MSG_SOURCE_COL_HAS_SUBJECT, sourceParent); // has a real subject (vs. a "display subject" containing intro words from the msg body)


    QAbstractItemModel * pModel    = sourceModel();
    ModelMessages      * pMsgModel = dynamic_cast<ModelMessages*>(pModel);

    if (nullptr != pMsgModel)
    {
        // Check the folder here.
        QVariant dataFolder = pMsgModel->data(indexFolder);
        const int nFolder = dataFolder.isValid() ? dataFolder.toInt() : (-1);

        if ((nFolder != -1)  && (nFolder != nFolder_))
            return false;
        // ------------------------------------
        QVariant dataArchived = pMsgModel->data(indexArchived);
        const bool bArchived = dataArchived.isValid() ? (0 != dataArchived.toInt()) : false;
        if (bArchived)
            return false;
        // ------------------------------------
        // Grab the data for the current row.
        //
        const QVariant dataMethodType       = pMsgModel->data(indexMethodType);
        const QVariant dataSenderNym        = pMsgModel->data(indexSenderNym);
        const QVariant dataRecipientNym     = pMsgModel->data(indexRecipNym);
        const QVariant dataSenderAddress    = pMsgModel->data(indexSenderAddr);
        const QVariant dataRecipientAddress = pMsgModel->data(indexRecipAddr);
        const QVariant dataNotaryID         = pMsgModel->data(indexNotary);
//      const QVariant dataSubject          = pMsgModel->data(indexSubject);
        const QVariant dataHasSubject       = pMsgModel->data(indexHasSubject);

//      const QVariant dataSenderName       = this->data(mapFromSource(indexSenderNym));
//      const QVariant dataRecipientName    = this->data(mapFromSource(indexRecipNym));
//      const QVariant dataNotaryName       = this->data(mapFromSource(indexNotary));

        const QString qstrMethodType        = dataMethodType.isValid() ? dataMethodType.toString() : "";
        const QString qstrSenderNym         = dataSenderNym.isValid() ? dataSenderNym.toString() : "";
        const QString qstrRecipientNym      = dataRecipientNym.isValid() ? dataRecipientNym.toString() : "";
        const QString qstrSenderAddress     = dataSenderAddress.isValid() ? dataSenderAddress.toString() : "";
        const QString qstrRecipientAddress  = dataRecipientAddress.isValid() ? dataRecipientAddress.toString() : "";
        const QString qstrNotaryID          = dataNotaryID.isValid() ? dataNotaryID.toString() : "";
//      const QString qstrSubject           = dataSubject.isValid() ? dataSubject.toString() : "";
        const bool    bHasSubject           = dataHasSubject.isValid() ? dataHasSubject.toBool() : false;
        // ------------------------------------
        // This is because the messages page should ignore all SMS-style conversation messages
        // (which are the ones that don't have a real subject -- even though their "subject" field may
        // have been populated in the GUI DB to contain the intro words from the msg body, for display
        // purposes on the mail screen, the message doesn't necessarily have an ACTUAL subject, meaning
        // it should be instead displayed on the "chat" screen, not the "mail" screen).
        //
        if (!bHasSubject)
        {
            return false;
        }
        // ------------------------------------
        // Here we check the filterString (optional string the user can type.)
        //
        if (!filterString_.isEmpty())
        {
            QModelIndex    indexSubject   = sourceModel()->index(sourceRow, MSG_SOURCE_COL_SUBJECT, sourceParent); // subject
            const QVariant dataSubject    = pMsgModel->data(indexSubject);
            const QString  qstrSubject    = dataSubject.isValid()  ? dataSubject.toString() : "";
            const QString  qstrNotaryName = qstrNotaryID.isEmpty() ? QString("") :
                                                QString::fromStdString(opentxs::OT::App().Client().Exec().GetServer_Name(qstrNotaryID.toStdString()));
            QString qstrSenderName    = qstrSenderNym   .isEmpty() ? "" : QString::fromStdString(opentxs::OT::App().Client().Exec().GetNym_Name(qstrSenderNym   .toStdString()));
            QString qstrRecipientName = qstrRecipientNym.isEmpty() ? "" : QString::fromStdString(opentxs::OT::App().Client().Exec().GetNym_Name(qstrRecipientNym.toStdString()));

            if (qstrSenderName.isEmpty() && !qstrSenderAddress.isEmpty())
                qstrSenderName = qstrSenderAddress;
            if (qstrRecipientName.isEmpty() && !qstrRecipientAddress.isEmpty())
                qstrRecipientName = qstrRecipientAddress;

            if (!qstrSubject.contains(filterString_) &&
                !qstrMethodType.contains(filterString_) &&
                !qstrSenderNym.contains(filterString_) &&
                !qstrRecipientNym.contains(filterString_) &&
                !qstrSenderAddress.contains(filterString_) &&
                !qstrRecipientAddress.contains(filterString_) &&
                !qstrNotaryID.contains(filterString_) &&
                !qstrSenderName.contains(filterString_) &&
                !qstrRecipientName.contains(filterString_) &&
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
            qDebug() << "Unexpected filter type in MessagesProxyModel::filterAcceptsRow.\n";
            return false;
        }
    }

    return true;
}

// --------------------------------------------

QVariant MessagesProxyModel::rawData ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
    return QSortFilterProxyModel::data(index,role);
}

// ------------------------------------------------------------

ModelMessages::ModelMessages(QObject * parent /*= 0*/, QSqlDatabase db /*=QSqlDatabase()*/)
: QSqlTableModel(parent, db) {}


// I'm overriding this so I can return the ACTUAL row or column back (depending on orientation) from the source
// model. This way, the proxy model can call this to find out the actual column or row, whenever it needs to.
//
QVariant ModelMessages::headerData(int section, Qt::Orientation orientation, int role /*=Qt::DisplayRole*/) const
{
    if (role==Qt::UserRole)
    {
        return section;
    }

    return QSqlTableModel::headerData(section, orientation, role);
}

bool ModelMessages::setData(const QModelIndex & index, const QVariant & value, int role /*=Qt::EditRole*/)
{
    bool success = false;

    if (role==Qt::DisplayRole && value.isValid())
    {
//        if (index.column() == MSG_SOURCE_COL_MY_ADDR) // my_address
//        {
//            success = QSqlTableModel::setData(index,QVariant(MTContactHandler::Encode(value.toString())),role);
//        }
//        else if (index.column() == MSG_SOURCE_COL_SENDER_ADDR) // sender_address
//        {
//            success = QSqlTableModel::setData(index,QVariant(MTContactHandler::Encode(value.toString())),role);
//        }
//        else if (index.column() == MSG_SOURCE_COL_RECIP_ADDR) // recipient_address
//        {
//            success = QSqlTableModel::setData(index,QVariant(MTContactHandler::Encode(value.toString())),role);
//        }
//        else if (index.column() == MSG_SOURCE_COL_METHOD_TYPE) // method_type
//        {
//            success = QSqlTableModel::setData(index,QVariant(MTContactHandler::Encode(value.toString())),role);
//        }
//        else if (index.column() == MSG_SOURCE_COL_METHOD_TYPE_DISP) // method_type_display
//        {
//            success = QSqlTableModel::setData(index,QVariant(MTContactHandler::Encode(value.toString())),role);
//        }
        if (index.column() == MSG_SOURCE_COL_SUBJECT) // subject
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

QVariant ModelMessages::rawData ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
    return QSqlTableModel::data(index,role);
}

QVariant ModelMessages::data ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
    if (role == Qt::TextAlignmentRole) // I don't want to center the subject column.
    {
        if (index.column() == MSG_SOURCE_COL_SUBJECT)
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
//        if (index.column() == MSG_SOURCE_COL_HAVE_REPLIED ||
//            index.column() == MSG_SOURCE_COL_HAVE_FORWARDED)
//            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
    }
    // ----------------------------------------
    if (role==Qt::DisplayRole && QSqlTableModel::data(index,role).isValid())
    {
//        if (index.column() == MSG_SOURCE_COL_MY_ADDR) // my_address
//        {
//            return QVariant(MTContactHandler::Decode(QSqlTableModel::data(index,role).toString()));
//        }
//        else if (index.column() == MSG_SOURCE_COL_SENDER_ADDR) // sender_address
//        {
//            return QVariant( MTContactHandler::Decode(QSqlTableModel::data(index,role).toString()));
//        }
//        else if (index.column() == MSG_SOURCE_COL_RECIP_ADDR) // recipient_address
//        {
//            return QVariant( MTContactHandler::Decode(QSqlTableModel::data(index,role).toString()));
//        }
//        else if (index.column() == MSG_SOURCE_COL_METHOD_TYPE) // method_type
//        {
//            return QVariant( MTContactHandler::Decode(QSqlTableModel::data(index,role).toString()));
//        }
//        else if (index.column() == MSG_SOURCE_COL_METHOD_TYPE_DISP) // method_type_display
//        {
//            return QVariant( MTContactHandler::Decode(QSqlTableModel::data(index,role).toString()));
//        }
        if (index.column() == MSG_SOURCE_COL_SUBJECT) // subject
        {
            QVariant qvarData    = QSqlTableModel::data(index,role);
            QString  qstrData    = qvarData.isValid() ? qvarData.toString() : "";
            QString  qstrDecoded = qstrData.isEmpty() ? "" : MTContactHandler::Decode(qstrData);
            return QVariant(qstrDecoded);
        }
    }

    return QSqlTableModel::data(index,role);
}
