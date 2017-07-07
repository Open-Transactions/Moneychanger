#ifndef MODELTRADEARCHIVE_H
#define MODELTRADEARCHIVE_H

#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QModelIndex>
#include <QVariant>
#include <QSortFilterProxyModel>


#include <memory>
#include <string>

namespace opentxs{
class OTRecord;
namespace OTDB {
class TradeListNym;
}
}

class ModelTradeArchive : public QSqlTableModel
{
    Q_OBJECT
public:
    explicit ModelTradeArchive(QObject * parent = 0, QSqlDatabase db = QSqlDatabase());

    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant rawData ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

    static std::shared_ptr<opentxs::OTDB::TradeListNym> LoadTradeListForNym(const std::string & strNotaryID, const std::string & strNymID);

    void updateDBFromOT();
    void updateDBFromOT(const std::string & strNotaryID, const std::string & strNymID);

signals:

public slots:
};

// This filter is used by the OfferDetails widget, which filters
// the trade archive to ONLY show trade receipts for a given offer,
// for a given Nym.
class TradeArchiveProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    TradeArchiveProxyModel(QObject *parent = 0);

    QString filterNymId() const { return nymId_; }
    void setFilterNymId(const QString &nymId);

    int64_t filterOfferId() const { return offerId_; }
    void setFilterOfferId(const int64_t offerId);

    bool filterIsBid() const { return isBid_; }
    void setFilterIsBid(const bool isBid);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const Q_DECL_OVERRIDE;
    bool filterAcceptsColumn(int source_column, const QModelIndex & source_parent) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

private:
    QString nymId_;
    int64_t offerId_=0;
    bool    isBid_=false;
};

// This filter is used by AddFinalReceiptToTradeArchive, which only
// cares about rows that correspond to that final receipt. (The code
// that uses this proxy then adds a copy of that final receipt to
// those specific rows.)
//
class FinalReceiptProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    FinalReceiptProxyModel(QObject *parent = 0);

    void setFilterOpentxsRecord(opentxs::OTRecord& recordmt);
    void setFilter(int64_t lTransNumForDisplay, std::string str_nym_id, std::string str_notary_id);
    void clearFilter();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const Q_DECL_OVERRIDE;

private:
    int64_t     lTransNum_{0};
    int64_t     lTransNumForDisplay_{0};
    std::string str_nym_id_;
    std::string str_notary_id_;
    bool        bIsFinalReceipt_{false};
    int64_t     lClosingNum_{0};
    bool        bGotClosingNum_{false};
};


#endif // MODELTRADEARCHIVE_H
