#ifndef MODELPAYMENTS_H
#define MODELPAYMENTS_H

#include <core/handlers/contacthandler.hpp>

#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QModelIndex>
#include <QVariant>
#include <QSortFilterProxyModel>

#include <memory>
#include <string>

#define PMNT_SOURCE_COL_PMNT_ID 0
#define PMNT_SOURCE_COL_HAVE_READ 1
#define PMNT_SOURCE_COL_HAVE_REPLIED 2
#define PMNT_SOURCE_COL_HAVE_FORWARDED 3
#define PMNT_SOURCE_COL_MEMO 4
#define PMNT_SOURCE_COL_MY_ASSET_TYPE 5
#define PMNT_SOURCE_COL_MY_NYM 6
#define PMNT_SOURCE_COL_MY_ACCT 7
#define PMNT_SOURCE_COL_MY_ADDR 8
#define PMNT_SOURCE_COL_DESCRIPTION 9
#define PMNT_SOURCE_COL_SENDER_NYM 10
#define PMNT_SOURCE_COL_SENDER_ACCT 11
#define PMNT_SOURCE_COL_SENDER_ADDR 12
#define PMNT_SOURCE_COL_RECIP_NYM 13
#define PMNT_SOURCE_COL_RECIP_ACCT 14
#define PMNT_SOURCE_COL_RECIP_ADDR 15
#define PMNT_SOURCE_COL_AMOUNT 16
#define PMNT_SOURCE_COL_PENDING_FOUND 17
#define PMNT_SOURCE_COL_COMPLETED_FOUND 18
#define PMNT_SOURCE_COL_TIMESTAMP 19
#define PMNT_SOURCE_COL_TXN_ID 20
#define PMNT_SOURCE_COL_TXN_ID_DISPLAY 21
#define PMNT_SOURCE_COL_METHOD_TYPE 22
#define PMNT_SOURCE_COL_METHOD_TYPE_DISP 23
#define PMNT_SOURCE_COL_NOTARY_ID 24
#define PMNT_SOURCE_COL_RECORD_NAME 25
#define PMNT_SOURCE_COL_INSTRUMENT_TYPE 26
#define PMNT_SOURCE_COL_FOLDER 27

class ModelPayments : public QSqlTableModel
{
    Q_OBJECT
public:
    explicit ModelPayments(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());

    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant rawData ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

//    void updateDBFromOT();
//    void updateDBFromOT(const std::string & strNotaryID, const std::string & strNymID);

signals:

public slots:
};


class QTableView;

class PaymentsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    enum FilterType {
        FilterNone = 0,
        FilterTopLevel = 1,
        FilterNotary = 2,
        FilterMethodAddress = 3
    };

public:
    PaymentsProxyModel(QObject *parent = 0);

    void setFilterFolder(int nFolder);

    void setFilterNone(); // Doesn't affect the filterFolder, but DOES affect all the others below. (Top level, notary, method address.)
    void setFilterTopLevel(int nContactID);
    void setFilterNotary(QString qstrNotaryId, int nContactID);
    void setFilterMethodAddress(QString qstrMethodType, QString qstrAddress);

    void setFilterString(QString qstrFilter);

    void clearFilterType();

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data    ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant rawData ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

    QWidget * CreateDetailHeaderWidget(const int nSourceRow, bool bExternal=true) const;

    void setTableView(QTableView * pTableView) { pTableView_ = pTableView; }

protected:
    // That means there are 3 main filter types:
    // 1. top-level   -- contact ID is only thing passed. Look up ALL Nyms and Addresses for that contact. Any of them is good enough for a match.
    // 2. notary    -- must pass NotaryID as well as ContactID (that's so we can look up the Nym list for that contact.) Requirement is "any Nym from that list, plus the notary."
    // 3. method_type -- must pass the method type and the address. That's the requirement, too (both.)
    //
    // THEREFORE at filter set, there will always be:
    // A Contact ID by itself, OR a Notary ID + contact ID, OR a method type and address, OR
    // But after filter set is done, it will be a Notary ID + a list of Nyms, OR a method type and address, OR a list of Nyms + a list of addresses.
    // To keep things simple, I will set another variable so that the type is clear.

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const Q_DECL_OVERRIDE;
    bool filterAcceptsColumn(int source_column, const QModelIndex & source_parent) const Q_DECL_OVERRIDE;

private:
    QString   notaryId_;
    QString   singleMethodType_;
    QString   singleAddress_;

    QString   filterString_;

    mapIDName mapNymIds_;
    mapIDName mapAddresses_;

    FilterType filterType_ = FilterNone;

    int nFolder_ = 1; // 0 for outbox, 1 for inbox, and 2+ for all the other custom boxes we'll have someday.

    QTableView * pTableView_=nullptr;
};



#endif // MODELPAYMENTS_H


