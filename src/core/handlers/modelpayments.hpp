#ifndef MODELPAYMENTS_H
#define MODELPAYMENTS_H

#include <core/handlers/contacthandler.hpp>

#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QModelIndex>
#include <QVariant>
#include <QSortFilterProxyModel>
#include <QFlags>

#include <memory>
#include <string>

#define PMNT_SOURCE_COL_ROW_WIDGET_ID 0
#define PMNT_SOURCE_COL_CURRENCY_TYPE 1
#define PMNT_SOURCE_COL_MY_NYM_ID 2
#define PMNT_SOURCE_COL_ACCOUNT_ID 3
#define PMNT_SOURCE_COL_UNIT_TYPE_ID 4
#define PMNT_SOURCE_COL_NOTARY_ID 5
#define PMNT_SOURCE_COL_AMOUNT 6
#define PMNT_SOURCE_COL_DISPLAY_AMOUNT 7
#define PMNT_SOURCE_COL_MEMO 8
#define PMNT_SOURCE_COL_TEXT 9
#define PMNT_SOURCE_COL_TIMESTAMP 10
#define PMNT_SOURCE_COL_STORAGE_BOX_TYPE 11

//pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("row_widget_id"));
//pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("currency_type"));
//pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("my_nym_id"));
//pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("account_id"));
//pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("unit_type_id"));
//pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("notary_id"));
//pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("amount"));
//pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("display_amount"));
//pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("memo"));
//pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("text"));
//pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("timestamp"));
//pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("storage_box_type"));


class ModelPayments : public QSqlTableModel
{
    Q_OBJECT
public:
    /*
class MyClass
{
public:
    enum Option {
        NoOptions = 0x0,
        ShowTabs = 0x1,
        ShowAll = 0x2,
        SqueezeBlank = 0x4
    };
    Q_DECLARE_FLAGS(Options, Option)
    ...
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MyClass::Options)

You can then use the MyClass::Options type to store combinations of MyClass::Option values.
    */
//    enum PaymentFlag {
//         NoFlags                 =  0x0
//        ,IsSpecialMail           =  0x1
//        ,IsPending               =  0x1 <<  1
//        ,IsOutgoing              =  0x1 <<  2
//        ,IsRecord                =  0x1 <<  3
//        ,IsReceipt               =  0x1 <<  4
//        ,IsFinalReceipt          =  0x1 <<  5
//        ,IsMail                  =  0x1 <<  6
//        ,IsTransfer              =  0x1 <<  7
//        ,IsCheque                =  0x1 <<  8
//        ,IsInvoice               =  0x1 <<  9
//        ,IsVoucher               =  0x1 << 10
//        ,IsContract              =  0x1 << 11
//        ,IsPaymentPlan           =  0x1 << 12
//        ,IsCash                  =  0x1 << 13
//        ,IsNotice                =  0x1 << 14
//        ,IsExpired               =  0x1 << 15
//        ,IsCanceled              =  0x1 << 16
//        ,CanDelete               =  0x1 << 17
//        ,CanAcceptIncoming       =  0x1 << 18
//        ,CanDiscardIncoming      =  0x1 << 19
//        ,CanCancelOutgoing       =  0x1 << 20
//        ,CanDiscardOutgoingCash  =  0x1 << 21
//        ,OriginTypePaymentPlan   =  0x1 << 22 // For notices, finalReceipts and paymentReceipts re: a payment plan
//        ,OriginTypeSmartContract =  0x1 << 23 // For notices, finalReceipts and paymentReceipts re: a smart contract
//    };
//    Q_DECLARE_FLAGS(PaymentFlags, PaymentFlag)

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

//Q_DECLARE_OPERATORS_FOR_FLAGS(ModelPayments::PaymentFlags)


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

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data    ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
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



class ActivityPaymentsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    enum FilterType {
        FilterNone = 0,
        FilterTopLevel = 1,
        FilterNotary = 2,
        FilterCurrency = 3,
        FilterAccount = 4
    };

public:
    ActivityPaymentsProxyModel(QObject *parent = 0);

    void setFilterFolder(int nFolder);

    void setFilterNone(); // Doesn't affect the filterFolder, but DOES affect all the others below. (Top level, notary, method address.)
    void setFilterTopLevel(QString qstrContactID);
    void setFilterNotary(QString qstrNotaryId);
    void setFilterCurrency(int nCurrencyType);
    void setFilterAccount(QString qstrAccountId);

    void setFilterString(QString qstrFilter);

    void clearFilterType();

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data    ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
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
    QString   accountId_;

    QString   filterString_;

    mapIDName mapNymIds_;
    mapIDName mapAssetTypes_;

    FilterType filterType_ = FilterNone;

    int nFolder_ = 1; // 0 for outbox, 1 for inbox, and 2+ for all the other custom boxes we'll have someday.

    QTableView * pTableView_=nullptr;
};




class AccountRecordsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    enum FilterType {
        FilterNone = 0,
        FilterSent = 1,
        FilterReceived = 2
    };

public:
    AccountRecordsProxyModel(QObject *parent = 0);

    void setFilterSent();
    void setFilterReceived();
    void setFilterNone();

    void clearFilterType();

    void setFilterString(QString qstrFilter);

    void setFilterAccountId(QString qstrFilter);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data    ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
    QVariant rawData ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

    QWidget * CreateDetailHeaderWidget(const int nSourceRow, bool bExternal=true) const;

    void setTableView(QTableView * pTableView) { pTableView_ = pTableView; }

protected:
    void setFilterFolder(int nFolder);

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const Q_DECL_OVERRIDE;
    bool filterAcceptsColumn(int source_column, const QModelIndex & source_parent) const Q_DECL_OVERRIDE;

private:
    QString   filterString_;
    QString   filterAccount_;

    FilterType filterType_ = FilterNone;

    QTableView * pTableView_=nullptr;
};


#endif // MODELPAYMENTS_H


