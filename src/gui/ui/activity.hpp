#ifndef ACTIVITY_HPP
#define ACTIVITY_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include "core/mapidname.hpp"

#include "core/handlers/DBHandler.hpp"
#include "core/handlers/modelmessages.hpp"

#include <QWidget>
#include <QMenu>
#include <QScopedPointer>
#include <QPointer>
#include <QList>
#include <QSqlRecord>
#include <QTimer>
#include <QDebug>

#include <tuple>
#include <map>


namespace Ui {
class Activity;
}



#define ACTIVITY_TREE_HEADER_TOTALS   0
#define ACTIVITY_TREE_HEADER_LOCAL    1
#define ACTIVITY_TREE_HEADER_SNP      2
#define ACTIVITY_TREE_HEADER_HOSTED   3
#define ACTIVITY_TREE_HEADER_CONTACTS 4

#define AC_NODE_TYPE_HEADER          0
#define AC_NODE_TYPE_ASSET_TOTAL     1
#define AC_NODE_TYPE_LOCAL_WALLET    2
#define AC_NODE_TYPE_LOCAL_ACCOUNT   3
#define AC_NODE_TYPE_SNP_NOTARY      4
#define AC_NODE_TYPE_SNP_ACCOUNT     5
#define AC_NODE_TYPE_HOSTED_NOTARY   6
#define AC_NODE_TYPE_HOSTED_ACCOUNT  7
#define AC_NODE_TYPE_CONTACT         8

//class ActivityPaymentsProxyModel;
//class ModelPayments;

class QListWidget;
class QTreeWidget;
class QTreeWidgetItem;

// These are used for re-selecting the row when you return
// to a certain filter. Like if you clicked on Alice, then back
// to Bob, the same row would be selected for you that was
// selected before.
//QString qstrCurrentTLA_;  // If the user clicks on "BTC" near the top, the current TLA is "BTC".
//QString qstrCurrentNotary_; // If the user clicks on "localhost" (hosted notary) then that Notary ID is set here.
//QString qstrCurrentAccount_; // If the user clicks on one of his Opentxs accounts, or a notary with only 1 account under it, we put the acct ID here.
//QString qstrCurrentContact_; // If the user clicks on one of his contacts, we put the Opentxs Contact ID here.
// TLA, notary, account, contact
typedef std::tuple<std::string, std::string, std::string, std::string> ACTIVITY_TREE_ITEM;
typedef std::map< ACTIVITY_TREE_ITEM, int> mapOfActivityTreeItems;

class Activity : public QWidget
{
    Q_OBJECT

private:
    bool already_init{false};

public:
    explicit Activity(QWidget *parent = 0);
    ~Activity();

    void dialog(int nSourceRow=-1, int nFolder=-1);

    int nSelectedTab_{0};

    QString qstrCurrentTLA_;  // If the user clicks on "BTC" near the top, the current TLA is "BTC".
    QString qstrCurrentNotary_; // If the user clicks on "localhost" (hosted notary) then that Notary ID is set here.
    QString qstrCurrentAccount_; // If the user clicks on one of his Opentxs accounts, or a notary with only 1 account under it, we put the acct ID here.
    QString qstrCurrentContact_; // If the user clicks on one of his contacts, we put the Opentxs Contact ID here.
//  QString qstrCurrentWallet_; // If the user clicks on his local BTC wallet, then we set something in here I guess to distinguish it from the local LTC wallet.

    void AcceptIncoming     (QPointer<ModelPayments> & pModel, ActivityPaymentsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView);
    void CancelOutgoing     (QPointer<ModelPayments> & pModel, ActivityPaymentsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView);
    void DiscardOutgoingCash(QPointer<ModelPayments> & pModel, ActivityPaymentsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView);
    void DiscardIncoming    (QPointer<ModelPayments> & pModel, ActivityPaymentsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView);

public slots:
    void onClaimsUpdatedForNym(QString nymId);
    void onRecordlistPopulated();
    void onBalancesChanged();
    void onNeedToRefreshUserBar();
    void onNeedToRefreshRecords();
    void resetPopupMenus();

signals:
    void showDashboard();
    void needToPopulateRecordlist();
    void needToDownloadAccountData();
    void showContact(QString);
    void showContacts();
    void showMessages();
    void showActiveAgreements();
    void showExchange();
    void showSettings();
    void messageContact(QString,QString);
    void showServer(QString);
    void showAsset(QString);
    void showAccount(QString);
    void payFromAccountToContact(QString, QString);
    void requestToAccountFromContact(QString, QString);
    void proposeToAccountFromContact(QString, QString);
    void showContactAndRefreshHome(QString);
    void needToCheckNym(QString, QString, QString);
    void needToPairStashNode();

    void sig_on_toolButton_payments_clicked();
    void sig_on_toolButton_pending_clicked();
    void sig_on_toolButton_markets_clicked();
    void sig_on_toolButton_secrets_clicked();
    void sig_on_toolButton_importCash_clicked();
    void sig_on_toolButton_contacts_clicked();
    void sig_on_toolButton_smartContracts_clicked();
    void sig_on_toolButton_Corporations_clicked();
    void sig_on_toolButton_transport_clicked();
    void sig_on_toolButton_quit_clicked();
    void sig_on_toolButton_encrypt_clicked();
    void sig_on_toolButton_sign_clicked();
    void sig_on_toolButton_decrypt_clicked();
    void sig_on_toolButton_liveAgreements_clicked();

protected:
    void enableButtons();
    void disableButtons();

    void doSearchConversations(QString qstrInput);
    void doSearchPayments(QString qstrInput);

    ACTIVITY_TREE_ITEM make_tree_item(QString & qstrCurrentTLA, QString & qstrCurrentNotary, QString & qstrCurrentAccount, QString & qstrCurrentContact);
    void set_inbox_pmntid_for_tree_item(ACTIVITY_TREE_ITEM & theItem, int nPmntID);
    void set_outbox_pmntid_for_tree_item(ACTIVITY_TREE_ITEM & theItem, int nPmntID);
    int get_inbox_pmntid_for_tree_item(ACTIVITY_TREE_ITEM & theItem); // returns payment ID
    int get_outbox_pmntid_for_tree_item(ACTIVITY_TREE_ITEM & theItem); // returns payment ID

    bool eventFilter(QObject *obj, QEvent *event);

    void RefreshAll();
    void RefreshUserBar();
    void RefreshAccountTab();
    void ClearAccountTree();
    void RefreshAccountTree();
    void RefreshConversationsTab();
    void RefreshConversationDetails(int nRow);
    void ClearListWidgetConversations();

    void PopulateConversationsForNym(QListWidget * pListWidgetConversations, int & nIndex, const std::string & str_my_nym_id);

    int PairedNodeCount(std::set<opentxs::Identifier> * pUniqueServers=nullptr);
    bool PairingStarted(const opentxs::Identifier & nymId, const opentxs::Identifier & issuerNymId);

    mapIDName & GetOrCreateAssetIdMapByCurrencyCode(QString qstrTLA, mapOfMapIDName & bigMap);
    void GetAssetIdMapsByCurrencyCode(mapOfMapIDName & bigMap);
    int64_t GetAccountBalancesTotaledForUnitTypes(const mapIDName & mapUnitTypeIds);

    mapIDName & GetOrCreateAccountIdMapByServerId(QString qstrServerId, mapOfMapIDName & bigMap);
    void GetAccountIdMapsByServerId(mapOfMapIDName & bigMap, const bool bTrusted, // true == paired, false == hosted.
                                    mapIDName * pMapTLAbyAccountId=nullptr);

    void GetAccountsByTLAFromMap(const mapIDName & mapAccounts, // input
                                 const mapIDName & mapTLAByAccountId, // input
                                 mapOfMapIDName  & bigMap); // output

    mapIDName & GetOrCreateAccountIdMapByTLA(QString qstrTLA, mapOfMapIDName & bigMap);

    bool get_deposit_address(
        const std::string str_notary_id,
        const std::string str_my_nym_id,
        const std::string str_issuer_nym_id,
        const std::string str_unit_type_id,
        opentxs::Identifier & returnBailmentId,
        std::string & returnBailmentInstructions);

    bool request_outbailment(
        const std::string str_notary_id,
        const std::string str_my_nym_id,
        const std::string str_issuer_nym_id,
        const std::string str_unit_type_id,
        const std::int64_t amount,
        const std::string str_blockchain_address);

    bool GetUnitAndTLAMapForAccountsOnServer(mapIDName& mapUnitTLA,
                                             mapOfMapIDName& bigMapAccountsByServer,
                                             mapOfMapIDName& bigMapAssetsByTLA,
                                             QString qstrServerId,
                                             const std::string & str_my_nym_id);


    void resetConversationItemsDataModel(const bool bProvidedIds=false, const QString * pstrMyNymID=nullptr, const QString * pstrThreadID=nullptr);

    void setAsCurrentPayment(int nSourceRow, int nFolder);

    void tableViewPayments_PopupMenu(const QPoint &pos, QTableView * pTableView, ActivityPaymentsProxyModel * pProxyModel);
    void tableViewPayments_DoubleClicked(const QModelIndex &index, ActivityPaymentsProxyModel * pProxyModel);

    void treeWidgetAccounts_PopupMenu(const QPoint &pos, QTreeWidget * pTreeWidget);

private slots:
    void on_listWidgetConversations_currentRowChanged(int currentRow);
    void on_listWidgetConversations_customContextMenuRequested(const QPoint &pos);

    void on_treeWidgetAccounts_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_pushButtonSendMsg_clicked();

    void on_checkBoxSearchConversations_toggled(bool checked);
    void on_pushButtonSearchConversations_clicked();
    void on_lineEditSearchConversations_textChanged(const QString &arg1);
    void on_lineEditSearchConversations_returnPressed();

    void on_checkBoxSearchPayments_toggled(bool checked);
    void on_pushButtonSearchPayments_clicked();
    void on_lineEditSearchPayments_textChanged(const QString &arg1);
    void on_lineEditSearchPayments_returnPressed();

    void on_tableViewSentSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous);
    void on_tableViewReceivedSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous);

    void on_tabWidgetTransactions_currentChanged(int index);

    void on_MarkAsRead_timer();
    void on_MarkAsUnread_timer();
    void on_MarkAsReplied_timer();
    void on_MarkAsForwarded_timer();

    void RefreshPayments();

    void on_tableViewReceived_customContextMenuRequested(const QPoint &pos);
    void on_tableViewSent_customContextMenuRequested(const QPoint &pos);

    void on_tableViewSent_doubleClicked(const QModelIndex &index);
    void on_tableViewReceived_doubleClicked(const QModelIndex &index);

    void on_toolButtonReply_clicked();
    void on_toolButtonForward_clicked();
    void on_toolButtonDelete_clicked();
    void on_toolButtonRefresh_clicked();

    void on_toolButtonAddContact_clicked();
    void on_toolButtonPayContact_clicked();
    void on_toolButtonMsgContact_clicked();
    void on_toolButtonInvoiceContact_clicked();
    void on_toolButtonImportCash_clicked();
    void on_toolButtonSettings_clicked();

    void on_treeWidgetAccounts_customContextMenuRequested(const QPoint &pos);

    void on_tabWidgetMain_currentChanged(int index);

    void on_toolButton_payments_clicked();
    void on_toolButton_pending_clicked();
    void on_toolButton_markets_clicked();
    void on_toolButton_importCash_clicked();
    void on_toolButton_contacts_clicked();
    void on_toolButton_smartContracts_clicked();
    void on_toolButton_Corporations_clicked();
    void on_toolButton_quit_clicked();
    void on_toolButton_secrets_clicked();
    void on_toolButton_encrypt_clicked();
    void on_toolButton_sign_clicked();
    void on_toolButton_decrypt_clicked();
    void on_toolButton_transport_clicked();
    void on_toolButton_liveAgreements_clicked();

    void on_tableViewConversation_clicked(const QModelIndex &index);

private:
    Ui::Activity *ui{nullptr};

    QWidget * pTab0_ {nullptr};
    QWidget * pTab1_ {nullptr};
    QWidget * pTab2_ {nullptr};
    QWidget * pTab3_ {nullptr};
    QWidget * pTab4_ {nullptr};
    QWidget * pTab5_ {nullptr};
    QWidget * pTab6_ {nullptr};

    QScopedPointer<QMenu> popupMenuPayments_;
    QScopedPointer<QMenu> popupMenuAccounts_;

    QAction * pActionDelete              {nullptr};
    QAction * pActionOpenNewWindow       {nullptr};
    QAction * pActionReply               {nullptr};
    QAction * pActionForward             {nullptr};
    QAction * pActionMarkRead            {nullptr};
    QAction * pActionMarkUnread          {nullptr};
    QAction * pActionViewContact         {nullptr};
    QAction * pActionCreateContact       {nullptr};
    QAction * pActionExistingContact     {nullptr};
    QAction * pActionAcceptIncoming      {nullptr};
    QAction * pActionCancelOutgoing      {nullptr};
    QAction * pActionDiscardOutgoingCash {nullptr};
    QAction * pActionDiscardIncoming     {nullptr};
    QAction * pActionDownloadCredentials {nullptr};

    QAction * pActionPairWithSNP         {nullptr};
    QAction * pActionManageNotaries      {nullptr};
    QAction * pActionManageAssets        {nullptr};
    QAction * pActionManageAccounts      {nullptr};
    QAction * pActionManageContacts      {nullptr};

    QAction * pActionContactMsg          {nullptr};
    QAction * pActionContactPay          {nullptr};
    QAction * pActionContactInvoice      {nullptr};
    QAction * pActionContactRecurring    {nullptr};

    QAction * pActionBailment            {nullptr};
    QAction * pActionOutbailment         {nullptr};

    QSharedPointer<QStandardItemModel>  pStandardModelMessages_;

//  QSharedPointer<QSqlQueryMessages>  pModelMessages_;
    QSharedPointer<QStandardItemModel> pModelMessages_;
    QSharedPointer<ConvMsgsProxyModel> pThreadItemsProxyModel_;

    QPointer<ActivityPaymentsProxyModel> pPmntProxyModelInbox_;
    QPointer<ActivityPaymentsProxyModel> pPmntProxyModelOutbox_;

    QTableView                 * pCurrentTabTableView_  {nullptr};
    ActivityPaymentsProxyModel * pCurrentTabProxyModel_ {nullptr};

    mapOfActivityTreeItems    mapCurrentRows_inbox; // For each tree item, we store a "currently selected" row so we can re-select it when that tree item is clicked.
    mapOfActivityTreeItems    mapCurrentRows_outbox;

    QList<QModelIndex> listRecordsToMarkAsRead_;
    QList<QModelIndex> listRecordsToMarkAsUnread_;

    QList<QModelIndex> listRecordsToMarkAsReplied_;
    QList<QModelIndex> listRecordsToMarkAsForwarded_;

    bool bRefreshingAfterUpdatedClaims_{false};
};

#endif // ACTIVITY_HPP
