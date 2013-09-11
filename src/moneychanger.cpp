#include "moneychanger.h"
#include "ot_worker.h"
#include "MarketWindow.h"

#include "opentxs/OTAPI.h"
#include "opentxs/OT_ME.h"

/**
 ** Constructor & Destructor
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
    
    //SQLite databases
    addressbook_db = QSqlDatabase::addDatabase("QSQLITE", "addressBook");
    addressbook_db.setDatabaseName("./db/mc_db");
    qDebug() << addressbook_db.lastError();
    bool db_opened = addressbook_db.open();
    qDebug() << "DB OPENED " << db_opened;
    
    /** Default Nym **/
    //Query for default nym (So we know for setting later on -- Pseudonym manager)
    QSqlQuery default_nym_query(addressbook_db);
    default_nym_query.exec(QString("SELECT `nym` FROM `default_nym` LIMIT 0,1"));
    if(default_nym_query.size() == 0){
        QSqlQuery insert_blank_row(addressbook_db);
        insert_blank_row.exec(QString("INSERT INTO `default_nym` (`nym`) VALUES('')"));
        
    }else{
        if(default_nym_query.next()){
            QString default_nym_id_db = default_nym_query.value(0).toString();
            default_nym_id = default_nym_id_db;
            
            //Ask OT what the display name of this nym is and store it for quick retrieval later on(mostly for "Default Nym" displaying purposes)
            if(default_nym_id != ""){
                default_nym_name =  QString::fromStdString(OTAPI_Wrap::GetNym_Name(default_nym_id.toStdString()));
            }
        }
    }
    
    /** Default Server **/
    //Query for the default server (So we know for setting later on -- Auto select server associations on later dialogs)
    QSqlQuery default_server_query(addressbook_db);
    default_server_query.exec(QString("SELECT `server` FROM `default_server` LIMIT 0,1"));
    if(default_server_query.size() == 0){
        QSqlQuery insert_blank_row(addressbook_db);
        insert_blank_row.exec(QString("INSERT INTO `default_server` (`server`) VALUES(' ')"));
        
    }else{
        qDebug() << "DEFAULT SERVER LOADED FROM SQL";
        if(default_server_query.first()){
            QString default_server_id_db = default_server_query.value(0).toString();
            default_server_id = default_server_id_db;
            
            //Ask OT what the display name of this server is and store it for a quick retrieval later on(mostly for "Default Server" displaying purposes)
            if(default_server_id != ""){
                default_server_name = QString::fromStdString(OTAPI_Wrap::GetServer_Name(default_server_id.toStdString()));
                
            }
        }
    }
    
    //Ask OT for "cash account" information (might be just "Account" balance)
    //Ask OT the purse information
    
    
    /* *** *** ***
     * Init Memory Trackers (there may be other int below than just memory trackers but generally there will be mostly memory trackers below)
     * Allows the program to boot with a low footprint -- keeps start times low no matter the program complexity;
     * Memory will expand as the operator opens dialogs;
     * Also prevents HTTP requests from overloading or spamming the operators device by only allowing one window of that request;
     * *** *** ***/
    
    //Address Book
    mc_addressbook_already_init = 0;
    mc_addressbook_refreshing = 0;
    mc_addressbook_paste_into = ""; //When set, the selected nym will be pasted into the desired area.
    
    //Menu
    //Overview
    mc_overview_already_init = 0;
    
    //Nym Manager
    mc_nymmanager_already_init = 0;
    mc_nymmanager_refreshing = 0;
    mc_nymmanager_proccessing_dataChanged = 0;
    
    //"Add Nym" dialog
    mc_nymmanager_addnym_dialog_already_init = 0;
    mc_nymmanager_addnym_dialog_advanced_showing = 0;
    
    //"Remove Nym" dialog
    mc_nymmanager_removenym_dialog_already_init = 0;
    
    //Server Manager
    mc_servermanager_already_init = 0;
    mc_servermanager_refreshing = 0;
    mc_servermanager_proccessing_dataChanged = 0;
    
    //"Add server" dialog
    mc_servermanager_addserver_dialog_already_init = 0;
    mc_servermanager_addserver_dialog_advanced_showing = 0;
    
    //"Remove server" dialog
    mc_servermanager_removeserver_dialog_already_init = 0;
    
    //Asset Manager
    mc_assetmanager_already_init = 0;
    mc_assetmanager_refreshing = 0;
    mc_assetmanager_proccessing_dataChanged = 0;
    
    //"Add asset" dialog
    mc_assetmanager_addasset_dialog_already_init = 0;
    mc_assetmanager_addasset_dialog_advanced_showing = 0;
    
    //"Remove asset" dialog
    mc_assetmanager_removeasset_dialog_already_init = 0;
    
    //Account Manager
    mc_accountmanager_already_init = 0;
    mc_accountmanager_refreshing = 0;
    mc_accountmanager_proccessing_dataChanged = 0;
    
    //"Add account" dialog
    mc_accountmanager_addaccount_dialog_already_init = 0;
    mc_accountmanager_addaccount_dialog_advanced_showing = 0;
    
    //"Remove account" dialog
    mc_accountmanager_removeaccount_dialog_already_init = 0;
    
    //Withdraw
    //As Cash
    mc_withdraw_ascash_dialog_already_init = 0;
    mc_withdraw_ascash_confirm_dialog_already_init = 0;
    
    //As Voucher
    mc_withdraw_asvoucher_dialog_already_init = 0;
    mc_withdraw_asvoucher_confirm_dialog_already_init = 0;
    
    //Deposit
    mc_deposit_already_init = 0;
    
    //Send funds
    mc_sendfunds_already_init = 0;
    
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
    mc_systrayMenu_account_setDefaultAccount(default_account_id, default_account_name);
    
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
    mc_systrayMenu_asset_setDefaultAsset(default_asset_id, default_asset_name);
    
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
    mc_systrayMenu_nym_setDefaultNym(default_nym_id, default_nym_name);
    
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
    mc_systrayMenu_server_setDefaultServer(default_server_id, default_server_name);
    
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


/** ****** ****** ******  **
 ** Public Function/Calls **/

// Mine

void Moneychanger::mc_market_slot(){
    
    // This is a glaring memory leak, but it's only a temporary placeholder before I redo how windows are handled.
    MarketWindow *market_window = new MarketWindow(this);
    market_window->show();
}

// End Mine


//start
void Moneychanger::bootTray(){
    //Show systray
    mc_systrayIcon->show();
    
    qDebug() << "BOOTING";
}

/** ****** ****** ******   **
 ** Private Function/Calls **/

/* **
 * Address Book Related Calls
 */
/** Show Address Book **/
//Dummy Call
void Moneychanger::mc_addressbook_show(){
    //The caller dosen't wish to have the address book paste to anything (they just want to see/manage the address book), just call blank.
    mc_addressbook_show("");
}


//This will show the addressbook (and paste the selection accordingly if set)
void Moneychanger::mc_addressbook_show(QString paste_selection_to){
    /** Order of Operations
     *Initialize
     *Paste Selection Logic
     *Data Refresh/Fill Logic
     **/
    
    
    /** ***
     ** Initialize Address Book and/or just show
     **/
    //Check if address book has been init before.
    if(mc_addressbook_already_init == 0){
        //Init address book, then show
        mc_addressbook_dialog = new QDialog(0);
        mc_addressbook_dialog->setModal(1); //(Nice effect; Dims all windows except the address book and makes the address book on top upon showing
        mc_addressbook_dialog->setWindowTitle("Address Book | Moneychanger");
        
        //Set layout
        mc_addressbook_gridlayout = new QGridLayout(0);
        mc_addressbook_dialog->setLayout(mc_addressbook_gridlayout);
        
        /* First Row in Address Book Grid */
        //Label (Address Book)
        mc_addressbook_label = new QLabel("<h3>Address Book</h3>");
        mc_addressbook_label->setAlignment(Qt::AlignRight);
        mc_addressbook_gridlayout->addWidget(mc_addressbook_label, 0,0, 1,2);
        
        /* Second Row in Address Book Grid */
        /** First column in address book grid (left side) **/
        //Table View (backend and visual init)
        mc_addressbook_tableview_itemmodel = new QStandardItemModel(0,3,0);
        mc_addressbook_tableview_itemmodel->setHorizontalHeaderItem(0, new QStandardItem(QString("Display Nym")));
        mc_addressbook_tableview_itemmodel->setHorizontalHeaderItem(1, new QStandardItem(QString("Nym ID")));
        mc_addressbook_tableview_itemmodel->setHorizontalHeaderItem(2, new QStandardItem(QString("Backend DB ID")));
        //Connect tableviews' backend "dataChanged" signal to a re-action.
        connect(mc_addressbook_tableview_itemmodel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(mc_addressbook_dataChanged_slot(QModelIndex,QModelIndex)));
        
        mc_addressbook_tableview = new QTableView(0);
        mc_addressbook_tableview->setSelectionMode(QAbstractItemView::SingleSelection);
        mc_addressbook_tableview->setModel(mc_addressbook_tableview_itemmodel);
        mc_addressbook_tableview->hideColumn(2);
        mc_addressbook_gridlayout->addWidget(mc_addressbook_tableview, 1,0, 1,1);
        
        
        /** Second column in address book grid (right side) **/
        //2 Buttons (Add/Remove)
        mc_addressbook_addremove_btngroup_widget = new QWidget(0);
        mc_addressbook_addremove_btngroup_holder = new QVBoxLayout(0);
        
        mc_addressbook_addremove_btngroup_widget->setLayout(mc_addressbook_addremove_btngroup_holder);
        mc_addressbook_gridlayout->addWidget(mc_addressbook_addremove_btngroup_widget, 1,1, 1,1, Qt::AlignTop);
        
        //Add button
        mc_addressbook_addremove_add_btn = new QPushButton("Add Contact", 0);
        mc_addressbook_addremove_add_btn->setStyleSheet("QPushButton{padding:0.5em;margin:0}");
        mc_addressbook_addremove_btngroup_holder->addWidget(mc_addressbook_addremove_add_btn, 0, Qt::AlignTop);
        //Connect the add contact button with a re-action
        connect(mc_addressbook_addremove_add_btn, SIGNAL(clicked()), this, SLOT(mc_addressbook_addblankrow_slot()));
        
        //Remove button
        mc_addressbook_addremove_remove_btn = new QPushButton("Remove Contact", 0);
        mc_addressbook_addremove_remove_btn->setStyleSheet("QPushButton{padding:0.5em;margin:0}");
        mc_addressbook_addremove_btngroup_holder->addWidget(mc_addressbook_addremove_remove_btn, 0, Qt::AlignTop);
        //Connect the remove contact button with a re-action
        connect(mc_addressbook_addremove_remove_btn, SIGNAL(clicked()), this, SLOT(mc_addressbook_confirm_remove_contact_slot()));
        
        /* Third row in Address Book Grid */
        /** Spans 2 columns **/
        //This "select" button will be shown if the address book was initiated with the intention of pasting the selection into a dialog/window
        mc_addressbook_select_nym_for_paste_btn = new QPushButton("Paste selected contact as Nym Id",0);
        mc_addressbook_select_nym_for_paste_btn->setStyleSheet("QPushButton{padding:0.5em;}");
        mc_addressbook_select_nym_for_paste_btn->hide();
        mc_addressbook_gridlayout->addWidget(mc_addressbook_select_nym_for_paste_btn, 2,0, 1,2, Qt::AlignHCenter);
        //Connect the "select" button with a re-action
        connect(mc_addressbook_select_nym_for_paste_btn, SIGNAL(clicked()), SLOT(mc_addressbook_paste_selected_slot()));
        //Show dialog
        mc_addressbook_dialog->show();
        mc_addressbook_dialog->activateWindow();
        
        /** Flag already init **/
        mc_addressbook_already_init = 1;
        
    }else{
        //Address book already init, show it!
        mc_addressbook_dialog->show();
        mc_addressbook_dialog->activateWindow();
    }
    
    /** ***
     ** Paste Selection Logic
     **/
    //If paste_selection_to is nothing then "hide" the select button
    if(paste_selection_to == ""){
        mc_addressbook_select_nym_for_paste_btn->hide();
    }else{
        //Set mc_addressbook_paste_into = paste_selection_to
        mc_addressbook_paste_into = paste_selection_to;
        
        //Show select button
        mc_addressbook_select_nym_for_paste_btn->show();
    }
    
    
    
    /** ***
     ** Data Refresh/Fill Logic
     **/
    //Refresh Addressbook with listing
    /** Flag Refreshing Address Book **/
    mc_addressbook_refreshing = 1;
    
    //remove all rows from the address book (so we can refresh any newly changed data)
    mc_addressbook_tableview_itemmodel->removeRows(0, mc_addressbook_tableview_itemmodel->rowCount());
    
    QSqlQuery mc_addressbook_query(addressbook_db);
    mc_addressbook_query.exec(QString("SELECT `id`, `nym_display_name`, `nym_id` FROM `address_book`"));
    qDebug() << "DB QUERY LAST ERROR: " << mc_addressbook_query.lastError();
    //Add Rows of data to the backend of the table view (QStandardItemModel)
    int row_index = 0;
    while(mc_addressbook_query.next()){
        //Extract data
        QString addressbook_row_id = mc_addressbook_query.value(0).toString();
        QString addressbook_row_nym_display_name = mc_addressbook_query.value(1).toString();
        QString addressbook_row_nym_id = mc_addressbook_query.value(2).toString();
        
        //Place extracted data into the table view
        QStandardItem * col_one = new QStandardItem(addressbook_row_nym_display_name);
        QStandardItem * col_two = new QStandardItem(addressbook_row_nym_id);
        QStandardItem * col_three = new QStandardItem(addressbook_row_id);
        
        mc_addressbook_tableview_itemmodel->setItem(row_index, 0, col_one);
        mc_addressbook_tableview_itemmodel->setItem(row_index, 1, col_two);
        mc_addressbook_tableview_itemmodel->setItem(row_index, 2, col_three);
        
        //Increment index
        row_index += 1;
        
        //Clear address book variables
        addressbook_row_id = "";
        addressbook_row_nym_display_name = "";
        addressbook_row_nym_id = "";
    }
    
    /** Un-Flag Refreshing Address Book **/
    mc_addressbook_refreshing = 0;
    
    //Resize
    mc_addressbook_dialog->resize(400, 300);
}


/* **
 * Menu Dialog Related Calls
 */

/** Overview Dialog **/
void Moneychanger::mc_overview_dialog(){
    /** If the overview dialog has already been init
     *  just show it, Other wise, init and show if this is
     *  the first time.
     **/
    if(mc_overview_already_init == 0){
        //The overview dialog has not been init yet; Init, then show it.
        mc_overview_dialog_page = new QDialog(0);
        mc_overview_dialog_page->setWindowFlags(Qt::WindowStaysOnTopHint);
        mc_overview_dialog_page->setWindowTitle("Overview | Moneychanger");
        //Grid Layout
        mc_overview_gridlayout = new QGridLayout(0);
        mc_overview_dialog_page->setLayout(mc_overview_gridlayout);
        
        //Label (header)
        mc_overview_header_label = new QLabel("<h3>Overview of Transactions</h3>", 0);
        mc_overview_gridlayout->addWidget(mc_overview_header_label, 0,0, 1,1, Qt::AlignRight);
        
        //in/Outgoing (Pane)
        mc_overview_inoutgoing_pane_holder = new QWidget(0);
        mc_overview_inoutgoing_pane = new QVBoxLayout(0);
        mc_overview_inoutgoing_pane_holder->setLayout(mc_overview_inoutgoing_pane);
        mc_overview_gridlayout->addWidget(mc_overview_inoutgoing_pane_holder, 1,0, 1,1);
        
        //Label (inOutgoing header)
        //                              mc_overview_inoutgoing_header_label = new QLabel("<b>Incoming & Outgoing Transactions</b>");
        mc_overview_inoutgoing_header_label = new QLabel;
        mc_overview_inoutgoing_pane->addWidget(mc_overview_inoutgoing_header_label);
        
        //GridView (Lists of TX)
        mc_overview_inoutgoing_scroll = new QScrollArea;
        mc_overview_inoutgoing_gridview_widget = new QWidget(0);
        mc_overview_inoutgoing_gridview = new QGridLayout(0);
        mc_overview_inoutgoing_gridview_widget->setLayout(mc_overview_inoutgoing_gridview);
        mc_overview_inoutgoing_gridview->setGeometry(QRect(100,100,100,100));
        mc_overview_inoutgoing_scroll->setWidgetResizable(true);
        mc_overview_inoutgoing_scroll->setBackgroundRole(QPalette::Light);
        mc_overview_inoutgoing_scroll->setWidget(mc_overview_inoutgoing_gridview_widget);
        mc_overview_inoutgoing_pane->addWidget(mc_overview_inoutgoing_scroll);
        
        
        /** Flag Already Init **/
        mc_overview_already_init = 1;
    }
    //Resize
    mc_overview_dialog_page->resize(800, 400);
    //Show
    mc_overview_dialog_page->show();
    
    //Refresh visual data
    //Tell OT to repopulate, and refresh backend.
    ot_worker_background->mc_overview_ping();
    
    //Now refresh the repopulated data visually
    mc_overview_dialog_refresh();
}

//Overview refresh function
void Moneychanger::mc_overview_dialog_refresh(){
    //(Lock the overview dialog refreshing mechinism until finished)
    QMutexLocker overview_refresh_locker(&mc_overview_refreshing_visuals_mutex);
    
    //Update the overview dialog visuals
    QList< QMap<QString,QVariant> > current_list_copy = ot_worker_background->mc_overview_get_currentlist();
    
    //Clear all records (In the future we should have a scan for updates records mechinism for now we will go for a browser "refresh" all mechinism)
    int items_in_inoutgoing_gridview = mc_overview_inoutgoing_gridview->rowCount();
    for(int a = 0; a < items_in_inoutgoing_gridview; a++){
        QLayoutItem * item = mc_overview_inoutgoing_gridview->takeAt(0);
        mc_overview_inoutgoing_gridview->removeItem(item);
        delete item;
        
    }
    
    int total_records_to_visualize = current_list_copy.size();
    for(int a = 0; a < total_records_to_visualize; a++){
        //Get map for this record
        QMap<QString, QVariant> record_map = current_list_copy.at(a);
        
        //Append to transactions list in overview dialog.
        QWidget * row_widget = new QWidget(0);
        QGridLayout * row_widget_layout = new QGridLayout(0);
        row_widget->setLayout(row_widget_layout);
        row_widget->setStyleSheet("QWidget{background-color:#c0cad4;}");
        
        //Render row.
        //Header of row
        QString tx_name = QString(record_map["name"].toString());
        if(tx_name.trimmed() == ""){
            //Tx has no name
            tx_name.clear();
            tx_name = "Transaction";
        }
        
        QLabel * header_of_row = new QLabel(0);
        QString header_of_row_string = QString();
        header_of_row_string.append(tx_name);
        
        header_of_row->setText(header_of_row_string);
        
        //Append header to layout
        row_widget_layout->addWidget(header_of_row, 0, 0, 1,1, Qt::AlignLeft);
        
        // Amount (with currency tla)
        QLabel * currency_amount_label = new QLabel(0);
        QString currency_amount = QString();
        
        
        if (record_map["ismail"].toBool())
        {
            if (record_map["isoutgoing"].toBool())
                currency_amount.append("sent message");
            else
                currency_amount.append("message");
        }
        else
            currency_amount.append(record_map["formatAmount"].toString());
        //                                    currency_amount.append(record_map["currencyTLA"].toString());
        //                                    currency_amount.append(QString(" %1"));
        //                                    currency_amount = currency_amount.arg(record_map["amount"].toString());
        
        if (record_map["isoutgoing"].toBool() || (record_map["amount"].toInt() < 0))
            currency_amount_label->setStyleSheet("QLabel { color : red; }");
        else
            currency_amount_label->setStyleSheet("QLabel { color : green; }");
        
        currency_amount_label->setText(currency_amount);
        row_widget_layout->addWidget(currency_amount_label, 0, 1, 1,1, Qt::AlignRight);
        
        //Sub-info
        QWidget * row_content_container = new QWidget(0);
        QGridLayout * row_content_grid = new QGridLayout(0);
        row_content_container->setLayout(row_content_grid);
        row_widget_layout->addWidget(row_content_container, 1,0, 1,2);
        
        /** Column one **/
        //Date (sub-info)
        //Calc/convert date/times
        QDateTime timestamp;
        timestamp.setTime_t(record_map["date"].toInt());
        
        QLabel * row_content_date_label = new QLabel(0);
        QString row_content_date_label_string = QString();
        row_content_date_label_string.append(QString(timestamp.toString(Qt::SystemLocaleShortDate)));
        row_content_date_label->setText(row_content_date_label_string);
        row_content_grid->addWidget(row_content_date_label, 0,0, 1,1, Qt::AlignLeft);
        
        /** Column two **/
        //Status
        QLabel * row_content_status_label = new QLabel(0);
        QString row_content_status_string = QString();
        
        //                                    if (record_map["ismail"].toBool())
        //                                        row_content_status_string.append(record_map["shortMail"].toString());
        //                                    else
        row_content_status_string.append(record_map["formatDescription"].toString());
        
        //add string to label
        row_content_status_label->setText(row_content_status_string);
        
        //add to row_content grid
        row_content_grid->addWidget(row_content_status_label, 0,1, 1,1, Qt::AlignRight);
        
        
        /** Append information to the grid/visuals. **/
        mc_overview_inoutgoing_gridview->addWidget(row_widget, a,0, 1,1);
        
    }
    
}
// ----------------------------------------------------------------------
/** Nym Manager Dialog **/
void Moneychanger::mc_nymmanager_dialog(){
    
    /** If the nym managerh dialog has already been init,
     *  just show it, Other wise, init and show if this is the
     *  first time.
     **/
    if(mc_nymmanager_already_init == 0){
        
        //The Nym Manager has not been init yet; Init, then show it.
        mc_nym_manager_dialog = new QDialog(0);
        
        /** window properties **/
        //Set window title
        mc_nym_manager_dialog->setWindowTitle("Nym Manager | Moneychanger");
        
        //Set window on top
        mc_nym_manager_dialog->setWindowFlags(Qt::WindowStaysOnTopHint);
        
        /** layout and content **/
        //Grid layout
        mc_nym_manager_gridlayout = new QGridLayout(0);
        mc_nym_manager_dialog->setLayout(mc_nym_manager_gridlayout);
        
        /* First Row in nym manager Grid */
        //Label (header)
        mc_nym_manager_label = new QLabel("<h3>Pseudonyms</h3>", 0);
        mc_nym_manager_label->setAlignment(Qt::AlignRight);
        mc_nym_manager_gridlayout->addWidget(mc_nym_manager_label, 0,0, 1,2, Qt::AlignRight);
        
        
        /* Second Row in nym manager Grid */
        /** First column in address book grid (left side) **/
        //Horizontal box (contains: List of pseudo nyms, add/remove buttons)
        mc_nym_manager_holder = new QWidget(0);
        mc_nym_manager_hbox = new QHBoxLayout(0);
        mc_nym_manager_holder->setLayout(mc_nym_manager_hbox);
        mc_nym_manager_gridlayout->addWidget(mc_nym_manager_holder);
        
        //Table View (backend and visual init)
        mc_nym_manager_tableview_itemmodel = new QStandardItemModel(0,3,0);
        mc_nym_manager_tableview_itemmodel->setHorizontalHeaderItem(0, new QStandardItem(QString("Pseudonym Display Name")));
        mc_nym_manager_tableview_itemmodel->setHorizontalHeaderItem(1, new QStandardItem(QString("Pseudonym ID")));
        mc_nym_manager_tableview_itemmodel->setHorizontalHeaderItem(2, new QStandardItem(QString("Default")));
        
        //Connect tableviews' backend "dataChanged" signal to a re-action.
        connect(mc_nym_manager_tableview_itemmodel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(mc_nymmanager_dataChanged_slot(QModelIndex,QModelIndex)));
        
        mc_nym_manager_tableview = new QTableView(0);
        mc_nym_manager_tableview->setSelectionMode(QAbstractItemView::SingleSelection);
        mc_nym_manager_tableview->setModel(mc_nym_manager_tableview_itemmodel);
        mc_nym_manager_tableview->setColumnWidth(0, 175);
        mc_nym_manager_tableview->setColumnWidth(1, 150);
        mc_nym_manager_tableview->setColumnWidth(2, 75);
        mc_nym_manager_gridlayout->addWidget(mc_nym_manager_tableview, 1,0, 1,1);
        
        
        /** Second column in nym manager grid (right side) **/
        //Vertical box (contains: add/remove buttons)
        mc_nym_manager_addremove_btngroup_holder = new QWidget(0);
        mc_nym_manager_addremove_btngroup_vbox = new QVBoxLayout(0);
        mc_nym_manager_addremove_btngroup_holder->setLayout(mc_nym_manager_addremove_btngroup_vbox);
        mc_nym_manager_addremove_btngroup_vbox->setAlignment(Qt::AlignTop);
        mc_nym_manager_gridlayout->addWidget(mc_nym_manager_addremove_btngroup_holder, 1,1, 1,1);
        
        //"Add Nym" button
        mc_nym_manager_addremove_btngroup_addbtn = new QPushButton("Add Nym", 0);
        mc_nym_manager_addremove_btngroup_addbtn->setStyleSheet("QPushButton{padding:0.5em;}");
        mc_nym_manager_addremove_btngroup_vbox->addWidget(mc_nym_manager_addremove_btngroup_addbtn, Qt::AlignTop);
        //Connect the add nym button with a re-action to it being "clicked"
        connect(mc_nym_manager_addremove_btngroup_addbtn, SIGNAL(clicked()), this, SLOT(mc_nymmanager_addnym_slot()));
        
        //"Remove Nym" button
        mc_nym_manager_addremove_btngroup_removebtn = new QPushButton("Remove Nym", 0);
        mc_nym_manager_addremove_btngroup_removebtn->setStyleSheet("QPushButton{padding:0.5em;}");
        mc_nym_manager_addremove_btngroup_vbox->addWidget(mc_nym_manager_addremove_btngroup_removebtn, Qt::AlignTop);
        //Connect the remove nym button with a re-action to it being "clicked"
        connect(mc_nym_manager_addremove_btngroup_removebtn, SIGNAL(clicked()), this, SLOT(mc_nymmanager_removenym_slot()));
        
        /** Third column (Most revent error) **/
        //Label
        mc_nym_manager_most_recent_erorr = new QLabel("");
        mc_nym_manager_gridlayout->addWidget(mc_nym_manager_most_recent_erorr, 2,0, 1,2, Qt::AlignHCenter);
        /** Flag as init **/
        mc_nymmanager_already_init = 1;
    }
    /** ***
     ** Resize & Show
     **/
    mc_nym_manager_dialog->resize(600, 300);
    mc_nym_manager_dialog->show();
    
    /** ***
     ** Data Refresh/Fill Logic
     **/
    //Refresh Nym manger list
    /** Flag Refreshing Nym Manger **/
    mc_nymmanager_refreshing = 1;
    
    //remove all rows from the nym manager (so we can refresh any newly changed data)
    mc_nym_manager_tableview_itemmodel->removeRows(0, mc_nym_manager_tableview_itemmodel->rowCount());
    
    //Refresh the nym manager
    //Refresh nym list (can't be done, there is a glitch where if you open the nym manger dialog twice it does wierd things to the systray for nym menus )
    //mc_systrayMenu_reload_nymlist();
    
    //Add nym id and names to the manager list
    int total_nym_accounts = nym_list_id->size();
    qDebug() << "total: " << total_nym_accounts;
    int row_index = 0;
    for(int a = 0; a < total_nym_accounts; a++){
        //Add nym account name and id to the list.
        
        
        //Extract stuff for this row
        QString nym_id = nym_list_id->at(a).toString();
        QString nym_name = nym_list_name->at(a).toString();
        qDebug() << "ADDING NYM ID: " << nym_id;
        //Place extracted data into the table view
        QStandardItem * col_one = new QStandardItem(nym_name);
        QStandardItem * col_two = new QStandardItem(nym_id);
        //Column two is uneditable
        col_two->setEditable(0);
        
        QStandardItem * col_three = new QStandardItem();
        //Column three is a checkmark, we need to set some options in this case.
        col_three->setCheckable(1);
        
        //If this is the default pseudonym; if yes, mark as checked
        if(default_nym_id == nym_id){
            col_three->setCheckState(Qt::Checked);
        }
        
        mc_nym_manager_tableview_itemmodel->setItem(row_index, 0, col_one);
        mc_nym_manager_tableview_itemmodel->setItem(row_index, 1, col_two);
        mc_nym_manager_tableview_itemmodel->setItem(row_index, 2, col_three);
        
        //Increment index
        row_index += 1;
    }
    /** Unflag as current refreshing **/
    mc_nymmanager_refreshing = 0;
}


//This was mistakenly named nym_load_nym, should be set default nym
//Set Default Nym
void Moneychanger::mc_systrayMenu_nym_setDefaultNym(QString nym_id, QString nym_name){
    //Set default nym internal memory
    default_nym_id = nym_id;
    default_nym_name = nym_name;
    
    //SQL UPDATE default nym
    QSqlQuery update_default_nym(addressbook_db);
    update_default_nym.exec(QString("UPDATE `default_nym` SET `nym` = '%1'").arg(nym_id));
    
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
// ----------------------------------------------------------------------
/** Asset Manager Dialog **/
void Moneychanger::mc_assetmanager_dialog(){
    
    /** If the asset manager dialog has already been init,
     *  just show it, Other wise, init and show if this is the
     *  first time.
     **/
    if(mc_assetmanager_already_init == 0){
        
        //The asset Manager has not been init yet; Init, then show it.
        mc_asset_manager_dialog = new QDialog(0);
        
        /** window properties **/
        //Set window title
        mc_asset_manager_dialog->setWindowTitle("Asset Contracts | Moneychanger");
        
        //Set window on top
        mc_asset_manager_dialog->setWindowFlags(Qt::WindowStaysOnTopHint);
        
        /** layout and content **/
        //Grid layout
        mc_asset_manager_gridlayout = new QGridLayout(0);
        mc_asset_manager_dialog->setLayout(mc_asset_manager_gridlayout);
        
        /* First Row in asset manager Grid */
        //Label (header)
        mc_asset_manager_label = new QLabel("<h3>Asset Contracts</h3>", 0);
        mc_asset_manager_label->setAlignment(Qt::AlignRight);
        mc_asset_manager_gridlayout->addWidget(mc_asset_manager_label, 0,0, 1,2, Qt::AlignRight);
        
        
        /* Second Row in asset manager Grid */
        /** First column in address book grid (left side) **/
        //Horizontal box (contains: List of assets, add/remove buttons)
        mc_asset_manager_holder = new QWidget(0);
        mc_asset_manager_hbox = new QHBoxLayout(0);
        mc_asset_manager_holder->setLayout(mc_asset_manager_hbox);
        mc_asset_manager_gridlayout->addWidget(mc_asset_manager_holder);
        
        //Table View (backend and visual init)
        mc_asset_manager_tableview_itemmodel = new QStandardItemModel(0,3,0);
        mc_asset_manager_tableview_itemmodel->setHorizontalHeaderItem(0, new QStandardItem(QString("Asset Display Name")));
        mc_asset_manager_tableview_itemmodel->setHorizontalHeaderItem(1, new QStandardItem(QString("Asset ID")));
        mc_asset_manager_tableview_itemmodel->setHorizontalHeaderItem(2, new QStandardItem(QString("Default")));
        
        //Connect tableviews' backend "dataChanged" signal to a re-action.
        connect(mc_asset_manager_tableview_itemmodel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(mc_assetmanager_dataChanged_slot(QModelIndex,QModelIndex)));
        
        mc_asset_manager_tableview = new QTableView(0);
        mc_asset_manager_tableview->setSelectionMode(QAbstractItemView::SingleSelection);
        mc_asset_manager_tableview->setModel(mc_asset_manager_tableview_itemmodel);
        mc_asset_manager_tableview->setColumnWidth(0, 175);
        mc_asset_manager_tableview->setColumnWidth(1, 150);
        mc_asset_manager_tableview->setColumnWidth(2, 75);
        mc_asset_manager_gridlayout->addWidget(mc_asset_manager_tableview, 1,0, 1,1);
        
        
        /** Second column in asset manager grid (right side) **/
        //Vertical box (contains: add/remove buttons)
        mc_asset_manager_addremove_btngroup_holder = new QWidget(0);
        mc_asset_manager_addremove_btngroup_vbox = new QVBoxLayout(0);
        mc_asset_manager_addremove_btngroup_holder->setLayout(mc_asset_manager_addremove_btngroup_vbox);
        mc_asset_manager_addremove_btngroup_vbox->setAlignment(Qt::AlignTop);
        mc_asset_manager_gridlayout->addWidget(mc_asset_manager_addremove_btngroup_holder, 1,1, 1,1);
        
        //"Add asset" button
        mc_asset_manager_addremove_btngroup_addbtn = new QPushButton("Add Asset Contract", 0);
        mc_asset_manager_addremove_btngroup_addbtn->setStyleSheet("QPushButton{padding:0.5em;}");
        mc_asset_manager_addremove_btngroup_vbox->addWidget(mc_asset_manager_addremove_btngroup_addbtn, Qt::AlignTop);
        //Connect the add asset button with a re-action to it being "clicked"
        connect(mc_asset_manager_addremove_btngroup_addbtn, SIGNAL(clicked()), this, SLOT(mc_assetmanager_addasset_slot()));
        
        //"Remove asset" button
        mc_asset_manager_addremove_btngroup_removebtn = new QPushButton("Remove Asset Contract", 0);
        mc_asset_manager_addremove_btngroup_removebtn->setStyleSheet("QPushButton{padding:0.5em;}");
        mc_asset_manager_addremove_btngroup_vbox->addWidget(mc_asset_manager_addremove_btngroup_removebtn, Qt::AlignTop);
        //Connect the remove asset button with a re-action to it being "clicked"
        connect(mc_asset_manager_addremove_btngroup_removebtn, SIGNAL(clicked()), this, SLOT(mc_assetmanager_removeasset_slot()));
        
        /** Third column (Most revent error) **/
        //Label
        mc_asset_manager_most_recent_erorr = new QLabel("");
        mc_asset_manager_gridlayout->addWidget(mc_asset_manager_most_recent_erorr, 2,0, 1,2, Qt::AlignHCenter);
        /** Flag as init **/
        mc_assetmanager_already_init = 1;
    }
    /** ***
     ** Resize & Show
     **/
    mc_asset_manager_dialog->resize(600, 300);
    mc_asset_manager_dialog->show();
    
    /** ***
     ** Data Refresh/Fill Logic
     **/
    //Refresh asset manger list
    /** Flag Refreshing asset Manger **/
    mc_assetmanager_refreshing = 1;
    
    //remove all rows from the asset manager (so we can refresh any newly changed data)
    mc_asset_manager_tableview_itemmodel->removeRows(0, mc_asset_manager_tableview_itemmodel->rowCount());
    
    //Refresh the asset manager
    //Refresh asset list (can't be done, there is a glitch where if you open the asset manger dialog twice it does wierd things to the systray for asset menus )
    //mc_systrayMenu_reload_assetlist();
    
    //Add asset id and names to the manager list
    int total_assets = asset_list_id->size();
    qDebug() << "total: " << total_assets;
    int row_index = 0;
    for(int a = 0; a < total_assets; a++){
        //Add asset account name and id to the list.
        
        
        //Extract stuff for this row
        QString asset_id = asset_list_id->at(a).toString();
        QString asset_name = asset_list_name->at(a).toString();
        qDebug() << "ADDING asset ID: " << asset_id;
        //Place extracted data into the table view
        QStandardItem * col_one = new QStandardItem(asset_name);
        QStandardItem * col_two = new QStandardItem(asset_id);
        //Column two is uneditable
        col_two->setEditable(0);
        
        QStandardItem * col_three = new QStandardItem();
        //Column three is a checkmark, we need to set some options in this case.
        col_three->setCheckable(1);
        
        //If this is the default asset type; if yes, mark as checked
        if(default_asset_id == asset_id){
            col_three->setCheckState(Qt::Checked);
        }
        
        mc_asset_manager_tableview_itemmodel->setItem(row_index, 0, col_one);
        mc_asset_manager_tableview_itemmodel->setItem(row_index, 1, col_two);
        mc_asset_manager_tableview_itemmodel->setItem(row_index, 2, col_three);
        
        //Increment index
        row_index += 1;
    }
    /** Unflag as current refreshing **/
    mc_assetmanager_refreshing = 0;
}


//This was mistakenly named asset_load_asset, should be set default asset
//Set Default asset
void Moneychanger::mc_systrayMenu_asset_setDefaultAsset(QString asset_id, QString asset_name){
    //Set default asset internal memory
    default_asset_id = asset_id;
    default_asset_name = asset_name;
    
    //SQL UPDATE default asset
    QSqlQuery update_default_asset(addressbook_db);
    update_default_asset.exec(QString("UPDATE `default_asset` SET `asset` = '%1'").arg(asset_id));
    
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
// ----------------------------------------------------------------------
/** account Manager Dialog **/
void Moneychanger::mc_accountmanager_dialog(){
    
    /** If the account manager dialog has already been init,
     *  just show it, Other wise, init and show if this is the
     *  first time.
     **/
    if(mc_accountmanager_already_init == 0){
        
        //The account Manager has not been init yet; Init, then show it.
        mc_account_manager_dialog = new QDialog(0);
        
        /** window properties **/
        //Set window title
        mc_account_manager_dialog->setWindowTitle("Account Manager | Moneychanger");
        
        //Set window on top
        mc_account_manager_dialog->setWindowFlags(Qt::WindowStaysOnTopHint);
        
        /** layout and content **/
        //Grid layout
        mc_account_manager_gridlayout = new QGridLayout(0);
        mc_account_manager_dialog->setLayout(mc_account_manager_gridlayout);
        
        /* First Row in account manager Grid */
        //Label (header)
        mc_account_manager_label = new QLabel("<h3>Account Manager</h3>", 0);
        mc_account_manager_label->setAlignment(Qt::AlignRight);
        mc_account_manager_gridlayout->addWidget(mc_account_manager_label, 0,0, 1,2, Qt::AlignRight);
        
        
        /* Second Row in account manager Grid */
        /** First column in address book grid (left side) **/
        //Horizontal box (contains: List of accounts, add/remove buttons)
        mc_account_manager_holder = new QWidget(0);
        mc_account_manager_hbox = new QHBoxLayout(0);
        mc_account_manager_holder->setLayout(mc_account_manager_hbox);
        mc_account_manager_gridlayout->addWidget(mc_account_manager_holder);
        
        //Table View (backend and visual init)
        mc_account_manager_tableview_itemmodel = new QStandardItemModel(0,3,0);
        mc_account_manager_tableview_itemmodel->setHorizontalHeaderItem(0, new QStandardItem(QString("Account Display Name")));
        mc_account_manager_tableview_itemmodel->setHorizontalHeaderItem(1, new QStandardItem(QString("Account ID")));
        mc_account_manager_tableview_itemmodel->setHorizontalHeaderItem(2, new QStandardItem(QString("Default")));
        
        //Connect tableviews' backend "dataChanged" signal to a re-action.
        connect(mc_account_manager_tableview_itemmodel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(mc_accountmanager_dataChanged_slot(QModelIndex,QModelIndex)));
        
        mc_account_manager_tableview = new QTableView(0);
        mc_account_manager_tableview->setSelectionMode(QAbstractItemView::SingleSelection);
        mc_account_manager_tableview->setModel(mc_account_manager_tableview_itemmodel);
        mc_account_manager_tableview->setColumnWidth(0, 175);
        mc_account_manager_tableview->setColumnWidth(1, 150);
        mc_account_manager_tableview->setColumnWidth(2, 75);
        mc_account_manager_gridlayout->addWidget(mc_account_manager_tableview, 1,0, 1,1);
        
        
        /** Second column in account manager grid (right side) **/
        //Vertical box (contains: add/remove buttons)
        mc_account_manager_addremove_btngroup_holder = new QWidget(0);
        mc_account_manager_addremove_btngroup_vbox = new QVBoxLayout(0);
        mc_account_manager_addremove_btngroup_holder->setLayout(mc_account_manager_addremove_btngroup_vbox);
        mc_account_manager_addremove_btngroup_vbox->setAlignment(Qt::AlignTop);
        mc_account_manager_gridlayout->addWidget(mc_account_manager_addremove_btngroup_holder, 1,1, 1,1);
        
        //"Add account" button
        mc_account_manager_addremove_btngroup_addbtn = new QPushButton("Add Account", 0);
        mc_account_manager_addremove_btngroup_addbtn->setStyleSheet("QPushButton{padding:0.5em;}");
        mc_account_manager_addremove_btngroup_vbox->addWidget(mc_account_manager_addremove_btngroup_addbtn, Qt::AlignTop);
        //Connect the add account button with a re-action to it being "clicked"
        connect(mc_account_manager_addremove_btngroup_addbtn, SIGNAL(clicked()), this, SLOT(mc_accountmanager_addaccount_slot()));
        
        //"Remove account" button
        mc_account_manager_addremove_btngroup_removebtn = new QPushButton("Remove Account", 0);
        mc_account_manager_addremove_btngroup_removebtn->setStyleSheet("QPushButton{padding:0.5em;}");
        mc_account_manager_addremove_btngroup_vbox->addWidget(mc_account_manager_addremove_btngroup_removebtn, Qt::AlignTop);
        //Connect the remove account button with a re-action to it being "clicked"
        connect(mc_account_manager_addremove_btngroup_removebtn, SIGNAL(clicked()), this, SLOT(mc_accountmanager_removeaccount_slot()));
        
        /** Third column (Most revent error) **/
        //Label
        mc_account_manager_most_recent_erorr = new QLabel("");
        mc_account_manager_gridlayout->addWidget(mc_account_manager_most_recent_erorr, 2,0, 1,2, Qt::AlignHCenter);
        /** Flag as init **/
        mc_accountmanager_already_init = 1;
    }
    /** ***
     ** Resize & Show
     **/
    mc_account_manager_dialog->resize(600, 300);
    mc_account_manager_dialog->show();
    
    /** ***
     ** Data Refresh/Fill Logic
     **/
    //Refresh account manger list
    /** Flag Refreshing account Manger **/
    mc_accountmanager_refreshing = 1;
    
    //remove all rows from the account manager (so we can refresh any newly changed data)
    mc_account_manager_tableview_itemmodel->removeRows(0, mc_account_manager_tableview_itemmodel->rowCount());
    
    //Refresh the account manager
    //Refresh account list (can't be done, there is a glitch where if you open the account manger dialog twice it does wierd things to the systray for account menus )
    //mc_systrayMenu_reload_accountlist();
    
    //Add account id and names to the manager list
    int total_accounts = account_list_id->size();
    qDebug() << "total: " << total_accounts;
    int row_index = 0;
    for(int a = 0; a < total_accounts; a++){
        //Add account name and id to the list.
        
        
        //Extract stuff for this row
        QString account_id = account_list_id->at(a).toString();
        QString account_name = account_list_name->at(a).toString();
        qDebug() << "ADDING account ID: " << account_id;
        //Place extracted data into the table view
        QStandardItem * col_one = new QStandardItem(account_name);
        QStandardItem * col_two = new QStandardItem(account_id);
        //Column two is uneditable
        col_two->setEditable(0);
        
        QStandardItem * col_three = new QStandardItem();
        //Column three is a checkmark, we need to set some options in this case.
        col_three->setCheckable(1);
        
        //If this is the default account; if yes, mark as checked
        if(default_account_id == account_id){
            col_three->setCheckState(Qt::Checked);
        }
        
        mc_account_manager_tableview_itemmodel->setItem(row_index, 0, col_one);
        mc_account_manager_tableview_itemmodel->setItem(row_index, 1, col_two);
        mc_account_manager_tableview_itemmodel->setItem(row_index, 2, col_three);
        
        //Increment index
        row_index += 1;
    }
    /** Unflag as current refreshing **/
    mc_accountmanager_refreshing = 0;
}


//This was mistakenly named account_load_account, should be set default account
//Set Default account
void Moneychanger::mc_systrayMenu_account_setDefaultAccount(QString account_id, QString account_name){
    //Set default account internal memory
    default_account_id = account_id;
    default_account_name = account_name;
    
    //SQL UPDATE default account
    QSqlQuery update_default_account(addressbook_db);
    update_default_account.exec(QString("UPDATE `default_account` SET `account` = '%1'").arg(account_id));
    
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
// ----------------------------------------------------------------------
/** Server **/
/** *********************************************
 * @brief Moneychanger::mc_servermanager_dialog
 * @info Will init & show the server list manager
 ** *********************************************/
void Moneychanger::mc_servermanager_dialog(){
    /** If the server list manager dialog has already been init,
     *  just show it, Other wise, init and show if this is the
     *  first time.
     **/
    if(mc_servermanager_already_init == 0){
        //Init
        mc_server_manager_dialog = new QDialog(0);
        mc_server_manager_dialog->setWindowTitle("Server Manager | Moneychanger");
        mc_server_manager_gridlayout = new QGridLayout(0);
        mc_server_manager_gridlayout->setColumnStretch(1, 0);
        mc_server_manager_dialog->setLayout(mc_server_manager_gridlayout);
        
        /** First Row (Takes up two columns) **/
        //Header (Server List Manager)
        mc_server_manager_label = new QLabel("<h2>Server Manager</h2>");
        mc_server_manager_gridlayout->addWidget(mc_server_manager_label, 0,0, 1,2, Qt::AlignRight);
        
        /** Second Row **/
        //Column One
        //Tableview (Server List)
        mc_server_manager_tableview_itemmodel = new QStandardItemModel(0);
        mc_server_manager_tableview = new QTableView(0);
        mc_server_manager_tableview->setSelectionMode(QAbstractItemView::SingleSelection);
        mc_server_manager_tableview->setModel(mc_server_manager_tableview_itemmodel);
        
        mc_server_manager_tableview_itemmodel->setColumnCount(3);
        mc_server_manager_tableview_itemmodel->setHorizontalHeaderItem(0, new QStandardItem(QString("Display Name")));
        mc_server_manager_tableview_itemmodel->setHorizontalHeaderItem(1, new QStandardItem(QString("Server ID")));
        mc_server_manager_tableview_itemmodel->setHorizontalHeaderItem(2, new QStandardItem(QString("Default")));
        
        //Add to grid
        mc_server_manager_gridlayout->addWidget(mc_server_manager_tableview, 1,0, 1,1);
        
        //Column Two
        mc_server_manager_addremove_btngroup_removebtn = new QPushButton("Remove Server", 0);
        mc_server_manager_addremove_btngroup_removebtn->setStyleSheet("QPushButton{padding:0.5em;}");
        //Make a "click" reaction to the remove server button
        connect(mc_server_manager_addremove_btngroup_removebtn, SIGNAL(clicked()), this, SLOT(mc_server_manager_request_remove_server_slot()));
        
        //Add to grid
        mc_server_manager_gridlayout->addWidget(mc_server_manager_addremove_btngroup_removebtn, 1,1, 1,1, Qt::AlignTop);
        
        /** Flag already int **/
        mc_servermanager_already_init = 1;
    }
    
    //Resize
    mc_server_manager_dialog->resize(500,300);
    //Show
    mc_server_manager_dialog->show();
    
    /**
     ** Refresh server list data
     **/
    
    //Remove all servers in the list
    mc_server_manager_tableview_itemmodel->removeRows(0, mc_server_manager_tableview_itemmodel->rowCount(), QModelIndex());
    
    //Add/Append/Refresh server list.
    int row_index = 0;
    int32_t serverlist_count = OTAPI_Wrap::GetServerCount();
    for(int a = 0; a < serverlist_count; a++){
        std::string server_id =  OTAPI_Wrap::GetServer_ID(a);
        std::string server_name = OTAPI_Wrap::GetServer_Name(server_id);
        
        //Extract data
        QString server_name_string = QString::fromStdString(server_name);
        QString server_id_string = QString::fromStdString(server_id);
        
        //Place extracted data into the table view
        QStandardItem * col_one = new QStandardItem(server_name_string);
        QStandardItem * col_two = new QStandardItem(server_id_string);
        QStandardItem * col_three = new QStandardItem(0);
        //Set as checkbox
        col_three->setCheckable(1);
        
        //Check if this is the default server; If it is, then mark it as "Checked"
        if(server_id_string == default_server_id){
            col_three->setCheckState(Qt::Checked);
        }
        mc_server_manager_tableview_itemmodel->setItem(row_index, 0, col_one);
        mc_server_manager_tableview_itemmodel->setItem(row_index, 1, col_two);
        mc_server_manager_tableview_itemmodel->setItem(row_index, 2, col_three);
    }
    
    
}

void Moneychanger::mc_systrayMenu_server_setDefaultServer(QString server_id, QString server_name){
    //Set default server internal memory
    default_server_id = server_id;
    default_server_name = server_name;
    
    qDebug() << default_server_id;
    qDebug() << default_server_name;
    
    //SQL UPDATE default server
    QSqlQuery update_default_server(addressbook_db);
    update_default_server.exec(QString("UPDATE `default_server` SET `server` = '%1'").arg(server_id));
    
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


/** Withdraw **/
//As Cash
void Moneychanger::mc_withdraw_ascash_dialog(){
    
    
    /** If the withdraw as cash dialog has already been init,
     *  just show it, Other wise, init and show if this is the
     *  first time.
     **/
    if(mc_withdraw_ascash_dialog_already_init == 0){
        //Init, then show
        //Init
        mc_systrayMenu_withdraw_ascash_dialog = new QDialog(0);
        /** window properties **/
        //Set window title
        mc_systrayMenu_withdraw_ascash_dialog->setWindowTitle("Withdraw as Cash | Moneychanger");
        mc_systrayMenu_withdraw_ascash_dialog->setWindowFlags(Qt::WindowStaysOnTopHint);
        
        /** layout and content **/
        //Grid layout
        mc_systrayMenu_withdraw_ascash_gridlayout = new QGridLayout(0);
        mc_systrayMenu_withdraw_ascash_dialog->setLayout(mc_systrayMenu_withdraw_ascash_gridlayout);
        
        //Withdraw As Cash (header label)
        mc_systrayMenu_withdraw_ascash_header_label = new QLabel("<h3>Withdraw as Cash</h3>", 0);
        mc_systrayMenu_withdraw_ascash_header_label->setAlignment(Qt::AlignRight);
        mc_systrayMenu_withdraw_ascash_gridlayout->addWidget(mc_systrayMenu_withdraw_ascash_header_label, 0, 0, 1, 1);
        
        //Account ID (label) Note: Value is set when the dropdown box is selected and/or highlighted
        mc_systrayMenu_withdraw_ascash_accountid_label = new QLabel("", 0);
        mc_systrayMenu_withdraw_ascash_accountid_label->setStyleSheet("QLabel{padding:0.5em;}");
        mc_systrayMenu_withdraw_ascash_accountid_label->setAlignment(Qt::AlignHCenter);
        mc_systrayMenu_withdraw_ascash_gridlayout->addWidget(mc_systrayMenu_withdraw_ascash_accountid_label, 1, 0, 1, 1);
        
        //Account Dropdown (combobox)
        mc_systrayMenu_withdraw_ascash_account_dropdown = new QComboBox(0);
        mc_systrayMenu_withdraw_ascash_account_dropdown->setStyleSheet("QComboBox{padding:0.5em;}");
        mc_systrayMenu_withdraw_ascash_gridlayout->addWidget(mc_systrayMenu_withdraw_ascash_account_dropdown, 2, 0, 1, 1);
        
        //Make connection to "hovering over items" to showing their IDs above the combobox (for user clarity and backend id indexing)
        connect(mc_systrayMenu_withdraw_ascash_account_dropdown, SIGNAL(highlighted(int)), this, SLOT(mc_withdraw_ascash_account_dropdown_highlighted_slot(int)));
        
        //Amount Instructions
        //TODO ^^
        
        //Amount Input
        mc_systrayMenu_withdraw_ascash_amount_input = new QLineEdit;
        mc_systrayMenu_withdraw_ascash_amount_input->setPlaceholderText("Amount");
        mc_systrayMenu_withdraw_ascash_amount_input->setStyleSheet("QLineEdit{padding:0.5em;}");
        mc_systrayMenu_withdraw_ascash_gridlayout->addWidget(mc_systrayMenu_withdraw_ascash_amount_input, 3, 0, 1, 1);
        
        //Withdraw Button
        mc_systrayMenu_withdraw_ascash_button = new QPushButton("Withdraw as Cash");
        mc_systrayMenu_withdraw_ascash_button->setStyleSheet("QPushButton{padding:0.5em;}");
        mc_systrayMenu_withdraw_ascash_gridlayout->addWidget(mc_systrayMenu_withdraw_ascash_button, 4, 0, 1, 1);
        //Connect button with re-action
        connect(mc_systrayMenu_withdraw_ascash_button, SIGNAL(pressed()), this, SLOT(mc_withdraw_ascash_confirm_amount_dialog_slot()));
        
        /** Flag already init **/
        mc_withdraw_ascash_dialog_already_init = 1;
    }
    //Resize
    mc_systrayMenu_withdraw_ascash_dialog->resize(400, 120);
    //Show
    mc_systrayMenu_withdraw_ascash_dialog->show();
    mc_systrayMenu_withdraw_ascash_dialog->activateWindow();
    
    /** Refresh dynamic lists **/
    //remove all items from nym dropdown box
    while (mc_systrayMenu_withdraw_ascash_account_dropdown->count() > 0)
        mc_systrayMenu_withdraw_ascash_account_dropdown->removeItem(0);
    
    for(int a = 0; a < nym_list_id->size(); a++){
        //Add to combobox
        //Get OT Account ID
        mc_systrayMenu_withdraw_ascash_account_dropdown->addItem(account_list_name->at(a).toString(), account_list_id->at(a).toString());
    }
}

//As Voucher
void Moneychanger::mc_withdraw_asvoucher_dialog(){
    
    
    /** If the withdraw as voucher dialog has already been init,
     *  just show it, Other wise, init and show if this is the
     *  first time.
     **/
    if(mc_withdraw_asvoucher_dialog_already_init == 0){
        //Init, then show
        //Init
        mc_systrayMenu_withdraw_asvoucher_dialog = new QDialog(0);
        /** window properties **/
        //Set window title
        mc_systrayMenu_withdraw_asvoucher_dialog->setWindowTitle("Withdraw as Voucher | Moneychanger");
        mc_systrayMenu_withdraw_asvoucher_dialog->setWindowFlags(Qt::WindowStaysOnTopHint);
        
        /** layout and content **/
        //Grid layout Input
        mc_systrayMenu_withdraw_asvoucher_gridlayout = new QGridLayout(0);
        mc_systrayMenu_withdraw_asvoucher_dialog->setLayout(mc_systrayMenu_withdraw_asvoucher_gridlayout);
        
        //Label (withdraw as voucher)
        mc_systrayMenu_withdraw_asvoucher_header_label = new QLabel("<h3>Withdraw as Voucher</h3>", 0);
        mc_systrayMenu_withdraw_asvoucher_header_label->setAlignment(Qt::AlignRight);
        mc_systrayMenu_withdraw_asvoucher_gridlayout->addWidget(mc_systrayMenu_withdraw_asvoucher_header_label, 0,0, 1,1);
        
        //Account ID (label) Note: Value is set when the dropdown box is selected and/or highlighted
        mc_systrayMenu_withdraw_asvoucher_accountid_label = new QLabel("", 0);
        mc_systrayMenu_withdraw_asvoucher_accountid_label->setAlignment(Qt::AlignHCenter);
        mc_systrayMenu_withdraw_asvoucher_gridlayout->addWidget(mc_systrayMenu_withdraw_asvoucher_accountid_label, 1,0, 1,1);
        
        //Account Dropdown (combobox)
        mc_systrayMenu_withdraw_asvoucher_account_dropdown = new QComboBox(0);
        
        mc_systrayMenu_withdraw_asvoucher_account_dropdown->setStyleSheet("QComboBox{padding:0.5em;}");
        mc_systrayMenu_withdraw_asvoucher_gridlayout->addWidget(mc_systrayMenu_withdraw_asvoucher_account_dropdown, 2,0, 1,1);
        
        //Make connection to "hovering over items" to showing their IDs above the combobox (for user clarity and backend id indexing)
        connect(mc_systrayMenu_withdraw_asvoucher_account_dropdown, SIGNAL(highlighted(int)), this, SLOT(mc_withdraw_asvoucher_account_dropdown_highlighted_slot(int)));
        
        //To Nym ID
        //Horizontal Box (to hold Nym Id input/Address Box Icon/QR Code Scanner Icon)
        mc_systrayMenu_withdraw_asvoucher_nym_holder = new QWidget(0);
        mc_systrayMenu_withdraw_asvoucher_nym_hbox = new QHBoxLayout(0);
        mc_systrayMenu_withdraw_asvoucher_nym_hbox->setMargin(0);
        mc_systrayMenu_withdraw_asvoucher_nym_holder->setLayout(mc_systrayMenu_withdraw_asvoucher_nym_hbox);
        mc_systrayMenu_withdraw_asvoucher_gridlayout->addWidget(mc_systrayMenu_withdraw_asvoucher_nym_holder, 3,0, 1,1);
        
        //Nym ID (Paste input)
        mc_systrayMenu_withdraw_asvoucher_nym_input = new QLineEdit;
        mc_systrayMenu_withdraw_asvoucher_nym_input->setPlaceholderText("Recipient Nym Id");
        mc_systrayMenu_withdraw_asvoucher_nym_input->setStyleSheet("QLineEdit{padding:0.5em;}");
        mc_systrayMenu_withdraw_asvoucher_nym_hbox->addWidget(mc_systrayMenu_withdraw_asvoucher_nym_input);
        
        
        //Address Book (button)
        mc_systrayMenu_withdraw_asvoucher_nym_addressbook_icon = QIcon(":/icons/addressbook");
        mc_systrayMenu_withdraw_asvoucher_nym_addressbook_btn = new QPushButton(mc_systrayMenu_withdraw_asvoucher_nym_addressbook_icon, "", 0);
        mc_systrayMenu_withdraw_asvoucher_nym_addressbook_btn->setStyleSheet("QPushButton{padding:0.5em;}");
        mc_systrayMenu_withdraw_asvoucher_nym_hbox->addWidget(mc_systrayMenu_withdraw_asvoucher_nym_addressbook_btn);
        //Connect Address book button with a re-action
        connect(mc_systrayMenu_withdraw_asvoucher_nym_addressbook_btn, SIGNAL(clicked()), this, SLOT(mc_withdraw_asvoucher_show_addressbook_slot()));
        
        //QR Code scanner (button)
        //TO DO^^
        
        
        //Amount input
        mc_systrayMenu_withdraw_asvoucher_amount_input = new QLineEdit;
        mc_systrayMenu_withdraw_asvoucher_amount_input->setPlaceholderText("Amount as Integer");
        mc_systrayMenu_withdraw_asvoucher_amount_input->setStyleSheet("QLineEdit{padding:0.5em;}");
        mc_systrayMenu_withdraw_asvoucher_gridlayout->addWidget(mc_systrayMenu_withdraw_asvoucher_amount_input, 4,0, 1,1);
        
        //Memo input box
        mc_systrayMenu_withdraw_asvoucher_memo_input = new QTextEdit("Memo", 0);
        mc_systrayMenu_withdraw_asvoucher_gridlayout->addWidget(mc_systrayMenu_withdraw_asvoucher_memo_input, 5,0, 1,1);
        
        //Withdraw Button
        mc_systrayMenu_withdraw_asvoucher_button = new QPushButton("Withdraw as Voucher");
        mc_systrayMenu_withdraw_asvoucher_button->setStyleSheet("QPushButton{padding:1em;}");
        mc_systrayMenu_withdraw_asvoucher_gridlayout->addWidget(mc_systrayMenu_withdraw_asvoucher_button, 6,0, 1,1);
        //Connect button with re-action
        connect(mc_systrayMenu_withdraw_asvoucher_button, SIGNAL(clicked()), this, SLOT(mc_withdraw_asvoucher_confirm_amount_dialog_slot()));
        
        /** Flag as init **/
        mc_withdraw_asvoucher_dialog_already_init = 1;
    }
    //Resize & Show
    mc_systrayMenu_withdraw_asvoucher_dialog->resize(400, 120);
    mc_systrayMenu_withdraw_asvoucher_dialog->show();
    mc_systrayMenu_withdraw_asvoucher_dialog->setFocus();
    
    /** Refresh dynamic lists **/
    //remove all items from nym dropdown box
    while (mc_systrayMenu_withdraw_asvoucher_account_dropdown->count() > 0)
        mc_systrayMenu_withdraw_asvoucher_account_dropdown->removeItem(0);
    
    for(int a = 0; a < nym_list_id->size(); a++){
        //Add to combobox
        //Get OT Account ID
        mc_systrayMenu_withdraw_asvoucher_account_dropdown->addItem(account_list_name->at(a).toString(), account_list_id->at(a).toString());
    }
}



/** ****** ****** ****** **
 ** Private Slots        **/

// ---------------------------------------------------------
/* Nym Slots */
void Moneychanger::mc_nymmanager_addnym_slot(){
    //Decide if we should init and show, or just show
    if(mc_nymmanager_addnym_dialog_already_init == 0){
        //Init, then show.
        mc_nym_manager_addnym_dialog = new QDialog(0);
        mc_nym_manager_addnym_dialog->setWindowTitle("Add Pseudonym | Moneychanger");
        mc_nym_manager_addnym_dialog->setModal(1);
        
        //Gridlayout
        mc_nym_manager_addnym_gridlayout = new QGridLayout(0);
        mc_nym_manager_addnym_dialog->setLayout(mc_nym_manager_addnym_gridlayout);
        
        //Label (header)
        mc_nym_manager_addnym_header = new QLabel("<h2>Add Pseudonym</h2>",0);
        mc_nym_manager_addnym_gridlayout->addWidget(mc_nym_manager_addnym_header, 0,0, 1,1, Qt::AlignRight);
        
        //Label (Show Advanced Option(s)) (Also a button/connection)
        mc_nym_manager_addnym_subheader_toggleadvanced_options_label = new QLabel("<a href='#'>Advanced Option(s)</a>", 0);
        mc_nym_manager_addnym_gridlayout->addWidget(mc_nym_manager_addnym_subheader_toggleadvanced_options_label, 1,0, 1,1, Qt::AlignRight);
        //Connect with a re-action
        connect(mc_nym_manager_addnym_subheader_toggleadvanced_options_label, SIGNAL(linkActivated(QString)), this, SLOT(mc_addnym_dialog_showadvanced_slot(QString)));
        
        
        //Label (instructions)
        mc_nym_manager_addnym_subheader_instructions = new QLabel("Below are some options that will help determine how your Pseudonym will be added. Selecting \"No-Source\" is for self-signed Pseudonyms.");
        mc_nym_manager_addnym_subheader_instructions->setWordWrap(1);
        mc_nym_manager_addnym_subheader_instructions->setStyleSheet("QLabel{padding:0.5em;}");
        mc_nym_manager_addnym_gridlayout->addWidget(mc_nym_manager_addnym_subheader_instructions, 2,0, 1,1);
        
        //Label (Choose Source Question)
        mc_nym_manager_addnym_choosesource_label = new QLabel("<h3>Choose the source of the Pseudonym</h3>");
        mc_nym_manager_addnym_choosesource_label->setStyleSheet("QLabel{padding:1em;}");
        mc_nym_manager_addnym_gridlayout->addWidget(mc_nym_manager_addnym_choosesource_label, 3,0, 1,1);
        
        //Combobox (Dropdown box: Choose Source)
        mc_nym_manager_addnym_choosesource_answer_selection = new QComboBox(0);
        mc_nym_manager_addnym_choosesource_answer_selection->addItem("Namecoin");
        mc_nym_manager_addnym_choosesource_answer_selection->addItem("No-Source");
        mc_nym_manager_addnym_choosesource_answer_selection->setCurrentIndex(1);
        mc_nym_manager_addnym_choosesource_answer_selection->setStyleSheet("QComboBox{padding:0.5em;}");
        mc_nym_manager_addnym_gridlayout->addWidget(mc_nym_manager_addnym_choosesource_answer_selection, 4,0, 1,1);
        
        //Create Nym (button)
        mc_nym_manager_addnym_create_nym_btn = new QPushButton("Create a new Pseudonym", 0);
        mc_nym_manager_addnym_create_nym_btn->setStyleSheet("QPushButton{padding:1em;}");
        mc_nym_manager_addnym_gridlayout->addWidget(mc_nym_manager_addnym_create_nym_btn, 5,0, 1,1, Qt::AlignHCenter);
        //Connect create nym button with a re-action;
        connect(mc_nym_manager_addnym_create_nym_btn, SIGNAL(clicked()), this, SLOT(mc_addnym_dialog_createnym_slot()));
        
        /** Flag as already init **/
        mc_nymmanager_addnym_dialog_already_init = 1;
    }
    //Resize
    mc_nym_manager_addnym_dialog->resize(400, 290);
    //Show
    mc_nym_manager_addnym_dialog->show();
}

void Moneychanger::mc_nymmanager_removenym_slot(){
    //Init, then show; If already init, then just show
    if(mc_nymmanager_removenym_dialog_already_init == 0){
        mc_nym_manager_removenym_dialog = new QDialog(0);
        mc_nym_manager_removenym_dialog->setWindowTitle("Remove Pseudonym | Moneychanger");
        mc_nym_manager_removenym_dialog->setModal(1);
        //Grid layout
        mc_nym_manager_removenym_gridlayout = new QGridLayout(0);
        mc_nym_manager_removenym_dialog->setLayout(mc_nym_manager_removenym_gridlayout);
    }
    mc_nym_manager_removenym_dialog->show();
}

void Moneychanger::mc_nymmanager_dataChanged_slot(QModelIndex topLeft, QModelIndex bottomRight){
    //Ignore triggers while "refreshing" the nym manager.
    if(mc_nymmanager_refreshing == 0 && mc_nymmanager_proccessing_dataChanged == 0){
        /** Flag Proccessing dataChanged **/
        mc_nymmanager_proccessing_dataChanged = 1;
        
        /** Proccess the "Display Name" column **/
        if(topLeft.column() == 0){
            //Get the value (as std::string) of the nym id
            QStandardItem * nym_id_item = mc_nym_manager_tableview_itemmodel->item(topLeft.row(), 1);
            QString nym_id_string = nym_id_item->text();
            std::string nym_id = nym_id_string.toStdString();
            
            //Get the value (as std::string) of the newly set name of the nym id
            QVariant new_nym_name_variant = topLeft.data();
            QString new_nym_name_string = new_nym_name_variant.toString();
            std::string new_nym_name = new_nym_name_string.toStdString();
            qDebug() << nym_id_string;
            //Update the newly set display name for this nym in OT ( call ot for updating )
            bool setName_successfull = OTAPI_Wrap::SetNym_Name(nym_id, nym_id, new_nym_name);
            if(setName_successfull == true){
                //Do nothing (There is nothing that needs to be done)
            }else{
                //Setting of the display name for this nym failed, revert value visually, display recent error
                mc_nym_manager_most_recent_erorr->setText("<span style='color:#A80000'><b>Renaming that nym failed. (Error Code: 100)</b></span>");
            }
        }
        
        /** Proccess the "Default" column (if triggered) **/
        if(topLeft.column() == 2){
            //The (default) 2 column has checked(or unchecked) a box, (uncheck all checkboxes, except the newly selected one)
            int total_checkboxes = mc_nym_manager_tableview_itemmodel->rowCount();
            
            //Uncheck all boxes (always checkMarked the triggered box)
            for(int a = 0; a < total_checkboxes; a++){
                
                //Check if this is a checkbox we should checkmark
                if(a != topLeft.row()){
                    
                    //Get checkbox item
                    QStandardItem * checkbox_model = mc_nym_manager_tableview_itemmodel->item(a, 2);
                    
                    //Update the checkbox item at the backend.
                    checkbox_model->setCheckState(Qt::Unchecked);
                    
                    //Update the checkbox item visually.
                    mc_nym_manager_tableview_itemmodel->setItem(a, 2, checkbox_model);
                }else if(a == topLeft.row()){
                    
                    //Get checkbox item
                    QStandardItem * checkbox_model = mc_nym_manager_tableview_itemmodel->item(a, 2);
                    
                    //Update the checkbox item at the backend.
                    //Get nym id we are targeting to update.
                    QStandardItem * nym_id = mc_nym_manager_tableview_itemmodel->item(a, 1);
                    QVariant nym_id_variant = nym_id->text();
                    QString nym_id_string = nym_id_variant.toString();
                    QString nym_name_string = QString::fromStdString(OTAPI_Wrap::GetNym_Name(nym_id_string.toStdString()));
                    //Update the checkbox item visually.
                    checkbox_model->setCheckState(Qt::Checked);
                    mc_nym_manager_tableview_itemmodel->setItem(a, 2, checkbox_model);
                    
                    //Update default nym at realtime memory backend
                    
                    mc_systrayMenu_nym_setDefaultNym(nym_id_string, nym_name_string);
                    
                    
                }
                
            }
            
        }
        
        /** Unflag Proccessing Data Changed **/
        mc_nymmanager_proccessing_dataChanged = 0;
    }
}


/**** ****
 **** Nym Manager -> Add Nym Dialog (Private Slots)
 **** ****/
void Moneychanger::mc_addnym_dialog_showadvanced_slot(QString link_href){
    //If advanced options are already showing, hide, if they are hidden, show them.
    if(mc_nymmanager_addnym_dialog_advanced_showing == 0){
        //Show advanced options.
        //Show the Bits option
        
    }else if(mc_nymmanager_addnym_dialog_advanced_showing == 1){
        //Hide advanced options.
        //Hide the Bits option
    }
}


void Moneychanger::mc_addnym_dialog_createnym_slot(){
    std::string pseudonym = OTAPI_Wrap::CreateNym(1024, "", "");
    QString new_pseudonym = QString::fromStdString(pseudonym);
    
    //Success if non null
    if(new_pseudonym != ""){
        
    }else{
        //Failed to create pseudonym
    }
}
// ---------------------------------------------------------
// ASSET TYPE
void Moneychanger::mc_assetmanager_addasset_slot(){
    //Decide if we should init and show, or just show
    if(mc_assetmanager_addasset_dialog_already_init == 0){
        //Init, then show.
        mc_asset_manager_addasset_dialog = new QDialog(0);
        mc_asset_manager_addasset_dialog->setWindowTitle("Add Asset Contract | Moneychanger");
        mc_asset_manager_addasset_dialog->setModal(1);
        
        //Gridlayout
        mc_asset_manager_addasset_gridlayout = new QGridLayout(0);
        mc_asset_manager_addasset_dialog->setLayout(mc_asset_manager_addasset_gridlayout);
        
        //Label (header)
        mc_asset_manager_addasset_header = new QLabel("<h2>Add Asset Contract</h2>",0);
        mc_asset_manager_addasset_gridlayout->addWidget(mc_asset_manager_addasset_header, 0,0, 1,1, Qt::AlignRight);
        
        //Label (Show Advanced Option(s)) (Also a button/connection)
        mc_asset_manager_addasset_subheader_toggleadvanced_options_label = new QLabel("<a href='#'>Advanced Option(s)</a>", 0);
        mc_asset_manager_addasset_gridlayout->addWidget(mc_asset_manager_addasset_subheader_toggleadvanced_options_label, 1,0, 1,1, Qt::AlignRight);
        //Connect with a re-action
        connect(mc_asset_manager_addasset_subheader_toggleadvanced_options_label, SIGNAL(linkActivated(QString)), this, SLOT(mc_addasset_dialog_showadvanced_slot(QString)));
        
        
        //Label (instructions)
        mc_asset_manager_addasset_subheader_instructions = new QLabel("Below are some options that will help determine how your Asset will be added.");
        mc_asset_manager_addasset_subheader_instructions->setWordWrap(1);
        mc_asset_manager_addasset_subheader_instructions->setStyleSheet("QLabel{padding:0.5em;}");
        mc_asset_manager_addasset_gridlayout->addWidget(mc_asset_manager_addasset_subheader_instructions, 2,0, 1,1);
        
        //Label (Choose Source Question)
        mc_asset_manager_addasset_choosesource_label = new QLabel("<h3>Enter the Asset Contract</h3>");
        mc_asset_manager_addasset_choosesource_label->setStyleSheet("QLabel{padding:1em;}");
        mc_asset_manager_addasset_gridlayout->addWidget(mc_asset_manager_addasset_choosesource_label, 3,0, 1,1);
        
        //Combobox (Dropdown box: Choose Source)
        mc_asset_manager_addasset_choosesource_answer_selection = new QComboBox(0);
        mc_asset_manager_addasset_choosesource_answer_selection->addItem("Namecoin");
        mc_asset_manager_addasset_choosesource_answer_selection->addItem("No-Source");
        mc_asset_manager_addasset_choosesource_answer_selection->setCurrentIndex(1);
        mc_asset_manager_addasset_choosesource_answer_selection->setStyleSheet("QComboBox{padding:0.5em;}");
        mc_asset_manager_addasset_gridlayout->addWidget(mc_asset_manager_addasset_choosesource_answer_selection, 4,0, 1,1);
        
        //Create asset (button)
        mc_asset_manager_addasset_create_asset_btn = new QPushButton("Add an Asset Contract", 0);
        mc_asset_manager_addasset_create_asset_btn->setStyleSheet("QPushButton{padding:1em;}");
        mc_asset_manager_addasset_gridlayout->addWidget(mc_asset_manager_addasset_create_asset_btn, 5,0, 1,1, Qt::AlignHCenter);
        //Connect create asset button with a re-action;
        connect(mc_asset_manager_addasset_create_asset_btn, SIGNAL(clicked()), this, SLOT(mc_addasset_dialog_createasset_slot()));
        
        /** Flag as already init **/
        mc_assetmanager_addasset_dialog_already_init = 1;
    }
    //Resize
    mc_asset_manager_addasset_dialog->resize(400, 290);
    //Show
    mc_asset_manager_addasset_dialog->show();
}

void Moneychanger::mc_assetmanager_removeasset_slot(){
    //Init, then show; If already init, then just show
    if(mc_assetmanager_removeasset_dialog_already_init == 0){
        mc_asset_manager_removeasset_dialog = new QDialog(0);
        mc_asset_manager_removeasset_dialog->setWindowTitle("Remove Asset Contract | Moneychanger");
        mc_asset_manager_removeasset_dialog->setModal(1);
        //Grid layout
        mc_asset_manager_removeasset_gridlayout = new QGridLayout(0);
        mc_asset_manager_removeasset_dialog->setLayout(mc_asset_manager_removeasset_gridlayout);
    }
    mc_asset_manager_removeasset_dialog->show();
}

void Moneychanger::mc_assetmanager_dataChanged_slot(QModelIndex topLeft, QModelIndex bottomRight){
    //Ignore triggers while "refreshing" the asset manager.
    if(mc_assetmanager_refreshing == 0 && mc_assetmanager_proccessing_dataChanged == 0){
        /** Flag Proccessing dataChanged **/
        mc_assetmanager_proccessing_dataChanged = 1;
        
        /** Proccess the "Display Name" column **/
        if(topLeft.column() == 0){
            //Get the value (as std::string) of the asset id
            QStandardItem * asset_id_item = mc_asset_manager_tableview_itemmodel->item(topLeft.row(), 1);
            QString asset_id_string = asset_id_item->text();
            std::string asset_id = asset_id_string.toStdString();
            
            //Get the value (as std::string) of the newly set name of the asset id
            QVariant new_asset_name_variant = topLeft.data();
            QString new_asset_name_string = new_asset_name_variant.toString();
            std::string new_asset_name = new_asset_name_string.toStdString();
            qDebug() << asset_id_string;
            //Update the newly set display name for this asset in OT ( call ot for updating )
            bool setName_successfull = OTAPI_Wrap::SetAssetType_Name(asset_id, new_asset_name);
            if(setName_successfull == true){
                //Do nothing (There is nothing that needs to be done)
            }else{
                //Setting of the display name for this asset failed, revert value visually, display recent error
                mc_asset_manager_most_recent_erorr->setText("<span style='color:#A80000'><b>Renaming that asset failed. (Error Code: 100)</b></span>");
            }
        }
        
        /** Proccess the "Default" column (if triggered) **/
        if(topLeft.column() == 2){
            //The (default) 2 column has checked(or unchecked) a box, (uncheck all checkboxes, except the newly selected one)
            int total_checkboxes = mc_asset_manager_tableview_itemmodel->rowCount();
            
            //Uncheck all boxes (always checkMarked the triggered box)
            for(int a = 0; a < total_checkboxes; a++){
                
                //Check if this is a checkbox we should checkmark
                if(a != topLeft.row()){
                    
                    //Get checkbox item
                    QStandardItem * checkbox_model = mc_asset_manager_tableview_itemmodel->item(a, 2);
                    
                    //Update the checkbox item at the backend.
                    checkbox_model->setCheckState(Qt::Unchecked);
                    
                    //Update the checkbox item visually.
                    mc_asset_manager_tableview_itemmodel->setItem(a, 2, checkbox_model);
                }else if(a == topLeft.row()){
                    
                    //Get checkbox item
                    QStandardItem * checkbox_model = mc_asset_manager_tableview_itemmodel->item(a, 2);
                    
                    //Update the checkbox item at the backend.
                    //Get asset id we are targeting to update.
                    QStandardItem * asset_id = mc_asset_manager_tableview_itemmodel->item(a, 1);
                    QVariant asset_id_variant = asset_id->text();
                    QString asset_id_string = asset_id_variant.toString();
                    QString asset_name_string = QString::fromStdString(OTAPI_Wrap::GetAssetType_Name(asset_id_string.toStdString()));
                    //Update the checkbox item visually.
                    checkbox_model->setCheckState(Qt::Checked);
                    mc_asset_manager_tableview_itemmodel->setItem(a, 2, checkbox_model);
                    
                    //Update default asset at realtime memory backend
                    
                    mc_systrayMenu_asset_setDefaultAsset(asset_id_string, asset_name_string);
                    
                    
                }
                
            }
            
        }
        
        /** Unflag Proccessing Data Changed **/
        mc_assetmanager_proccessing_dataChanged = 0;
    }
}


/**** ****
 **** asset Manager -> Add asset Dialog (Private Slots)
 **** ****/
void Moneychanger::mc_addasset_dialog_showadvanced_slot(QString link_href){
    //If advanced options are already showing, hide, if they are hidden, show them.
    if(mc_assetmanager_addasset_dialog_advanced_showing == 0){
        //Show advanced options.
        //Show the Bits option
        
    }else if(mc_assetmanager_addasset_dialog_advanced_showing == 1){
        //Hide advanced options.
        //Hide the Bits option
    }
}


void Moneychanger::mc_addasset_dialog_createasset_slot(){
    //            std::string pseudonym = OTAPI_Wrap::CreateNym(1024, "", "");
    //            QString new_pseudonym = QString::fromStdString(pseudonym);
    QString new_asset;
    
    //Success if non null
    if(new_asset != ""){
        
    }else{
        //Failed to create asset type
    }
}

// ---------------------------------------------------------
// SERVER
void Moneychanger::mc_servermanager_addserver_slot(){
    //Decide if we should init and show, or just show
    if(mc_servermanager_addserver_dialog_already_init == 0){
        //Init, then show.
        mc_server_manager_addserver_dialog = new QDialog(0);
        mc_server_manager_addserver_dialog->setWindowTitle("Add Server Contract | Moneychanger");
        mc_server_manager_addserver_dialog->setModal(1);
        
        //Gridlayout
        mc_server_manager_addserver_gridlayout = new QGridLayout(0);
        mc_server_manager_addserver_dialog->setLayout(mc_server_manager_addserver_gridlayout);
        
        //Label (header)
        mc_server_manager_addserver_header = new QLabel("<h2>Add Server Contract</h2>",0);
        mc_server_manager_addserver_gridlayout->addWidget(mc_server_manager_addserver_header, 0,0, 1,1, Qt::AlignRight);
        
        //Label (Show Advanced Option(s)) (Also a button/connection)
        mc_server_manager_addserver_subheader_toggleadvanced_options_label = new QLabel("<a href='#'>Advanced Option(s)</a>", 0);
        mc_server_manager_addserver_gridlayout->addWidget(mc_server_manager_addserver_subheader_toggleadvanced_options_label, 1,0, 1,1, Qt::AlignRight);
        //Connect with a re-action
        connect(mc_server_manager_addserver_subheader_toggleadvanced_options_label, SIGNAL(linkActivated(QString)), this, SLOT(mc_addserver_dialog_showadvanced_slot(QString)));
        
        
        //Label (instructions)
        mc_server_manager_addserver_subheader_instructions = new QLabel("Below are some options that will help determine how your server will be added.");
        mc_server_manager_addserver_subheader_instructions->setWordWrap(1);
        mc_server_manager_addserver_subheader_instructions->setStyleSheet("QLabel{padding:0.5em;}");
        mc_server_manager_addserver_gridlayout->addWidget(mc_server_manager_addserver_subheader_instructions, 2,0, 1,1);
        
        //Label (Choose Source Question)
        //                    mc_server_manager_addserver_choosesource_label = new QLabel("<h3>Enter the Server Contract</h3>");
        //                    mc_server_manager_addserver_choosesource_label->setStyleSheet("QLabel{padding:1em;}");
        //                    mc_server_manager_addserver_gridlayout->addWidget(mc_server_manager_addserver_choosesource_label, 3,0, 1,1);
        
        //                    //Combobox (Dropdown box: Choose Source)
        //                    mc_server_manager_addserver_choosesource_answer_selection = new QComboBox(0);
        //                    mc_server_manager_addserver_choosesource_answer_selection->addItem("Namecoin");
        //                    mc_server_manager_addserver_choosesource_answer_selection->addItem("No-Source");
        //                    mc_server_manager_addserver_choosesource_answer_selection->setCurrentIndex(1);
        //                    mc_server_manager_addserver_choosesource_answer_selection->setStyleSheet("QComboBox{padding:0.5em;}");
        //                    mc_server_manager_addserver_gridlayout->addWidget(mc_server_manager_addserver_choosesource_answer_selection, 4,0, 1,1);
        
        //Create server (button)
        mc_server_manager_addserver_create_server_btn = new QPushButton("Add a Server Contract", 0);
        mc_server_manager_addserver_create_server_btn->setStyleSheet("QPushButton{padding:1em;}");
        mc_server_manager_addserver_gridlayout->addWidget(mc_server_manager_addserver_create_server_btn, 5,0, 1,1, Qt::AlignHCenter);
        //Connect create server button with a re-action;
        connect(mc_server_manager_addserver_create_server_btn, SIGNAL(clicked()), this, SLOT(mc_addserver_dialog_createserver_slot()));
        
        /** Flag as already init **/
        mc_servermanager_addserver_dialog_already_init = 1;
    }
    //Resize
    mc_server_manager_addserver_dialog->resize(400, 290);
    //Show
    mc_server_manager_addserver_dialog->show();
}

void Moneychanger::mc_servermanager_removeserver_slot(){
    //Init, then show; If already init, then just show
    if(mc_servermanager_removeserver_dialog_already_init == 0){
        mc_server_manager_removeserver_dialog = new QDialog(0);
        mc_server_manager_removeserver_dialog->setWindowTitle("Remove Server Contract | Moneychanger");
        mc_server_manager_removeserver_dialog->setModal(1);
        //Grid layout
        mc_server_manager_removeserver_gridlayout = new QGridLayout(0);
        mc_server_manager_removeserver_dialog->setLayout(mc_server_manager_removeserver_gridlayout);
    }
    mc_server_manager_removeserver_dialog->show();
}

void Moneychanger::mc_servermanager_dataChanged_slot(QModelIndex topLeft, QModelIndex bottomRight){
    //Ignore triggers while "refreshing" the server manager.
    if(mc_servermanager_refreshing == 0 && mc_servermanager_proccessing_dataChanged == 0){
        /** Flag Proccessing dataChanged **/
        mc_servermanager_proccessing_dataChanged = 1;
        
        /** Proccess the "Display Name" column **/
        if(topLeft.column() == 0){
            //Get the value (as std::string) of the server id
            QStandardItem * server_id_item = mc_server_manager_tableview_itemmodel->item(topLeft.row(), 1);
            QString server_id_string = server_id_item->text();
            std::string server_id = server_id_string.toStdString();
            
            //Get the value (as std::string) of the newly set name of the server id
            QVariant new_server_name_variant = topLeft.data();
            QString new_server_name_string = new_server_name_variant.toString();
            std::string new_server_name = new_server_name_string.toStdString();
            qDebug() << server_id_string;
            //Update the newly set display name for this server in OT ( call ot for updating )
            bool setName_successfull = OTAPI_Wrap::SetServer_Name(server_id, new_server_name);
            if(setName_successfull == true){
                //Do nothing (There is nothing that needs to be done)
            }else{
                //Setting of the display name for this server failed, revert value visually, display recent error
                mc_server_manager_most_recent_erorr->setText("<span style='color:#A80000'><b>Renaming that server failed. (Error Code: 100)</b></span>");
            }
        }
        
        /** Proccess the "Default" column (if triggered) **/
        if(topLeft.column() == 2){
            //The (default) 2 column has checked(or unchecked) a box, (uncheck all checkboxes, except the newly selected one)
            int total_checkboxes = mc_server_manager_tableview_itemmodel->rowCount();
            
            //Uncheck all boxes (always checkMarked the triggered box)
            for(int a = 0; a < total_checkboxes; a++){
                
                //Check if this is a checkbox we should checkmark
                if(a != topLeft.row()){
                    
                    //Get checkbox item
                    QStandardItem * checkbox_model = mc_server_manager_tableview_itemmodel->item(a, 2);
                    
                    //Update the checkbox item at the backend.
                    checkbox_model->setCheckState(Qt::Unchecked);
                    
                    //Update the checkbox item visually.
                    mc_server_manager_tableview_itemmodel->setItem(a, 2, checkbox_model);
                }else if(a == topLeft.row()){
                    
                    //Get checkbox item
                    QStandardItem * checkbox_model = mc_server_manager_tableview_itemmodel->item(a, 2);
                    
                    //Update the checkbox item at the backend.
                    //Get server id we are targeting to update.
                    QStandardItem * server_id = mc_server_manager_tableview_itemmodel->item(a, 1);
                    QVariant server_id_variant = server_id->text();
                    QString server_id_string = server_id_variant.toString();
                    QString server_name_string = QString::fromStdString(OTAPI_Wrap::GetServer_Name(server_id_string.toStdString()));
                    //Update the checkbox item visually.
                    checkbox_model->setCheckState(Qt::Checked);
                    mc_server_manager_tableview_itemmodel->setItem(a, 2, checkbox_model);
                    
                    //Update default server at realtime memory backend
                    
                    mc_systrayMenu_server_setDefaultServer(server_id_string, server_name_string);
                    
                    
                }
                
            }
            
        }
        
        /** Unflag Proccessing Data Changed **/
        mc_servermanager_proccessing_dataChanged = 0;
    }
}


/**** ****
 **** server Manager -> Add server Dialog (Private Slots)
 **** ****/
void Moneychanger::mc_addserver_dialog_showadvanced_slot(QString link_href){
    //If advanced options are already showing, hide, if they are hidden, show them.
    if(mc_servermanager_addserver_dialog_advanced_showing == 0){
        //Show advanced options.
        //Show the Bits option
        
    }else if(mc_servermanager_addserver_dialog_advanced_showing == 1){
        //Hide advanced options.
        //Hide the Bits option
    }
}


void Moneychanger::mc_addserver_dialog_createserver_slot(){
    //            std::string pseudonym = OTAPI_Wrap::CreateNym(1024, "", "");
    //            QString new_pseudonym = QString::fromStdString(pseudonym);
    QString new_server;
    
    //Success if non null
    if(new_server != ""){
        
    }else{
        //Failed to create server type
    }
}
// ---------------------------------------------------------
// ACCOUNTS
void Moneychanger::mc_accountmanager_addaccount_slot(){
    //Decide if we should init and show, or just show
    if(mc_accountmanager_addaccount_dialog_already_init == 0){
        //Init, then show.
        mc_account_manager_addaccount_dialog = new QDialog(0);
        mc_account_manager_addaccount_dialog->setWindowTitle("Add Account | Moneychanger");
        mc_account_manager_addaccount_dialog->setModal(1);
        
        //Gridlayout
        mc_account_manager_addaccount_gridlayout = new QGridLayout(0);
        mc_account_manager_addaccount_dialog->setLayout(mc_account_manager_addaccount_gridlayout);
        
        //Label (header)
        mc_account_manager_addaccount_header = new QLabel("<h2>Add Account</h2>",0);
        mc_account_manager_addaccount_gridlayout->addWidget(mc_account_manager_addaccount_header, 0,0, 1,1, Qt::AlignRight);
        
        //Label (Show Advanced Option(s)) (Also a button/connection)
        mc_account_manager_addaccount_subheader_toggleadvanced_options_label = new QLabel("<a href='#'>Advanced Option(s)</a>", 0);
        mc_account_manager_addaccount_gridlayout->addWidget(mc_account_manager_addaccount_subheader_toggleadvanced_options_label, 1,0, 1,1, Qt::AlignRight);
        //Connect with a re-action
        connect(mc_account_manager_addaccount_subheader_toggleadvanced_options_label, SIGNAL(linkActivated(QString)), this, SLOT(mc_addaccount_dialog_showadvanced_slot(QString)));
        
        
        //Label (instructions)
        mc_account_manager_addaccount_subheader_instructions = new QLabel("Below are some options that will help determine how your Account will be added.");
        mc_account_manager_addaccount_subheader_instructions->setWordWrap(1);
        mc_account_manager_addaccount_subheader_instructions->setStyleSheet("QLabel{padding:0.5em;}");
        mc_account_manager_addaccount_gridlayout->addWidget(mc_account_manager_addaccount_subheader_instructions, 2,0, 1,1);
        
        //Label (Choose Source Question)
        //                    mc_account_manager_addaccount_choosesource_label = new QLabel("<h3>Choose the source of the Account</h3>");
        //                    mc_account_manager_addaccount_choosesource_label->setStyleSheet("QLabel{padding:1em;}");
        //                    mc_account_manager_addaccount_gridlayout->addWidget(mc_account_manager_addaccount_choosesource_label, 3,0, 1,1);
        
        //                    //Combobox (Dropdown box: Choose Source)
        //                    mc_account_manager_addaccount_choosesource_answer_selection = new QComboBox(0);
        //                    mc_account_manager_addaccount_choosesource_answer_selection->addItem("Namecoin");
        //                    mc_account_manager_addaccount_choosesource_answer_selection->addItem("No-Source");
        //                    mc_account_manager_addaccount_choosesource_answer_selection->setCurrentIndex(1);
        //                    mc_account_manager_addaccount_choosesource_answer_selection->setStyleSheet("QComboBox{padding:0.5em;}");
        //                    mc_account_manager_addaccount_gridlayout->addWidget(mc_account_manager_addaccount_choosesource_answer_selection, 4,0, 1,1);
        
        //Create account (button)
        mc_account_manager_addaccount_create_account_btn = new QPushButton("Create a new Account", 0);
        mc_account_manager_addaccount_create_account_btn->setStyleSheet("QPushButton{padding:1em;}");
        mc_account_manager_addaccount_gridlayout->addWidget(mc_account_manager_addaccount_create_account_btn, 5,0, 1,1, Qt::AlignHCenter);
        //Connect create account button with a re-action;
        connect(mc_account_manager_addaccount_create_account_btn, SIGNAL(clicked()), this, SLOT(mc_addaccount_dialog_createaccount_slot()));
        
        /** Flag as already init **/
        mc_accountmanager_addaccount_dialog_already_init = 1;
    }
    //Resize
    mc_account_manager_addaccount_dialog->resize(400, 290);
    //Show
    mc_account_manager_addaccount_dialog->show();
}

void Moneychanger::mc_accountmanager_removeaccount_slot(){
    //Init, then show; If already init, then just show
    if(mc_accountmanager_removeaccount_dialog_already_init == 0){
        mc_account_manager_removeaccount_dialog = new QDialog(0);
        mc_account_manager_removeaccount_dialog->setWindowTitle("Remove Account | Moneychanger");
        mc_account_manager_removeaccount_dialog->setModal(1);
        //Grid layout
        mc_account_manager_removeaccount_gridlayout = new QGridLayout(0);
        mc_account_manager_removeaccount_dialog->setLayout(mc_account_manager_removeaccount_gridlayout);
    }
    mc_account_manager_removeaccount_dialog->show();
}

void Moneychanger::mc_accountmanager_dataChanged_slot(QModelIndex topLeft, QModelIndex bottomRight){
    //Ignore triggers while "refreshing" the account manager.
    if(mc_accountmanager_refreshing == 0 && mc_accountmanager_proccessing_dataChanged == 0){
        /** Flag Proccessing dataChanged **/
        mc_accountmanager_proccessing_dataChanged = 1;
        
        /** Proccess the "Display Name" column **/
        if(topLeft.column() == 0){
            //Get the value (as std::string) of the account id
            QStandardItem * account_id_item = mc_account_manager_tableview_itemmodel->item(topLeft.row(), 1);
            QString account_id_string = account_id_item->text();
            std::string account_id = account_id_string.toStdString();
            
            //Get the value (as std::string) of the newly set name of the account id
            QVariant new_account_name_variant = topLeft.data();
            QString new_account_name_string = new_account_name_variant.toString();
            std::string new_account_name = new_account_name_string.toStdString();
            qDebug() << account_id_string;
            //Update the newly set display name for this account in OT ( call ot for updating )
            bool setName_successfull = OTAPI_Wrap::SetAccountWallet_Name(account_id, account_id, new_account_name);
            if(setName_successfull == true){
                //Do nothing (There is nothing that needs to be done)
            }else{
                //Setting of the display name for this account failed, revert value visually, display recent error
                mc_account_manager_most_recent_erorr->setText("<span style='color:#A80000'><b>Renaming that account failed. (Error Code: 100)</b></span>");
            }
        }
        
        /** Proccess the "Default" column (if triggered) **/
        if(topLeft.column() == 2){
            //The (default) 2 column has checked(or unchecked) a box, (uncheck all checkboxes, except the newly selected one)
            int total_checkboxes = mc_account_manager_tableview_itemmodel->rowCount();
            
            //Uncheck all boxes (always checkMarked the triggered box)
            for(int a = 0; a < total_checkboxes; a++){
                
                //Check if this is a checkbox we should checkmark
                if(a != topLeft.row()){
                    
                    //Get checkbox item
                    QStandardItem * checkbox_model = mc_account_manager_tableview_itemmodel->item(a, 2);
                    
                    //Update the checkbox item at the backend.
                    checkbox_model->setCheckState(Qt::Unchecked);
                    
                    //Update the checkbox item visually.
                    mc_account_manager_tableview_itemmodel->setItem(a, 2, checkbox_model);
                }else if(a == topLeft.row()){
                    
                    //Get checkbox item
                    QStandardItem * checkbox_model = mc_account_manager_tableview_itemmodel->item(a, 2);
                    
                    //Update the checkbox item at the backend.
                    //Get account id we are targeting to update.
                    QStandardItem * account_id = mc_account_manager_tableview_itemmodel->item(a, 1);
                    QVariant account_id_variant = account_id->text();
                    QString account_id_string = account_id_variant.toString();
                    QString account_name_string = QString::fromStdString(OTAPI_Wrap::GetAccountWallet_Name(account_id_string.toStdString()));
                    //Update the checkbox item visually.
                    checkbox_model->setCheckState(Qt::Checked);
                    mc_account_manager_tableview_itemmodel->setItem(a, 2, checkbox_model);
                    
                    //Update default account at realtime memory backend
                    
                    mc_systrayMenu_account_setDefaultAccount(account_id_string, account_name_string);
                    
                    
                }
                
            }
            
        }
        
        /** Unflag Proccessing Data Changed **/
        mc_accountmanager_proccessing_dataChanged = 0;
    }
}


/**** ****
 **** account Manager -> Add account Dialog (Private Slots)
 **** ****/
void Moneychanger::mc_addaccount_dialog_showadvanced_slot(QString link_href){
    //If advanced options are already showing, hide, if they are hidden, show them.
    if(mc_accountmanager_addaccount_dialog_advanced_showing == 0){
        //Show advanced options.
        //Show the Bits option
        
    }else if(mc_accountmanager_addaccount_dialog_advanced_showing == 1){
        //Hide advanced options.
        //Hide the Bits option
    }
}


void Moneychanger::mc_addaccount_dialog_createaccount_slot(){
    //            std::string pseudonym = OTAPI_Wrap::CreateNym(1024, "", "");
    //            QString new_pseudonym = QString::fromStdString(pseudonym);
    QString new_account;
    
    //Success if non null
    if(new_account != ""){
        
    }else{
        //Failed to create account
    }
}


/* Address Book Slots */
//When "add contact" is clicked, Add a blank row to the address book so the user can edit it and save their changes.
void Moneychanger::mc_addressbook_addblankrow_slot(){
    //Get total rows from the table view
    int total_rows_in_table = 0;
    total_rows_in_table = mc_addressbook_tableview_itemmodel->rowCount();
    
    //Insert blank row
    int blank_row_target_index = total_rows_in_table;
    QStandardItem * blank_row_item = new QStandardItem("");
    mc_addressbook_tableview_itemmodel->setItem(blank_row_target_index,0,blank_row_item);
    
    
}

//Confirm "remove contact" from address book
void Moneychanger::mc_addressbook_confirm_remove_contact_slot(){
    //First validate if anything is selected, before continuing (this is a must).
    QModelIndexList indexList = mc_addressbook_tableview->selectionModel()->selectedIndexes();
    int total_selected = indexList.size();
    if(total_selected >= 1){
        //Loop through every selected menu and delete it visually as well as through the database/storage.
        int total_deleted = 0;
        while(total_deleted < total_selected){
            //Increment right away
            total_deleted += 1;
            
            //Extract the database index id.
            QModelIndex data_row_model = indexList.at(0);
            QModelIndex db_id_model = mc_addressbook_tableview_itemmodel->index(data_row_model.row(), 2, QModelIndex());
            QVariant db_id_variant = mc_addressbook_tableview_itemmodel->data(db_id_model);
            int db_id = db_id_variant.toInt();
            //Delete data from the database/storage.
            QSqlQuery mc_addressbook_delete_row(addressbook_db);
            mc_addressbook_delete_row.exec(QString("DELETE FROM `address_book` WHERE `id` = %1").arg(db_id));
            
            //Delete data from the visuals.
            mc_addressbook_tableview_itemmodel->removeRow(data_row_model.row());
            
        }
        
    }else{
        qDebug() <<"nothing was selected to be removed";
        QMessageBox::information(this,"Moneychanger","Nothing selected to remove.");
    }
}

//When a row is edited/updated this will be triggered to sync the changes to the database.
void Moneychanger::mc_addressbook_dataChanged_slot(QModelIndex topLeft, QModelIndex bottomRight){
    
    //This slot will ignore everything while the address book is refreshing; If not refreshing, go about regular logic.
    if(mc_addressbook_refreshing == 0){
        
        //Get Database Index that is associated with the edited row.
        int target_index_row = topLeft.row();
        QModelIndex index = mc_addressbook_tableview_itemmodel->index(target_index_row, 2, QModelIndex());
        QVariant index_id_variant = mc_addressbook_tableview_itemmodel->data(index);
        QString index_id_string = index_id_variant.toString();
        int index_id = index_id_string.toInt();
        
        
        //Get Nym Display Name
        QModelIndex nym_display_name = mc_addressbook_tableview_itemmodel->index(target_index_row, 0, QModelIndex());
        QVariant nym_display_name_variant = mc_addressbook_tableview_itemmodel->data(nym_display_name);
        QString nym_display_name_string = nym_display_name_variant.toString();
        
        //Get Nym Id
        QModelIndex nym_id = mc_addressbook_tableview_itemmodel->index(target_index_row, 1, QModelIndex());
        QVariant nym_id_variant = mc_addressbook_tableview_itemmodel->data(nym_id);
        QString nym_id_string = nym_id_variant.toString();
        
        //If Index_id is not greater than zero, then insert, if greater then zero, update.
        if(index_id == 0){
            //Before inserting, check if any data has been entered in
            if(nym_display_name_string != "" || nym_id_string != ""){
                QSqlQuery mc_addressbook_insert_query(addressbook_db);
                mc_addressbook_insert_query.exec(QString("INSERT INTO `address_book` (`id`, `nym_id`, `nym_display_name`) VALUES(NULL, '%1', '%2')").arg(nym_id_string).arg(nym_display_name_string));
                //Get last insert id (This is so we can attribute the visual with a DB id.
                //Set the associated visual data with the row id from the database/storage index.
                mc_addressbook_tableview_itemmodel->setData(index, mc_addressbook_insert_query.lastInsertId());
            }
        }else{
            //Update
            QSqlQuery mc_addressbook_insert_query(addressbook_db);
            mc_addressbook_insert_query.exec(QString("UPDATE `address_book` SET `nym_id` = '%1', `nym_display_name` = '%2' WHERE `id` = %3").arg(nym_id_string).arg(nym_display_name_string).arg(index_id_string));
        }
    }
}

//"paste selected" button then we will detect here where to paste and what to paste.
void Moneychanger::mc_addressbook_paste_selected_slot(){
    //First validate if anything is selected, before continuing (this is a must).
    QModelIndexList indexList = mc_addressbook_tableview->selectionModel()->selectedIndexes();
    int total_selected = indexList.size();
    qDebug() << "TOTAL SELECTED" << total_selected;
    if(total_selected >= 1){
        //Paste the selection into the appropriate contact.
        if(mc_addressbook_paste_into == "withdraw_as_voucher"){
            //Extract only the first row of the column of Nym ID
            QModelIndex data_row_model = indexList.at(0);
            qDebug() << "ROW " << data_row_model.row();
            
            QModelIndex selected_nym_id = mc_addressbook_tableview_itemmodel->index(data_row_model.row(), 1, QModelIndex());
            QVariant selected_nym_id_variant = mc_addressbook_tableview_itemmodel->data(selected_nym_id);
            QString selected_nym_id_string = selected_nym_id_variant.toString();
            
            //Paste into the "Withdraw as voucher" window in the receipient nym area.
            mc_systrayMenu_withdraw_asvoucher_nym_input->setText(selected_nym_id_string);
            
            //Hide address book now that the operator has selected a nym
            mc_addressbook_dialog->hide();
        }
    }else{
        qDebug() << "nothing was selected to paste into the target area";
        QMessageBox::information(this,"Moneychanger","Nothing selected to paste.");
    }
}


/* Systray menu slots */

//Shutdown slots
void Moneychanger::mc_shutdown_slot(){
    //Disconnect all signals from callin class (probubly main) to this class
    //Disconnect
    QObject::disconnect(this);
    //Close qt app (no need to deinit anything as of the time of this comment)
    //TO DO: Check if the OT queue caller is still proccessing calls.... Then quit the app. (Also tell user that the OT is still calling other wise they might think it froze during OT calls)
    qApp->quit();
}

//Overview slots
void Moneychanger::mc_overview_slot(){
    //The operator has requested to open the dialog to the "Overview";
    mc_overview_dialog();
}


//Default Nym slots
//Nym manager "clicked"
void Moneychanger::mc_defaultnym_slot(){
    //The operator has requested to open the dialog to the "Nym Manager";
    mc_nymmanager_dialog();
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
        mc_systrayMenu_nym_setDefaultNym(action_triggered_string, action_triggered_string_nym_name);
        
        //Refresh the nym default selection in the nym manager (ONLY if it is open)
        //Check if nym manager has ever been opened (then apply logic) [prevents crash if the dialog hasen't be opend before]
        if(mc_nymmanager_already_init == 1){
            //Refresh if the nym manager is currently open
            if(mc_nym_manager_dialog->isVisible()){
                mc_nymmanager_dialog();
            }
        }
    }
    
}

//Default Asset slots
//Asset manager "clicked"
void Moneychanger::mc_defaultasset_slot(){
    //The operator has requested to open the dialog to the "Asset Manager";
    mc_assetmanager_dialog();
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
        mc_systrayMenu_asset_setDefaultAsset(action_triggered_string, action_triggered_string_asset_name);
        
        //Refresh the asset default selection in the asset manager (ONLY if it is open)
        //Check if asset manager has ever been opened (then apply logic) [prevents crash if the dialog hasen't be opend before]
        if(mc_assetmanager_already_init == 1){
            //Refresh if the asset manager is currently open
            if(mc_asset_manager_dialog->isVisible()){
                mc_assetmanager_dialog();
            }
        }
    }
    
}

//Default Account slots
//Account manager "clicked"
void Moneychanger::mc_defaultaccount_slot(){
    //The operator has requested to open the dialog to the "account Manager";
    mc_accountmanager_dialog();
}

//account new default selected from systray
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
        mc_systrayMenu_account_setDefaultAccount(action_triggered_string, action_triggered_string_account_name);
        
        //Refresh the account default selection in the account manager (ONLY if it is open)
        //Check if account manager has ever been opened (then apply logic) [prevents crash if the dialog hasen't be opend before]
        if(mc_accountmanager_already_init == 1){
            //Refresh if the account manager is currently open
            if(mc_account_manager_dialog->isVisible()){
                mc_accountmanager_dialog();
            }
        }
    }
    
}

//Server Slots
/** *****************************************
 * @brief Moneychanger::mc_defaultserver_slot
 * @info Universal call to opening the server list manager.
 ** *****************************************/
void Moneychanger::mc_defaultserver_slot(){
    mc_servermanager_dialog();
}



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
        mc_systrayMenu_server_setDefaultServer(action_triggered_string, action_triggered_string_server_name);
        
        //Refresh the server default selection in the server manager (ONLY if it is open)
        //Check if server manager has ever been opened (then apply logic) [prevents crash if the dialog hasen't be opend before]
        if(mc_servermanager_already_init == 1){
            //Refresh if the server manager is currently open
            if(mc_server_manager_dialog->isVisible()){
                mc_servermanager_dialog();
            }
        }
    }
}


/**
 * @brief Moneychanger::mc_servermanager_request_remove_server_slot
 * @info This will attempt to remove the server from the loaded wallet,
 *       At the moment only "one" server can be selected but the for loop is there for
 *       future upgrades of such functionality.
 **/
void Moneychanger::mc_servermanager_request_remove_server_slot(){
    //Extract the currently selected server from the server-list.
    QModelIndexList selected_indexes = mc_server_manager_tableview->selectionModel()->selectedIndexes();
    
    for(int a = 0; a < selected_indexes.size(); a++){
        QModelIndex selected_index = selected_indexes.at(a);
        int selected_row = selected_index.row();
        
        //Get server id
        QModelIndex server_id_modelindex = mc_server_manager_tableview_itemmodel->index(selected_row, 1, QModelIndex());
        QVariant server_id_variant = server_id_modelindex.data();
        QString server_id_string = server_id_variant.toString();
        bool can_remove = OTAPI_Wrap::Wallet_CanRemoveServer(server_id_string.toStdString());
        
        if(can_remove == true){
            //Remove it
            OTAPI_Wrap::Wallet_RemoveServer(server_id_string.toStdString());
        }else{
            //Find out why it can't be removed and alert the user the reasoning.
            //Loop through nyms
            std::string server_id_std = server_id_string.toStdString();
            int num_nyms_registered_at_server = 0;
            int num_nyms = OTAPI_Wrap::GetNymCount();
            for(int b = 0; b < num_nyms; b++){
                bool nym_index_at_server = OTAPI_Wrap::IsNym_RegisteredAtServer(OTAPI_Wrap::GetNym_ID(b), server_id_std);
                if(nym_index_at_server == true){
                    num_nyms_registered_at_server += 1;
                }
            }
        }
        
    }
    
}


void Moneychanger::mc_assetmanager_request_remove_asset_slot(){
    //Extract the currently selected asset from the asset-list.
    QModelIndexList selected_indexes = mc_asset_manager_tableview->selectionModel()->selectedIndexes();
    
    for(int a = 0; a < selected_indexes.size(); a++){
        QModelIndex selected_index = selected_indexes.at(a);
        int selected_row = selected_index.row();
        
        //Get asset id
        QModelIndex asset_id_modelindex = mc_asset_manager_tableview_itemmodel->index(selected_row, 1, QModelIndex());
        QVariant asset_id_variant = asset_id_modelindex.data();
        QString asset_id_string = asset_id_variant.toString();
        bool can_remove = OTAPI_Wrap::Wallet_CanRemoveAssetType(asset_id_string.toStdString());
        
        if(can_remove == true){
            //Remove it
            OTAPI_Wrap::Wallet_RemoveAssetType(asset_id_string.toStdString());
        }else{
            //Find out why it can't be removed and alert the user the reasoning.
            //Loop through nyms
            //                        std::string asset_id_std = asset_id_string.toStdString();
            //                        int num_nyms_registered_at_asset = 0;
            //                        int num_nyms = OTAPI_Wrap::GetNymCount();
            //                        for(int b = 0; b < num_nyms; b++){
            //                            bool nym_index_at_asset = OTAPI_Wrap::IsNym_RegisteredAtAsset(OTAPI_Wrap::GetNym_ID(b), asset_id_std);
            //                            if(nym_index_at_asset == true){
            //                                num_nyms_registered_at_asset += 1;
            //                            }
            //                        }
        }
        
    }
    
}


void Moneychanger::mc_nymmanager_request_remove_nym_slot(){
    //Extract the currently selected nym from the nym-list.
    QModelIndexList selected_indexes = mc_nym_manager_tableview->selectionModel()->selectedIndexes();
    
    for(int a = 0; a < selected_indexes.size(); a++){
        QModelIndex selected_index = selected_indexes.at(a);
        int selected_row = selected_index.row();
        
        //Get nym id
        QModelIndex nym_id_modelindex = mc_nym_manager_tableview_itemmodel->index(selected_row, 1, QModelIndex());
        QVariant nym_id_variant = nym_id_modelindex.data();
        QString nym_id_string = nym_id_variant.toString();
        bool can_remove = OTAPI_Wrap::Wallet_CanRemoveNym(nym_id_string.toStdString());
        
        if(can_remove == true){
            //Remove it
            OTAPI_Wrap::Wallet_RemoveNym(nym_id_string.toStdString());
        }else{
            //Find out why it can't be removed and alert the user the reasoning.
            //                        //Loop through nyms
            //                        std::string nym_id_std = nym_id_string.toStdString();
            //                        int num_nyms_registered_at_nym = 0;
            //                        int num_nyms = OTAPI_Wrap::GetNymCount();
            //                        for(int b = 0; b < num_nyms; b++){
            //                            bool nym_index_at_nym = OTAPI_Wrap::IsNym_RegisteredAtnym(OTAPI_Wrap::GetNym_ID(b), nym_id_std);
            //                            if(nym_index_at_nym == true){
            //                                num_nyms_registered_at_nym += 1;
            //                            }
            //                        }
        }
        
    }
    
}



void Moneychanger::mc_accountmanager_request_remove_account_slot(){
    //Extract the currently selected account from the account-list.
    QModelIndexList selected_indexes = mc_account_manager_tableview->selectionModel()->selectedIndexes();
    
    for(int a = 0; a < selected_indexes.size(); a++){
        QModelIndex selected_index = selected_indexes.at(a);
        int selected_row = selected_index.row();
        
        //Get account id
        QModelIndex account_id_modelindex = mc_account_manager_tableview_itemmodel->index(selected_row, 1, QModelIndex());
        QVariant account_id_variant = account_id_modelindex.data();
        QString account_id_string = account_id_variant.toString();
        bool can_remove = OTAPI_Wrap::Wallet_CanRemoveAccount(account_id_string.toStdString());
        
        if(can_remove == true){
            //Remove it
            //                    OTAPI_Wrap::Wallet_RemoveAccount(account_id_string.toStdString());
        }else{
            //                    //Find out why it can't be removed and alert the user the reasoning.
            //                        //Loop through nyms
            //                        std::string account_id_std = account_id_string.toStdString();
            //                        int num_nyms_registered_at_account = 0;
            //                        int num_nyms = OTAPI_Wrap::GetNymCount();
            //                        for(int b = 0; b < num_nyms; b++){
            //                            bool nym_index_at_account = OTAPI_Wrap::IsNym_RegisteredAtaccount(OTAPI_Wrap::GetNym_ID(b), account_id_std);
            //                            if(nym_index_at_account == true){
            //                                num_nyms_registered_at_account += 1;
            //                            }
            //                        }
        }
        
    }
    
}

//Withdraw Slots
/*
 ** AS CASH SLOTS()
 */


/** Open the dialog window **/
void Moneychanger::mc_withdraw_ascash_slot(){
    //The operator has requested to open the dialog to withdraw as cash.
    mc_withdraw_ascash_dialog();
}

/**
 ** Button from dialog window has been activated;
 ** Confirm amount;
 ** Upon confirmation call OT withdraw_cash()
 **/
void Moneychanger::mc_withdraw_ascash_confirm_amount_dialog_slot(){
    //Close the (withdraw as cash) dialog
    mc_systrayMenu_withdraw_ascash_dialog->hide();
    
    //First confirm this is the correct amount before calling OT
    //Has this dialog already been init before?
    if(mc_withdraw_ascash_confirm_dialog_already_init == 0){
        //First time init
        mc_systrayMenu_withdraw_ascash_confirm_dialog = new QDialog(0);
        
        //Attach layout
        mc_systrayMenu_withdraw_ascash_confirm_gridlayout = new QGridLayout(0);
        mc_systrayMenu_withdraw_ascash_confirm_dialog->setLayout(mc_systrayMenu_withdraw_ascash_confirm_gridlayout);
        
        //Ask the operator to confirm the amount requested
        mc_systrayMenu_withdraw_ascash_confirm_label = new QLabel("Please confirm the amount to withdraw.");
        mc_systrayMenu_withdraw_ascash_confirm_gridlayout->addWidget(mc_systrayMenu_withdraw_ascash_confirm_label, 0,0, 1,1);
        
        //Label (Amount)
        QString confirm_amount_string = "<b>"+mc_systrayMenu_withdraw_ascash_amount_input->text()+"</b>";
        mc_systrayMenu_withdraw_ascash_confirm_amount_label = new QLabel(confirm_amount_string);
        mc_systrayMenu_withdraw_ascash_confirm_gridlayout->addWidget(mc_systrayMenu_withdraw_ascash_confirm_amount_label, 1,0, 1,1);
        
        //Set Withdraw as cash amount int
        QString confirm_amount_string_int = mc_systrayMenu_withdraw_ascash_amount_input->text();
        
        withdraw_ascash_confirm_amount_int = confirm_amount_string_int.toInt();
        
        //Spacer
        
        //Horizontal Box
        mc_systrayMenu_withdraw_ascash_confirm_amount_confirm_cancel_widget = new QWidget(0);
        mc_systrayMenu_withdraw_ascash_confirm_amount_confirm_cancel_layout = new QHBoxLayout(0);
        mc_systrayMenu_withdraw_ascash_confirm_amount_confirm_cancel_widget->setLayout(mc_systrayMenu_withdraw_ascash_confirm_amount_confirm_cancel_layout);
        mc_systrayMenu_withdraw_ascash_confirm_gridlayout->addWidget(mc_systrayMenu_withdraw_ascash_confirm_amount_confirm_cancel_widget, 3, 0, 1, 1);
        
        //Button (Cancel amount)
        mc_systrayMenu_withdraw_ascash_confirm_amount_btn_cancel = new QPushButton("Cancel Amount", 0);
        mc_systrayMenu_withdraw_ascash_confirm_amount_confirm_cancel_layout->addWidget(mc_systrayMenu_withdraw_ascash_confirm_amount_btn_cancel);
        //Connect the cancel button with a re-action
        connect(mc_systrayMenu_withdraw_ascash_confirm_amount_btn_cancel, SIGNAL(clicked()), this, SLOT(mc_withdraw_ascash_cancel_amount_slot()));
        
        //Button (Confirm amount)
        mc_systrayMenu_withdraw_ascash_confirm_amount_btn_confirm = new QPushButton("Confirm Amount", 0);
        mc_systrayMenu_withdraw_ascash_confirm_amount_confirm_cancel_layout->addWidget(mc_systrayMenu_withdraw_ascash_confirm_amount_btn_confirm);
        //Connect the Confirm button with a re-action
        connect(mc_systrayMenu_withdraw_ascash_confirm_amount_btn_confirm, SIGNAL(clicked()), this, SLOT(mc_withdraw_ascash_confirm_amount_slot()));
        
        /** Flag already init **/
        mc_withdraw_ascash_confirm_dialog_already_init = 1;
        
        //Show
        mc_systrayMenu_withdraw_ascash_confirm_dialog->show();
        
        
    }else{
        //Not first time init, just show the dialog.
        
        //Set Withdraw as cash amount int
        QString confirm_amount_string_int = mc_systrayMenu_withdraw_ascash_amount_input->text();
        withdraw_ascash_confirm_amount_int = confirm_amount_string_int.toInt();
        
        //Show dialog.
        mc_systrayMenu_withdraw_ascash_confirm_dialog->show();
    }
    
}


/**
 ** This will display the account id that the user has selected (for convience also for backend id tracking)
 **/
void Moneychanger::mc_withdraw_ascash_account_dropdown_highlighted_slot(int dropdown_index){
    //Change Account ID label to the highlighted(bymouse) dropdown index.
    mc_systrayMenu_withdraw_ascash_accountid_label->setText(mc_systrayMenu_withdraw_ascash_account_dropdown->itemData(dropdown_index).toString());
}

/**
 ** This will be triggered when the user click the "confirm amount" button from the withdraw/confirm dialog
 **/

void Moneychanger::mc_withdraw_ascash_confirm_amount_slot(){
    //Close the dialog/window
    mc_systrayMenu_withdraw_ascash_confirm_dialog->hide();
    
    //Collect require information to call the OT_ME::withdraw_cash(?,?,?) function
    QString selected_account_id = mc_systrayMenu_withdraw_ascash_account_dropdown->itemData(mc_systrayMenu_withdraw_ascash_account_dropdown->currentIndex()).toString();
    std::string selected_account_id_string = selected_account_id.toStdString();
    
    QString amount_to_withdraw_string = mc_systrayMenu_withdraw_ascash_amount_input->text();
    int64_t amount_to_withdraw_int = amount_to_withdraw_string.toInt();
    
    //Get Nym ID
    std::string nym_id = OTAPI_Wrap::GetAccountWallet_NymID(selected_account_id_string);
    
    //Get Server ID
    std::string selected_server_id_string = OTAPI_Wrap::GetAccountWallet_ServerID(selected_account_id_string);
    
    //Call OTAPI Withdraw cash
    std::string withdraw_cash_response = ot_me->withdraw_cash(selected_server_id_string, nym_id, selected_account_id_string, amount_to_withdraw_int);
    //qDebug() << QString::fromStdString(withdraw_cash_response);
    
    
}

/**
 ** This will be triggered when the user click the "cancel amount" button from the withdraw/confirm dialog
 **/
void Moneychanger::mc_withdraw_ascash_cancel_amount_slot(){
    //Close the dialog/window
    mc_systrayMenu_withdraw_ascash_confirm_dialog->hide();
}


/*
 ** AS VOUCHER SLOTS()
 */

/** Open a new dialog window **/
void Moneychanger::mc_withdraw_asvoucher_slot(){
    //The operator has requested to open the dialog to withdraw as cash.
    mc_withdraw_asvoucher_dialog();
}

/**
 ** This will be triggered when the user hovers over a dropdown (combobox) item in the withdraw as voucher account selection
 **/
void Moneychanger::mc_withdraw_asvoucher_account_dropdown_highlighted_slot(int dropdown_index){
    //Change Account ID label to the highlighted(bymouse) dropdown index.
    mc_systrayMenu_withdraw_asvoucher_accountid_label->setText(mc_systrayMenu_withdraw_asvoucher_account_dropdown->itemData(dropdown_index).toString());
}


//This will show the address book, the opened address book will be set to paste in recipient nym ids if/when selecting a nymid in the addressbook.
void Moneychanger::mc_withdraw_asvoucher_show_addressbook_slot(){
    //Show address book
    mc_addressbook_show("withdraw_as_voucher");
}


/**
 ** Button from dialog window has been activated;
 ** Confirm amount;
 ** Upon confirmation call OT withdraw_voucher()
 **/
void Moneychanger::mc_withdraw_asvoucher_confirm_amount_dialog_slot(){
    //Close the (withdraw as voucher) dialog
    mc_systrayMenu_withdraw_asvoucher_dialog->hide();
    
    //First confirm this is the correct amount before calling OT
    //Has this dialog already been init before?
    if(mc_withdraw_asvoucher_confirm_dialog_already_init == 0){
        //First time init
        mc_systrayMenu_withdraw_asvoucher_confirm_dialog = new QDialog(0);
        //Set window properties
        mc_systrayMenu_withdraw_asvoucher_confirm_dialog->setWindowTitle("Confirm Amount | Withdraw as Voucher | Moneychanger");
        mc_systrayMenu_withdraw_asvoucher_confirm_dialog->setWindowFlags(Qt::WindowStaysOnTopHint);
        
        //Attach layout
        mc_systrayMenu_withdraw_asvoucher_confirm_gridlayout = new QGridLayout(0);
        mc_systrayMenu_withdraw_asvoucher_confirm_dialog->setLayout(mc_systrayMenu_withdraw_asvoucher_confirm_gridlayout);
        
        
        //Ask the operator to confirm the amount
        //Ask Label
        mc_systrayMenu_withdraw_asvoucher_confirm_label = new QLabel("<h3>Please confirm the amount to withdraw.</h3>", 0);
        mc_systrayMenu_withdraw_asvoucher_confirm_label->setAlignment(Qt::AlignRight);
        mc_systrayMenu_withdraw_asvoucher_confirm_gridlayout->addWidget(mc_systrayMenu_withdraw_asvoucher_confirm_label, 0,0, 1,1);
        
        //Label (Amount)
        QString confirm_amount_string = "<b>"+mc_systrayMenu_withdraw_asvoucher_amount_input->text()+"</b>";
        mc_systrayMenu_withdraw_asvoucher_confirm_amount_label = new QLabel(confirm_amount_string);
        mc_systrayMenu_withdraw_asvoucher_confirm_amount_label->setAlignment(Qt::AlignHCenter);
        mc_systrayMenu_withdraw_asvoucher_confirm_gridlayout->addWidget(mc_systrayMenu_withdraw_asvoucher_confirm_amount_label, 1,0, 1,1);
        
        
        //Set Withdraw as voucher amount int
        QString confirm_amount_string_int = mc_systrayMenu_withdraw_asvoucher_amount_input->text();
        withdraw_asvoucher_confirm_amount_int = confirm_amount_string_int.toInt();
        
        
        //Spacer
        
        //Horizontal Box
        mc_systrayMenu_withdraw_asvoucher_confirm_amount_confirm_cancel_widget = new QWidget(0);
        mc_systrayMenu_withdraw_asvoucher_confirm_amount_confirm_cancel_layout = new QHBoxLayout(0);
        mc_systrayMenu_withdraw_asvoucher_confirm_amount_confirm_cancel_widget->setLayout(mc_systrayMenu_withdraw_asvoucher_confirm_amount_confirm_cancel_layout);
        mc_systrayMenu_withdraw_asvoucher_confirm_gridlayout->addWidget(mc_systrayMenu_withdraw_asvoucher_confirm_amount_confirm_cancel_widget, 3, 0, 1, 1);
        
        
        //Button (Cancel amount)
        mc_systrayMenu_withdraw_asvoucher_confirm_amount_btn_cancel = new QPushButton("Cancel Amount", 0);
        mc_systrayMenu_withdraw_asvoucher_confirm_amount_confirm_cancel_layout->addWidget(mc_systrayMenu_withdraw_asvoucher_confirm_amount_btn_cancel);
        //Connect the cancel button with a re-action
        connect(mc_systrayMenu_withdraw_asvoucher_confirm_amount_btn_cancel, SIGNAL(clicked()), this, SLOT(mc_withdraw_asvoucher_cancel_amount_slot()));
        
        //Button (Confirm amount)
        mc_systrayMenu_withdraw_asvoucher_confirm_amount_btn_confirm = new QPushButton("Confirm Amount", 0);
        mc_systrayMenu_withdraw_asvoucher_confirm_amount_confirm_cancel_layout->addWidget(mc_systrayMenu_withdraw_asvoucher_confirm_amount_btn_confirm);
        //Connect the Confirm button with a re-action
        connect(mc_systrayMenu_withdraw_asvoucher_confirm_amount_btn_confirm, SIGNAL(clicked()), this, SLOT(mc_withdraw_asvoucher_confirm_amount_slot()));
        
        
        /** Flag already init **/
        mc_withdraw_asvoucher_confirm_dialog_already_init = 1;
        
        //Show
        mc_systrayMenu_withdraw_asvoucher_confirm_dialog->show();
        mc_systrayMenu_withdraw_asvoucher_confirm_dialog->setFocus();
    }else{
        //Show
        mc_systrayMenu_withdraw_asvoucher_confirm_dialog->show();
        mc_systrayMenu_withdraw_asvoucher_confirm_dialog->setFocus();
    }
}


//This is activated when the user clicks "Confirm amount"
void Moneychanger::mc_withdraw_asvoucher_confirm_amount_slot(){
    //Close the dialog/window
    mc_systrayMenu_withdraw_asvoucher_confirm_dialog->hide();
    
    //Collect require information to call the OT_ME::withdraw_cash(?,?,?) function
    QString selected_account_id = mc_systrayMenu_withdraw_asvoucher_account_dropdown->itemData(mc_systrayMenu_withdraw_asvoucher_account_dropdown->currentIndex()).toString();
    std::string selected_account_id_string = selected_account_id.toStdString();
    
    QString amount_to_withdraw_string = mc_systrayMenu_withdraw_asvoucher_amount_input->text();
    int64_t amount_to_withdraw_int = amount_to_withdraw_string.toInt();
    
    //Get Nym ID
    std::string nym_id = OTAPI_Wrap::GetAccountWallet_NymID(selected_account_id_string);
    
    //Get Server ID
    std::string selected_server_id_string = OTAPI_Wrap::GetAccountWallet_ServerID(selected_account_id_string);
    
    //Get receipent nym id
    std::string recip_nym_string = QString(mc_systrayMenu_withdraw_asvoucher_nym_input->text()).toStdString();
    
    //Get memo string
    std::string memo_string = QString(mc_systrayMenu_withdraw_asvoucher_memo_input->toPlainText()).toStdString();
    
    //Call OTAPI Withdraw voucher
    std::string withdraw_voucher_response = ot_me->withdraw_voucher(selected_server_id_string, nym_id, selected_account_id_string, recip_nym_string, memo_string, amount_to_withdraw_int);
    qDebug() << QString::fromStdString(withdraw_voucher_response);
    
}

//This is activated when the user clicks "cancel confirmation amount"
void Moneychanger::mc_withdraw_asvoucher_cancel_amount_slot(){
    //Close the dialog/window
    mc_systrayMenu_withdraw_asvoucher_confirm_dialog->hide();
}






//Deposit Slots

/**
 ** Deposit menu button clicked
 **/
void Moneychanger::mc_deposit_slot(){
    mc_deposit_show_dialog();
}


/**
 ** Deposit dialog
 **/
void Moneychanger::mc_deposit_show_dialog(){
    if(mc_deposit_already_init == 0){
        //Init deposit, then show.
        mc_deposit_dialog = new QDialog(0);
        mc_deposit_dialog->setWindowTitle("Deposit | Moneychanger");
        //Gridlayout
        mc_deposit_gridlayout = new QGridLayout(0);
        mc_deposit_gridlayout->setColumnStretch(0, 1);
        mc_deposit_gridlayout->setColumnStretch(1,0);
        mc_deposit_dialog->setLayout(mc_deposit_gridlayout);
        
        //Label (header)
        mc_deposit_header_label = new QLabel("<h1>Deposit</h1>");
        mc_deposit_header_label->setAlignment(Qt::AlignRight);
        mc_deposit_gridlayout->addWidget(mc_deposit_header_label, 0,1, 1,1);
        //Label ("Into Account") (subheader)
        mc_deposit_account_header_label = new QLabel("<h3>Into Account</h3>");
        mc_deposit_account_header_label->setAlignment(Qt::AlignRight);
        mc_deposit_gridlayout->addWidget(mc_deposit_account_header_label, 1,1, 1,1);
        
        //Label ("Into Purse") (subheader)
        mc_deposit_purse_header_label = new QLabel("<h3>Into Purse</h3>");
        mc_deposit_purse_header_label->setAlignment(Qt::AlignRight);
        mc_deposit_gridlayout->addWidget(mc_deposit_purse_header_label, 1,1, 1,1);
        mc_deposit_purse_header_label->hide();
        
        
        //Combobox (choose deposit type)
        mc_deposit_deposit_type = new QComboBox(0);
        mc_deposit_deposit_type->setStyleSheet("QComboBox{padding:1em;}");
        mc_deposit_gridlayout->addWidget(mc_deposit_deposit_type, 0,0, 1,1, Qt::AlignHCenter);
        mc_deposit_deposit_type->addItem("Deposit into your Account", QVariant(0));
        mc_deposit_deposit_type->addItem("Deposit into your Purse", QVariant(1));
        //connect "update" to switching open depsoit account/purse screens.
        connect(mc_deposit_deposit_type, SIGNAL(currentIndexChanged(int)), this, SLOT(mc_deposit_type_changed_slot(int)));
        
        /** Deposit into Account **/
        mc_deposit_account_widget = new QWidget(0);
        mc_deposit_account_layout = new QHBoxLayout(0);
        mc_deposit_account_widget->setLayout(mc_deposit_account_layout);
        mc_deposit_gridlayout->addWidget(mc_deposit_account_widget, 1,0, 1,1);
        //Add to account screen
        
        /** Deposit into Purse **/
        mc_deposit_purse_widget = new QWidget(0);
        mc_deposit_purse_layout = new QHBoxLayout(0);
        mc_deposit_purse_widget->setLayout(mc_deposit_purse_layout);
        mc_deposit_gridlayout->addWidget(mc_deposit_purse_widget, 1,0, 1,1);
        //Add to purse screen
        
        
        //Hide by default
        mc_deposit_purse_widget->hide();
    }
    //Resize
    mc_deposit_dialog->resize(600, 300);
    //Show
    mc_deposit_dialog->show();
}



void Moneychanger::mc_deposit_type_changed_slot(int newIndex){
    /** 0 = Account; 1 = purse **/
    if(newIndex == 0){
        //Show account, hide purse.
        mc_deposit_account_widget->show();
        mc_deposit_account_header_label->show();
        
        mc_deposit_purse_widget->hide();
        mc_deposit_purse_header_label->hide();
        
    }else if(newIndex == 1){
        //Hide account, show purse.
        mc_deposit_account_widget->hide();
        mc_deposit_account_header_label->hide();
        
        mc_deposit_purse_header_label->show();
        mc_deposit_purse_widget->show();
        
    }
}

//Send Funds slots
/**
 ** Send Funds Menu Button Clicked
 **/
void Moneychanger::mc_sendfunds_slot(){
    mc_sendfunds_show_dialog();
}

void Moneychanger::mc_requestfunds_slot(){
    mc_requestfunds_show_dialog();
}

void Moneychanger::mc_sendfunds_show_dialog(){
    if(mc_sendfunds_already_init == 0){
        mc_sendfunds_dialog = new QDialog(0);
        mc_sendfunds_gridlayout = new QGridLayout(0);
        mc_sendfunds_dialog->setLayout(mc_sendfunds_gridlayout);
        //Set window title
        mc_sendfunds_dialog->setWindowTitle("Send Funds | Moneychanger");
        
        //Content
        //Select sendfunds type
        mc_sendfunds_sendtype_combobox = new QComboBox(0);
        mc_sendfunds_sendtype_combobox->setStyleSheet("QComboBox{font-size:15pt;}");
        //Add selection options
        mc_sendfunds_sendtype_combobox->addItem("Send a Payment");
        mc_sendfunds_sendtype_combobox->addItem("Send a Cheque");
        mc_sendfunds_sendtype_combobox->addItem("Send Cash");
        mc_sendfunds_sendtype_combobox->addItem("Send an Account Transfer");
        
        mc_sendfunds_gridlayout->addWidget(mc_sendfunds_sendtype_combobox, 0,0, 1,1, Qt::AlignHCenter);
    }
    
    //Resize
    mc_sendfunds_dialog->resize(500, 300);
    
    //Show
    mc_sendfunds_dialog->show();
}

void Moneychanger::mc_requestfunds_show_dialog(){
    if(mc_requestfunds_already_init == 0){
        mc_requestfunds_dialog = new QDialog(0);
        mc_requestfunds_gridlayout = new QGridLayout(0);
        mc_requestfunds_dialog->setLayout(mc_requestfunds_gridlayout);
        //Set window title
        mc_requestfunds_dialog->setWindowTitle("Request Funds | Moneychanger");
        
        //Content
        //                                //Select requestfunds type
        //                                mc_requestfunds_requesttype_combobox = new QComboBox(0);
        //                                mc_requestfunds_requesttype_combobox->setStyleSheet("QComboBox{font-size:15pt;}");
        //                                    //Add selection options
        //                                    mc_requestfunds_requesttype_combobox->addItem("Write a Cheque");
        
        //                                mc_requestfunds_gridlayout->addWidget(mc_requestfunds_requesttype_combobox, 0,0, 1,1, Qt::AlignHCenter);
    }
    
    //Resize
    mc_requestfunds_dialog->resize(500, 300);
    
    //Show
    mc_requestfunds_dialog->show();
}
