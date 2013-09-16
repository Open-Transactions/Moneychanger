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

#include "Handlers/DBHandler.h"

#include "opentxs/OTAPI.h"
#include "opentxs/OT_ME.h"

/**
 * Constructor & Destructor
 **/

Moneychanger::Moneychanger(QWidget *parent)
: QWidget(parent)
{
    /**
     ** Init variables *
     **/
    
    //Default nym
    default_nym_id = "";
    default_nym_name = "";
    
    //Default server
    
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
    if(DBHandler::getInstance()->querySize("SELECT `nym` FROM `default_nym` LIMIT 0,1") == 0){
        DBHandler::getInstance()->runQuery("INSERT INTO `default_nym` (`nym`) VALUES('')"); // Blank Row
    }
    else{
        if(DBHandler::getInstance()->isNext("SELECT `nym` FROM `default_nym` LIMIT 0,1")){
            default_nym_id = DBHandler::getInstance()->queryString("SELECT `nym` FROM `default_nym` LIMIT 0,1", 0, 0);
        }
        //Ask OT what the display name of this nym is and store it for quick retrieval later on(mostly for "Default Nym" displaying purposes)
        if(default_nym_id != ""){
            default_nym_name =  QString::fromStdString(OTAPI_Wrap::GetNym_Name(default_nym_id.toStdString()));
        }
        else
            qDebug() << "Default Nym loaded from SQL";

    }
    
    /** Default Server **/
    //Query for the default server (So we know for setting later on -- Auto select server associations on later dialogs)
    if(DBHandler::getInstance()->querySize("SELECT `server` FROM `default_server` LIMIT 0,1") == 0){
        DBHandler::getInstance()->runQuery("INSERT INTO `default_server` (`server`) VALUES('')"); // Blank Row
    }
    else{
        if(DBHandler::getInstance()->runQuery("SELECT `server` FROM `default_server` LIMIT 0,1")){
            default_server_id = DBHandler::getInstance()->queryString("SELECT `server` FROM `default_server` LIMIT 0,1", 0, 0);
        }
        //Ask OT what the display name of this server is and store it for a quick retrieval later on(mostly for "Default Server" displaying purposes)
        if(default_server_id != ""){
            default_server_name = QString::fromStdString(OTAPI_Wrap::GetServer_Name(default_server_id.toStdString()));
        }
        else
            qDebug() << "DEFAULT SERVER LOADED FROM SQL";

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
    
    //Menu
    //Address Book
    mc_addressbook_already_init = false;
    //Overview
    mc_overview_already_init = false;
    // Nym Manager
    mc_nymmanager_already_init = false;
    // Asset Manager
    mc_assetmanager_already_init = false;
    // Server Manager
    mc_servermanager_already_init = false;
    // Withdraw as cash
    mc_withdraw_ascash_already_init = false;
    // Withdraw as Voucher
    mc_withdraw_asvoucher_already_init = false;
    //Deposit
    mc_deposit_already_init = false;
    //Send funds
    mc_sendfunds_already_init = false;
    //Request Funds
    mc_requestfunds_already_init = false;
    
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
    mc_systrayMenu_headertext = new QAction("Moneychanger", 0);
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
    mc_systrayMenu_overview = new QAction(mc_systrayIcon_overview, "Transaction History", 0);
    mc_systrayMenu->addAction(mc_systrayMenu_overview);
    //Connect the Overview to a re-action when "clicked";
    connect(mc_systrayMenu_overview, SIGNAL(triggered()), this, SLOT(mc_overview_slot()));
    // --------------------------------------------------------------
    //Separator
    mc_systrayMenu->addSeparator();
    // --------------------------------------------------------------
    //Send funds
    mc_systrayMenu_sendfunds = new QAction(mc_systrayIcon_sendfunds, "Send Funds...", 0);
    mc_systrayMenu->addAction(mc_systrayMenu_sendfunds);
    //Connect button with re-aciton
    connect(mc_systrayMenu_sendfunds, SIGNAL(triggered()), this, SLOT(mc_sendfunds_slot()));
    // --------------------------------------------------------------
    //Request payment
    mc_systrayMenu_requestfunds = new QAction(mc_systrayIcon_requestfunds, "Request Payment...", 0);
    mc_systrayMenu->addAction(mc_systrayMenu_requestfunds);
    // Currently causes a crash , likely due to malformed Dialog construction.
    connect(mc_systrayMenu_requestfunds, SIGNAL(triggered()), this, SLOT(mc_requestfunds_slot()));
    // --------------------------------------------------------------
    //Separator
    mc_systrayMenu->addSeparator();
    // --------------------------------------------------------------
    //Account section
    mc_systrayMenu_account = new QMenu("Set Default Account...", 0);
    mc_systrayMenu_account->setIcon(mc_systrayIcon_goldaccount);
    mc_systrayMenu->addMenu(mc_systrayMenu_account);
    
    //Add a "Manage accounts" action button (and connection)
    QAction * manage_accounts = new QAction("Manage Accounts...", 0);
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
    mc_systrayMenu_asset = new QMenu("Set Default Asset Type...", 0);
    mc_systrayMenu_asset->setIcon(mc_systrayIcon_purse);
    mc_systrayMenu->addMenu(mc_systrayMenu_asset);
    
    //Add a "Manage asset types" action button (and connection)
    QAction * manage_assets = new QAction("Manage Asset Contracts...", 0);
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
    mc_systrayMenu_withdraw = new QMenu("Withdraw", 0);
    mc_systrayMenu_withdraw->setIcon(mc_systrayIcon_withdraw);
    mc_systrayMenu->addMenu(mc_systrayMenu_withdraw);
    //(Withdraw) as Cash
    mc_systrayMenu_withdraw_ascash = new QAction("As Cash",0);
    mc_systrayMenu_withdraw->addAction(mc_systrayMenu_withdraw_ascash);
    //Connect Button with re-action
    connect(mc_systrayMenu_withdraw_ascash, SIGNAL(triggered()), this, SLOT(mc_withdraw_ascash_slot()));
    
    //(Withdraw) as Voucher
    mc_systrayMenu_withdraw_asvoucher = new QAction("As Voucher", 0);
    mc_systrayMenu_withdraw->addAction(mc_systrayMenu_withdraw_asvoucher);
    //Connect Button with re-action
    connect(mc_systrayMenu_withdraw_asvoucher, SIGNAL(triggered()), this, SLOT(mc_withdraw_asvoucher_slot()));
    // --------------------------------------------------------------
    //Deposit
    mc_systrayMenu_deposit = new QAction(mc_systrayIcon_deposit, "Deposit...", 0);
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
    //Advanced
    mc_systrayMenu_advanced = new QMenu("Advanced", 0);
    mc_systrayMenu_advanced->setIcon(mc_systrayIcon_advanced);
    mc_systrayMenu->addMenu(mc_systrayMenu_advanced);
    //Advanced submenu
    
    mc_systrayMenu_advanced_agreements = new QAction(mc_systrayIcon_advanced_agreements, "Agreements", 0);
    mc_systrayMenu_advanced->addAction(mc_systrayMenu_advanced_agreements);
    
    mc_systrayMenu_advanced_markets = new QAction(mc_systrayIcon_advanced_markets, "Markets", 0);
    mc_systrayMenu_advanced->addAction(mc_systrayMenu_advanced_markets);
    connect(mc_systrayMenu_advanced_markets, SIGNAL(triggered()), this, SLOT(mc_market_slot()));
    mc_systrayMenu_advanced->addSeparator();
    
    mc_systrayMenu_advanced_settings = new QAction(mc_systrayIcon_advanced_settings, "Settings...", 0);
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
    QAction * manage_nyms = new QAction("Manage Nyms...", 0);
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
    mc_systrayMenu_server = new QMenu("Set Default Server...", 0);
    mc_systrayMenu_server->setIcon(mc_systrayIcon_server);
    mc_systrayMenu_advanced->addMenu(mc_systrayMenu_server);
    
    //Add a "Manage server" action button (and connection)
    QAction * manage_servers = new QAction("Manage Servers...", 0);
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
    mc_systrayMenu_shutdown = new QAction(mc_systrayIcon_shutdown, "Quit", 0);
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
void Moneychanger::bootTray(){
    //Show systray
    mc_systrayIcon->show();
    
    qDebug() << "BOOTING";
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
void Moneychanger::mc_overview_slot(){
    //The operator has requested to open the dialog to the "Overview";
    mc_overview_dialog();
}

void Moneychanger::mc_overview_dialog(){
    if(!mc_overview_already_init){
        overviewwindow = new OverviewWindow(this);
        overviewwindow->setAttribute(Qt::WA_DeleteOnClose);
        overviewwindow->dialog();
        mc_overview_already_init = true;
        qDebug() << "Overview Opened";
    }
}

void Moneychanger::close_overview_dialog(){
    delete overviewwindow;
    mc_overview_already_init = false;
    qDebug() << "Overview Closed";
}

// End Overview







/** 
 * Address Book 
 **/

void Moneychanger::mc_addressbook_show(QString text){
    //The caller dosen't wish to have the address book paste to anything (they just want to see/manage the address book), just call blank.    
    if(!mc_addressbook_already_init){
        addressbookwindow = new AddressBookWindow(this);
        addressbookwindow->setAttribute(Qt::WA_DeleteOnClose);
        addressbookwindow->show(text);
        mc_overview_already_init = true;
    }
    else
        addressbookwindow->show(text);
}

// End Address Book






/**  
 * Nym Manager 
 **/

//Nym manager "clicked"
void Moneychanger::mc_defaultnym_slot(){
    //The operator has requested to open the dialog to the "Nym Manager";
    mc_nymmanager_dialog();
}

void Moneychanger::mc_nymmanager_dialog(){
    
    if(!mc_nymmanager_already_init){
        NymManagerWindow *nymmanagerwindow = new NymManagerWindow(this);
        nymmanagerwindow->setAttribute(Qt::WA_DeleteOnClose);
        nymmanagerwindow->dialog();
        mc_overview_already_init = true;
    }
}

//Additional Nym Manager Functions
void Moneychanger::set_systrayMenu_nym_setDefaultNym(QString nym_id, QString nym_name){
    //Set default nym internal memory
    default_nym_id = nym_id;
    default_nym_name = nym_name;
    
    //SQL UPDATE default nym
    DBHandler::getInstance()->AddressBookUpdateDefaultNym(nym_id);
    
    //Rename "NYM:" if a nym is loaded
    if(nym_id != ""){
        mc_systrayMenu_nym->setTitle("Nym: "+nym_name);
    }
}

void Moneychanger::mc_systrayMenu_reload_nymlist(){
    qDebug() << "RELOAD NYM LIST";
    //Count nyms
    int32_t nym_count_int32_t = OTAPI_Wrap::GetNymCount();
    int nym_count = nym_count_int32_t;
    
    //Retrieve updated list of nym submenu actions list
    QList<QAction*> action_list_to_nym_submenu = mc_systrayMenu_nym->actions();
    
    //Remove all sub-menus from the nym submenu
    for(int a = action_list_to_nym_submenu.size(); a > 0; a--){
        qDebug() << "REMOVING" << a;
        QPoint tmp_point = QPoint(a, 0);
        mc_systrayMenu_nym->removeAction(mc_systrayMenu_nym->actionAt(tmp_point));
    }
    
    //Remove all nyms from the backend list
    //Remove ids from backend list
    int tmp_nym_list_id_size = nym_list_id->size();
    for(int a = 0; a < tmp_nym_list_id_size; a++){
        nym_list_id->removeLast();
    }
    
    //Remove names from the backend list
    int tmp_nym_list_name_size = nym_list_name->size();
    for(int a = 0; a < tmp_nym_list_name_size; a++){
        nym_list_name->removeLast();
    }
    
    //Add/append to the id + name lists
    for(int a = 0; a < nym_count; a++){
        //Get OT Account ID
        QString OT_nym_id = QString::fromStdString(OTAPI_Wrap::GetNym_ID(a));
        
        //Add to qlist
        nym_list_id->append(QVariant(OT_nym_id));
        
        //Get OT Account Name
        QString OT_nym_name = QString::fromStdString(OTAPI_Wrap::GetNym_Name(OTAPI_Wrap::GetNym_ID(a)));
        
        //Add to qlist
        nym_list_name->append(QVariant(OT_nym_name));
        
        //Append to submenu of nym
        QAction * next_nym_action = new QAction(mc_systrayIcon_nym, OT_nym_name, 0);
        next_nym_action->setData(QVariant(OT_nym_id));
        mc_systrayMenu_nym->addAction(next_nym_action);
    }
    
    
}

//Nym new default selected from systray
void Moneychanger::mc_nymselection_triggered(QAction*action_triggered){
    //Check if the user wants to open the nym manager (or) select a different default nym
    QString action_triggered_string = QVariant(action_triggered->data()).toString();
    qDebug() << "NYM TRIGGERED" << action_triggered_string;
    if(action_triggered_string == "openmanager"){
        //Open nym manager
        mc_defaultnym_slot();
    }else{
        //Set new nym default
        QString action_triggered_string_nym_name = QVariant(action_triggered->text()).toString();
        set_systrayMenu_nym_setDefaultNym(action_triggered_string, action_triggered_string_nym_name);
        
        //Refresh the nym default selection in the nym manager (ONLY if it is open)
        //Check if nym manager has ever been opened (then apply logic) [prevents crash if the dialog hasen't be opend before]
        if(mc_nymmanager_already_init){
            //Refresh if the nym manager is currently open
            if(mc_nymmanager_already_init){
                mc_nymmanager_dialog();
            }
        }
    }
    
}

// End Nym Manager







/** 
 * Asset Manager 
 **/

//Asset manager "clicked"
void Moneychanger::mc_defaultasset_slot(){
    //The operator has requested to open the dialog to the "Asset Manager";
    mc_assetmanager_dialog();
}

void Moneychanger::mc_assetmanager_dialog(){
    if(!mc_assetmanager_already_init){
        AssetManagerWindow *assetmanagerwindow = new AssetManagerWindow(this);
        assetmanagerwindow->setAttribute(Qt::WA_DeleteOnClose);
        assetmanagerwindow->dialog();
        mc_assetmanager_already_init = true;
    }
}

//Additional Asset slots

//This was mistakenly named asset_load_asset, should be set default asset
//Set Default asset
void Moneychanger::set_systrayMenu_asset_setDefaultAsset(QString asset_id, QString asset_name){
    //Set default asset internal memory
    default_asset_id = asset_id;
    default_asset_name = asset_name;
    
    //SQL UPDATE default asset
    DBHandler::getInstance()->AddressBookUpdateDefaultAsset(asset_id);
    
    //Rename "ASSET:" if a asset is loaded
    if(asset_id != ""){
        mc_systrayMenu_asset->setTitle("Asset Type: "+asset_name);
    }
}

void Moneychanger::mc_systrayMenu_reload_assetlist(){
    qDebug() << "RELOAD asset LIST";
    //Count assets
    int32_t asset_count_int32_t = OTAPI_Wrap::GetAssetTypeCount();
    int asset_count = asset_count_int32_t;
    
    //Retrieve updated list of asset submenu actions list
    QList<QAction*> action_list_to_asset_submenu = mc_systrayMenu_asset->actions();
    
    //Remove all sub-menus from the asset submenu
    for(int a = action_list_to_asset_submenu.size(); a > 0; a--){
        qDebug() << "REMOVING" << a;
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
void Moneychanger::mc_defaultaccount_slot(){
    //The operator has requested to open the dialog to the "account Manager";
    mc_accountmanager_dialog();
}

void Moneychanger::mc_accountmanager_dialog(){
    if(!mc_accountmanager_already_init){
        accountmanagerwindow = new AccountManagerWindow(this);
        accountmanagerwindow->setAttribute(Qt::WA_DeleteOnClose);
        accountmanagerwindow->dialog();
        mc_accountmanager_already_init = true;
    }
}

// Our public interface for closing the account manager
void Moneychanger::close_accountmanager_dialog(){
    delete accountmanagerwindow;
    mc_accountmanager_already_init = false;
}

//Account Manager Additional Functions

//Account new default selected from systray
void Moneychanger::mc_accountselection_triggered(QAction*action_triggered){
    //Check if the user wants to open the account manager (or) select a different default account
    QString action_triggered_string = QVariant(action_triggered->data()).toString();
    qDebug() << "account TRIGGERED" << action_triggered_string;
    if(action_triggered_string == "openmanager"){
        //Open account manager
        mc_defaultaccount_slot();
    }else{
        //Set new account default
        QString action_triggered_string_account_name = QVariant(action_triggered->text()).toString();
        set_systrayMenu_account_setDefaultAccount(action_triggered_string, action_triggered_string_account_name);
        
        //Refresh the account default selection in the account manager (ONLY if it is open)
        //Check if account manager has ever been opened (then apply logic) [prevents crash if the dialog hasen't be opend before]
        if(mc_accountmanager_already_init){
            //Refresh if the account manager is currently open
            if(mc_accountmanager_already_init){
                mc_accountmanager_dialog();
            }
        }
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
        QString result = "Account: " + account_name;
        
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

void Moneychanger::mc_systrayMenu_reload_accountlist(){
    qDebug() << "RELOAD account LIST";
    //Count accounts
    int32_t account_count_int32_t = OTAPI_Wrap::GetAccountCount();
    int account_count = account_count_int32_t;
    
    //Retrieve updated list of account submenu actions list
    QList<QAction*> action_list_to_account_submenu = mc_systrayMenu_account->actions();
    
    //Remove all sub-menus from the account submenu
    for(int a = action_list_to_account_submenu.size(); a > 0; a--){
        qDebug() << "REMOVING" << a;
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

void Moneychanger::mc_defaultserver_slot(){
    mc_servermanager_dialog();
}

void Moneychanger::mc_servermanager_dialog(){
    if(!mc_servermanager_already_init){
        servermanagerwindow = new ServerManagerWindow(this);
        servermanagerwindow->setAttribute(Qt::WA_DeleteOnClose);
        servermanagerwindow->dialog();
        mc_servermanager_already_init = true;
    }
}

void Moneychanger::close_servermanager_dialog(){
    delete servermanagerwindow;
    mc_servermanager_already_init = false;
}

void Moneychanger::set_systrayMenu_server_setDefaultServer(QString server_id, QString server_name){
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
        new_server_title = "Set Default...";
    }
    
    mc_systrayMenu_server->setTitle("Server: "+new_server_title);
}

void Moneychanger::mc_systrayMenu_reload_serverlist(){
    qDebug() << "RELOAD SERVER LIST";
    //Count server
    int32_t server_count_int32_t = OTAPI_Wrap::GetServerCount();
    int server_count = server_count_int32_t;
    
    //Retrieve updated list of server submenu actions list
    QList<QAction*> action_list_to_server_submenu = mc_systrayMenu_server->actions();
    
    //Remove all sub-menus from the server submenu
    for(int a = action_list_to_server_submenu.size(); a > 0; a--){
        qDebug() << "REMOVING" << a;
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
        if(mc_servermanager_already_init == 1){
            //Refresh if the server manager is currently open
            if(servermanagerwindow->isVisible()){
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
void Moneychanger::mc_withdraw_ascash_slot(){
    //The operator has requested to open the dialog to withdraw as cash.
    mc_withdraw_ascash_dialog();
}

//Open the WithdrawAsCash dialog window

void Moneychanger::mc_withdraw_ascash_dialog(){
    if(!mc_withdraw_ascash_already_init){
        withdrawascashwindow = new WithdrawAsCashWindow(this);
        withdrawascashwindow->setAttribute(Qt::WA_DeleteOnClose);
        withdrawascashwindow->dialog();
        mc_withdraw_ascash_already_init = true;
    }
};




//--- As Voucher ---//

void Moneychanger::mc_withdraw_asvoucher_slot(){
    //The operator has requested to open the dialog to withdraw as cash.
    mc_withdraw_asvoucher_dialog();
}

// Open the WithdrawAsVoucher dialog window

void Moneychanger::mc_withdraw_asvoucher_dialog(){
    
    if(!mc_withdraw_asvoucher_already_init){
        withdrawasvoucherwindow = new WithdrawAsVoucherWindow(this);
        withdrawasvoucherwindow->setAttribute(Qt::WA_DeleteOnClose);
        withdrawasvoucherwindow->dialog();
        mc_withdraw_asvoucher_already_init = true;
    }
    else
        withdrawasvoucherwindow->show();
    
}

void Moneychanger::set_systrayMenu_withdraw_asvoucher_nym_input(QString input){
    
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
    
    if(!mc_deposit_already_init){
        depositwindow = new DepositWindow(this);
        depositwindow->setAttribute(Qt::WA_DeleteOnClose);
        depositwindow->dialog();
        mc_deposit_already_init = true;
    }
    else
        depositwindow->show();
}

// End Deposit






/** 
 * Send Funds 
 **/

void Moneychanger::mc_sendfunds_slot(){
    mc_sendfunds_show_dialog();
}

void Moneychanger::mc_sendfunds_show_dialog(){
    
    if(!mc_sendfunds_already_init){
        sendfundswindow = new SendFundsWindow(this);
        sendfundswindow->setAttribute(Qt::WA_DeleteOnClose);
        sendfundswindow->dialog();
        mc_sendfunds_already_init = true;
    }
    else
        sendfundswindow->show();

}

// End Send Funds






/** 
 * Request Funds 
 **/

void Moneychanger::mc_requestfunds_slot(){
    mc_requestfunds_show_dialog();
}

void Moneychanger::mc_requestfunds_show_dialog(){
    
    if(!mc_requestfunds_already_init){
        requestfundswindow = new RequestFundsWindow(this);
        requestfundswindow->setAttribute(Qt::WA_DeleteOnClose);
        requestfundswindow->dialog();
        mc_requestfunds_already_init = true;
    }
    else
        requestfundswindow->show();
    
}

// End Request Funds







/**
 * Market Window
 **/

void Moneychanger::mc_market_slot(){
    
    // This is a glaring memory leak, but it's only a temporary placeholder before I redo how windows are handled.
    if(!mc_market_window_already_init){
        MarketWindow *market_window = new MarketWindow(this);
        market_window->setAttribute(Qt::WA_DeleteOnClose);
        market_window->show();
        mc_market_window_already_init = true;
    }
}

// End Market Window




