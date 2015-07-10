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

#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/moneychanger.hpp>
#include <core/mtcomms.h>
#include <core/handlers/DBHandler.hpp>
#include <rpc/rpcserver.h>

#include <gui/widgets/compose.hpp>
#include <gui/widgets/home.hpp>
#include <gui/widgets/overridecursor.hpp>
#include <gui/widgets/editdetails.hpp>
#include <gui/widgets/requestdlg.hpp>
#include <gui/widgets/dlgchooser.hpp>
#include <gui/widgets/senddlg.hpp>
#include <gui/widgets/createinsurancecompany.hpp>
#include <gui/widgets/wizardrunsmartcontract.hpp>
#include <gui/widgets/wizardpartyacct.hpp>
#include <gui/widgets/settings.hpp>
#include <gui/widgets/btcguitest.hpp>
#include <gui/widgets/btcpoolmanager.hpp>
#include <gui/widgets/btctransactionmanager.hpp>
#include <gui/widgets/btcconnectdlg.hpp>
#include <gui/widgets/btcsenddlg.hpp>
#include <gui/widgets/btcreceivedlg.hpp>
#include <gui/ui/dlgimport.hpp>
#include <gui/ui/dlglog.hpp>
#include <gui/ui/dlgmenu.hpp>
#include <gui/ui/dlgmarkets.hpp>
#include <gui/ui/dlgencrypt.hpp>
#include <gui/ui/dlgdecrypt.hpp>
#include <gui/ui/dlgpassphrasemanager.hpp>



#include <opentxs/client/OTAPI.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>
#include <opentxs/client/OT_ME.hpp>
#include <opentxs/core/util/OTPaths.hpp>

#include <QMenu>
#include <QApplication>
#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QTimer>

#include <sstream>

/**
 * Constructor & Destructor
 **/

//static
Moneychanger * Moneychanger::It(QWidget *parent/*=0*/, bool bShuttingDown/*=false*/)
{
    // NOTE: parent is only used the first time this function is called.
    // (And the last time.)
    //
    static QScopedPointer<Moneychanger> pMoneychanger(new Moneychanger(parent));

    if (bShuttingDown)
    {
        // bShuttingDown is only passed in the very last time this is called,
        // (at application shutdown.)
        //
        // (If we don't delete it at this time, then it won't get deleted until
        // the static objects are deleted, which causes a crash since apparently
        // that's too late to be deleting the widgets.)
        //
        pMoneychanger->disconnect();
        delete pMoneychanger.take();
        return NULL;
    }
    // -------------------------------------
    return pMoneychanger.data();
}

Moneychanger::Moneychanger(QWidget *parent)
: QWidget(parent),
  nmc(new NMC_Interface ()),
  nmc_names(NULL),
  mc_overall_init(false),
  nmc_update_timer(NULL),
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
        
    /* Set up Namecoin name manager.  */
    nmc_names = new NMC_NameManager (*nmc);

    /* Set up the Namecoin update timer.  */
    nmc_update_timer = new QTimer (this);
    connect (nmc_update_timer, SIGNAL(timeout()),
             this, SLOT(nmc_timer_event()));
    nmc_update_timer->start (1000 * 60 * 10);
    nmc_timer_event ();
    
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
//        if (default_nym_id.isEmpty() && (opentxs::OTAPI_Wrap::It()->GetNymCount() > 0))
//        {
//            default_nym_id = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetNym_ID(0));
//        }
//        // -------------------------------------------------
//        //Ask OT what the display name of this nym is and store it for quick retrieval later on(mostly for "Default Nym" displaying purposes)
//        if (!default_nym_id.isEmpty())
//        {
//            default_nym_name =  QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetNym_Name(default_nym_id.toStdString()));
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
            default_notary_id = DBHandler::getInstance()->queryString("SELECT `server` FROM `default_server` WHERE `default_id`='1' LIMIT 0,1", 0, 0);
        }
        // -------------------------------------------------
//        if (default_notary_id.isEmpty() && (opentxs::OTAPI_Wrap::It()->GetServerCount() > 0))
//        {
//            default_notary_id = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetServer_ID(0));
//        }
//        // -------------------------------------------------
//        //Ask OT what the display name of this server is and store it for a quick retrieval later on(mostly for "Default Server" displaying purposes)
//        if (!default_notary_id.isEmpty())
//        {
//            default_server_name = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetServer_Name(default_notary_id.toStdString()));
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
//        if (default_asset_id.isEmpty() && (opentxs::OTAPI_Wrap::It()->GetAssetTypeCount() > 0))
//        {
//            default_asset_id = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAssetType_ID(0));
//        }
//        // -------------------------------------------------
//        //Ask OT what the display name of this asset type is and store it for a quick retrieval later on(mostly for "Default Asset" displaying purposes)
//        if (!default_asset_id.isEmpty())
//        {
//            default_asset_name = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAssetType_Name(default_asset_id.toStdString()));
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
//        if (default_account_id.isEmpty() && (opentxs::OTAPI_Wrap::It()->GetAccountCount() > 0))
//        {
//            default_account_id = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAccountWallet_ID(0));
//        }
//        // -------------------------------------------------
//        //Ask OT what the display name of this account is and store it for a quick retrieval later on(mostly for "Default Account" displaying purposes)
//        if (!default_account_id.isEmpty())
//        {
//            default_account_name = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAccountWallet_Name(default_account_id.toStdString()));
//        }
//        else
//            qDebug() << "Error loading DEFAULT ACCOUNT from SQL";
    }

    // Check for RPCServer Settings (Config read will populate the database)

    RPCServer::getInstance()->init();

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
    
    mc_systrayIcon_nym = QIcon(":/icons/icons/identity_BW2.png");
    mc_systrayIcon_server = QIcon(":/icons/server");
    
    mc_systrayIcon_goldaccount = QIcon(":/icons/icons/safe_box.png");
    mc_systrayIcon_purse = QIcon(":/icons/icons/assets.png");
    
//  mc_systrayIcon_sendfunds      = QIcon(":/icons/icons/fistful_of_cash_72.png");
    mc_systrayIcon_sendfunds      = QIcon(":/icons/icons/money_fist4_small.png");
//  mc_systrayIcon_sendfunds      = QIcon(":/icons/sendfunds");
    mc_systrayIcon_requestfunds   = QIcon(":/icons/requestpayment");
//  mc_systrayIcon_contacts       = QIcon(":/icons/addressbook");
    mc_systrayIcon_contacts       = QIcon(":/icons/icons/rolodex_card2");
    mc_systrayIcon_composemessage = QIcon(":/icons/icons/pencil.png");
//  mc_systrayIcon_composemessage = QIcon(":/icons/icons/compose.png");

    mc_systrayIcon_markets = QIcon(":/icons/markets");

    // ---------------------------------------------------------------
    mc_systrayIcon_bitcoin  = QIcon(":/icons/icons/bitcoin.png");
    mc_systrayIcon_crypto   = QIcon(":/icons/icons/lock.png");

    //Submenu
    mc_systrayIcon_crypto_encrypt  = QIcon(":/icons/icons/lock.png");
    mc_systrayIcon_crypto_decrypt  = QIcon(":/icons/icons/padlock_open.png");
    mc_systrayIcon_crypto_sign     = QIcon(":/icons/icons/signature-small.png");
    mc_systrayIcon_crypto_verify   = QIcon(":/icons/icons/check-mark-small.png");
    // ---------------------------------------------------------------
    mc_systrayIcon_advanced = QIcon(":/icons/advanced");

    //Submenu
    mc_systrayIcon_advanced_import = QIcon(":/icons/icons/request.png");
    mc_systrayIcon_advanced_agreements = QIcon(":/icons/agreements");
    mc_systrayIcon_advanced_corporations = QIcon(":/icons/icons/buildings.png");
    mc_systrayIcon_advanced_transport = QIcon(":/icons/icons/p2p.png");
    mc_systrayIcon_advanced_log = QIcon(":/icons/icons/p2p.png");
    mc_systrayIcon_advanced_settings = QIcon(":/icons/settings");
    
    // ----------------------------------------------------------------------------

    mc_overall_init = true;
}


Moneychanger::~Moneychanger()
{
    delete nmc_update_timer;
    delete nmc_names;
    delete nmc;
}

// ---------------------------------------------------------------
// Check and see if the Nym has exhausted his usage credits.
//
// Return value: -2 for error, -1 for "unlimited" (or "server isn't enforcing"),
//                0 for "exhausted", and non-zero for the exact number of credits available.
int64_t Moneychanger::HasUsageCredits(const std::string & notary_id,
                                      const std::string & NYM_ID)
{
    // Usually when a message fails, Moneychanger calls HasUsageCredits because it assumes
    // the failure probably happened due to a lack of usage credits.
    // ...But what if there was a network failure? What if messages can't even get out?
    //
    if (opentxs::OTAPI_Wrap::networkFailure())
    {
        QString qstrErrorMsg;
        qstrErrorMsg = tr("HasUsageCredits: Failed trying to contact the notary. Perhaps it is down, or there might be a network problem.");
        emit appendToLog(qstrErrorMsg);
        return -2;
    }
    // --------------------------------------------------------
    opentxs::OT_ME madeEasy;
    // --------------------------------------------------------
    const std::string strAdjustment("0");
    // --------------------------------------------------------
    std::string strMessage;
    {
        MTSpinner theSpinner;

        strMessage = madeEasy.adjust_usage_credits(notary_id, NYM_ID, NYM_ID, strAdjustment);
    }
    if (strMessage.empty())
    {
//        QString qstrErrorMsg;
//        qstrErrorMsg = tr("Moneychanger::HasUsageCredits: Error 'strMessage' is Empty!");
//        emit appendToLog(qstrErrorMsg);
        return -2;
    }

    // --------------------------------------------------------
    const int64_t lReturnValue = opentxs::OTAPI_Wrap::It()->Message_GetUsageCredits(strMessage);
    // --------------------------------------------------------
    QString qstrErrorMsg;

    switch (lReturnValue)
    {
    case (-2): // error
        qstrErrorMsg    = tr("Error checking usage credits. Perhaps the server is down or inaccessible?");
        break;
        // --------------------------------
    case (-1): // unlimited, or server isn't enforcing
        qstrErrorMsg    = tr("Nym has unlimited usage credits (or the server isn't enforcing credits.')");
        break;
        // --------------------------------
    case (0): // Exhausted
        qstrErrorMsg    = tr("Sorry, but the Nym attempting this action is all out of usage credits on the server. "
                             "(You should contact the server operator and purchase more usage credits.)");
        break;
        // --------------------------------
    default: // Nym has X usage credits remaining.
        qstrErrorMsg    = tr("The Nym still has usage credits remaining. Should be fine.");
        break;
    }
    // --------------------------------
    switch (lReturnValue)
    {
    case (-2): // Error
    case (0):  // Exhausted
        emit appendToLog(qstrErrorMsg);
        // --------------------------------
    default: // Nym has X usage credits remaining, or server isn't enforcing credits.
        break;
    }
    // --------------------------------
    return lReturnValue;
}


int64_t Moneychanger::HasUsageCredits(QString   notary_id,
                                      QString   NYM_ID)
{
    const std::string str_server(notary_id.toStdString());
    const std::string str_nym   (NYM_ID   .toStdString());

    return HasUsageCredits(str_server, str_nym);
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
    // ----------------------------------------------------------------------------
    QString qstrMenuFileExists = QString(opentxs::OTPaths::AppDataFolder().Get()) + QString("/knotworkpigeons");

    if (QFile::exists(qstrMenuFileExists))
        mc_main_menu_dialog();
    // ----------------------------------------------------------------------------
}


// Shutdown
void Moneychanger::mc_shutdown_slot()
{
    //Disconnect all signals from callin class (probubly main) to this class
    //Disconnect
    QObject::disconnect(this);

    qApp->quit();
}

// End Systray

// ---------------------------------------------------------------


void Moneychanger::SetupMainMenu()
{
    if (!mc_overall_init)
        return;

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

    // --------------------------------------------------------------
    if (mc_systrayMenu)
    {
        mc_systrayMenu->setParent(NULL);
        mc_systrayMenu->disconnect();
        mc_systrayMenu->deleteLater();

        mc_systrayMenu = NULL;
    }
    // --------------------------------------------------------------
    //MC System tray menu
    //
    mc_systrayMenu = new QMenu(this);

    //Init Skeleton of system tray menu
    //App name
    mc_systrayMenu_headertext = new QAction(tr("Moneychanger"), mc_systrayMenu);
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
    mc_systrayMenu_overview = new QAction(mc_systrayIcon_overview, tr("Transaction History"), mc_systrayMenu);
    mc_systrayMenu->addAction(mc_systrayMenu_overview);
    connect(mc_systrayMenu_overview, SIGNAL(triggered()), this, SLOT(mc_overview_slot()));
    // --------------------------------------------------------------
    //Crypto
    mc_systrayMenu_crypto = new QMenu(tr("Crypto"), mc_systrayMenu);
    mc_systrayMenu_crypto->setIcon(mc_systrayIcon_crypto);
    mc_systrayMenu->addMenu(mc_systrayMenu_crypto);
    // --------------------------------------------------------------
    mc_systrayMenu_crypto_sign = new QAction(mc_systrayIcon_crypto_sign, tr("Sign"), mc_systrayMenu_crypto);
    mc_systrayMenu_crypto->addAction(mc_systrayMenu_crypto_sign);
    connect(mc_systrayMenu_crypto_sign, SIGNAL(triggered()), this, SLOT(mc_crypto_sign_slot()));
    // --------------------------------------------------------------
    mc_systrayMenu_crypto_encrypt = new QAction(mc_systrayIcon_crypto_encrypt, tr("Encrypt"), mc_systrayMenu_crypto);
    mc_systrayMenu_crypto->addAction(mc_systrayMenu_crypto_encrypt);
    connect(mc_systrayMenu_crypto_encrypt, SIGNAL(triggered()), this, SLOT(mc_crypto_encrypt_slot()));
    // --------------------------------------------------------------
    //Separator
    mc_systrayMenu_crypto->addSeparator();
    // --------------------------------------------------------------
    mc_systrayMenu_crypto_decrypt = new QAction(mc_systrayIcon_crypto_decrypt, tr("Decrypt / Verify"), mc_systrayMenu_crypto);
    mc_systrayMenu_crypto->addAction(mc_systrayMenu_crypto_decrypt);
    connect(mc_systrayMenu_crypto_decrypt, SIGNAL(triggered()), this, SLOT(mc_crypto_decrypt_slot()));
    // --------------------------------------------------------------
    //Separator
    mc_systrayMenu->addSeparator();
    // --------------------------------------------------------------
    //Passphrase Manager
    mc_systrayMenu_passphrase_manager = new QAction(mc_systrayIcon_crypto, tr("Passphrase Manager"), mc_systrayMenu);
    mc_systrayMenu->addAction(mc_systrayMenu_passphrase_manager);
    connect(mc_systrayMenu_passphrase_manager, SIGNAL(triggered()), this, SLOT(mc_passphrase_manager_slot()));
    // --------------------------------------------------------------
    //Separator
    mc_systrayMenu->addSeparator();
    // --------------------------------------------------------------
    //Send funds
    mc_systrayMenu_sendfunds = new QAction(mc_systrayIcon_sendfunds, tr("Pay Funds..."), mc_systrayMenu);
    mc_systrayMenu->addAction(mc_systrayMenu_sendfunds);
    connect(mc_systrayMenu_sendfunds, SIGNAL(triggered()), this, SLOT(mc_sendfunds_slot()));
    // --------------------------------------------------------------
    //Request payment
    mc_systrayMenu_requestfunds = new QAction(mc_systrayIcon_requestfunds, tr("Request Payment..."), mc_systrayMenu);
    mc_systrayMenu->addAction(mc_systrayMenu_requestfunds);
    connect(mc_systrayMenu_requestfunds, SIGNAL(triggered()), this, SLOT(mc_requestfunds_slot()));
    // --------------------------------------------------------------
    //Separator
    mc_systrayMenu->addSeparator();
    // --------------------------------------------------------------
    //Compose Message
    mc_systrayMenu_composemessage = new QAction(mc_systrayIcon_composemessage, tr("Compose Message"), mc_systrayMenu);
    mc_systrayMenu->addAction(mc_systrayMenu_composemessage);
    connect(mc_systrayMenu_composemessage, SIGNAL(triggered()), this, SLOT(mc_composemessage_slot()));
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
    //Nym section
    SetupNymMenu();
    // --------------------------------------------------------------
    //Contacts
    mc_systrayMenu_contacts = new QAction(mc_systrayIcon_contacts, tr("Contacts"), mc_systrayMenu);
    mc_systrayMenu->addAction(mc_systrayMenu_contacts);
    connect(mc_systrayMenu_contacts, SIGNAL(triggered()), this, SLOT(mc_addressbook_slot()));
    // --------------------------------------------------------------



    // --------------------------------------------------------------
    //Separator
    mc_systrayMenu->addSeparator();
    // --------------------------------------------------------------
    //Company
//    mc_systrayMenu_company = new QMenu("Company", 0);
//    mc_systrayMenu->addMenu(mc_systrayMenu_company);
    // --------------------------------------------------------------

    // :/icons/icons/lock.png
    //
//    QPointer<QAction> mc_systrayMenu_crypto_encrypt;
//    QPointer<QAction> mc_systrayMenu_crypto_decrypt;
//    QPointer<QAction> mc_systrayMenu_crypto_sign;
//    QPointer<QAction> mc_systrayMenu_crypto_verify;

    // --------------------------------------------------------------
    //Advanced
    mc_systrayMenu_advanced = new QMenu(tr("Advanced"), mc_systrayMenu);
    mc_systrayMenu_advanced->setIcon(mc_systrayIcon_advanced);
    mc_systrayMenu->addMenu(mc_systrayMenu_advanced);
    //Advanced submenu
    // --------------------------------------------------------------
    mc_systrayMenu_advanced_import = new QAction(mc_systrayIcon_advanced_import, tr("Import Cash..."), mc_systrayMenu_advanced);
    mc_systrayMenu_advanced->addAction(mc_systrayMenu_advanced_import);
    connect(mc_systrayMenu_advanced_import, SIGNAL(triggered()), this, SLOT(mc_import_slot()));
    // --------------------------------------------------------------
    //Separator
    mc_systrayMenu_advanced->addSeparator();
    // ------------------------------------------------
    mc_systrayMenu_advanced_markets = new QAction(mc_systrayIcon_markets, tr("Markets"), mc_systrayMenu_advanced);
    mc_systrayMenu_advanced->addAction(mc_systrayMenu_advanced_markets);
    connect(mc_systrayMenu_advanced_markets, SIGNAL(triggered()), this, SLOT(mc_market_slot()));
    // --------------------------------------------------------------
    mc_systrayMenu_advanced_agreements = new QAction(mc_systrayIcon_advanced_agreements, tr("Smart Contracts"), mc_systrayMenu_advanced);
    mc_systrayMenu_advanced->addAction(mc_systrayMenu_advanced_agreements);
    connect(mc_systrayMenu_advanced_agreements, SIGNAL(triggered()), this, SLOT(mc_agreement_slot()));
    // --------------------------------------------------------------
    // Corporations
    mc_systrayMenu_advanced_corporations = new QAction(mc_systrayIcon_advanced_corporations, tr("Corporations"), mc_systrayMenu_advanced);
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
    mc_systrayMenu_advanced_bazaar = new QMenu(tr("Bazaar"), mc_systrayMenu_advanced);
    mc_systrayMenu_advanced->addMenu(mc_systrayMenu_advanced_bazaar);

    // Bazaar actions
    mc_systrayMenu_bazaar_search = new QAction(mc_systrayIcon_advanced_agreements, tr("Search Listings"), mc_systrayMenu_advanced_bazaar);
    mc_systrayMenu_advanced_bazaar->addAction(mc_systrayMenu_bazaar_search);
//  connect(mc_systrayMenu_bazaar_search, SIGNAL(triggered()), this, SLOT(mc_bazaar_search_slot()));

    mc_systrayMenu_bazaar_post = new QAction(mc_systrayIcon_advanced_agreements, tr("Post an Ad"), mc_systrayMenu_advanced_bazaar);
    mc_systrayMenu_advanced_bazaar->addAction(mc_systrayMenu_bazaar_post);
//  connect(mc_systrayMenu_bazaar_post, SIGNAL(triggered()), this, SLOT(mc_bazaar_search_slot()));

    mc_systrayMenu_bazaar_orders = new QAction(mc_systrayIcon_advanced_agreements, tr("Orders"), mc_systrayMenu_advanced_bazaar);
    mc_systrayMenu_advanced_bazaar->addAction(mc_systrayMenu_bazaar_orders);
//  connect(mc_systrayMenu_bazaar_orders, SIGNAL(triggered()), this, SLOT(mc_bazaar_search_slot()));

    // -------------------------------------------------
    mc_systrayMenu_advanced->addSeparator();
    // -------------------------------------------------
    // Settings

    mc_systrayMenu_advanced_settings = new QAction(mc_systrayIcon_advanced_settings, tr("Settings"), mc_systrayMenu_advanced);
    mc_systrayMenu_advanced_settings->setMenuRole(QAction::NoRole);
    mc_systrayMenu_advanced->addAction(mc_systrayMenu_advanced_settings);
    connect(mc_systrayMenu_advanced_settings, SIGNAL(triggered()), this, SLOT(mc_settings_slot()));

    // --------------------------------------------------------------
    // Transport
    mc_systrayMenu_advanced_transport = new QAction(mc_systrayIcon_advanced_transport, tr("P2P Transport"), mc_systrayMenu_advanced);
    mc_systrayMenu_advanced->addAction(mc_systrayMenu_advanced_transport);
    connect(mc_systrayMenu_advanced_transport, SIGNAL(triggered()), this, SLOT(mc_transport_slot()));

    // --------------------------------------------------------------
    // Error Log
    mc_systrayMenu_advanced_log = new QAction(mc_systrayIcon_advanced_log, tr("Error Log"), mc_systrayMenu_advanced);
    mc_systrayMenu_advanced->addAction(mc_systrayMenu_advanced_log);
    connect(mc_systrayMenu_advanced_log, SIGNAL(triggered()), this, SLOT(mc_log_slot()));

    connect(this, SIGNAL(appendToLog(QString)),
            this, SLOT(mc_showlog_slot(QString)));
    // --------------------------------------------------------------

    // Bitcoin
    mc_systrayMenu_bitcoin = new QMenu(tr("Bitcoin"), mc_systrayMenu);
    mc_systrayMenu_bitcoin->setIcon(mc_systrayIcon_bitcoin);
    mc_systrayMenu_advanced->addMenu(mc_systrayMenu_bitcoin);
    mc_systrayMenu_bitcoin_test = new QAction(tr("Test"), mc_systrayMenu_bitcoin);
    mc_systrayMenu_bitcoin_connect = new QAction(tr("Connect to wallet"), mc_systrayMenu_bitcoin);
    mc_systrayMenu_bitcoin_pools = new QAction(tr("Pools"), mc_systrayMenu_bitcoin);
    mc_systrayMenu_bitcoin_transactions = new QAction(tr("Transactions"), mc_systrayMenu_bitcoin);
    mc_systrayMenu_bitcoin_send = new QAction(tr("Send"), mc_systrayMenu_bitcoin);
    mc_systrayMenu_bitcoin_receive = new QAction(tr("Receive"), mc_systrayMenu_bitcoin);
    mc_systrayMenu_bitcoin->addAction(mc_systrayMenu_bitcoin_connect);
    mc_systrayMenu_bitcoin->addAction(mc_systrayMenu_bitcoin_send);
    mc_systrayMenu_bitcoin->addAction(mc_systrayMenu_bitcoin_receive);
    mc_systrayMenu_bitcoin->addAction(mc_systrayMenu_bitcoin_transactions);
    mc_systrayMenu_bitcoin->addSeparator();
    mc_systrayMenu_bitcoin->addAction(mc_systrayMenu_bitcoin_test);
    mc_systrayMenu_bitcoin->addAction(mc_systrayMenu_bitcoin_pools);
    connect(mc_systrayMenu_bitcoin_test, SIGNAL(triggered()), this, SLOT(mc_bitcoin_slot()));
    connect(mc_systrayMenu_bitcoin_connect, SIGNAL(triggered()), this, SLOT(mc_bitcoin_connect_slot()));
    connect(mc_systrayMenu_bitcoin_pools, SIGNAL(triggered()), this, SLOT(mc_bitcoin_pools_slot()));
    connect(mc_systrayMenu_bitcoin_transactions, SIGNAL(triggered()), this, SLOT(mc_bitcoin_transactions_slot()));
    connect(mc_systrayMenu_bitcoin_send, SIGNAL(triggered()), this, SLOT(mc_bitcoin_send_slot()));
    connect(mc_systrayMenu_bitcoin_receive, SIGNAL(triggered()), this, SLOT(mc_bitcoin_receive_slot()));

    // --------------------------------------------------------------
    //Separator
    mc_systrayMenu_advanced->addSeparator();

    // TODO: If the default isn't set, then choose the first one and select it.

    // TODO: If there isn't even ONE to select, then this menu item should say "Create Nym..." with no sub-menu.

    // TODO: When booting up, if there is already a default server and asset id, but no nyms exist, create a default nym.

    // TODO: When booting up, if there is already a default nym, but no accounts exist, create a default account.

    // --------------------------------------------------------------
    //Server section
    SetupServerMenu();

    //Separator
    mc_systrayMenu->addSeparator();
    // --------------------------------------------------------------
    //Shutdown Moneychanger
    mc_systrayMenu_shutdown = new QAction(mc_systrayIcon_shutdown, tr("Quit"), mc_systrayMenu);
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


void Moneychanger::SetupAssetMenu()
{
    mc_systrayMenu_asset = new QMenu(tr("Set Default Asset Type..."), mc_systrayMenu);
    mc_systrayMenu_asset->setIcon(mc_systrayIcon_purse);
    mc_systrayMenu->addMenu(mc_systrayMenu_asset);
    // --------------------------------------------------
    //Add a "Manage asset types" action button (and connection)
    QAction * manage_assets = new QAction(tr("Manage Asset Contracts..."), mc_systrayMenu_asset);
    manage_assets->setData(QVariant(QString("openmanager")));
    mc_systrayMenu_asset->addAction(manage_assets);
    connect(mc_systrayMenu_asset, SIGNAL(triggered(QAction*)), this, SLOT(mc_assetselection_triggered(QAction*)));
    // -------------------------------------------------
    mc_systrayMenu_asset->addSeparator();
    // -------------------------------------------------
    if (default_asset_id.isEmpty() && (opentxs::OTAPI_Wrap::It()->GetAssetTypeCount() > 0))
    {
        default_asset_id = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAssetType_ID(0));
    }
    // -------------------------------------------------
    if (!default_asset_id.isEmpty())
    {
        default_asset_name = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAssetType_Name(default_asset_id.toStdString()));
    }
    // -------------------------------------------------
    //Load "default" asset type
    setDefaultAsset(default_asset_id, default_asset_name);

    //Init asset submenu
    asset_list_id   = new QList<QVariant>;
    asset_list_name = new QList<QVariant>;
    // ------------------------------------------
    int32_t asset_count = opentxs::OTAPI_Wrap::It()->GetAssetTypeCount();

    for (int aa = 0; aa < asset_count; aa++)
    {
        QString OT_asset_id   = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAssetType_ID(aa));
        QString OT_asset_name = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAssetType_Name(OT_asset_id.toStdString()));

        asset_list_id  ->append(QVariant(OT_asset_id));
        asset_list_name->append(QVariant(OT_asset_name));

        QAction * next_asset_action = new QAction(mc_systrayIcon_purse, OT_asset_name, mc_systrayMenu_asset);
        next_asset_action->setData(QVariant(OT_asset_id));
        next_asset_action->setCheckable(true);

        if (0 == OT_asset_id.compare(default_asset_id)) {
            next_asset_action->setChecked(true);
        }
        else {
            next_asset_action->setChecked(false);
        }

        mc_systrayMenu_asset->addAction(next_asset_action);
    }
}

void Moneychanger::SetupServerMenu()
{
    mc_systrayMenu_server = new QMenu(tr("Set Default Server..."), mc_systrayMenu_advanced);
    mc_systrayMenu_server->setIcon(mc_systrayIcon_server);
    mc_systrayMenu_advanced->addMenu(mc_systrayMenu_server);
    // --------------------------------------------------
    //Add a "Manage Servers" action button (and connection)
    QAction * manage_servers = new QAction(tr("Manage Servers..."), mc_systrayMenu_server);
    manage_servers->setData(QVariant(QString("openmanager")));
    mc_systrayMenu_server->addAction(manage_servers);
    connect(mc_systrayMenu_server, SIGNAL(triggered(QAction*)), this, SLOT(mc_serverselection_triggered(QAction*)));
    // -------------------------------------------------
    mc_systrayMenu_server->addSeparator();
    // -------------------------------------------------
    if (default_notary_id.isEmpty() && (opentxs::OTAPI_Wrap::It()->GetServerCount() > 0))
    {
        default_notary_id = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetServer_ID(0));
    }
    // -------------------------------------------------
    //Ask OT what the display name of this server is and store it for a quick retrieval later on(mostly for "Default Server" displaying purposes)
    if (!default_notary_id.isEmpty())
    {
        default_server_name = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetServer_Name(default_notary_id.toStdString()));
    }
    // -------------------------------------------------
    //Load "default" server
    setDefaultServer(default_notary_id, default_server_name);

    //Init server submenu
    server_list_id   = new QList<QVariant>;
    server_list_name = new QList<QVariant>;
    // ------------------------------------------
    int32_t server_count = opentxs::OTAPI_Wrap::It()->GetServerCount();

    for (int32_t aa = 0; aa < server_count; aa++)
    {
        QString OT_notary_id   = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetServer_ID(aa));
        QString OT_server_name = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetServer_Name(OT_notary_id.toStdString()));

        server_list_id  ->append(QVariant(OT_notary_id));
        server_list_name->append(QVariant(OT_server_name));

        //Append to submenu of server
        QAction * next_server_action = new QAction(mc_systrayIcon_server, OT_server_name, mc_systrayMenu_server);
        next_server_action->setData(QVariant(OT_notary_id));
        next_server_action->setCheckable(true);

        if (0 == OT_notary_id.compare(default_notary_id)) {
            next_server_action->setChecked(true);
        }
        else {
            next_server_action->setChecked(false);
        }

        mc_systrayMenu_server->addAction(next_server_action);
    }
}

void Moneychanger::SetupNymMenu()
{
    mc_systrayMenu_nym = new QMenu("Set Default Identity...", mc_systrayMenu);
    mc_systrayMenu_nym->setIcon(mc_systrayIcon_nym);
    mc_systrayMenu->addMenu(mc_systrayMenu_nym);

    //Add a "Manage pseudonym" action button (and connection)
    QAction * manage_nyms = new QAction(tr("Manage Identities..."), mc_systrayMenu_nym);
    manage_nyms->setData(QVariant(QString("openmanager")));
    mc_systrayMenu_nym->addAction(manage_nyms);
    connect(mc_systrayMenu_nym, SIGNAL(triggered(QAction*)), this, SLOT(mc_nymselection_triggered(QAction*)));
    // -------------------------------------------------
    mc_systrayMenu_nym->addSeparator();
    // -------------------------------------------------
    if (default_nym_id.isEmpty() && (opentxs::OTAPI_Wrap::It()->GetNymCount() > 0))
    {
        default_nym_id = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetNym_ID(0));
    }
    // -------------------------------------------------
    //Ask OT what the display name of this nym is and store it for quick retrieval later on(mostly for "Default Nym" displaying purposes)
    if (!default_nym_id.isEmpty())
    {
        default_nym_name =  QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetNym_Name(default_nym_id.toStdString()));
    }
    // -------------------------------------------------
    //Init nym submenu
    nym_list_id = new QList<QVariant>;
    nym_list_name = new QList<QVariant>;
    // --------------------------------------------------------
    //Count nyms
    int32_t nym_count = opentxs::OTAPI_Wrap::It()->GetNymCount();

    //Add/append to the id + name lists
    for (int32_t a = 0; a < nym_count; a++)
    {
        QString OT_nym_id   = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetNym_ID(a));
        QString OT_nym_name = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetNym_Name(OT_nym_id.toStdString()));

        nym_list_id  ->append(QVariant(OT_nym_id));
        nym_list_name->append(QVariant(OT_nym_name));

        //Append to submenu of nym
        QAction * next_nym_action = new QAction(mc_systrayIcon_nym, OT_nym_name, mc_systrayMenu_nym);
        next_nym_action->setData(QVariant(OT_nym_id));
        next_nym_action->setCheckable(true);

        if (0 == OT_nym_id.compare(default_nym_id)) {
            next_nym_action->setChecked(true);
        }
        else {
            next_nym_action->setChecked(false);
        }

        mc_systrayMenu_nym->addAction(next_nym_action);

    } // for
    // -------------------------------------------------
    setDefaultNym(default_nym_id, default_nym_name);
}

void Moneychanger::SetupAccountMenu()
{
    mc_systrayMenu_account = new QMenu(tr("Set Default Account..."), mc_systrayMenu);
    mc_systrayMenu_account->setIcon(mc_systrayIcon_goldaccount);
    mc_systrayMenu->addMenu(mc_systrayMenu_account);

    //Add a "Manage accounts" action button (and connection)
    QAction * manage_accounts = new QAction(tr("Manage Accounts..."), mc_systrayMenu_account);
    manage_accounts->setData(QVariant(QString("openmanager")));
    mc_systrayMenu_account->addAction(manage_accounts);
    connect(mc_systrayMenu_account, SIGNAL(triggered(QAction*)), this, SLOT(mc_accountselection_triggered(QAction*)));
    // -------------------------------------------------
    mc_systrayMenu_account->addSeparator();
    // -------------------------------------------------
    if (default_account_id.isEmpty() && (opentxs::OTAPI_Wrap::It()->GetAccountCount() > 0))
    {
        default_account_id = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAccountWallet_ID(0));
    }
    // -------------------------------------------------
    //Ask OT what the display name of this account is and store it for a quick retrieval later on(mostly for "Default Account" displaying purposes)
    if (!default_account_id.isEmpty())
    {
        default_account_name = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAccountWallet_Name(default_account_id.toStdString()));
    }
    // -------------------------------------------------
    //Init account submenu
    account_list_id   = new QList<QVariant>;
    account_list_name = new QList<QVariant>;
    // ------------------------------------------
    int32_t account_count = opentxs::OTAPI_Wrap::It()->GetAccountCount();

    for (int aa = 0; aa < account_count; aa++)
    {
        QString OT_account_id   = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAccountWallet_ID(aa));
        QString OT_account_name = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAccountWallet_Name(OT_account_id.toStdString()));

        account_list_id  ->append(QVariant(OT_account_id));
        account_list_name->append(QVariant(OT_account_name));

        QAction * next_account_action = new QAction(mc_systrayIcon_goldaccount, OT_account_name, mc_systrayMenu_account);
        next_account_action->setData(QVariant(OT_account_id));
        next_account_action->setCheckable(true);

        if (0 == OT_account_id.compare(default_account_id)) {
            next_account_action->setChecked(true);
        }
        else {
            next_account_action->setChecked(false);
        }

        mc_systrayMenu_account->addAction(next_account_action);
    }
    // -------------------------------------------------
    setDefaultAccount(default_account_id, default_account_name);
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
  * CRYPTO FUNCTIONS
  **/

void Moneychanger::mc_crypto_encrypt_slot()
{
    mc_encrypt_show_dialog(true, true);
}

void Moneychanger::mc_crypto_sign_slot()
{
    mc_encrypt_show_dialog(false, true);
}

void Moneychanger::mc_decrypt_show_dialog()
{
    // --------------------------------------------------
    DlgDecrypt * decrypt_window = new DlgDecrypt(NULL);
    decrypt_window->setAttribute(Qt::WA_DeleteOnClose);
    // --------------------------------------------------
    decrypt_window->dialog();
    // --------------------------------------------------
}

void Moneychanger::mc_crypto_decrypt_slot()
{
    mc_decrypt_show_dialog();
}

void Moneychanger::mc_crypto_verify_slot()
{
 //nothing for now. prolly forever.
}

void Moneychanger::mc_encrypt_show_dialog(bool bEncrypt/*=true*/, bool bSign/*=true*/)
{
    // --------------------------------------------------
    DlgEncrypt * encrypt_window = new DlgEncrypt(NULL);
    encrypt_window->setAttribute(Qt::WA_DeleteOnClose);
    // --------------------------------------------------
    encrypt_window->SetEncrypt(bEncrypt);
    encrypt_window->SetSign   (bSign);
    // ---------------------------------------
    encrypt_window->dialog();
    // --------------------------------------------------
}



void Moneychanger::mc_passphrase_manager_slot()
{
    mc_passphrase_manager_show_dialog();
}


void Moneychanger::mc_passphrase_manager_show_dialog()
{
    if (!passphrase_window)
        passphrase_window = new DlgPassphraseManager(this);
    // --------------------------------------------------
    passphrase_window->dialog();
}



/** 
 * Address Book 
 **/

// text may contain a "pre-selected" Contact ID (an integer in string form.)
void Moneychanger::mc_addressbook_show(QString text/*=QString("")*/)
{
    // The caller dosen't wish to have the address book paste to anything
    // (they just want to see/manage the address book), just call blank.
    //
    if (!contactswindow)
        contactswindow = new MTDetailEdit(this);
    // -------------------------------------
    contactswindow->m_map.clear();
    // -------------------------------------
    MTContactHandler::getInstance()->GetContacts(contactswindow->m_map);
    // -------------------------------------
    if (!text.isEmpty())
        contactswindow->SetPreSelected(text);
    // -------------------------------------
    contactswindow->setWindowTitle(tr("Contacts"));
    // -------------------------------------
    contactswindow->dialog(MTDetailEdit::DetailEditTypeContact);
}

void Moneychanger::mc_addressbook_slot()
{
    mc_addressbook_show();
}

void Moneychanger::mc_showcontact_slot(QString text)
{
    mc_addressbook_show(text);
}




/**  
 * Nym Manager 
 **/

//Nym manager "clicked"
void Moneychanger::mc_defaultnym_slot()
{
    //The operator has requested to open the dialog to the "Nym Manager";
    mc_nymmanager_dialog();
}

void Moneychanger::mc_show_nym_slot(QString text)
{
    mc_nymmanager_dialog(text);

}

void Moneychanger::mc_nymmanager_dialog(QString qstrPresetID/*=QString("")*/)
{
    QString qstr_default_id = this->get_default_nym_id();
    // -------------------------------------
    if (qstrPresetID.isEmpty())
        qstrPresetID = qstr_default_id;
    // -------------------------------------
    if (!nymswindow)
        nymswindow = new MTDetailEdit(this);
    // -------------------------------------
    mapIDName & the_map = nymswindow->m_map;
    // -------------------------------------
    the_map.clear();
    // -------------------------------------
    int32_t nym_count = opentxs::OTAPI_Wrap::It()->GetNymCount();
    bool bFoundPreset = false;

    for (int32_t ii = 0; ii < nym_count; ii++)
    {
        QString OT_id   = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetNym_ID(ii));
        QString OT_name = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetNym_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);
        // ------------------------------
        if (!qstrPresetID.isEmpty() && (qstrPresetID == OT_id))
            bFoundPreset = true;
        // ------------------------------
    } // for
    // -------------------------------------
    nymswindow->setWindowTitle(tr("Manage Identities"));
    // -------------------------------------
    if (bFoundPreset)
        nymswindow->SetPreSelected(qstrPresetID);
    // -------------------------------------
    nymswindow->dialog(MTDetailEdit::DetailEditTypeNym);
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
    if (nym_id != "")
    {
        mc_systrayMenu_nym->setTitle(tr("Identity: ")+nym_name);

        if (mc_overall_init)
        {
            // Loop through actions in mc_systrayMenu_nym.
            // Ignore the "openmanager" action.
            // For all others, compare the data to the default nym ID.
            // If one matches, set the "checked" property to true, and for
            // all others, set to false.

            foreach (QAction* a, mc_systrayMenu_nym->actions())
            {
                QString qstrActionData = a->data().toString();

                if (0 == qstrActionData.compare(default_nym_id)) {
                    a->setChecked(true);
                }
                else {
                    a->setChecked(false);
                }
            }
        }
    }
}


//Nym new default selected from systray
void Moneychanger::mc_nymselection_triggered(QAction*action_triggered)
{
    //Check if the user wants to open the nym manager (or) select a different default nym
    QString action_triggered_string = QVariant(action_triggered->data()).toString();
//    qDebug() << "NYM TRIGGERED" << action_triggered_string;

    if (action_triggered_string == "openmanager")
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
        if (nymswindow && !nymswindow->isHidden())
        {
            mc_nymmanager_dialog();
        }
    }
}

// End Nym Manager





void Moneychanger::onNeedToDownloadSingleAcct(QString qstrAcctID)
{
    if (qstrAcctID.isEmpty())
        return;
    // ------------------------------
    opentxs::OT_ME madeEasy;

    std::string accountId = qstrAcctID.toStdString();
    std::string acctNymID = opentxs::OTAPI_Wrap::It()->GetAccountWallet_NymID   (accountId);
    std::string acctSvrID = opentxs::OTAPI_Wrap::It()->GetAccountWallet_NotaryID(accountId);

    bool bRetrievalAttempted = false;
    bool bRetrievalSucceeded = false;

    if (!acctNymID.empty() && !acctSvrID.empty())
    {
        MTSpinner theSpinner;

        bRetrievalAttempted = true;
        bRetrievalSucceeded = madeEasy.retrieve_account(acctSvrID, acctNymID, accountId, true);
    }
    // ----------------------------------------------------------------
    if (bRetrievalAttempted)
    {
        if (!bRetrievalSucceeded) {
            Moneychanger::It()->HasUsageCredits(acctSvrID, acctNymID);
            return;
        }
        else {
            emit downloadedAccountData();
            return;
        }
    }
}

// ----------------------------------------------------------------

void Moneychanger::onNeedToDownloadAccountData()
{
    //Also refreshes/initializes client data

    QString qstrErrorMsg;
    qstrErrorMsg = tr("Failed trying to contact the notary. Perhaps it is down, or there might be a network problem.");
    // -----------------------------
    opentxs::OT_ME madeEasy;

    if ((get_server_list_id_size() > 0) && (get_asset_list_id_size() > 0) )
    {
        std::string defaultNotaryID(get_default_notary_id().toStdString());
        // ----------------------------------------------------------------
        if (defaultNotaryID.empty())
        {
            defaultNotaryID = get_notary_id_at(0).toStdString();
            DBHandler::getInstance()->AddressBookUpdateDefaultServer(QString::fromStdString(defaultNotaryID));
        }
        // ----------------------------------------------------------------
        int32_t nymCount = opentxs::OTAPI_Wrap::It()->GetNymCount();

        if (0 == nymCount)
        {
            qDebug() << "Making 'Me' Nym";

            std::string strSource(""), strAlt("");

            std::string newNymId = madeEasy.create_nym(1024, strSource, strAlt);

            if (!newNymId.empty())
            {
                opentxs::OTAPI_Wrap::It()->SetNym_Name(newNymId, newNymId, tr("Me").toLatin1().data());
                DBHandler::getInstance()->AddressBookUpdateDefaultNym(QString::fromStdString(newNymId));
                qDebug() << "Finished Making Nym";
            }

            nymCount = opentxs::OTAPI_Wrap::It()->GetNymCount();
        }
        // ----------------------------------------------------------------
        std::string defaultNymID(get_default_nym_id().toStdString());
        // ----------------------------------------------------------------
        if (!defaultNymID.empty() && !defaultNotaryID.empty())
        {
            bool isReg = opentxs::OTAPI_Wrap::It()->IsNym_RegisteredAtServer(defaultNymID, defaultNotaryID);

            if (!isReg)
            {
                std::string response;
                {
                    MTSpinner theSpinner;

                    response = madeEasy.register_nym(defaultNotaryID, defaultNymID);

                    if (opentxs::OTAPI_Wrap::networkFailure())
                    {
                        emit appendToLog(qstrErrorMsg);
                        return;
                    }
                }

                if (!madeEasy.VerifyMessageSuccess(response)) {
                    Moneychanger::It()->HasUsageCredits(defaultNotaryID, defaultNymID);
                    return;
                }

//              qDebug() << QString("Creation Response: %1").arg(QString::fromStdString(response));
            }
        }
        // ----------------------------------------------------------------
        std::string defaultInstrumentDefinitionID (get_default_asset_id().toStdString());
        // ----------------------------------------------------------------
        if (defaultInstrumentDefinitionID.empty())
        {
            defaultInstrumentDefinitionID = get_asset_id_at(0).toStdString();
            DBHandler::getInstance()->AddressBookUpdateDefaultAsset(QString::fromStdString(defaultInstrumentDefinitionID));
        }
        // ----------------------------------------------------------------
        int32_t accountCount = opentxs::OTAPI_Wrap::It()->GetAccountCount();

//        qDebug() << QString("Account Count: %1").arg(accountCount);

        if (0 == accountCount)
        {
            if (!defaultNymID.empty() && !defaultNotaryID.empty() && !defaultInstrumentDefinitionID.empty())
            {
                std::string response;
                {
                    MTSpinner theSpinner;
                    response = madeEasy.create_asset_acct(defaultNotaryID, defaultNymID, defaultInstrumentDefinitionID);

                    if (opentxs::OTAPI_Wrap::networkFailure())
                    {
                        emit appendToLog(qstrErrorMsg);
                        return;
                    }
                }

                if (!madeEasy.VerifyMessageSuccess(response)) {
                    Moneychanger::It()->HasUsageCredits(defaultNotaryID, defaultNymID);
                    return;
                }

                accountCount = opentxs::OTAPI_Wrap::It()->GetAccountCount();

                if (accountCount > 0)
                {
                    std::string accountID = opentxs::OTAPI_Wrap::It()->GetAccountWallet_ID(0);
                    opentxs::OTAPI_Wrap::It()->SetAccountWallet_Name(accountID, defaultNymID, tr("My Acct").toLatin1().data());

                    DBHandler::getInstance()->AddressBookUpdateDefaultAccount(QString::fromStdString(accountID));
                }
            }
        }
        // ----------------------------------------------------------------
        // Retrieve Nyms
        //
        int32_t serverCount = opentxs::OTAPI_Wrap::It()->GetServerCount();

        for (int32_t serverIndex = 0; serverIndex < serverCount; ++serverIndex)
        {
            std::string NotaryID = opentxs::OTAPI_Wrap::It()->GetServer_ID(serverIndex);

            for (int32_t nymIndex = 0; nymIndex < nymCount; ++nymIndex)
            {
                std::string nymId = opentxs::OTAPI_Wrap::It()->GetNym_ID(nymIndex);

                bool bRetrievalAttempted = false;
                bool bRetrievalSucceeded = false;

                if (opentxs::OTAPI_Wrap::It()->IsNym_RegisteredAtServer(nymId, NotaryID))
                {
                    MTSpinner theSpinner;

                    bRetrievalAttempted = true;
                    bRetrievalSucceeded = madeEasy.retrieve_nym(NotaryID, nymId, true);

                    if (opentxs::OTAPI_Wrap::networkFailure())
                    {
                        emit appendToLog(qstrErrorMsg);
                        return;
                    }
                }
                // ----------------------------------------------------------------
                if (bRetrievalAttempted && !bRetrievalSucceeded) {
                    Moneychanger::It()->HasUsageCredits(NotaryID, nymId);
                    return;
                }
            }
        }
        // ----------------------------------------------------------------
        for (int32_t i = 0; i < accountCount; i++)
        {
            std::string accountId = opentxs::OTAPI_Wrap::It()->GetAccountWallet_ID(i);
            std::string acctNymID = opentxs::OTAPI_Wrap::It()->GetAccountWallet_NymID(accountId);
            std::string acctSvrID = opentxs::OTAPI_Wrap::It()->GetAccountWallet_NotaryID(accountId);

            bool bRetrievalAttempted = false;
            bool bRetrievalSucceeded = false;

            {
                MTSpinner theSpinner;

                bRetrievalAttempted = true;
                bRetrievalSucceeded = madeEasy.retrieve_account(acctSvrID, acctNymID, accountId, true);

                if (opentxs::OTAPI_Wrap::networkFailure())
                {
                    emit appendToLog(qstrErrorMsg);
                    return;
                }
            }

            if (bRetrievalAttempted && !bRetrievalSucceeded) {
                Moneychanger::It()->HasUsageCredits(acctSvrID, acctNymID);
                return;
            }
        }
        // ----------------------------------------------------------------
        emit downloadedAccountData();
        return;
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
    if (!assetswindow)
        assetswindow = new MTDetailEdit(this);
    // -------------------------------------
    mapIDName & the_map = assetswindow->m_map;
    // -------------------------------------
    the_map.clear();
    // -------------------------------------
    int32_t  asset_count = opentxs::OTAPI_Wrap::It()->GetAssetTypeCount();
    bool    bFoundPreset = false;

    for (int32_t ii = 0; ii < asset_count; ii++)
    {
        QString OT_id   = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAssetType_ID(ii));
        QString OT_name = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAssetType_Name(OT_id.toStdString()));

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
    if (asset_id != "")
    {
        mc_systrayMenu_asset->setTitle(tr("Asset Type: ")+asset_name);

        if (mc_overall_init)
        {
            // Loop through actions in mc_systrayMenu_asset.
            // Ignore the "openmanager" action.
            // For all others, compare the data to the default asset ID.
            // If one matches, set the "checked" property to true, and for
            // all others, set to false.

            foreach (QAction* a, mc_systrayMenu_asset->actions())
            {
                QString qstrActionData = a->data().toString();

                if (0 == qstrActionData.compare(default_asset_id)) {
                    a->setChecked(true);
                }
                else {
                    a->setChecked(false);
                }
            }
        }
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
        if (assetswindow && !assetswindow->isHidden())
        {
            mc_assetmanager_dialog();
        }
    }
    
}

// End Asset Manager






void Moneychanger::onBalancesChanged()
{
    SetupMainMenu();

    emit balancesChanged();
}

void Moneychanger::onNeedToUpdateMenu()
{
    SetupMainMenu();
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


void Moneychanger::mc_show_account_slot(QString text)
{
    mc_accountmanager_dialog(text);
}

void Moneychanger::mc_show_account_manager_slot()
{
    mc_accountmanager_dialog();
}

void Moneychanger::mc_accountmanager_dialog(QString qstrAcctID/*=QString("")*/)
{
    QString qstr_default_acct_id = this->get_default_account_id();
    // -------------------------------------
    if (qstrAcctID.isEmpty())
        qstrAcctID = qstr_default_acct_id;
    // -------------------------------------
    if (!accountswindow)
    {
        accountswindow = new MTDetailEdit(this);

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

        qDebug() << "Account Manager Opened";
    }
    // -------------------------------------
    mapIDName & the_map = accountswindow->m_map;
    // -------------------------------------
    the_map.clear();
    // -------------------------------------
    int32_t acct_count = opentxs::OTAPI_Wrap::It()->GetAccountCount();
    bool bFoundDefault = false;

    for (int32_t ii = 0; ii < acct_count; ii++)
    {
        QString OT_id   = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAccountWallet_ID(ii));
        QString OT_name = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAccountWallet_Name(OT_id.toStdString()));

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


//Account Manager Additional Functions

//Account new default selected from systray
void Moneychanger::mc_accountselection_triggered(QAction*action_triggered)
{
    //Check if the user wants to open the account manager (or) select a different default account
    QString action_triggered_string = QVariant(action_triggered->data()).toString();
//    qDebug() << "account TRIGGERED" << action_triggered_string;

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
        //Check if account manager has ever been opened (then apply logic) [prevents crash if the dialog hasen't be opened before]
        //
        if (accountswindow && !accountswindow->isHidden())
        {
            mc_accountmanager_dialog();
        }
        // ------------------------------
        // NOTE: I just commented this out because it's already done in setDefaultAccount (above.)
//      emit downloadedAccountData();
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
        
        int64_t     lBalance  = opentxs::OTAPI_Wrap::It()->GetAccountWallet_Balance    (account_id.toStdString());
        std::string strAsset  = opentxs::OTAPI_Wrap::It()->GetAccountWallet_InstrumentDefinitionID(account_id.toStdString());
        // ----------------------------------------------------------
        std::string str_amount;
        
        if (!strAsset.empty())
        {
            str_amount = opentxs::OTAPI_Wrap::It()->FormatAmount(strAsset, lBalance);
            result += " ("+ QString::fromStdString(str_amount) +")";
        }
        
        mc_systrayMenu_account->setTitle(result);
        // -----------------------------------------------------------
        std::string strNym    = opentxs::OTAPI_Wrap::It()->GetAccountWallet_NymID   (account_id.toStdString());
        std::string strServer = opentxs::OTAPI_Wrap::It()->GetAccountWallet_NotaryID(account_id.toStdString());

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
                QString qstrAssetName = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAssetType_Name(strAsset));

                if (!qstrAssetName.isEmpty() && (mc_systrayMenu_asset))
                    setDefaultAsset(QString::fromStdString(strAsset),
                                    qstrAssetName);
            }
            // -----------------------------------------------------------
            if (!strNym.empty())
            {
                QString qstrNymName = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetNym_Name(strNym));

                if (!qstrNymName.isEmpty() && (mc_systrayMenu_nym))
                    setDefaultNym(QString::fromStdString(strNym),
                                  qstrNymName);
            }
            // -----------------------------------------------------------
            if (!strServer.empty())
            {
                QString qstrServerName = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetServer_Name(strServer));

                if (!qstrServerName.isEmpty() && (mc_systrayMenu_server))
                    setDefaultServer(QString::fromStdString(strServer),
                                     qstrServerName);
            }
            // -----------------------------------------------------------

            // Loop through actions in mc_systrayMenu_account.
            // Ignore the "openmanager" action.
            // For all others, compare the data to the default account ID.
            // If one matches, set the "checked" property to true, and for
            // all others, set to false.

            foreach (QAction* a, mc_systrayMenu_account->actions())
            {
                QString qstrActionData = a->data().toString();

                if (0 == qstrActionData.compare(default_account_id)) {
                    a->setChecked(true);
                }
                else {
                    a->setChecked(false);
                }
            }
            // -----------------------------------------------------------
            emit downloadedAccountData();
        }
        // -----------------------------------------------------------
    }
}





void Moneychanger::mc_show_asset_slot(QString text)
{
    mc_assetmanager_dialog(text);
}


void Moneychanger::mc_show_server_slot(QString text)
{
    mc_servermanager_dialog(text);
}



/** 
 * Server Manager 
 **/

void Moneychanger::mc_defaultserver_slot()
{
    mc_servermanager_dialog();
}


void Moneychanger::mc_servermanager_dialog(QString qstrPresetID/*=QString("")*/)
{
    QString qstr_default_id = this->get_default_notary_id();
    // -------------------------------------
    if (qstrPresetID.isEmpty())
        qstrPresetID = qstr_default_id;
    // -------------------------------------
    if (!serverswindow)
        serverswindow = new MTDetailEdit(this);
    // -------------------------------------
    mapIDName & the_map = serverswindow->m_map;
    // -------------------------------------
    the_map.clear();
    // -------------------------------------
    int32_t server_count = opentxs::OTAPI_Wrap::It()->GetServerCount();
    bool    bFoundPreset = false;

    for (int32_t ii = 0; ii < server_count; ii++)
    {
        QString OT_id   = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetServer_ID(ii));
        QString OT_name = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetServer_Name(OT_id.toStdString()));

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




void Moneychanger::setDefaultServer(QString notary_id, QString server_name)
{
    //Set default server internal memory
    default_notary_id = notary_id;
    default_server_name = server_name;
    
//    qDebug() << default_notary_id;
//    qDebug() << default_server_name;
    
    //SQL UPDATE default server
    DBHandler::getInstance()->AddressBookUpdateDefaultServer(default_notary_id);
    
    //Update visuals
    QString new_server_title = default_server_name;

    if (new_server_title.isEmpty())
    {
        new_server_title = tr("Set Default...");
    }
    
    if (mc_overall_init)
    {
        mc_systrayMenu_server->setTitle(tr("Server: ")+new_server_title);

        // Loop through actions in mc_systrayMenu_server.
        // Ignore the "openmanager" action.
        // For all others, compare the data to the default server ID.
        // If one matches, set the "checked" property to true, and for
        // all others, set to false.

        foreach (QAction* a, mc_systrayMenu_server->actions())
        {
            QString qstrActionData = a->data().toString();

            if (0 == qstrActionData.compare(default_notary_id)) {
                a->setChecked(true);
            }
            else {
                a->setChecked(false);
            }
        }
    }
}

//Server Slots

void Moneychanger::mc_serverselection_triggered(QAction * action_triggered)
{
    //Check if the user wants to open the nym manager (or) select a different default nym
    QString action_triggered_string = QVariant(action_triggered->data()).toString();
//    qDebug() << "SERVER TRIGGERED" << action_triggered_string;

    if(action_triggered_string == "openmanager"){
        //Open server-list manager
        mc_defaultserver_slot();
    }
    else
    {
        //Set new server default
        QString action_triggered_string_server_name = QVariant(action_triggered->text()).toString();
        setDefaultServer(action_triggered_string, action_triggered_string_server_name);
        
        //Refresh if the server manager is currently open
        if (serverswindow && !serverswindow->isHidden())
        {
            mc_servermanager_dialog();
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

void Moneychanger::mc_request_to_acct(QString qstrAcct)
{
    mc_requestfunds_show_dialog(qstrAcct);
}

void Moneychanger::mc_requestfunds_show_dialog(QString qstrAcct/*=QString("")*/)
{
    // --------------------------------------------------
    MTRequestDlg * request_window = new MTRequestDlg(NULL);
    request_window->setAttribute(Qt::WA_DeleteOnClose);
    // --------------------------------------------------
    QString qstr_acct_id = qstrAcct.isEmpty() ? this->get_default_account_id() : qstrAcct;

    if (!qstr_acct_id.isEmpty())
        request_window->setInitialMyAcct(qstr_acct_id);
    // ---------------------------------------
    request_window->dialog();
    // --------------------------------------------------
}



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
                QMessageBox::warning(this, tr("Moneychanger"),
                                     QString("%1: %2").arg(tr("File was apparently empty")).arg(fileName));
                return;
            }
            // ----------------------------
        }
        else
        {
            QMessageBox::warning(this, tr("Moneychanger"),
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
    std::string strType = opentxs::OTAPI_Wrap::It()->Instrmnt_GetType(strInstrument);

    if (strType.empty())
    {
        QMessageBox::warning(this, tr("Moneychanger"),
                             tr("Unable to determine instrument type. Are you sure this is a financial instrument?"));
        return;
    }
    // -----------------------
    std::string strNotaryID = opentxs::OTAPI_Wrap::It()->Instrmnt_GetNotaryID(strInstrument);

    if (strNotaryID.empty())
    {
        QMessageBox::warning(this, tr("Indeterminate Server"),
                             tr("Unable to determine server ID for this instrument. Are you sure it's properly formed?"));
        return;
    }
    // -----------------------
    std::string strInstrumentDefinitionID = opentxs::OTAPI_Wrap::It()->Instrmnt_GetInstrumentDefinitionID(strInstrument);

    if (strInstrumentDefinitionID.empty())
    {
        QMessageBox::warning(this, tr("Indeterminate Asset Type"),
                             tr("Unable to determine asset ID for this instrument. Are you sure it's properly formed?"));
        return;
    }
    // -----------------------
    std::string strServerContract = opentxs::OTAPI_Wrap::It()->LoadServerContract(strNotaryID);

    if (strServerContract.empty())
    {
        QMessageBox::warning(this, tr("No Server Contract Found"),
                             QString("%1 '%2'<br/>%3").arg(tr("Unable to load the server contract for server ID")).
                             arg(QString::fromStdString(strNotaryID)).arg(tr("Are you sure that server contract is even in your wallet?")));
        return;
    }
    // -----------------------
    std::string strAssetContract = opentxs::OTAPI_Wrap::It()->LoadAssetContract(strInstrumentDefinitionID);

    if (strAssetContract.empty())
    {
        QMessageBox::warning(this, tr("No Asset Contract Found"),
                             QString("%1 '%2'<br/>%3").arg(tr("Unable to load the asset contract for asset ID")).
                             arg(QString::fromStdString(strInstrumentDefinitionID)).arg(tr("Are you sure that asset type is even in your wallet?")));
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
    const bool  bHasPassword = opentxs::OTAPI_Wrap::It()->Purse_HasPassword(strNotaryID, strInstrument);
    std::string strPurseOwner("");

    if (!bHasPassword)
    {
        // If the purse isn't password-protected, then it's DEFINITELY encrypted to
        // a specific Nym.
        //
        // The purse MAY include the NymID for this Nym, but it MAY also be blank, in
        // which case the user will have to select a Nym to TRY.
        //
        strPurseOwner = opentxs::OTAPI_Wrap::It()->Instrmnt_GetRecipientNymID(strInstrument); // TRY and get the Nym ID (it may have been left blank.)

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
            const int32_t nym_count = opentxs::OTAPI_Wrap::It()->GetNymCount();
            // -----------------------------------------------
            for (int32_t ii = 0; ii < nym_count; ++ii)
            {
                //Get OT Nym ID
                QString OT_nym_id = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetNym_ID(ii));
                QString OT_nym_name("");
                // -----------------------------------------------
                if (!OT_nym_id.isEmpty())
                {
                    if (!default_nym_id.isEmpty() && (OT_nym_id == default_nym_id))
                        bFoundDefault = true;
                    // -----------------------------------------------
                    MTNameLookupQT theLookup;

                    OT_nym_name = QString::fromStdString(theLookup.GetNymName(OT_nym_id.toStdString(), ""));
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
        if (!opentxs::OTAPI_Wrap::It()->IsNym_RegisteredAtServer(strPurseOwner, strNotaryID))
        {
            QMessageBox::warning(this, tr("Nym Isn't Registered at Server"),
                                 QString("%1 '%2'<br/>%3 '%4'<br/>%5").
                                 arg(tr("The Nym with ID")).
                                 arg(QString::fromStdString(strPurseOwner)).
                                 arg(tr("isn't registered at the Server with ID")).
                                 arg(QString::fromStdString(strNotaryID)).
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
    // Use opentxs::OT_ME::deposit_cash (vs deposit_local_purse) since the cash is external.
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
    const int32_t acct_count = opentxs::OTAPI_Wrap::It()->GetAccountCount();
    // -----------------------------------------------
    for (int32_t ii = 0; ii < acct_count; ++ii)
    {
        //Get OT Acct ID
        QString OT_acct_id = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAccountWallet_ID(ii));
        QString OT_acct_name("");
        // -----------------------------------------------
        if (!OT_acct_id.isEmpty())
        {
            std::string str_acct_nym_id    = opentxs::OTAPI_Wrap::It()->GetAccountWallet_NymID      (OT_acct_id.toStdString());
            std::string str_acct_asset_id  = opentxs::OTAPI_Wrap::It()->GetAccountWallet_InstrumentDefinitionID(OT_acct_id.toStdString());
            std::string str_acct_notary_id = opentxs::OTAPI_Wrap::It()->GetAccountWallet_NotaryID   (OT_acct_id.toStdString());

            if (!strPurseOwner.empty() && (0 != strPurseOwner.compare(str_acct_nym_id)))
                continue;
            if (0 != strInstrumentDefinitionID.compare(str_acct_asset_id))
                continue;
            if (0 != strNotaryID.compare(str_acct_notary_id))
                continue;
            // -----------------------------------------------
            if (!default_account_id.isEmpty() && (OT_acct_id == default_account_id))
                bFoundDefault = true;
            // -----------------------------------------------
            MTNameLookupQT theLookup;

            OT_acct_name = QString::fromStdString(theLookup.GetAcctName(OT_acct_id.toStdString(), "", "", ""));
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
                strPurseOwner = opentxs::OTAPI_Wrap::It()->GetAccountWallet_NymID(theChooser.m_qstrCurrentID.toStdString());
            // -------------------------------------------
            opentxs::OT_ME madeEasy;
//          const bool bImported = opentxs::OTAPI_Wrap::It()->Wallet_ImportPurse(strNotaryID, strInstrumentDefinitionID, strPurseOwner, strInstrument);

            int32_t nDepositCash = 0;
            {
                MTSpinner theSpinner;

                nDepositCash = madeEasy.deposit_cash(strNotaryID, strPurseOwner,
                                                     theChooser.m_qstrCurrentID.toStdString(), // AcctID.
                                                     strInstrument);
            }
            // --------------------------------------------
            if (1 == nDepositCash)
            {
                QMessageBox::information(this, tr("Success"), tr("Success depositing cash purse."));
                emit balancesChanged();
            }
            else
            {
                const int64_t lUsageCredits = Moneychanger::It()->HasUsageCredits(strNotaryID, strPurseOwner);

                // In the case of -2 and 0, the problem has to do with the usage credits,
                // and it already pops up an error box. Otherwise, the user had enough usage
                // credits, so there must have been some other problem, so we pop up an error box.
                //
                if ((lUsageCredits != (-2)) && (lUsageCredits != 0)) {
                    QMessageBox::information(this, tr("Import Failure"), tr("Failed trying to deposit cash purse."));
                    return;
                }
            }
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

void Moneychanger::mc_sendfunds_show_dialog(QString qstrAcct/*=QString("")*/)
{
    // --------------------------------------------------
    MTSendDlg * send_window = new MTSendDlg(NULL);
    send_window->setAttribute(Qt::WA_DeleteOnClose);
    // --------------------------------------------------
    QString qstr_acct_id = qstrAcct.isEmpty() ? this->get_default_account_id() : qstrAcct;

    if (!qstr_acct_id.isEmpty())
        send_window->setInitialMyAcct(qstr_acct_id);
    // ---------------------------------------
    send_window->dialog();
    // --------------------------------------------------
}


void Moneychanger::mc_send_from_acct(QString qstrAcct)
{
    mc_sendfunds_show_dialog(qstrAcct);
}


/**
 * Compose Message
 **/

void Moneychanger::mc_composemessage_slot()
{
    mc_composemessage_show_dialog();
}

void Moneychanger::mc_composemessage_show_dialog()
{
    // --------------------------------------------------
    MTCompose * compose_window = new MTCompose;
    compose_window->setAttribute(Qt::WA_DeleteOnClose);
    // --------------------------------------------------
    // If Moneychanger has a default Nym set, we use that for the Sender.
    // (User can always change it.)
    //
    QString qstrDefaultNym = this->get_default_nym_id();

    if (!qstrDefaultNym.isEmpty()) // Sender Nym is set.
        compose_window->setInitialSenderNym(qstrDefaultNym);
    // --------------------------------------------------
    QString qstrDefaultServer = this->get_default_notary_id();

    if (!qstrDefaultServer.isEmpty())
        compose_window->setInitialServer(qstrDefaultServer);
    // --------------------------------------------------
    compose_window->dialog();
    compose_window->show();
    // --------------------------------------------------
}



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
    if (homewindow && !homewindow->isHidden())
    {
        mc_overview_dialog();
    }
}

void Moneychanger::mc_overview_dialog()
{
    if (!homewindow)
    {
        // MTHome is a widget, not a dialog. Therefore *this is set as the parent.
        // (Therefore no need to delete it ever, since *this will delete it on destruction.)
        //
        homewindow = new MTHome(this);

        connect(homewindow, SIGNAL(needToDownloadAccountData()),
                this,       SLOT(onNeedToDownloadAccountData()));

        connect(this,       SIGNAL(downloadedAccountData()),
                homewindow, SLOT(onAccountDataDownloaded()));

        connect(this,       SIGNAL(balancesChanged()),
                homewindow, SLOT(onBalancesChanged()));

        qDebug() << "Overview Opened";
    }
    // ---------------------------------
    homewindow->dialog();
}



// End Overview

void Moneychanger::onNewServerAdded(QString qstrID)
{
    if (homewindow)
    {
        homewindow->onNewServerAdded(qstrID);
    }
}

void Moneychanger::onNewAssetAdded(QString qstrID)
{
    if (homewindow)
    {
        homewindow->onNewAssetAdded(qstrID);
    }
}

void Moneychanger::onNewNymAdded(QString qstrID)
{
    if (homewindow)
    {
        homewindow->onNewNymAdded(qstrID);
    }
}

void Moneychanger::onNewAccountAdded(QString qstrID)
{
    if (homewindow)
    {
        homewindow->onNewAccountAdded(qstrID);
    }
}




/**
 * Main Menu Window  (For people who can't see the menu on the systray.)
 **/

// Main Menu slots
void Moneychanger::mc_main_menu_slot()
{
    //The operator has requested to open the dialog to the "main menu";
    mc_main_menu_dialog();
}

// --------------------------------------------------

void Moneychanger::mc_main_menu_dialog()
{
    if (!menuwindow)
    {
        // --------------------------------------------------
        menuwindow = new DlgMenu(this);
        // --------------------------------------------------
        connect(menuwindow, SIGNAL(sig_on_toolButton_main_clicked()),
                this,       SLOT(mc_overview_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_markets_clicked()),
                this,       SLOT(mc_market_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_importCash_clicked()),
                this,       SLOT(mc_import_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_contacts_clicked()),
                this,       SLOT(mc_addressbook_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_manageAccounts_clicked()),
                this,       SLOT(mc_show_account_manager_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_manageAssets_clicked()),
                this,       SLOT(mc_defaultasset_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_manageNyms_clicked()),
                this,       SLOT(mc_defaultnym_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_manageServers_clicked()),
                this,       SLOT(mc_defaultserver_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_smartContracts_clicked()),
                this,       SLOT(mc_agreement_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_Corporations_clicked()),
                this,       SLOT(mc_corporation_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_Transport_clicked()),
                this,       SLOT(mc_transport_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_settings_clicked()),
                this,       SLOT(mc_settings_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_quit_clicked()),
                this,       SLOT(mc_shutdown_slot()));

        qDebug() << "Main Menu Opened";
    }
    // ---------------------------------
    menuwindow->dialog();
}

// End Main Menu





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
    if (!agreement_window)
        agreement_window = new MTDetailEdit(this);
    // -------------------------------------
    mapIDName & the_map = agreement_window->m_map;
    // -------------------------------------
    the_map.clear();
    // -------------------------------------
    MTContactHandler::getInstance()->GetSmartContracts(the_map);
    // -------------------------------------
    agreement_window->setWindowTitle(tr("Smart Contracts"));
    // -------------------------------------
    agreement_window->dialog(MTDetailEdit::DetailEditTypeAgreement);
}



/**
 * Market Window
 **/

void Moneychanger::mc_market_slot()
{
    mc_market_dialog();
}

void Moneychanger::mc_market_dialog()
{
    if (!market_window)
    {
        market_window = new DlgMarkets(this);

        // When Moneychanger's signal "balancesChanged" is triggered,
        // it will call accountswindow's "onBalancesChangedFromAbove" function.
        //
        connect(this,          SIGNAL(balancesChanged()),
                market_window, SLOT(onBalancesChangedFromAbove()));
    }
    // ------------------------------------
    market_window->dialog();
}




// LOG:  The Error Log dialog.

void Moneychanger::mc_log_slot()
{
    mc_log_dialog();
}

// Same, except choose a specific one when opening.
void Moneychanger::mc_showlog_slot(QString text)
{
    mc_log_dialog(text);
}

void Moneychanger::mc_log_dialog(QString qstrAppend/*=QString("")*/)
{
    if (!log_window)
        log_window = new DlgLog(this);
    // -------------------------------------
    if (!qstrAppend.isEmpty())
        log_window->appendToLog(qstrAppend);
    // -------------------------------------
//    log_window->setWindowTitle(tr("Error Log"));
    // -------------------------------------
    log_window->dialog();
}




// CORPORATIONS

void Moneychanger::mc_corporation_slot()
{
    mc_corporation_dialog();
}

void Moneychanger::mc_corporation_dialog()
{
    // -------------------------------------
    if (!corporation_window)
        corporation_window = new MTDetailEdit(this);
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



// TRANSPORT:  Various messaging system connection info

void Moneychanger::mc_transport_slot()
{
    mc_transport_dialog();
}

// Same, except choose a specific one when opening.
void Moneychanger::mc_showtransport_slot(QString text)
{
    mc_transport_dialog(text);
}

void Moneychanger::mc_transport_dialog(QString qstrPresetID/*=QString("")*/)
{
    if (!transport_window)
        transport_window = new MTDetailEdit(this);
    // -------------------------------------
    transport_window->m_map.clear();
    // -------------------------------------
    MTContactHandler::getInstance()->GetMsgMethods(transport_window->m_map);
    // -------------------------------------
    if (!qstrPresetID.isEmpty())
        transport_window->SetPreSelected(qstrPresetID);
    // -------------------------------------
    transport_window->setWindowTitle(tr("Transport Methods"));
    // -------------------------------------
    transport_window->dialog(MTDetailEdit::DetailEditTypeTransport);
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
    if(!createinsurancecompany_window)
        createinsurancecompany_window = new CreateInsuranceCompany(this);
    // ------------------------------------
    createinsurancecompany_window->show();
}


void Moneychanger::onRunSmartContract(QString qstrTemplate, QString qstrLawyerID, int32_t index)
{
    if (qstrTemplate.isEmpty())
        return;

    std::string str_template = qstrTemplate.toStdString();
    // ------------------------------------------------
    if (qstrLawyerID.isEmpty())
        qstrLawyerID = get_default_nym_id();
    // ------------------------------------------------
    if (0 == opentxs::OTAPI_Wrap::It()->Smart_GetPartyCount(str_template))
    {
       QMessageBox::information(this, tr("Moneychanger"), tr("There are no parties listed on this smart contract, so you cannot sign it as a party."));
       return;
    }
    // ------------------------------------------------
    if (opentxs::OTAPI_Wrap::It()->Smart_AreAllPartiesConfirmed(str_template))
    {
        QMessageBox::information(this, tr("Moneychanger"), tr("Strange, all parties are already confirmed on this contract. (Failure.)"));
        return;
    }
    // ------------------------------------------------
    std::string str_server = opentxs::OTAPI_Wrap::It()->Instrmnt_GetNotaryID(str_template);
    // ------------------------------------------------    
    WizardRunSmartContract theWizard(this);

    theWizard.setWindowTitle(tr("Run smart contract"));

    if (!str_server.empty())
        theWizard.setField(QString("NotaryID"), QString::fromStdString(str_server));
    // ------------------------------------------------
    if (!qstrLawyerID.isEmpty())
        theWizard.setField(QString("NymID"), qstrLawyerID);

    theWizard.setField(QString("SmartTemplate"), qstrTemplate);
    // ------------------------------------------------
    theWizard.setField(QString("NymPrompt"), QString(tr("Choose a Nym to be a party to, and a signer of, the smart contract:")));
    // ------------------------------------------------
    if (QDialog::Accepted != theWizard.exec())
        return;
    // ------------------------------------------------
    qstrLawyerID = theWizard.field("NymID") .toString();
    std::string str_lawyer_id = qstrLawyerID.toStdString();

    QString qstrNotaryID = theWizard.field("NotaryID").toString();
    str_server = qstrNotaryID.toStdString();
    // -----------------------------------------
    QString qstrPartyName  = theWizard.field("PartyName") .toString();
    std::string str_party  = qstrPartyName.toStdString();
    // ---------------------------------------------------
    // By this point the user has selected the server ID, the Nym ID, and the Party Name.
    //
    std::string serverFromContract = opentxs::OTAPI_Wrap::It()->Instrmnt_GetNotaryID(str_template);
    if ("" != serverFromContract && str_server != serverFromContract) {
        QMessageBox::information(this, tr("Moneychanger"), tr("Mismatched server ID in contract. (Failure.)"));
        return;
    }
    // ----------------------------------------------------
    if (!opentxs::OTAPI_Wrap::It()->IsNym_RegisteredAtServer(str_lawyer_id, str_server)) {
        QMessageBox::information(this, tr("Moneychanger"), tr("Nym is not registered on server. (Failure.)"));
        return;
    }
    // ----------------------------------------------------
    // See if there are accounts for that party via Party_GetAcctCount.
    // If there are, confirm those accounts.
    //
    int32_t nAccountCount = opentxs::OTAPI_Wrap::It()->Party_GetAcctCount(str_template, str_party);

    if (0 >= nAccountCount)
    {
        QMessageBox::information(this, tr("Moneychanger"), tr("Strange, the chosen party has no accounts named on this smart contract. (Failed.)"));
        return;
    }
    // ----------------------------------------------------
    // Need to LOOP here until all the accounts are confirmed.

    mapIDName mapConfirmed, mapAgents;

    while (nAccountCount > 0)
    {
        WizardPartyAcct otherWizard(this);

        otherWizard.setWindowTitle(tr("Run smart contract"));

        otherWizard.setField("PartyName", qstrPartyName);
        otherWizard.setField("SmartTemplate", qstrTemplate);

        std::string str_shown_state;
        if (showPartyAccounts(str_template, str_party, str_shown_state) && !str_shown_state.empty())
            otherWizard.setField("AccountState", QString::fromStdString(str_shown_state));

        otherWizard.m_qstrNotaryID = qstrNotaryID;
        otherWizard.m_qstrNymID = qstrLawyerID;

        otherWizard.m_mapConfirmed = mapConfirmed;
        // ------------------------------------------------
        if (QDialog::Accepted != otherWizard.exec())
            return;

        mapConfirmed = otherWizard.m_mapConfirmed;
        // ------------------------------------------------
        QString qstrAcctName = otherWizard.field("AcctName").toString();
        std::string str_acct_name = qstrAcctName.toStdString();

        QString qstrAcctID = otherWizard.field("AcctID").toString();
        std::string str_acct_id = qstrAcctID.toStdString();
        // -----------------------------------------
        std::string agentName = opentxs::OTAPI_Wrap::It()->Party_GetAcctAgentName(str_template, str_party, str_acct_name);

        if ("" == agentName)
            agentName = opentxs::OTAPI_Wrap::It()->Party_GetAgentNameByIndex(str_template, str_party, 0);

        if ("" == agentName)
        {
            QMessageBox::information(this, tr("Moneychanger"), tr("Strange, but apparently this smart contract doesn't have any agents for this party. (Failed.)"));
            return;
        }

        mapConfirmed.insert(qstrAcctName, qstrAcctID);
        mapAgents.insert(qstrAcctName, QString::fromStdString(agentName));

        --nAccountCount;
    }
    // --------------------------------------------
    // By this point, mapConfirmed and mapAgents are populated for all the relevant accounts.
    //
    // Let's make sure we have enough transaction numbers for all those accounts.
    //
    int32_t needed = 0;

    for (auto x = mapConfirmed.begin(); x != mapConfirmed.end(); x++)
    {
        QString qstrAgent = mapAgents[x.key()];

        needed += opentxs::OTAPI_Wrap::It()->SmartContract_CountNumsNeeded(str_template, qstrAgent.toStdString());
    }
    // --------------------------------------------
    opentxs::OT_ME ot_me;
    if (!ot_me.make_sure_enough_trans_nums(needed + 1, str_server, str_lawyer_id))
    {
        QMessageBox::information(this, tr("Moneychanger"), tr("Failed trying to reserve enough transaction numbers from the notary."));
        return;
    }
    // --------------------------------------------
    // CONFIRM THE ACCOUNTS HERE
    //
    // Note: Any failure below this point needs to harvest back ALL
    // transaction numbers. Because we haven't even TRIED to activate it,
    // therefore ALL numbers on the contract are still good (even the opening
    // number.)
    //
    // Whereas after a failed activation, we'd need to harvest only the closing
    // numbers, and not the opening numbers. But in here, this is confirmation,
    // not activation.
    //
    std::string myAcctID = "";
    std::string myAcctAgentName = "";

    QString qstr_default_acct_id = get_default_account_id();

    for (auto x = mapConfirmed.begin(); x != mapConfirmed.end(); x++)
    {
        // Here we check to see if default account ID exists -- if so we compare it to the
        // current acctID in the loop and if they match, we set myAcctID. Later on,
        // if/when activating, we can just use myAcctID to activate.
        // (Otherwise we will have to pick one from the confirmed accounts.)
        if ("" == myAcctID && qstr_default_acct_id == x.value())
        {
            myAcctID = qstr_default_acct_id.toStdString();
            QString qstrMyAcctAgentName = mapAgents[x.key()];
            myAcctAgentName = qstrMyAcctAgentName.toStdString();
        }

        QString qstrCurrentAcctName  = x.key();
        QString qstrCurrentAcctID    = x.value();
        QString qstrCurrentAgentname = mapAgents[x.key()];

        // confirm a theoretical acct by giving it a real acct id.
        std::string confirmed = opentxs::OTAPI_Wrap::It()->SmartContract_ConfirmAccount(
            str_template, str_lawyer_id, str_party, qstrCurrentAcctName.toStdString(), qstrCurrentAgentname.toStdString(), qstrCurrentAcctID.toStdString());

        if ("" == confirmed)
        {
            qDebug() << "Failure while calling "
                     "OT_API_SmartContract_ConfirmAccount. Acct Name: "
                  << qstrCurrentAcctName << "  Agent Name: " << qstrCurrentAgentname
                  << "  Acct ID: " << qstrCurrentAcctID << " \n";

            QMessageBox::information(this, tr("Moneychanger"), tr("Failed while calling OT_API_SmartContract_ConfirmAccount."));

            opentxs::OTAPI_Wrap::It()->Msg_HarvestTransactionNumbers(str_template, str_lawyer_id, false, false, false, false, false);

            return;
        }

        str_template = confirmed;
    }
    // ----------------------------------------
    // Then we try to activate it or pass on to the next party.

    std::string confirmed =
        opentxs::OTAPI_Wrap::It()->SmartContract_ConfirmParty(str_template, str_party, str_lawyer_id);

    if ("" == confirmed)
    {
        qDebug() << "Error: cannot confirm smart contract party.\n";
        QMessageBox::information(this, tr("Moneychanger"), tr("Failed while calling SmartContract_ConfirmParty."));
        opentxs::OTAPI_Wrap::It()->Msg_HarvestTransactionNumbers(str_template, str_lawyer_id, false, false, false, false, false);
        return;
    }

    if (opentxs::OTAPI_Wrap::It()->Smart_AreAllPartiesConfirmed(confirmed))
    {
        // If you are the last party to sign, then ACTIVATE THE SMART CONTRACT.
        activateContract(str_server, str_lawyer_id, confirmed, str_party, myAcctID, myAcctAgentName);
        return;
    }
    // -------------------------------
    // Below this point, we know there are still parties waiting to confirm the contract before
    // it can be activated.
    // So we pop up a list of Contacts to send the contract on to next.
    // -----------------------------------------------
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;

    MTContactHandler::getInstance()->GetContacts(the_map);

    theChooser.setWindowTitle(tr("Choose the next signer"));
    // -----------------------------------------------
    if (theChooser.exec() != QDialog::Accepted)
    {
        opentxs::OTAPI_Wrap::It()->Msg_HarvestTransactionNumbers(str_template, str_lawyer_id, false, false, false, false, false);
        return;
    }
    // -----------------------------------------------
    QString qstrContactName    = theChooser.m_qstrCurrentName;
    int     nSelectedContactID = theChooser.m_qstrCurrentID.toInt();
    // -----------------------------------------------
    DlgChooser theNymChooser(this);

    mapIDName & the_nym_map = theNymChooser.m_map;

    MTContactHandler::getInstance()->GetNyms(the_nym_map, nSelectedContactID);

    theNymChooser.setWindowTitle(tr("Choose one of his nyms"));
    // -----------------------------------------------
    if (theNymChooser.exec() != QDialog::Accepted)
    {
        opentxs::OTAPI_Wrap::It()->Msg_HarvestTransactionNumbers(str_template, str_lawyer_id, false, false, false, false, false);
        return;
    }
    // -----------------------------------------------
    QString qstrNymID   = theNymChooser.m_qstrCurrentID;
    QString qstrNymName = theNymChooser.m_qstrCurrentName;
    // -----------------------------------------------
    // NOTE: No matter which party you are (perhaps you are the middle one),
    // when you confirm the contract, you will send it on to the NEXT
    // UNCONFIRMED ONE. This means you don't know which party it will be,
    // since all the unconfirmed parties have no NymID (yet.) Rather, it's
    // YOUR problem to provide the NymID you're sending the contract on to.
    // And then it's HIS problem to decide which party he will sign on as.
    // (Unless you are the LAST PARTY to confirm, in which case YOU are the
    // activator.)
    //
    sendToNextParty(str_server, str_lawyer_id, qstrNymID.toStdString(), confirmed);

    if (-1 != index)
    {
        // not a pasted contract, but it's an index in the payments inbox.
        //
        opentxs::OTAPI_Wrap::It()->RecordPayment(str_server, str_lawyer_id, true, index, false);
    }
}



int32_t Moneychanger::activateContract(const std::string& server, const std::string& mynym,
                                       const std::string& contract, const std::string& name,
                                       std::string myAcctID,
                                       std::string myAcctAgentName)
{
    // We don't need MyAcct except when actually ACTIVATING the smart contract
    // on the server. This variable might get set later to MyAcct, if it matches
    // one of the accounts being confirmed. (Meaning if this variable is set by
    // the time we reach the bottom, then we can use it for activation, if/when
    // needed.)

    // We need the ACCT_ID that we're using to activate it with, and we need the
    // AGENT NAME for that account.
    if ("" == myAcctID || "" == myAcctAgentName)
    {
        DlgChooser theChooser(this);
        // -----------------------------------------------
        mapIDName & the_map = theChooser.m_map;
        // -----------------------------------------------
        int32_t acct_count = opentxs::OTAPI_Wrap::It()->Party_GetAcctCount(contract, name);

        for (int32_t i = 0; i < acct_count; i++)
        {
            std::string acctName = opentxs::OTAPI_Wrap::It()->Party_GetAcctNameByIndex(contract, name, i);
            QString qstrAcctName = QString::fromStdString(acctName);
            // -----------------------------------------------
            std::string partyAcctID = opentxs::OTAPI_Wrap::It()->Party_GetAcctID(contract, name, acctName);
            QString qstrPartyAcctID = QString::fromStdString(partyAcctID);
            // -----------------------------------------------
            QString OT_id = qstrPartyAcctID;
            QString OT_name = qstrAcctName;
            // -----------------------------------------------
            if (!OT_id.isEmpty())
                the_map.insert(OT_id, OT_name);
        }
        // -----------------------------------------------
        theChooser.setWindowTitle(tr("Choose the activation account"));
        // -----------------------------------------------
        if (theChooser.exec() != QDialog::Accepted || theChooser.m_qstrCurrentID.isEmpty())
        {
            return opentxs::OTAPI_Wrap::It()->Msg_HarvestTransactionNumbers(contract, mynym, false, false, false, false, false);
        }
        // ------------------------------------------------
        std::string acctName = theChooser.m_qstrCurrentName.toStdString();
        if ("" == acctName) {
            qDebug() << "Error: account name empty on smart contract.\n";
            return opentxs::OTAPI_Wrap::It()->Msg_HarvestTransactionNumbers(contract, mynym, false, false, false, false, false);
        }

        myAcctID = theChooser.m_qstrCurrentID.toStdString();

        myAcctAgentName = opentxs::OTAPI_Wrap::It()->Party_GetAcctAgentName(contract, name, acctName);
        if ("" == myAcctAgentName) {
            qDebug() << "Error: account agent is not yet confirmed.\n";
            return opentxs::OTAPI_Wrap::It()->Msg_HarvestTransactionNumbers(contract, mynym, false, false, false, false, false);
        }
    }

    opentxs::OT_ME ot_me;
    std::string response = ot_me.activate_smart_contract(server, mynym, myAcctID, myAcctAgentName, contract);

    if (1 != ot_me.VerifyMessageSuccess(response))
    {
        qDebug() << "Error: cannot activate smart contract.\n";
        return opentxs::OTAPI_Wrap::It()->Msg_HarvestTransactionNumbers(contract, mynym, false, false, false, false, false);
    }

    // BELOW THIS POINT, the transaction has definitely processed.

    int32_t reply = ot_me.InterpretTransactionMsgReply(server, mynym, myAcctID, "activate_smart_contract", response);

    if (1 != reply) {
        return reply;
    }

    if (!ot_me.retrieve_account(server, mynym, myAcctID, true)) {
        qDebug() << "Error retrieving intermediary files for account.\n";
    }

    return 1;
}

int32_t Moneychanger::sendToNextParty(const std::string& server, const std::string& mynym,
                                      const std::string& hisnym,
                                      const std::string& contract)
{
    // But if all the parties are NOT confirmed, then we need to send it to
    // the next guy. In that case:
    // If HisNym is provided, and it's different than mynym, then use it.
    // He's the next receipient.
    // If HisNym is NOT provided, then display the list of NymIDs, and allow
    // the user to paste one. We can probably select him based on abbreviated
    // ID or Name as well (I think there's an API call for that...)
    std::string hisNymID = hisnym;

    opentxs::OT_ME ot_me;
    std::string response =
        ot_me.send_user_payment(server, mynym, hisNymID, contract);
    if (1 != ot_me.VerifyMessageSuccess(response)) {
        qDebug() << "\nFor whatever reason, our attempt to send the instrument on "
                 "to the next user has failed.\n";
        QMessageBox::information(this, tr("Moneychanger"), tr("Failed while calling send_user_payment."));
        return opentxs::OTAPI_Wrap::It()->Msg_HarvestTransactionNumbers(contract, mynym, false, false, false, false, false);
    }

    // Success. (Remove the payment instrument we just successfully sent from
    // our payments inbox.)

    // In the case of smart contracts, it might be sent on to a chain of 2 or
    // 3 users, before finally being activated by the last one in the chain.
    // All of the users in the chain (except the first one) will thus have a
    // copy of the smart contract in their payments inbox AND outbox.
    //
    // But once the smart contract has successfully been sent on to the next
    // user, and thus a copy of it is in my outbox already, then there is
    // definitely no reason for a copy of it to stay in my inbox as well.
    // Might as well remove that copy.
    //
    // We can't really expect to remove the payments inbox copy inside OT
    // itself, when we receive the server's SendNymInstrumentResponse reply
    // message,
    // without opening up the (encrypted) contents. (Although that would
    // actually be ideal, since it would cover all cases included dropped
    // messages...) But we CAN easily remove it RIGHT HERE.
    // Perhaps in the future I WILL move this code to the
    // SendNymInstrumentResponse
    // reply processing, but that will require it to be encrypted to my own
    // key as well as the recipient's, which we already do for sending cash,
    // but which we up until now have not done for the other instruments.
    // So perhaps we'll start doing that sometime in the future, and then move
    // this code.
    //
    // In the meantime, this is good enough.

    qDebug() << "Success sending the agreement on to the next party.\n";
    QMessageBox::information(this, tr("Moneychanger"), tr("Success sending the agreement on to the next party."));

    return 1;
}


bool Moneychanger::showPartyAccounts(const std::string& contract, const std::string& name, std::string & str_output)
{
    std::ostringstream os;

    int32_t accounts = opentxs::OTAPI_Wrap::It()->Party_GetAcctCount(contract, name);

    if (0 > accounts) {
        qDebug() << QString("Error: Party '%1' has bad value for number of asset accounts.").arg(QString::fromStdString(name));
        return false;
    }

    for (int32_t i = 0; i < accounts; i++)
    {
        std::string acctName =
            opentxs::OTAPI_Wrap::It()->Party_GetAcctNameByIndex(contract, name, i);
        if ("" == acctName) {
            qDebug() << QString("Error: Failed retrieving Asset Account Name from party '%1' at account index: %2")
                        .arg(QString::fromStdString(name)).arg(i);
            return false;
        }

        std::string acctInstrumentDefinitionID =
            opentxs::OTAPI_Wrap::It()->Party_GetAcctInstrumentDefinitionID(contract, name,
                                                            acctName);
        if ("" != acctInstrumentDefinitionID) {
            os << "-------------------\nAccount '" << acctName << "' (index "
                 << i << " on Party '" << name
                 << "') has instrument definition: "
                 << acctInstrumentDefinitionID << " ("
                 << opentxs::OTAPI_Wrap::It()->GetAssetType_Name(acctInstrumentDefinitionID)
                 << ")\n";
        }

        std::string acctID = opentxs::OTAPI_Wrap::It()->Party_GetAcctID(contract, name, acctName);
        if ("" != acctID) {
            os << "Account '" << acctName << "' (party '" << name
                 << "') is confirmed as Account ID: " << acctID << " ("
                 << opentxs::OTAPI_Wrap::It()->GetAccountWallet_Name(acctID) << ")\n";
        }

        std::string strAcctAgentName =
            opentxs::OTAPI_Wrap::It()->Party_GetAcctAgentName(contract, name, acctName);
        if ("" != strAcctAgentName) {
            os << "Account '" << acctName << "' (party '" << name
                 << "') is managed by agent: " << strAcctAgentName << "\n";
        }
    }

    str_output = os.str();

    return true;
}









/**
 * (Advantced ->) Settings Window
 **/

void Moneychanger::mc_settings_slot()
{
    // This is a glaring memory leak, but it's only a temporary placeholder before I redo how windows are handled.
    if (!settingswindow)
        settingswindow = new Settings(this);
    // ------------------------------------
    settingswindow->show();
}


/**
 * Namecoin update timer event.
 */
void Moneychanger::nmc_timer_event()
{
  nmc_names->timerUpdate ();
}


/**
  * (Bitcoin ->) Test Window
  **/
void Moneychanger::mc_bitcoin_slot()
{
    if(!bitcoinwindow)
        bitcoinwindow = new BtcGuiTest(this);
    bitcoinwindow->show();
}

/**
  * (Bitcoin ->) Connect to wallet
  **/
void Moneychanger::mc_bitcoin_connect_slot()
{
    if(!bitcoinConnectWindow)
        bitcoinConnectWindow = new BtcConnectDlg(this);
    bitcoinConnectWindow->show();
}

/**
  * (Bitcoin ->) Pools
  **/
void Moneychanger::mc_bitcoin_pools_slot()
{
    if(!bitcoinPoolWindow)
        bitcoinPoolWindow = new BtcPoolManager(this);
    bitcoinPoolWindow->show();
}

/**
  * (Bitcoin ->) Transactions
  **/
void Moneychanger::mc_bitcoin_transactions_slot()
{
    if(!bitcoinTxWindow)
        bitcoinTxWindow = new BtcTransactionManager(this);
    bitcoinTxWindow->show();
}

/**
  * (Bitcoin ->) Send
  **/
void Moneychanger::mc_bitcoin_send_slot()
{
    if(!bitcoinSendWindow)
        bitcoinSendWindow = new BtcSendDlg(this);
    bitcoinSendWindow->show();
}

/**
  * (Bitcoin ->) Receive
  **/
void Moneychanger::mc_bitcoin_receive_slot()
{
    if(!bitcoinReceiveWindow)
        bitcoinReceiveWindow = new BtcReceiveDlg(this);
    bitcoinReceiveWindow->show();
}

/*
 * Moneychanger RPC Callback Functions
 *
 */


void Moneychanger::mc_rpc_sendfunds_show_dialog(QString qstrAcct/*=QString("")*/, QString qstrRecipientNym/*=QString("")*/,
                                                QString qstrAsset/*=QString("")*/, QString qstrAmount/*=QString("")*/)
{
    // --------------------------------------------------
    MTSendDlg * send_window = new MTSendDlg(NULL);
    send_window->setAttribute(Qt::WA_DeleteOnClose);
    // --------------------------------------------------
    QString qstr_acct_id;
    QString qstr_recipient_id;


    if(!qstrAcct.isEmpty())
    {
        qstr_acct_id = qstrAcct;
    }
    else if(qstrAcct.isEmpty() && qstrAsset.isEmpty())
    {
        qstr_acct_id = this->get_default_account_id();
    }
    else if(qstrAcct.isEmpty() && !qstrAsset.isEmpty())
    {
        mapIDName theAccountMap;

        if (MTContactHandler::getInstance()->GetAccounts(theAccountMap, QString(""), QString(""), qstrAsset))
        {
            // This will be replaced with a popup dialog to select
            // from the accounts rather than using the first in the map.
            qstr_acct_id = theAccountMap.begin().key();
        }

        // If the asset is empty and the account is empty,
        // or no account exists for the asset given, use the default account id.
        if(qstrAsset.isEmpty())
        {
            qstr_acct_id = this->get_default_account_id();
        }
    }

    if (!qstr_acct_id.isEmpty())
        send_window->setInitialMyAcct(qstr_acct_id);

    if (!qstrAmount.isEmpty())
        send_window->setInitialAmount(qstrAmount);

    if(!qstrRecipientNym.isEmpty())
        send_window->setInitialHisNym(qstrRecipientNym);

    // ---------------------------------------
    send_window->dialog();
    // --------------------------------------------------
}

void Moneychanger::mc_rpc_requestfunds_show_dialog(QString qstrAcct/*=QString("")*/, QString qstrRecipientNym/*=QString("")*/,
                                                   QString qstrAsset/*=QString("")*/, QString qstrAmount/*=QString("")*/)
{
    // --------------------------------------------------
    MTRequestDlg * request_window = new MTRequestDlg(NULL);
    request_window->setAttribute(Qt::WA_DeleteOnClose);
    // --------------------------------------------------

    QString qstr_acct_id;

    if(!qstrAcct.isEmpty())
    {
        qstr_acct_id = qstrAcct;
    }
    else if(qstrAcct.isEmpty() && qstrAsset.isEmpty())
    {
        qstr_acct_id = this->get_default_account_id();
    }
    else if(qstrAcct.isEmpty() && !qstrAsset.isEmpty())
    {
        mapIDName theAccountMap;

        if (MTContactHandler::getInstance()->GetAccounts(theAccountMap, QString(""), QString(""), qstrAsset))
        {
            // This will be replaced with a popup dialog to select
            // from the accounts rather than using the first in the map.
            qstr_acct_id = theAccountMap.begin().key();
        }

        // If the asset is empty and the account is empty,
        // or no account exists for the asset given, use the default account id.
        if(qstrAsset.isEmpty())
        {
            qstr_acct_id = this->get_default_account_id();
        }
    }

    if (!qstr_acct_id.isEmpty())
        request_window->setInitialMyAcct(qstr_acct_id);

    if (!qstrAmount.isEmpty())
        request_window->setInitialAmount(qstrAmount);

    if(!qstrRecipientNym.isEmpty())
        request_window->setInitialHisNym(qstrRecipientNym);

    // ---------------------------------------
    request_window->dialog();
    // --------------------------------------------------
}

