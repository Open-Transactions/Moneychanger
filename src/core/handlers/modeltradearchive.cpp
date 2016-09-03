#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/moneychanger.hpp>
#include <core/handlers/DBHandler.hpp>
#include <core/handlers/contacthandler.hpp>

#include <core/handlers/modeltradearchive.hpp>

#include <opentxs/core/OTStorage.hpp>
#include <opentxs/client/OTAPI_Wrap.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>
#include <opentxs/client/OT_ME.hpp>
#include <opentxs/core/util/OTPaths.hpp>

#include <opentxs/client/OTRecordList.hpp>

#include <QDebug>
#include <QtGlobal>
#include <QSqlTableModel>
#include <QDateTime>
#include <Qt>

FinalReceiptProxyModel::FinalReceiptProxyModel(QObject *parent /*= 0*/)
: QSortFilterProxyModel(parent)
{
}

void FinalReceiptProxyModel::setFilterOpentxsRecord(opentxs::OTRecord& recordmt)
{
    pRecordMT_ = &recordmt;
    invalidateFilter();
}

bool FinalReceiptProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex theIndex = sourceModel()->index(sourceRow, 6, sourceParent); // OfferID at index 6
    QAbstractItemModel * pModel = sourceModel();
    ModelTradeArchive  * pTradeModel = dynamic_cast<ModelTradeArchive*>(pModel);

    if (nullptr != pTradeModel)
    {
        int64_t lCurrentOfferID  = pTradeModel->rawData(theIndex).toLongLong();

        if (nullptr != pRecordMT_)
        {
            const int64_t lTransNumForDisplay = pRecordMT_->GetTransNumForDisplay();
            return (lTransNumForDisplay == lCurrentOfferID);
        }
    }
    return false;
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

            std::string str_display = opentxs::OTAPI_Wrap::It()->FormatAmount(str_currency_id, lPrice);
            QString qstrDisplay = QString::fromStdString(str_display);

            return QVariant(qstrDisplay);
        }
        else if (index.column() == 2) // scale
        {
            int64_t lPrice = QSqlTableModel::data(index, role).toLongLong();

            QModelIndex sibling = index.sibling(index.row(), 12);

            QString qstrCurrencyID = QSqlTableModel::data(sibling,role).toString();
            const std::string str_currency_id = qstrCurrencyID.toStdString();

            std::string str_display = opentxs::OTAPI_Wrap::It()->FormatAmount(str_currency_id, lPrice);
            QString qstrDisplay = QString::fromStdString(str_display);

            return QVariant(qstrDisplay);
        }
        else if (index.column() == 3) // actual_paid
        {
            int64_t lPrice = QSqlTableModel::data(index, role).toLongLong();

            QModelIndex sibling = index.sibling(index.row(), 13);

            QString qstrCurrencyID = QSqlTableModel::data(sibling,role).toString();

            const std::string str_currency_id = qstrCurrencyID.toStdString();

            std::string str_display = opentxs::OTAPI_Wrap::It()->FormatAmount(str_currency_id, lPrice);
            QString qstrDisplay = QString::fromStdString(str_display);

            return QVariant(qstrDisplay);
        }
        else if (index.column() == 4) // amount_purchased
        {
            int64_t lPrice = QSqlTableModel::data(index, role).toLongLong();

            QModelIndex sibling = index.sibling(index.row(), 12);

            QString qstrCurrencyID = QSqlTableModel::data(sibling,role).toString();
            const std::string str_currency_id = qstrCurrencyID.toStdString();

            std::string str_display = opentxs::OTAPI_Wrap::It()->FormatAmount(str_currency_id, lPrice);
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
            const std::string str_name = opentxs::OTAPI_Wrap::It()->GetServer_Name(str_id);
            // ------------------------
            if (str_name.empty())
                return QSqlTableModel::data(index,role);
            return QVariant(QString::fromStdString(str_name));
        }
        else if (index.column() == 9) // nym id
        {
            QString qstrID = QSqlTableModel::data(index,role).toString();
            const std::string str_id = qstrID.toStdString();
            const std::string str_name = opentxs::OTAPI_Wrap::It()->GetNym_Name(str_id);
            // ------------------------
            if (str_name.empty())
                return QSqlTableModel::data(index,role);
            return QVariant(QString::fromStdString(str_name));
        }
        else if (index.column() == 10) // asset_acct_id
        {
            QString qstrID = QSqlTableModel::data(index,role).toString();
            const std::string str_id = qstrID.toStdString();
            const std::string str_name = opentxs::OTAPI_Wrap::It()->GetAccountWallet_Name(str_id);
            // ------------------------
            if (str_name.empty())
                return QSqlTableModel::data(index,role);
            return QVariant(QString::fromStdString(str_name));
        }
        else if (index.column() == 11) // currency_acct_id
        {
            QString qstrID = QSqlTableModel::data(index,role).toString();
            const std::string str_id = qstrID.toStdString();
            const std::string str_name = opentxs::OTAPI_Wrap::It()->GetAccountWallet_Name(str_id);
            // ------------------------
            if (str_name.empty())
                return QSqlTableModel::data(index,role);
            return QVariant(QString::fromStdString(str_name));
        }
        else if (index.column() == 12) // asset_id
        {
            QString qstrID = QSqlTableModel::data(index,role).toString();
            const std::string str_id = qstrID.toStdString();
            const std::string str_name = opentxs::OTAPI_Wrap::It()->GetAssetType_Name(str_id);
            // ------------------------
            if (str_name.empty())
                return QSqlTableModel::data(index,role);
            return QVariant(QString::fromStdString(str_name));
        }
        else if (index.column() == 13) // currency_id
        {
            QString qstrID = QSqlTableModel::data(index,role).toString();
            const std::string str_id = qstrID.toStdString();
            const std::string str_name = opentxs::OTAPI_Wrap::It()->GetAssetType_Name(str_id);
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
            // this record, before I import it. (When the record is ready, BOTH asset IDs
            // will be present, so skip whenever that's not the case.)
            if (pTradeData->currency_id.empty() || pTradeData->instrument_definition_id.empty())
                continue;
            // -----------------------------------------------------------------------
            int64_t lScale     = opentxs::OTAPI_Wrap::It()->StringToLong(pTradeData->scale);
            int64_t lReceiptID = opentxs::OTAPI_Wrap::It()->StringToLong(pTradeData->updated_id);
            int64_t lOfferID   = opentxs::OTAPI_Wrap::It()->StringToLong(pTradeData->transaction_id);
            // -----------------------------------------------------------------------
//          time_t tDate = static_cast<time_t>(opentxs::OTAPI_Wrap::It()->StringToLong(pTradeData->date));
            time64_t tDate = static_cast<time64_t>(opentxs::OTAPI_Wrap::It()->StringToLong(pTradeData->date));
            // -----------------------------------------------------------------------
            std::string & str_price = pTradeData->price;
            int64_t       lPrice    = opentxs::OTAPI_Wrap::It()->StringToLong(str_price); // this price is "per scale"

            if (lPrice < 0)
                lPrice *= (-1);
            // -----------------------------------------------------------------------
            std::string & str_amount_sold    = pTradeData->amount_sold;
            int64_t       lQuantity          = opentxs::OTAPI_Wrap::It()->StringToLong(str_amount_sold); // Amount of asset sold for that price.

            if (lQuantity < 0)
                lQuantity *= (-1);
            // -----------------------------------------------------------------------
            std::string & str_currency_paid   = pTradeData->currency_paid;
            int64_t       lPayQuantity        = opentxs::OTAPI_Wrap::It()->StringToLong(str_currency_paid); // Amount of currency paid for this trade.

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
    const int32_t nymCount    = opentxs::OTAPI_Wrap::It()->GetNymCount();
    const int32_t serverCount = opentxs::OTAPI_Wrap::It()->GetServerCount();

    for (int32_t serverIndex = 0; serverIndex < serverCount; ++serverIndex)
    {
        std::string NotaryID = opentxs::OTAPI_Wrap::It()->GetServer_ID(serverIndex);

        for (int32_t nymIndex = 0; nymIndex < nymCount; ++nymIndex)
        {
            std::string nymId = opentxs::OTAPI_Wrap::It()->GetNym_ID(nymIndex);

            if (opentxs::OTAPI_Wrap::It()->IsNym_RegisteredAtServer(nymId, NotaryID))
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


