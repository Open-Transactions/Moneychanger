#ifndef ACTIVITY_HPP
#define ACTIVITY_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include "core/mapidname.hpp"

#include "core/handlers/DBHandler.hpp"
#include "core/handlers/modelmessages.hpp"

#include <opentxs/opentxs.hpp>

#include <QWidget>
#include <QMenu>
#include <QScopedPointer>
#include <QPointer>
#include <QList>
#include <QListWidgetItem>
#include <QSqlRecord>
#include <QTimer>
#include <QDebug>
#include <QIcon>
#include <QComboBox>

#include <tuple>
#include <map>
#include <set>
#include <string>


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
#define AC_NODE_TYPE_SNP_ISSUER      4
#define AC_NODE_TYPE_SNP_ACCOUNT     5
#define AC_NODE_TYPE_HOSTED_ISSUER   6
#define AC_NODE_TYPE_HOSTED_ACCOUNT  7
#define AC_NODE_TYPE_CONTACT         8

#define USER_ROLE_NODE_TYPE         (Qt::UserRole+0)
#define USER_ROLE_HEADER_TYPE       (Qt::UserRole+1)
#define USER_ROLE_CURRENCY_TYPE     (Qt::UserRole+2)
#define USER_ROLE_MY_NYM_ID         (Qt::UserRole+3)
#define USER_ROLE_ACCOUNT_ID        (Qt::UserRole+4)
#define USER_ROLE_ACCOUNT_UNIT_TYPE (Qt::UserRole+5)
#define USER_ROLE_ACCOUNT_SERVER_ID (Qt::UserRole+6)
#define USER_ROLE_CONTACT_ID        (Qt::UserRole+7)
#define USER_ROLE_ISSUER_WIDGET_ID  (Qt::UserRole+8)
#define USER_ROLE_ACCOUNT_WIDGET_ID (Qt::UserRole+9)


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

typedef std::map<std::string, int> StringIntMap;
typedef std::pair<std::string, int> StringIntPair;
//    std::map<std::pair<nym_id_str, currency>, widget_id_str> MapActivityWidgetIds;
typedef std::map<StringIntPair, std::string> MapActivityWidgetIds;



typedef std::tuple<bool, bool, bool, bool, bool, bool> ActivitySelectionInfo;
//bool     & bSelectedTrusted,  // Is the selection on an SNP?
//bool     & bSelectedHosted,   // or maybe is it (account or issuer) operated by a third party? (Neither might be true -- might be an asset or contact that's selected).
//bool     & bSelectedCurrencyType, // The user has selected the "BTC" total at the top of the list. There is no unit type ID however, but only the currency enum.
//bool     & bSelectedIssuer,   // The user has selected an issuer, which appears as "Stash Demonstration Notary" or "Your Stash Node Pro" (just for a few examples)
//bool     & bSelectedAccount,  // This one is assumed to be accompanied by a nym ID, an asset ID, and a server ID. Also issuer and account widget IDs.
//bool     & bSelectedContact   // This means a Contact is selected, so the user wants to see payments filtered by Contact.


/*
 std::set<int> currency_types = opentxs::SwigWrap::GetCurrencyTypesForLocalAccounts();

 for (const auto currency_type : currency_types)
 {
     std::set<std::string> set_unit_types = opentxs::SwigWrap::GetContractIdsForCurrencyType(currency_type);

     for (const auto & unit_type_id : set_unit_types)
     {
        std::set<std::string> accounts = opentxs::SwigWrap::GetAccountIdsForContractId(unit_type_id);

        for (const auto & account_id : accounts)
        {
            // Here we have:
            //   account_id
            //   unit_type_id
            //   currency_type
        }

     }
 }

 Okay these are now in SwigWrap:
    EXPORT static std::string GetAccountsByCurrency(const int currency);
    EXPORT static std::set<int> GetCurrencyTypesForLocalAccounts();
    EXPORT static std::set<std::string> GetContractIdsForCurrencyType(
        const int currency);
    EXPORT static std::set<std::string> GetAccountIdsForContractId(
        const std::string& ASSET_ID);
*/

class Activity : public QWidget
{
    Q_OBJECT

private:
    bool already_init{false};

    QIcon icon_nym_;

public:
    explicit Activity(QWidget *parent = nullptr);
    ~Activity();

    void dialog(int nSourceRow=-1, int nFolder=-1);

    int nSelectedTab_{0};

    std::string active_account_summary_issuer_widget_id_;
    std::map<std::string, std::string> active_account_widget_id_for_issuer_;

    std::string active_thread_;
    std::map<std::string, std::string> thread_summary_;

    std::map<std::string, std::string> accountWidgets_;

    MapActivityWidgetIds issuers_;

    QString qstrCurrentTLA_;  // If the user clicks on "BTC" near the top, the current TLA is "BTC".
    QString qstrCurrentNotary_; // If the user clicks on "localhost" (hosted notary) then that Notary ID is set here.
    QString qstrCurrentAccount_; // If the user clicks on one of his Opentxs accounts, or a notary with only 1 account under it, we put the acct ID here.
    QString qstrCurrentContact_; // If the user clicks on one of his contacts, we put the Opentxs Contact ID here.
//  QString qstrCurrentWallet_; // If the user clicks on his local BTC wallet, then we set something in here I guess to distinguish it from the local LTC wallet.

    std::set<int> GetCurrencyTypesForLocalAccounts();

    bool GetAccounts(mapIDName & mapOutput, const int nCurrencyType = 0, const std::string str_nym_id = "");

    void AcceptIncoming     (QPointer<ModelPayments> & pModel, ActivityPaymentsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView);
    void CancelOutgoing     (QPointer<ModelPayments> & pModel, ActivityPaymentsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView);
    void DiscardOutgoingCash(QPointer<ModelPayments> & pModel, ActivityPaymentsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView);
    void DiscardIncoming    (QPointer<ModelPayments> & pModel, ActivityPaymentsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView);

    void GetAssetContractIdsInWallet(std::map<std::string, std::string> & map_output);
    void GetCurrencyTypesByAssetContractsInWallet(std::set<StringIntPair> & set_output);

    void Populate_comboBoxCurrency();

    void populateNymComboBoxes();
    void populateNymComboBox(QComboBox * pComboBox);

public slots:
    void onClaimsUpdatedForNym(QString nymId);
    void onRecordlistPopulated();
    void onBalancesChanged();
    void onNeedToRefreshUserBar();
    void onNeedToRefreshRecords();
    void resetPopupMenus();
    void onOpentxsWidgetUpdated(QString qstrWidgetID);

signals:
    void RefreshRecordsAndUpdateMenu();
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

    void PopulateIssuerWidgetIds();

    bool RetrieveSelectedIdsAccountTab(
            int      & nNodeType,
            int      & nUnderHeader,
            int      & nCurrencyType,
            QString  & qstrMyNymId,
            QString  & qstrAccountId,
            QString  & qstrAssetTypeId,
            QString  & qstrServerId,
            QString  & qstrContactId,
            QString  & qstrIssuerWidgetId,
            QString  & qstrAccountWidgetId,
            ActivitySelectionInfo & aboutSelection
            );

    bool RetrieveSelectedIdsChatTab(
            QString  & qstrSelectedMyNymId,
            QString  & qstrSelectedThreadId,
            QString  & qstrActivitySummaryId
        );

    QSharedPointer<QStandardItemModel>  getAccountActivityModel();

    void enableButtons();
    void disableButtons();

//  void doSearchConversations(QString qstrInput);
//  void doSearchPayments(QString qstrInput);

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
    void RefreshConversationDetails(int nRow);
    void ClearListWidgetConversations();

    void PopulateConversationsForNym(
        QListWidget * pListWidgetConversations,
        int & nIndex,
        const std::string & str_my_nym_id,
        const std::string & activity_summary_id,
        QListWidgetItem  *& pItemToSelect);

//  int PairedNodeCount(std::set<opentxs::Identifier> * pUniqueServers=nullptr);
//  bool PairingStarted(const opentxs::Identifier & nymId, const opentxs::Identifier & issuerNymId);

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

    void treeWidgetSummary_PopupMenu(const QPoint &pos, QTreeWidget * pTreeWidget);

private slots:
    void on_listWidgetConversations_currentRowChanged(int currentRow);
    void on_listWidgetConversations_customContextMenuRequested(const QPoint &pos);

    void on_treeWidgetSummary_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_plainTextEditMsg_textChanged();

//  void on_checkBoxSearchConversations_toggled(bool checked);
//  void on_pushButtonSearchConversations_clicked();
//  void on_lineEditSearchConversations_textChanged(const QString &arg1);
//  void on_lineEditSearchConversations_returnPressed();

//  void on_checkBoxSearchPayments_toggled(bool checked);
//  void on_lineEditSearchPayments_textChanged(const QString &arg1);

    void on_tableViewPaymentsSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous);
//  void on_tableViewReceivedSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous);

    void on_tabWidgetTransactions_currentChanged(int index);

    void on_MarkAsRead_timer();
    void on_MarkAsUnread_timer();
    void on_MarkAsReplied_timer();
    void on_MarkAsForwarded_timer();

//  void RefreshAccountTree();
    void RefreshSummaryTree();
//  void RefreshPayments();
    void NewRefreshPayments();

    void RefreshConversationsTab();

//  void on_tableViewReceived_customContextMenuRequested(const QPoint &pos);
    void on_tableViewPayments_customContextMenuRequested(const QPoint &pos);

    void on_tableViewPayments_doubleClicked(const QModelIndex &index);
//  void on_tableViewReceived_doubleClicked(const QModelIndex &index);

    void on_toolButtonReply_clicked();
    void on_toolButtonForward_clicked();
    void on_toolButtonDelete_clicked();
    void on_toolButtonRefresh_clicked();

    void on_toolButtonAddContact_clicked();
//  void on_toolButtonPayContact_clicked();
//  void on_toolButtonMsgContact_clicked();
    void on_toolButtonInvoiceContact_clicked();
    void on_toolButtonImportCash_clicked();
    void on_toolButtonSettings_clicked();

    void on_treeWidgetSummary_customContextMenuRequested(const QPoint &pos);

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

    void on_comboBoxMyNym_activated(int index);
    void on_comboBoxCurrency_activated(int index);
    void on_comboBoxMyNymChat_activated(int index);

    void on_toolButtonDeposit_clicked();
    void on_toolButtonWithdraw_clicked();
    void on_toolButtonPay_clicked();
    void on_toolButtonPayContact_clicked();
    void on_toolButtonMsgContact_clicked();

    void on_toolButtonManageContacts_clicked();

    void on_toolButtonSettings_2_clicked();

    void on_toolButtonPairNode_clicked();

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

    QSharedPointer<QStandardItemModel>   pModelPayments_;
    QPointer<ActivityPaymentsProxyModel> pPmntProxyModel_;
//  QPointer<ActivityPaymentsProxyModel> pPmntProxyModelInbox_;
//  QPointer<ActivityPaymentsProxyModel> pPmntProxyModelOutbox_;

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
