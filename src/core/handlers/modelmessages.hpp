#ifndef MODELMESSAGES_H
#define MODELMESSAGES_H

#include <core/handlers/contacthandler.hpp>

#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QModelIndex>
#include <QVariant>
#include <QSortFilterProxyModel>
#include <QTableView>

#include <memory>
#include <string>

#define MSG_SOURCE_COL_MSG_ID 0
#define MSG_SOURCE_COL_HAVE_READ 1
#define MSG_SOURCE_COL_HAVE_REPLIED 2
#define MSG_SOURCE_COL_HAVE_FORWARDED 3
#define MSG_SOURCE_COL_SUBJECT 4
#define MSG_SOURCE_COL_SENDER_NYM 5
#define MSG_SOURCE_COL_SENDER_ADDR 6
#define MSG_SOURCE_COL_RECIP_NYM 7
#define MSG_SOURCE_COL_RECIP_ADDR 8
#define MSG_SOURCE_COL_TIMESTAMP 9
#define MSG_SOURCE_COL_METHOD_TYPE 10
#define MSG_SOURCE_COL_METHOD_TYPE_DISP 11
#define MSG_SOURCE_COL_NOTARY_ID 12
#define MSG_SOURCE_COL_MY_NYM 13
#define MSG_SOURCE_COL_MY_ADDR 14
#define MSG_SOURCE_COL_FOLDER 15
#define MSG_SOURCE_COL_THREAD_ITEM_ID 16
#define MSG_SOURCE_COL_ARCHIVED 17
#define MSG_SOURCE_COL_HAS_SUBJECT 18

// The above defines are for the old message source model. (A QSqlTableModel) Used on Mail screen.
// The below defines are for the new "conversational" model (A QSqlQueryModel) Used on Activity screen.

#define CONV_SOURCE_COL_MSG_ID 0
#define CONV_SOURCE_COL_MY_NYM 1
#define CONV_SOURCE_COL_THREAD_ID 2
#define CONV_SOURCE_COL_ITEM_ID 3
#define CONV_SOURCE_COL_TIMESTAMP 4
#define CONV_SOURCE_COL_FOLDER 5
#define CONV_SOURCE_COL_BODY 6


class QSqlQueryMessages : public QSqlQueryModel
{
    Q_OBJECT
public:
    explicit QSqlQueryMessages(QObject *parent = 0);

    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant rawData ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

signals:

public slots:
};



class ModelMessages : public QSqlTableModel
{
    Q_OBJECT
public:
    explicit ModelMessages(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());

    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant rawData ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

//    void updateDBFromOT();
//    void updateDBFromOT(const std::string & strNotaryID, const std::string & strNymID);

signals:

public slots:
};


class MessagesProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    enum FilterType {
        FilterNone = 0,
        FilterTopLevel = 1,
        FilterNotary = 2,
        FilterMethodAddress = 3
    };

public:
    MessagesProxyModel(QObject *parent = 0);

    void setFilterFolder(int nFolder);

    void setFilterNone(); // Doesn't affect the filterFolder, but DOES affect all the others below. (Top level, notary, method address.)
    void setFilterTopLevel(int nContactID);
    void setFilterTopLevel(QString qstrContactID);
    void setFilterNotary(QString qstrNotaryId, int nContactID);
    void setFilterNotary(QString qstrNotaryId, QString qstrContactID);
    void setFilterMethodAddress(QString qstrMethodType, QString qstrAddress);

    void setFilterString(QString qstrFilter);

    void clearFilterType();

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data    ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
    QVariant rawData ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

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

private: // These are filters:
    QString   notaryId_;
    QString   singleMethodType_;
    QString   singleAddress_;

    QString   filterString_;

    mapIDName mapNymIds_;
    mapIDName mapAddresses_;

    FilterType filterType_ = FilterNone;

    int nFolder_ = 1; // 0 for outbox, 1 for inbox, and 2+ for all the other custom boxes we'll have someday.
};



class ConvMsgsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    enum FilterType {
        FilterNone = 0,
        FilterTopLevel = 1,
        FilterNotary = 2
    };

public:
    ConvMsgsProxyModel(QObject *parent = 0);

    void setFilterFolder(int nFolder); // Outgoing, incoming, or (default for conversation view): ALL.

    void setFilterNone(); // Doesn't affect the filterFolder, but DOES affect all the others below. (Top level.)
//  void setFilterTopLevel(const std::string & strContactID);
    void setFilterTopLevel_Contact(const std::string & strMyNymID, const std::string & strContactID);
    void setFilterTopLevel_Convo  (const std::string & strMyNymID, const std::string & strConversationID); // Note: until we have group convos, ConvoId IS the Contact ID.
//  void setFilterNotary(QString qstrNotaryId, const std::string & strContactID);
    void setFilterNotary_Contact(QString qstrNotaryId, const std::string & strMyNymID, const std::string & strContactID);
    void setFilterNotary_Convo  (QString qstrNotaryId, const std::string & strMyNymID, const std::string & strConversationID); // Note: until we have group convos, ConvoId IS the Contact ID.

    void setFilterString(QString qstrFilter);

    void clearFilterType();

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data    ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
    QVariant rawData ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

    void setTableView(QTableView * pTableView) { pTableView_ = pTableView; }

    QWidget * CreateDetailHeaderWidget(const int nSourceRow, bool bExternal=false) const;

protected:
    // That means there is only 1 main filter type:
    // 1. top-level -- contact ID is only thing passed. Look up ALL Nyms for that contact.
    //
    // THEREFORE at filter set, there will always be:
    // A Contact ID by itself, OR a Notary ID + contact ID, OR a method type and address, OR
    // But after filter set is done, it will be a Notary ID + a list of Nyms, OR a method type and address, OR a list of Nyms + a list of addresses.
    // To keep things simple, I will set another variable so that the type is clear.

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const Q_DECL_OVERRIDE;
    bool filterAcceptsColumn(int source_column, const QModelIndex & source_parent) const Q_DECL_OVERRIDE;

private:
    QString   conversationId_;
    QString   notaryId_;
    QString   myNymId_; // We won't really need this at first either, since we're already filtering the source model with a SELECT based (in part) on my_nym_id.
    QString   singleMethodType_;
    QString   singleAddress_;

    QString   filterString_;

    QString   hisContactId_; // We won't use this initially but I'm storing it in case useful later.
    mapIDName mapNymIds_; // NymIds for the Contact I'm conversing with.
    mapIDName mapAddresses_; // Known addresses for the Contact I'm conversing with.

    FilterType filterType_{FilterNone};

    int nFolder_{2}; // 0 for outbox, 1 for inbox, and 2 for both.

    QTableView * pTableView_{nullptr};
};


#endif // MODELMESSAGES_H


