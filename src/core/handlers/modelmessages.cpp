#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/moneychanger.hpp>
#include <core/handlers/DBHandler.hpp>
#include <core/handlers/contacthandler.hpp>

#include <core/handlers/modelmessages.hpp>

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
                str_name = str_id.empty() ? "" : opentxs::OTAPI_Wrap::It()->GetNym_Name(str_id);
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
//                const std::string str_name = str_id.empty() ? "" : opentxs::OTAPI_Wrap::It()->GetNym_Name(str_id);
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
            MTNameLookupQT theLookup;
            const std::string str_name = str_id.empty() ? "" : theLookup.GetNymName(str_id, "");
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
            MTNameLookupQT theLookup;
            const std::string str_name = str_id.empty() ? "" : theLookup.GetNymName(str_id, "");
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
            const std::string str_name = str_id.empty() ? "" : opentxs::OTAPI_Wrap::It()->GetServer_Name(str_id);
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
        else if (nSourceColumn == MSG_SOURCE_COL_HAVE_REPLIED) // have_replied
        {
            bool bHaveI = sourceData.isValid() ? sourceData.toBool() : false;
            return QVariant(QString(bHaveI ? "Replied" : ""));
        }
        else if (nSourceColumn == MSG_SOURCE_COL_HAVE_FORWARDED) // have_forwarded
        {
            bool bHaveI = sourceData.isValid() ? sourceData.toBool() : false;
            return QVariant(QString(bHaveI ? "Forwarded" : ""));
        }
        else if (nSourceColumn == MSG_SOURCE_COL_FOLDER) // folder
        {
            int nFolder = sourceData.isValid() ? sourceData.toInt() : 0;
            return QVariant(tr(0 == nFolder ? "Sent" : "Receieved"));
        }
    }
    // -------------------------------
//    else if (role==Qt::DecorationRole && index.isValid())
//    {
//        const int nSourceRow    = headerData(index.row(),    Qt::Vertical,   Qt::UserRole).toInt();
//        const int nSourceColumn = headerData(index.column(), Qt::Horizontal, Qt::UserRole).toInt();

//        QModelIndex sourceIndex = sourceModel()->index(nSourceRow, nSourceColumn);
//        QVariant    sourceData  = sourceModel()->data(sourceIndex, role);

//        if (nSourceColumn == MSG_SOURCE_COL_HAVE_REPLIED) // have_replied
//        {
//            bool bHaveI = sourceData.isValid() ? sourceData.toBool() : false;
////            if (bHaveI)
//            {
//                QPixmap pixmapReply(":/icons/icons/reply.png");
//                return pixmapReply;
//            }
//        }
//        else if (nSourceColumn == MSG_SOURCE_COL_HAVE_FORWARDED) // have_forwarded
//        {
//            bool bHaveI = sourceData.isValid() ? sourceData.toBool() : false;
////            if (bHaveI)
//            {
//                QPixmap pixmapReply(":/icons/sendfunds");
//                return pixmapReply;
//            }
//        }
//    }
    // --------------------------------------------
//    else if (role == Qt::SizeHintRole)
//    {
////      const int nSourceRow    = headerData(index.row(),    Qt::Vertical,   Qt::UserRole).toInt();
//        const int nSourceColumn = headerData(index.column(), Qt::Horizontal, Qt::UserRole).toInt();

//        if (nSourceColumn == MSG_SOURCE_COL_HAVE_REPLIED) // have_replied
//        {
//            return QSize (16,16);
//        }
//        else if (nSourceColumn == MSG_SOURCE_COL_HAVE_FORWARDED) // have_forwarded
//        {
//            return QSize (16,16);
//        }
//    }

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
/*
QString create_message_body_table = "CREATE TABLE IF NOT EXISTS message_body"
       "(message_id INTEGER PRIMARY KEY,"
       " body TEXT"
       ")";*/

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
    case MSG_SOURCE_COL_TIMESTAMP:       bReturn = true;  break;
    case MSG_SOURCE_COL_HAVE_READ:       bReturn = false;  break;
    case MSG_SOURCE_COL_HAVE_REPLIED:    bReturn = false;  break;
    case MSG_SOURCE_COL_HAVE_FORWARDED:  bReturn = false;  break;
//    case MSG_SOURCE_COL_HAVE_REPLIED:    bReturn = true;  break;  //coming soon.
//    case MSG_SOURCE_COL_HAVE_FORWARDED:  bReturn = true;  break;
    case MSG_SOURCE_COL_SUBJECT:         bReturn = true;  break;
    case MSG_SOURCE_COL_FOLDER:
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
    QModelIndex indexSubject    = sourceModel()->index(sourceRow, MSG_SOURCE_COL_SUBJECT,     sourceParent); // subject

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
        // Grab the data for the current row.
        //
        const QVariant dataMethodType       = pMsgModel->data(indexMethodType);
        const QVariant dataSenderNym        = pMsgModel->data(indexSenderNym);
        const QVariant dataRecipientNym     = pMsgModel->data(indexRecipNym);
        const QVariant dataSenderAddress    = pMsgModel->data(indexSenderAddr);
        const QVariant dataRecipientAddress = pMsgModel->data(indexRecipAddr);
        const QVariant dataNotaryID         = pMsgModel->data(indexNotary);
        const QVariant dataSubject          = pMsgModel->data(indexSubject);

//        const QVariant dataSenderName       = this->data(mapFromSource(indexSenderNym));
//        const QVariant dataRecipientName    = this->data(mapFromSource(indexRecipNym));
//        const QVariant dataNotaryName       = this->data(mapFromSource(indexNotary));

        const QString qstrMethodType       = dataMethodType.isValid() ? dataMethodType.toString() : "";
        const QString qstrSenderNym        = dataSenderNym.isValid() ? dataSenderNym.toString() : "";
        const QString qstrRecipientNym     = dataRecipientNym.isValid() ? dataRecipientNym.toString() : "";
        const QString qstrSenderAddress    = dataSenderAddress.isValid() ? dataSenderAddress.toString() : "";
        const QString qstrRecipientAddress = dataRecipientAddress.isValid() ? dataRecipientAddress.toString() : "";
        const QString qstrNotaryID         = dataNotaryID.isValid() ? dataNotaryID.toString() : "";
        // ------------------------------------
        // Here we check the filterString (optional string the user can type.)
        //
        if (!filterString_.isEmpty())
        {
            const QString qstrSubject = dataSubject.isValid() ? dataSubject.toString() : "";

            const QString qstrNotaryName = qstrNotaryID.isEmpty() ? QString("") :
                                           QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetServer_Name(qstrNotaryID.toStdString()));
            MTNameLookupQT theLookup;
            QString qstrSenderName    = qstrSenderNym   .isEmpty() ? "" : QString::fromStdString(theLookup.GetNymName(qstrSenderNym   .toStdString(), ""));
            QString qstrRecipientName = qstrRecipientNym.isEmpty() ? "" : QString::fromStdString(theLookup.GetNymName(qstrRecipientNym.toStdString(), ""));

            if (qstrSenderName.isEmpty() && !qstrSenderAddress.isEmpty())
                qstrSenderName = QString::fromStdString(theLookup.GetAddressName(qstrSenderAddress.toStdString()));
            if (qstrRecipientName.isEmpty() && !qstrRecipientAddress.isEmpty())
                qstrRecipientName = QString::fromStdString(theLookup.GetAddressName(qstrRecipientAddress.toStdString()));

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


QVariant ModelMessages::rawData ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
    return QSqlTableModel::data(index,role);
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

QVariant ModelMessages::data ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
    if (role == Qt::TextAlignmentRole) // I don't want to center the subject column.
    {
        if (index.column() == MSG_SOURCE_COL_SUBJECT)
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
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
            return QVariant( MTContactHandler::Decode(QSqlTableModel::data(index,role).toString()));
        }
    }

    return QSqlTableModel::data(index,role);
}
