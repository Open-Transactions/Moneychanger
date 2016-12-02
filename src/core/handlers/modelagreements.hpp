#ifndef MODELAGREEMENTS_H
#define MODELAGREEMENTS_H

#include <core/handlers/contacthandler.hpp>

#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QModelIndex>
#include <QVariant>
#include <QSortFilterProxyModel>
#include <QFlags>

#include <memory>
#include <string>

#define AGRMT_SOURCE_COL_AGRMT_ID 0
#define AGRMT_SOURCE_COL_HAVE_READ 1
#define AGRMT_SOURCE_COL_TXN_ID_DISPLAY 2
#define AGRMT_SOURCE_COL_NOTARY_ID 3
#define AGRMT_SOURCE_COL_CONTRACT_ID 4
#define AGRMT_SOURCE_COL_NEWEST_RECEIPT_ID 5
#define AGRMT_SOURCE_COL_NEWEST_KNOWN_STATE 6
#define AGRMT_SOURCE_COL_TIMESTAMP 7
#define AGRMT_SOURCE_COL_MEMO 8
#define AGRMT_SOURCE_COL_FOLDER 9


#define AGRMT_NYM_COL_AGRMT_ID 0
#define AGRMT_NYM_COL_NYM_ID 1
#define AGRMT_NYM_COL_OPENING_TXN 2
#define AGRMT_NYM_COL_KNOWN_STATE 3
#define AGRMT_NYM_COL_LAST_RECEIPT_ID 4
#define AGRMT_NYM_COL_TIMESTAMP 5
#define AGRMT_NYM_COL_HAVE_READ 6
#define AGRMT_NYM_COL_LAST_PAID_ID 7
#define AGRMT_NYM_COL_LAST_COLLECTED_ID 8
#define AGRMT_NYM_COL_LAST_SENT_ID 9
#define AGRMT_NYM_COL_LAST_RECEIVED_ID 10
#define AGRMT_NYM_COL_LAST_NOTICE_ID 11
#define AGRMT_NYM_COL_ACTIVATION_ID 12
#define AGRMT_NYM_COL_LAST_FINAL_ID 13


#define AGRMT_RECEIPT_COL_AGRMT_RECEIPT_KEY 0
#define AGRMT_RECEIPT_COL_AGRMT_ID 1
#define AGRMT_RECEIPT_COL_RECEIPT_ID 2
#define AGRMT_RECEIPT_COL_TIMESTAMP 3
#define AGRMT_RECEIPT_COL_HAVE_READ 4
#define AGRMT_RECEIPT_COL_TXN_ID_DISPLAY 5
#define AGRMT_RECEIPT_COL_EVENT_ID 6
#define AGRMT_RECEIPT_COL_MEMO 7
#define AGRMT_RECEIPT_COL_MY_ASSET_TYPE 8
#define AGRMT_RECEIPT_COL_MY_NYM 9
#define AGRMT_RECEIPT_COL_MY_ACCT 10
#define AGRMT_RECEIPT_COL_MY_ADDR 11
#define AGRMT_RECEIPT_COL_SENDER_NYM 12
#define AGRMT_RECEIPT_COL_SENDER_ACCT 13
#define AGRMT_RECEIPT_COL_SENDER_ADDR 14
#define AGRMT_RECEIPT_COL_RECIP_NYM 15
#define AGRMT_RECEIPT_COL_RECIP_ACCT 16
#define AGRMT_RECEIPT_COL_RECIP_ADDR 17
#define AGRMT_RECEIPT_COL_AMOUNT 18
#define AGRMT_RECEIPT_COL_FOLDER 19
#define AGRMT_RECEIPT_COL_METHOD_TYPE 20
#define AGRMT_RECEIPT_COL_METHOD_TYPE_DISP 21
#define AGRMT_RECEIPT_COL_NOTARY_ID 22
#define AGRMT_RECEIPT_COL_DESCRIPTION 23
#define AGRMT_RECEIPT_COL_RECORD_NAME 24
#define AGRMT_RECEIPT_COL_INSTRUMENT_TYPE 25
#define AGRMT_RECEIPT_COL_FLAGS 26


class ModelAgreements : public QSqlTableModel
{
    Q_OBJECT
public:

    explicit ModelAgreements(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());

    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant rawData ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

signals:

public slots:
};


class QTableView;

class AgreementsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    AgreementsProxyModel(QObject *parent = 0);

    void setFilterFolder(int nFolder);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data    ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
    QVariant rawData ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

    void setTableView(QTableView * pTableView) { pTableView_ = pTableView; }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const Q_DECL_OVERRIDE;
    bool filterAcceptsColumn(int source_column, const QModelIndex & source_parent) const Q_DECL_OVERRIDE;

private:
    int nFolder_ = 0;  // 0 Payment Plan, 1 Smart Contract, 2 Entity

    QTableView * pTableView_=nullptr;
};

// ----------------------------------------------

class ModelAgreementReceipts : public QSqlTableModel
{
    Q_OBJECT
public:

    explicit ModelAgreementReceipts(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());

    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant rawData ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

signals:

public slots:
};


class AgreementReceiptsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    enum FilterType {
        FilterNone = 0,
        FilterSent = 1,
        FilterReceived = 2
    };

public:
    AgreementReceiptsProxyModel(QObject *parent = 0);

    void setFilterSent();
    void setFilterReceived();
    void setFilterNone();

    void clearFilterType();

    void setFilterString(QString qstrFilter);

    void setFilterAccountId(QString qstrFilter); // unused so far.
    void setFilterMySignerId(QString qstrFilter); // unused so far.
    void setFilterAgreementId(int nFilter); // In use!

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data    ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant rawData ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

    QWidget * CreateDetailHeaderWidget(const int nSourceRow, bool bExternal=true) const;

    void setTableView(QTableView * pTableView) { pTableView_ = pTableView; }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const Q_DECL_OVERRIDE;
    bool filterAcceptsColumn(int source_column, const QModelIndex & source_parent) const Q_DECL_OVERRIDE;

private:
    QString   filterString_;
    QString   filterAccountId_;
    QString   filterMySignerId_;
    int       filterAgreementId_{0};

    FilterType filterType_{FilterNone};

    QTableView * pTableView_{nullptr};
};


// ----------------------------------------------

/*
class ModelAgreementMySigners : public QSqlTableModel
{
    Q_OBJECT
public:

    explicit ModelAgreementMySigners(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());

    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant rawData ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

signals:

public slots:
};


class AgreementMySignersProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    AgreementMySignersProxyModel(QObject *parent = 0);

    void setFilterString(QString qstrFilter);

    void setFilterAgreementId(int nFilter);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data    ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant rawData ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

    void setTableView(QTableView * pTableView) { pTableView_ = pTableView; }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const Q_DECL_OVERRIDE;
    bool filterAcceptsColumn(int source_column, const QModelIndex & source_parent) const Q_DECL_OVERRIDE;

private:
    QString   filterString_;
    int       filterAgreementId_=0;

    QTableView * pTableView_=nullptr;
};

*/

#endif // MODELAGREEMENTS_H


