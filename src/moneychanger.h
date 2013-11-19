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
#include "Namecoin.hpp"

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

private:
    /** Constructor & Destructor **/
    Moneychanger(QWidget *parent = 0);
public:
    virtual ~Moneychanger();
    
    static Moneychanger * It(QWidget *parent = 0);

    /** Start **/
    void bootTray();
    
signals:
    void balancesChanged();
    void downloadedAccountData();

public slots:

    void onBalancesChanged();
    void onNeedToDownloadAccountData();

    /**
     * Functions for setting Systray Values
     **/
    
    void setDefaultNym(QString, QString);
    void setDefaultAsset(QString, QString);
    void setDefaultAccount(QString, QString);
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
    
    
private:

    /** Namecoin name manager.  */
    NMC_NameManager* nmc_names;
    
    /**
     * Booleans for tracking initialization
     **/
    
    bool mc_overall_init;

    /** Timer used to update Namecoin names.  */
    QTimer* nmc_update_timer;
    
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
    void mc_accountmanager_dialog(QString qstrAcctID=QString(""));

    void mc_addressbook_show(QString text);

    void mc_overview_dialog_refresh();

private:
    void SetupAssetMenu();
    void SetupServerMenu();
    void SetupNymMenu();
    void SetupAccountMenu();
    // ------------------------------------------------

private:
    /**
     * Menu Dialogs
     **/
    
    
    void mc_overview_dialog();
    // ------------------------------------------------
    void mc_sendfunds_show_dialog();    
    void mc_requestfunds_show_dialog();
    // ------------------------------------------------
    void mc_market_dialog();
    void mc_corporation_dialog();
    void mc_agreement_dialog();
    void mc_createinsurancecompany_dialog();
    // ------------------------------------------------    
    QList<QVariant> * nym_list_id;
    QList<QVariant> * nym_list_name;
    // ---------------------------------------------------------
    QString default_nym_id;
    QString default_nym_name;
    // ---------------------------------------------------------
    QList<QVariant> * server_list_id;
    QList<QVariant> * server_list_name;
    // ---------------------------------------------------------
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
    QPointer<QMenu> mc_systrayMenu_asset;
    // ---------------------------------------------------------
    QList<QVariant> * asset_list_id;
    QList<QVariant> * asset_list_name;
    // ---------------------------------------------------------
    QString default_asset_id;
    QString default_asset_name;
    // ---------------------------------------------------------        
    QPointer<QMenu> mc_systrayMenu_account;
    // ---------------------------------------------------------
    QList<QVariant> * account_list_id;
    QList<QVariant> * account_list_name;
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
    // ---------------------------------------------------------
    QPointer<QAction> mc_systrayMenu_markets;
    // ---------------------------------------------------------
    //Company submenu
    QPointer<QMenu>   mc_systrayMenu_company_create;
    QPointer<QAction> mc_systrayMenu_company_create_insurance;
    // ---------------------------------------------------------
    //Advanced submenu
    QPointer<QMenu> mc_systrayMenu_advanced;

    QPointer<QAction> mc_systrayMenu_advanced_agreements;
    QPointer<QAction> mc_systrayMenu_advanced_import;
    QPointer<QAction> mc_systrayMenu_advanced_settings;
    QPointer<QAction> mc_systrayMenu_advanced_corporations;
    QPointer<QMenu>   mc_systrayMenu_advanced_bazaar;
    // ---------------------------------------------------------
    // Bazaar
    QPointer<QAction> mc_systrayMenu_bazaar_search;
    QPointer<QAction> mc_systrayMenu_bazaar_post;
    QPointer<QAction> mc_systrayMenu_bazaar_orders;
    // ---------------------------------------------------------
    QPointer<QAction> mc_systrayMenu_bottomblank;
    // ---------------------------------------------------------
    
    
private slots:

    /**
     * Namecoin update timer event.
     */
    void nmc_timer_event();

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
