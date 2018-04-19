#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/moneychanger.hpp>
#include <core/handlers/DBHandler.hpp>
#include <core/handlers/contacthandler.hpp>

#include <core/handlers/modeltradearchive.hpp>

#include <opentxs/opentxs.hpp>

#include <QDebug>
#include <QtGlobal>
#include <QSqlTableModel>
#include <QDateTime>
#include <Qt>

FinalReceiptProxyModel::FinalReceiptProxyModel(QObject *parent /*= 0*/)
: QSortFilterProxyModel(parent)
{
}

bool FinalReceiptProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QAbstractItemModel * pModel = sourceModel();
    ModelTradeArchive  * pTradeModel = nullptr;

    if (nullptr != pModel)
        pTradeModel = dynamic_cast<ModelTradeArchive*>(pModel);

    if (nullptr != pTradeModel)
    {
        QModelIndex offerIndex  = pModel->index(sourceRow, 6, sourceParent); // OfferID at index 6  todo hardcoding
        QModelIndex nymIndex    = pModel->index(sourceRow, 9, sourceParent); // NymID at index 9  todo hardcoding
        QModelIndex notaryIndex = pModel->index(sourceRow, 8, sourceParent); // Notary at index 8  todo hardcoding

        // NOTE: What's happening is, it's ACTUALLY importing the finalReceipt FIRST,
        // and THEN it's importing the marketReceipt (which actually happened first).
        // So the marketReceipt import code is just ASSUMING that it's first and that
        // no other records exist (only the finalReceipt import code checks to see if
        // there are pre-existing marketReceipt records).
        // So the solution (theoretically) is to just make sure when the marketReceipts
        // are imported, that they check for a pre-existing finalReceipt record at that
        // time.
        //
        // The alternate theory (which I was going with first) is that when the finalReceipt
        // is imported, the filter fails to find the record for the other receipts (even
        // though it supposedly already exists). Therefore causing the finalReceipt to be
        // imported as its own record instead of being added to the pre-existing record.
        // This theory is still plausible but, having examined the filter, and given the fact
        // that the filter WORKS most of the time, I'd have to place my bet on the first
        // described theory at this point.

        QString qstrBlah(offerIndex.isValid() ? "Offer index is VALID" : "Offer index is INVALID");
        qDebug() << qstrBlah;

        int64_t lCurrentOfferID    = pTradeModel->rawData(offerIndex).toLongLong();
        const QString qstrNymID    = pTradeModel->rawData(nymIndex).toString();
        const QString qstrNotaryID = pTradeModel->rawData(notaryIndex).toString();
        const std::string strNymId = qstrNymID.toStdString();
        const std::string strNotaryId = qstrNotaryID.toStdString();

        const bool bNymIdMatches = //str_nym_id_.empty() ||
                                    (0 == strNymId.compare(str_nym_id_));
        const bool bNotaryIdMatches = //str_notary_id_.empty() ||
                                    (0 == strNotaryId.compare(str_notary_id_));

        const bool bMatches = ((lTransNumForDisplay_ == lCurrentOfferID) && bNymIdMatches && bNotaryIdMatches);

        QString qstrMatches = QString(bMatches ? "MATCHING %1" : "mis-match %1").
                arg(bNymIdMatches ? "(+ Nym ID does match)" : "(+ Nym ID does NOT match)");
        QString qstrFinal(bIsFinalReceipt_ ? "TRUE" : "FALSE");
        QString qstrNotaryMatches = QString(bNotaryIdMatches ? "True" : "False");

        qDebug() << QString("%5 RECEIPT: lTransNumForDisplay_ = %1  lCurrentOfferID = %2  lClosingNum_ = %3  lTransNum_: %4 IsFinalReceipt: %6 Notary Matches: %7").
                    arg(lTransNumForDisplay_).arg(lCurrentOfferID).arg(lClosingNum_).arg(lTransNum_).arg(qstrMatches).arg(qstrFinal).arg(qstrNotaryMatches);

        return bMatches;
    }
    return false;

    // NOTE: Here's the code in the filterRow for the other (non-final) receipts:

//    int64_t lCurrentOfferID  = pTradeModel->rawData(index6).toLongLong();
//    QString qstrCurrentNymID = pTradeModel->rawData(index9).toString();

//    return (( lCurrentOfferID  ==  offerId_) &&
//            (0 == qstrCurrentNymID.compare(nymId_)));

}

void FinalReceiptProxyModel::setFilter(int64_t lTransNumForDisplay, std::string str_nym_id, std::string str_notary_id)
{
    lTransNum_ = 0;
    lTransNumForDisplay_ = lTransNumForDisplay; // OfferId
    str_nym_id_ = str_nym_id;
    str_notary_id_ = str_notary_id;

    bIsFinalReceipt_ = false;
    lClosingNum_ = 0;
    bGotClosingNum_ = false;

    invalidateFilter();
}


void FinalReceiptProxyModel::setFilterOpentxsRecord(opentxs::OTRecord& recordmt)
{
    lTransNum_           = recordmt.GetTransactionNum();
    lTransNumForDisplay_ = recordmt.GetTransNumForDisplay();
    str_nym_id_          = recordmt.GetNymID();
    str_notary_id_       = recordmt.GetNotaryID();
    bIsFinalReceipt_     = recordmt.IsFinalReceipt();
    lClosingNum_         = 0;
    bGotClosingNum_      = recordmt.GetClosingNum(lClosingNum_);

    invalidateFilter();
}


void FinalReceiptProxyModel::clearFilter()
{
    lTransNum_           = 0;
    lTransNumForDisplay_ = 0;
    str_nym_id_          = "";
    str_notary_id_       = "";
    bIsFinalReceipt_     = false;
    lClosingNum_         = 0;
    bGotClosingNum_      = false;

    invalidateFilter();
}

TradeArchiveProxyModel::TradeArchiveProxyModel(QObject *parent /*=0*/)
: QSortFilterProxyModel(parent)
{
}

void TradeArchiveProxyModel::setFilterNymId(const QString &nymId)
{
    nymId_ = nymId;
    invalidateFilter();
}

void TradeArchiveProxyModel::setFilterOfferId(const int64_t offerId)
{
    offerId_ = offerId;
    invalidateFilter();
}

void TradeArchiveProxyModel::setFilterIsBid(const bool isBid)
{
    isBid_ = isBid;
    invalidateFilter();
}

QVariant TradeArchiveProxyModel::headerData(int section, Qt::Orientation orientation, int role/*=Qt::DisplayRole*/) const
{
    if (Qt::Horizontal == orientation && Qt::DisplayRole == role)
    {
        if (2 == section)
        {
            if (isBid_)
                return QVariant(tr("Currency Paid"));
            else
                return QVariant(tr("Currency Received"));
        }
        if (3 == section)
        {
            if (isBid_)
                return QVariant(tr("Assets Purchased"));
            else
                return QVariant(tr("Assets Sold"));
        }
    }

    return QSortFilterProxyModel::headerData(section, orientation, role);
}

bool TradeArchiveProxyModel::filterAcceptsColumn(int source_column, const QModelIndex & source_parent) const
{
    bool bReturn = false;

    switch (source_column)
    {
    case 0:   bReturn = false; break;
    case 1:   bReturn = true;  break;
    case 2:   bReturn = true;  break;
    case 3:   bReturn = true;  break;
    case 4:   bReturn = true;  break;
    case 5:   bReturn = true;  break;
    case 6:   bReturn = false; break;
    case 7:   bReturn = true;  break;
    case 8:   bReturn = false; break;
    case 9:   bReturn = false; break;
    case 10:  bReturn = false; break;
    case 11:  bReturn = false; break;
    case 12:  bReturn = false; break;
    case 13:  bReturn = false; break;
    case 14:  bReturn = true;  break;
    case 15:  bReturn = true;  break;
    case 16:  bReturn = true;  break;
    default:  bReturn = true;  break;
    }
    return bReturn;
}

bool TradeArchiveProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index6 = sourceModel()->index(sourceRow, 6, sourceParent); // OfferID
    QModelIndex index9 = sourceModel()->index(sourceRow, 9, sourceParent); // NymID

    QAbstractItemModel * pModel = sourceModel();
    ModelTradeArchive  * pTradeModel = dynamic_cast<ModelTradeArchive*>(pModel);

    int64_t lCurrentOfferID  = pTradeModel->rawData(index6).toLongLong();
    QString qstrCurrentNymID = pTradeModel->rawData(index9).toString();

    return (( lCurrentOfferID  ==  offerId_) &&
            (0 == qstrCurrentNymID.compare(nymId_)));
}


// ------------------------------------------------------------


ModelTradeArchive::ModelTradeArchive(QObject * parent /*= 0*/, QSqlDatabase db/* = QSqlDatabase()*/)
: QSqlTableModel(parent,db) {}


QVariant ModelTradeArchive::rawData ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
    return QSqlTableModel::data(index,role);
}

QVariant ModelTradeArchive::data ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
    if (role == Qt::TextAlignmentRole )
        return QVariant(Qt::AlignCenter | Qt::AlignVCenter);

    if (role==Qt::DisplayRole)
    {
        if (index.column() == 0) // bid/ask
        {
            bool bIsBid = QSqlTableModel::data(index, role).toBool();
            const std::string str_is_bid = bIsBid ? "Bid" : "Ask";
            QString qstrDisplay = QString::fromStdString(str_is_bid);
            return QVariant(qstrDisplay);
        }
        else if (index.column() == 1) // actual price
        {
            int64_t lPrice = QSqlTableModel::data(index, role).toLongLong();

            QModelIndex sibling = index.sibling(index.row(), 13);

            QString qstrCurrencyID = QSqlTableModel::data(sibling,role).toString();

            const std::string str_currency_id = qstrCurrencyID.toStdString();

            std::string str_display = opentxs::OT::App().API().Exec().FormatAmount(str_currency_id, lPrice);
            QString qstrDisplay = QString::fromStdString(str_display);

            return QVariant(qstrDisplay);
        }
        else if (index.column() == 2) // scale
        {
            int64_t lPrice = QSqlTableModel::data(index, role).toLongLong();

            QModelIndex sibling = index.sibling(index.row(), 12);

            QString qstrCurrencyID = QSqlTableModel::data(sibling,role).toString();
            const std::string str_currency_id = qstrCurrencyID.toStdString();

            std::string str_display = opentxs::OT::App().API().Exec().FormatAmount(str_currency_id, lPrice);
            QString qstrDisplay = QString::fromStdString(str_display);

            return QVariant(qstrDisplay);
        }
        else if (index.column() == 3) // actual_paid
        {
            int64_t lPrice = QSqlTableModel::data(index, role).toLongLong();

            QModelIndex sibling = index.sibling(index.row(), 13);

            QString qstrCurrencyID = QSqlTableModel::data(sibling,role).toString();

            const std::string str_currency_id = qstrCurrencyID.toStdString();

            std::string str_display = opentxs::OT::App().API().Exec().FormatAmount(str_currency_id, lPrice);
            QString qstrDisplay = QString::fromStdString(str_display);

            return QVariant(qstrDisplay);
        }
        else if (index.column() == 4) // amount_purchased
        {
            int64_t lPrice = QSqlTableModel::data(index, role).toLongLong();

            QModelIndex sibling = index.sibling(index.row(), 12);

            QString qstrCurrencyID = QSqlTableModel::data(sibling,role).toString();
            const std::string str_currency_id = qstrCurrencyID.toStdString();

            std::string str_display = opentxs::OT::App().API().Exec().FormatAmount(str_currency_id, lPrice);
            QString qstrDisplay = QString::fromStdString(str_display);

            return QVariant(qstrDisplay);
        }
        else if (index.column() == 5) // timestamp
        {
            time64_t the_time = QSqlTableModel::data(index, role).toLongLong();
            QDateTime timestamp;
            timestamp.setTime_t(the_time);
            return QVariant(QString(timestamp.toString(Qt::SystemLocaleShortDate)));
        }
        else if (index.column() == 6)
        {
            return QSqlTableModel::data(index,role);
        }
        else if (index.column() == 7)
        {
            return QSqlTableModel::data(index,role);
        }
        else if (index.column() == 8) //notary ID
        {
            QString qstrID = QSqlTableModel::data(index,role).toString();
            const std::string str_id = qstrID.toStdString();
            const std::string str_name = opentxs::OT::App().API().Exec().GetServer_Name(str_id);
            // ------------------------
            if (str_name.empty())
                return QSqlTableModel::data(index,role);
            return QVariant(QString::fromStdString(str_name));
        }
        else if (index.column() == 9) // nym id
        {
            QString qstrID = QSqlTableModel::data(index,role).toString();
            const std::string str_id = qstrID.toStdString();
            const std::string str_name = opentxs::OT::App().API().Exec().GetNym_Name(str_id);
            // ------------------------
            if (str_name.empty())
                return QSqlTableModel::data(index,role);
            return QVariant(QString::fromStdString(str_name));
        }
        else if (index.column() == 10) // asset_acct_id
        {
            QString qstrID = QSqlTableModel::data(index,role).toString();
            const std::string str_id = qstrID.toStdString();
            std::string str_name;
            if (!str_id.empty())
                str_name = opentxs::OT::App().API().Exec().GetAccountWallet_Name(str_id);
            // ------------------------
            if (str_name.empty())
                return QSqlTableModel::data(index,role);
            return QVariant(QString::fromStdString(str_name));
        }
        else if (index.column() == 11) // currency_acct_id
        {
            QString qstrID = QSqlTableModel::data(index,role).toString();
            const std::string str_id = qstrID.toStdString();
            std::string str_name;
            if (!str_id.empty())
                str_name = opentxs::OT::App().API().Exec().GetAccountWallet_Name(str_id);
            // ------------------------
            if (str_name.empty())
                return QSqlTableModel::data(index,role);
            return QVariant(QString::fromStdString(str_name));
        }
        else if (index.column() == 12) // asset_id
        {
            QString qstrID = QSqlTableModel::data(index,role).toString();
            const std::string str_id = qstrID.toStdString();
            const std::string str_name = opentxs::OT::App().API().Exec().GetAssetType_Name(str_id);
            // ------------------------
            if (str_name.empty())
                return QSqlTableModel::data(index,role);
            return QVariant(QString::fromStdString(str_name));
        }
        else if (index.column() == 13) // currency_id
        {
            QString qstrID = QSqlTableModel::data(index,role).toString();
            const std::string str_id = qstrID.toStdString();
            const std::string str_name = opentxs::OT::App().API().Exec().GetAssetType_Name(str_id);
            // ------------------------
            if (str_name.empty())
                return QSqlTableModel::data(index,role);
            return QVariant(QString::fromStdString(str_name));
        }
        else if (index.column() == 14) // asset_receipt
        {
            QString qstrReceipt = QSqlTableModel::data(index,role).toString();
            if (qstrReceipt.isEmpty())
                return QVariant(QString(tr("(NO ASSET RECEIPT)")));
            return QVariant(QString(tr("(Asset receipt)")));
        }
        else if (index.column() == 15) // currency_receipt
        {
            QString qstrReceipt = QSqlTableModel::data(index,role).toString();
            if (qstrReceipt.isEmpty())
                return QVariant(QString(tr("(NO CURRENCY RECEIPT)")));
            return QVariant(QString(tr("(Currency receipt)")));
        }
        else if (index.column() == 16) // final_receipt
        {
            QString qstrReceipt = QSqlTableModel::data(index,role).toString();
            if (qstrReceipt.isEmpty())
                return QVariant(QString(tr("(NO FINAL RECEIPT)")));
            return QVariant(QString(tr("(Final receipt)")));
        }
    }

    return QSqlTableModel::data(index,role);
}

void ModelTradeArchive::updateDBFromOT(const std::string & strNotaryID, const std::string & strNymID)
{
    std::shared_ptr<opentxs::OTDB::TradeListNym> pTradeList = ModelTradeArchive::LoadTradeListForNym(strNotaryID, strNymID);

    if (pTradeList)
    {
        bool bEditing = false;

        const size_t nTradeDataCount = pTradeList->GetTradeDataNymCount();
        // -------------------------------------
        for (size_t ttii = 0; ttii < nTradeDataCount; ++ttii)
        {
            size_t trade_index = nTradeDataCount - ttii - 1; // This way we iterate in reverse. (In case we delete anything...)

            opentxs::OTDB::TradeDataNym * pTradeData = pTradeList->GetTradeDataNym(trade_index);

            if (NULL == pTradeData) // Should never happen.
                continue;

            // OT constructs this record from TWO incoming receipts. So here, I'm making
            // sure that OT has already received both of those, and finished constructing
            // this record, before I import it.
            //
            // ===> When the record is ready, BOTH asset IDs will be present, so:
            //        Skip whenever that's not the case.
            if (pTradeData->currency_id.empty() || pTradeData->instrument_definition_id.empty())
                continue;
            // -----------------------------------------------------------------------
            int64_t lScale     = opentxs::OT::App().API().Exec().StringToLong(pTradeData->scale);
            int64_t lReceiptID = opentxs::OT::App().API().Exec().StringToLong(pTradeData->updated_id);
            int64_t lOfferID   = opentxs::OT::App().API().Exec().StringToLong(pTradeData->transaction_id);
            // -----------------------------------------------------------------------
//          time_t tDate = static_cast<time_t>(opentxs::OT::App().API().Exec().StringToLong(pTradeData->date));
            time64_t tDate = static_cast<time64_t>(opentxs::OT::App().API().Exec().StringToLong(pTradeData->date));
            // -----------------------------------------------------------------------
            std::string & str_price = pTradeData->price;
            int64_t       lPrice    = opentxs::OT::App().API().Exec().StringToLong(str_price); // this price is "per scale"

            if (lPrice < 0)
                lPrice *= (-1);
            // -----------------------------------------------------------------------
            std::string & str_amount_sold    = pTradeData->amount_sold;
            int64_t       lQuantity          = opentxs::OT::App().API().Exec().StringToLong(str_amount_sold); // Amount of asset sold for that price.

            if (lQuantity < 0)
                lQuantity *= (-1);
            // -----------------------------------------------------------------------
            std::string & str_currency_paid   = pTradeData->currency_paid;
            int64_t       lPayQuantity        = opentxs::OT::App().API().Exec().StringToLong(str_currency_paid); // Amount of currency paid for this trade.

            if (lPayQuantity < 0)
                lPayQuantity *= (-1);
            // -----------------------------------------------------------------------
            // If the "actual price" is zero, we'll interpolate it.
            //
            if (0 == lPrice)
            {
                if ((lQuantity != 0) && (lPayQuantity != 0) && (lScale != 0))
                {
                    lPrice = lPayQuantity / (lQuantity / lScale);
                }
            }
            // -----------------------------------------------------------------------
            bool bIsBid = pTradeData->is_bid;
            // -----------------------------------------------------------------------
            if (!bEditing)
            {
                bEditing = true;
                this->database().transaction();
            }
            // -----------------------------------------------------------------------
            // lOfferId is a critical value -- that plus strNymID
            //
            QPointer<FinalReceiptProxyModel> pFinalReceiptProxy = new FinalReceiptProxyModel;
            pFinalReceiptProxy->setSourceModel(this);
            pFinalReceiptProxy->setFilter(lOfferID, strNymID, strNotaryID);

            int nRowCount = pFinalReceiptProxy->rowCount();

            if (nRowCount > 0) // Matching rows are present. Update them.
            {
                for (int nIndex = 0; nIndex < nRowCount; ++nIndex)
                {
                    QModelIndex proxyIndex  = pFinalReceiptProxy->index(nIndex, 0);
                    QModelIndex actualIndex = pFinalReceiptProxy->mapToSource(proxyIndex);
                    QSqlRecord  record      = this->record(actualIndex.row());

                    record.setValue("is_bid", bIsBid);
//                    record.setValue("receipt_id", QVariant::fromValue(lReceiptID));
//                    record.setValue("offer_id", QVariant::fromValue(lOfferID));
                    record.setValue("scale", QVariant::fromValue(lScale));
                    record.setValue("actual_price", QVariant::fromValue(lPrice));
                    record.setValue("actual_paid", QVariant::fromValue(lPayQuantity));
                    record.setValue("amount_purchased", QVariant::fromValue(lQuantity));
//                    record.setValue("timestamp",  QVariant::fromValue(tDate));
//                    record.setValue("notary_id", QString::fromStdString(strNotaryID));
//                    record.setValue("nym_id", QString::fromStdString(strNymID));
                    record.setValue("asset_id", QString::fromStdString(pTradeData->instrument_definition_id));
                    record.setValue("currency_id", QString::fromStdString(pTradeData->currency_id));
                    record.setValue("asset_acct_id", QString::fromStdString(pTradeData->asset_acct_id));
                    record.setValue("currency_acct_id", QString::fromStdString(pTradeData->currency_acct_id));

                    record.setValue("asset_receipt", QString::fromStdString(pTradeData->asset_receipt));
                    record.setValue("currency_receipt", QString::fromStdString(pTradeData->currency_receipt));
//                    record.setValue("final_receipt", QString::fromStdString(pTradeData->final_receipt));

                    this->setRecord(actualIndex.row(), record);
                }
            }
            else // No matching rows are present. (Insert a new one).
            {
                QSqlRecord record = this->record();

                record.setValue("is_bid", bIsBid);
                record.setValue("receipt_id", QVariant::fromValue(lReceiptID));
                record.setValue("offer_id", QVariant::fromValue(lOfferID));
                record.setValue("scale", QVariant::fromValue(lScale));
                record.setValue("actual_price", QVariant::fromValue(lPrice));
                record.setValue("actual_paid", QVariant::fromValue(lPayQuantity));
                record.setValue("amount_purchased", QVariant::fromValue(lQuantity));
                record.setValue("timestamp",  QVariant::fromValue(tDate));
                record.setValue("notary_id", QString::fromStdString(strNotaryID));
                record.setValue("nym_id", QString::fromStdString(strNymID));
                record.setValue("asset_id", QString::fromStdString(pTradeData->instrument_definition_id));
                record.setValue("currency_id", QString::fromStdString(pTradeData->currency_id));
                record.setValue("asset_acct_id", QString::fromStdString(pTradeData->asset_acct_id));
                record.setValue("currency_acct_id", QString::fromStdString(pTradeData->currency_acct_id));

                record.setValue("asset_receipt", QString::fromStdString(pTradeData->asset_receipt));
                record.setValue("currency_receipt", QString::fromStdString(pTradeData->currency_receipt));
                record.setValue("final_receipt", QString::fromStdString(pTradeData->final_receipt));

                this->insertRecord(0, record);
            }

            pTradeList->RemoveTradeDataNym(trade_index);

        } // for (trades)
        // -----------------------------------------------------------------------
        if (bEditing)
        {
            if (this->submitAll())
            {
                if (this->database().commit())
                    opentxs::OTDB::StoreObject(*pTradeList, "nyms", "trades",
                        strNotaryID, strNymID);
            }
            else
            {
                this->database().rollback();
                qDebug() << "Database Write Error" <<
                           "The database reported an error: " <<
                           this->lastError().text();
            }
        }
    } // if (NULL != pTradeList)
}


void ModelTradeArchive::updateDBFromOT()
{
    const int32_t nymCount    = opentxs::OT::App().API().Exec().GetNymCount();
    const int32_t serverCount = opentxs::OT::App().API().Exec().GetServerCount();

    for (int32_t serverIndex = 0; serverIndex < serverCount; ++serverIndex)
    {
        std::string NotaryID = opentxs::OT::App().API().Exec().GetServer_ID(serverIndex);

        for (int32_t nymIndex = 0; nymIndex < nymCount; ++nymIndex)
        {
            std::string nymId = opentxs::OT::App().API().Exec().GetNym_ID(nymIndex);

            if (opentxs::OT::App().API().Exec().IsNym_RegisteredAtServer(nymId, NotaryID))
            {
                updateDBFromOT(NotaryID, nymId);
            }
        }
    }
}

std::shared_ptr<opentxs::OTDB::TradeListNym> ModelTradeArchive::LoadTradeListForNym(const std::string & strNotaryID, const std::string & strNymID)
{
    std::shared_ptr<opentxs::OTDB::TradeListNym> pReturn;
    // ------------------------------------------
    opentxs::OTDB::TradeListNym * pTradeList = nullptr;
    opentxs::OTDB::Storable     * pStorable  = nullptr;
    // ------------------------------------------
    if (strNotaryID.empty() || strNymID.empty())
        return nullptr;
    // ------------------------------------------
    if (opentxs::OTDB::Exists("nyms", "trades", strNotaryID, strNymID))
    {
        pStorable = opentxs::OTDB::QueryObject(opentxs::OTDB::STORED_OBJ_TRADE_LIST_NYM, "nyms", "trades",
                                      strNotaryID, strNymID);
        if (nullptr == pStorable) {
            return nullptr;
        }
        // -------------------------------
        pTradeList = opentxs::OTDB::TradeListNym::ot_dynamic_cast(pStorable);

        if (nullptr == pTradeList) {
            delete pStorable;
        }

        pReturn.reset(pTradeList);
    }

    return pReturn;
}


