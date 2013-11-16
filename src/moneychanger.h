#ifndef MONEYCHANGER_H
#define MONEYCHANGER_H


#include <QMap>
#include <QPointer>
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

#include "MTRecordList.hpp"
#include "MTRecord.hpp"

#include "Widgets/home.h"

#include "Widgets/createinsurancecompany.h"

#include "UI/dlgmarkets.h"

#include "Widgets/detailedit.h"
#include "Widgets/settings.h"

class MTHome;
class MTDetailEdit;

class CreateInsuranceCompany;


class Moneychanger : public QWidget
{
    Q_OBJECT
    
public:
    /** Constructor & Destructor **/
    Moneychanger(QWidget *parent = 0);
    virtual ~Moneychanger();
    
    /** Start **/
    void bootTray();

    
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

    void close_sendfunds_dialog();
    void close_requestfunds_dialog();

    void close_market_dialog();

    void close_agreement_dialog();
    void close_corporation_dialog();

    void close_settings_dialog();

    void close_createinsurancecompany_dialog();

    //Show address book
    void mc_addressbook_show(QString text);
    
    void new_compose_dialog();
    void new_send_dialog();
    void new_request_dialog();

signals:
    void balancesChanged();

public slots:

    void onBalancesChanged();

    void downloadAccountData();

    /**
     * Functions for setting Systray Values
     **/
    
    // Set Systray Nym Value
    void setDefaultNym(QString, QString);
    
    // Set Systray Asset Value
    void setDefaultAsset(QString, QString);
    
    // Set Systray Account Value
    void setDefaultAccount(QString, QString);
    
    // Set Systray Server Value
    void setDefaultServer(QString, QString);
    
    
public:

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
    
    
private:

    /**
     * Open Transaction Variables
     **/
        
    
    
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

    bool mc_sendfunds_already_init;
    bool mc_requestfunds_already_init;

    bool mc_createinsurancecompany_already_init;
    bool mc_settings_already_init;

    bool mc_agreement_already_init;
    bool mc_corporation_already_init;

    
    /**
     * Window Classes
     **/
    
    QPointer<MTHome> homewindow;

    QPointer<MTDetailEdit> contactswindow;
    QPointer<MTDetailEdit> nymswindow;
    QPointer<MTDetailEdit> serverswindow;
    QPointer<MTDetailEdit> assetswindow;
    QPointer<MTDetailEdit> accountswindow;
    QPointer<MTDetailEdit> corporation_window;
    QPointer<MTDetailEdit> agreement_window;

    QPointer<DlgMarkets  > market_window;


    QPointer<CreateInsuranceCompany> createinsurancecompany_window;
    QPointer<Settings> settingswindow;
        
public:
    void SetupMainMenu();

    void mc_nymmanager_dialog   (QString qstrPresetID=QString(""));
    void mc_servermanager_dialog(QString qstrPresetID=QString(""));
    void mc_assetmanager_dialog (QString qstrPresetID=QString(""));

    void mc_overview_dialog_refresh();

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
//  void mc_overview_dialog_refresh();
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
public:
    void mc_accountmanager_dialog(QString qstrAcctID=QString(""));

private:
    //Reload account list
    void mc_systrayMenu_reload_accountlist();
    // ------------------------------------------------
    //Send Funds
    void mc_sendfunds_show_dialog();    
    // ------------------------------------------------

    //Request Funds
    void mc_requestfunds_show_dialog();
    // ------------------------------------------------
    
    void mc_market_dialog();

    void mc_corporation_dialog();

    void mc_agreement_dialog();

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
    
    QPointer<QMenu> mc_systrayMenu_server;
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
    
    QPointer<QSystemTrayIcon> mc_systrayIcon;
    
    QIcon mc_systrayIcon_shutdown;
    
    QIcon mc_systrayIcon_overview;
    
    QIcon mc_systrayIcon_nym;
    QIcon mc_systrayIcon_server;
    
    QIcon mc_systrayIcon_goldaccount;
    QIcon mc_systrayIcon_purse;
    
    QIcon mc_systrayIcon_sendfunds;
    QIcon mc_systrayIcon_requestfunds;
    
    QIcon mc_systrayIcon_markets;

    QIcon mc_systrayIcon_advanced;
    QIcon mc_systrayIcon_advanced_agreements;
    QIcon mc_systrayIcon_advanced_import;
    QIcon mc_systrayIcon_advanced_settings;
    
    QIcon mc_systrayIcon_advanced_corporations;
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
    
    //Asset type list (backend )
    QPointer<QMenu> mc_systrayMenu_asset;

    QList<QVariant> * asset_list_id;
    QList<QVariant> * asset_list_name;
    
    //Asset default selected (backend)
    QString default_asset_id;
    QString default_asset_name;
    // ---------------------------------------------------------
        
    //Account list (backend )
    QPointer<QMenu> mc_systrayMenu_account;

    QList<QVariant> * account_list_id;
    QList<QVariant> * account_list_name;
    
    //Account default selected (backend)
    QString default_account_id;
    QString default_account_name;
    // ---------------------------------------------------------
    QPointer<QMenu> mc_systrayMenu_nym;
    // ---------------------------------------------------------
    QPointer<QAction> mc_systrayMenu_goldaccount;
    QPointer<QAction> mc_systrayMenu_purse;
    // ---------------------------------------------------------    
    QPointer<QAction> mc_systrayMenu_sendfunds;
    QPointer<QAction> mc_systrayMenu_requestfunds;
    // ---------------------------------------------------------
    QPointer<QAction> mc_systrayMenu_markets;

    //Company submenu
    QPointer<QMenu>   mc_systrayMenu_company_create;
    QPointer<QAction> mc_systrayMenu_company_create_insurance;

    //Advanced submenu
    QPointer<QMenu> mc_systrayMenu_advanced;

    QPointer<QAction> mc_systrayMenu_advanced_agreements;
    QPointer<QAction> mc_systrayMenu_advanced_import;
    QPointer<QAction> mc_systrayMenu_advanced_settings;
    QPointer<QAction> mc_systrayMenu_advanced_corporations;
    QPointer<QMenu>   mc_systrayMenu_advanced_bazaar;

    // Bazaar
    QPointer<QAction> mc_systrayMenu_bazaar_search;
    QPointer<QAction> mc_systrayMenu_bazaar_post;
    QPointer<QAction> mc_systrayMenu_bazaar_orders;
    // ---------------------------------------------------------
    QPointer<QAction> mc_systrayMenu_bottomblank;
    // ---------------------------------------------------------
    
    
private slots:

    /** 
     * Systray Menu Slots
     **/
    
    //Shutdown
    void mc_shutdown_slot();
    // ---------------------------------------------------------------------------
    void mc_overview_slot();                // Overview
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
    void mc_sendfunds_slot();               // Send Funds
    void mc_requestfunds_slot();            // Request Funds
    void mc_market_slot();                  // Market Slot
    void mc_agreement_slot();               // Agreements Slot
    void mc_corporation_slot();               // Agreements Slot
    void mc_import_slot();                  // Import Slot
    // ---------------------------------------------------------------------------
    void mc_createinsurancecompany_slot();  // Create Insurance Company Slot
    // ---------------------------------------------------------------------------
    void mc_settings_slot();                //Settings
};

#endif // MONEYCHANGER_H
