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

#include <QFile>

#include "moneychanger.h"

#include "Handlers/contacthandler.h"
#include "Handlers/DBHandler.h"

#include "Widgets/detailedit.h"

#include "Widgets/senddlg.h"
#include "Widgets/requestdlg.h"

#include "Widgets/dlgchooser.h"

#include "UI/dlgimport.h"

#include "overridecursor.h"


#include "opentxs/OTAPI.h"
#include "opentxs/OT_ME.h"

/**
 * Constructor & Destructor
 **/

Moneychanger::Moneychanger(QWidget *parent)
: QWidget(parent),
  mc_overall_init(false),
  mc_overview_already_init(false),
  mc_market_window_already_init(false),
  mc_addressbook_already_init(false),
  mc_nymmanager_already_init(false),
  mc_assetmanager_already_init(false),
  mc_accountmanager_already_init(false),
  mc_servermanager_already_init(false),
  mc_sendfunds_already_init(false),
  mc_requestfunds_already_init(false),
  mc_createinsurancecompany_already_init(false),
  mc_settings_already_init(false),
  mc_agreement_already_init(false),
  mc_corporation_already_init(false),
  nym_list_id(NULL),
  nym_list_name(NULL),
  server_list_id(NULL),
  server_list_name(NULL),
  asset_list_id(NULL),
  asset_list_name(NULL),
  account_list_id(NULL),
  account_list_name(NULL)
{
    /**
     ** Init variables *
     **/
        
    //OT Related
    
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
//        if (default_nym_id.isEmpty() && (OTAPI_Wrap::GetNymCount() > 0))
//        {
//            default_nym_id = QString::fromStdString(OTAPI_Wrap::GetNym_ID(0));
//        }
//        // -------------------------------------------------
//        //Ask OT what the display name of this nym is and store it for quick retrieval later on(mostly for "Default Nym" displaying purposes)
//        if (!default_nym_id.isEmpty())
//        {
//            default_nym_name =  QString::fromStdString(OTAPI_Wrap::GetNym_Name(default_nym_id.toStdString()));
//        }
//        else
//            qDebug() << "Error loading DEFAULT NYM from SQL";
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
        // -------------------------------------------------
//        if (default_server_id.isEmpty() && (OTAPI_Wrap::GetServerCount() > 0))
//        {
//            default_server_id = QString::fromStdString(OTAPI_Wrap::GetServer_ID(0));
//        }
//        // -------------------------------------------------
//        //Ask OT what the display name of this server is and store it for a quick retrieval later on(mostly for "Default Server" displaying purposes)
//        if (!default_server_id.isEmpty())
//        {
//            default_server_name = QString::fromStdString(OTAPI_Wrap::GetServer_Name(default_server_id.toStdString()));
//        }
//        else
//            qDebug() << "Error loading DEFAULT SERVER from SQL";
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
        // -------------------------------------------------
//        if (default_asset_id.isEmpty() && (OTAPI_Wrap::GetAssetTypeCount() > 0))
//        {
//            default_asset_id = QString::fromStdString(OTAPI_Wrap::GetAssetType_ID(0));
//        }
//        // -------------------------------------------------
//        //Ask OT what the display name of this asset type is and store it for a quick retrieval later on(mostly for "Default Asset" displaying purposes)
//        if (!default_asset_id.isEmpty())
//        {
//            default_asset_name = QString::fromStdString(OTAPI_Wrap::GetAssetType_Name(default_asset_id.toStdString()));
//        }
//        else
//            qDebug() << "Error loading DEFAULT ASSET from SQL";
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
        // -------------------------------------------------
//        if (default_account_id.isEmpty() && (OTAPI_Wrap::GetAccountCount() > 0))
//        {
//            default_account_id = QString::fromStdString(OTAPI_Wrap::GetAccountWallet_ID(0));
//        }
//        // -------------------------------------------------
//        //Ask OT what the display name of this account is and store it for a quick retrieval later on(mostly for "Default Account" displaying purposes)
//        if (!default_account_id.isEmpty())
//        {
//            default_account_name = QString::fromStdString(OTAPI_Wrap::GetAccountWallet_Name(default_account_id.toStdString()));
//        }
//        else
//            qDebug() << "Error loading DEFAULT ACCOUNT from SQL";
    }



    qDebug() << "Database Populated";
    
    
    // ----------------------------------------------------------------------------

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
    
    mc_systrayIcon_nym = QIcon(":/icons/icons/identity_BW.png");
    mc_systrayIcon_server = QIcon(":/icons/server");
    
    mc_systrayIcon_goldaccount = QIcon(":/icons/icons/safe_box.png");
    mc_systrayIcon_purse = QIcon(":/icons/icons/assets.png");
    
    mc_systrayIcon_sendfunds = QIcon(":/icons/sendfunds");
    mc_systrayIcon_requestfunds = QIcon(":/icons/requestpayment");
    
    mc_systrayIcon_markets = QIcon(":/icons/markets");

    mc_systrayIcon_advanced = QIcon(":/icons/advanced");

    //Submenu
    mc_systrayIcon_advanced_import = QIcon(":/icons/icons/request.png");
    mc_systrayIcon_advanced_agreements = QIcon(":/icons/agreements");
    mc_systrayIcon_advanced_corporations = QIcon(":/icons/icons/buildings.png");
    mc_systrayIcon_advanced_settings = QIcon(":/icons/settings");
    
    // ----------------------------------------------------------------------------

    mc_overall_init = true;
}


Moneychanger::~Moneychanger()
{
    ClearMainMenu();
}

// ---------------------------------------------------------------


/**
 * Systray
 **/

// Startup
void Moneychanger::bootTray()
{
    SetupMainMenu();
    // ----------------------------------------------------------------------------
    //Show systray
    mc_systrayIcon->show();
    // ----------------------------------------------------------------------------
    // Pop up the home screen.
    mc_overview_dialog();

//  qDebug() << "BOOTING";
}


// Shutdown
void Moneychanger::mc_shutdown_slot()
{
    //Disconnect all signals from callin class (probubly main) to this class
    //Disconnect
    QObject::disconnect(this);
    //Close qt app (no need to deinit anything as of the time of this comment)
    //TO DO: Check if the OT queue caller is still proccessing calls.... Then quit the app. (Also tell user that the OT is still calling other wise they might think it froze during OT calls)
    qApp->quit();
}

// End Systray

// ---------------------------------------------------------------





void Moneychanger::ClearAssetMenu()
{
    // --------------------------------------------------
    if (mc_systrayMenu_asset)
    {
        mc_systrayMenu_asset->setParent(NULL);
        mc_systrayMenu_asset->disconnect();
        mc_systrayMenu_asset->deleteLater();

        mc_systrayMenu_asset.clear();
    }
    // --------------------------------------------------
    if (asset_list_id)
    {
        delete asset_list_id;
        asset_list_id = NULL;
    }

    if (asset_list_name)
    {
        delete asset_list_name;
        asset_list_name = NULL;
    }
    // --------------------------------------------------
}

void Moneychanger::ClearServerMenu()
{
    // --------------------------------------------------
    if (mc_systrayMenu_server)
    {
        mc_systrayMenu_server->setParent(NULL);
        mc_systrayMenu_server->disconnect();
        mc_systrayMenu_server->deleteLater();

        mc_systrayMenu_server.clear();
    }
    // --------------------------------------------------
    if (server_list_id)
    {
        delete server_list_id;
        server_list_id = NULL;
    }

    if (server_list_name)
    {
        delete server_list_name;
        server_list_name = NULL;
    }
    // --------------------------------------------------
}

void Moneychanger::ClearNymMenu()
{
    // --------------------------------------------------
    if (mc_systrayMenu_nym)
    {
        mc_systrayMenu_nym->setParent(NULL);
        mc_systrayMenu_nym->disconnect();
        mc_systrayMenu_nym->deleteLater();

        mc_systrayMenu_nym.clear();
    }
    // --------------------------------------------------
    if (nym_list_id)
    {
        delete nym_list_id;
        nym_list_id = NULL;
    }

    if (nym_list_name)
    {
        delete nym_list_name;
        nym_list_name = NULL;
    }
    // --------------------------------------------------
}


void Moneychanger::ClearAccountMenu()
{
    // --------------------------------------------------
    if (mc_systrayMenu_account)
    {
        mc_systrayMenu_account->setParent(NULL);
        mc_systrayMenu_account->disconnect();
        mc_systrayMenu_account->deleteLater();

        mc_systrayMenu_account.clear();
    }
    // --------------------------------------------------
    if (account_list_id)
    {
        delete account_list_id;
        account_list_id = NULL;
    }

    if (account_list_name)
    {
        delete account_list_name;
        account_list_name = NULL;
    }
    // --------------------------------------------------
}

void Moneychanger::SetupMainMenu()
{
    if (!mc_overall_init)
        return;

    ClearMainMenu();

    // -----------------------------------

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
    SetupAccountMenu();
    // --------------------------------------------------------------
    //Asset section
    SetupAssetMenu();
    // --------------------------------------------------------------
    //Separator
    mc_systrayMenu->addSeparator();
    // --------------------------------------------------------------
    mc_systrayMenu_markets = new QAction(mc_systrayIcon_markets, tr("Markets"), 0);
    mc_systrayMenu->addAction(mc_systrayMenu_markets);
    connect(mc_systrayMenu_markets, SIGNAL(triggered()), this, SLOT(mc_market_slot()));
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
    // --------------------------------------------------------------

    mc_systrayMenu_advanced_import = new QAction(mc_systrayIcon_advanced_import, tr("Import Cash..."), 0);
    mc_systrayMenu_advanced->addAction(mc_systrayMenu_advanced_import);
    connect(mc_systrayMenu_advanced_import, SIGNAL(triggered()), this, SLOT(mc_import_slot()));
    // --------------------------------------------------------------
    //Separator
    mc_systrayMenu_advanced->addSeparator();
    // ------------------------------------------------

    mc_systrayMenu_advanced_agreements = new QAction(mc_systrayIcon_advanced_agreements, tr("Agreements"), 0);
    mc_systrayMenu_advanced->addAction(mc_systrayMenu_advanced_agreements);
    connect(mc_systrayMenu_advanced_agreements, SIGNAL(triggered()), this, SLOT(mc_agreement_slot()));
    // --------------------------------------------------------------
    // Corporations
    mc_systrayMenu_advanced_corporations = new QAction(mc_systrayIcon_advanced_corporations, tr("Corporations"), 0);
    mc_systrayMenu_advanced->addAction(mc_systrayMenu_advanced_corporations);
    connect(mc_systrayMenu_advanced_corporations, SIGNAL(triggered()), this, SLOT(mc_corporation_slot()));

    // --------------------------------------------------------------

    //Separator
    mc_systrayMenu_advanced->addSeparator();
    // --------------------------------------------------------------

    // Corporations submenu
//    mc_systrayMenu_company_create = new QMenu(tr("Create"), 0);
//    mc_systrayMenu_advanced_corporations->addMenu(mc_systrayMenu_company_create);

//    // Create insurance company action on submenu
//    mc_systrayMenu_company_create_insurance = new QAction(mc_systrayIcon_advanced_agreements, tr("Insurance Company"), 0);
//    mc_systrayMenu_company_create->addAction(mc_systrayMenu_company_create_insurance);
//    connect(mc_systrayMenu_company_create_insurance, SIGNAL(triggered()), this, SLOT(mc_createinsurancecompany_slot()));
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
    connect(mc_systrayMenu_advanced_settings, SIGNAL(triggered()), this, SLOT(mc_settings_slot()));

    // --------------------------------------------------------------
    //Separator
    mc_systrayMenu_advanced->addSeparator();
    // --------------------------------------------------------------

    // TODO: If the default isn't set, then choose the first one and select it.

    // TODO: If there isn't even ONE to select, then this menu item should say "Create Nym..." with no sub-menu.

    // TODO: When booting up, if there is already a default server and asset id, but no nyms exist, create a default nym.

    // TODO: When booting up, if there is already a default nym, but no accounts exist, create a default account.

    // --------------------------------------------------------------
    //Nym section
    SetupNymMenu();
    // --------------------------------------------------------------
    //Server section
    SetupServerMenu();
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
    //
    mc_systrayIcon->setContextMenu(mc_systrayMenu);
}

void Moneychanger::ClearMainMenu()
{
    if (mc_systrayMenu_headertext)
    {
        mc_systrayMenu_headertext->setParent(NULL);
        mc_systrayMenu_headertext->disconnect();
        mc_systrayMenu_headertext->deleteLater();

        mc_systrayMenu_headertext.clear();
    }
    // --------------------------------------------------------------
    if (mc_systrayMenu_overview)
    {
        mc_systrayMenu_overview->setParent(NULL);
        mc_systrayMenu_overview->disconnect();
        mc_systrayMenu_overview->deleteLater();

        mc_systrayMenu_overview.clear();

    }
    // --------------------------------------------------------------
    if (mc_systrayMenu_sendfunds)
    {
        mc_systrayMenu_sendfunds->setParent(NULL);
        mc_systrayMenu_sendfunds->disconnect();
        mc_systrayMenu_sendfunds->deleteLater();

        mc_systrayMenu_sendfunds.clear();
    }
    // --------------------------------------------------------------
    if (mc_systrayMenu_requestfunds)
    {
        mc_systrayMenu_requestfunds->setParent(NULL);
        mc_systrayMenu_requestfunds->disconnect();
        mc_systrayMenu_requestfunds->deleteLater();

        mc_systrayMenu_requestfunds.clear();
    }
    // --------------------------------------------------------------
    ClearAccountMenu();
    ClearAssetMenu();
    // --------------------------------------------------------------
    if (mc_systrayMenu_advanced)
    {
        mc_systrayMenu_advanced->setParent(NULL);
        mc_systrayMenu_advanced->disconnect();
        mc_systrayMenu_advanced->deleteLater();

        mc_systrayMenu_advanced.clear();
    }
    // --------------------------------------------------------------
    if (mc_systrayMenu_advanced_agreements)
    {
        mc_systrayMenu_advanced_agreements->setParent(NULL);
        mc_systrayMenu_advanced_agreements->disconnect();
        mc_systrayMenu_advanced_agreements->deleteLater();

        mc_systrayMenu_advanced_agreements.clear();
    }
    // --------------------------------------------------------------
    if (mc_systrayMenu_advanced_corporations)
    {
        mc_systrayMenu_advanced_corporations->setParent(NULL);
        mc_systrayMenu_advanced_corporations->disconnect();
        mc_systrayMenu_advanced_corporations->deleteLater();

        mc_systrayMenu_advanced_corporations.clear();
    }
    // --------------------------------------------------------------
    if (mc_systrayMenu_company_create)
    {
        mc_systrayMenu_company_create->setParent(NULL);
        mc_systrayMenu_company_create->disconnect();
        mc_systrayMenu_company_create->deleteLater();

        mc_systrayMenu_company_create.clear();
    }
    // --------------------------------------------------------------
    if (mc_systrayMenu_company_create_insurance)
    {
        mc_systrayMenu_company_create_insurance->setParent(NULL);
        mc_systrayMenu_company_create_insurance->disconnect();
        mc_systrayMenu_company_create_insurance->deleteLater();

        mc_systrayMenu_company_create_insurance.clear();
    }
    // --------------------------------------------------------------
    if (mc_systrayMenu_advanced_bazaar)
    {
        mc_systrayMenu_advanced_bazaar->setParent(NULL);
        mc_systrayMenu_advanced_bazaar->disconnect();
        mc_systrayMenu_advanced_bazaar->deleteLater();

        mc_systrayMenu_advanced_bazaar.clear();
    }
    // --------------------------------------------------------------
    if (mc_systrayMenu_bazaar_search)
    {
        mc_systrayMenu_bazaar_search->setParent(NULL);
        mc_systrayMenu_bazaar_search->disconnect();
        mc_systrayMenu_bazaar_search->deleteLater();

        mc_systrayMenu_bazaar_search.clear();
    }
    // --------------------------------------------------------------
    if (mc_systrayMenu_bazaar_post)
    {
        mc_systrayMenu_bazaar_post->setParent(NULL);
        mc_systrayMenu_bazaar_post->disconnect();
        mc_systrayMenu_bazaar_post->deleteLater();

        mc_systrayMenu_bazaar_post.clear();
    }
    // --------------------------------------------------------------
    if (mc_systrayMenu_bazaar_orders)
    {
        mc_systrayMenu_bazaar_orders->setParent(NULL);
        mc_systrayMenu_bazaar_orders->disconnect();
        mc_systrayMenu_bazaar_orders->deleteLater();

        mc_systrayMenu_bazaar_orders.clear();
    }
    // --------------------------------------------------------------
    if (mc_systrayMenu_advanced_settings)
    {
        mc_systrayMenu_advanced_settings->setParent(NULL);
        mc_systrayMenu_advanced_settings->disconnect();
        mc_systrayMenu_advanced_settings->deleteLater();

        mc_systrayMenu_advanced_settings.clear();
    }
    // --------------------------------------------------------------
    ClearNymMenu();
    // --------------------------------------------------------------
    ClearServerMenu();
    // --------------------------------------------------------------
    if (mc_systrayMenu_shutdown)
    {
        mc_systrayMenu_shutdown->setParent(NULL);
        mc_systrayMenu_shutdown->disconnect();
        mc_systrayMenu_shutdown->deleteLater();

        mc_systrayMenu_shutdown.clear();
    }
    // --------------------------------------------------------------
    if (mc_systrayMenu)
    {
        mc_systrayMenu->setParent(NULL);
        mc_systrayMenu->disconnect();
        mc_systrayMenu->deleteLater();

        mc_systrayMenu.clear();
    }
    // --------------------------------------------------------------
}


void Moneychanger::SetupAssetMenu()
{
    mc_systrayMenu_asset = new QMenu(tr("Set Default Asset Type..."), 0);
    mc_systrayMenu_asset->setIcon(mc_systrayIcon_purse);
    mc_systrayMenu->addMenu(mc_systrayMenu_asset);
    // --------------------------------------------------
    //Add a "Manage asset types" action button (and connection)
    QAction * manage_assets = new QAction(tr("Manage Asset Contracts..."), 0);
    manage_assets->setData(QVariant(QString("openmanager")));

    mc_systrayMenu_asset->addAction(manage_assets);
    mc_systrayMenu_asset->addSeparator();

    //Add reaction to the "asset" action.
    connect(mc_systrayMenu_asset, SIGNAL(triggered(QAction*)), this, SLOT(mc_assetselection_triggered(QAction*)));

    // -------------------------------------------------
    if (default_asset_id.isEmpty() && (OTAPI_Wrap::GetAssetTypeCount() > 0))
    {
        default_asset_id = QString::fromStdString(OTAPI_Wrap::GetAssetType_ID(0));
    }
    // -------------------------------------------------
    if (!default_asset_id.isEmpty())
    {
        default_asset_name = QString::fromStdString(OTAPI_Wrap::GetAssetType_Name(default_asset_id.toStdString()));
    }
    // -------------------------------------------------
    //Load "default" asset type
    setDefaultAsset(default_asset_id, default_asset_name);

    //Init asset submenu
    asset_list_id   = new QList<QVariant>();
    asset_list_name = new QList<QVariant>();
    // ------------------------------------------
    mc_systrayMenu_reload_assetlist();
}

void Moneychanger::SetupServerMenu()
{
    mc_systrayMenu_server = new QMenu(tr("Set Default Server..."), 0);
    mc_systrayMenu_server->setIcon(mc_systrayIcon_server);
    mc_systrayMenu_advanced->addMenu(mc_systrayMenu_server);
    // --------------------------------------------------

    //Add a "Manage Servers" action button (and connection)
    QAction * manage_servers = new QAction(tr("Manage Servers..."), 0);
    manage_servers->setData(QVariant(QString("openmanager")));

    mc_systrayMenu_server->addAction(manage_servers);
    mc_systrayMenu_server->addSeparator();

    //Add reaction to the "server" action.
    connect(mc_systrayMenu_server, SIGNAL(triggered(QAction*)), this, SLOT(mc_serverselection_triggered(QAction*)));

    // -------------------------------------------------
    if (default_server_id.isEmpty() && (OTAPI_Wrap::GetServerCount() > 0))
    {
        default_server_id = QString::fromStdString(OTAPI_Wrap::GetServer_ID(0));
    }
    // -------------------------------------------------
    //Ask OT what the display name of this server is and store it for a quick retrieval later on(mostly for "Default Server" displaying purposes)
    if (!default_server_id.isEmpty())
    {
        default_server_name = QString::fromStdString(OTAPI_Wrap::GetServer_Name(default_server_id.toStdString()));
    }
    // -------------------------------------------------
    //Load "default" server
    setDefaultServer(default_server_id, default_server_name);

    //Init server submenu
    server_list_id   = new QList<QVariant>();
    server_list_name = new QList<QVariant>();
    // ------------------------------------------
    mc_systrayMenu_reload_serverlist();
}

void Moneychanger::SetupNymMenu()
{
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

    // -------------------------------------------------
    if (default_nym_id.isEmpty() && (OTAPI_Wrap::GetNymCount() > 0))
    {
        default_nym_id = QString::fromStdString(OTAPI_Wrap::GetNym_ID(0));
    }
    // -------------------------------------------------
    //Ask OT what the display name of this nym is and store it for quick retrieval later on(mostly for "Default Nym" displaying purposes)
    if (!default_nym_id.isEmpty())
    {
        default_nym_name =  QString::fromStdString(OTAPI_Wrap::GetNym_Name(default_nym_id.toStdString()));
    }
    // -------------------------------------------------
    //Load "default" nym
    setDefaultNym(default_nym_id, default_nym_name);

    //Init nym submenu
    nym_list_id = new QList<QVariant>();
    nym_list_name = new QList<QVariant>();
    // ------------------------------------------
    mc_systrayMenu_reload_nymlist();
}

void Moneychanger::SetupAccountMenu()
{
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
    // -------------------------------------------------
    if (default_account_id.isEmpty() && (OTAPI_Wrap::GetAccountCount() > 0))
    {
        default_account_id = QString::fromStdString(OTAPI_Wrap::GetAccountWallet_ID(0));
    }
    // -------------------------------------------------
    //Ask OT what the display name of this account is and store it for a quick retrieval later on(mostly for "Default Account" displaying purposes)
    if (!default_account_id.isEmpty())
    {
        default_account_name = QString::fromStdString(OTAPI_Wrap::GetAccountWallet_Name(default_account_id.toStdString()));
    }
    // -------------------------------------------------
    //Load "default" account
    setDefaultAccount(default_account_id, default_account_name);

    //Init account submenu
    account_list_id   = new QList<QVariant>();
    account_list_name = new QList<QVariant>();
    // ------------------------------------------
    mc_systrayMenu_reload_accountlist();
}









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
 * Address Book 
 **/

void Moneychanger::mc_addressbook_show(QString text) // text may contain a "pre-selected" Contact ID (an integer in string form.)
{
    // The caller dosen't wish to have the address book paste to anything
    // (they just want to see/manage the address book), just call blank.
    //
    if (!mc_addressbook_already_init)
    {
        contactswindow = new MTDetailEdit(this, *this);

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
    if (contactswindow)
    {
        contactswindow->setParent(NULL);
        contactswindow->disconnect();
        contactswindow->deleteLater();

        contactswindow.clear();
    }

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

void Moneychanger::mc_nymmanager_dialog(QString qstrPresetID/*=QString("")*/)
{
    QString qstr_default_id = this->get_default_nym_id();
    // -------------------------------------
    if (qstrPresetID.isEmpty())
        qstrPresetID = qstr_default_id;
    // -------------------------------------
    if (!mc_nymmanager_already_init)
    {
        nymswindow = new MTDetailEdit(this, *this);

        mc_nymmanager_already_init = true;
        qDebug() << "Nym Manager Opened";
    }
    // -------------------------------------
    mapIDName & the_map = nymswindow->m_map;
    // -------------------------------------
    the_map.clear();
    // -------------------------------------
    int32_t nym_count = OTAPI_Wrap::GetNymCount();
    bool bFoundPreset = false;

    for (int32_t ii = 0; ii < nym_count; ii++)
    {
        QString OT_id   = QString::fromStdString(OTAPI_Wrap::GetNym_ID(ii));
        QString OT_name = QString::fromStdString(OTAPI_Wrap::GetNym_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);
        // ------------------------------
        if (!qstrPresetID.isEmpty() && (qstrPresetID == OT_id))
            bFoundPreset = true;
        // ------------------------------
    } // for
    // -------------------------------------
    nymswindow->setWindowTitle(tr("Manage Nyms"));
    // -------------------------------------
    if (bFoundPreset)
        nymswindow->SetPreSelected(qstrPresetID);
    // -------------------------------------
    nymswindow->dialog(MTDetailEdit::DetailEditTypeNym);
}


void Moneychanger::close_nymmanager_dialog()
{
    if (nymswindow)
    {
        nymswindow->setParent(NULL);
        nymswindow->disconnect();
        nymswindow->deleteLater();

        nymswindow.clear();
    }
    // --------------------------------
    mc_nymmanager_already_init = false;
    // --------------------------------
    qDebug() << "Nym Manager Closed";
}

//Additional Nym Manager Functions
void Moneychanger::setDefaultNym(QString nym_id, QString nym_name)
{
    //Set default nym internal memory
    default_nym_id   = nym_id;
    default_nym_name = nym_name;
    
    //SQL UPDATE default nym
    DBHandler::getInstance()->AddressBookUpdateDefaultNym(nym_id);
    
    //Rename "NYM:" if a nym is loaded
    if (!nym_id.isEmpty())
    {
        mc_systrayMenu_nym->setTitle(tr("Nym: ")+nym_name);
    }
}

void Moneychanger::mc_systrayMenu_reload_nymlist()
{
//  qDebug() << "RELOAD NYM LIST";
    // --------------------------------------------------------
    //Count nyms
    int32_t nym_count = OTAPI_Wrap::GetNymCount();

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
        setDefaultNym(action_triggered_string, action_triggered_string_nym_name);
        
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
                MTOverrideCursor theSpinner;

                std::string response = madeEasy.register_nym(defaultServerId, defaultNymID);
                qDebug() << QString("Creation Response: %1").arg(QString::fromStdString(response));
            }

            {
                MTOverrideCursor theSpinner;

                madeEasy.retrieve_nym(defaultServerId, defaultNymID, true);
            }
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
                std::string response;
                {
                    MTOverrideCursor theSpinner;
                    response = madeEasy.create_asset_acct(defaultServerId, defaultNymID, defaultAssetId);
                }
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

            {
                MTOverrideCursor theSpinner;
                madeEasy.retrieve_account(acctSvrID, acctNymID, accountId, true);
            }

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


void Moneychanger::mc_assetmanager_dialog(QString qstrPresetID/*=QString("")*/)
{
    QString qstr_default_id = this->get_default_asset_id();
    // -------------------------------------
    if (qstrPresetID.isEmpty())
        qstrPresetID = qstr_default_id;
    // -------------------------------------
    if (!mc_assetmanager_already_init)
    {
        assetswindow = new MTDetailEdit(this, *this);

        mc_assetmanager_already_init = true;
        qDebug() << "Asset Manager Opened";
    }
    // -------------------------------------
    mapIDName & the_map = assetswindow->m_map;
    // -------------------------------------
    the_map.clear();
    // -------------------------------------
    int32_t  asset_count = OTAPI_Wrap::GetAssetTypeCount();
    bool    bFoundPreset = false;

    for (int32_t ii = 0; ii < asset_count; ii++)
    {
        QString OT_id   = QString::fromStdString(OTAPI_Wrap::GetAssetType_ID(ii));
        QString OT_name = QString::fromStdString(OTAPI_Wrap::GetAssetType_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);
        // ------------------------------
        if (!qstrPresetID.isEmpty() && (qstrPresetID == OT_id))
            bFoundPreset = true;
        // ------------------------------
    } // for
    // -------------------------------------
    assetswindow->setWindowTitle(tr("Asset Types"));
    // -------------------------------------
    if (bFoundPreset)
        assetswindow->SetPreSelected(qstrPresetID);
    // -------------------------------------
    assetswindow->dialog(MTDetailEdit::DetailEditTypeAsset);
}


void Moneychanger::close_assetmanager_dialog()
{
    if (assetswindow)
    {
        assetswindow->setParent(NULL);
        assetswindow->disconnect();
        assetswindow->deleteLater();

        assetswindow.clear();
    }
    // --------------------------------
    mc_assetmanager_already_init = false;
    // --------------------------------
    qDebug() << "Asset Manager Closed";
}








//Additional Asset slots

//This was mistakenly named asset_load_asset, should be set default asset
//Set Default asset
void Moneychanger::setDefaultAsset(QString asset_id, QString asset_name)
{
    //Set default asset internal memory
    default_asset_id = asset_id;
    default_asset_name = asset_name;
    
    //SQL UPDATE default asset
    DBHandler::getInstance()->AddressBookUpdateDefaultAsset(asset_id);
    
    //Rename "ASSET:" if a asset is loaded
    if (!asset_id.isEmpty())
    {
        mc_systrayMenu_asset->setTitle(tr("Asset Type: ")+asset_name);
    }
}

void Moneychanger::mc_systrayMenu_reload_assetlist()
{
//  qDebug() << "RELOAD asset LIST";
    // -------------------------------------------
    //Add/append to the id + name lists
    //
    int32_t asset_count = OTAPI_Wrap::GetAssetTypeCount();

    for (int aa = 0; aa < asset_count; aa++)
    {
        //Get OT Account ID
        QString OT_asset_id = QString::fromStdString(OTAPI_Wrap::GetAssetType_ID(aa));
        
        //Add to qlist
        asset_list_id->append(QVariant(OT_asset_id));
        
        //Get OT Account Name
        QString OT_asset_name = QString::fromStdString(OTAPI_Wrap::GetAssetType_Name(OT_asset_id.toStdString()));
        
        //Add to qlist
        asset_list_name->append(QVariant(OT_asset_name));
        
        //Append to submenu of asset
        QAction * next_asset_action = new QAction(mc_systrayIcon_purse, OT_asset_name, 0);
        next_asset_action->setData(QVariant(OT_asset_id));
        mc_systrayMenu_asset->addAction(next_asset_action);
    }
}

//Asset new default selected from systray
void Moneychanger::mc_assetselection_triggered(QAction*action_triggered)
{
    //Check if the user wants to open the asset manager (or) select a different default asset
    QString action_triggered_string = QVariant(action_triggered->data()).toString();
    qDebug() << "asset TRIGGERED" << action_triggered_string;
    if(action_triggered_string == "openmanager"){
        //Open asset manager
        mc_defaultasset_slot();
    }else{
        //Set new asset default
        QString action_triggered_string_asset_name = QVariant(action_triggered->text()).toString();
        setDefaultAsset(action_triggered_string, action_triggered_string_asset_name);
        
        //Refresh if the asset manager is currently open
        if (mc_assetmanager_already_init)
        {
            mc_assetmanager_dialog();
        }
    }
    
}

// End Asset Manager






void Moneychanger::onBalancesChanged()
{
    emit balancesChanged();
}


/** 
 * Account Manager 
 **/

//Account manager "clicked"
void Moneychanger::mc_defaultaccount_slot()
{
    //The operator has requested to open the dialog to the "account Manager";
    mc_accountmanager_dialog();
}


void Moneychanger::mc_accountmanager_dialog(QString qstrAcctID/*=QString("")*/)
{
    QString qstr_default_acct_id = this->get_default_account_id();
    // -------------------------------------
    if (qstrAcctID.isEmpty())
        qstrAcctID = qstr_default_acct_id;
    // -------------------------------------
    if (!mc_accountmanager_already_init)
    {
        accountswindow = new MTDetailEdit(this, *this);

        // When the accountswindow signal "balancesChanged" is triggered,
        // it will call Moneychanger's "onBalancesChanged" function.
        // (Which will trigger Moneychanger's "balancesChanged" signal.)
        //
        connect(accountswindow, SIGNAL(balancesChanged()),
                this,           SLOT(onBalancesChanged()));

        // When Moneychanger's signal "balancesChanged" is triggered,
        // it will call accountswindow's "onBalancesChangedFromAbove" function.
        //
        connect(this,           SIGNAL(balancesChanged()),
                accountswindow, SLOT(onBalancesChangedFromAbove()));

        mc_accountmanager_already_init = true;
        qDebug() << "Account Manager Opened";
    }
    // -------------------------------------
    mapIDName & the_map = accountswindow->m_map;
    // -------------------------------------
    the_map.clear();
    // -------------------------------------
    int32_t acct_count = OTAPI_Wrap::GetAccountCount();
    bool bFoundDefault = false;

    for (int32_t ii = 0; ii < acct_count; ii++)
    {
        QString OT_id   = QString::fromStdString(OTAPI_Wrap::GetAccountWallet_ID(ii));
        QString OT_name = QString::fromStdString(OTAPI_Wrap::GetAccountWallet_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);
        // ------------------------------
        if (!qstrAcctID.isEmpty() && (qstrAcctID == OT_id))
            bFoundDefault = true;
        // ------------------------------
    } // for
    // -------------------------------------
    accountswindow->setWindowTitle(tr("Manage Accounts"));
    // -------------------------------------
    if (bFoundDefault)
        accountswindow->SetPreSelected(qstrAcctID);
    // -------------------------------------
    accountswindow->dialog(MTDetailEdit::DetailEditTypeAccount);
}

void Moneychanger::close_accountmanager_dialog()
{
    if (accountswindow)
    {
        accountswindow->setParent(NULL);
        accountswindow->disconnect();
        accountswindow->deleteLater();

        accountswindow.clear();
    }
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

    if (action_triggered_string == "openmanager")
    {
        //Open account manager
        mc_defaultaccount_slot();
    }
    else
    {
        //Set new account default
        QString action_triggered_string_account_name = QVariant(action_triggered->text()).toString();
        setDefaultAccount(action_triggered_string, action_triggered_string_account_name);
        // ------------------------------
        //Refresh the account default selection in the account manager (ONLY if it is open)
        //Check if account manager has ever been opened (then apply logic) [prevents crash if the dialog hasen't be opend before]
        //
        if (mc_accountmanager_already_init)
        {
            mc_accountmanager_dialog();
        }
        // ------------------------------
        mc_overview_dialog_refresh();
        // ------------------------------
    }
}

//Set Default account
void Moneychanger::setDefaultAccount(QString account_id, QString account_name)
{
    //Set default account internal memory
    default_account_id   = account_id;
    default_account_name = account_name;
    
    //SQL UPDATE default account
    DBHandler::getInstance()->AddressBookUpdateDefaultAccount(account_id);
    
    //Rename "ACCOUNT:" if a account is loaded
    if (account_id != "")
    {
        QString result = tr("Account: ") + account_name;
        
        int64_t     lBalance  = OTAPI_Wrap::GetAccountWallet_Balance    (account_id.toStdString());
        std::string strAsset  = OTAPI_Wrap::GetAccountWallet_AssetTypeID(account_id.toStdString());
        // ----------------------------------------------------------
        std::string str_amount;
        
        if (!strAsset.empty())
        {
            str_amount = OTAPI_Wrap::FormatAmount(strAsset, lBalance);
            result += " ("+ QString::fromStdString(str_amount) +")";
        }
        
        mc_systrayMenu_account->setTitle(result);
        // -----------------------------------------------------------
        std::string strNym    = OTAPI_Wrap::GetAccountWallet_NymID   (account_id.toStdString());
        std::string strServer = OTAPI_Wrap::GetAccountWallet_ServerID(account_id.toStdString());

        if (!strAsset.empty())
            DBHandler::getInstance()->AddressBookUpdateDefaultAsset (QString::fromStdString(strAsset));
        if (!strNym.empty())
            DBHandler::getInstance()->AddressBookUpdateDefaultNym   (QString::fromStdString(strNym));
        if (!strServer.empty())
            DBHandler::getInstance()->AddressBookUpdateDefaultServer(QString::fromStdString(strServer));

        if (mc_overall_init)
        {
            // -----------------------------------------------------------
            if (!strAsset.empty())
            {
                QString qstrAssetName = QString::fromStdString(OTAPI_Wrap::GetAssetType_Name(strAsset));

                if (!qstrAssetName.isEmpty() && (mc_systrayMenu_asset))
                    mc_systrayMenu_asset->setTitle(tr("Asset Type: ")+qstrAssetName);
//                  setDefaultAsset(QString::fromStdString(strAsset),
//                                  QString::fromStdString(strAssetName));
            }
            // -----------------------------------------------------------
            if (!strNym.empty())
            {
                QString qstrNymName = QString::fromStdString(OTAPI_Wrap::GetNym_Name(strNym));

                if (!qstrNymName.isEmpty() && (mc_systrayMenu_nym))
                    mc_systrayMenu_nym->setTitle(tr("Nym: ")+qstrNymName);
//                  setDefaultNym(QString::fromStdString(strNym),
//                                QString::fromStdString(strNymName));
            }
            // -----------------------------------------------------------
            if (!strServer.empty())
            {
                QString qstrServerName = QString::fromStdString(OTAPI_Wrap::GetServer_Name(strServer));

                if (!qstrServerName.isEmpty() && (mc_systrayMenu_server))
                    mc_systrayMenu_server->setTitle(tr("Server: ")+qstrServerName);
//                  setDefaultServer(QString::fromStdString(strServer),
//                                   QString::fromStdString(strServerName));
            }
            // -----------------------------------------------------------
            mc_overview_dialog_refresh();
        }
        // -----------------------------------------------------------
    }
}

void Moneychanger::mc_systrayMenu_reload_accountlist()
{
//  qDebug() << "RELOAD account LIST";
    // ---------------------------------------------
    //Add/append to the id + name lists
    //
    int32_t account_count = OTAPI_Wrap::GetAccountCount();

    for (int aa = 0; aa < account_count; aa++)
    {
        //Get OT Account ID
        QString OT_account_id = QString::fromStdString(OTAPI_Wrap::GetAccountWallet_ID(aa));
        
        //Add to qlist
        account_list_id->append(QVariant(OT_account_id));
        
        //Get OT Account Name
        QString OT_account_name = QString::fromStdString(OTAPI_Wrap::GetAccountWallet_Name(OT_account_id.toStdString()));
        
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


void Moneychanger::mc_servermanager_dialog(QString qstrPresetID/*=QString("")*/)
{
    QString qstr_default_id = this->get_default_server_id();
    // -------------------------------------
    if (qstrPresetID.isEmpty())
        qstrPresetID = qstr_default_id;
    // -------------------------------------
    if (!mc_servermanager_already_init)
    {
        serverswindow = new MTDetailEdit(this, *this);

        mc_servermanager_already_init = true;
        qDebug() << "Server Manager Opened";
    }
    // -------------------------------------
    mapIDName & the_map = serverswindow->m_map;
    // -------------------------------------
    the_map.clear();
    // -------------------------------------
    int32_t server_count = OTAPI_Wrap::GetServerCount();
    bool    bFoundPreset = false;

    for (int32_t ii = 0; ii < server_count; ii++)
    {
        QString OT_id   = QString::fromStdString(OTAPI_Wrap::GetServer_ID(ii));
        QString OT_name = QString::fromStdString(OTAPI_Wrap::GetServer_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);
        // ------------------------------
        if (!qstrPresetID.isEmpty() && (qstrPresetID == OT_id))
            bFoundPreset = true;
        // ------------------------------
    } // for
    // -------------------------------------
    serverswindow->setWindowTitle(tr("Server Contracts"));
    // -------------------------------------
    if (bFoundPreset)
        serverswindow->SetPreSelected(qstrPresetID);
    // -------------------------------------
    serverswindow->dialog(MTDetailEdit::DetailEditTypeServer);
}


void Moneychanger::close_servermanager_dialog()
{
    if (serverswindow)
    {
        serverswindow->setParent(NULL);
        serverswindow->disconnect();
        serverswindow->deleteLater();

        serverswindow.clear();
    }
    // --------------------------------
    mc_servermanager_already_init = false;
    // --------------------------------
    qDebug() << "Server Manager Closed";
}






void Moneychanger::setDefaultServer(QString server_id, QString server_name)
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

    if (new_server_title.isEmpty())
    {
        new_server_title = tr("Set Default...");
    }
    
    mc_systrayMenu_server->setTitle(tr("Server: ")+new_server_title);
}

void Moneychanger::mc_systrayMenu_reload_serverlist()
{
//  qDebug() << "RELOAD SERVER LIST";
    // -----------------------------------------------
    //Add/append to the id + name lists
    //
    int32_t server_count = OTAPI_Wrap::GetServerCount();

    for (int32_t aa = 0; aa < server_count; aa++)
    {
        //Get OT server ID
        QString OT_server_id = QString::fromStdString(OTAPI_Wrap::GetServer_ID(aa));
        
        //Add to qlist
        server_list_id->append(QVariant(OT_server_id));
        
        //Get OT server Name
        QString OT_server_name = QString::fromStdString(OTAPI_Wrap::GetServer_Name(OT_server_id.toStdString()));
        
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
        setDefaultServer(action_triggered_string, action_triggered_string_server_name);
        
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
 * Request Funds 
 **/

void Moneychanger::mc_requestfunds_slot()
{
    mc_requestfunds_show_dialog();
}

void Moneychanger::mc_requestfunds_show_dialog()
{
    // --------------------------------------------------
    MTRequestDlg * request_window = new MTRequestDlg(NULL, *this);
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

}


// End Request Funds









/**
 * Import Cash
 **/

void Moneychanger::mc_import_slot()
{
    DlgImport dlgImport;

    if (QDialog::Accepted != dlgImport.exec())
        return;
    // ----------------------------------------
    QString qstrContents;
    // ----------------------------------------
    if (dlgImport.IsPasted()) // Pasted contents (not filename)
        qstrContents = dlgImport.GetPasted(); // Dialog prohibits empty contents, so no need to check here.
    // --------------------------------
    else  // Filename
    {
        QString fileName = dlgImport.GetFilename(); // Dialog prohibits empty filename, so no need to check here.
        // -----------------------------------------------
        QFile   plainFile(fileName);

        if (plainFile.open(QIODevice::ReadOnly))//| QIODevice::Text)) // Text flag translates /n/r to /n
        {
            QTextStream in(&plainFile); // Todo security: check filesize here and place a maximum size.
            qstrContents = in.readAll();

            plainFile.close();
            // ----------------------------
            if (qstrContents.isEmpty())
            {
                QMessageBox::warning(this, tr("File Was Empty"),
                                     QString("%1: %2").arg(tr("File was apparently empty")).arg(fileName));
                return;
            }
            // ----------------------------
        }
        else
        {
            QMessageBox::warning(this, tr("Failed Reading File"),
                                 QString("%1: %2").arg(tr("Failed trying to read file")).arg(fileName));
            return;
        }
    }
    // --------------------------------
    // By this point, we have the contents of the instrument,
    // and we know they aren't empty.
    //
    std::string strInstrument = qstrContents.toStdString();
    // ---------------------------------------------
    std::string strType = OTAPI_Wrap::Instrmnt_GetType(strInstrument);

    if (strType.empty())
    {
        QMessageBox::warning(this, tr("Indeterminate Instrument"),
                             tr("Unable to determine instrument type. Are you sure this is a financial instrument?"));
        return;
    }
    // -----------------------
    std::string strServerID = OTAPI_Wrap::Instrmnt_GetServerID(strInstrument);

    if (strServerID.empty())
    {
        QMessageBox::warning(this, tr("Indeterminate Server"),
                             tr("Unable to determine server ID for this instrument. Are you sure it's properly formed?"));
        return;
    }
    // -----------------------
    std::string strAssetID = OTAPI_Wrap::Instrmnt_GetAssetID(strInstrument);

    if (strAssetID.empty())
    {
        QMessageBox::warning(this, tr("Indeterminate Asset Type"),
                             tr("Unable to determine asset ID for this instrument. Are you sure it's properly formed?"));
        return;
    }
    // -----------------------
    std::string strServerContract = OTAPI_Wrap::LoadServerContract(strServerID);

    if (strServerContract.empty())
    {
        QMessageBox::warning(this, tr("No Server Contract Found"),
                             QString("%1 '%2'<br/>%3").arg(tr("Unable to load the server contract for server ID")).
                             arg(QString::fromStdString(strServerID)).arg(tr("Are you sure that server contract is even in your wallet?")));
        return;
    }
    // -----------------------
    std::string strAssetContract = OTAPI_Wrap::LoadAssetContract(strAssetID);

    if (strAssetContract.empty())
    {
        QMessageBox::warning(this, tr("No Asset Contract Found"),
                             QString("%1 '%2'<br/>%3").arg(tr("Unable to load the asset contract for asset ID")).
                             arg(QString::fromStdString(strAssetID)).arg(tr("Are you sure that asset type is even in your wallet?")));
        return;
    }
    // -----------------------
    if (0 != strType.compare("PURSE"))
    {
        QMessageBox::warning(this, tr("Not a Purse"),
                             QString("%1 '%2'<br/>%3").arg(tr("Expected a cash PURSE, but instead found a")).
                             arg(QString::fromStdString(strType)).arg(tr("Currently, only cash PURSE is supported. (Sorry.)")));
        return;
    }
    // -----------------------
    // By this point, we know it's a purse, and we know it has Server
    // and Asset IDs for contracts that are found in this wallet.
    //
    // Next, let's see if the purse is password-protected, and if not,
    // let's see if the recipient Nym is named on the instrument. (He may not be.)
    //
    const bool  bHasPassword = OTAPI_Wrap::Purse_HasPassword(strServerID, strInstrument);
    std::string strPurseOwner("");

    if (!bHasPassword)
    {
        // If the purse isn't password-protected, then it's DEFINITELY encrypted to
        // a specific Nym.
        //
        // The purse MAY include the NymID for this Nym, but it MAY also be blank, in
        // which case the user will have to select a Nym to TRY.
        //
        strPurseOwner = OTAPI_Wrap::Instrmnt_GetRecipientUserID(strInstrument); // TRY and get the Nym ID (it may have been left blank.)

        if (strPurseOwner.empty())
        {
            // Looks like it was left blank...
            // (Meaning we need to ask the user to tell us which Nym it was.)
            //
            // Select from Nyms in local wallet.
            //
            DlgChooser theChooser(this, tr("The cash purse is encrypted to a specific Nym, "
                                           "but that Nym isn't named on the purse. Therefore, you must select the intended Nym. "
                                           "But choose wisely -- for if you pick the wrong Nym, the import will fail!"));
            // -----------------------------------------------
            mapIDName & the_map = theChooser.m_map;

            bool bFoundDefault = false;
            // -----------------------------------------------
            const int32_t nym_count = OTAPI_Wrap::GetNymCount();
            // -----------------------------------------------
            for (int32_t ii = 0; ii < nym_count; ++ii)
            {
                //Get OT Nym ID
                QString OT_nym_id = QString::fromStdString(OTAPI_Wrap::GetNym_ID(ii));
                QString OT_nym_name("");
                // -----------------------------------------------
                if (!OT_nym_id.isEmpty())
                {
                    if (!default_nym_id.isEmpty() && (OT_nym_id == default_nym_id))
                        bFoundDefault = true;
                    // -----------------------------------------------
                    MTNameLookupQT theLookup;

                    OT_nym_name = QString::fromStdString(theLookup.GetNymName(OT_nym_id.toStdString()));
                    // -----------------------------------------------
                    the_map.insert(OT_nym_id, OT_nym_name);
                }
             }
            // -----------------------------------------------
            if (bFoundDefault && !default_nym_id.isEmpty())
                theChooser.SetPreSelected(default_nym_id);
            // -----------------------------------------------
            theChooser.setWindowTitle(tr("Choose Recipient Nym for Cash"));
            // -----------------------------------------------
            if (theChooser.exec() != QDialog::Accepted)
                return;
            else
            {
                if (theChooser.m_qstrCurrentID.isEmpty())
                    return; // Should never happen.
                // -----------------------------
                strPurseOwner = theChooser.m_qstrCurrentID.toStdString();
            }
        } // if strPurseOwner is empty (above the user selects him then.)
        // --------------------------------------
        if (!OTAPI_Wrap::IsNym_RegisteredAtServer(strPurseOwner, strServerID))
        {
            QMessageBox::warning(this, tr("Nym Isn't Registered at Server"),
                                 QString("%1 '%2'<br/>%3 '%4'<br/>%5").
                                 arg(tr("The Nym with ID")).
                                 arg(QString::fromStdString(strPurseOwner)).
                                 arg(tr("isn't registered at the Server with ID")).
                                 arg(QString::fromStdString(strServerID)).
                                 arg(tr("Try using that nym to create an asset account on that server, and then try importing this cash again.")));
            return;
        }
    } // If the purse is NOT password protected. (e.g. if the purse is encrypted to a specific Nym.)
    // -----------------------------------------------------------------
    // By this point, we know the purse is either password-protected, or that it's
    // encrypted to a Nym who IS registered at the appropriate server.
    //
    // ---------------------------------------------
    // DEPOSIT the cash to an ACCOUNT.
    //
    // We can't just import it to the wallet because the cash tokens aren't truly safe
    // until they are redeemed. (Until then, the sender still has a copy of them.)
    //
    // Use OT_ME::deposit_cash (vs deposit_local_purse) since the cash is external.
    // Otherwise if the deposit fails, OT will try to "re-import" them, even though
    // they were never in the purse in the first place.
    //
    // This would, of course, mix up coins that the sender has a copy of, with coins
    // that only I have a copy of -- which we don't want to do.
    //
    // Select from Accounts in local wallet.
    //
    DlgChooser theChooser(this);
    theChooser.SetIsAccounts();
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;

    bool bFoundDefault = false;
    // -----------------------------------------------
    const int32_t acct_count = OTAPI_Wrap::GetAccountCount();
    // -----------------------------------------------
    for (int32_t ii = 0; ii < acct_count; ++ii)
    {
        //Get OT Acct ID
        QString OT_acct_id = QString::fromStdString(OTAPI_Wrap::GetAccountWallet_ID(ii));
        QString OT_acct_name("");
        // -----------------------------------------------
        if (!OT_acct_id.isEmpty())
        {
            std::string str_acct_nym_id    = OTAPI_Wrap::GetAccountWallet_NymID      (OT_acct_id.toStdString());
            std::string str_acct_asset_id  = OTAPI_Wrap::GetAccountWallet_AssetTypeID(OT_acct_id.toStdString());
            std::string str_acct_server_id = OTAPI_Wrap::GetAccountWallet_ServerID   (OT_acct_id.toStdString());

            if (!strPurseOwner.empty() && (0 != strPurseOwner.compare(str_acct_nym_id)))
                continue;
            if (0 != strAssetID.compare(str_acct_asset_id))
                continue;
            if (0 != strServerID.compare(str_acct_server_id))
                continue;
            // -----------------------------------------------
            if (!default_account_id.isEmpty() && (OT_acct_id == default_account_id))
                bFoundDefault = true;
            // -----------------------------------------------
            MTNameLookupQT theLookup;

            OT_acct_name = QString::fromStdString(theLookup.GetAcctName(OT_acct_id.toStdString()));
            // -----------------------------------------------
            the_map.insert(OT_acct_id, OT_acct_name);
        }
     }
    // -----------------------------------------------
    if (the_map.size() < 1)
    {
        QMessageBox::warning(this, tr("No Matching Accounts"),
                             tr("The Nym doesn't have any accounts of the appropriate asset type, "
                                "on the appropriate server. Please create one and then try again."));
        return;
    }
    // -----------------------------------------------
    if (bFoundDefault && !default_account_id.isEmpty())
        theChooser.SetPreSelected(default_account_id);
    // -----------------------------------------------
    theChooser.setWindowTitle(tr("Deposit Cash to Which Account?"));
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
        if (!theChooser.m_qstrCurrentID.isEmpty())
        {
            if (strPurseOwner.empty())
                strPurseOwner = OTAPI_Wrap::GetAccountWallet_NymID(theChooser.m_qstrCurrentID.toStdString());
            // -------------------------------------------
            OT_ME madeEasy;
//          const bool bImported = OTAPI_Wrap::Wallet_ImportPurse(strServerID, strAssetID, strPurseOwner, strInstrument);

            if (1 == madeEasy.deposit_cash(strServerID, strPurseOwner,
                                           theChooser.m_qstrCurrentID.toStdString(), // AcctID.
                                           strInstrument))
            {
                QMessageBox::information(this, tr("Success"), tr("Success depositing cash purse."));
                emit balancesChanged();
            }
            else
                QMessageBox::warning(this, tr("Failed Import"), tr("Failed trying to deposit cash purse."));
        }
    }
}

// -----------------------------------------------



/**
 * Send Funds
 **/

void Moneychanger::mc_sendfunds_slot()
{
    mc_sendfunds_show_dialog();
}

void Moneychanger::mc_sendfunds_show_dialog()
{
    // --------------------------------------------------
    MTSendDlg * send_window = new MTSendDlg(NULL, *this);
    send_window->setAttribute(Qt::WA_DeleteOnClose);
    // --------------------------------------------------
    QString qstr_acct_id = this->get_default_account_id();

    if (!qstr_acct_id.isEmpty())
        send_window->setInitialMyAcct(qstr_acct_id);
    // ---------------------------------------
    connect(send_window, SIGNAL(balancesChanged()),
            this,        SLOT  (onBalancesChanged()));
    // ---------------------------------------
    send_window->dialog();
    send_window->show();
    // --------------------------------------------------
}

void Moneychanger::close_sendfunds_dialog()
{

}

// End Send Funds



/**
 * Overview Window
 **/

//Overview slots
void Moneychanger::mc_overview_slot()
{
    //The operator has requested to open the dialog to the "Overview";
    mc_overview_dialog();
}

void Moneychanger::mc_overview_dialog_refresh()
{
    if (mc_overview_already_init)
    {
        if (!homewindow->isHidden())
        {
            homewindow->SetNeedRefresh();
            mc_overview_dialog();
        }
    }
}

void Moneychanger::mc_overview_dialog()
{
    if (!mc_overview_already_init)
    {
        homewindow = new MTHome(this);
        mc_overview_already_init = true;

        connect(homewindow, SIGNAL(needToDownloadAccountData()),
                this,       SLOT(downloadAccountData()));

        connect(this,       SIGNAL(balancesChanged()),
                homewindow, SLOT(onBalancesChanged()));

        qDebug() << "Overview Opened";
    }
    // ---------------------------------
    homewindow->dialog();
}

void Moneychanger::close_overview_dialog()
{
    if (homewindow)
    {
        homewindow->setParent(NULL);
        homewindow->disconnect();
        homewindow->deleteLater();

        homewindow.clear();
    }
    mc_overview_already_init = false;
    qDebug() << "Overview Closed";
}

// End Overview







/**
 * Agreement Window
 **/

void Moneychanger::mc_agreement_slot()
{
    mc_agreement_dialog();
}

void Moneychanger::mc_agreement_dialog()
{
    // -------------------------------------
    if (!mc_agreement_already_init)
    {
        agreement_window = new MTDetailEdit(this, *this);

        mc_agreement_already_init = true;
        qDebug() << "Agreement Manager Opened";
    }
    // -------------------------------------
    mapIDName & the_map = agreement_window->m_map;
    // -------------------------------------
    the_map.clear();

    // TODO: populate the map here.

    // -------------------------------------
    agreement_window->setWindowTitle(tr("Agreements"));
    // -------------------------------------
    agreement_window->dialog(MTDetailEdit::DetailEditTypeAgreement);
}

void Moneychanger::close_agreement_dialog()
{
    if (agreement_window)
    {
        agreement_window->setParent(NULL);
        agreement_window->disconnect();
        agreement_window->deleteLater();

        agreement_window.clear();
    }

    mc_agreement_already_init = false;
    qDebug() << "Agreement Window Closed";
}

// End Agreement Window






/**
 * Market Window
 **/

void Moneychanger::mc_market_slot()
{
    mc_market_dialog();
}

void Moneychanger::mc_market_dialog()
{
    if (!mc_market_window_already_init)
    {
        market_window = new DlgMarkets(this, *this);
        mc_market_window_already_init = true;
    }
    // ------------------------------------
    market_window->dialog();
}

void Moneychanger::close_market_dialog()
{
    if (market_window)
    {
        market_window->setParent(NULL);
        market_window->disconnect();
        market_window->deleteLater();

        market_window.clear();
    }

    mc_market_window_already_init = false;
    qDebug() << "Market Window Closed";
}

// End Market Window








// CORPORATIONS

void Moneychanger::mc_corporation_slot()
{
    mc_corporation_dialog();
}

void Moneychanger::mc_corporation_dialog()
{
    // -------------------------------------
    if (!mc_corporation_already_init)
    {
        corporation_window = new MTDetailEdit(this, *this);

        mc_corporation_already_init = true;
        qDebug() << "Corporation Manager Opened";
    }
    // -------------------------------------
    mapIDName & the_map = corporation_window->m_map;
    // -------------------------------------
    the_map.clear();

    // TODO: populate the map here.

    // -------------------------------------
    corporation_window->setWindowTitle(tr("Corporations"));
    // -------------------------------------
    corporation_window->dialog(MTDetailEdit::DetailEditTypeCorporation);
}

void Moneychanger::close_corporation_dialog()
{
    if (corporation_window)
    {
        corporation_window->setParent(NULL);
        corporation_window->disconnect();
        corporation_window->deleteLater();

        corporation_window.clear();
    }

    mc_corporation_already_init = false;
    qDebug() << "Corporation Window Closed";
}







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
    if (createinsurancecompany_window)
    {
        createinsurancecompany_window->setParent(NULL);
        createinsurancecompany_window->disconnect();
        createinsurancecompany_window->deleteLater();

        createinsurancecompany_window.clear();
    }

    mc_createinsurancecompany_already_init = false;
    qDebug() << "Create Insurance Company Window Closed";
}

/**
 * (Advantced ->) Settings Window
 **/

void Moneychanger::mc_settings_slot()
{
    // This is a glaring memory leak, but it's only a temporary placeholder before I redo how windows are handled.
    if(!mc_settings_already_init)
    {
        settingswindow = new Settings();
//        market_window->setAttribute(Qt::WA_DeleteOnClose);
        mc_settings_already_init = true;
    }
    // ------------------------------------
    settingswindow->show();
}

void Moneychanger::close_settings_dialog()
{
    if (settingswindow)
    {
        settingswindow->setParent(NULL);
        settingswindow->disconnect();
        settingswindow->deleteLater();

        settingswindow.clear();
    }

    mc_settings_already_init = false;
    qDebug() << "Settings Window Closed";
}

// End Settings Window
