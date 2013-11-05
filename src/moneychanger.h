#ifndef MONEYCHANGER_H
#define MONEYCHANGER_H


#include <QMap>
#include <QtSql>
#include <QMenu>
#include <QList>
#include <QMutex>
#include <QDebug>
#include <QTimer>
#include <QLabel>
#include <QWidget>
#include <QDialog>
#include <QAction>
#include <QVariant>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QTableView>
#include <QScrollArea>
#include <QLayoutItem>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QMutexLocker>
#include <QRadioButton>
#include <QSystemTrayIcon>
#include <QCoreApplication>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QtSql/QtSql>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>
#include <opentxs/OTLog.h>

#include "ot_worker.h"

#include "MTRecordList.hpp"
#include "MTRecord.hpp"

#include "Widgets/marketwindow.h"
#include "Widgets/overviewwindow.h"
#include "Widgets/home.h"
#include "Widgets/withdrawascashwindow.h"
#include "Widgets/withdrawasvoucherwindow.h"
#include "Widgets/depositwindow.h"
#include "Widgets/requestfundswindow.h"
#include "Widgets/sendfundswindow.h"
#include "Widgets/createinsurancecompany.h"
#include "Widgets/detailedit.h"
#include "Widgets/settings.h"

class OverviewWindow;
class MTHome;
class MTDetailEdit;

class WithdrawAsCashWindow;
class WithdrawAsVoucherWindow;
class DepositWindow;
class RequestFundsWindow;
class SendFundsWindow;
class MarketWindow;
class CreateInsuranceCompany;


class Moneychanger : public QWidget
{
    Q_OBJECT
    
public:
    /** Constructor & Destructor **/
    Moneychanger(QWidget *parent = 0);
    ~Moneychanger();
    
    /** Start **/
    void bootTray();

    void downloadAccountData();

    
    /** 
     * Interfaces 
     **/
    
    // Close Dialog Functions
    void close_overview_dialog();
    void close_addressbook();
    void close_accountmanager_dialog();
    void close_nymmanager_dialog();
    void close_servermanager_dialog();
    void close_assetmanager_dialog();
    void close_withdrawascash_dialog();
    void close_withdrawasvoucher_dialog();
    void close_deposit_dialog();
    void close_sendfunds_dialog();
    void close_requestfunds_dialog();
    void close_market_dialog();
    void close_createinsurancecompany_dialog();
    void close_settings_dialog();
    
    //Show address book
    void mc_addressbook_show(QString text);
    
    void new_compose_dialog();
    void new_send_dialog();
    void new_request_dialog();

    
    /**
     * Functions for setting Systray Values
     **/
    
    // Set Systray Nym Value
    void set_systrayMenu_nym_setDefaultNym(QString, QString);
    
    void set_systrayMenu_withdraw_asvoucher_nym_input(QString input);
    
    // Set Systray Asset Value
    void set_systrayMenu_asset_setDefaultAsset(QString, QString);
    
    // Set Systray Account Value
    void set_systrayMenu_account_setDefaultAccount(QString, QString);
    
    // Set Systray Server Value
    void set_systrayMenu_server_setDefaultServer(QString, QString);
    
    
    

    /**
     * Functions for pulling account information out of locally constructed lists.
     **/
    
    QString get_default_nym_id(){return default_nym_id;}
    int get_nym_list_id_size(){return nym_list_id->size();}
    QString get_nym_id_at(int a){return nym_list_id->at(a).toString();}
    QString get_nym_name_at(int a){return nym_list_name->at(a).toString();}
    
    QString get_default_asset_id(){return default_asset_id;}
    int get_asset_list_id_size(){return asset_list_id->size();}
    QString get_asset_id_at(int a){return asset_list_id->at(a).toString();}
    QString get_asset_name_at(int a){return asset_list_name->at(a).toString();}
    
    QString get_default_account_id(){return default_account_id;}
    int get_account_list_id_size(){return account_list_id->size();}
    QString get_account_id_at(int a){return account_list_id->at(a).toString();}
    QString get_account_name_at(int a){return account_list_name->at(a).toString();}
    
    QString get_default_server_id(){return default_server_id;}
    int get_server_list_id_size(){return server_list_id->size();}
    QString get_server_id_at(int a){return server_list_id->at(a).toString();}
    QString get_server_name_at(int a){return server_list_name->at(a).toString();}
    




    
    /**
     * OT Interface Functions
     **/
    
    // These should probably be moved to the main class file.
    ot_worker * get_ot_worker_background(){return ot_worker_background;}; // Should probably replace this soon.

    std::string ot_withdraw_cash(std::string selected_server_id_string, std::string nym_id, std::string selected_account_id_string, int amount_to_withdraw_int){return ot_me->withdraw_cash(selected_server_id_string, nym_id, selected_account_id_string, amount_to_withdraw_int);};
    std::string ot_withdraw_voucher(std::string selected_server_id_string, std::string nym_id, std::string selected_account_id_string, std::string recip_nym_string, std::string memo_string, int amount_to_withdraw_int){return ot_me->withdraw_voucher(selected_server_id_string, nym_id, selected_account_id_string, recip_nym_string, memo_string, amount_to_withdraw_int);};
    
private:

    /**
     * Open Transaction Variables
     **/
    
    OT_ME * ot_me;
    ot_worker * ot_worker_background;
    
    
    
    /**
     * Booleans for tracking initialization
     **/
    
    bool mc_overall_init;

    bool mc_overview_already_init;
    bool mc_market_window_already_init;
    bool mc_addressbook_already_init;
    bool mc_nymmanager_already_init;
    bool mc_assetmanager_already_init;
    bool mc_accountmanager_already_init;
    bool mc_servermanager_already_init;
    bool mc_withdraw_ascash_already_init;
    bool mc_withdraw_asvoucher_already_init;
    bool mc_deposit_already_init;
    bool mc_sendfunds_already_init;
    bool mc_requestfunds_already_init;
    bool mc_createinsurancecompany_already_init;
    bool mc_settings_already_init;

    
    
    /**
     * Window Classes
     **/
    
    OverviewWindow * overviewwindow;
    MTHome * homewindow;

    MTDetailEdit      * contactswindow;
    MTDetailEdit      * nymswindow;
    MTDetailEdit      * serverswindow;
    MTDetailEdit      * assetswindow;
    MTDetailEdit      * accountswindow;

    WithdrawAsCashWindow    * withdrawascashwindow;
    WithdrawAsVoucherWindow * withdrawasvoucherwindow;
    DepositWindow           * depositwindow;
    RequestFundsWindow      * requestfundswindow;
    SendFundsWindow         * sendfundswindow;
    MarketWindow            * market_window;
    CreateInsuranceCompany  * createinsurancecompany_window;
    Settings                * settingswindow;
        
public:
    void SetupMainMenu();

    void mc_nymmanager_dialog();
    void mc_servermanager_dialog();
    void mc_assetmanager_dialog();

private:
    void SetupAssetMenu();
    void SetupServerMenu();
    void SetupNymMenu();
    void SetupAccountMenu();
    // ------------------------------------------------
    void ClearAssetMenu();
    void ClearServerMenu();
    void ClearNymMenu();
    void ClearAccountMenu();
    // ------------------------------------------------
    void ClearMainMenu();

private:
    /**
     * Menu Dialogs
     **/
    
    
    //Overview
    void mc_overview_dialog();
    //Refresh visual
    void mc_overview_dialog_refresh();
    // ------------------------------------------------
    
    //Default Nym
//  void mc_nymmanager_dialog();

    //Reload nym list
    void mc_systrayMenu_reload_nymlist();
    // ------------------------------------------------
    
    //Default Server
//  void mc_servermanager_dialog();

    //Reload server list
    void mc_systrayMenu_reload_serverlist();
    // ------------------------------------------------
    
    //Default Asset
//  void mc_assetmanager_dialog();

    //Reload asset list
    void mc_systrayMenu_reload_assetlist();
    // ------------------------------------------------
    
    //Default Account
    void mc_accountmanager_dialog();
    //Reload account list
    void mc_systrayMenu_reload_accountlist();
    // ------------------------------------------------
    
    //Withdraw As Cash
    void mc_withdraw_ascash_dialog();
    // ------------------------------------------------

    //Withdraw As Voucher
    void mc_withdraw_asvoucher_dialog();
    // ------------------------------------------------
    
    //Deposit
    void mc_deposit_show_dialog();
    // ------------------------------------------------
    
    //Send Funds
    void mc_sendfunds_show_dialog();    
    // ------------------------------------------------

    //Request Funds
    void mc_requestfunds_show_dialog();
    // ------------------------------------------------
    
    //Create Insurance Company
    void mc_createinsurancecompany_dialog();
    // ------------------------------------------------

    
    /**
     * Variables For Various Pieces of Account Information
     **/
    
    
    //pseudonym list (backend) [For nym list in the qmenu and the nym manager]
    QList<QVariant> * nym_list_id;
    QList<QVariant> * nym_list_name;
    
    //pseudonym default selected (backend) [For saving the user supplied default, set from DB and user selections]
    QString default_nym_id;
    QString default_nym_name;
    // ---------------------------------------------------------
    
    QMenu * mc_systrayMenu_server;
    //server list (backend )
    QList<QVariant> * server_list_id;
    QList<QVariant> * server_list_name;
    
    //server default selected (backend)
    QString default_server_id;
    QString default_server_name;
    // ---------------------------------------------------------
    
    
    
    
    /** 
     * Systray Icons
     **/
    
    QSystemTrayIcon * mc_systrayIcon;
    
    QIcon mc_systrayIcon_shutdown;
    
    QIcon mc_systrayIcon_overview;
    
    QIcon mc_systrayIcon_nym;
    QIcon mc_systrayIcon_server;
    
    QIcon mc_systrayIcon_goldaccount;
    QIcon mc_systrayIcon_purse;
    
    QIcon mc_systrayIcon_withdraw;
    QIcon mc_systrayIcon_deposit;
    
    QIcon mc_systrayIcon_sendfunds;
    QIcon mc_systrayIcon_requestfunds;
    
    QIcon mc_systrayIcon_advanced;
    QIcon mc_systrayIcon_advanced_agreements;
    QIcon mc_systrayIcon_advanced_markets;
    QIcon mc_systrayIcon_advanced_settings;
    
    QIcon mc_systrayIcon_advanced_corporations;
    QIcon mc_systrayIcon_advanced_bazaar;
    // ------------------------------------------------
    
    
    
    /**  
     * Systray menu
     **/
    
    //Systray Menu Skeleton
    QMenu * mc_systrayMenu;
    
    QAction * mc_systrayMenu_headertext;
    QAction * mc_systrayMenu_aboveBlank;
    QAction * mc_systrayMenu_shutdown;
    QAction * mc_systrayMenu_overview;
    // ---------------------------------------------------------
    
    //Asset type list (backend )
    QMenu * mc_systrayMenu_asset;

    QList<QVariant> * asset_list_id;
    QList<QVariant> * asset_list_name;
    
    //Asset default selected (backend)
    QString default_asset_id;
    QString default_asset_name;
    // ---------------------------------------------------------
        
    //Account list (backend )
    QMenu * mc_systrayMenu_account;

    QList<QVariant> * account_list_id;
    QList<QVariant> * account_list_name;
    
    //Account default selected (backend)
    QString default_account_id;
    QString default_account_name;
    // ---------------------------------------------------------

    QMenu * mc_systrayMenu_nym;
    
    // ---------------------------------------------------------
    
    QAction * mc_systrayMenu_goldaccount;
    QAction * mc_systrayMenu_purse;
    // ---------------------------------------------------------
    
    //Withdraw submenu
    QMenu * mc_systrayMenu_withdraw;
    
    QAction * mc_systrayMenu_withdraw_ascash;
    QAction * mc_systrayMenu_withdraw_asvoucher;
    // ---------------------------------------------------------
    
    QAction * mc_systrayMenu_deposit;
    // ---------------------------------------------------------
    
    QAction * mc_systrayMenu_sendfunds;
    QAction * mc_systrayMenu_requestfunds;
    // ---------------------------------------------------------
    
    //Company submenu
    QMenu   * mc_systrayMenu_company_create;
    QAction * mc_systrayMenu_company_create_insurance;

    //Advanced submenu
    QMenu * mc_systrayMenu_advanced;

    QAction * mc_systrayMenu_advanced_agreements;
    QAction * mc_systrayMenu_advanced_markets;
    QAction * mc_systrayMenu_advanced_settings;
    QMenu   * mc_systrayMenu_advanced_corporations;
    QMenu   * mc_systrayMenu_advanced_bazaar;

    // Bazaar
    QAction * mc_systrayMenu_bazaar_search;
    QAction * mc_systrayMenu_bazaar_post;
    QAction * mc_systrayMenu_bazaar_orders;
    // ---------------------------------------------------------
    QAction * mc_systrayMenu_bottomblank;
    // ---------------------------------------------------------
    
    
private slots:

    /** 
     * Systray Menu Slots
     **/
    
    //Shutdown
    void mc_shutdown_slot();
    
    //Overview
    void mc_overview_slot();
    
    
    //Nym
    void mc_defaultnym_slot();
    
    //new default nym selected
    void mc_nymselection_triggered(QAction*);
    
    
    //Server
    void mc_defaultserver_slot();

    //new default server selected
    void mc_serverselection_triggered(QAction*);
    

    //Asset
    void mc_defaultasset_slot();
    
    //new default asset selected
    void mc_assetselection_triggered(QAction*);

   
    //Account
    void mc_defaultaccount_slot();
    
    //new default account selected
    void mc_accountselection_triggered(QAction*);
    
    
    //Withdraw As Cash
    void mc_withdraw_ascash_slot();
    
    
    //Withdraw As Voucher
    void mc_withdraw_asvoucher_slot();

    
    //Deposit
    void mc_deposit_slot();

   
    //Send Funds
    void mc_sendfunds_slot();
    
   
    //Request Funds
    void mc_requestfunds_slot();
    
    
    // Market Slot
    void mc_market_slot();
    
    // Create Insurance Company Slot
    void mc_createinsurancecompany_slot();

    //Settings
    void mc_settings_slot();

};

#endif // MONEYCHANGER_H
