/*
 *  MoneyChanger
 *  moneychanger.cpp
 *
 *  File organized as follows:
 *  
 *      /-- Constructor (Lengthy)--/
 *      /-- Destructor --/
 *
 *      /-- Systray Functions --/
 *
 *      /-- Menu Dialog Functions --/
 *
 */

#include "moneychanger.h"
#include "ot_worker.h"

#include "Handlers/contacthandler.h"
#include "Handlers/DBHandler.h"

#include "Widgets/detailedit.h"

#include "Widgets/senddlg.h"
#include "Widgets/requestdlg.h"

#include "opentxs/OTAPI.h"
#include "opentxs/OT_ME.h"

/**
 * Constructor & Destructor
 **/

Moneychanger::Moneychanger(QWidget *parent)
: QWidget(parent),
  ot_me(NULL),
  ot_worker_background(NULL),
  mc_overview_already_init(false),
  mc_market_window_already_init(false),
  mc_addressbook_already_init(false),
  mc_nymmanager_already_init(false),
  mc_assetmanager_already_init(false),
  mc_accountmanager_already_init(false),
  mc_servermanager_already_init(false),
  mc_withdraw_ascash_already_init(false),
  mc_withdraw_asvoucher_already_init(false),
  mc_deposit_already_init(false),
  mc_sendfunds_already_init(false),
  mc_requestfunds_already_init(false),
  mc_createinsurancecompany_already_init(false),
  overviewwindow(NULL),
  homewindow(NULL),
  contactswindow(NULL),
  nymswindow(NULL),
  serverswindow(NULL),
  assetswindow(NULL),
  accountswindow(NULL),
  withdrawascashwindow(NULL),
  withdrawasvoucherwindow(NULL),
  depositwindow(NULL),
  requestfundswindow(NULL),
  sendfundswindow(NULL),
  market_window(NULL),
  createinsurancecompany_window(NULL),
  nym_list_id(NULL),
  nym_list_name(NULL),
  mc_systrayMenu_server(NULL),
  server_list_id(NULL),
  server_list_name(NULL),
  mc_systrayIcon(NULL),
  mc_systrayMenu(NULL),
  mc_systrayMenu_headertext(NULL),
  mc_systrayMenu_aboveBlank(NULL),
  mc_systrayMenu_shutdown(NULL),
  mc_systrayMenu_overview(NULL),
  mc_systrayMenu_asset(NULL),
  asset_list_id(NULL),
  asset_list_name(NULL),
  mc_systrayMenu_account(NULL),
  account_list_id(NULL),
  account_list_name(NULL),
  mc_systrayMenu_nym(NULL),
  mc_systrayMenu_goldaccount(NULL),
  mc_systrayMenu_purse(NULL),
  mc_systrayMenu_withdraw(NULL),
  mc_systrayMenu_withdraw_ascash(NULL),
  mc_systrayMenu_withdraw_asvoucher(NULL),
  mc_systrayMenu_deposit(NULL),
  mc_systrayMenu_sendfunds(NULL),
  mc_systrayMenu_requestfunds(NULL),
  mc_systrayMenu_company_create(NULL),
  mc_systrayMenu_company_create_insurance(NULL),
  mc_systrayMenu_advanced(NULL),
  mc_systrayMenu_advanced_agreements(NULL),
  mc_systrayMenu_advanced_markets(NULL),
  mc_systrayMenu_advanced_settings(NULL),
  mc_systrayMenu_advanced_corporations(NULL),
  mc_systrayMenu_advanced_bazaar(NULL),
  mc_systrayMenu_bazaar_search(NULL),
  mc_systrayMenu_bazaar_post(NULL),
  mc_systrayMenu_bazaar_orders(NULL),
  mc_systrayMenu_bottomblank(NULL)
{
    /**
     ** Init variables *
     **/
    
    //Thread Related
    ot_worker_background = new ot_worker();
    ot_worker_background->mc_overview_ping();
    
    //OT Related
    ot_me = new OT_ME();
    
    //SQLite database
    // This can be moved very easily into a different class
    // Which I will inevitably end up doing.
    
    /** Default Nym **/
    qDebug() << "Setting up Nym table";
    if (DBHandler::getInstance()->querySize("SELECT `nym` FROM `default_nym` WHERE `default_id`='1' LIMIT 0,1") == 0)
    {
        qDebug() << "Default Nym wasn't set in the database. Inserting blank record...";
        DBHandler::getInstance()->runQuery("INSERT INTO `default_nym` (`default_id`,`nym`) VALUES('1','')"); // Blank Row
    }
    else
    {
        if (DBHandler::getInstance()->isNext("SELECT `nym` FROM `default_nym` WHERE `default_id`='1' LIMIT 0,1"))
        {
            default_nym_id = DBHandler::getInstance()->queryString("SELECT `nym` FROM `default_nym` WHERE `default_id`='1' LIMIT 0,1", 0, 0);
        }
        // -------------------------------------------------
        //Ask OT what the display name of this nym is and store it for quick retrieval later on(mostly for "Default Nym" displaying purposes)
        if (!default_nym_id.isEmpty())
        {
            default_nym_name =  QString::fromStdString(OTAPI_Wrap::GetNym_Name(default_nym_id.toStdString()));
        }
        else
            qDebug() << "Error loading DEFAULT NYM from SQL";
    }
    
    /** Default Server **/
    //Query for the default server (So we know for setting later on -- Auto select server associations on later dialogs)
    if (DBHandler::getInstance()->querySize("SELECT `server` FROM `default_server` WHERE `default_id`='1' LIMIT 0,1") == 0)
    {
        qDebug() << "Default Server wasn't set in the database. Inserting blank record...";
        DBHandler::getInstance()->runQuery("INSERT INTO `default_server` (`default_id`, `server`) VALUES('1','')"); // Blank Row
    }
    else
    {
        if (DBHandler::getInstance()->runQuery("SELECT `server` FROM `default_server` WHERE `default_id`='1' LIMIT 0,1"))
        {
            default_server_id = DBHandler::getInstance()->queryString("SELECT `server` FROM `default_server` WHERE `default_id`='1' LIMIT 0,1", 0, 0);
        }
        // ---------------------------------------------
        //Ask OT what the display name of this server is and store it for a quick retrieval later on(mostly for "Default Server" displaying purposes)
        if (!default_server_id.isEmpty())
        {
            default_server_name = QString::fromStdString(OTAPI_Wrap::GetServer_Name(default_server_id.toStdString()));
        }
        else
            qDebug() << "Error loading DEFAULT SERVER from SQL";
    }


    /** Default Asset Type **/
    //Query for the default asset (So we know for setting later on -- Auto select asset associations on later dialogs)
    if (DBHandler::getInstance()->querySize("SELECT `asset` FROM `default_asset` WHERE `default_id`='1' LIMIT 0,1") == 0)
    {
        qDebug() << "Default Asset Type wasn't set in the database. Inserting blank record...";
        DBHandler::getInstance()->runQuery("INSERT INTO `default_asset` (`default_id`,`asset`) VALUES('1','')"); // Blank Row
    }
    else
    {
        if (DBHandler::getInstance()->runQuery("SELECT `asset` FROM `default_asset` WHERE `default_id`='1' LIMIT 0,1"))
        {
            default_asset_id = DBHandler::getInstance()->queryString("SELECT `asset` FROM `default_asset` WHERE `default_id`='1' LIMIT 0,1", 0, 0);
        }
        // ------------------------------
        //Ask OT what the display name of this asset type is and store it for a quick retrieval later on(mostly for "Default Asset" displaying purposes)
        if (!default_asset_id.isEmpty())
        {
            default_asset_name = QString::fromStdString(OTAPI_Wrap::GetAssetType_Name(default_asset_id.toStdString()));
        }
        else
            qDebug() << "Error loading DEFAULT ASSET from SQL";
    }

    /** Default Account **/
    //Query for the default account (So we know for setting later on -- Auto select account associations on later dialogs)
    if (DBHandler::getInstance()->querySize("SELECT `account` FROM `default_account` WHERE `default_id`='1' LIMIT 0,1") == 0)
    {
        qDebug() << "Default Account wasn't set in the database. Inserting blank record...";

        DBHandler::getInstance()->runQuery("INSERT INTO `default_account` (`default_id`,`account`) VALUES('1','')"); // Blank Row
    }
    else
    {
        if (DBHandler::getInstance()->runQuery("SELECT `account` FROM `default_account` WHERE `default_id`='1' LIMIT 0,1"))
        {
            default_account_id = DBHandler::getInstance()->queryString("SELECT `account` FROM `default_account` WHERE `default_id`='1' LIMIT 0,1", 0, 0);
        }
        // --------------------------------------------------
        //Ask OT what the display name of this account is and store it for a quick retrieval later on(mostly for "Default Account" displaying purposes)
        if (!default_account_id.isEmpty())
        {
            default_account_name = QString::fromStdString(OTAPI_Wrap::GetAccountWallet_Name(default_account_id.toStdString()));
        }
        else
            qDebug() << "Error loading DEFAULT ACCOUNT from SQL";
    }



    qDebug() << "Database Populated";
    
    
    //Ask OT for "cash account" information (might be just "Account" balance)
    //Ask OT the purse information
    
    
    /* *** *** ***
     * Init Memory Trackers (there may be other int below than just memory trackers but generally there will be mostly memory trackers below)
     * Allows the program to boot with a low footprint -- keeps start times low no matter the program complexity;
     * Memory will expand as the operator opens dialogs;
     * Also prevents HTTP requests from overloading or spamming the operators device by only allowing one window of that request;
     * *** *** ***/
    
    //Init MC System Tray Icon
    mc_systrayIcon = new QSystemTrayIcon(this);
    mc_systrayIcon->setIcon(QIcon(":/icons/moneychanger"));
    
    //Init Icon resources (Loading resources/access Harddrive first; then send to GPU; This specific order will in theory prevent bottle necking between HDD/GPU)
    mc_systrayIcon_shutdown = QIcon(":/icons/quit");
    
    mc_systrayIcon_overview = QIcon(":/icons/overview");
    
    mc_systrayIcon_nym = QIcon(":/icons/nym");
    mc_systrayIcon_server = QIcon(":/icons/server");
    
    mc_systrayIcon_goldaccount = QIcon(":/icons/account");
    mc_systrayIcon_purse = QIcon(":/icons/purse");
    
    mc_systrayIcon_withdraw = QIcon(":/icons/withdraw");
    mc_systrayIcon_deposit = QIcon(":/icons/deposit");
    
    mc_systrayIcon_sendfunds = QIcon(":/icons/sendfunds");
    mc_systrayIcon_requestfunds = QIcon(":/icons/requestpayment");
    
    mc_systrayIcon_advanced = QIcon(":/icons/advanced");
    //Submenu
    mc_systrayIcon_advanced_agreements = QIcon(":/icons/agreements");
    mc_systrayIcon_advanced_markets = QIcon(":/icons/markets");
    mc_systrayIcon_advanced_settings = QIcon(":/icons/settings");
    
    //MC System tray menu
    mc_systrayMenu = new QMenu(this);
    
    //Init Skeleton of system tray menu
    //App name
    mc_systrayMenu_headertext = new QAction(tr("Moneychanger"), 0);
    mc_systrayMenu_headertext->setDisabled(1);
    mc_systrayMenu->addAction(mc_systrayMenu_headertext);
    // --------------------------------------------------------------
    //Blank
    //            mc_systrayMenu_aboveBlank = new QAction(" ", 0);
    //            mc_systrayMenu_aboveBlank->setDisabled(1);
    //            mc_systrayMenu->addAction(mc_systrayMenu_aboveBlank);
    // --------------------------------------------------------------
    //Separator
    mc_systrayMenu->addSeparator();
    // --------------------------------------------------------------
    //Overview button
    mc_systrayMenu_overview = new QAction(mc_systrayIcon_overview, tr("Transaction History"), 0);
    mc_systrayMenu->addAction(mc_systrayMenu_overview);
    //Connect the Overview to a re-action when "clicked";
    connect(mc_systrayMenu_overview, SIGNAL(triggered()), this, SLOT(mc_overview_slot()));
    // --------------------------------------------------------------
    //Separator
    mc_systrayMenu->addSeparator();
    // --------------------------------------------------------------
    //Send funds
    mc_systrayMenu_sendfunds = new QAction(mc_systrayIcon_sendfunds, tr("Send Funds..."), 0);
    mc_systrayMenu->addAction(mc_systrayMenu_sendfunds);
    //Connect button with re-aciton
    connect(mc_systrayMenu_sendfunds, SIGNAL(triggered()), this, SLOT(mc_sendfunds_slot()));
    // --------------------------------------------------------------
    //Request payment
    mc_systrayMenu_requestfunds = new QAction(mc_systrayIcon_requestfunds, tr("Request Payment..."), 0);
    mc_systrayMenu->addAction(mc_systrayMenu_requestfunds);
    // Currently causes a crash , likely due to malformed Dialog construction.
    connect(mc_systrayMenu_requestfunds, SIGNAL(triggered()), this, SLOT(mc_requestfunds_slot()));
    // --------------------------------------------------------------
    //Separator
    mc_systrayMenu->addSeparator();
    // --------------------------------------------------------------
    //Account section
    mc_systrayMenu_account = new QMenu(tr("Set Default Account..."), 0);
    mc_systrayMenu_account->setIcon(mc_systrayIcon_goldaccount);
    mc_systrayMenu->addMenu(mc_systrayMenu_account);
    
    //Add a "Manage accounts" action button (and connection)
    QAction * manage_accounts = new QAction(tr("Manage Accounts..."), 0);
    manage_accounts->setData(QVariant(QString("openmanager")));
    mc_systrayMenu_account->addAction(manage_accounts);
    mc_systrayMenu_account->addSeparator();
    
    //Add reaction to the "account" action.
    connect(mc_systrayMenu_account, SIGNAL(triggered(QAction*)), this, SLOT(mc_accountselection_triggered(QAction*)));
    
    //Load "default" account
    set_systrayMenu_account_setDefaultAccount(default_account_id, default_account_name);
    
    //Init account submenu
    account_list_id   = new QList<QVariant>();
    account_list_name = new QList<QVariant>();
    
    mc_systrayMenu_reload_accountlist();
    // --------------------------------------------------------------
    //Asset section
    mc_systrayMenu_asset = new QMenu(tr("Set Default Asset Type..."), 0);
    mc_systrayMenu_asset->setIcon(mc_systrayIcon_purse);
    mc_systrayMenu->addMenu(mc_systrayMenu_asset);
    
    //Add a "Manage asset types" action button (and connection)
    QAction * manage_assets = new QAction(tr("Manage Asset Contracts..."), 0);
    manage_assets->setData(QVariant(QString("openmanager")));
    mc_systrayMenu_asset->addAction(manage_assets);
    mc_systrayMenu_asset->addSeparator();
    
    //Add reaction to the "asset" action.
    connect(mc_systrayMenu_asset, SIGNAL(triggered(QAction*)), this, SLOT(mc_assetselection_triggered(QAction*)));
    
    //Load "default" asset type
    set_systrayMenu_asset_setDefaultAsset(default_asset_id, default_asset_name);
    
    //Init asset submenu
    asset_list_id   = new QList<QVariant>();
    asset_list_name = new QList<QVariant>();
    
    mc_systrayMenu_reload_assetlist();
    // --------------------------------------------------------------
    //Separator
    mc_systrayMenu->addSeparator();
    // --------------------------------------------------------------
    //Withdraw
    mc_systrayMenu_withdraw = new QMenu(tr("Withdraw"), 0);
    mc_systrayMenu_withdraw->setIcon(mc_systrayIcon_withdraw);
    mc_systrayMenu->addMenu(mc_systrayMenu_withdraw);
    //(Withdraw) as Cash
    mc_systrayMenu_withdraw_ascash = new QAction(tr("As Cash"),0);
    mc_systrayMenu_withdraw->addAction(mc_systrayMenu_withdraw_ascash);
    //Connect Button with re-action
    connect(mc_systrayMenu_withdraw_ascash, SIGNAL(triggered()), this, SLOT(mc_withdraw_ascash_slot()));
    
    //(Withdraw) as Voucher
    mc_systrayMenu_withdraw_asvoucher = new QAction(tr("As Voucher"), 0);
    mc_systrayMenu_withdraw->addAction(mc_systrayMenu_withdraw_asvoucher);
    //Connect Button with re-action
    connect(mc_systrayMenu_withdraw_asvoucher, SIGNAL(triggered()), this, SLOT(mc_withdraw_asvoucher_slot()));
    // --------------------------------------------------------------
    //Deposit
    mc_systrayMenu_deposit = new QAction(mc_systrayIcon_deposit, tr("Deposit..."), 0);
    mc_systrayMenu->addAction(mc_systrayMenu_deposit);
    //Connect button with re-action
    connect(mc_systrayMenu_deposit, SIGNAL(triggered()), this, SLOT(mc_deposit_slot()));
    // --------------------------------------------------------------
    //Gold account/cash purse/wallet
    //            mc_systrayMenu_goldaccount = new QAction("Gold Account: $60,000", 0);
    //            mc_systrayMenu_goldaccount->setIcon(mc_systrayIcon_goldaccount);
    //            mc_systrayMenu->addAction(mc_systrayMenu_goldaccount);
    //            // --------------------------------------------------------------
    //            //purse wallet
    //            mc_systrayMenu_purse = new QAction("Purse: $40,000", 0);
    //            mc_systrayMenu_purse->setIcon(mc_systrayIcon_purse);
    //            mc_systrayMenu->addAction(mc_systrayMenu_purse);
    // --------------------------------------------------------------
    //Separator
    mc_systrayMenu->addSeparator();
    // --------------------------------------------------------------

    //Company
//    mc_systrayMenu_company = new QMenu("Company", 0);
//    mc_systrayMenu->addMenu(mc_systrayMenu_company);
    // --------------------------------------------------------------
    //Advanced
    mc_systrayMenu_advanced = new QMenu("Advanced", 0);
    mc_systrayMenu_advanced->setIcon(mc_systrayIcon_advanced);
    mc_systrayMenu->addMenu(mc_systrayMenu_advanced);
    //Advanced submenu
        
    mc_systrayMenu_advanced_markets = new QAction(mc_systrayIcon_advanced_markets, tr("Markets"), 0);
    mc_systrayMenu_advanced->addAction(mc_systrayMenu_advanced_markets);
    connect(mc_systrayMenu_advanced_markets, SIGNAL(triggered()), this, SLOT(mc_market_slot()));

    mc_systrayMenu_advanced_agreements = new QAction(mc_systrayIcon_advanced_agreements, tr("Agreements"), 0);
    mc_systrayMenu_advanced->addAction(mc_systrayMenu_advanced_agreements);
    // --------------------------------------------------------------
    //Separator
    mc_systrayMenu_advanced->addSeparator();
    // ------------------------------------------------
    // Corporations
    mc_systrayMenu_advanced_corporations = new QMenu(tr("Corporations"), 0);
    mc_systrayMenu_advanced->addMenu(mc_systrayMenu_advanced_corporations);

    // Corporations submenu
    mc_systrayMenu_company_create = new QMenu(tr("Create"), 0);
    mc_systrayMenu_advanced_corporations->addMenu(mc_systrayMenu_company_create);

    // Create insurance company action on submenu
    mc_systrayMenu_company_create_insurance = new QAction(mc_systrayIcon_advanced_agreements, tr("Insurance Company"), 0);
    mc_systrayMenu_company_create->addAction(mc_systrayMenu_company_create_insurance);
    connect(mc_systrayMenu_company_create_insurance, SIGNAL(triggered()), this, SLOT(mc_createinsurancecompany_slot()));
    // --------------------------------------------------------------
    // Bazaar
    mc_systrayMenu_advanced_bazaar = new QMenu(tr("Bazaar"), 0);
    mc_systrayMenu_advanced->addMenu(mc_systrayMenu_advanced_bazaar);

    // Bazaar actions
    mc_systrayMenu_bazaar_search = new QAction(mc_systrayIcon_advanced_agreements, tr("Search Listings"), 0);
    mc_systrayMenu_advanced_bazaar->addAction(mc_systrayMenu_bazaar_search);
//  connect(mc_systrayMenu_bazaar_search, SIGNAL(triggered()), this, SLOT(mc_bazaar_search_slot()));

    mc_systrayMenu_bazaar_post = new QAction(mc_systrayIcon_advanced_agreements, tr("Post an Ad"), 0);
    mc_systrayMenu_advanced_bazaar->addAction(mc_systrayMenu_bazaar_post);
//  connect(mc_systrayMenu_bazaar_post, SIGNAL(triggered()), this, SLOT(mc_bazaar_search_slot()));

    mc_systrayMenu_bazaar_orders = new QAction(mc_systrayIcon_advanced_agreements, tr("Orders"), 0);
    mc_systrayMenu_advanced_bazaar->addAction(mc_systrayMenu_bazaar_orders);
//  connect(mc_systrayMenu_bazaar_orders, SIGNAL(triggered()), this, SLOT(mc_bazaar_search_slot()));

    // -------------------------------------------------
    mc_systrayMenu_advanced->addSeparator();
    // -------------------------------------------------
    // Settings
    
    mc_systrayMenu_advanced_settings = new QAction(mc_systrayIcon_advanced_settings, tr("Settings..."), 0);
    mc_systrayMenu_advanced_settings->setMenuRole(QAction::NoRole);
    mc_systrayMenu_advanced->addAction(mc_systrayMenu_advanced_settings);
    // --------------------------------------------------------------
    //Separator
    mc_systrayMenu_advanced->addSeparator();
    // --------------------------------------------------------------
    
    // TODO: If the default isn't set, then choose the first one and select it.
    
    // TODO: If there isn't even ONE to select, then this menu item should say "Create Nym..." with no sub-menu.
    
    // TODO: When booting up, if there is already a default server and asset id, but no nyms exist, create a default nym.
    
    // TODO: When booting up, if there is already a default nym, but no accounts exist, create a default account.
    
    //Nym section
    mc_systrayMenu_nym = new QMenu("Set Default Nym...", 0);
    mc_systrayMenu_nym->setIcon(mc_systrayIcon_nym);
    mc_systrayMenu_advanced->addMenu(mc_systrayMenu_nym);
    
    //Add a "Manage pseudonym" action button (and connection)
    QAction * manage_nyms = new QAction(tr("Manage Nyms..."), 0);
    manage_nyms->setData(QVariant(QString("openmanager")));
    mc_systrayMenu_nym->addAction(manage_nyms);
    mc_systrayMenu_nym->addSeparator();
    
    //Add reaction to the "pseudonym" action.
    connect(mc_systrayMenu_nym, SIGNAL(triggered(QAction*)), this, SLOT(mc_nymselection_triggered(QAction*)));
    
    //Load "default" nym
    set_systrayMenu_nym_setDefaultNym(default_nym_id, default_nym_name);
    
    //Init nym submenu
    nym_list_id = new QList<QVariant>();
    nym_list_name = new QList<QVariant>();
    
    mc_systrayMenu_reload_nymlist();
    // --------------------------------------------------------------
    //Server section
    mc_systrayMenu_server = new QMenu(tr("Set Default Server..."), 0);
    mc_systrayMenu_server->setIcon(mc_systrayIcon_server);
    mc_systrayMenu_advanced->addMenu(mc_systrayMenu_server);
    
    //Add a "Manage server" action button (and connection)
    QAction * manage_servers = new QAction(tr("Manage Servers..."), 0);
    manage_servers->setData(QVariant(QString("openmanager")));
    mc_systrayMenu_server->addAction(manage_servers);
    mc_systrayMenu_server->addSeparator();
    
    //Add reaction to the "server selection" action
    connect(mc_systrayMenu_server, SIGNAL(triggered(QAction*)), this, SLOT(mc_serverselection_triggered(QAction*)));
    
    //Load "default" server
    set_systrayMenu_server_setDefaultServer(default_server_id, default_server_name);
    
    //Init server submenu
    server_list_id = new QList<QVariant>();
    server_list_name = new QList<QVariant>();
    
    mc_systrayMenu_reload_serverlist();
    // --------------------------------------------------------------
    //Separator
    mc_systrayMenu->addSeparator();
    // --------------------------------------------------------------
    //Shutdown Moneychanger
    mc_systrayMenu_shutdown = new QAction(mc_systrayIcon_shutdown, tr("Quit"), 0);
    mc_systrayMenu_shutdown->setMenuRole(QAction::NoRole);
    mc_systrayMenu_shutdown->setIcon(mc_systrayIcon_shutdown);
    mc_systrayMenu->addAction(mc_systrayMenu_shutdown);
    //connection
    connect(mc_systrayMenu_shutdown, SIGNAL(triggered()), this, SLOT(mc_shutdown_slot()));
    // --------------------------------------------------------------
    //Blank
    //            mc_systrayMenu_bottomblank = new QAction(" ", 0);
    //            mc_systrayMenu_bottomblank->setDisabled(1);
    //            mc_systrayMenu->addAction(mc_systrayMenu_bottomblank);
    // --------------------------------------------------------------
    //Set Skeleton to systrayIcon object code
    mc_systrayIcon->setContextMenu(mc_systrayMenu);
}


Moneychanger::~Moneychanger()
{
    
}





/** 
 * Systray 
 **/

// Startup
void Moneychanger::bootTray()
{
    //Show systray
    mc_systrayIcon->show();
    
//    qDebug() << "BOOTING";
}


// Shutdown
void Moneychanger::mc_shutdown_slot(){
    //Disconnect all signals from callin class (probubly main) to this class
    //Disconnect
    QObject::disconnect(this);
    //Close qt app (no need to deinit anything as of the time of this comment)
    //TO DO: Check if the OT queue caller is still proccessing calls.... Then quit the app. (Also tell user that the OT is still calling other wise they might think it froze during OT calls)
    qApp->quit();
}

// End Systray







/****
 *
 * Menu Dialog Related Calls
 *
 * First you'll want to create a class for the dialog window you want to display.
 * Then you'll want to follow one of the dialog examples below to see how the startup should happen.
 *
 ****
 *
 * Remember that you'll need to define a close public function here in Moneychanger, and the accompanying
 * QEvent Handler for the class in question.
 *
 ****
 * 
 * At a minimum, a Window needs three Functions to operate in the Systray:
 * 
 *  1) Startup Slot to connect to Moneychanger Buttons (mc_classname_slot)
 *  2) Dialog Function to launch the Window (mc_classname_dialog)
 *  3) A Function to handled the Close Event Handlers (close_classname_dialog)
 *
 * If you're confused, take a look at the window functions defined below :-)
 * (NOTE) : Some of these may not be fully complete yet, this is a work in progress.
 *
 * Try to keep everything you add for your windows organized and grouped below by Window Class.
 *
 ****/



/** 
 * Overview Window 
 **/

//Overview slots
void Moneychanger::mc_overview_slot()
{
    //The operator has requested to open the dialog to the "Overview";
    mc_overview_dialog();
}

void Moneychanger::mc_overview_dialog()
{
    if(!mc_overview_already_init)
    {
        homewindow = new MTHome(this);
        mc_overview_already_init = true;
        qDebug() << "Overview Opened";
    }
    // ---------------------------------
    homewindow->dialog();
}

void Moneychanger::close_overview_dialog()
{
    delete homewindow;
    homewindow = NULL;
    mc_overview_already_init = false;
    qDebug() << "Overview Closed";
}

// End Overview







/** 
 * Address Book 
 **/

void Moneychanger::mc_addressbook_show(QString text) // text may contain a "pre-selected" Contact ID (an integer in string form.)
{
    // The caller dosen't wish to have the address book paste to anything
    // (they just want to see/manage the address book), just call blank.
    //
    if (!mc_addressbook_already_init)
    {
        contactswindow = new MTDetailEdit(this);

        mc_addressbook_already_init = true;
        qDebug() << "Address Book Opened";
    }
    // -------------------------------------
    contactswindow->m_map.clear();
    // -------------------------------------
    MTContactHandler::getInstance()->GetContacts(contactswindow->m_map);
    // -------------------------------------
    contactswindow->SetPreSelected(text);
    contactswindow->setWindowTitle(tr("Contacts"));
    // -------------------------------------
    contactswindow->dialog(MTDetailEdit::DetailEditTypeContact);
}

void Moneychanger::close_addressbook()
{
    delete contactswindow;
    contactswindow = NULL;
    mc_addressbook_already_init = false;
    qDebug() << "Address Book Closed";
}


// End Address Book






/**  
 * Nym Manager 
 **/

//Nym manager "clicked"
void Moneychanger::mc_defaultnym_slot()
{
    //The operator has requested to open the dialog to the "Nym Manager";
    mc_nymmanager_dialog();
}

void Moneychanger::mc_nymmanager_dialog()
{
    // -------------------------------------
    if (!mc_nymmanager_already_init)
    {
        nymswindow = new MTDetailEdit(this);
        mc_nymmanager_already_init = true;
        qDebug() << "Nym Manager Opened";
    }
    // -------------------------------------
    mapIDName & the_map = nymswindow->m_map;
    // -------------------------------------
    the_map.clear();
    // -------------------------------------
    int32_t nym_count = OTAPI_Wrap::GetNymCount();

    for (int32_t ii = 0; ii < nym_count; ii++)
    {
        QString OT_id   = QString::fromStdString(OTAPI_Wrap::GetNym_ID(ii));
        QString OT_name = QString::fromStdString(OTAPI_Wrap::GetNym_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);
    } // for
    // -------------------------------------
    nymswindow->setWindowTitle(tr("Nyms"));
    // -------------------------------------
    nymswindow->dialog(MTDetailEdit::DetailEditTypeNym);
}

void Moneychanger::close_nymmanager_dialog()
{
    // --------------------------------
    if (NULL != nymswindow)
        delete nymswindow;
    nymswindow = NULL;
    // --------------------------------
    mc_nymmanager_already_init = false;
    // --------------------------------
    qDebug() << "Nym Manager Closed";
}

//Additional Nym Manager Functions
void Moneychanger::set_systrayMenu_nym_setDefaultNym(QString nym_id, QString nym_name)
{
    //Set default nym internal memory
    default_nym_id   = nym_id;
    default_nym_name = nym_name;
    
    //SQL UPDATE default nym
    DBHandler::getInstance()->AddressBookUpdateDefaultNym(nym_id);
    
    //Rename "NYM:" if a nym is loaded
    if (nym_id != "")
    {
        mc_systrayMenu_nym->setTitle(tr("Nym: ")+nym_name);
    }
}

void Moneychanger::mc_systrayMenu_reload_nymlist()
{
//  qDebug() << "RELOAD NYM LIST";

    //Count nyms
    int32_t nym_count = OTAPI_Wrap::GetNymCount();
    
    //Retrieve updated list of nym submenu actions list
    QList<QAction*> action_list_to_nym_submenu = mc_systrayMenu_nym->actions();
    
    //Remove all sub-menus from the nym submenu
    for (int a = action_list_to_nym_submenu.size(); a > 0; a--)
    {
//      qDebug() << "REMOVING" << a;
        QPoint tmp_point = QPoint(a, 0);
        mc_systrayMenu_nym->removeAction(mc_systrayMenu_nym->actionAt(tmp_point));
    }
    
    //Remove all nyms from the backend list
    //Remove ids from backend list
    int tmp_nym_list_id_size = nym_list_id->size();
    for (int a = 0; a < tmp_nym_list_id_size; a++)
    {
        nym_list_id->removeLast();
    }
    
    //Remove names from the backend list
    int tmp_nym_list_name_size = nym_list_name->size();
    for (int a = 0; a < tmp_nym_list_name_size; a++)
    {
        nym_list_name->removeLast();
    }
    
    //Add/append to the id + name lists
    for (int32_t a = 0; a < nym_count; a++)
    {
        //Get OT Account ID
        QString OT_nym_id = QString::fromStdString(OTAPI_Wrap::GetNym_ID(a));
        
        //Add to qlist
        nym_list_id->append(QVariant(OT_nym_id));
        
        //Get OT Account Name
        QString OT_nym_name = QString::fromStdString(OTAPI_Wrap::GetNym_Name(OT_nym_id.toStdString()));
        
        //Add to qlist
        nym_list_name->append(QVariant(OT_nym_name));
        
        //Append to submenu of nym
        QAction * next_nym_action = new QAction(mc_systrayIcon_nym, OT_nym_name, 0);
        next_nym_action->setData(QVariant(OT_nym_id));
        mc_systrayMenu_nym->addAction(next_nym_action);
    } // for
}


//Nym new default selected from systray
void Moneychanger::mc_nymselection_triggered(QAction*action_triggered)
{
    //Check if the user wants to open the nym manager (or) select a different default nym
    QString action_triggered_string = QVariant(action_triggered->data()).toString();
    qDebug() << "NYM TRIGGERED" << action_triggered_string;

    if(action_triggered_string == "openmanager")
    {
        //Open nym manager
        mc_defaultnym_slot();
    }
    else
    {
        //Set new nym default
        QString action_triggered_string_nym_name = QVariant(action_triggered->text()).toString();
        set_systrayMenu_nym_setDefaultNym(action_triggered_string, action_triggered_string_nym_name);
        
        //Refresh the nym default selection in the nym manager (ONLY if it is open)
        //Check if nym manager has ever been opened (then apply logic) [prevents crash if the dialog hasen't be opend before]
        //
        if (mc_nymmanager_already_init)
        {
            mc_nymmanager_dialog();
        }
    }
}

// End Nym Manager






//QString get_default_server_id(){return default_server_id;}

//int get_server_list_id_size(){return server_list_id->size();}

//QString get_server_id_at(int a){return server_list_id->at(a).toString();}

void Moneychanger::downloadAccountData()
{
    //Also refreshes/initializes client data

    OT_ME madeEasy;

    if ((get_server_list_id_size() > 0) && (get_asset_list_id_size() > 0) )
    {
        std::string defaultServerId(get_default_server_id().toStdString());
        // ----------------------------------------------------------------
        if (defaultServerId.empty())
        {
            defaultServerId = get_server_id_at(0).toStdString();
            DBHandler::getInstance()->AddressBookUpdateDefaultServer(QString::fromStdString(defaultServerId));
        }
        // ----------------------------------------------------------------
        int nymCount = OTAPI_Wrap::GetNymCount();

        if (0 == nymCount)
        {
            qDebug() << "Making 'Me' Nym";

            std::string strSource(""), strAlt("");

            std::string newNymId = madeEasy.create_pseudonym(1024, strSource, strAlt);

            if (!newNymId.empty())
            {
                OTAPI_Wrap::SetNym_Name(newNymId, newNymId, tr("Me").toLatin1().data());
                DBHandler::getInstance()->AddressBookUpdateDefaultNym(QString::fromStdString(newNymId));
                qDebug() << "Finished Making Nym";
            }
        }
        // ----------------------------------------------------------------
        std::string defaultNymID(get_default_nym_id().toStdString());
        // ----------------------------------------------------------------
        if (!defaultNymID.empty() && !defaultServerId.empty())
        {
            bool isReg = OTAPI_Wrap::IsNym_RegisteredAtServer(defaultNymID, defaultServerId);

            if (!isReg)
            {
                std::string response = madeEasy.register_nym(defaultServerId, defaultNymID);
                qDebug() << QString("Creation Response: %1").arg(QString::fromStdString(response));
            }
            madeEasy.retrieve_nym(defaultServerId, defaultNymID, true);
        }
        // ----------------------------------------------------------------
        std::string defaultAssetId (get_default_asset_id().toStdString());
        // ----------------------------------------------------------------
        if (defaultAssetId.empty())
        {
            defaultAssetId = get_asset_id_at(0).toStdString();
            DBHandler::getInstance()->AddressBookUpdateDefaultAsset(QString::fromStdString(defaultAssetId));
        }
        // ----------------------------------------------------------------
        int accountCount = OTAPI_Wrap::GetAccountCount();

        qDebug() << QString("Account Count: %1").arg(accountCount);

        if (0 == accountCount)
        {
            if (!defaultNymID.empty() && !defaultServerId.empty() && !defaultAssetId.empty())
            {
                std::string response = madeEasy.create_asset_acct(defaultServerId, defaultNymID, defaultAssetId);
                qDebug() << QString("Creation Response: %1").arg(QString::fromStdString(response));

                accountCount = OTAPI_Wrap::GetAccountCount();

                if (accountCount > 0)
                {
                    std::string accountID = OTAPI_Wrap::GetAccountWallet_ID(0);
                    OTAPI_Wrap::SetAccountWallet_Name(accountID, defaultNymID, tr("My Acct").toLatin1().data());

                    DBHandler::getInstance()->AddressBookUpdateDefaultAccount(QString::fromStdString(accountID));
                }
            }
        }
        // ----------------------------------------------------------------
        for (int i = 0; i < accountCount; i++)
        {
            std::string accountId = OTAPI_Wrap::GetAccountWallet_ID(i);
            std::string acctNymID = OTAPI_Wrap::GetAccountWallet_NymID(accountId);
            std::string acctSvrID = OTAPI_Wrap::GetAccountWallet_ServerID(accountId);

            madeEasy.retrieve_account(acctSvrID, acctNymID, accountId, true);

            std::string statAccount = madeEasy.stat_asset_account(accountId);
            qDebug() << QString("statAccount: %1").arg(QString::fromStdString(statAccount));
        }
        // ----------------------------------------------------------------
    }
    else
    {
        qDebug() << QString("%1: Not at least 1 server contract and 1 asset contract registered, doing nothing.").arg(__FUNCTION__);
    }
}








/** 
 * Asset Manager 
 **/

//Asset manager "clicked"
void Moneychanger::mc_defaultasset_slot()
{
    //The operator has requested to open the dialog to the "Asset Manager";
    mc_assetmanager_dialog();
}


void Moneychanger::mc_assetmanager_dialog()
{
    // -------------------------------------
    if (!mc_assetmanager_already_init)
    {
        assetswindow = new MTDetailEdit(this);
        mc_assetmanager_already_init = true;
        qDebug() << "Asset Manager Opened";
    }
    // -------------------------------------
    mapIDName & the_map = assetswindow->m_map;
    // -------------------------------------
    the_map.clear();
    // -------------------------------------
    int32_t asset_count = OTAPI_Wrap::GetAssetTypeCount();

    for (int32_t ii = 0; ii < asset_count; ii++)
    {
        QString OT_id   = QString::fromStdString(OTAPI_Wrap::GetAssetType_ID(ii));
        QString OT_name = QString::fromStdString(OTAPI_Wrap::GetAssetType_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);
    } // for
    // -------------------------------------
    assetswindow->setWindowTitle(tr("Asset Types"));
    // -------------------------------------
    assetswindow->dialog(MTDetailEdit::DetailEditTypeAsset);
}

void Moneychanger::close_assetmanager_dialog()
{
    // --------------------------------
    if (NULL != assetswindow)
        delete assetswindow;
    assetswindow = NULL;
    // --------------------------------
    mc_assetmanager_already_init = false;
    // --------------------------------
    qDebug() << "Asset Manager Closed";
}








//Additional Asset slots

//This was mistakenly named asset_load_asset, should be set default asset
//Set Default asset
void Moneychanger::set_systrayMenu_asset_setDefaultAsset(QString asset_id, QString asset_name)
{
    //Set default asset internal memory
    default_asset_id = asset_id;
    default_asset_name = asset_name;
    
    //SQL UPDATE default asset
    DBHandler::getInstance()->AddressBookUpdateDefaultAsset(asset_id);
    
    //Rename "ASSET:" if a asset is loaded
    if(asset_id != ""){
        mc_systrayMenu_asset->setTitle(tr("Asset Type: ")+asset_name);
    }
}

void Moneychanger::mc_systrayMenu_reload_assetlist()
{
//    qDebug() << "RELOAD asset LIST";
    //Count assets
    int32_t asset_count_int32_t = OTAPI_Wrap::GetAssetTypeCount();
    int asset_count = asset_count_int32_t;
    
    //Retrieve updated list of asset submenu actions list
    QList<QAction*> action_list_to_asset_submenu = mc_systrayMenu_asset->actions();
    
    //Remove all sub-menus from the asset submenu
    for(int a = action_list_to_asset_submenu.size(); a > 0; a--)
    {
//        qDebug() << "REMOVING" << a;
        QPoint tmp_point = QPoint(a, 0);
        mc_systrayMenu_asset->removeAction(mc_systrayMenu_asset->actionAt(tmp_point));
    }
    
    //Remove all assets from the backend list
    //Remove ids from backend list
    int tmp_asset_list_id_size = asset_list_id->size();
    for(int a = 0; a < tmp_asset_list_id_size; a++){
        asset_list_id->removeLast();
    }
    
    //Remove names from the backend list
    int tmp_asset_list_name_size = asset_list_name->size();
    for(int a = 0; a < tmp_asset_list_name_size; a++){
        asset_list_name->removeLast();
    }
    
    //Add/append to the id + name lists
    for(int a = 0; a < asset_count; a++){
        //Get OT Account ID
        QString OT_asset_id = QString::fromStdString(OTAPI_Wrap::GetAssetType_ID(a));
        
        //Add to qlist
        asset_list_id->append(QVariant(OT_asset_id));
        
        //Get OT Account Name
        QString OT_asset_name = QString::fromStdString(OTAPI_Wrap::GetAssetType_Name(OTAPI_Wrap::GetAssetType_ID(a)));
        
        //Add to qlist
        asset_list_name->append(QVariant(OT_asset_name));
        
        //Append to submenu of asset
        QAction * next_asset_action = new QAction(mc_systrayIcon_purse, OT_asset_name, 0);
        next_asset_action->setData(QVariant(OT_asset_id));
        mc_systrayMenu_asset->addAction(next_asset_action);
    }
    
    
}

//Asset new default selected from systray
void Moneychanger::mc_assetselection_triggered(QAction*action_triggered){
    //Check if the user wants to open the asset manager (or) select a different default asset
    QString action_triggered_string = QVariant(action_triggered->data()).toString();
    qDebug() << "asset TRIGGERED" << action_triggered_string;
    if(action_triggered_string == "openmanager"){
        //Open asset manager
        mc_defaultasset_slot();
    }else{
        //Set new asset default
        QString action_triggered_string_asset_name = QVariant(action_triggered->text()).toString();
        set_systrayMenu_asset_setDefaultAsset(action_triggered_string, action_triggered_string_asset_name);
        
        //Refresh the asset default selection in the asset manager (ONLY if it is open)
        //Check if asset manager has ever been opened (then apply logic) [prevents crash if the dialog hasen't be opend before]
        if(mc_assetmanager_already_init == 1){
            //Refresh if the asset manager is currently open
            if(mc_assetmanager_already_init){
                mc_assetmanager_dialog();
            }
        }
    }
    
}

// End Asset Manager







/** 
 * Account Manager 
 **/

//Account manager "clicked"
void Moneychanger::mc_defaultaccount_slot()
{
    //The operator has requested to open the dialog to the "account Manager";
    mc_accountmanager_dialog();
}


void Moneychanger::mc_accountmanager_dialog()
{
    // -------------------------------------
    if (!mc_accountmanager_already_init)
    {
        accountswindow = new MTDetailEdit(this);
        mc_accountmanager_already_init = true;
        qDebug() << "Account Manager Opened";
    }
    // -------------------------------------
    mapIDName & the_map = accountswindow->m_map;
    // -------------------------------------
    the_map.clear();
    // -------------------------------------
    int32_t acct_count = OTAPI_Wrap::GetAccountCount();

    for (int32_t ii = 0; ii < acct_count; ii++)
    {
        QString OT_id   = QString::fromStdString(OTAPI_Wrap::GetAccountWallet_ID(ii));
        QString OT_name = QString::fromStdString(OTAPI_Wrap::GetAccountWallet_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);
    } // for
    // -------------------------------------
    accountswindow->setWindowTitle(tr("Accounts"));
    // -------------------------------------
    accountswindow->dialog(MTDetailEdit::DetailEditTypeAccount);
}

void Moneychanger::close_accountmanager_dialog()
{
    // --------------------------------
    if (NULL != accountswindow)
        delete accountswindow;
    accountswindow = NULL;
    // --------------------------------
    mc_accountmanager_already_init = false;
    // --------------------------------
    qDebug() << "Account Manager Closed";
}


//Account Manager Additional Functions

//Account new default selected from systray
void Moneychanger::mc_accountselection_triggered(QAction*action_triggered)
{
    //Check if the user wants to open the account manager (or) select a different default account
    QString action_triggered_string = QVariant(action_triggered->data()).toString();
    qDebug() << "account TRIGGERED" << action_triggered_string;

    if(action_triggered_string == "openmanager")
    {
        //Open account manager
        mc_defaultaccount_slot();
    }
    else
    {
        //Set new account default
        QString action_triggered_string_account_name = QVariant(action_triggered->text()).toString();
        set_systrayMenu_account_setDefaultAccount(action_triggered_string, action_triggered_string_account_name);
        // ------------------------------
        //Refresh the account default selection in the account manager (ONLY if it is open)
        //Check if account manager has ever been opened (then apply logic) [prevents crash if the dialog hasen't be opend before]
        //
        if (mc_accountmanager_already_init)
        {
            mc_accountmanager_dialog();
        }
        // ------------------------------
        if (mc_overview_already_init)
        {
            mc_overview_dialog();
        }
        // ------------------------------
    }
    
}

//Set Default account
void Moneychanger::set_systrayMenu_account_setDefaultAccount(QString account_id, QString account_name){
    //Set default account internal memory
    default_account_id = account_id;
    default_account_name = account_name;
    
    //SQL UPDATE default account
    DBHandler::getInstance()->AddressBookUpdateDefaultAccount(account_id);
    
    //Rename "ACCOUNT:" if a account is loaded
    if(account_id != ""){
        QString result = tr("Account: ") + account_name;
        
        int64_t     lBalance = OTAPI_Wrap::GetAccountWallet_Balance(account_id.toStdString());
        std::string strAsset = OTAPI_Wrap::GetAccountWallet_AssetTypeID(account_id.toStdString());
        
        std::string str_amount;
        
        if (!strAsset.empty())
        {
            str_amount = OTAPI_Wrap::FormatAmount(strAsset, lBalance);
            result += " ("+ QString::fromStdString(str_amount) +")";
        }
        
        mc_systrayMenu_account->setTitle(result);
    }
}

void Moneychanger::mc_systrayMenu_reload_accountlist()
{
//    qDebug() << "RELOAD account LIST";
    //Count accounts
    int32_t account_count_int32_t = OTAPI_Wrap::GetAccountCount();
    int account_count = account_count_int32_t;
    
    //Retrieve updated list of account submenu actions list
    QList<QAction*> action_list_to_account_submenu = mc_systrayMenu_account->actions();
    
    //Remove all sub-menus from the account submenu
    for(int a = action_list_to_account_submenu.size(); a > 0; a--)
    {
//        qDebug() << "REMOVING" << a;
        QPoint tmp_point = QPoint(a, 0);
        mc_systrayMenu_account->removeAction(mc_systrayMenu_account->actionAt(tmp_point));
    }
    
    //Remove all accounts from the backend list
    //Remove ids from backend list
    int tmp_account_list_id_size = account_list_id->size();
    for(int a = 0; a < tmp_account_list_id_size; a++){
        account_list_id->removeLast();
    }
    
    //Remove names from the backend list
    int tmp_account_list_name_size = account_list_name->size();
    for(int a = 0; a < tmp_account_list_name_size; a++){
        account_list_name->removeLast();
    }
    
    //Add/append to the id + name lists
    for(int a = 0; a < account_count; a++){
        //Get OT Account ID
        QString OT_account_id = QString::fromStdString(OTAPI_Wrap::GetAccountWallet_ID(a));
        
        //Add to qlist
        account_list_id->append(QVariant(OT_account_id));
        
        //Get OT Account Name
        QString OT_account_name = QString::fromStdString(OTAPI_Wrap::GetAccountWallet_Name(OTAPI_Wrap::GetAccountWallet_ID(a)));
        
        //Add to qlist
        account_list_name->append(QVariant(OT_account_name));
        
        //Append to submenu of account
        QAction * next_account_action = new QAction(mc_systrayIcon_goldaccount, OT_account_name, 0);
        next_account_action->setData(QVariant(OT_account_id));
        mc_systrayMenu_account->addAction(next_account_action);
    }
    
    
}

// End Account Manager







/** 
 * Server Manager 
 **/

void Moneychanger::mc_defaultserver_slot()
{
    mc_servermanager_dialog();
}


void Moneychanger::mc_servermanager_dialog()
{
    // -------------------------------------
    if (!mc_servermanager_already_init)
    {
        serverswindow = new MTDetailEdit(this);
        mc_servermanager_already_init = true;
        qDebug() << "Server Manager Opened";
    }
    // -------------------------------------
    mapIDName & the_map = serverswindow->m_map;
    // -------------------------------------
    the_map.clear();
    // -------------------------------------
    int32_t server_count = OTAPI_Wrap::GetServerCount();

    for (int32_t ii = 0; ii < server_count; ii++)
    {
        QString OT_id   = QString::fromStdString(OTAPI_Wrap::GetServer_ID(ii));
        QString OT_name = QString::fromStdString(OTAPI_Wrap::GetServer_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);
    } // for
    // -------------------------------------
    serverswindow->setWindowTitle(tr("Server Contracts"));
    // -------------------------------------
    serverswindow->dialog(MTDetailEdit::DetailEditTypeServer);
}

void Moneychanger::close_servermanager_dialog()
{
    // --------------------------------
    if (NULL != serverswindow)
        delete serverswindow;
    serverswindow = NULL;
    // --------------------------------
    mc_servermanager_already_init = false;
    // --------------------------------
    qDebug() << "Server Manager Closed";
}






void Moneychanger::set_systrayMenu_server_setDefaultServer(QString server_id, QString server_name)
{
    //Set default server internal memory
    default_server_id = server_id;
    default_server_name = server_name;
    
    qDebug() << default_server_id;
    qDebug() << default_server_name;
    
    //SQL UPDATE default server
    DBHandler::getInstance()->AddressBookUpdateDefaultServer(default_server_id);
    
    //Update visuals
    QString new_server_title = default_server_name;
    if(new_server_title == "" || new_server_title == " "){
        new_server_title = tr("Set Default...");
    }
    
    mc_systrayMenu_server->setTitle(tr("Server: ")+new_server_title);
}

void Moneychanger::mc_systrayMenu_reload_serverlist()
{
//    qDebug() << "RELOAD SERVER LIST";
    //Count server
    int32_t server_count_int32_t = OTAPI_Wrap::GetServerCount();
    int server_count = server_count_int32_t;
    
    //Retrieve updated list of server submenu actions list
    QList<QAction*> action_list_to_server_submenu = mc_systrayMenu_server->actions();
    
    //Remove all sub-menus from the server submenu
    for(int a = action_list_to_server_submenu.size(); a > 0; a--){
//        qDebug() << "REMOVING" << a;
        QPoint tmp_point = QPoint(a, 0);
        mc_systrayMenu_server->removeAction(mc_systrayMenu_server->actionAt(tmp_point));
    }
    
    //Remove all servers from the backend list
    //Remove ids from backend list
    int tmp_server_list_id_size = server_list_id->size();
    for(int a = 0; a < tmp_server_list_id_size; a++){
        server_list_id->removeLast();
    }
    
    //Remove names from the backend list
    int tmp_server_list_name_size = server_list_name->size();
    for(int a = 0; a < tmp_server_list_name_size; a++){
        server_list_name->removeLast();
    }
    
    
    //Add/append to the id + name lists
    for(int a = 0; a < server_count; a++){
        //Get OT server ID
        QString OT_server_id = QString::fromStdString(OTAPI_Wrap::GetServer_ID(a));
        
        //Add to qlist
        server_list_id->append(QVariant(OT_server_id));
        
        //Get OT server Name
        QString OT_server_name = QString::fromStdString(OTAPI_Wrap::GetServer_Name(OTAPI_Wrap::GetServer_ID(a)));
        
        //Add to qlist
        server_list_name->append(QVariant(OT_server_name));
        
        //Append to submenu of server
        QAction * next_server_action = new QAction(mc_systrayIcon_server, OT_server_name, 0);
        next_server_action->setData(QVariant(OT_server_id));
        mc_systrayMenu_server->addAction(next_server_action);
    }
}

//Server Slots

void Moneychanger::mc_serverselection_triggered(QAction * action_triggered){
    //Check if the user wants to open the nym manager (or) select a different default nym
    QString action_triggered_string = QVariant(action_triggered->data()).toString();
    qDebug() << "SERVER TRIGGERED" << action_triggered_string;
    if(action_triggered_string == "openmanager"){
        //Open server-list manager
        mc_defaultserver_slot();
    }else{
        //Set new server default
        QString action_triggered_string_server_name = QVariant(action_triggered->text()).toString();
        set_systrayMenu_server_setDefaultServer(action_triggered_string, action_triggered_string_server_name);
        
        //Refresh the server default selection in the server manager (ONLY if it is open)
        //Check if server manager has ever been opened (then apply logic) [prevents crash if the dialog hasen't be opend before]
        //
        if (mc_servermanager_already_init == 1)
        {
            //Refresh if the server manager is currently open
            if (serverswindow->isVisible())
            {
                mc_servermanager_dialog();
            }
        }
    }
}
// End Server Manager






/** 
 * Withdraw 
 **/

//--- As Cash ---//
void Moneychanger::mc_withdraw_ascash_slot()
{
    //The operator has requested to open the dialog to withdraw as cash.
    mc_withdraw_ascash_dialog();
}

//Open the WithdrawAsCash dialog window

void Moneychanger::mc_withdraw_ascash_dialog()
{
    if (!mc_withdraw_ascash_already_init)
    {
        withdrawascashwindow = new WithdrawAsCashWindow(this);
//        withdrawascashwindow->setAttribute(Qt::WA_DeleteOnClose);
        mc_withdraw_ascash_already_init = true;
    }
    // ------------------------------------
    withdrawascashwindow->dialog();
}

void Moneychanger::close_withdrawascash_dialog()
{
    delete withdrawascashwindow;
    withdrawascashwindow = NULL;
    mc_withdraw_ascash_already_init = false;
    qDebug() << "Withdraw As Cash Closed";
}



//--- As Voucher ---//

void Moneychanger::mc_withdraw_asvoucher_slot()
{
    //The operator has requested to open the dialog to withdraw as cash.
    mc_withdraw_asvoucher_dialog();
}

// Open the WithdrawAsVoucher dialog window

void Moneychanger::mc_withdraw_asvoucher_dialog()
{
    
    if (!mc_withdraw_asvoucher_already_init)
    {
        withdrawasvoucherwindow = new WithdrawAsVoucherWindow(this);
//        withdrawasvoucherwindow->setAttribute(Qt::WA_DeleteOnClose);
        mc_withdraw_asvoucher_already_init = true;
    }
    // ------------------------------------
    withdrawasvoucherwindow->dialog();
}

void Moneychanger::close_withdrawasvoucher_dialog()
{
    delete withdrawasvoucherwindow;
    withdrawasvoucherwindow = NULL;
    mc_withdraw_asvoucher_already_init = false;
    qDebug() << "Withdraw As Voucher Closed";
    
}

void Moneychanger::set_systrayMenu_withdraw_asvoucher_nym_input(QString input)
{
    
    if(!mc_withdraw_asvoucher_already_init){
        mc_withdraw_asvoucher_dialog();
        set_systrayMenu_withdraw_asvoucher_nym_input(input);
    }
    else
        withdrawasvoucherwindow->set_systrayMenu_withdraw_asvoucher_nym_input_text(input);
}

// End Withdrawals






/** 
 * Deposit 
 **/

void Moneychanger::mc_deposit_slot(){
    mc_deposit_show_dialog();
}

void Moneychanger::mc_deposit_show_dialog(){
    
    if (!mc_deposit_already_init)
    {
        depositwindow = new DepositWindow(this);
//        depositwindow->setAttribute(Qt::WA_DeleteOnClose);
        mc_deposit_already_init = true;
    }
    // ------------------------------------
    depositwindow->dialog();
}

void Moneychanger::close_deposit_dialog()
{
    delete depositwindow;
    depositwindow = NULL;
    mc_deposit_already_init = false;
    qDebug() << "Deposit Window Closed";
}

// End Deposit




// resume todo

void Moneychanger::new_compose_dialog() {}
void Moneychanger::new_send_dialog() {}
void Moneychanger::new_request_dialog() {}


/** 
 * Send Funds 
 **/

void Moneychanger::mc_sendfunds_slot()
{
    mc_sendfunds_show_dialog();
}

void Moneychanger::mc_sendfunds_show_dialog()
{
//    if (!mc_sendfunds_already_init)
//    {
//        sendfundswindow = new SendFundsWindow(this);
//        mc_sendfunds_already_init = true;
//    }
//    // ------------------------------------
//    sendfundswindow->dialog();

    // --------------------------------------------------
    MTSendDlg * send_window = new MTSendDlg;
    send_window->setAttribute(Qt::WA_DeleteOnClose);
    // --------------------------------------------------
    QString qstr_acct_id = this->get_default_account_id();

    if (!qstr_acct_id.isEmpty())
        send_window->setInitialMyAcct(qstr_acct_id);
    // ---------------------------------------
    send_window->dialog();
    send_window->show();
    // --------------------------------------------------
}

void Moneychanger::close_sendfunds_dialog()
{
//    delete sendfundswindow;
//    sendfundswindow = NULL;
//    mc_sendfunds_already_init = false;
//    qDebug() << "Send Funds Window Closed";
}

// End Send Funds







/** 
 * Request Funds 
 **/

void Moneychanger::mc_requestfunds_slot()
{
    mc_requestfunds_show_dialog();
}

void Moneychanger::mc_requestfunds_show_dialog()
{
//    if (!mc_requestfunds_already_init)
//    {
//        requestfundswindow = new RequestFundsWindow(this);
//        mc_requestfunds_already_init = true;
//    }
//    // ------------------------------------
//    requestfundswindow->dialog();

    // --------------------------------------------------
    MTRequestDlg * request_window = new MTRequestDlg;
    request_window->setAttribute(Qt::WA_DeleteOnClose);
    // --------------------------------------------------
    QString qstr_acct_id = this->get_default_account_id();

    if (!qstr_acct_id.isEmpty())
        request_window->setInitialMyAcct(qstr_acct_id);
    // ---------------------------------------
    request_window->dialog();
    request_window->show();
    // --------------------------------------------------
}

void Moneychanger::close_requestfunds_dialog()
{
//    delete requestfundswindow;
//    requestfundswindow = NULL;
//    mc_requestfunds_already_init = false;
//    qDebug() << "Request Funds Window Closed";
}


// End Request Funds







/**
 * Market Window
 **/

void Moneychanger::mc_market_slot()
{
    // This is a glaring memory leak, but it's only a temporary placeholder before I redo how windows are handled.
    if(!mc_market_window_already_init)
    {
        market_window = new MarketWindow(this);
//        market_window->setAttribute(Qt::WA_DeleteOnClose);
        mc_market_window_already_init = true;
    }
    // ------------------------------------
    market_window->show();
}

void Moneychanger::close_market_dialog()
{
    delete market_window;
    market_window = NULL;
    mc_market_window_already_init = false;
    qDebug() << "Market Window Closed";
}

// End Market Window


/**
 * Create insurance company wizard
 **/

void Moneychanger::mc_createinsurancecompany_slot()
{
    //The operator has requested to open the create insurance company wizard;
    mc_createinsurancecompany_dialog();
}

void Moneychanger::mc_createinsurancecompany_dialog()
{
    if(!mc_createinsurancecompany_already_init)
    {
        createinsurancecompany_window = new CreateInsuranceCompany(this);
//        createinsurancecompany_window->setAttribute(Qt::WA_DeleteOnClose);
        mc_createinsurancecompany_already_init = true;
        qDebug() << "Create Insurance Company Window Opened";
    }
    // ------------------------------------
    createinsurancecompany_window->show();
}

void Moneychanger::close_createinsurancecompany_dialog()
{
    delete createinsurancecompany_window;
    createinsurancecompany_window = NULL;
    mc_createinsurancecompany_already_init = false;
    qDebug() << "Create Insurance Company Window Closed";
}
