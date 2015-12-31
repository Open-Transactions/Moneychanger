#ifndef MONEYCHANGER_HPP
#define MONEYCHANGER_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"
#include "core/TR1_Wrapper.hpp"

#include <core/handlers/focuser.h>

#include <opentxs/client/OTRecordList.hpp>

#include <namecoin/Namecoin.hpp>

#include _CINTTYPES
#include _MEMORY

#include <QPointer>
#include <QList>
#include <QIcon>
#include <QVariant>
#include <QWidget>

class MTHome;
class MTDetailEdit;
class DlgLog;
class DlgMenu;
class DlgMarkets;
class DlgTradeArchive;
class Settings;
class BtcGuiTest;
class BtcPoolManager;
class BtcTransactionManager;
class BtcConnectDlg;
class BtcSendDlg;
class BtcReceiveDlg;
class DlgPassphraseManager;
class Messages;
class Payments;
class QMenu;
class QSystemTrayIcon;
class CreateInsuranceCompany;

class Moneychanger : public QWidget
{
    Q_OBJECT

private:
    // ------------------------------------------------
    opentxs::OTRecordList   m_list;
    // ------------------------------------------------
    /** Constructor & Destructor **/
    Moneychanger(QWidget *parent = 0);
public:
    virtual ~Moneychanger();
    
    static Moneychanger * It(QWidget *parent = 0, bool bShuttingDown = false);

    int64_t HasUsageCredits(const std::string & notary_id,
                            const std::string & NYM_ID);

    int64_t HasUsageCredits(QString   notary_id,
                            QString   NYM_ID);

    bool hideNav() const { return bHideNav_; }
    bool expertMode() const { return bExpertMode_; }
    bool hasNyms() const;
    bool hasAccounts() const;

    /** Start **/
    void bootTray();
    
    opentxs::OTRecordList & GetRecordlist();
    void setupRecordList();  // Sets up the RecordList object with the IDs etc.
    void populateRecords();  // Calls OTRecordList::Populate(), and then additionally adds records from Bitmessage, etc.

    void modifyRecords(); // After we populate the recordlist, we make some changes to the list (move messages to a separate db table, move receipts to a separate table, etc.)
    bool AddMailToMsgArchive(opentxs::OTRecord& recordmt);
    bool AddPaymentToPmntArchive(opentxs::OTRecord& recordmt, const bool bCanDeleteRecord=true);
    void AddPaymentBasedOnNotification(const std::string & str_acct_id,
                                       const std::string & p_nym_id,
                                       const std::string & p_notary_id,
                                       const std::string & p_txn_contents,
                                       int64_t & lTransactionNum,
                                       int64_t & lTransNumForDisplay);
    bool AddFinalReceiptToTradeArchive(opentxs::OTRecord& recordmt);

signals:
    void balancesChanged();
    void populatedRecordlist();
    void appendToLog(QString);
    void expertModeUpdated(bool);
    void hideNavUpdated(bool);
    void claimsUpdatedForNym(QString);

public slots:
    void onBalancesChanged();
    void onNeedToUpdateMenu();
    void onNeedToPopulateRecordlist();
    void onNeedToDownloadAccountData();
    void onNeedToDownloadSingleAcct(QString qstrAcctID, QString qstrOptionalAcctID);
    void onNeedToDownloadMail();
    void onExpertModeUpdated(bool bExpertMode);
    void onHideNavUpdated(bool bHideNav);
    void onCheckNym(QString nymId);

    /**
     * Functions for setting Systray Values
     **/
    
    void setDefaultNym(QString, QString);
    QString getDefaultNymID(){return default_nym_id;}
    QString getDefaultNymName(){return default_nym_name;}

    void setDefaultAsset(QString, QString);
    QString getDefaultAssetID(){return default_asset_id;}
    QString getDefaultAssetName(){return default_asset_name;}

    void setDefaultAccount(QString, QString);
    QString getDefaultAccountID(){return default_account_id;}
    QString getDefaultAccountName(){return default_account_name;}

    void setDefaultServer(QString, QString);
    QString getDefaultNotaryID(){return default_notary_id;}
    QString getDefaultServerName(){return default_server_name;}
    
    void onNewServerAdded(QString qstrID);
    void onNewAssetAdded(QString qstrID);
    void onNewNymAdded(QString qstrID);  // NOTE This means "new private Nym created in local wallet" -- not, "New public Nym downloaded from outside."
    void onNewAccountAdded(QString qstrID);
    
    void onServersChanged();
    void onAssetsChanged();
    void onNymsChanged();
    void onAccountsChanged();

    void onRunSmartContract(QString qstrTemplate, QString qstrLawyer, int32_t index);

public:

    /**
     * Functions for pulling account information out of locally constructed lists.
     **/
    
    QString get_default_nym_id(){return default_nym_id;}
    int get_nym_list_id_size(){return nym_list_id ? nym_list_id->size() : 0;}
    QString get_nym_id_at(int a){return nym_list_id ? nym_list_id->at(a).toString() : "";}
    QString get_nym_name_at(int a){return nym_list_name ? nym_list_name->at(a).toString() : "";}
    
    QString get_default_asset_id(){return default_asset_id;}
    int get_asset_list_id_size(){return asset_list_id ? asset_list_id->size() : 0;}
    QString get_asset_id_at(int a){return asset_list_id ? asset_list_id->at(a).toString() : "";}
    QString get_asset_name_at(int a){return asset_list_name ? asset_list_name->at(a).toString() : "";}
    
    QString get_default_account_id(){return default_account_id;}
    int get_account_list_id_size(){return account_list_id ? account_list_id->size() : 0;}
    QString get_account_id_at(int a){return account_list_id ? account_list_id->at(a).toString() : "";}
    QString get_account_name_at(int a){return account_list_name ? account_list_name->at(a).toString() : "";}
    
    QString get_default_notary_id(){return default_notary_id;}
    int get_server_list_id_size(){return server_list_id ? server_list_id->size() : 0;}
    QString get_notary_id_at(int a){return server_list_id ? server_list_id->at(a).toString() : "";}
    QString get_server_name_at(int a){return server_list_name ? server_list_name->at(a).toString() : "";}
    
    
private:

    /** Namecoin interface used for the NameManager.  */
    NMC_Interface* nmc=nullptr;
    /** Namecoin name manager.  */
    NMC_NameManager* nmc_names=nullptr;
    
    /**
     * Booleans for tracking initialization
     **/
    
    bool mc_overall_init=false;

    bool bExpertMode_=false;
    bool bHideNav_=false;

    /** Timer used to update Namecoin names.  */
    QTimer* nmc_update_timer=nullptr;
    
    /**
     * Window Classes
     **/
    
    QPointer<MTHome>  homewindow;
    QPointer<DlgMenu> menuwindow;

    QPointer<Messages> messages_window;
    QPointer<Payments> payments_window;

    QPointer<MTDetailEdit> contactswindow;
    QPointer<MTDetailEdit> nymswindow;
    QPointer<MTDetailEdit> serverswindow;
    QPointer<MTDetailEdit> assetswindow;
    QPointer<MTDetailEdit> accountswindow;
    QPointer<MTDetailEdit> corporation_window;
    QPointer<MTDetailEdit> agreement_window;
    QPointer<MTDetailEdit> transport_window;
    QPointer<DlgPassphraseManager> passphrase_window;

    QPointer<CreateInsuranceCompany> createinsurancecompany_window;

    QPointer<DlgLog           > log_window;
    QPointer<DlgMarkets       > market_window;
    QPointer<DlgTradeArchive  > trade_archive_window;

    QPointer<Settings> settingswindow;

    QPointer<BtcGuiTest> bitcoinwindow;
    QPointer<BtcPoolManager> bitcoinPoolWindow;
    QPointer<BtcTransactionManager> bitcoinTxWindow;
    QPointer<BtcConnectDlg> bitcoinConnectWindow;
    QPointer<BtcSendDlg> bitcoinSendWindow;
    QPointer<BtcReceiveDlg> bitcoinReceiveWindow;
        
private:
    void SetupMainMenu();

    void mc_nymmanager_dialog    (QString qstrPresetID=QString(""));
    void mc_servermanager_dialog (QString qstrPresetID=QString(""));
    void mc_assetmanager_dialog  (QString qstrPresetID=QString(""));
    void mc_accountmanager_dialog(QString qstrAcctID=QString(""));

    void mc_addressbook_show(QString text=QString(""));

    void mc_overview_dialog_refresh();

private:
    void SetupAdvancedMenu(QPointer<QMenu> & parent_menu);
    void SetupExperimentalMenu(QPointer<QMenu> & parent_menu);
    void SetupAssetMenu(QPointer<QMenu> & parent_menu);
    void SetupServerMenu(QPointer<QMenu> & parent_menu);
    void SetupNymMenu(QPointer<QMenu> & parent_menu);
    void SetupAccountMenu(QPointer<QMenu> & parent_menu);
    void SetupPaymentsMenu(QPointer<QMenu> & parent_menu);
    void SetupExchangeMenu(QPointer<QMenu> & parent_menu);
    void SetupContractsMenu(QPointer<QMenu> & parent_menu);
    void SetupMessagingMenu(QPointer<QMenu> & parent_menu);
    void SetupToolsMenu(QPointer<QMenu> & parent_menu);
    // ------------------------------------------------
    // Used for smart contracts.

    bool showPartyAccounts(const std::string& contract, const std::string& name, std::string & str_output);
    int32_t sendToNextParty(const std::string& server, const std::string& mynym,
                            const std::string& hisnym,
                            const std::string& contract);
    int32_t activateContract(const std::string& server, const std::string& mynym,
                             const std::string& contract, const std::string& name,
                             std::string myAcctID,
                             std::string myAcctAgentName);
    // ------------------------------------------------

private:
    /**
     * Menu Dialogs
     **/
    
    
    void mc_overview_dialog();
    void mc_main_menu_dialog(bool bShow=true);
    void mc_messages_dialog();
    void mc_payments_dialog(int nSourceRow=-1, int nFolder=-1);
    // ------------------------------------------------
    void mc_sendfunds_show_dialog(QString qstrAcct=QString(""));
    void mc_requestfunds_show_dialog(QString qstrAcct=QString(""));
    void mc_proposeplan_show_dialog(QString qstrAcct=QString(""));
    // ------------------------------------------------
    void mc_composemessage_show_dialog();
    // ------------------------------------------------
    void mc_encrypt_show_dialog(bool bEncrypt=true, bool bSign=true);
    void mc_decrypt_show_dialog();
    // ------------------------------------------------
    void mc_passphrase_manager_show_dialog();
    // ------------------------------------------------
    void mc_market_dialog();
    void mc_trade_archive_dialog();
    void mc_corporation_dialog();
    void mc_agreement_dialog();
    void mc_transport_dialog(QString qstrPresetID=QString(""));
    void mc_log_dialog(QString qstrAppend=QString(""));
    void mc_createinsurancecompany_dialog();
    // ------------------------------------------------    
    QList<QVariant> * nym_list_id=nullptr;
    QList<QVariant> * nym_list_name=nullptr;
    // ---------------------------------------------------------
    QString default_nym_id;
    QString default_nym_name;
    // ---------------------------------------------------------
    QList<QVariant> * server_list_id=nullptr;
    QList<QVariant> * server_list_name=nullptr;
    // ---------------------------------------------------------
    QString default_notary_id;
    QString default_server_name;
    // ---------------------------------------------------------
    
    /** 
     * Systray Icons
     **/
    
    QPointer<QSystemTrayIcon> mc_systrayIcon;
    
    QIcon mc_systrayIcon_shutdown;
    
    QIcon mc_systrayIcon_overview;
    
    QIcon mc_systrayIcon_nym;
    QIcon mc_systrayIcon_server;
    
    QIcon mc_systrayIcon_goldaccount;
    QIcon mc_systrayIcon_purse;
    
    QIcon mc_systrayIcon_sendfunds;
    QIcon mc_systrayIcon_requestfunds;
    QIcon mc_systrayIcon_proposeplan;
    QIcon mc_systrayIcon_contacts;
    QIcon mc_systrayIcon_composemessage;

    QIcon mc_systrayIcon_markets;
    QIcon mc_systrayIcon_trade_archive;

    QIcon mc_systrayIcon_bitcoin;
    QIcon mc_systrayIcon_crypto;
    QIcon mc_systrayIcon_crypto_encrypt;
    QIcon mc_systrayIcon_crypto_decrypt;
    QIcon mc_systrayIcon_crypto_sign;
    QIcon mc_systrayIcon_crypto_verify;

    QIcon mc_systrayIcon_advanced;
    QIcon mc_systrayIcon_advanced_agreements;
    QIcon mc_systrayIcon_advanced_import;
    QIcon mc_systrayIcon_settings;
    
    QIcon mc_systrayIcon_advanced_corporations;
    QIcon mc_systrayIcon_advanced_transport;
    QIcon mc_systrayIcon_advanced_log;
    QIcon mc_systrayIcon_advanced_bazaar;
    // ------------------------------------------------
    /**
     * Systray menu
     **/
    
    //Systray Menu Skeleton
    QPointer<QMenu> mc_systrayMenu;
    
    QPointer<QAction> mc_systrayMenu_headertext;
    QPointer<QAction> mc_systrayMenu_aboveBlank;
    QPointer<QAction> mc_systrayMenu_shutdown;
    QPointer<QAction> mc_systrayMenu_overview;
    // ---------------------------------------------------------
    QPointer<QMenu> mc_systrayMenu_asset;
    // ---------------------------------------------------------
    QList<QVariant> * asset_list_id = nullptr;
    QList<QVariant> * asset_list_name = nullptr;
    // ---------------------------------------------------------
    QString default_asset_id;
    QString default_asset_name;
    // ---------------------------------------------------------        
    QPointer<QMenu> mc_systrayMenu_account;
    QPointer<QMenu> mc_systrayMenu_payments;
    QPointer<QMenu> mc_systrayMenu_contracts;
    QPointer<QMenu> mc_systrayMenu_exchange;
    QPointer<QMenu> mc_systrayMenu_messaging;
    // ---------------------------------------------------------
    QList<QVariant> * account_list_id = nullptr;
    QList<QVariant> * account_list_name = nullptr;
    // ---------------------------------------------------------
    QString default_account_id;
    QString default_account_name;
    // ---------------------------------------------------------
    QPointer<QMenu> mc_systrayMenu_nym;
    QPointer<QMenu> mc_systrayMenu_server;
    // ---------------------------------------------------------
    QPointer<QAction> mc_systrayMenu_goldaccount;
    QPointer<QAction> mc_systrayMenu_purse;
    // ---------------------------------------------------------    
    QPointer<QAction> mc_systrayMenu_sendfunds;
    QPointer<QAction> mc_systrayMenu_requestfunds;
    QPointer<QAction> mc_systrayMenu_proposeplan;
    QPointer<QAction> mc_systrayMenu_contacts;
    QPointer<QAction> mc_systrayMenu_receipts;
    QPointer<QAction> mc_systrayMenu_messages;
    QPointer<QAction> mc_systrayMenu_composemessage;
    QPointer<QAction> mc_systrayMenu_passphrase_manager;
    // ---------------------------------------------------------
    //Company submenu
    QPointer<QMenu>   mc_systrayMenu_company_create;
    QPointer<QAction> mc_systrayMenu_company_create_insurance;
    // ---------------------------------------------------------
    //Crypto submenu
    QPointer<QMenu> mc_systrayMenu_tools;

    QPointer<QAction> mc_systrayMenu_crypto_encrypt;
    QPointer<QAction> mc_systrayMenu_crypto_decrypt;
    QPointer<QAction> mc_systrayMenu_crypto_sign;
    QPointer<QAction> mc_systrayMenu_crypto_verify;
    // ---------------------------------------------------------
    //Advanced submenu
    QPointer<QMenu>   mc_systrayMenu_advanced;
    // ---------------------------------------------------------
    //Experimental submenu
    QPointer<QMenu>   mc_systrayMenu_experimental;
    QPointer<QAction> mc_systrayMenu_corporations;
    QPointer<QMenu>   mc_systrayMenu_bazaar;
    // ---------------------------------------------------------
    QPointer<QAction> mc_systrayMenu_markets;
    QPointer<QAction> mc_systrayMenu_trade_archive;
    QPointer<QAction> mc_systrayMenu_smart_contracts;
    QPointer<QAction> mc_systrayMenu_import_cash;
    QPointer<QAction> mc_systrayMenu_settings;
    QPointer<QAction> mc_systrayMenu_p2p_transport;
    QPointer<QAction> mc_systrayMenu_error_log;
    // ---------------------------------------------------------
    // Bazaar
    QPointer<QAction> mc_systrayMenu_bazaar_search;
    QPointer<QAction> mc_systrayMenu_bazaar_post;
    QPointer<QAction> mc_systrayMenu_bazaar_orders;
    // ---------------------------------------------------------
    QPointer<QAction> mc_systrayMenu_bottomblank;
    // ---------------------------------------------------------

    // Bitcoin
    QPointer<QMenu> mc_systrayMenu_bitcoin;
    QPointer<QAction> mc_systrayMenu_bitcoin_test;
    QPointer<QAction> mc_systrayMenu_bitcoin_connect;
    QPointer<QAction> mc_systrayMenu_bitcoin_pools;
    QPointer<QAction> mc_systrayMenu_bitcoin_transactions;
    QPointer<QAction> mc_systrayMenu_bitcoin_send;
    QPointer<QAction> mc_systrayMenu_bitcoin_receive;
    
    
public slots:

    /**
     * Namecoin update timer event.
     */
    void nmc_timer_event();

    /** 
     * Systray Menu Slots
     **/
    
    void mc_shutdown_slot(); //Shutdown
    // ---------------------------------------------------------------------------
    void mc_overview_slot();                // Overview
    void mc_main_menu_slot();               // Main Menu
    // ---------------------------------------------------------------------------
    void mc_addressbook_slot();             // Address Book
    void mc_showcontact_slot(QString text); // Address Book, Select a Contact
    // ---------------------------------------------------------------------------
    void mc_transport_slot();                 // Various Transport connection strings
    void mc_showtransport_slot(QString text); // Same, except choose a specific one when opening.
    // ---------------------------------------------------------------------------
    void mc_log_slot();
    void mc_showlog_slot(QString text);     // Same, except appends a string when opening.
    // ---------------------------------------------------------------------------
    void mc_defaultnym_slot();              // Nym
    void mc_nymselection_triggered(QAction*); //new default nym selected
    // ---------------------------------------------------------------------------
    void mc_defaultserver_slot();           // Server
    void mc_serverselection_triggered(QAction*); //new default server selected
    // ---------------------------------------------------------------------------
    void mc_defaultasset_slot();            // Asset
    void mc_assetselection_triggered(QAction*);  //new default asset selected
    // ---------------------------------------------------------------------------
    void mc_defaultaccount_slot();          // Account
    void mc_accountselection_triggered(QAction*); //new default account selected
    // ---------------------------------------------------------------------------
    void mc_show_account_slot(QString text);
    void mc_show_account_manager_slot();
    // ---------------------------------------------------------------------------
    void mc_show_asset_slot (QString text);
    void mc_show_nym_slot   (QString text);
    void mc_show_server_slot(QString text);
    // ---------------------------------------------------------------------------
    void mc_sendfunds_slot();               // Send Funds
    void mc_requestfunds_slot();            // Request Funds
    void mc_proposeplan_slot();             // Propose Payment Plan
    void mc_composemessage_slot();          // Compose Message
    void mc_messages_slot();
    void mc_payments_slot();
    void mc_show_payment_slot(int nSourceRow, int nFolder);
    // ---------------------------------------------------------------------------
    void mc_proposeplan_from_acct (QString qstrAcct);
    void mc_send_from_acct (QString qstrAcct);
    void mc_request_to_acct(QString qstrAcct);
    // ---------------------------------------------------------------------------
    void mc_passphrase_manager_slot();
    void mc_crypto_encrypt_slot();
    void mc_crypto_decrypt_slot();
    void mc_crypto_sign_slot();
    void mc_crypto_verify_slot();
    // ---------------------------------------------------------------------------
    void mc_market_slot();                  // Market Slot
    void mc_trade_archive_slot();
    void mc_agreement_slot();               // Agreements Slot
    void mc_corporation_slot();             // Agreements Slot
    void mc_import_slot();                  // Import Slot
    // ---------------------------------------------------------------------------
    void mc_createinsurancecompany_slot();  // Create Insurance Company Slot
    // ---------------------------------------------------------------------------
    void mc_settings_slot();                //Settings
    // ---------------------------------------------------------------------------
    void mc_bitcoin_slot();                 // Bitcoin -> Test
    void mc_bitcoin_connect_slot();         // Bitcoin -> Connect to wallet
    void mc_bitcoin_pools_slot();           // Bitcoin -> Mange Pools
    void mc_bitcoin_transactions_slot();    // Bitcoin -> Transactions
    void mc_bitcoin_send_slot();            // Bitcoin -> Send
    void mc_bitcoin_receive_slot();            // Bitcoin -> Receive

    // ---------------------------------------------------------------------------
    // RPC callback functions
    // ---------------------------------------------------------------------------

    void mc_rpc_sendfunds_show_dialog(QString qstrAcct=QString(""), QString qstrRecipientNym=QString(""),
                                      QString qstrAsset=QString(""), QString qstrAmount=QString(""));

    void mc_rpc_requestfunds_show_dialog(QString qstrAcct=QString(""), QString qstrRecipientNym=QString(""),
                                         QString qstrAsset=QString(""), QString qstrAmount=QString(""));
    void mc_rpc_messages_show_dialog();
    void mc_rpc_exchange_show_dialog();
    //void mc_rpc_payments_show_dialog();
    void mc_rpc_manage_accounts_show_dialog();
    void mc_rpc_manage_nyms_show_dialog();
    void mc_rpc_manage_assets_show_dialog();
    void mc_rpc_manage_smartcontracts_show_dialog();

};

#endif // MONEYCHANGER_HPP
