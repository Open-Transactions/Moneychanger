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
#include <core/handlers/contacthandler.hpp>
#include <core/handlers/modeltradearchive.hpp>

#include <rpc/rpcserver.h>

#include <gui/widgets/compose.hpp>
#include <gui/widgets/home.hpp>
#include <gui/widgets/overridecursor.hpp>
#include <gui/widgets/editdetails.hpp>
#include <gui/widgets/requestdlg.hpp>
#include <gui/widgets/dlgchooser.hpp>
#include <gui/widgets/senddlg.hpp>
#include <gui/widgets/proposeplandlg.hpp>
#include <gui/widgets/createinsurancecompany.hpp>
#include <gui/widgets/wizardconfirmsmartcontract.hpp>
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
#include <gui/ui/dlgtradearchive.hpp>
#include <gui/ui/dlgencrypt.hpp>
#include <gui/ui/dlgdecrypt.hpp>
#include <gui/ui/dlginbailment.hpp>
#include <gui/ui/dlgpairnode.hpp>
#include <gui/ui/dlgpassphrasemanager.hpp>
#include <gui/ui/messages.hpp>
#include <gui/ui/payments.hpp>
#include <gui/ui/agreements.hpp>
#include <gui/ui/activity.hpp>
#include <gui/ui/getstringdialog.hpp>

#include <opentxs/api/client/Cash.hpp>
#include <opentxs/api/client/Pair.hpp>
#include <opentxs/api/client/ServerAction.hpp>
#include <opentxs/api/client/Sync.hpp>
#include <opentxs/api/client/Wallet.hpp>
#include <opentxs/api/network/ZMQ.hpp>
#include <opentxs/api/storage/Storage.hpp>
#include <opentxs/api/Activity.hpp>
#include <opentxs/api/Api.hpp>
#include <opentxs/api/UI.hpp>
#include <opentxs/api/Native.hpp>
#include <opentxs/client/OT_API.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>
#include <opentxs/client/OTWallet.hpp>
#include <opentxs/client/ServerAction.hpp>
#include <opentxs/client/Utility.hpp>
#include <opentxs/core/util/OTPaths.hpp>
#include <opentxs/core/crypto/OTPasswordData.hpp>
#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/script/OTSmartContract.hpp>
#include <opentxs/core/Cheque.hpp>
#include <opentxs/core/Identifier.hpp>
#include <opentxs/core/Nym.hpp>
#include <opentxs/ext/OTPayment.hpp>
#include <opentxs/network/zeromq/Context.hpp>
#include <opentxs/network/zeromq/ListenCallback.hpp>
#include <opentxs/network/zeromq/Message.hpp>
#include <opentxs/network/zeromq/SubscribeSocket.hpp>
#include <opentxs/OT.hpp>
#include <opentxs/Proto.hpp>

#include <opentxs/ui/ContactList.hpp>
#include <opentxs/ui/ContactListItem.hpp>


//    CmdShowContacts::CmdShowContacts()
//    {
//        command = "showcontacts";
//        args[0] = "--mynym <nym>";
//        category = catOtherUsers;
//        help = "Show the contact list for a nym in the wallet.";
//    }
//
//    std::int32_t CmdShowContacts::runWithOptions()
//    {
//        return run(getOption("mynym"));
//    }
//
//    std::int32_t CmdShowContacts::run(std::string mynym)
//    {
//        if (!checkNym("mynym", mynym)) {
//            return -1;
//        }
//
//        const Identifier nymID{mynym};
//        auto& list = OT::App().UI().ContactList(nymID);
//        otOut << "Contacts:\n";
//        dashLine();
//        auto& line = list.First();
//        auto last = line.Last();
//        otOut << " " << line.Section() << " " << line.DisplayName() << " ("
//        << line.ContactID() << ")\n";
//
//        while (false == last) {
//            auto& line = list.Next();
//            last = line.Last();
//            otOut << " " << line.Section() << "  " << line.DisplayName() << " ("
//            << line.ContactID() << ")\n";
//        }
//
//        otOut << std::endl;
//
//        return 1;
//    }
//




#include <QMenu>
#include <QApplication>
#include <QFile>
#include <QDateTime>
#include <QDebug>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QRegExp>
#include <QFlags>

#include <chrono>
#include <functional>
#include <sstream>
#include <utility>


#define DEMO_SERVER_NYM "otqHAZTKobdWCon2FL7T3thYfTAvoJr9Ck8"

bool Moneychanger::is_base64(QString string)
{
    QRegExp rx("[^a-zA-Z0-9+/=]");
    if (   rx.indexIn(string)  == -1
        && (string.length()%4) ==  0
        && string.length()>=4) {
        return true;
    }
    return false;
}

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
  ot_(opentxs::OT::App()),
  notify_bailment_callback_(opentxs::network::zeromq::ListenCallback::Factory(
          [this](const opentxs::network::zeromq::Message& message) -> void {
              this->process_notify_bailment(message);
          })),
  notify_bailment_(
      ot_.ZMQ().Context().SubscribeSocket(
          notify_bailment_callback_.get())),
  m_list(*(new MTNameLookupQT)),
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

    refresh_count_.store(0);
    notify_bailment_->Start(
        opentxs::network::zeromq::Socket::PendingBailmentEndpoint);

    /* Set up Namecoin name manager.  */
    nmc_names = new NMC_NameManager (*nmc);

    /* Set up the Namecoin update timer.  */
    nmc_update_timer = new QTimer (this);
    connect (nmc_update_timer, SIGNAL(timeout()),
             this, SLOT(nmc_timer_event()));
    nmc_update_timer->start (1000 * 60 * 10);
    nmc_timer_event ();

    ot_.Schedule(
        std::chrono::seconds(5),
        [&]()->void
            {
                const auto count =
                    ot_.API().Sync().RefreshCount();
                const auto existing = refresh_count_.load();

                if (existing < count) {
                    refresh_count_.store(count);
                    //processPeerMessages();
                    emit needToPopulateRecordlist();
                }
            }
      );

    ot_.Schedule(
        std::chrono::seconds(20),
        [this]()->void{ ot_.API().Sync().Refresh(); },
        (std::chrono::seconds(std::time(nullptr))));

    check_pairing();

    //SQLite database
    // This can be moved very easily into a different class
    // Which I will inevitably end up doing.

    /** Default Nym **/
    qDebug() << "Setting up Nym table";
    if (0 == DBHandler::getInstance()->querySize("SELECT `nym` FROM `default_nym` WHERE `default_id`='1' LIMIT 0,1"))
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
//        if (default_nym_id.isEmpty() && (ot_.API().Exec().GetNymCount() > 0))
//        {
//            default_nym_id = QString::fromStdString(ot_.API().Exec().GetNym_ID(0));
//        }
//        // -------------------------------------------------
//        //Ask OT what the display name of this nym is and store it for quick retrieval later on(mostly for "Default Nym" displaying purposes)
//        if (!default_nym_id.isEmpty())
//        {
//            default_nym_name =  QString::fromStdString(ot_.API().Exec().GetNym_Name(default_nym_id.toStdString()));
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
//        if (default_notary_id.isEmpty() && (ot_.API().Exec().GetServerCount() > 0))
//        {
//            default_notary_id = QString::fromStdString(ot_.API().Exec().GetServer_ID(0));
//        }
//        // -------------------------------------------------
//        //Ask OT what the display name of this server is and store it for a quick retrieval later on(mostly for "Default Server" displaying purposes)
//        if (!default_notary_id.isEmpty())
//        {
//            default_server_name = QString::fromStdString(ot_.API().Exec().GetServer_Name(default_notary_id.toStdString()));
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
//        if (default_asset_id.isEmpty() && (ot_.API().Exec().GetAssetTypeCount() > 0))
//        {
//            default_asset_id = QString::fromStdString(ot_.API().Exec().GetAssetType_ID(0));
//        }
//        // -------------------------------------------------
//        //Ask OT what the display name of this asset type is and store it for a quick retrieval later on(mostly for "Default Asset" displaying purposes)
//        if (!default_asset_id.isEmpty())
//        {
//            default_asset_name = QString::fromStdString(ot_.API().Exec().GetAssetType_Name(default_asset_id.toStdString()));
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
//        if (default_account_id.isEmpty() && (ot_.API().Exec().GetAccountCount() > 0))
//        {
//            default_account_id = QString::fromStdString(ot_.API().Exec().GetAccountWallet_ID(0));
//        }
//        // -------------------------------------------------
//        //Ask OT what the display name of this account is and store it for a quick retrieval later on(mostly for "Default Account" displaying purposes)
//        if (!default_account_id.isEmpty())
//        {
//            default_account_name = QString::fromStdString(ot_.API().Exec().GetAccountWallet_Name(default_account_id.toStdString()));
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
    mc_systrayIcon_proposeplan    = QIcon(":/icons/icons/timer.png");
//  mc_systrayIcon_contacts       = QIcon(":/icons/addressbook");
    mc_systrayIcon_contacts       = QIcon(":/icons/icons/rolodex_card2");
    mc_systrayIcon_composemessage = QIcon(":/icons/icons/pencil.png");
//  mc_systrayIcon_composemessage = QIcon(":/icons/icons/compose.png");

    mc_systrayIcon_markets = QIcon(":/icons/markets");
    mc_systrayIcon_trade_archive = QIcon(":/icons/overview");

    mc_systrayIcon_active_agreements = QIcon(":/icons/agreements");
    mc_systrayIcon_pending = QIcon(":/icons/icons/pending.png");

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
    mc_systrayIcon_advanced_smartcontracts = QIcon(":/icons/icons/smart_contract_64.png");

    mc_systrayIcon_advanced_corporations = QIcon(":/icons/icons/buildings.png");
    mc_systrayIcon_advanced_transport = QIcon(":/icons/icons/p2p.png");
    mc_systrayIcon_advanced_log = QIcon(":/icons/icons/p2p.png");
    mc_systrayIcon_settings = QIcon(":/icons/settings");
    // ----------------------------------------------
    QString qstrExpertMode = MTContactHandler::getInstance()->
            GetValueByID("expertmode", "parameter1", "settings", "setting");

    bExpertMode_ = (0 == qstrExpertMode.compare("on"));
    // -------------------------------------------------
    QString qstrHideNav = MTContactHandler::getInstance()->
            GetValueByID("hidenav", "parameter1", "settings", "setting");

    bHideNav_ = (0 == qstrHideNav.compare("on"));
    // -------------------------------------------------
    setupRecordList();

    mc_overall_init = true;
}

void Moneychanger::process_notify_bailment(
    const opentxs::network::zeromq::Message& message) const
{
    auto request =
        opentxs::proto::TextToProto<opentxs::proto::PeerRequest>(message);

    if (false == opentxs::proto::Validate(request, false)) {
        std::cout << __FUNCTION__ << ":Invalid peer request";

        return;
    }

    if (opentxs::proto::PEERREQUEST_PENDINGBAILMENT != request.type()) {
        std::cout << __FUNCTION__ << ": Not a notify bailment request";

        return;
    }

    std::cout << __FUNCTION__ << ": Received notice of pending bailment: "
              << request.pendingbailment().txid().c_str();
}

void Moneychanger::check_pairing() const
{
    if (default_nym_id.isEmpty()) {

        return;
    }

    const opentxs::Identifier nymID(default_nym_id.toStdString());
    const opentxs::Identifier demoNymID(std::string(DEMO_SERVER_NYM));

    if (nymID.empty()) {

        return;
    }

    const auto& pair = ot_.API().Pair();
    auto paired = pair.IssuerList(nymID, false);

    if (0 == paired.count(demoNymID)) {
        pair.AddIssuer(nymID, demoNymID, "");
    }
}

// RETRIEVE NYM INTERMEDIARY FILES
// Returns:
//  True if I have enough numbers, or if there was success getting more
// transaction numbers.
//  False if I didn't have enough numbers, tried to get more, and failed
// somehow.
//
bool Moneychanger::retrieve_nym(
    const std::string& strNotaryID,
    const std::string& strMyNymID) const
{
    auto context = ot_.Wallet().mutable_ServerContext(
        opentxs::Identifier(strMyNymID), opentxs::Identifier(strNotaryID));
    opentxs::Utility MsgUtil(context.It(), ot_.API().OTAPI());

    if (0 >= context.It().UpdateRequestNumber()) {
        return false;
    } 

    bool msgWasSent = false;
    std::int32_t nGetAndProcessNymbox = MsgUtil.getAndProcessNymbox_4(
        strNotaryID, strMyNymID, msgWasSent, true);

    if (0 > nGetAndProcessNymbox) {
    	return false;
    }
    return true;
}

void Moneychanger::onNeedToCheckNym(QString myNymId, QString hisNymId, QString notaryId)
{
    if (hisNymId.isEmpty())
    {
        qDebug() << __FUNCTION__ << ": ERROR: no hisNymId passed in! (Returning.)";
        return;
    }
    // ---------------------------
    if (myNymId.isEmpty())
        myNymId = getDefaultNymID();

    if (myNymId.isEmpty())
    {
        //Count nyms
        const int32_t nym_count = ot_.API().Exec().GetNymCount();

        if (nym_count > 0)
            myNymId = QString::fromStdString(ot_.API().Exec().GetNym_ID(0));
    }

    if (myNymId.isEmpty())
    {
        qDebug() << __FUNCTION__ << ": no myNymId passed in, and unable to figure out one on my own! (Returning.)";
        return;
    }
    // ---------------------------
    // If a notary was passed in, let's make sure we have its contract!
    //
    if (!notaryId.isEmpty())
    {
        const std::string str_notary_contract = ot_.API().Exec().GetServer_Contract(notaryId.toStdString());

        if (str_notary_contract.empty())
        {
            notaryId = QString("");
            qDebug() << __FUNCTION__ << ": We don't have the server contract for the specified notary ID. "
                        "(I guess we'll try the default, it may yet work.) TODO: Download it from the DHT.";
        }
    }
    // ---------------------------
    mapIDName mapServers;

    bool bGotServersForNym = false;

    // Hmm, no notary ID was passed. Perhaps one wasn't available.
    // In that case we'll see if there are any known notaries for hisNymId.
    // If so, we'll use them all.
    // Otherwise, we'll try to use the default notary instead.
    // Otherwise we'll give up  :P
    //
    if (notaryId.isEmpty())
        bGotServersForNym = MTContactHandler::getInstance()->GetServers(mapServers, hisNymId);
    else
        mapServers.insert(notaryId, "Server Name Unused Here");

    if (mapServers.size() < 1)
    {
        notaryId = getDefaultNotaryID();

        if (!notaryId.isEmpty())
            mapServers.insert(notaryId, "Server Name Unused Here");
    }

    // Oh well. We tried.
    if (mapServers.size() < 1)
    {
        qDebug() << __FUNCTION__ << ": no notaryId passed in, and unable to figure out one on my own! (Returning.)";
        return;
    }
    // --------------------------------
    const std::string my_nym_id  = myNymId.toStdString();
    const std::string his_nym_id = hisNymId.toStdString();

    for (mapIDName::iterator it_servers = mapServers.begin(); it_servers != mapServers.end(); ++it_servers)
    {
        QString qstrNotaryID   = it_servers.key();
//      QString qstrNotaryName = it_servers.value();

        if (qstrNotaryID.isEmpty()) // Weird, should never happen.
        {
            qDebug() << __FUNCTION__ << ": Unexpectedly empty NotaryID, should not happen. (Returning.)";
            return;
        }
        const std::string notary_id = qstrNotaryID.toStdString();
        // ---------------------------------------------
        // Need to verify that I'M registered at this server, before I try to
        // download some guy's credentials from a "server he's known to use."
        // I may not even be registered there, in which case the check_nym call would fail.
        //
        // And how do I know if I even have the server contract at all?
        //
        const std::string str_server_contract = ot_.API().Exec().GetServer_Contract(notary_id);

        if (str_server_contract.empty())
        {
            qDebug() << __FUNCTION__ << ": We don't have the server contract for this notary ID. (Skipping.) TODO: Download it from the DHT.";
            continue;
        }
        // ---------------------------------------------
        const bool isReg = ot_.API().Exec().IsNym_RegisteredAtServer(my_nym_id, notary_id);

        if (!isReg)
        {
            std::string response;
            {
                MTSpinner theSpinner;
                response = opentxs::String(ot_.API().Sync().RegisterNym(opentxs::Identifier(my_nym_id), opentxs::Identifier(notary_id), true)).Get();
                if (response.empty() && !ot_.API().Exec().CheckConnection(notary_id))
                {
                    QString qstrErrorMsg;
                    qstrErrorMsg = QString("%1: %2. %3.").
                            arg(tr("Failed trying to contact notary")).
                            arg(qstrNotaryID).arg(tr("Perhaps it is down, or there might be a network problem"));
                    emit appendToLog(qstrErrorMsg);
                    continue;
                }
            }

            if (!opentxs::VerifyMessageSuccess(response)) {
                Moneychanger::It()->HasUsageCredits(notary_id, my_nym_id);
                continue;
            }
            else
                MTContactHandler::getInstance()->NotifyOfNymServerPair(myNymId, qstrNotaryID);
        }
        // ------------------------------
        {
            std::string response;
            {
                MTSpinner theSpinner;
                const opentxs::Identifier notaryID{notary_id}, myNymID{my_nym_id}, hisNymID{his_nym_id};
                auto action = ot_.API().ServerAction().DownloadNym(myNymID, notaryID, hisNymID);
                response = action->Run();
            }
            if (response.empty() && !ot_.API().Exec().CheckConnection(notary_id))
            {
                QString qstrErrorMsg;
                qstrErrorMsg = QString("%1: %2. %3.").
                        arg(tr("Failed trying to contact notary")).
                        arg(qstrNotaryID).arg(tr("Perhaps it is down, or there might be a network problem"));
                emit appendToLog(qstrErrorMsg);
                continue;
            }

            int32_t  nReturnVal = opentxs::VerifyMessageSuccess(response);
            if (1 == nReturnVal)
            {
                emit nymWasJustChecked(hisNymId);
                break;
            }
        }
    } // for (servers)
}


//QString getBitmessageAddressFromClaims(const QString & claimant_nym_id);
//QString getDisplayNameFromClaims(const QString & claimant_nym_id);

// Someone did this: OT_ME::check_nym(id);  emit nymWasJustChecked(id);
//
void Moneychanger::onCheckNym(QString nymId)
{
    // ----------------------------------------------
    auto strNymId = opentxs::String(nymId.toStdString());
    opentxs::Identifier id_nym(strNymId);
    // ----------------------------------------------
    // Get the Nym. Make sure we have the latest copy, since his credentials were apparently
    // just downloaded and overwritten.
    //
    opentxs::OTPasswordData thePWData("Sometimes need to load private part of nym in order to use its public key. (Fix that!)");
    const opentxs::Nym * pCurrentNym =
        ot_.API().OTAPI().
            reloadAndGetNym(id_nym, false, __FUNCTION__,  &thePWData);

    if (nullptr == pCurrentNym)
    {
        qDebug() << "onCheckNym: GetOrLoadNym failed. (Which should NOT happen since we supposedly JUST downloaded that Nym's credentials...)";
        return;
    }
    // ------------------------------------------------
    // Clear the claims and verifications we already have in the database. (If any.)
    //
    MTContactHandler::getInstance()->clearClaimsForNym(nymId);
    // ----------------------------------------------
    // Import the claims.

    const std::string str_checked_nym_id(strNymId.Get());

    const auto data =
        ot_.API().Exec().GetContactData(nymId.toStdString());
    auto claims =
        opentxs::proto::DataToProto<opentxs::proto::ContactData>
            (opentxs::Data::Factory(data.c_str(), static_cast<uint32_t>(data.length())));

    for (const auto& section: claims.section()) {
        for (const auto& claim: section.item()) {
            // ---------------------------------------
            const uint32_t claim_section = section.name();
            const uint32_t claim_type = claim.type();
            const QString claim_value = QString::fromStdString(claim.value());

            // Add the claim to the database if not there already.
            const bool upserted =
                MTContactHandler::getInstance()->upsertClaim(
                    *pCurrentNym,
                    claim_section,
                    claim);

            if (!upserted) {
                qDebug() << "onCheckNym: the call to upsertClaim just failed. "
                         << "(Returning.)";
                return;
            }

            bool claim_att_active  = false;
            bool claim_att_primary = false;

            for (const auto& attribute : claim.attribute()) {
                if (opentxs::proto::CITEMATTR_ACTIVE  == attribute) {
                    claim_att_active  = true;
                }
                if (opentxs::proto::CITEMATTR_PRIMARY == attribute) {
                    claim_att_primary = true;
                }
            }

            if (claim_att_active && claim_att_primary) {
                if (claim_section == opentxs::proto::CONTACTSECTION_IDENTIFIER) {
                    MTContactHandler::getInstance()->NotifyOfNymNamePair(
                        nymId,
                        claim_value);
                }
                if ((claim_section == opentxs::proto::CONTACTSECTION_COMMUNICATION) &&
                    (claim_type == opentxs::proto::CITEMTYPE_BITMESSAGE)) {
                        // NOTE: May not need to do anything here. We already
                        // imported the claims, and we can already search the
                        // claims for Bitmessage address and NymID, which we
                        // are already doing.
                }
            }
        }
    }
    // -------------------------------------------------------
    // Import the verifications.
    //
    const auto ver_data =
        ot_.API().Exec().GetVerificationSet(nymId.toStdString());

    auto the_set =
        opentxs::proto::DataToProto<opentxs::proto::VerificationSet>
            (opentxs::Data::Factory(ver_data.c_str(), ver_data.length()));

    // Internal verifications:
    // Here I'm looping through pCurrentNym's verifications of other people's claims.
    for (auto& claimant: the_set.internal().identity()) {
        // Here we're looping through those other people. (Claimants.)
        const std::string& str_claimant_id = claimant.nym();

        for (auto& verification : claimant.verification()) {
            const bool success =
                MTContactHandler::getInstance()->upsertClaimVerification(
                    str_claimant_id,
                    str_checked_nym_id,
                    verification,
                    true);  //bIsInternal=true
            if (!success) {
                qDebug() << "onCheckNym: the call to "
                         << "upsertInternalClaimVerification just failed. "
                         << "(Returning.)";
                return;
            }
        }
    }

    // External verifications:
    // Here I'm looping through other people's verifications of pCurrentNym's claims.
    for (auto& verifier: the_set.external().identity()) {
        const std::string& str_verifier_id  = verifier.nym();

        for (auto& verification : verifier.verification()) {
            const bool success =
                MTContactHandler::getInstance()->upsertClaimVerification(
                    str_checked_nym_id,
                    str_verifier_id,
                    verification,
                    false); //bIsInternal=true by default.
            if (!success) {
                qDebug() << "onCheckNym: the call to "
                         << "upsertExternalClaimVerification just failed. "
                         << "(Returning.)";
                return;
            }
        }
    }
    // -------------------------------------------------------
    // Import the repudiations.



    // -------------------------------------------------------
    // emit signal that claims / verifications were updated.
    //
    emit claimsUpdatedForNym(nymId);
}


static void blah()
{
//resume
//todo

// OT_API.hpp
//EXPORT VerificationSet GetVerificationSet(const Nym& fromNym) const;
// EXPORT bool SetVerifications(Nym& onNym,
//                            const proto::VerificationSet&) const;

// Nym.hpp
//    std::shared_ptr<proto::VerificationSet> VerificationSet() const;
//    bool SetVerificationSet(const proto::VerificationSet& data);
//
//    proto::Verification Sign(
//        const std::string& claim,
//        const bool polarity,
//        const int64_t start = 0,
//        const int64_t end = 0,
//        const OTPasswordData* pPWData = nullptr) const;
//    bool Verify(const proto::Verification& item) const;

    // VerificationSet has 2 groups, internal and external.
    // Internal is for your signatures on other people's claims.
    // External is for other people's signatures on your claims.
    // When you find that in the external, you copy it to your own credential.
    // So external is for re-publishing other people's verifications of your claims.

    // If we've repudiated any claims, you can add their IDs to the repudiated field in the verification set.
}

Moneychanger::~Moneychanger()
{
    delete nmc_update_timer;
    delete nmc_names;
    delete nmc;

    nmc_update_timer = nullptr;
    nmc_names = nullptr;
    nmc = nullptr;
}

opentxs::OTRecordList & Moneychanger::GetRecordlist()
{
    return m_list;
}

void Moneychanger::setupRecordList()
{
    int nServerCount  = ot_.API().Exec().GetServerCount();
    int nAssetCount   = ot_.API().Exec().GetAssetTypeCount();
    int nNymCount     = ot_.API().Exec().GetNymCount();
    int nAccountCount = ot_.API().Exec().GetAccountCount();
    // ----------------------------------------------------
    GetRecordlist().ClearServers();
    GetRecordlist().ClearAssets();
    GetRecordlist().ClearNyms();
    GetRecordlist().ClearAccounts();
    // ----------------------------------------------------
    for (int ii = 0; ii < nServerCount; ++ii)
    {
        std::string NotaryID = ot_.API().Exec().GetServer_ID(ii);
        GetRecordlist().AddNotaryID(NotaryID);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nAssetCount; ++ii)
    {
        std::string InstrumentDefinitionID = ot_.API().Exec().GetAssetType_ID(ii);
        GetRecordlist().AddInstrumentDefinitionID(InstrumentDefinitionID);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nNymCount; ++ii)
    {
        std::string nymId = ot_.API().Exec().GetNym_ID(ii);
        GetRecordlist().AddNymID(nymId);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nAccountCount; ++ii)
    {
        std::string accountID = ot_.API().Exec().GetAccountWallet_ID(ii);
        GetRecordlist().AddAccountID(accountID);
    }
    // ----------------------------------------------------
    GetRecordlist().AcceptChequesAutomatically  (true);
    GetRecordlist().AcceptReceiptsAutomatically (true);
    GetRecordlist().AcceptTransfersAutomatically(false);
}

// Calls OTRecordList::Populate(), and then additionally adds records from Bitmessage, etc.
//
void Moneychanger::populateRecords(bool bCurrentlyModifying/*=false*/)
{
    GetRecordlist().Populate(); // Refreshes the OT data from local storage.   < << <<==============***
    // ---------------------------------------------------------------------
    QList<QString> listCheckOnlyOnce; // So we don't call checkMail more than once for the same connect string.
    // ---------------------------------------------------------------------
    // Let's see if, additionally, there are any Bitmessage records (etc)
    // for the Nyms that we care about. (If we didn't add a Nym ID to GetRecordlist()'s
    // list of Nyms, then we don't care about any Bitmessages for that Nym.)
    //
    bool bNeedsReSorting = false;

    const opentxs::list_of_strings & the_nyms = GetRecordlist().GetNyms();

    for (opentxs::list_of_strings::const_iterator it = the_nyms.begin(); it != the_nyms.end(); ++it)
    {
        const std::string str_nym_id = *it;
        // -----------------------------
        mapIDName mapMethods;
        QString   filterByNym = QString::fromStdString(str_nym_id);

        bool bGotMethods = !filterByNym.isEmpty() ? MTContactHandler::getInstance()->GetMsgMethodsByNym(mapMethods, filterByNym, false, QString("")) : false;

        if (bGotMethods)
        {
            // Loop through mapMethods and for each methodID, call GetAddressesByNym.
            // Then for each address, grab the inbox and outbox from MTComms, and add
            // the messages to GetRecordlist().
            //
            for (mapIDName::iterator ii = mapMethods.begin(); ii != mapMethods.end(); ++ii)
            {
                QString qstrID        = ii.key();
                int nFilterByMethodID = 0;

                QStringList stringlist = qstrID.split("|");

                if (stringlist.size() >= 2) // Should always be 2...
                {
//                  QString qstrType     = stringlist.at(0);
                    QString qstrMethodID = stringlist.at(1);
                    nFilterByMethodID    = qstrMethodID.isEmpty() ? 0 : qstrMethodID.toInt();
                    // --------------------------------------
                    if (nFilterByMethodID > 0)
                    {
                        QString   qstrMethodType  = MTContactHandler::getInstance()->GetMethodType       (nFilterByMethodID);
                        QString   qstrTypeDisplay = MTContactHandler::getInstance()->GetMethodTypeDisplay(nFilterByMethodID);
                        QString   qstrConnectStr  = MTContactHandler::getInstance()->GetMethodConnectStr (nFilterByMethodID);

                        if (!qstrConnectStr.isEmpty())
                        {
                            NetworkModule * pModule = MTComms::find(qstrConnectStr.toStdString());

                            if ((NULL == pModule) && MTComms::add(qstrMethodType.toStdString(), qstrConnectStr.toStdString()))
                                pModule = MTComms::find(qstrConnectStr.toStdString());

                            if (NULL == pModule)
                                // todo probably need a messagebox here.
                                qDebug() << QString("PopulateRecords: Unable to add a %1 interface with connection string: %2").arg(qstrMethodType).arg(qstrConnectStr);

                            if ((NULL != pModule) && pModule->accessible())
                            {
                                if ((-1) == listCheckOnlyOnce.indexOf(qstrConnectStr)) // Not on the list yet.
                                {
                                    pModule->checkMail();
                                    listCheckOnlyOnce.insert(0, qstrConnectStr);
                                }
                                // ------------------------------
                                mapIDName mapAddresses;

                                if (MTContactHandler::getInstance()->GetAddressesByNym(mapAddresses, filterByNym, nFilterByMethodID))
                                {
                                    for (mapIDName::iterator jj = mapAddresses.begin(); jj != mapAddresses.end(); ++jj)
                                    {
                                        QString qstrAddress = jj.key();

                                        if (!qstrAddress.isEmpty())
                                        {
                                            // --------------------------------------------------------------------------------------------
                                            // INBOX
                                            //
                                            std::vector< _SharedPtr<NetworkMail> > theInbox = pModule->getInbox(qstrAddress.toStdString());

                                            for (std::vector< _SharedPtr<NetworkMail> >::size_type nIndex = 0; nIndex < theInbox.size(); ++nIndex)
                                            {
                                                _SharedPtr<NetworkMail> & theMsg = theInbox[nIndex];

                                                std::string strSubject  = theMsg->getSubject();
                                                std::string strContents = theMsg->getMessage();
                                                // ----------------------------------------------------
                                                QString qstrFinal;

                                                if (!strSubject.empty())
                                                    qstrFinal = QString("%1: %2\n%3").
                                                            arg(tr("Subject")).
                                                            arg(QString::fromStdString(strSubject)).
                                                            arg(QString::fromStdString(strContents));
                                                else
                                                    qstrFinal = QString::fromStdString(strContents);
                                                // ----------------------------------------------------
                                                bNeedsReSorting = true;

                                                if (!theMsg->getMessageID().empty())
                                                    GetRecordlist().AddSpecialMsg(theMsg->getMessageID(),
                                                                         false, //bIsOutgoing=false
                                                                         static_cast<int32_t>(nFilterByMethodID),
                                                                         qstrFinal.toStdString(),
                                                                         theMsg->getTo(),
                                                                         theMsg->getFrom(),
                                                                         qstrMethodType.toStdString(),
                                                                         qstrTypeDisplay.toStdString(),
                                                                         str_nym_id,
                                                                         static_cast<time64_t>(theMsg->getReceivedTime()));
                                            } // for (inbox)
                                            // --------------------------------------------------------------------------------------------
                                            // OUTBOX
                                            //
                                            std::vector< _SharedPtr<NetworkMail> > theOutbox = pModule->getOutbox(qstrAddress.toStdString());

                                            for (std::vector< _SharedPtr<NetworkMail> >::size_type nIndex = 0; nIndex < theOutbox.size(); ++nIndex)
                                            {
                                                _SharedPtr<NetworkMail> & theMsg = theOutbox[nIndex];

                                                std::string strSubject  = theMsg->getSubject();
                                                std::string strContents = theMsg->getMessage();
                                                // ----------------------------------------------------
                                                QString qstrFinal;

                                                if (!strSubject.empty())
                                                    qstrFinal = QString("%1: %2\n%3").
                                                            arg(tr("Subject")).
                                                            arg(QString::fromStdString(strSubject)).
                                                            arg(QString::fromStdString(strContents));
                                                else
                                                    qstrFinal = QString::fromStdString(strContents);
                                                // ----------------------------------------------------
                                                bNeedsReSorting = true;

//                                                qDebug() << QString("Adding OUTGOING theMsg->getMessageID(): %1 \n filterByNym: %2 \n qstrAddress: %3 \n nIndex: %4")
//                                                            .arg(QString::fromStdString(theMsg->getMessageID()))
//                                                            .arg(filterByNym)
//                                                            .arg(qstrAddress)
//                                                            .arg(nIndex)
//                                                            ;


                                                if (!theMsg->getMessageID().empty())
                                                    GetRecordlist().AddSpecialMsg(theMsg->getMessageID(),
                                                                         true, //bIsOutgoing=true
                                                                         static_cast<int32_t>(nFilterByMethodID),
                                                                         qstrFinal.toStdString(),
                                                                         theMsg->getFrom(),
                                                                         theMsg->getTo(),
                                                                         qstrMethodType.toStdString(),
                                                                         qstrTypeDisplay.toStdString(),
                                                                         str_nym_id,
                                                                         static_cast<time64_t>(theMsg->getSentTime()));
                                            } // for (outbox)
                                        } // if (!qstrAddress.isEmpty())
                                    } // for (addresses)
                                } // if GetAddressesByNym
                            } // if ((NULL != pModule) && pModule->accessible())
                        } // if (!qstrConnectStr.isEmpty())
                    } // if nFilterByMethodID > 0
                } // if (stringlist.size() >= 2)
            } // for (methods)
        } // if bGotMethods
    } // for (nyms)
    // -----------------------------------------------------
    if (bNeedsReSorting)
        GetRecordlist().SortRecords();
    // -----------------------------------------------------
    // This takes things like market receipts out of the record list
    // and moves to their own database table.
    // Same thing for mail messages, etc.
    //
    if (!bCurrentlyModifying)
        modifyRecords();
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
    if (!ot_.API().Exec().CheckConnection(notary_id))
    {
        QString qstrErrorMsg;
        qstrErrorMsg = tr("HasUsageCredits: Failed trying to contact the notary. Perhaps it is down, or there might be a network problem.");
        emit appendToLog(qstrErrorMsg);
        return -2;
    }
    // --------------------------------------------------------
    std::string strMessage;
    {
        MTSpinner theSpinner;

        const opentxs::Identifier notaryID{notary_id}, nymID{NYM_ID};
        auto action = ot_.API().ServerAction().AdjustUsageCredits(nymID, notaryID, nymID, 0);
        strMessage = action->Run();
    }
    if (strMessage.empty())
    {
//        QString qstrErrorMsg;
//        qstrErrorMsg = tr("Moneychanger::HasUsageCredits: Error 'strMessage' is Empty!");
//        emit appendToLog(qstrErrorMsg);
        return -2;
    }

    // --------------------------------------------------------
    const int64_t lReturnValue = ot_.API().Exec().Message_GetUsageCredits(strMessage);
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
    connect(this, SIGNAL(appendToLog(QString)),            this, SLOT(mc_showlog_slot(QString)));
    connect(this, SIGNAL(expertModeUpdated(bool)),         this, SLOT(onExpertModeUpdated(bool)));
    connect(this, SIGNAL(needToCheckNym(QString,QString,QString)), this, SLOT(onNeedToCheckNym(QString,QString,QString)));
    connect(this, SIGNAL(nymWasJustChecked(QString)),      this, SLOT(onCheckNym(QString)));
    connect(this, SIGNAL(serversChanged()),                this, SLOT(onServersChanged()));
    connect(this, SIGNAL(assetsChanged()),                 this, SLOT(onAssetsChanged()));
    connect(this, SIGNAL(needToPopulateRecordlist()),      this, SLOT(onNeedToPopulateRecordlist()));
    connect(this, SIGNAL(needToUpdateMenu()),              this, SLOT(onNeedToUpdateMenu()));
    connect(this, SIGNAL(updateMenuAndPopulateRecords()),  this, SLOT(onNeedToUpdateMenu()));
    connect(this, SIGNAL(updateMenuAndPopulateRecords()),  this, SLOT(onNeedToPopulateRecordlist()));
    connect(this, SIGNAL(newServerAdded(QString)),         this, SLOT(onNewServerAdded(QString)));
    connect(this, SIGNAL(newAssetAdded(QString)),          this, SLOT(onNewAssetAdded(QString)));
    // ----------------------------------------------------------------------------
    connect(this, SIGNAL(serversChanged()),                this, SIGNAL(updateMenuAndPopulateRecords()));
    connect(this, SIGNAL(assetsChanged()),                 this, SIGNAL(updateMenuAndPopulateRecords()));
    connect(this, SIGNAL(newServerAdded(QString)),         this, SIGNAL(updateMenuAndPopulateRecords()));
    connect(this, SIGNAL(newAssetAdded(QString)),          this, SIGNAL(updateMenuAndPopulateRecords()));
    // ----------------------------------------------------------------------------
    connect(this, SIGNAL(newNymAdded(QString)),            this, SLOT(onNewNymAdded(QString)));
    // ----------------------------------------------------------------------------
    //Show systray
    mc_systrayIcon->show();
    // ----------------------------------------------------------------------------
    SetupMainMenu();
    // ----------------------------------------------------------------------------
    QString qstrMenuFileExists = QString(opentxs::OTPaths::AppDataFolder().Get()) + QString("/knotworkpigeons");

    if (QFile::exists(qstrMenuFileExists))
        mc_main_menu_dialog();
    // ----------------------------------------------------------------------------
    if (!hideNav())
        mc_activity_dialog();
    // ----------------------------------------------------------------------------
    if (!hasNyms()) {
        std::string strSource(""), strMyName("Me");
        // --------------------------------------
        MTGetStringDialog nameDlg(this, tr("Creating your Nym. "
                                           "Enter your display name (that others will see)"));

        bool bNameEmpty{true};
        if (QDialog::Accepted == nameDlg.exec())
        {
            const QString qstrNewMyName = nameDlg.GetOutputString();
            if (!qstrNewMyName.isEmpty()) {
                bNameEmpty = false;
                strMyName = qstrNewMyName.toStdString();
            }
        }
        // --------------------------------------------------
        if (bNameEmpty) {
            QString name;
            name = qgetenv("USER"); // get the user name in Linux
            if (!name.isEmpty()) {
                bNameEmpty = false;
                strMyName = name.toStdString();
            }
            else {
                name = qgetenv("USERNAME"); // get the name in Windows
            }
            if (!name.isEmpty()) {
                bNameEmpty = false;
                strMyName = name.toStdString();
            }
        }
        // --------------------------------------------------
        std::string str_id = ot_.API().Exec().CreateNymHD(
            opentxs::proto::CITEMTYPE_INDIVIDUAL, strMyName, strSource, 0);

        if (!str_id.empty())
        {
            setDefaultNym(QString::fromStdString(str_id), QString::fromStdString(strMyName));
            //DBHandler::getInstance()->AddressBookUpdateDefaultNym(QString::fromStdString(newNymId));
            // -----------------------------------------------------------------------------------

            SetupMainMenu(); // Since we just created a new default nym since setting up the menu.

            const QString qstrNymId = QString::fromStdString(str_id);
//          mc_nymmanager_dialog(qstrNymId);
            emit newNymAdded(qstrNymId);

            check_pairing();
        }
    }
    // ----------------------------------------------------------------------------
//    bool bEnoughForNow = false;
//    if (ot_.API().Exec().GetAssetTypeCount() <= 0) {
//        mc_assetmanager_dialog();
//        bEnoughForNow = true;
//    }
//    else if (ot_.API().Exec().GetServerCount() <= 0) {
//        mc_servermanager_dialog();
//        bEnoughForNow = true;
//    }
//    // ----------------------------------------------------------------------------
//    if (!bEnoughForNow) {
//        if (ot_.API().Exec().GetAccountCount() <= 0)
//            mc_accountmanager_dialog();
//        else
//            mc_payments_dialog();
//    }
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
//    mc_systrayMenu_headertext = new QAction(tr(MONEYCHANGER_APP_NAME), mc_systrayMenu);
//    mc_systrayMenu_headertext->setDisabled(1);
//    mc_systrayMenu->addAction(mc_systrayMenu_headertext);
    // --------------------------------------------------------------
    if (hasNyms() &&
            ot_.API().Exec().GetServerCount() > 0 &&
            ot_.API().Exec().GetAssetTypeCount() > 0)
        SetupAccountMenu(mc_systrayMenu);
    SetupNymMenu(mc_systrayMenu);
    // --------------------------------------------------------------
    mc_systrayMenu->addSeparator();
    // --------------------------------------------------------------
    if (hasAccounts())
        SetupPaymentsMenu(mc_systrayMenu);
    if (expertMode() && hasNyms())
        SetupMessagingMenu(mc_systrayMenu);
    // --------------------------------------------------------------
    //mc_systrayMenu->addSeparator();
    // --------------------------------------------------------------
    //Passphrase Manager
    if (expertMode())
    {
        mc_systrayMenu_passphrase_manager = new QAction(mc_systrayIcon_crypto, tr("Secrets"), mc_systrayMenu);
        mc_systrayMenu->addAction(mc_systrayMenu_passphrase_manager);
        connect(mc_systrayMenu_passphrase_manager, SIGNAL(triggered()), this, SLOT(mc_passphrase_manager_slot()));
        // --------------------------------------------------------------
        mc_systrayMenu->addSeparator();
    }
    // --------------------------------------------------------------
    //Blank
    //            mc_systrayMenu_aboveBlank = new QAction(" ", 0);
    //            mc_systrayMenu_aboveBlank->setDisabled(1);
    //            mc_systrayMenu->addAction(mc_systrayMenu_aboveBlank);
    // --------------------------------------------------------------
    if (expertMode() && hasAccounts())
    {
        SetupExchangeMenu(mc_systrayMenu);
        mc_systrayMenu->addSeparator();
    }
    // --------------------------------------------------------------
//  if (expertMode())
    {
        SetupContractsMenu(mc_systrayMenu);
        mc_systrayMenu->addSeparator();
    }
    // --------------------------------------------------------------
    if (expertMode() && hasNyms())
    {
        SetupToolsMenu(mc_systrayMenu);
        mc_systrayMenu->addSeparator();
    }
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
    // Settings
    mc_systrayMenu_settings = new QAction(mc_systrayIcon_settings, tr("Settings"), mc_systrayMenu);
    mc_systrayMenu_settings->setMenuRole(QAction::NoRole);
    mc_systrayMenu->addAction(mc_systrayMenu_settings);
    connect(mc_systrayMenu_settings, SIGNAL(triggered()), this, SLOT(mc_settings_slot()));
    // --------------------------------------------------------------
    mc_systrayMenu->addSeparator();
    // --------------------------------------------------------------
    //Advanced submenu
    SetupAdvancedMenu(mc_systrayMenu);
    // --------------------------------------------------------------
    SetupExperimentalMenu(mc_systrayMenu);
    // --------------------------------------------------------------

    // TODO: If the default isn't set, then choose the first one and select it.

    // TODO: If there isn't even ONE to select, then this menu item should say "Create Nym..." with no sub-menu.

    // TODO: When booting up, if there is already a default server and asset id, but no nyms exist, create a default nym.

    // TODO: When booting up, if there is already a default nym, but no accounts exist, create a default account.

    // --------------------------------------------------------------

    //Separator
    mc_systrayMenu->addSeparator();
    // --------------------------------------------------------------
    //Shutdown Moneychanger
    mc_systrayMenu_shutdown = new QAction(mc_systrayIcon_shutdown, tr("Quit"), mc_systrayMenu);
    mc_systrayMenu_shutdown->setMenuRole(QAction::NoRole);
    mc_systrayMenu_shutdown->setIcon(mc_systrayIcon_shutdown);
    mc_systrayMenu->addAction(mc_systrayMenu_shutdown);
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



bool Moneychanger::hasAccounts() const
{
    return (!default_account_id.isEmpty() ||
            (ot_.API().Exec().GetAccountCount() > 0));
}

bool Moneychanger::hasNyms() const
{
    return (!default_nym_id.isEmpty() ||
            (ot_.API().Exec().GetNymCount() > 0));
}

//bool Moneychanger::financeMode() const
//{
//    // NOTE: We don't even bother to show financial stuff
//    // on the menu, if you have zero Nyms.
//    //
//    return (!default_account_id.isEmpty() ||
//            !default_nym_id    .isEmpty() ||
//            (ot_.API().Exec().GetNymCount() > 0));
//}

void Moneychanger::SetupAssetMenu(QPointer<QMenu> & parent_menu)
{
    if (default_asset_id.isEmpty() && (ot_.API().Exec().GetAssetTypeCount() > 0))
    {
        default_asset_id = QString::fromStdString(ot_.API().Exec().GetAssetType_ID(0));
    }
    // -------------------------------------------------
    if (ot_.API().Exec().GetAssetTypeCount() <= 0)
    {
//        if (mc_systrayMenu_asset)
//            mc_systrayMenu_asset->disconnect();
//        mc_systrayMenu_asset = nullptr;

        QAction * manage_assets = new QAction(tr("Add asset contract..."), parent_menu);
        parent_menu->addAction(manage_assets);
        connect(manage_assets, SIGNAL(triggered()), this, SLOT(mc_defaultasset_slot()));
        return;
    }
    // -------------------------------------------------
    QPointer<QMenu> & current_menu = mc_systrayMenu_asset;
    // -------------------------------------------------
    current_menu = new QMenu(tr("Set default asset..."), parent_menu);
    current_menu->setIcon(mc_systrayIcon_purse);
    parent_menu->addMenu(current_menu);
    // --------------------------------------------------
    //Add a "Manage asset types" action button (and connection)
    QAction * manage_assets = new QAction(tr("Manage asset contracts..."), current_menu);
    manage_assets->setData(QVariant(QString("openmanager")));
    current_menu->addAction(manage_assets);
    connect(current_menu, SIGNAL(triggered(QAction*)), this, SLOT(mc_assetselection_triggered(QAction*)));
    // -------------------------------------------------
    current_menu->addSeparator();
    // -------------------------------------------------
    if (!default_asset_id.isEmpty())
    {
        default_asset_name = QString::fromStdString(ot_.API().Exec().GetAssetType_Name(default_asset_id.toStdString()));
    }
    // -------------------------------------------------
    //Load "default" asset type

    //Init asset submenu
    asset_list_id   = new QList<QVariant>;
    asset_list_name = new QList<QVariant>;
    // ------------------------------------------
    int32_t asset_count = ot_.API().Exec().GetAssetTypeCount();

    for (int aa = 0; aa < asset_count; aa++)
    {
        QString OT_asset_id   = QString::fromStdString(ot_.API().Exec().GetAssetType_ID(aa));
        QString OT_asset_name = QString::fromStdString(ot_.API().Exec().GetAssetType_Name(OT_asset_id.toStdString()));

        asset_list_id  ->append(QVariant(OT_asset_id));
        asset_list_name->append(QVariant(OT_asset_name));

        QAction * next_asset_action = new QAction(mc_systrayIcon_purse, OT_asset_name, current_menu);
        next_asset_action->setData(QVariant(OT_asset_id));
        next_asset_action->setCheckable(true);

        if (0 == OT_asset_id.compare(default_asset_id)) {
            next_asset_action->setChecked(true);
        }
        else {
            next_asset_action->setChecked(false);
        }

        current_menu->addAction(next_asset_action);
    }

    setDefaultAsset(default_asset_id, default_asset_name);
}

void Moneychanger::SetupServerMenu(QPointer<QMenu> & parent_menu)
{
    if (default_notary_id.isEmpty() && (ot_.API().Exec().GetServerCount() > 0))
    {
        default_notary_id = QString::fromStdString(ot_.API().Exec().GetServer_ID(0));
    }
    // -------------------------------------------------
    if (ot_.API().Exec().GetServerCount() <= 0)
    {
//        if (mc_systrayMenu_server)
//            mc_systrayMenu_server->disconnect();
//        mc_systrayMenu_server = nullptr;

        QAction * manage_servers = new QAction(tr("Add server contract..."), parent_menu);
        parent_menu->addAction(manage_servers);
        connect(manage_servers, SIGNAL(triggered()), this, SLOT(mc_defaultserver_slot()));
        return;
    }
    // -------------------------------------------------
    QPointer<QMenu> & current_menu = mc_systrayMenu_server;
    // -------------------------------------------------
    current_menu = new QMenu(tr("Set default server..."), parent_menu);
    current_menu->setIcon(mc_systrayIcon_server);
    parent_menu->addMenu(current_menu);
    // --------------------------------------------------
    //Add a "Manage Servers" action button (and connection)
    QAction * manage_servers = new QAction(tr("Manage Server Contracts..."), current_menu);
    manage_servers->setData(QVariant(QString("openmanager")));
    current_menu->addAction(manage_servers);
    connect(current_menu, SIGNAL(triggered(QAction*)), this, SLOT(mc_serverselection_triggered(QAction*)));
    // -------------------------------------------------
    current_menu->addSeparator();
    // -------------------------------------------------
    //Ask OT what the display name of this server is and store it for a quick retrieval later on(mostly for "Default Server" displaying purposes)
    if (!default_notary_id.isEmpty())
    {
        default_server_name = QString::fromStdString(ot_.API().Exec().GetServer_Name(default_notary_id.toStdString()));
    }
    // -------------------------------------------------
    //Load "default" server

    //Init server submenu
    server_list_id   = new QList<QVariant>;
    server_list_name = new QList<QVariant>;
    // ------------------------------------------
    int32_t server_count = ot_.API().Exec().GetServerCount();

    for (int32_t aa = 0; aa < server_count; aa++)
    {
        QString OT_notary_id   = QString::fromStdString(ot_.API().Exec().GetServer_ID(aa));
        QString OT_server_name = QString::fromStdString(ot_.API().Exec().GetServer_Name(OT_notary_id.toStdString()));

        server_list_id  ->append(QVariant(OT_notary_id));
        server_list_name->append(QVariant(OT_server_name));

        //Append to submenu of server
        QAction * next_server_action = new QAction(mc_systrayIcon_server, OT_server_name, current_menu);
        next_server_action->setData(QVariant(OT_notary_id));
        next_server_action->setCheckable(true);

        if (0 == OT_notary_id.compare(default_notary_id)) {
            next_server_action->setChecked(true);
        }
        else {
            next_server_action->setChecked(false);
        }

        current_menu->addAction(next_server_action);
    }

    setDefaultServer(default_notary_id, default_server_name);
}

void Moneychanger::SetupNymMenu(QPointer<QMenu> & parent_menu)
{
    // -------------------------------------------------
    if (default_nym_id.isEmpty() && (ot_.API().Exec().GetNymCount() > 0))
    {
        default_nym_id = QString::fromStdString(ot_.API().Exec().GetNym_ID(0));
    }
    // -------------------------------------------------
    if (!hasNyms())
    {
//        if (mc_systrayMenu_nym)
//            mc_systrayMenu_nym->disconnect();
//        mc_systrayMenu_nym = nullptr;
        QAction * manage_nyms = new QAction(tr("Create my identity..."), parent_menu);
        parent_menu->addAction(manage_nyms);
        connect(manage_nyms, SIGNAL(triggered()), this, SLOT(mc_defaultnym_slot()));
        return;
    }
    // -------------------------------------------------
    QPointer<QMenu> & current_menu = mc_systrayMenu_nym;
    // -------------------------------------------------
    current_menu = new QMenu("Set default identity...", parent_menu);
    current_menu->setIcon(mc_systrayIcon_nym);
    parent_menu->addMenu(current_menu);

    //Add a "Manage pseudonym" action button (and connection)
    QAction * manage_nyms = new QAction(tr("Manage my identities..."), current_menu);
    manage_nyms->setData(QVariant(QString("openmanager")));
    current_menu->addAction(manage_nyms);
    connect(current_menu, SIGNAL(triggered(QAction*)), this, SLOT(mc_nymselection_triggered(QAction*)));
    // -------------------------------------------------
    current_menu->addSeparator();
    // -------------------------------------------------
    //Ask OT what the display name of this nym is and store it for quick retrieval later on(mostly for "Default Nym" displaying purposes)
    if (!default_nym_id.isEmpty())
    {
        default_nym_name =  QString::fromStdString(ot_.API().Exec().GetNym_Name(default_nym_id.toStdString()));
    }
    // -------------------------------------------------
    //Init nym submenu
    nym_list_id = new QList<QVariant>;
    nym_list_name = new QList<QVariant>;
    // --------------------------------------------------------
    //Count nyms
    int32_t nym_count = ot_.API().Exec().GetNymCount();

    //Add/append to the id + name lists
    for (int32_t a = 0; a < nym_count; a++)
    {
        QString OT_nym_id   = QString::fromStdString(ot_.API().Exec().GetNym_ID(a));
        QString OT_nym_name = QString::fromStdString(ot_.API().Exec().GetNym_Name(OT_nym_id.toStdString()));

        nym_list_id  ->append(QVariant(OT_nym_id));
        nym_list_name->append(QVariant(OT_nym_name));

        //Append to submenu of nym
        QAction * next_nym_action = new QAction(mc_systrayIcon_nym, OT_nym_name, current_menu);
        next_nym_action->setData(QVariant(OT_nym_id));
        next_nym_action->setCheckable(true);

        if (0 == OT_nym_id.compare(default_nym_id)) {
            next_nym_action->setChecked(true);
        }
        else {
            next_nym_action->setChecked(false);
        }

        current_menu->addAction(next_nym_action);

    } // for
    // -------------------------------------------------
    setDefaultNym(default_nym_id, default_nym_name);
}


void Moneychanger::SetupPaymentsMenu(QPointer<QMenu> & parent_menu)
{
    // -------------------------------------------------
    QPointer<QMenu> & current_menu = mc_systrayMenu_payments;
    // -------------------------------------------------
    current_menu = new QMenu(tr("Payments"), parent_menu);
    current_menu->setIcon(mc_systrayIcon_sendfunds);
    parent_menu->addMenu(current_menu);
    // --------------------------------------------------------------
    //Send funds
    mc_systrayMenu_sendfunds = new QAction(mc_systrayIcon_sendfunds, tr("Pay Funds..."), current_menu);
    current_menu->addAction(mc_systrayMenu_sendfunds);
    connect(mc_systrayMenu_sendfunds, SIGNAL(triggered()), this, SLOT(mc_sendfunds_slot()));
    // -------------------------------------------------
    current_menu->addSeparator();
    // -------------------------------------------------
    //Payment History
    mc_systrayMenu_receipts = new QAction(mc_systrayIcon_overview, tr("Payment History"), current_menu);
    current_menu->addAction(mc_systrayMenu_receipts);
    connect(mc_systrayMenu_receipts, SIGNAL(triggered()), this, SLOT(mc_payments_slot()));
    // --------------------------------------------------------------
    //Pending Transactions
    mc_systrayMenu_overview = new QAction(mc_systrayIcon_pending, tr("Pending Transactions"), current_menu);
    current_menu->addAction(mc_systrayMenu_overview);
    connect(mc_systrayMenu_overview, SIGNAL(triggered()), this, SLOT(mc_overview_slot()));
    // --------------------------------------------------------------
    //Live Agreements
    if (bExpertMode_)
    {
        mc_systrayMenu_agreements = new QAction(mc_systrayIcon_active_agreements, tr("Active Agreements"), current_menu);
        current_menu->addAction(mc_systrayMenu_agreements);
        connect(mc_systrayMenu_agreements, SIGNAL(triggered()), this, SLOT(mc_agreements_slot()));
    }
    // -------------------------------------------------
    current_menu->addSeparator();
    // -------------------------------------------------
    //Request payment
    mc_systrayMenu_requestfunds = new QAction(mc_systrayIcon_requestfunds, tr("Request Payment..."), current_menu);
    current_menu->addAction(mc_systrayMenu_requestfunds);
    connect(mc_systrayMenu_requestfunds, SIGNAL(triggered()), this, SLOT(mc_requestfunds_slot()));
    // --------------------------------------------------------------
    if (bExpertMode_)
    {
        mc_systrayMenu_proposeplan = new QAction(mc_systrayIcon_proposeplan, tr("Recurring Payment..."), current_menu);
        current_menu->addAction(mc_systrayMenu_proposeplan);
        connect(mc_systrayMenu_proposeplan, SIGNAL(triggered()), this, SLOT(mc_proposeplan_slot()));
        // -------------------------------------------------
        mc_systrayMenu_import_cash = new QAction(mc_systrayIcon_advanced_import, tr("Import Cash..."), current_menu);
        current_menu->addAction(mc_systrayMenu_import_cash);
        connect(mc_systrayMenu_import_cash, SIGNAL(triggered()), this, SLOT(mc_import_slot()));
    }
}

// --------------------------------------------------------------

void Moneychanger::SetupExchangeMenu(QPointer<QMenu> & parent_menu)
{
    // -------------------------------------------------
    QPointer<QMenu> & current_menu = mc_systrayMenu_exchange;
    // -------------------------------------------------
    current_menu = new QMenu(tr("Exchange"), parent_menu);
    current_menu->setIcon(mc_systrayIcon_markets);
    parent_menu->addMenu(current_menu);
    // --------------------------------------------------------------
    mc_systrayMenu_markets = new QAction(mc_systrayIcon_markets, tr("Live Offers"), current_menu);
    current_menu->addAction(mc_systrayMenu_markets);
    connect(mc_systrayMenu_markets, SIGNAL(triggered()), this, SLOT(mc_market_slot()));
    // --------------------------------------------------------------
    mc_systrayMenu_trade_archive = new QAction(mc_systrayIcon_trade_archive, tr("Historical Trades"), mc_systrayMenu_exchange);
    mc_systrayMenu_exchange->addAction(mc_systrayMenu_trade_archive);
    connect(mc_systrayMenu_trade_archive, SIGNAL(triggered()), this, SLOT(mc_trade_archive_slot()));
}


void Moneychanger::SetupContractsMenu(QPointer<QMenu> & parent_menu)
{
    QPointer<QMenu> & current_menu = mc_systrayMenu_contracts;
    // -------------------------------------------------
    current_menu = new QMenu(tr("Contracts"), parent_menu);
    current_menu->setIcon(mc_systrayIcon_active_agreements);
    parent_menu->addMenu(current_menu);
    // --------------------------------------------------------------
    SetupAssetMenu(current_menu);
    SetupServerMenu(current_menu);
    // -------------------------------------------------
    current_menu->addSeparator();
    // -------------------------------------------------
    if (bExpertMode_)
    {
        mc_systrayMenu_smart_contracts = new QAction(mc_systrayIcon_advanced_smartcontracts, tr("Smart Contract Editor"), current_menu);
        current_menu->addAction(mc_systrayMenu_smart_contracts);
        connect(mc_systrayMenu_smart_contracts, SIGNAL(triggered()), this, SLOT(mc_smartcontract_slot()));
    }
}

void Moneychanger::SetupAdvancedMenu(QPointer<QMenu> & parent_menu)
{
    if (bExpertMode_)
        return;
    // -------------------------------------------------

    QPointer<QMenu> & current_menu = mc_systrayMenu_advanced;

    // -------------------------------------------------
    current_menu = new QMenu(tr("Advanced"), parent_menu);
    current_menu->setIcon(mc_systrayIcon_active_agreements);
    parent_menu->addMenu(current_menu);
    // --------------------------------------------------------------
//    SetupContractsMenu(current_menu);
    // --------------------------------------------------------------
    if (hasNyms())
    {
        SetupToolsMenu(current_menu);
    }
    // --------------------------------------------------------------
    current_menu->addSeparator();
    // --------------------------------------------------------------
    if (bExpertMode_ && hasNyms())
    {
        mc_systrayMenu_smart_contracts = new QAction(mc_systrayIcon_advanced_smartcontracts, tr("Smart Contract Editor"), current_menu);
        current_menu->addAction(mc_systrayMenu_smart_contracts);
        connect(mc_systrayMenu_smart_contracts, SIGNAL(triggered()), this, SLOT(mc_smartcontract_slot()));
        // --------------------------------------------------------------
        current_menu->addSeparator();
    }
    // --------------------------------------------------------------
    // Basic mode doesn't show the cash purse on the account details page.
    // So what's the point of importing cash, when you then can't see it in the
    // cash purse? It would just be confusing.
    // Therefore, if you cannot see the cash purse, neither can you import cash.
    // Put yourself in expert mode if you want to do that stuff.
    //
//    if (hasAccounts())
//    {
//        // --------------------------------------------------------------
//        mc_systrayMenu_import_cash = new QAction(mc_systrayIcon_advanced_import, tr("Import Cash..."), current_menu);
//        current_menu->addAction(mc_systrayMenu_import_cash);
//        connect(mc_systrayMenu_import_cash, SIGNAL(triggered()), this, SLOT(mc_import_slot()));
//        // --------------------------------------------------------------
//        current_menu->addSeparator();
//    }
    // -------------------------------------------------
    current_menu->addSeparator();
    // -------------------------------------------------
    mc_systrayMenu_pair_node = new QAction(mc_systrayIcon_advanced_corporations, tr("Pair Stash Node"), current_menu);
    current_menu->addAction(mc_systrayMenu_pair_node);
    connect(mc_systrayMenu_pair_node, SIGNAL(triggered()), this, SLOT(mc_pair_node_slot()));
    // --------------------------------------------------------------
    //Separator
    current_menu->addSeparator();
    // --------------------------------------------------------------
    // Transport
    if (bExpertMode_) {
        mc_systrayMenu_p2p_transport = new QAction(mc_systrayIcon_advanced_transport, tr("P2P Transport"), current_menu);
        current_menu->addAction(mc_systrayMenu_p2p_transport);
        connect(mc_systrayMenu_p2p_transport, SIGNAL(triggered()), this, SLOT(mc_transport_slot()));
    }
    // --------------------------------------------------------------
    // Error Log
    mc_systrayMenu_error_log = new QAction(mc_systrayIcon_overview, tr("Error Log"), current_menu);
    current_menu->addAction(mc_systrayMenu_error_log);
    connect(mc_systrayMenu_error_log, SIGNAL(triggered()), this, SLOT(mc_log_slot()));
}

void Moneychanger::SetupExperimentalMenu(QPointer<QMenu> & parent_menu)
{
    if (!bExpertMode_)
        return;
    // -------------------------------------------------

    QPointer<QMenu> & current_menu = mc_systrayMenu_experimental;

    // -------------------------------------------------
    //Experimental
    current_menu = new QMenu(tr("Experimental"), parent_menu);
    current_menu->setIcon(mc_systrayIcon_advanced);
    parent_menu->addMenu(current_menu);
    // --------------------------------------------------------------
    // Corporations
    mc_systrayMenu_corporations = new QAction(mc_systrayIcon_advanced_corporations, tr("Corporations"), current_menu);
    current_menu->addAction(mc_systrayMenu_corporations);
    connect(mc_systrayMenu_corporations, SIGNAL(triggered()), this, SLOT(mc_corporation_slot()));
    // -------------------------------------------------
    current_menu->addSeparator();
    // -------------------------------------------------
    mc_systrayMenu_pair_node = new QAction(mc_systrayIcon_advanced_corporations, tr("Pair Stash Node"), current_menu);
    current_menu->addAction(mc_systrayMenu_pair_node);
    connect(mc_systrayMenu_pair_node, SIGNAL(triggered()), this, SLOT(mc_pair_node_slot()));
    // --------------------------------------------------------------
    //Separator
//    current_menu->addSeparator();
    // --------------------------------------------------------------
    // Corporations submenu
//    mc_systrayMenu_company_create = new QMenu(tr("Create"), 0);
//    mc_systrayMenu_corporations->addMenu(mc_systrayMenu_company_create);

//    // Create insurance company action on submenu
//    mc_systrayMenu_company_create_insurance = new QAction(mc_systrayIcon_advanced_smartcontracts, tr("Insurance Company"), 0);
//    mc_systrayMenu_company_create->addAction(mc_systrayMenu_company_create_insurance);
//    connect(mc_systrayMenu_company_create_insurance, SIGNAL(triggered()), this, SLOT(mc_createinsurancecompany_slot()));
    // --------------------------------------------------------------
    // Bazaar
//    mc_systrayMenu_bazaar = new QMenu(tr("Bazaar"), current_menu);
//    current_menu->addMenu(mc_systrayMenu_bazaar);
//
//    // Bazaar actions
//    mc_systrayMenu_bazaar_search = new QAction(mc_systrayIcon_advanced_smartcontracts, tr("Search Listings"), mc_systrayMenu_bazaar);
//    mc_systrayMenu_bazaar->addAction(mc_systrayMenu_bazaar_search);
////  connect(mc_systrayMenu_bazaar_search, SIGNAL(triggered()), this, SLOT(mc_bazaar_search_slot()));
//
//    mc_systrayMenu_bazaar_post = new QAction(mc_systrayIcon_advanced_smartcontracts, tr("Post an Ad"), mc_systrayMenu_bazaar);
//    mc_systrayMenu_bazaar->addAction(mc_systrayMenu_bazaar_post);
////  connect(mc_systrayMenu_bazaar_post, SIGNAL(triggered()), this, SLOT(mc_bazaar_search_slot()));
//
//    mc_systrayMenu_bazaar_orders = new QAction(mc_systrayIcon_advanced_smartcontracts, tr("Orders"), mc_systrayMenu_bazaar);
//    mc_systrayMenu_bazaar->addAction(mc_systrayMenu_bazaar_orders);
////  connect(mc_systrayMenu_bazaar_orders, SIGNAL(triggered()), this, SLOT(mc_bazaar_search_slot()));
    // -------------------------------------------------
//    current_menu->addSeparator();
    // -------------------------------------------------
    // Bitcoin
//    mc_systrayMenu_bitcoin = new QMenu(tr("Bitcoin"), current_menu);
//    mc_systrayMenu_bitcoin->setIcon(mc_systrayIcon_bitcoin);
//
//    current_menu->addMenu(mc_systrayMenu_bitcoin);
//
//    mc_systrayMenu_bitcoin_test = new QAction(tr("Test"), mc_systrayMenu_bitcoin);
//    mc_systrayMenu_bitcoin_connect = new QAction(tr("Connect to wallet"), mc_systrayMenu_bitcoin);
//    mc_systrayMenu_bitcoin_pools = new QAction(tr("Pools"), mc_systrayMenu_bitcoin);
//    mc_systrayMenu_bitcoin_transactions = new QAction(tr("Transactions"), mc_systrayMenu_bitcoin);
//    mc_systrayMenu_bitcoin_send = new QAction(tr("Send"), mc_systrayMenu_bitcoin);
//    mc_systrayMenu_bitcoin_receive = new QAction(tr("Receive"), mc_systrayMenu_bitcoin);
//    mc_systrayMenu_bitcoin->addAction(mc_systrayMenu_bitcoin_connect);
//    mc_systrayMenu_bitcoin->addAction(mc_systrayMenu_bitcoin_send);
//    mc_systrayMenu_bitcoin->addAction(mc_systrayMenu_bitcoin_receive);
//    mc_systrayMenu_bitcoin->addAction(mc_systrayMenu_bitcoin_transactions);
//    mc_systrayMenu_bitcoin->addSeparator();
//    mc_systrayMenu_bitcoin->addAction(mc_systrayMenu_bitcoin_test);
//    mc_systrayMenu_bitcoin->addAction(mc_systrayMenu_bitcoin_pools);
//    connect(mc_systrayMenu_bitcoin_test, SIGNAL(triggered()), this, SLOT(mc_bitcoin_slot()));
//    connect(mc_systrayMenu_bitcoin_connect, SIGNAL(triggered()), this, SLOT(mc_bitcoin_connect_slot()));
//    connect(mc_systrayMenu_bitcoin_pools, SIGNAL(triggered()), this, SLOT(mc_bitcoin_pools_slot()));
//    connect(mc_systrayMenu_bitcoin_transactions, SIGNAL(triggered()), this, SLOT(mc_bitcoin_transactions_slot()));
//    connect(mc_systrayMenu_bitcoin_send, SIGNAL(triggered()), this, SLOT(mc_bitcoin_send_slot()));
//    connect(mc_systrayMenu_bitcoin_receive, SIGNAL(triggered()), this, SLOT(mc_bitcoin_receive_slot()));
    // --------------------------------------------------------------
    //Separator
//    current_menu->addSeparator();
}

void Moneychanger::SetupToolsMenu(QPointer<QMenu> & parent_menu)
{
    // -------------------------------------------------

    QPointer<QMenu> & current_menu = mc_systrayMenu_tools;

    // -------------------------------------------------
    current_menu = new QMenu(tr("Crypto Tools"), parent_menu);
    current_menu->setIcon(mc_systrayIcon_crypto_sign);
    parent_menu->addMenu(current_menu);
    // --------------------------------------------------------------
    mc_systrayMenu_crypto_sign = new QAction(mc_systrayIcon_crypto_sign, tr("Sign"), current_menu);
    current_menu->addAction(mc_systrayMenu_crypto_sign);
    connect(mc_systrayMenu_crypto_sign, SIGNAL(triggered()), this, SLOT(mc_crypto_sign_slot()));
    // --------------------------------------------------------------
    mc_systrayMenu_crypto_encrypt = new QAction(mc_systrayIcon_crypto_encrypt, tr("Encrypt"), current_menu);
    current_menu->addAction(mc_systrayMenu_crypto_encrypt);
    connect(mc_systrayMenu_crypto_encrypt, SIGNAL(triggered()), this, SLOT(mc_crypto_encrypt_slot()));
    // --------------------------------------------------------------
    //Separator
    current_menu->addSeparator();
    // --------------------------------------------------------------
    mc_systrayMenu_crypto_decrypt = new QAction(mc_systrayIcon_crypto_decrypt, tr("Decrypt / Verify"), current_menu);
    current_menu->addAction(mc_systrayMenu_crypto_decrypt);
    connect(mc_systrayMenu_crypto_decrypt, SIGNAL(triggered()), this, SLOT(mc_crypto_decrypt_slot()));
    // --------------------------------------------------------------
}

void Moneychanger::SetupMessagingMenu(QPointer<QMenu> & parent_menu)
{
    // -------------------------------------------------

    QPointer<QMenu> & current_menu = mc_systrayMenu_messaging;

    // -------------------------------------------------
    current_menu = new QMenu(tr("Messages"), parent_menu);
    current_menu->setIcon(mc_systrayIcon_composemessage);
    parent_menu->addMenu(current_menu);
    // -------------------------------------------------
    //Compose Message
    mc_systrayMenu_composemessage = new QAction(mc_systrayIcon_composemessage, tr("Compose Message"), current_menu);
    current_menu->addAction(mc_systrayMenu_composemessage);
    connect(mc_systrayMenu_composemessage, SIGNAL(triggered()), this, SLOT(mc_composemessage_slot()));
    // --------------------------------------------------------------
    current_menu->addSeparator();
    // --------------------------------------------------------------
    //Message History
    mc_systrayMenu_messages = new QAction(mc_systrayIcon_overview, tr("Message History"), current_menu);
    current_menu->addAction(mc_systrayMenu_messages);
    connect(mc_systrayMenu_messages, SIGNAL(triggered()), this, SLOT(mc_messages_slot()));
    // --------------------------------------------------------------
    //Address Book
    mc_systrayMenu_contacts = new QAction(mc_systrayIcon_contacts, tr("Address Book"), current_menu);
    current_menu->addAction(mc_systrayMenu_contacts);
    connect(mc_systrayMenu_contacts, SIGNAL(triggered()), this, SLOT(mc_addressbook_slot()));
    // --------------------------------------------------------------
    current_menu->addSeparator();
    // --------------------------------------------------------------
    // Transport
    if (bExpertMode_)
    {
        mc_systrayMenu_p2p_transport = new QAction(mc_systrayIcon_advanced_transport, tr("P2P Transport"), current_menu);
        current_menu->addAction(mc_systrayMenu_p2p_transport);
        connect(mc_systrayMenu_p2p_transport, SIGNAL(triggered()), this, SLOT(mc_transport_slot()));
        // --------------------------------------------------------------
        // Error Log
        mc_systrayMenu_error_log = new QAction(mc_systrayIcon_overview, tr("Error Log"), current_menu);
        current_menu->addAction(mc_systrayMenu_error_log);
        connect(mc_systrayMenu_error_log, SIGNAL(triggered()), this, SLOT(mc_log_slot()));
    }
    // --------------------------------------------------------------
}

void Moneychanger::SetupAccountMenu(QPointer<QMenu> & parent_menu)
{
    if (default_account_id.isEmpty() && (ot_.API().Exec().GetAccountCount() > 0))
    {
        default_account_id = QString::fromStdString(ot_.API().Exec().GetAccountWallet_ID(0));
    }
    // -------------------------------------------------
    if (ot_.API().Exec().GetAccountCount() <= 0)
    {
//        if (mc_systrayMenu_account)
//            mc_systrayMenu_account->disconnect();
//        mc_systrayMenu_account = nullptr;
        QAction * manage_accounts = new QAction(tr("Create an account..."), parent_menu);
        parent_menu->addAction(manage_accounts);
        connect(manage_accounts, SIGNAL(triggered()), this, SLOT(mc_defaultaccount_slot()));
        return;
    }
    // -------------------------------------------------
    QPointer<QMenu> & current_menu = mc_systrayMenu_account;
    // -------------------------------------------------
    current_menu = new QMenu(tr("Set default account..."), parent_menu);
    current_menu->setIcon(mc_systrayIcon_goldaccount);
    parent_menu->addMenu(current_menu);

    //Add a "Manage accounts" action button (and connection)
    QAction * manage_accounts = new QAction(tr("Manage Accounts..."), current_menu);
    manage_accounts->setData(QVariant(QString("openmanager")));
    current_menu->addAction(manage_accounts);
    connect(current_menu, SIGNAL(triggered(QAction*)), this, SLOT(mc_accountselection_triggered(QAction*)));
    // -------------------------------------------------
    current_menu->addSeparator();
    // -------------------------------------------------
    //Ask OT what the display name of this account is and store it for a quick retrieval later on(mostly for "Default Account" displaying purposes)
    if (!default_account_id.isEmpty())
    {
        default_account_name = QString::fromStdString(ot_.API().Exec().GetAccountWallet_Name(default_account_id.toStdString()));
    }
    // -------------------------------------------------
    //Init account submenu
    account_list_id   = new QList<QVariant>;
    account_list_name = new QList<QVariant>;
    // ------------------------------------------
    int32_t account_count = ot_.API().Exec().GetAccountCount();

    for (int aa = 0; aa < account_count; aa++)
    {
        QString OT_account_id   = QString::fromStdString(ot_.API().Exec().GetAccountWallet_ID(aa));
        QString OT_account_name = QString::fromStdString(ot_.API().Exec().GetAccountWallet_Name(OT_account_id.toStdString()));

        account_list_id  ->append(QVariant(OT_account_id));
        account_list_name->append(QVariant(OT_account_name));

        QAction * next_account_action = new QAction(mc_systrayIcon_goldaccount, OT_account_name, current_menu);
        next_account_action->setData(QVariant(OT_account_id));
        next_account_action->setCheckable(true);

        if (0 == OT_account_id.compare(default_account_id)) {
            next_account_action->setChecked(true);
        }
        else {
            next_account_action->setChecked(false);
        }

        current_menu->addAction(next_account_action);
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
    contactswindow->setWindowTitle(tr("Address Book"));
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
 * Opentxs Contacts
 **/

// text may contain a "pre-selected" Contact ID
// (A new-style, string-based contact ID in opentxs)
void Moneychanger::mc_opentxs_contact_show(QString text/*=QString("")*/)
{
    if (!opentxscontactswindow)
        opentxscontactswindow = new MTDetailEdit(this);
    // -------------------------------------
    opentxscontactswindow->m_map.clear();
    // -------------------------------------
    MTContactHandler::getInstance()->GetOpentxsContacts(opentxscontactswindow->m_map);
    // -------------------------------------
    if (!text.isEmpty())
        opentxscontactswindow->SetPreSelected(text);
    // -------------------------------------
    opentxscontactswindow->setWindowTitle(tr("Opentxs Contacts"));
    // -------------------------------------
    opentxscontactswindow->dialog(MTDetailEdit::DetailEditTypeOpentxsContact);
}

void Moneychanger::mc_opentxs_contacts_slot()
{
    mc_opentxs_contact_show();
}

void Moneychanger::mc_show_opentxs_contact_slot(QString text)
{
    mc_opentxs_contact_show(text);
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

void Moneychanger::onHideNavUpdated(bool bHideNav)
{
    bHideNav_ = bHideNav;

    mc_main_menu_dialog(!bHideNav);
}

void Moneychanger::onExpertModeUpdated(bool bExpertMode)
{
    bExpertMode_ = bExpertMode;
    // --------------------------------
    SetupMainMenu();
    // --------------------------------
    bool bActivityWindowVisible = false;

    if (activity_window)
    {
        bActivityWindowVisible = activity_window->isVisible();
        // -------------------------------------
        activity_window->setParent(NULL);
        activity_window->disconnect();
        activity_window->setAttribute(Qt::WA_DeleteOnClose, true);
        activity_window->close();
    }

    activity_window = nullptr;

    if (bActivityWindowVisible) {
        mc_activity_slot();
    }
    // --------------------------------
    bool bNymsWindowVisible = false;
    QString nym_id("");

    if (nymswindow)
    {
        nym_id = nymswindow->m_qstrCurrentID;
        bNymsWindowVisible = nymswindow->isVisible();
        // -------------------------------------
        nymswindow->setParent(NULL);
        nymswindow->disconnect();
        nymswindow->setAttribute(Qt::WA_DeleteOnClose, true);
        nymswindow->close();
    }

    nymswindow = nullptr;

    if (bNymsWindowVisible)
        mc_nymmanager_dialog(nym_id);
    // --------------------------------
    bool bAccountWindowVisible = false;
    QString acct_id("");

    if (accountswindow)
    {
        acct_id = accountswindow->m_qstrCurrentID;
        bAccountWindowVisible = accountswindow->isVisible();
        // -------------------------------------
        accountswindow->setParent(NULL);
        accountswindow->disconnect();
        accountswindow->setAttribute(Qt::WA_DeleteOnClose, true);
        accountswindow->close();
    }

    accountswindow = nullptr;

    if (bAccountWindowVisible)
        mc_show_account_slot(acct_id);
    // --------------------------------
    bool bContactWindowVisible = false;
    QString contact_id("");

    if (contactswindow)
    {
        contact_id = contactswindow->m_qstrCurrentID;
        bContactWindowVisible = contactswindow->isVisible();
        // -------------------------------------
        contactswindow->setParent(NULL);
        contactswindow->disconnect();
        contactswindow->setAttribute(Qt::WA_DeleteOnClose, true);
        contactswindow->close();
    }

    contactswindow = nullptr;

    if (bContactWindowVisible)
        mc_showcontact_slot(contact_id);
    // --------------------------------
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
    int32_t nym_count = ot_.API().Exec().GetNymCount();
    bool bFoundPreset = false;

    for (int32_t ii = 0; ii < nym_count; ii++)
    {
        QString OT_id   = QString::fromStdString(ot_.API().Exec().GetNym_ID(ii));
        QString OT_name = QString::fromStdString(ot_.API().Exec().GetNym_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);
        // ------------------------------
        if (!qstrPresetID.isEmpty() && (0 == qstrPresetID.compare(OT_id)))
            bFoundPreset = true;
        // ------------------------------
    } // for
    // -------------------------------------
    nymswindow->setWindowTitle(tr("Manage my identities"));
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
        if (mc_systrayMenu_nym)
            mc_systrayMenu_nym->setTitle(tr("Identity: ")+nym_name);

        if (mc_overall_init)
        {
            // Loop through actions in mc_systrayMenu_nym.
            // Ignore the "openmanager" action.
            // For all others, compare the data to the default nym ID.
            // If one matches, set the "checked" property to true, and for
            // all others, set to false.

            if (mc_systrayMenu_nym)
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



void Moneychanger::onNeedToDownloadMail()
{
    QString qstrErrorMsg;
    qstrErrorMsg = tr("Failed trying to contact the notary. Perhaps it is down, or there might be a network problem.");
    // -----------------------------
    int32_t nymCount = ot_.API().Exec().GetNymCount();

    if (0 == nymCount)
    {
        std::string strSource(""), strMyName("Me");
        // --------------------------------------
        MTGetStringDialog nameDlg(this, tr("Creating your Nym. "
                                           "Enter your display name (that others will see)"));

        bool bNameEmpty{true};
        if (QDialog::Accepted == nameDlg.exec())
        {
            const QString qstrNewMyName = nameDlg.GetOutputString();
            if (!qstrNewMyName.isEmpty()) {
                bNameEmpty = false;
                strMyName = qstrNewMyName.toStdString();
            }
        }
        // --------------------------------------------------
        if (bNameEmpty) {
            QString name;
            name = qgetenv("USER"); // get the user name in Linux
            if (!name.isEmpty()) {
                bNameEmpty = false;
                strMyName = name.toStdString();
            }
            else {
                name = qgetenv("USERNAME"); // get the name in Windows
            }
            if (!name.isEmpty()) {
                bNameEmpty = false;
                strMyName = name.toStdString();
            }
        }
        // --------------------------------------------------
        std::string newNymId = ot_.API().Exec().CreateNymHD(
            opentxs::proto::CITEMTYPE_INDIVIDUAL, strMyName, strSource, 0);
        // --------------------------------------------------
        if (!newNymId.empty())
        {
            DBHandler::getInstance()->AddressBookUpdateDefaultNym(QString::fromStdString(newNymId));
            qDebug() << "Finished Making Nym";
        }

        nymCount = ot_.API().Exec().GetNymCount();
    }
    // ----------------------------------------------------------------
    std::string defaultNymID(get_default_nym_id().toStdString());
    // ----------------------------------------------------------------
    // Some messages come from Bitmessage or other transport layers, and
    // other messages from via OT. So let's download any latest Nym info
    // from OT... (Messages are shipped via the nymbox.)
    //
    if ((get_server_list_id_size() > 0))
    {
        std::string defaultNotaryID(get_default_notary_id().toStdString());
        // ----------------------------------------------------------------
        if (defaultNotaryID.empty())
        {
            defaultNotaryID = get_notary_id_at(0).toStdString();
            DBHandler::getInstance()->AddressBookUpdateDefaultServer(QString::fromStdString(defaultNotaryID));
        }
        // ----------------------------------------------------------------
        if (!defaultNymID.empty() && !defaultNotaryID.empty())
        {
            bool isReg = ot_.API().Exec().IsNym_RegisteredAtServer(defaultNymID, defaultNotaryID);

            if (!isReg)
            {
                std::string response;
                {
                    MTSpinner theSpinner;

                    response = opentxs::String(ot_.API().Sync().RegisterNym(opentxs::Identifier(defaultNymID), opentxs::Identifier(defaultNotaryID), true)).Get();

                    if (!ot_.API().Exec().CheckConnection(defaultNotaryID))
                    {
                        emit appendToLog(qstrErrorMsg);
                        return;
                    }
                }

                if (!opentxs::VerifyMessageSuccess(response)) {
                    Moneychanger::It()->HasUsageCredits(defaultNotaryID, defaultNymID);
                    return;
                }
                else
                    MTContactHandler::getInstance()->NotifyOfNymServerPair(QString::fromStdString(defaultNymID),
                                                                           QString::fromStdString(defaultNotaryID));

//              qDebug() << QString("Creation Response: %1").arg(QString::fromStdString(response));
            }
        }
        // ----------------------------------------------------------------
        // Retrieve Nyms
        //
        int32_t serverCount = ot_.API().Exec().GetServerCount();

        for (int32_t serverIndex = 0; serverIndex < serverCount; ++serverIndex)
        {
            std::string NotaryID = ot_.API().Exec().GetServer_ID(serverIndex);

            for (int32_t nymIndex = 0; nymIndex < nymCount; ++nymIndex)
            {
                std::string nymId = ot_.API().Exec().GetNym_ID(nymIndex);

                bool bRetrievalAttempted = false;
                bool bRetrievalSucceeded = false;

                if (ot_.API().Exec().IsNym_RegisteredAtServer(nymId, NotaryID))
                {
                    MTSpinner theSpinner;

                    bRetrievalAttempted = true;
                    bRetrievalSucceeded = retrieve_nym(NotaryID, nymId);

                    if (!ot_.API().Exec().CheckConnection(NotaryID))
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
        onNeedToPopulateRecordlist();
        return;
    }
    else
    {
        qDebug() << QString("%1: There's not at least 1 server contract; doing nothing.").arg(__FUNCTION__);
    }
}


bool Moneychanger::AddFinalReceiptToTradeArchive(opentxs::OTRecord& recordmt)
{
    QPointer<ModelTradeArchive> pModel = DBHandler::getInstance()->getTradeArchiveModel();

    if (pModel)
    {
        QPointer<FinalReceiptProxyModel> pFinalReceiptProxy = new FinalReceiptProxyModel;
        pFinalReceiptProxy->setSourceModel(pModel);
        pFinalReceiptProxy->setFilterOpentxsRecord(recordmt);

        bool bEditing = false;
        QString qstrReceipt;

        int nRowCount = pFinalReceiptProxy->rowCount();

        if (nRowCount > 0) // Matching rows are present.
        {
            for (int nIndex = 0; nIndex < nRowCount; ++nIndex)
            {
                if (!bEditing)
                {
                    bEditing = true;
                    pModel->database().transaction();
                    qstrReceipt = QString::fromStdString(recordmt.GetContents());
                }

                QModelIndex proxyIndex  = pFinalReceiptProxy->index(nIndex, 0);
                QModelIndex actualIndex = pFinalReceiptProxy->mapToSource(proxyIndex);
                QSqlRecord  record      = pModel->record(actualIndex.row());
                record.setValue("final_receipt", qstrReceipt);
                pModel->setRecord(actualIndex.row(), record);
            }
        }
        else if (recordmt.HasOriginType() && recordmt.IsOriginTypeMarketOffer())
        {
            bEditing = true;
            pModel->database().transaction();
            qstrReceipt = QString::fromStdString(recordmt.GetContents());

            QSqlRecord record = pModel->record();

//            const bool bIsBid = false; // I guess I have to derive this from the record's contents...

            const int64_t lReceiptID = recordmt.GetTransactionNum();
            const int64_t lOfferID   = recordmt.GetTransNumForDisplay();
            const std::string & strNotaryID = recordmt.GetNotaryID();
            const std::string & strNymID = recordmt.GetNymID();

            const time64_t tDate = static_cast<time64_t>(ot_.API().Exec().StringToLong(recordmt.GetDate()));

//            record.setValue("is_bid", bIsBid);
            record.setValue("receipt_id", QVariant::fromValue(lReceiptID));
            record.setValue("offer_id", QVariant::fromValue(lOfferID));
//            record.setValue("scale", QVariant::fromValue(lScale));
//            record.setValue("actual_price", QVariant::fromValue(lPrice));
//            record.setValue("actual_paid", QVariant::fromValue(lPayQuantity));
//            record.setValue("amount_purchased", QVariant::fromValue(lQuantity));
            record.setValue("timestamp",  QVariant::fromValue(tDate));
            record.setValue("notary_id", QString::fromStdString(strNotaryID));
            record.setValue("nym_id", QString::fromStdString(strNymID));
//            record.setValue("asset_id", QString::fromStdString(pTradeData->instrument_definition_id));
//            record.setValue("currency_id", QString::fromStdString(pTradeData->currency_id));
//            record.setValue("asset_acct_id", QString::fromStdString(pTradeData->asset_acct_id));
//            record.setValue("currency_acct_id", QString::fromStdString(pTradeData->currency_acct_id));

//            record.setValue("asset_receipt", QString::fromStdString(pTradeData->asset_receipt));
//            record.setValue("currency_receipt", QString::fromStdString(pTradeData->currency_receipt));
            record.setValue("final_receipt", qstrReceipt);

            qDebug() << "Kind of a weird situation -- a final receipt where there were "
                        "never any marketReceipts. (The offer expired without ever trading.) "
                        "Importing it here anyway to see if it causes any problems. "
                        "UPDATE: Maybe there WERE marketReceipts, but we just filtered it wrong, "
                        "or we got this finalReceipt first, possibly as a notice, and then get the others next.";

            qDebug() << QString("lOfferID for new insertion: %1").arg(lOfferID);

            pModel->insertRecord(0, record);
        }
        // ----------------------------
        if (bEditing)
        {
            if (pModel->submitAll())
            {
                if (pModel->database().commit())
                {
                    // Success.
                    return true;
                }
            }
            else
            {
                pModel->database().rollback();
                qDebug() << "Database Write Error" <<
                            "The database reported an error: " <<
                            pModel->lastError().text();
            }
        }
    }
    return false;
}


/// Returns true if it succeeds in creating the database record, including for the message body.
///
bool Moneychanger::low_level_AddMailToMsgArchive(
    opentxs::OTRecord& recordmt,
    MapOfPtrSetsOfStrings & mapOfSetsOfAlreadyImportedMsgs,
    MapOfConversationsByNym & mapOfConversationsByNym)
{
    // This function is low level because it assumes we already tried the lookup
    // table.
    // ------------------------------------------------------------------------------
    QPointer<ModelMessages> pModel = DBHandler::getInstance()->getMessageModel();
    bool bSuccessAddingMsg = false;
    QString qstrBody(""), threadItemId;

    if (pModel)
    {
        QString myNymID;
        if (!recordmt.GetNymID().empty())
            myNymID = QString::fromStdString(recordmt.GetNymID());
        // ---------------------------------
        QString myAddress;
        if (!recordmt.GetAddress().empty())
            myAddress = QString::fromStdString(recordmt.GetAddress());
//          myAddress = MTContactHandler::Encode(QString::fromStdString(recordmt.GetAddress()));
        // ---------------------------------
        QString senderNymID,    senderAddress,
                recipientNymID, recipientAddress;

        if (recordmt.IsOutgoing())
        {
            if (!recordmt.GetOtherNymID().empty())
                recipientNymID = QString::fromStdString(recordmt.GetOtherNymID());

            if (!recordmt.GetOtherAddress().empty())
                recipientAddress = QString::fromStdString(recordmt.GetOtherAddress());
//              recipientAddress = MTContactHandler::Encode(QString::fromStdString(recordmt.GetOtherAddress()));
        }
        else
        {
            if (!recordmt.GetOtherNymID().empty())
                senderNymID = QString::fromStdString(recordmt.GetOtherNymID());

            if (!recordmt.GetOtherAddress().empty())
                senderAddress = QString::fromStdString(recordmt.GetOtherAddress());
//              senderAddress = MTContactHandler::Encode(QString::fromStdString(recordmt.GetOtherAddress()));
        }
        // ---------------------------------
        QString notaryID, msgType, msgTypeDisplay;

        if (!recordmt.GetNotaryID().empty())
            notaryID = QString::fromStdString(recordmt.GetNotaryID());

        if (!recordmt.GetMsgType().empty())
            msgType = QString::fromStdString(recordmt.GetMsgType());
//          msgType = MTContactHandler::Encode(QString::fromStdString(recordmt.GetMsgType()));

        if (!recordmt.GetMsgTypeDisplay().empty())
            msgTypeDisplay = QString::fromStdString(recordmt.GetMsgTypeDisplay());
//          msgTypeDisplay = MTContactHandler::Encode(QString::fromStdString(recordmt.GetMsgTypeDisplay()));

        if (!recordmt.GetThreadItemId().empty())
            threadItemId =  QString::fromStdString(recordmt.GetThreadItemId());
        // ---------------------------------
        time64_t tDate = static_cast<time64_t>(ot_.API().Exec().StringToLong(recordmt.GetDate()));
        // ---------------------------------
        std::string str_mailDescription;
        recordmt.FormatMailSubject(str_mailDescription);
        QString mailDescription;

        if (!str_mailDescription.empty())
            mailDescription = MTContactHandler::Encode(QString::fromStdString(str_mailDescription));
        // ---------------------------------
        const int nFolder = recordmt.IsOutgoing() ? 0 : 1; // 0 for moneychanger's outbox, and 1 for inbox.
        // ---------------------------------
        pModel->database().transaction();
        // ---------------------------------
        // ADD THE RECORD HERE.
        //
        QSqlRecord record = pModel->record();

        record.setGenerated("message_id", true);

        if (!myNymID.isEmpty())
            record.setValue("my_nym_id", myNymID);
        if (!myAddress.isEmpty())
            record.setValue("my_address", myAddress);
        if (!senderNymID.isEmpty())
            record.setValue("sender_nym_id", senderNymID);
        if (!senderAddress.isEmpty())
            record.setValue("sender_address", senderAddress);
        if (!recipientNymID.isEmpty())
            record.setValue("recipient_nym_id", recipientNymID);
        if (!recipientAddress.isEmpty())
            record.setValue("recipient_address", recipientAddress);
        if (!msgType.isEmpty())
            record.setValue("method_type",  msgType);
        if (!msgTypeDisplay.isEmpty())
            record.setValue("method_type_display", msgTypeDisplay);
        if (!notaryID.isEmpty())
            record.setValue("notary_id", notaryID);
        record.setValue("timestamp", QVariant::fromValue(tDate));
        record.setValue("have_read", recordmt.IsOutgoing() ? 1 : 0);
        record.setValue("have_replied", 0);
        record.setValue("have_forwarded", 0);
        // ------------------------------------------------
        qstrBody = QString::fromStdString(recordmt.GetContents());

        const QString qstrSubject{"subject:"};
        const bool bHasContents = !qstrBody.isEmpty();
        const bool bHasSubject  = bHasContents && qstrBody.startsWith(qstrSubject, Qt::CaseInsensitive);
        const int nHasSubject = bHasSubject ? 1 : 0;
        record.setValue("has_subject", QVariant::fromValue(nHasSubject));

        if (!mailDescription.isEmpty())
            record.setValue("subject", mailDescription);
        // ------------------------------------------------
        record.setValue("folder", nFolder);

        if (!threadItemId.isEmpty())
            record.setValue("thread_item_id", threadItemId);
        // ------------------------------------------------
        record.setValue("archived", 0);

        pModel->insertRecord(0, record);
        // ---------------------------------
        if (pModel->submitAll())
        {
            if (pModel->database().commit())
            {
                // Success.
                bSuccessAddingMsg = true;
//                qstrBody = QString::fromStdString(recordmt.GetContents());
            }
        }
        else
        {
            pModel->database().rollback();
            qDebug() << "Database Write Error" <<
                       "The database reported an error: " <<
                       pModel->lastError().text();
        }
    }
    // ------------------------------------------------
    if (bSuccessAddingMsg)
    {
        if (!MTContactHandler::getInstance()->CreateMessageBody(qstrBody, threadItemId))
        {
            qDebug() << "AddMailToMsgArchive: Succeeded adding message record to database, but then failed writing message body.\n";
            return false;
        }
        // -------------------------------------------------------
        // Now that we've added it to our database, we need to delete it from Bitmessage.
        // (We don't need to delete normal mail messages because they are already deleted
        //  in our calling function).
        bool bSuccessDeletingSpecial = true;

        if (recordmt.IsSpecialMail())
        {
            bSuccessDeletingSpecial = false;

            int32_t     nMethodID   = recordmt.GetMethodID();
            std::string strMsgID    = recordmt.GetMsgID();
            std::string strMsgType  = recordmt.GetMsgType();

            if ((nMethodID > 0) && !strMsgID.empty())
            {
                // Get the comm string for this message ID.

                QString qstrConnect = MTContactHandler::getInstance()->GetMethodConnectStr(static_cast<int>(nMethodID));

                // Then find the NetworkModule based on the comm string:
                //
                if (!qstrConnect.isEmpty())
                {
                    NetworkModule * pModule = MTComms::find(qstrConnect.toStdString());

                    // Use net module to delete msg ID
                    //
                    if (NULL != pModule)
                    {
                        if (recordmt.IsOutgoing())
                        {
                            if (pModule->deleteOutMessage(strMsgID))
                                bSuccessDeletingSpecial = true;
                        }
                        else // incoming
                        {
                            if (pModule->deleteMessage(strMsgID))
                                bSuccessDeletingSpecial = true;
                        }
                    }
                }
            }
            if (!bSuccessDeletingSpecial)
                qDebug() << "AddMailToMsgArchive: FYI, FAILED while trying to delete special mail (probably bitmessage) from its native source.";
            else
                qDebug() << "AddMailToMsgArchive: FYI, SUCCESS deleting special mail (probably bitmessage) from its native source.";
        } // special mail
        // -----------------------------------
    }

    return bSuccessAddingMsg;
}

// Todo someday: Add a setting to the configuration so a user can choose whether or not to import Bitmessages.
// In which case they might never be added to the database here, or deleting from Bitmessage here (as they are now in both cases), unless that setting was set to true.
//
// When this function returns "true" it means "delete the record" (The mail message).
//
//resume
bool Moneychanger::AddMailToMsgArchive(
    opentxs::OTRecord& recordmt,
    MapOfPtrSetsOfStrings & mapOfSetsOfAlreadyImportedMsgs,
    MapOfConversationsByNym & mapOfConversationsByNym,
    SetNymThreadAndItem & setNewlyAddedNymThreadAndItem)
{
    // First let's make sure we're not re-importing a message we already imported before.
    //
    PtrSetOfStrings pAlreadyImportedIds; // For a given nymID and threadID. nullptr so far.

    std::string str_correct_thread_id_for_item;

    const std::string str_nym_id         = recordmt.GetNymID();
    const std::string str_thread_item_id = recordmt.GetThreadItemId();

    if ((false == recordmt.IsSpecialMail()) // Only for normal opentxs mail, not special mail integrations.
        && !str_nym_id.empty() && !str_thread_item_id.empty())
    {
        // Here we need to loop through the conversation Ids for this Nym,
        // and then search setNewlyAddedNymThreadAndItem to see if the relevant
        // three IDs appear on the set.
        // If it's newly-added, we'll import the message body as well. Otherwise we can
        // assume that we already did, on some previous refresh, and thus can skip any
        // re-importing of the message body and message table in general.
        //

//        typedef std::set<std::string> SetOfStrings;
//        typedef std::shared_ptr<SetOfStrings> PtrSetOfStrings;
//        typedef std::pair<std::string, std::string> PairOfStrings;
//        typedef std::map<PairOfStrings, PtrSetOfStrings> MapOfPtrSetsOfStrings;
//        typedef std::map<std::string, PtrSetOfStrings> MapOfConversationsByNym;
//
//        // NymID, (conversational) Thread Id, Thread Item Id.
//        typedef std::tuple<std::string, std::string, std::string> TupleNymThreadAndItem;
//        typedef std::shared_ptr<TupleNymThreadAndItem> PtrNymThreadAndItem;
//        typedef std::set<PtrNymThreadAndItem> SetNymThreadAndItem;

        MapOfConversationsByNym::iterator it_mapConversations = mapOfConversationsByNym.find(str_nym_id);

        if (mapOfConversationsByNym.end() != it_mapConversations)
        {
            PtrSetOfStrings & conversation_ids = it_mapConversations->second;

            if (conversation_ids)
            {
                for (auto& conversation_id : *conversation_ids)
                {
                    // str_nym_id, conversation_id, str_thread_item_id;
                    // setNewlyAddedNymThreadAndItem

                    const TupleNymThreadAndItem tupleNymThreadAndItem{str_nym_id, conversation_id, str_thread_item_id};

                    for (auto& ptrNymThreadAndItem : setNewlyAddedNymThreadAndItem)
                    {
                        // If the Nym Id, Thread Id, and Thread Item Id (as a tuple, for recordMT) were found on the
                        // set of newly-imported thread items, then let's import the message body (etc) to Moneychanger's
                        // internal DB.
                        //
                        if (tupleNymThreadAndItem == *ptrNymThreadAndItem)
                        {
                            qDebug() << "Importing conversation item...";

                            const bool bReturn = low_level_AddMailToMsgArchive(
                                        recordmt,
                                        mapOfSetsOfAlreadyImportedMsgs,
                                        mapOfConversationsByNym);
                            if (bReturn)
                            {
                                // once we've successfully imported it, can delete from this
                                // set, since we had the set for the express purpose of seeing
                                // which ones we need to import.
                                //
                                //setNewlyAddedNymThreadAndItem.erase(ptrNymThreadAndItem);
                                // commenting out for now in case this might cause any iteration issues.
                            }
                        }
                    }
                }
            }
        }
    }





//        for (auto& it_preimportedMap : mapOfSetsOfAlreadyImportedMsgs)
//        {
//            PairOfStrings   & pairNymIdAndThreadId = it_preimportedMap.first;
//            PtrSetOfStrings & pSetThreadItemIds    = it_preimportedMap.second;
//            // ---------------------
//            const std::string & str_current_nym_id    = pairNymIdAndThreadId.first;
//            const std::string & str_current_thread_id = pairNymIdAndThreadId.second;
//            // ---------------------
//            if (0 != str_current_nym_id.compare(str_nym_id)) {
//                continue;
//            }
//            if (!pSetThreadItemIds) {
//                continue;
//            }
//            // ---------------------
//            // We've got a map with the key being a pair of strings, and the value
//            // being a smart pointer to a set of strings.
//            // The key is a NymId/ThreadId (and we're looking to find out a ThreadID FYI)
//            // and the value is a pointer to a set of thread ITEM IDs for that Nym/Thread.
//            //
//            // By this point in the function, as we loop through the entire map, we grab
//            // the key (the two strings) first. If the NymID doesn't match the one on the
//            // record, we just skip it.
//            // That leads us to where we are now. We know the NymID matches, but we don't
//            // know yet if it's the right thread. The best thing we can do for now is to
//            // then grab it_preimported.second, which is a shared_ptr to a set of strings
//            // (thread Item IDs). We'll loop through those, and if the thread_item_id matches
//            // any of those, then we can confidentally say that str_current_thread_id is
//            // a valid thread id for this thread item. (We can also say with certainty, at
//            // that point, that it's already been imported...)
//            //
//            //  typedef std::set<std::string> SetOfStrings;
//            //  typedef std::shared_ptr<SetOfStrings> PtrSetOfStrings;
//            //  typedef std::pair<std::string, std::string> PairOfStrings;
//            //  typedef std::map<PairOfStrings, PtrSetOfStrings> MapOfPtrSetsOfStrings;
//            // ---------------------
//            bool bFoundThreadId = false;
//            for (auto& str_current_thread_item_id : *pSetThreadItemIds)
//            {
//                if (0 == str_current_thread_item_id.compare(str_thread_item_id)) // Match!
//                {
//                    bFoundThreadId = true;
//                    pAlreadyImportedIds = pSetThreadItemIds;
//                    str_correct_thread_id_for_item = str_current_thread_id;
//                    break;
//                }
//            } // for
//            // ---------------------
//            if (bFoundThreadId) // This means it's already been imported.
//            {
//                // Therefore no need to re-import it.
//            }
//            else // Otherwise it definitely HASN'T been yet imported
//            {

//            }
//            // ---------------------

//        } // for
//        // ----------------------------------------------

//        MapOfPtrSetsOfStrings::const_iterator it_map = mapOfSetsOfAlreadyImportedMsgs.find(PairOfStrings{str_nym_id, str_thread_id});
//    }
//    if (mapOfSetsOfAlreadyImportedMsgs.end() != it_map) // the set was already on the map
//    {
//        pAlreadyImportedIds = it_map->second();
//        bHaveGrabbedTheAlreadyImportedIds = true;
//        bHaveAlreadyImportedAnyIds = (pAlreadyImportedIds->size() > 0);
//    }
//    // ----------------------------------------------



//    // LOOP THROUGH THE NEWLY IMPORTED ONES AND LOW LEVEL ADD THEM!

////  NymID, (conversational) Thread Id, Thread Item Id.
////  typedef std::tuple<std::string, std::string, std::string> TupleNymThreadAndItem;


//    // ----------------------------------------------
//    bool Moneychanger::low_level_AddMailToMsgArchive(
//        opentxs::OTRecord& recordmt,
//        MapOfPtrSetsOfStrings & mapOfSetsOfAlreadyImportedMsgs,
//        MapOfConversationsByNym & mapOfConversationsByNym)


    return false;


}



// When OTRecord::AcceptIncomingInstrument is called, and the server reply is a success,
// it calls OTNameLookup::notifyOfSuccessfulNotarization so that the client GUI has the
// opportunity to make a database record that the transaction is complete. (That's what
// this function here is doing...)
//
// You see, if you deposit someone's incoming cheque, then HE is the one who gets the
// chequeReceipt (it's dropped into his inbox.) So you won't get any receipts in YOUR
// inbox. All YOU get is the server's success reply. (Directly.) If Moneychanger wasn't
// notified of that success reply, then it would have no way of knowing that the cheque
// had been deposited, and would still think it was "pending incoming."
//
// You might ask, "But wait a second, if the Moneychanger user did the cheque deposit,
// then Moneychanger should have seen it was a success." And that's true. But Moneychanger
// didn't do the cheque deposit. Rather, it asked OTRecord to do an AcceptIncomingInstrument.
// OTRecord did the actual deposit. That's why OTRecord has to notify Moneychanger of the success.
//
// You might also ask, "But we still have the pending incoming cheque, and if we know the
// deposit was a success, why not just move the incoming cheque to the record box, and then
// in the future we can think of it as a 'Deposited Cheque'?" The reason is, we only had the
// cheque when it was 'pending incoming'. But now that it's been deposited, we have the actual
// cheque deposit receipt that the server replied to us. So it's better to record that receipt
// since it PROVES the deposit, instead of merely inferring it based on the cheque being in
// Box B instead of Box A. Why not add the actual cheque DEPOSIT into the database, since we
// now have a copy of it? So that's what we do.
//
// One more thing: Since this function has "notification" in its name, you might think it's
// referring to a "notice" (a type of receipt that OT puts in the nymbox from time to time,
// to inform the user that something has happened.) But nope. That's not it. Here we use
// "Notification" because OTRecord has "notified" Moneychanger (by way of callback) that a
// transaction was successfully performed. The confusion is coincidental and unfortunate.
//
void Moneychanger::AddPaymentBasedOnNotification(const std::string & str_acct_id,
                                                 const std::string & p_nym_id,
                                                 const std::string & p_notary_id,
                                                 const std::string & p_txn_contents,
                                                 int64_t & lTransactionNum,
                                                 int64_t & lTransNumForDisplay)
{
    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();
    bool bSuccessAddingPmnt = false;
    QString qstrBody(""), qstrPendingBody("");
    QMap<QString, QVariant> mapFinalValues;

    bool bRecordAlreadyExisted = false;
    int  nPreExistingPaymentId = 0;

    if (pModel)
    {
        QString myNymID;
        if (!p_nym_id.empty())
            myNymID = QString::fromStdString(p_nym_id);
        // ---------------------------------
        QString myAcctID;
        if (!str_acct_id.empty())
            myAcctID = QString::fromStdString(str_acct_id);
        // ---------------------------------
        QString notaryID;
        if (!p_notary_id.empty())
            notaryID = QString::fromStdString(p_notary_id);
        // ---------------------------------
        int64_t & transNum        = lTransactionNum;
        int64_t & transNumDisplay = lTransNumForDisplay;

        int nPending = 1;
        int nCompleted = 1;
        // ---------------------------------
        const int nFolder = 1; // 0 for moneychanger's outbox, and 1 for inbox.
        // ---------------------------------
        // First let's see if a record already exists:
        //
        nPreExistingPaymentId = MTContactHandler::getInstance()->GetPaymentIdByTxnDisplayId(transNumDisplay, myNymID);
        bRecordAlreadyExisted = (nPreExistingPaymentId > 0);
        // ------------------------------------------
//        qDebug() << "Payment already existed: " << QString(bRecordAlreadyExisted ? "TRUE" : "FALSE");
//        qDebug() << "nPreExistingPaymentId: " << nPreExistingPaymentId;

        // We'll start by putting all the values into our map,
        // so we can then use that map when creating or updating
        // database records.
        //
        // if (bRecordAlreadyExisted) mapFinalValues.insert("payment_id", nPreExistingPaymentId);
        // Note: No need to insert this one.

        if (!myNymID.isEmpty()) mapFinalValues.insert("my_nym_id", myNymID);
        if (!myAcctID.isEmpty()) mapFinalValues.insert("my_acct_id", myAcctID);
        if (transNum > 0) mapFinalValues.insert("txn_id", QVariant::fromValue(transNum));
        if (transNumDisplay > 0) mapFinalValues.insert("txn_id_display", QVariant::fromValue(transNumDisplay));
        if (nPending > 0) mapFinalValues.insert("pending_found",   QVariant::fromValue(nPending));
        if (nCompleted > 0) mapFinalValues.insert("completed_found",   QVariant::fromValue(nCompleted));
        if (!notaryID.isEmpty()) mapFinalValues.insert("notary_id", notaryID);


//      qDebug() << "DEBUGGING AddPaymentBasedOnNotification. transNum: " << transNum << " transNumDisplay: " << transNumDisplay << "\n";

        if (!bRecordAlreadyExisted) mapFinalValues.insert("have_read", QVariant::fromValue(0));
        if (!bRecordAlreadyExisted) mapFinalValues.insert("have_replied", QVariant::fromValue(0));
        if (!bRecordAlreadyExisted) mapFinalValues.insert("have_forwarded", QVariant::fromValue(0));

        mapFinalValues.insert("folder", QVariant::fromValue(nFolder));
        // -------------------------------------------------
//        opentxs::OTPayment thePayment(p_txn_contents);

//        if (thePayment.IsValid() && thePayment.SetTempValues())
//        {
//            const std::string str_temp_type(thePayment.GetTypeString());
//            QString qstrType = QString::fromStdString(str_temp_type);

//            if (thePayment.IsNotice())
//                qstrPendingBody = QString::fromStdString(p_txn_contents);
//            else if ( thePayment.IsReceipt() )
//                qstrBody = QString::fromStdString(p_txn_contents);
//            else
//                qstrPendingBody = QString::fromStdString(p_txn_contents);
//        }
//        else

//        if (bRecordAlreadyExisted)
//            qstrBody = QString::fromStdString(p_txn_contents);
//        else
            qstrPendingBody = QString::fromStdString(p_txn_contents);
        // -------------------------------------------------
        if (bRecordAlreadyExisted)
        {
            // Success.
            if (MTContactHandler::getInstance()->UpdatePaymentRecord(nPreExistingPaymentId, mapFinalValues))
            {
                bSuccessAddingPmnt = true;

                pModel->select(); // Reset the model since we just inserted a new record.
                // AH WAIT!!! We don't want to do this for EVERY record inserted, do we?
                // Just the LAST ONE.
                // TODO!
            }
        }
        else // Record doesn't already exist in the database.
        {    // In that case we can use some code we already had:

            pModel->database().transaction();
            // ---------------------------------
            QSqlRecord record = pModel->record();
            record.setGenerated("payment_id", true);
            // ---------------------------------
            for (QMap<QString, QVariant>::iterator it_map = mapFinalValues.begin();
                 it_map != mapFinalValues.end(); ++it_map)
            {
                const QString  & qstrKey = it_map.key();
                const QVariant & qValue  = it_map.value();

                record.setValue(qstrKey, qValue);
            }

            pModel->insertRecord(0, record);
            // ---------------------------------
            if (pModel->submitAll())
            {
                // Success.
                if (pModel->database().commit())
                    bSuccessAddingPmnt = true;
            }
            else
            {
                pModel->database().rollback();
                qDebug() << "Database Write Error" <<
                           "The database reported an error: " <<
                           pModel->lastError().text();
            }
        } // Record didn't already exist. (Adding new.)
    } // if pModel
    // ------------------------------------------------
    if (bSuccessAddingPmnt)
    {
        qDebug() << "AddPaymentBasedOnNotification: Succeeded adding payment record to database.\n";

        if (bRecordAlreadyExisted)
        {
            if (!MTContactHandler::getInstance()->UpdatePaymentBody(nPreExistingPaymentId, qstrBody, qstrPendingBody))
            {
                qDebug() << "AddPaymentBasedOnNotification: ...but then failed updating payment body and/or pending body.\n";
            }
        }
        // ------------------------
        else
        {
            if (!MTContactHandler::getInstance()->CreatePaymentBody(qstrBody, qstrPendingBody))
            {
                qDebug() << "AddPaymentBasedOnNotification: ...but then failed creating payment body and/or pending body.\n";
            }
        }
    }
}


bool Moneychanger::AddPaymentBasedOnNotice(opentxs::OTRecord& recordmt, const bool bCanDeleteRecord/*=true*/)
{
    return AddPaymentToPmntArchive(recordmt, bCanDeleteRecord);
}

// Instead of the Payments UI, this is for the Agreements UI.
// Pretty similar though. Just make sure, if you call this function, that you know you are
// passing something related to a payment plan or smart contract. (Anything recurring except
// for market trades.)
// We'll start with paymentReceipts and finalReceipts and then go from there...
//
bool Moneychanger::AddAgreementRecord(opentxs::OTRecord& recordmt)
{
    ModelPayments::PaymentFlags flags = ModelPayments::NoFlags;

    QPointer<ModelAgreements> pAgreementModel = DBHandler::getInstance()->getAgreementModel();
    QPointer<ModelAgreementReceipts> pModel = DBHandler::getInstance()->getAgreementReceiptModel();
    bool bSuccessAddingReceipt = false;
    bool bSuccessUpdatingReceipt = false;
    QString qstrReceiptBody(""), qstrMemo("");
    QMap<QString, QVariant> mapFinalValues;

    bool bRecordAlreadyExisted = false;

    int nAgreementId         = 0;
    int nAgreementReceiptKey = 0;
    int nNewestKnownState    = 0;

    int64_t receiptNum = 0;
    time64_t tDate = 0;

    if (pModel)
    {
        // ---------------------------------
        if (recordmt.IsSpecialMail())           flags |= ModelPayments::IsSpecialMail;
        if (recordmt.IsPending())               flags |= ModelPayments::IsPending;
        if (recordmt.IsOutgoing())              flags |= ModelPayments::IsOutgoing;
        if (recordmt.IsRecord())                flags |= ModelPayments::IsRecord;
        if (recordmt.IsReceipt())               flags |= ModelPayments::IsReceipt;
        if (recordmt.IsFinalReceipt())          flags |= ModelPayments::IsFinalReceipt;
        if (recordmt.IsMail())                  flags |= ModelPayments::IsMail;
        if (recordmt.IsTransfer())              flags |= ModelPayments::IsTransfer;
        if (recordmt.IsCheque())                flags |= ModelPayments::IsCheque;
        if (recordmt.IsInvoice())               flags |= ModelPayments::IsInvoice;
        if (recordmt.IsVoucher())               flags |= ModelPayments::IsVoucher;
        if (recordmt.IsContract())              flags |= ModelPayments::IsContract;
        if (recordmt.IsPaymentPlan())           flags |= ModelPayments::IsPaymentPlan;
        if (recordmt.IsCash())                  flags |= ModelPayments::IsCash;
        if (recordmt.IsNotice())                flags |= ModelPayments::IsNotice;
        if (recordmt.IsExpired())               flags |= ModelPayments::IsExpired;
        if (recordmt.IsCanceled())              flags |= ModelPayments::IsCanceled;
        if (recordmt.CanDeleteRecord())         flags |= ModelPayments::CanDelete;
        if (recordmt.CanAcceptIncoming())       flags |= ModelPayments::CanAcceptIncoming;
        if (recordmt.CanDiscardIncoming())      flags |= ModelPayments::CanDiscardIncoming;
        if (recordmt.CanCancelOutgoing())       flags |= ModelPayments::CanCancelOutgoing;
        if (recordmt.CanDiscardOutgoingCash())  flags |= ModelPayments::CanDiscardOutgoingCash;
        // ---------------------------------
        if (recordmt.HasOriginType())  {
            if (recordmt.IsOriginTypePaymentPlan())   flags |= ModelPayments::OriginTypePaymentPlan;
            if (recordmt.IsOriginTypeSmartContract()) flags |= ModelPayments::OriginTypeSmartContract;
            if (recordmt.IsOriginTypeMarketOffer()) {
                qDebug() << __FUNCTION__
                         << ": Strange; Misguided attempt to import a market related receipt into the agreements table. (Failure.)";
                return false;
            }
        }
        // ---------------------------------
        QString myNymID;
        if (!recordmt.GetNymID().empty())
            myNymID = QString::fromStdString(recordmt.GetNymID());
        // ---------------------------------
        QString myAcctID;
        if (!recordmt.GetAccountID().empty())
            myAcctID = QString::fromStdString(recordmt.GetAccountID());
        // ---------------------------------
        QString instrumentType;
        if (!recordmt.GetInstrumentType().empty())
            instrumentType = QString::fromStdString(recordmt.GetInstrumentType());
        // ---------------------------------
        QString myAssetTypeID;
        if (!recordmt.GetInstrumentDefinitionID().empty())
            myAssetTypeID = QString::fromStdString(recordmt.GetInstrumentDefinitionID());
        // ---------------------------------
        QString myAddress;
        if (!recordmt.GetAddress().empty())
            myAddress = QString::fromStdString(recordmt.GetAddress());
//          myAddress = MTContactHandler::Encode(QString::fromStdString(recordmt.GetAddress()));
        // ---------------------------------
        QString senderNymID,    senderAccountID,    senderAddress,
                recipientNymID, recipientAccountID, recipientAddress;

        if (recordmt.IsOutgoing())
        {
            if (!recordmt.GetOtherNymID().empty())
                recipientNymID = QString::fromStdString(recordmt.GetOtherNymID());

            if (!recordmt.GetOtherAccountID().empty())
                recipientAccountID = QString::fromStdString(recordmt.GetOtherAccountID());

            if (!recordmt.GetOtherAddress().empty())
                recipientAddress = QString::fromStdString(recordmt.GetOtherAddress());
//              recipientAddress = MTContactHandler::Encode(QString::fromStdString(recordmt.GetOtherAddress()));
        }
        else
        {
            if (!recordmt.GetOtherNymID().empty())
                senderNymID = QString::fromStdString(recordmt.GetOtherNymID());

            if (!recordmt.GetOtherAccountID().empty())
                senderAccountID = QString::fromStdString(recordmt.GetOtherAccountID());

            if (!recordmt.GetOtherAddress().empty())
                senderAddress = QString::fromStdString(recordmt.GetOtherAddress());
//              senderAddress = MTContactHandler::Encode(QString::fromStdString(recordmt.GetOtherAddress()));
        }
        // ---------------------------------
        QString notaryID, msgType, msgTypeDisplay;

        if (!recordmt.GetNotaryID().empty())
            notaryID = QString::fromStdString(recordmt.GetNotaryID());

        if (!recordmt.GetMsgType().empty())
            msgType = QString::fromStdString(recordmt.GetMsgType());
//          msgType = MTContactHandler::Encode(QString::fromStdString(recordmt.GetMsgType()));

        if (!recordmt.GetMsgTypeDisplay().empty())
            msgTypeDisplay = QString::fromStdString(recordmt.GetMsgTypeDisplay());
//          msgTypeDisplay = MTContactHandler::Encode(QString::fromStdString(recordmt.GetMsgTypeDisplay()));
        // ---------------------------------
        tDate = static_cast<time64_t>(ot_.API().Exec().StringToLong(recordmt.GetDate()));

        // If it's a final Receipt, we want to get the closing number from it as the "receipt_id" which
        // is the Agreement table's version of the TransNum.
        // What's the difference? Well, they are basically identical, except for finalReceipts we have to use
        // the closing number instead of the "transaction number" since it's the only unique number on the
        // receipt. (A finalReceipt will have the same transaction number as other finalReceipts from the
        // same event.) So we're probably going to fix this soon by adding the "event_id" to OT.
        //
        int64_t transNum        = recordmt.GetTransactionNum();
        int64_t transNumDisplay = recordmt.GetTransNumForDisplay();
        // ---------------------------------------------------------------
        receiptNum = transNum;

        if (recordmt.IsFinalReceipt())
        {
            int64_t temp = 0;

            if (recordmt.GetClosingNum(temp) && (temp > 0))
                receiptNum = temp;
        }
        // What's going on here is, we normally use the Transaction Number on a receipt to identify it.
        // (The "trans num for display" might be the same on many receipts, but the actual txn_id isn't.)
        // But a problem arose, because some finalReceipts have the same transaction number. (Those that
        // were created from the same event.) We didn't mind this in the past since each finalReceipt went
        // to a different account, and the txnId wasn't signed out to the user anyway, but was owned by
        // the server.
        // Well, the chickens have come home to roost. We DO need a unique receipt number, and so for now,
        // we're using the "closing number" on the final receipt -- which is unique, at least for a given
        // notary -- and we're using the normal TransNum for all other receipts. This is what forms the
        // "receipt_id" that's used in the Agreement table.
        //
        // At some point soon we're going to fix this by adding an event_id to transactions, so we can tell
        // which ones came from the same event, without having to put the same transaction number on each.
        // Probably make a "receipt number" as well, on the OT side.
        // Why's that? Imagine that Alice does a single transaction #900, to exchange out of a basket currency
        // that has 4 asset types. She receives a server reply to her basket exchange, and then also receives
        // 4 finalReceipts in her 4 asset accounts. So that's 5 receipts total!
        // The server reply should be Transaction #900. (Which WAS signed out to Alice, though it's closed now.)
        // It should also have a receipt ID owned by the server. Say, number 54.
        // It should also have an event ID owned by the server. Say, number 600.
        // The 4 receipts will each have a unique receipt number owned by the server. Say, 55, 56, 57, and 58.
        // They should NOT have a transaction number, but they should have #900 as the "number of origin."
        // They should also all have the same event ID 600.
        // You might ask, why not just use the Transaction# 900 as the event ID? It's already the same on
        // all those receipts. The answer is because it's possible to have multiple events from the same
        // transaction. For example, market offer #900 might have 5 trades that occur. Each of those spawns
        // 4 marketReceipts. (20 in total.) So that's 5 different events for the same #900. Those events
        // might be 600, 601, 602, and 603. And there will be 20 receipt IDs as well. We'll get there.
        //
        // So this is a little confusing, but we'll simplify it soon, and for now, this works.
        // ---------------------------------------------------------------
        if (0 >= receiptNum)
        {
            qDebug() << __FUNCTION__ << ": Strange -- receiptNum is 0 or less. Should never happen. Failure.";
            return false;
        }
        // ---------------------------------------------------------------
        int64_t lAmount = ot_.API().Exec().StringToLong(recordmt.GetAmount());

//      qDebug() << "DEBUGGING! recordmt.GetAmount(): " << QString::fromStdString(recordmt.GetAmount())
//               << " lAmount: " << lAmount << "\n";
        // ---------------------------------
        std::string str_Name = recordmt.GetName();
        QString qstrName;
        if (!str_Name.empty())
            qstrName = MTContactHandler::Encode(QString::fromStdString(str_Name));
        // ---------------------------------
        std::string str_Memo = recordmt.HasMemo() ? recordmt.GetMemo() : "";
        if (!str_Memo.empty())
            qstrMemo = MTContactHandler::Encode(QString::fromStdString(str_Memo));
        // ---------------------------------
        std::string str_mailDescription;
        recordmt.FormatDescription(str_mailDescription);
        QString mailDescription;

        if (!str_mailDescription.empty())
            mailDescription = MTContactHandler::Encode(QString::fromStdString(str_mailDescription));
        // ---------------------------------
        const int nFolder = recordmt.IsOutgoing() ? 0 : 1; // 0 for moneychanger's outbox, and 1 for inbox.
        // ---------------------------------
        if (notaryID.isEmpty())
        {
            qDebug() << __FUNCTION__ << ": Strange: notaryID was empty. (And I needed it. Returning false. Sigh.)";
            return false;
        }
        // ----------------------------------------------------------
        int nAgreementFolder = 0;
        if (recordmt.IsPaymentPlan())   nAgreementFolder = 0; // Recurring Payment Plan
        else if (recordmt.IsContract()) nAgreementFolder = 1; // Smart Contract
//      else if (recordmt.IsEntity())   nAgreementFolder = 2; // Someday. (For digital corporations.)
        // ----------------------------------------------------------------------------------------
        // NOTE: This may be a paymentReceipt for a smart contract.
        // In which case recordmt.IsContract() returns FALSE!  (Though recordmt.IsReceipt() would return TRUE.)
        // In which case the above logic would set the folder wrong (It'd be set to the payment plan folder instead
        // of the smart contract folder.)
        // SO THEN, how does the below call to GetOrCreateLiveAgreementId work? Because it doesn't set the folder if
        // the record already exists.
        // It ONLY sets the folder when first creating the record. And it will get the folder correctly that time.
        // So I don't mind if the folder is wrong all the other times, because it's discarded in those cases anyway.
        // If it turns out later that this isn't good enough, I guess we can get more info from the record. But
        // this should work for now.
        // UPDATE: We now have the below flags which take the guesswork out of it.
        // ----------------------------------------------------------------------------------------
        else if (flags.testFlag(ModelPayments::OriginTypePaymentPlan))   nAgreementFolder = 0;
        else if (flags.testFlag(ModelPayments::OriginTypeSmartContract)) nAgreementFolder = 1;
        else {
            qDebug() << __FUNCTION__
                     << ": Strange; Trying to import an agreement receipt, but can't figure out which folder to put it in. (Failure.)";
            return false;
        }
        // ----------------------------------------------------------------------------------------
        // Let's see if we already have a "live agreement" in the Moneychanger DB's agreement table.
        // The Notary and the TxnDisplay (which is the same for all Nyms) are used to look up the
        // agreement, and nAgreementId is the unique key returned.
        // (Can't use TxnDisplay for unique key since there are multiple notaries out there...)
        //
        int nLastKnownState = 0;
        nAgreementId = MTContactHandler::getInstance()->GetOrCreateLiveAgreementId(transNumDisplay, notaryID, qstrMemo, nAgreementFolder, nLastKnownState);

        if (nAgreementId <= 0)
        {
            qDebug() << __FUNCTION__ << ": Failed in call to GetOrCreateLiveAgreementId (strange.)";
            return false;
        }
        // ----------------------------------------------------------
        // Let's see if a record already exists for this receipt:
        // UPDATE: Normally we search for an agreement receipt based on the "receiptNum"
        // but there is one case where it will fail: a cancellation notice for a pending incoming agreement
        // will have a different receiptNum than the pending incoming agreement does. Normally this is fine,
        // as we just collect ALL receipts on the Active Agreements UI.
        // But in the case of cancellation, we want to do a replacement. And we'll fail in the above-described
        // scenario.
        // Therefore, ONLY in the case of cancellation/activation, we pass the transNumDisplay in addition to the receiptNum,
        // here into DoesAgreementReceiptAlreadyExist. ONLY if it fails in the normal way (using the receiptNum)
        // will it then try the alternate search method of the transNumDisplay, since you can only cancel a pending
        // transaction BEFORE it has been activated. Therefore there could not be any other records stored except
        // for the pending incoming itself. So it should theoretically be the only one found, when searching based
        // on the transNumDisplay. (When in all other cases in this UI, there could be a plethora of other receipts
        // for the same transNumDisplay. Like all the paymentReceipts for that payment plan, etc.)
        //
        nAgreementReceiptKey = MTContactHandler::getInstance()->DoesAgreementReceiptAlreadyExist(nAgreementId, receiptNum, myNymID,
                                                                                                 recordmt.IsNotice() ? transNumDisplay : 0);
        bRecordAlreadyExisted = (nAgreementReceiptKey > 0);
        if (bRecordAlreadyExisted)
        {
            // If the agreement receipt key already exists, AND the current record is a NOTICE,
            // that means the current record should REPLACE or UPDATE the pre-existing version.
            // But in ALL OTHER CASES, we simply want to add the receipt, or return if it's already there.
            // This is the only case where we want to update an existing record.

            QString qstrTemp = QString("nAgreementId: %1  nAgreementReceiptKey: %2  receiptNum: %3  transNum: %4   transNumDisplay: %5\n Description: %6")
                    .arg(nAgreementId).arg(nAgreementReceiptKey).arg(receiptNum).arg(transNum).arg(transNumDisplay).arg(mailDescription);

            qDebug() << qstrTemp;

            QString tFinal = "false";
            QString tContract = "false";
            QString tPlan = "false";
            QString tNotice = "false";
            QString tExpired = "false";
            QString tCanceled = "false";
            QString tSuccess = "false";
            bool bIsSuccess = false;

            if (recordmt.IsFinalReceipt())          tFinal = "true";
            if (recordmt.IsContract())              tContract = "true";
            if (recordmt.IsPaymentPlan())           tPlan = "true";
            if (recordmt.IsNotice())                tNotice = "true";
            if (recordmt.IsExpired())               tExpired = "true";
            if (recordmt.IsCanceled())              tCanceled = "true";
            if (recordmt.HasSuccess(bIsSuccess))    tSuccess = bIsSuccess ? "true" : "false";

            qstrTemp = QString(" IsFinalReceipt: %1\n IsContract: %2 \n IsPaymentPlan: %3 \n IsNotice: %4 \n IsExpired: %5 \n IsCanceled: %6 \n IsSuccess: %7 ")
                    .arg(tFinal).arg(tContract).arg(tPlan).arg(tNotice).arg(tExpired).arg(tCanceled).arg(tSuccess);

            qDebug() << qstrTemp;

            if (!recordmt.IsNotice())
            {
                qDebug() << __FUNCTION__ << ": Skipping this agreement receipt since it's apparently already in the database. (Returning true, however, so the recordlist will dump it.)";
                return true;
            }
            // else if it IS a notice, we let it fall through, and the below code is smart
            // enough now to know when to add a new record, and when to update an existing one.
        }
        // ----------------------------------------------------------
        // We'll start by putting all the values into our map, so we can then use
        // that map when creating the DB record.
        //
        // if (bRecordAlreadyExisted) mapFinalValues.insert("agreement_receipt_key", nAgreementReceiptKey);
        // Note: No need to insert this one. It's auto-number.

        mapFinalValues.insert("agreement_id", QVariant::fromValue(nAgreementId));
        mapFinalValues.insert("receipt_id", QVariant::fromValue(receiptNum));

        if (tDate > 0) mapFinalValues.insert("timestamp", QVariant::fromValue(tDate));
        if (!bRecordAlreadyExisted) mapFinalValues.insert("have_read", QVariant::fromValue(recordmt.IsOutgoing() ? 1 : 0));
        if (transNumDisplay > 0) mapFinalValues.insert("txn_id_display", QVariant::fromValue(transNumDisplay));

        // (Someday event_id will go here.)

        if (!qstrMemo.isEmpty()) mapFinalValues.insert("memo", qstrMemo);
        if (!myAssetTypeID.isEmpty()) mapFinalValues.insert("my_asset_type_id", myAssetTypeID);
        if (!myNymID.isEmpty()) mapFinalValues.insert("my_nym_id", myNymID);
        if (!myAcctID.isEmpty()) mapFinalValues.insert("my_acct_id", myAcctID);
        if (!myAddress.isEmpty()) mapFinalValues.insert("my_address", myAddress);
        if (!senderNymID.isEmpty()) mapFinalValues.insert("sender_nym_id", senderNymID);
        if (!senderAccountID.isEmpty()) mapFinalValues.insert("sender_acct_id", senderAccountID);
        if (!senderAddress.isEmpty()) mapFinalValues.insert("sender_address", senderAddress);
        if (!recipientNymID.isEmpty()) mapFinalValues.insert("recipient_nym_id", recipientNymID);
        if (!recipientAccountID.isEmpty()) mapFinalValues.insert("recipient_acct_id", recipientAccountID);
        if (!recipientAddress.isEmpty()) mapFinalValues.insert("recipient_address", recipientAddress);
        if (lAmount != 0) mapFinalValues.insert("amount", QVariant::fromValue(lAmount));

        mapFinalValues.insert("folder", QVariant::fromValue(nFolder));

        if (!msgType.isEmpty()) mapFinalValues.insert("method_type", msgType);
        if (!msgTypeDisplay.isEmpty()) mapFinalValues.insert("method_type_display", msgTypeDisplay);

        if (!notaryID.isEmpty()) mapFinalValues.insert("notary_id", notaryID);
        if (!mailDescription.isEmpty()) mapFinalValues.insert("description", mailDescription);
        if (!qstrName.isEmpty()) mapFinalValues.insert("record_name", qstrName);
        if (!instrumentType.isEmpty()) mapFinalValues.insert("instrument_type", instrumentType);

        qint64 storedFlags = (qint64)flags;
        mapFinalValues.insert("flags", QVariant::fromValue(storedFlags));
        // -------------------------------------------------
        // 0 Error, 1 Outgoing, 2 Incoming, 3 Activated, 4 Paid, 5 Payment Failed, 6 Failed Activating, 7 Canceled, 8 Expired, 9 Completed, 10 Killed

        const int nError = 0;
        const int nOutgoing = 1;
        const int nIncoming = 2;
        const int nActivated = 3;
        const int nPaid = 4;
        const int nPaymentFailed = 5; // Live agreement here and above this point.
        const int nFailedActivating = 6; // Dead agreement here and below this point.
        const int nCanceled = 7;
        const int nExpired = 8;
        const int nNoLongerActive = 9;
        const int nKilled = 10;

        if (recordmt.IsCanceled())      nNewestKnownState = nCanceled;
        else if (recordmt.IsExpired())  nNewestKnownState = nExpired;
        else if (recordmt.IsFinalReceipt())
        {
            nNewestKnownState = nNoLongerActive;

            // NOTE: I think it's possible to be both a finalReceipt AND a Notice.
            // How? Because the Nym is sent a finalReceipt notice when a cron item
            // stops running. (Market offer, payment plan, or smart contract.)
            // There are four finalReceipt RECEIPTS, in the case of market offer,
            // one for each of the four asset accounts involved, but there is also
            // a NOTICE finalReceipt that is sent to each of the two NYMs involved.
            //
            // ===> That's why it's so important that this "if final receipt" block
            // happens ABOVE the "else if notice" block you see just below here.
            // Because otherwise, the logic might take a "no longer active" notice
            // (finalReceipt+IsNotice) and mistakenly mark it as "Activated".
            //
            // Todo: Make sure this "finalreceipt + isnotice" contains the REASON
            // or CAUSE of the cron item to cease functioning, whether it expired,
            // or was killed by another Nym, etc. That way we can import that information
            // here and display it for the user. We do have expired. And we do have
            // canceled, but that only shows if I canceled it before he activated
            // it in the first place. It doesn't show if someone killed it while it
            // was already running.
        }
        else if (recordmt.IsNotice())
        {
            // IMPORTANT: Should ASSERT !isFinalReceipt() here, so the logic is preserved
            // that the above finalReceipt block comes just BEFORE this Notice block.

            bool bIsSuccess = false;

            if (recordmt.HasSuccess(bIsSuccess))
            {
                if (bIsSuccess) nNewestKnownState = nActivated;
                else            nNewestKnownState = nFailedActivating;
            }
        }
        else if (recordmt.IsReceipt())
        {
            bool bIsSuccess = false;

            if (recordmt.HasSuccess(bIsSuccess))
            {
                if (bIsSuccess) nNewestKnownState = nPaid;
                else            nNewestKnownState = nPaymentFailed;
            }
        }
        else if (recordmt.IsPending())
        {
            if (recordmt.IsOutgoing()) nNewestKnownState = nOutgoing;
            else                       nNewestKnownState = nIncoming;
        }
        // else if (blah blah blah)    nNewestKnownState = 8; // Killed
        // TODO: What about if it's killed? Should be something on the finalReceipt that tells me whether it died naturally or was killed.
        // (There probably is, or there should be.)
        // ----------------------------------
        // If we already processed a finalReceipt previously, and now we're processing its related paymentReceipts,
        // (since they are coming from the recordBox and may not be in the order originally received...) then we know
        // the actual final state already. So we don't want to go backwards and set it back to "paid" again, in that case,
        // instead of keeping it at "finished." The below logic accomplishes this.
        //
        // 0 Error, 1 Outgoing, 2 Incoming, 3 Activated, 4 Paid, 5 Payment Failed, 6 Failed Activating, 7 Canceled, 8 Expired, 9 Completed, 10 Killed

        if (nNewestKnownState != nError)
        {
            if (nLastKnownState >= nFailedActivating)
                nNewestKnownState = nLastKnownState;
            else if (nLastKnownState >= nPaid && nNewestKnownState <= nActivated)
                nNewestKnownState = nLastKnownState;
            else if (nLastKnownState >= nActivated && nNewestKnownState < nActivated)
                nNewestKnownState = nLastKnownState;
        }
        // -------------------------------------------------
//      qDebug() << "DEBUGGING AddAgreementRecord. " << (recordmt.IsOutgoing() ? "OUT" : "IN") << ". receiptNum: " << receiptNum << " transNumDisplay: " << transNumDisplay << "\n";
        // -------------------------------------------------
        if (bRecordAlreadyExisted)
        {
            // With agreement receipts, in most cases, we either insert a new one
            // or just return since it's already there.
            // But there's this ONE case where we DO need to update an existing record.
            // It's when Payment Plan 757 comes in, and then later Notice 757 comes in
            // (like a cancellation notice.) So in THAT case, we need to update the "pending"
            // record to change it to "canceled."
            //
            if (MTContactHandler::getInstance()->UpdateAgreementReceiptRecord(nAgreementReceiptKey, mapFinalValues))
            {
                qstrReceiptBody = QString::fromStdString(recordmt.GetContents());
                bSuccessUpdatingReceipt = true;

                pModel->select(); // Reset the model since we just inserted a new record.
                // AH WAIT!!! We don't want to do this for EVERY record inserted, do we?
                // Just the LAST ONE.
                // TODO!
            }
        }
        else // Record didn't already exist in the DB, so we add it.
        { // AgreementReceipt table being updated here. Vs the Live Agreements, updated status below this block.
            pModel->database().transaction();
            // ---------------------------------
            QSqlRecord record = pModel->record();
            record.setGenerated("agreement_receipt_key", true);
            // ---------------------------------
            for (QMap<QString, QVariant>::iterator it_map = mapFinalValues.begin();
                 it_map != mapFinalValues.end(); ++it_map)
            {
                const QString  & qstrKey = it_map.key();
                const QVariant & qValue  = it_map.value();

                record.setValue(qstrKey, qValue);
            }

            pModel->insertRecord(0, record);
            // ---------------------------------
            if (pModel->submitAll())
            {
                // Success.
                if (pModel->database().commit())
                {
                    bSuccessAddingReceipt = true;
                    nAgreementReceiptKey = DBHandler::getInstance()->queryInt("SELECT last_insert_rowid() from `agreement_receipt`", 0, 0);
                    qstrReceiptBody = QString::fromStdString(recordmt.GetContents());
                }
            }
            else
            {
                pModel->database().rollback();
                qDebug() << "Database Write Error"
                         << "The database reported an error: "
                         << pModel->lastError().text();
            }
        }
    } // if pModel
    // ------------------------------------------------
    if (bSuccessAddingReceipt || bSuccessUpdatingReceipt)
    {
        if (nAgreementReceiptKey <= 0)
        {
            qDebug() << "AddAgreementRecord: Supposedly succeeded adding agreement receipt to database, but the resulting key is wrong! (Failure.)\n";
            return false;
        }
        else
            qDebug() << "AddAgreementRecord: Succeeded adding agreement receipt to database.\n";
        // -------------------------------------------
        if (!MTContactHandler::getInstance()->UpdateLiveAgreementRecord(nAgreementId, receiptNum, nNewestKnownState, tDate, qstrMemo))
            qDebug() << "AddAgreementRecord: Strange -- just failed trying to update a live agreement record.\n";
        else
            pAgreementModel->select();
        // -------------------------------------------
        if (bSuccessUpdatingReceipt &&
            !MTContactHandler::getInstance()->UpdateAgreementReceiptBody(nAgreementReceiptKey, qstrReceiptBody))
        {
            qDebug() << "AddAgreementRecord: Updated record, but then failed updating agreement receipt body.\n";
            return false;
        }
        else if (bSuccessAddingReceipt &&
            !MTContactHandler::getInstance()->CreateAgreementReceiptBody(nAgreementReceiptKey, qstrReceiptBody))
        {
            qDebug() << "AddAgreementRecord: ...but then failed creating agreement receipt body.\n";
            return false;
        }
    }
    // ------------------------------------------------
    return bSuccessAddingReceipt;
}


// ------------------------------


// Adds or updates.
// The payment archive stores up to multiple receipts per record.
// The primary key is the "display txn ID"
//
bool Moneychanger::AddPaymentToPmntArchive(opentxs::OTRecord& recordmt, const bool bCanDeleteRecord/*=true*/)
{
    ModelPayments::PaymentFlags flags = ModelPayments::NoFlags;

    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();
    bool bSuccessAddingPmnt = false;
    QString qstrBody(""), qstrPendingBody("");
    QMap<QString, QVariant> mapFinalValues;

    bool bRecordAlreadyExisted = false;
    int  nPreExistingPaymentId = 0;

    if (pModel)
    {
        // ---------------------------------
        if (recordmt.IsSpecialMail())           flags |= ModelPayments::IsSpecialMail;
        if (recordmt.IsPending())               flags |= ModelPayments::IsPending;
        if (recordmt.IsOutgoing())              flags |= ModelPayments::IsOutgoing;
        if (recordmt.IsRecord())                flags |= ModelPayments::IsRecord;
        if (recordmt.IsReceipt())               flags |= ModelPayments::IsReceipt;
        if (recordmt.IsFinalReceipt())          flags |= ModelPayments::IsFinalReceipt;
        if (recordmt.IsMail())                  flags |= ModelPayments::IsMail;
        if (recordmt.IsTransfer())              flags |= ModelPayments::IsTransfer;
        if (recordmt.IsCheque())                flags |= ModelPayments::IsCheque;
        if (recordmt.IsInvoice())               flags |= ModelPayments::IsInvoice;
        if (recordmt.IsVoucher())               flags |= ModelPayments::IsVoucher;
        if (recordmt.IsContract())              flags |= ModelPayments::IsContract;
        if (recordmt.IsPaymentPlan())           flags |= ModelPayments::IsPaymentPlan;
        if (recordmt.IsCash())                  flags |= ModelPayments::IsCash;
        if (recordmt.IsNotice())                flags |= ModelPayments::IsNotice;
        if (recordmt.IsExpired())               flags |= ModelPayments::IsExpired;
        if (recordmt.IsCanceled())              flags |= ModelPayments::IsCanceled;
        if (recordmt.CanDeleteRecord())         flags |= ModelPayments::CanDelete;
        if (recordmt.CanAcceptIncoming())       flags |= ModelPayments::CanAcceptIncoming;
        if (recordmt.CanDiscardIncoming())      flags |= ModelPayments::CanDiscardIncoming;
        if (recordmt.CanCancelOutgoing())       flags |= ModelPayments::CanCancelOutgoing;
        if (recordmt.CanDiscardOutgoingCash())  flags |= ModelPayments::CanDiscardOutgoingCash;
        // ---------------------------------
        QString myNymID;
        if (!recordmt.GetNymID().empty())
            myNymID = QString::fromStdString(recordmt.GetNymID());
        // ---------------------------------
        QString myAcctID;
        if (!recordmt.GetAccountID().empty())
            myAcctID = QString::fromStdString(recordmt.GetAccountID());
        // ---------------------------------
        QString instrumentType;
        if (!recordmt.GetInstrumentType().empty())
            instrumentType = QString::fromStdString(recordmt.GetInstrumentType());
        // ---------------------------------
        QString myAssetTypeID;
        if (!recordmt.GetInstrumentDefinitionID().empty())
            myAssetTypeID = QString::fromStdString(recordmt.GetInstrumentDefinitionID());
        // ---------------------------------
        QString myAddress;
        if (!recordmt.GetAddress().empty())
            myAddress = QString::fromStdString(recordmt.GetAddress());
//          myAddress = MTContactHandler::Encode(QString::fromStdString(recordmt.GetAddress()));
        // ---------------------------------
        QString senderNymID,    senderAccountID,    senderAddress,
                recipientNymID, recipientAccountID, recipientAddress;

        if (recordmt.IsOutgoing())
        {
            if (!recordmt.GetOtherNymID().empty())
                recipientNymID = QString::fromStdString(recordmt.GetOtherNymID());

            if (!recordmt.GetOtherAccountID().empty())
                recipientAccountID = QString::fromStdString(recordmt.GetOtherAccountID());

            if (!recordmt.GetOtherAddress().empty())
                recipientAddress = QString::fromStdString(recordmt.GetOtherAddress());
//              recipientAddress = MTContactHandler::Encode(QString::fromStdString(recordmt.GetOtherAddress()));
        }
        else
        {
            if (!recordmt.GetOtherNymID().empty())
                senderNymID = QString::fromStdString(recordmt.GetOtherNymID());

            if (!recordmt.GetOtherAccountID().empty())
                senderAccountID = QString::fromStdString(recordmt.GetOtherAccountID());

            if (!recordmt.GetOtherAddress().empty())
                senderAddress = QString::fromStdString(recordmt.GetOtherAddress());
//              senderAddress = MTContactHandler::Encode(QString::fromStdString(recordmt.GetOtherAddress()));
        }
        // ---------------------------------
        QString notaryID, msgType, msgTypeDisplay;

        if (!recordmt.GetNotaryID().empty())
            notaryID = QString::fromStdString(recordmt.GetNotaryID());

        if (!recordmt.GetMsgType().empty())
            msgType = QString::fromStdString(recordmt.GetMsgType());
//          msgType = MTContactHandler::Encode(QString::fromStdString(recordmt.GetMsgType()));

        if (!recordmt.GetMsgTypeDisplay().empty())
            msgTypeDisplay = QString::fromStdString(recordmt.GetMsgTypeDisplay());
//          msgTypeDisplay = MTContactHandler::Encode(QString::fromStdString(recordmt.GetMsgTypeDisplay()));
        // ---------------------------------
        time64_t tDate = static_cast<time64_t>(ot_.API().Exec().StringToLong(recordmt.GetDate()));

        int64_t transNum        = recordmt.GetTransactionNum();
        int64_t transNumDisplay = recordmt.GetTransNumForDisplay();

        int64_t lAmount = ot_.API().Exec().StringToLong(recordmt.GetAmount());

//      qDebug() << "DEBUGGING! recordmt.GetAmount(): " << QString::fromStdString(recordmt.GetAmount())
//               << " lAmount: " << lAmount << "\n";

        int nPending   = recordmt.CanDeleteRecord() ? 0 : 1;
        int nCompleted = recordmt.CanDeleteRecord() ? 1 : 0;
        // ---------------------------------
        std::string str_Name = recordmt.GetName();
        QString qstrName;
        if (!str_Name.empty())
            qstrName = MTContactHandler::Encode(QString::fromStdString(str_Name));
        // ---------------------------------
        std::string str_Memo = recordmt.HasMemo() ? recordmt.GetMemo() : "";
        QString qstrMemo;
        if (!str_Memo.empty())
            qstrMemo = MTContactHandler::Encode(QString::fromStdString(str_Memo));
        // ---------------------------------
        std::string str_mailDescription;
        recordmt.FormatDescription(str_mailDescription);
        QString mailDescription;

        if (!str_mailDescription.empty())
            mailDescription = MTContactHandler::Encode(QString::fromStdString(str_mailDescription));
        // ---------------------------------
        const int nFolder = recordmt.IsOutgoing() ? 0 : 1; // 0 for moneychanger's outbox, and 1 for inbox.
        // ---------------------------------
        // First let's see if a record already exists:
        //
        nPreExistingPaymentId = MTContactHandler::getInstance()->GetPaymentIdByTxnDisplayId(transNumDisplay, myNymID);
        bRecordAlreadyExisted = (nPreExistingPaymentId > 0);
        // ------------------------------------------
        // We'll start by putting all the values into our map,
        // so we can then use that map when creating or updating
        // database records.
        //
        // if (bRecordAlreadyExisted) mapFinalValues.insert("payment_id", nPreExistingPaymentId);
        // Note: No need to insert this one.

        if (!myNymID.isEmpty()) mapFinalValues.insert("my_nym_id", myNymID);
        if (!myAcctID.isEmpty()) mapFinalValues.insert("my_acct_id", myAcctID);
        if (!myAssetTypeID.isEmpty()) mapFinalValues.insert("my_asset_type_id", myAssetTypeID);
        if (!myAddress.isEmpty()) mapFinalValues.insert("my_address", myAddress);
        if (!senderNymID.isEmpty()) mapFinalValues.insert("sender_nym_id", senderNymID);
        if (!senderAccountID.isEmpty()) mapFinalValues.insert("sender_acct_id", senderAccountID);
        if (!senderAddress.isEmpty()) mapFinalValues.insert("sender_address", senderAddress);
        if (!recipientNymID.isEmpty()) mapFinalValues.insert("recipient_nym_id", recipientNymID);
        if (!recipientAccountID.isEmpty()) mapFinalValues.insert("recipient_acct_id", recipientAccountID);
        if (!recipientAddress.isEmpty()) mapFinalValues.insert("recipient_address", recipientAddress);

        if (transNum > 0)        mapFinalValues.insert("txn_id", QVariant::fromValue(transNum));
        if (transNumDisplay > 0) mapFinalValues.insert("txn_id_display", QVariant::fromValue(transNumDisplay));

//      qDebug() << "DEBUGGING AddPaymentToPmntArchive. " << (recordmt.IsOutgoing() ? "OUT" : "IN") << ". transNum: " << transNum << " transNumDisplay: " << transNumDisplay << "\n";

        // I receive a cheque. This is the incoming cheque I'm receiving.

        if (lAmount != 0)        mapFinalValues.insert("amount", QVariant::fromValue(lAmount));
        if (tDate > 0)           mapFinalValues.insert("timestamp", QVariant::fromValue(tDate));

        if (nPending   > 0) mapFinalValues.insert("pending_found",   QVariant::fromValue(nPending));
        if (nCompleted > 0) mapFinalValues.insert("completed_found", QVariant::fromValue(nCompleted));

        if (!msgType.isEmpty()) mapFinalValues.insert("method_type", msgType);
        if (!msgTypeDisplay.isEmpty()) mapFinalValues.insert("method_type_display", msgTypeDisplay);
        if (!notaryID.isEmpty()) mapFinalValues.insert("notary_id", notaryID);
        if (!qstrMemo.isEmpty()) mapFinalValues.insert("memo", qstrMemo);
        if (!mailDescription.isEmpty()) mapFinalValues.insert("description", mailDescription);
        if (!qstrName.isEmpty()) mapFinalValues.insert("record_name", qstrName);
        if (!instrumentType.isEmpty()) mapFinalValues.insert("instrument_type", instrumentType);

        if (!bRecordAlreadyExisted) mapFinalValues.insert("have_read", QVariant::fromValue(recordmt.IsOutgoing() ? 1 : 0));
        if (!bRecordAlreadyExisted) mapFinalValues.insert("have_replied", QVariant::fromValue(0));
        if (!bRecordAlreadyExisted) mapFinalValues.insert("have_forwarded", QVariant::fromValue(0));

        qint64 storedFlags = (qint64)flags;
        mapFinalValues.insert("folder", QVariant::fromValue(nFolder));
        mapFinalValues.insert("flags", QVariant::fromValue(storedFlags));
        // -------------------------------------------------
        bool bAddAsPending = false;

        if (!bCanDeleteRecord) bAddAsPending = true;
        else                   bAddAsPending = false;
        // -------------------------------------------------
        if (  (recordmt.IsNotice()) )
        {
            bAddAsPending = true;
        }
        // -------------------------------------------------
        if (
              (recordmt.IsReceipt())
           )
        {
            bAddAsPending = false;
        }
        // -------------------------------------------------
        if (bAddAsPending) qstrPendingBody = QString::fromStdString(recordmt.GetContents());
        else               qstrBody        = QString::fromStdString(recordmt.GetContents());
        // -------------------------------------------------
        if (bRecordAlreadyExisted)
        {
            // Success.
            if (MTContactHandler::getInstance()->UpdatePaymentRecord(nPreExistingPaymentId, mapFinalValues))
            {
                bSuccessAddingPmnt = true;

                pModel->select(); // Reset the model since we just inserted a new record.
                // AH WAIT!!! We don't want to do this for EVERY record inserted, do we?
                // Just the LAST ONE.
                // TODO!
            }
        }
        else // Record doesn't already exist in the database.
        {    // In that case we can use some code we already had:
            pModel->database().transaction();
            // ---------------------------------
            QSqlRecord record = pModel->record();
            record.setGenerated("payment_id", true);
            // ---------------------------------
            for (QMap<QString, QVariant>::iterator it_map = mapFinalValues.begin();
                 it_map != mapFinalValues.end(); ++it_map)
            {
                const QString  & qstrKey = it_map.key();
                const QVariant & qValue  = it_map.value();

                record.setValue(qstrKey, qValue);
            }

            pModel->insertRecord(0, record);
            // ---------------------------------
            if (pModel->submitAll())
            {
                // Success.
                if (pModel->database().commit())
                    bSuccessAddingPmnt = true;
            }
            else
            {
                pModel->database().rollback();
                qDebug() << "Database Write Error" <<
                           "The database reported an error: " <<
                           pModel->lastError().text();
            }
        } // Record didn't already exist. (Adding new.)
    } // if pModel
    // ------------------------------------------------
    if (bSuccessAddingPmnt)
    {
        qDebug() << "AddPaymentToPmntArchive: Succeeded adding payment record to database.\n";

        if (bRecordAlreadyExisted)
        {
            if (!MTContactHandler::getInstance()->UpdatePaymentBody(nPreExistingPaymentId, qstrBody, qstrPendingBody))
            {
                qDebug() << "AddPaymentToPmntArchive: ...but then failed updating payment body and/or pending body.\n";
                return false;
            }
        }
        // ------------------------
        else
        {
            if (!MTContactHandler::getInstance()->CreatePaymentBody(qstrBody, qstrPendingBody))
            {
                qDebug() << "AddPaymentToPmntArchive: ...but then failed creating payment body and/or pending body.\n";
                return false;
            }
        }
    }
    // ------------------------------------------------
    return bSuccessAddingPmnt;
}


//        message StorageThreadItem {
//            optional uint32 version = 1;
//            optional string id = 2;
//            optional uint64 index = 3;
//            optional uint64 time = 4;
//            optional uint32 box = 5;
//            optional string account = 6;
//            optional bool unread = 7;
//        }
//        enum class StorageBox : std::uint8_t {
//            SENTPEERREQUEST = 0,
//            INCOMINGPEERREQUEST = 1,
//            SENTPEERREPLY = 2,
//            INCOMINGPEERREPLY = 3,
//            FINISHEDPEERREQUEST = 4,
//            FINISHEDPEERREPLY = 5,
//            PROCESSEDPEERREQUEST = 6,
//            PROCESSEDPEERREPLY = 7,
//            MAILINBOX = 8,
//            MAILOUTBOX = 9
//        };


// Loop through conversations owned by this Nym and import copies to
// local DB for the GUI to use. Re-import if necessary since they
// change over time.
//
void Moneychanger::ImportConversationsForNym(const std::string & str_nym_id,
                                             MapOfPtrSetsOfStrings & mapOfSetsOfAlreadyImportedMsgs,
                                             SetNymThreadAndItem & setNewlyAddedNymThreadAndItem) // output
{
    OT_ASSERT(!str_nym_id.empty());
    // ----------------------------------------------
    opentxs::ObjectList threadList = ot_.Activity().Threads(opentxs::Identifier{str_nym_id});
    opentxs::ObjectList::const_iterator ci = threadList.begin();

    while (threadList.end() != ci)
    {
        const std::pair<std::string, std::string> & threadInfo = *ci;

        const std::string & str_thread_id   = threadInfo.first;
        const std::string & str_thread_name = threadInfo.second;
        // ----------------------------------------------
        std::shared_ptr<opentxs::proto::StorageThread> thread;
        ot_.DB().Load(str_nym_id, str_thread_id, thread);
        // ----------------------------------------------
        if (!thread) {
            ++ci;
            continue;
        }
        // ----------------------------------------------
        if (false == ImportConversationForNym(str_nym_id, str_thread_id, str_thread_name, thread, mapOfSetsOfAlreadyImportedMsgs, setNewlyAddedNymThreadAndItem)) {
            ++ci;
            continue;
        }
        // ----------------------------------------------

        // Anything else we might want to do can be put right here.
        // Basically, for those times when you need to do something
        // immediately after importing a conversation.

        // ----------------------------------------------
        ++ci;
    }
}

bool Moneychanger::ImportConversationForNym(const std::string & str_nym_id,
                                            const std::string & str_thread_id,
                                            const std::string & str_thread_name,
                                            std::shared_ptr<opentxs::proto::StorageThread> & thread,
                                            MapOfPtrSetsOfStrings & mapOfSetsOfAlreadyImportedMsgs,
                                            SetNymThreadAndItem & setNewlyAddedNymThreadAndItem) // output
{
    if (!thread)
        return false;
    // ----------------------------------------------
    if (thread->participant_size() <= 0)
        return false;
    // ----------------------------------------------
    OT_ASSERT(0 == str_thread_id.compare(thread->id()));

    // Note: in a conversation with a single contact, str_thread_id itself
    // is also that ContactId, so I don't have to grab the first participant
    // for his ID.

    // If it's a group conversation, I ALSOS don't need the participants' IDs
    // (in this case) since we're here mainly to import the conversation ID
    // itself, which again would be str_thread_id.
    //
    // I think the only time I care about a specific participant's ID is when
    // I'm trying to message that guy 1-on-1, and I just want to see if there's
    // a conversation already there between us (on the GUI) before I otherwise
    // create one and add it to the tree. But it seems, again, I can just use
    // the contact ID (which is also the thread id in that case) to lookup the
    // conversation on the tree.
    //
    //const std::string str_participant_contact_id = thread->participant(0);

    const QString qstrMyNymId  = QString::fromStdString(str_nym_id);
    const QString qstrThreadId = QString::fromStdString(str_thread_id);
    QString qstrThreadName     = QString::fromStdString(str_thread_name);
    // ----------------------------------------------
    if (qstrThreadName.isEmpty()) {
        qstrThreadName = QString("(%1)").arg(tr("Conversation name was empty"));
    }
    // ----------------------------------------------
//  qstrMyNymId, qstrThreadId, qstrThreadName;

    // If some of this seems like I did it weird, it's for lazy evaluation / optimization
    // purposes, since the import can be rather slow.

    // ----------------------------------------------
    // So we don't continually re-import thread items that we already imported before.
    //
    PtrSetOfStrings  pAlreadyImportedIds; // nullptr so far.
    bool bHaveAlreadyImportedAnyIds = false;
    bool bHaveGrabbedTheAlreadyImportedIds = false;
    // ----------------------------------------------
    // Loop through messages in this conversation, create or update each conversation_msg
    // record based on the thread item it's importing from. Assume we may re-import the
    // same records, and we don't want duplicates.
    //
    bool bConversationExistsInDB = false; // so far. (We'll check in a lazy way).

    for (const auto & item : thread->item()) //opentxs::proto::StorageThreadItem & item
    {
        //  QString create_conversation_msg_table = "CREATE TABLE IF NOT EXISTS conversation_msg"
        //         "(conversation_id TEXT,"
        //         " my_nym_id TEXT,"
        //         " thread_item_id TEXT," // Note: this field is also in message_body table
        //         " timestamp INTEGER,"
        //         " box INTEGER,"
        //         " account TEXT,"
        //         " unread INTEGER,"
        //         " PRIMARY KEY (conversation_id, my_nym_id, thread_item_id)"
        //         ")";
        // --------------------------------------------
        const std::string thread_item_id = item.has_id() ? item.id() : "" ;
        const std::string thread_item_account (item.has_account() ? item.account() : "");

        if (thread_item_id.empty())
        {
            qDebug() << "Found a conversational thread item with a blank ID field. Strange, probably should not ever be happening. (Skipping it).";
            continue;
        }
        // --------------------------------------------
//      const QString qstrMyNymId  = QString::fromStdString(str_nym_id);
//      const QString qstrThreadId = QString::fromStdString(str_thread_id);
        const QString qstrThreadItemId = QString::fromStdString(thread_item_id);
        const QString qstrThreadItemAccountId = QString::fromStdString(thread_item_account);
        // --------------------------------------------
        const int thread_item_box = item.has_box() ? item.box() : 0;
        const time64_t thread_item_timestamp  = item.has_time() ? item.time() : 0;
        const bool thread_item_unread = item.has_unread() ? item.unread() : false;
        // --------------------------------------------
        // By this point we know there's definitely an item, we need to either import it
        // or ignore it (if we already imported it in the past).
        // We'll also go ahead and grab the IDs from our local DB of the ones we've already imported.
        // We avoided doing that up until now for optimization reasons.
        //
        if (!bHaveGrabbedTheAlreadyImportedIds)
        {
            //  typedef std::set<std::string> SetOfStrings;
            //  typedef std::shared_ptr<SetOfStrings> PtrSetOfStrings;
            //  typedef std::pair<std::string, std::string> PairOfStrings;
            //  typedef std::map<PairOfStrings, PtrSetOfStrings> MapOfPtrSetsOfStrings;

            MapOfPtrSetsOfStrings::iterator it_map = mapOfSetsOfAlreadyImportedMsgs.find(PairOfStrings{str_nym_id, str_thread_id}); // Find the conversation on our lookup table.

            if (mapOfSetsOfAlreadyImportedMsgs.end() != it_map) // the set was already on the map (conversation already on map of conversations by nym/threadIDs.)
            {
                bHaveGrabbedTheAlreadyImportedIds = true;
                pAlreadyImportedIds = it_map->second;
            }
            else // This is the real part. Above block is just the optimization.
            {
                SetOfStrings * pSet = MTContactHandler::getInstance()->selectThreadItemIdsForNymAndConversation(qstrMyNymId, qstrThreadId);

                if (nullptr != pSet) // We have some thread items for the given pair<nym/thread_ID>, which are already imported into our local DB.
                {
                    pAlreadyImportedIds.reset( pSet );
                }
                else // We had none already imported for this nymId and ThreadID, but we'll go ahead and instantiate the shared_ptr (with an empty set) so it's on the map and consistent with the others.
                {    // (After all, we're currently looping through the opentxs thread items for THIS nym/threadID, so we KNOW we're GOING to be adding an item to it THIS iteration).
                     // This also makes it easy for us to assume, for the duration of the loop, that pAlreadyImportedIds is VALID. (Should assert in fact) since it should have been
                     // either set on a previous iteration of this loop, or on THIS iteration. Either way, below this point in the loop, we can now assume that it's good.
                     //
                    pAlreadyImportedIds.reset( new SetOfStrings );
                }
                bHaveGrabbedTheAlreadyImportedIds = true;

                // for optimization (lookup table)
                PairOfStrings nymIdAndThreadId{str_nym_id, str_thread_id};
                std::pair<PairOfStrings, PtrSetOfStrings> the_pair = std::make_pair(nymIdAndThreadId, pAlreadyImportedIds);
                mapOfSetsOfAlreadyImportedMsgs.insert(the_pair);
            }
            // --------------------------------------------
            // Since we can assume from here on out that pAlreadyImportedIds is always a good pointer, anywhere below this point
            // in the entire loop (see above comment) I'll just assert.
            //
            OT_ASSERT_MSG(pAlreadyImportedIds, "Should never happen: pAlreadyImportedIds contained nullptr.");

            bHaveAlreadyImportedAnyIds = (pAlreadyImportedIds->size() > 0);
            // --------------------------------------------
        }
        // --------------------------------------------
        // I do the same assert here, since the above block will not occur every iteration of this loop.
        //
        OT_ASSERT_MSG(pAlreadyImportedIds, "Should never happen: pAlreadyImportedIds contained nullptr.");
        // --------------------------------------------
        // Now let's SEE if we have already previously imported THIS thread item (for the loop iteration
        // we're on currently).
        //
        bool bHaveAlreadyImportedThisId = false;

        if (bHaveAlreadyImportedAnyIds) // we only bother looking if we've previously imported ANY items for the current thread.
        {
            const std::set<std::string>::iterator it = pAlreadyImportedIds->find(thread_item_id);

            bHaveAlreadyImportedThisId = (pAlreadyImportedIds->end() != it); // If true, no need to re-import this thread item.
        }
        // --------------------------------------------
        // Make sure we have the conversation itself, before adding the
        // conversational item.
        // Update conversation_name also.
        if (!bHaveAlreadyImportedThisId && !bConversationExistsInDB)
        {
            bConversationExistsInDB = MTContactHandler::getInstance()->EnsureConversationExists(qstrMyNymId, qstrThreadId, qstrThreadName);

            if (!bConversationExistsInDB) {
                qDebug() << "Moneychanger::ImportConversationForNym: Somehow failed to get or create conversation in DB when trying to import a conversational thread item.";
                return false;
            }
//          else
//          {
                // Since we just imported this conversation (NOT ANY ITEMS YET, NECESSARILY) to the DB,
                // *AND* since it wasn't previously there, we'll go ahead and add it to our lookup table
                // as well.
                //
                // UPDATE: No need, due to some code higher up in this function, we already know for a
                // fact that pAlreadyImportedIds is a valid sharedPtr to a std::set<std::string>,
                // and it's already pointing to an instantiated set, that's already on the map, for
                // the appropriate nym/thread ids.
                //
                // Of course we still need to insert the thread ITEM to it, for this iteration, but that
                // happens below.
//          }
        }
        // bConversationExistsInDB is now definitely true, below this point.
        // --------------------------------------------
        //
        bool bItemExistsInDB = false;

        if (bHaveAlreadyImportedThisId) // If it's on the lookup table, no need to hit the DB.
        {
            bItemExistsInDB = true;
        }
        else // We need to import this thread item -- it's never been imported before.
        {
            bItemExistsInDB = MTContactHandler::getInstance()->EnsureConversationItemExists(qstrMyNymId, qstrThreadId, qstrThreadItemId,
                qstrThreadItemAccountId, thread_item_box,
                thread_item_timestamp, thread_item_unread);
        }
        // --------------------------------------------
        if (bItemExistsInDB)
        {
            if (!bHaveAlreadyImportedThisId) // This means we JUST NOW imported it to DB
            {                                // (It wasn't previously there until this current loop iteration).
                // So let's add it to our lookup table.
                // (Just as we did above when we searched for this item in the DB
                // when it wasn't found in the lookup table. If found in the DB,
                // we add to the looktable table then as well.)
                // And remember, the lookup table might ALREADY have an element for the thread
                // itself, so we need to add the item to the existing one if that's found.
                //
                // UPDATE: The lookup table now DEFINITELY already has an element for the relevant
                // thread. And pAlreadyImportedIds is DEFINITELY a valid shared_ptr to a std::set<std::string>
                // of the thread item IDs, and it's definitely already on the lookup table map for the correct
                // nym/threadID.
                // So we don't have to search mapOfSetsOfAlreadyImportedMsgs here anymore. The map is already
                // set up, and pAlreadyImportedIds is already good, and we can insert the new thread item ID
                // into it.
                //
                pAlreadyImportedIds->insert(thread_item_id); // adding to lookup table since it's a new thread item that was imported.


//                // NymID, (conversational) Thread Id, Thread Item Id.
//                typedef std::tuple<std::string, std::string, std::string> TupleNymThreadAndItem;
//                typedef std::shared_ptr<TupleNymThreadAndItem> PtrNymThreadAndItem;
//                typedef std::set<PtrNymThreadAndItem> SetNymThreadAndItem;

                // Add the Nym / Thread item ID pair to an output parameter so the caller will know
                // later on, which ones need the message body imported for the first timne, versus which ones
                // we just "ensured" were already there.
                // UPDATE: hell, let's add the thread id while we're at it.
                //
                setNewlyAddedNymThreadAndItem.insert(PtrNymThreadAndItem{new TupleNymThreadAndItem{str_nym_id, str_thread_id, thread_item_id} });
            }
            // --------------------
            // By this point, we know the conversational item is in the DB, AND it's in the lookup table.
            // Done with this iteration.
        }
        else {
            qDebug() << "Moneychanger::ImportConversationForNym: Somehow failed to get or create conversation ITEM in DB.";
            return false;
        }
        // ----------------------------------------------
    } // for
    return true;
}

void Moneychanger::modifyRecords()
{
    MapOfPtrSetsOfStrings mapOfSetsOfAlreadyImportedMsgs; // key is a std::pair composed of nym_id/thread_id; value is shared_ptr to std::set<std::string> of thread_item_ids.
    MapOfConversationsByNym mapOfConversationsByNym; // key is nym_id, value is shared_ptr to std::set<std::string> of thread_ids.

//  typedef std::tuple<std::string, std::string, std::string> TupleNymThreadAndItem;
//  typedef std::shared_ptr<TupleNymThreadAndItem> PtrNymThreadAndItem;
//  typedef std::set<PtrNymThreadAndItem> SetNymThreadAndItem;

    SetNymThreadAndItem setNewlyAddedNymThreadAndItem;
    // -------------------------------------------------------------------
    const int32_t nymCount = ot_.API().Exec().GetNymCount();
    for ( int32_t nymIndex = 0; nymIndex < nymCount; ++nymIndex)
    {
        const std::string nymId = ot_.API().Exec().GetNym_ID(nymIndex);
        const QString qstrMyNymId = QString::fromStdString(nymId);

        //  typedef std::set<std::string> SetOfStrings;
        //  typedef std::shared_ptr<SetOfStrings> PtrSetOfStrings;
        //  typedef std::pair<std::string, std::string> PairOfStrings;
        //  typedef std::map<PairOfStrings, PtrSetOfStrings> MapOfPtrSetsOfStrings;
        //  typedef std::map<std::string, PtrSetOfStrings> MapOfConversationsByNym;

        // Before we import the RecordList, we have to iterate through
        // the conversations, and any messages therein, and continually re-import
        // them (since the lifecycle items may have updated since last time).
        //
        // We do NOT delete anything; we just import the conversations and messages
        // into the GUI DB for later.
        //
        // UPDATE: FOr now I'm adding lookup tables for thread-item-id to thread-id,
        // (that's conversational thread) so I can later find the thread id using the
        // thread-item-id.
        // First that allows me to determine if a Nymn has ever seen part of a given
        // conversation before at all. Second, it allows me to determine, without
        // a fresh database lookup each time, for each thread-item-id, whether or not
        // it has already been imported into my local database. (Thus sparing me from
        // having to re-import it, in that case).
        // That's why I ImportConversationsForNym and THEN import the record list.
        // Because the record list has the thread-item-id for each record, but NOT its
        // corresponding thread ID. But that's okay, since I already imported the
        // conversations AND put them into a lookup table,
        //
        // This call here checks the lookup table to see if a given msg has already
        // been imported. If it hasn't, then it checks the local database to see if
        // it's there. When it does that, it loads ALL the thread IDs for a given Nym
        // at the same time (into the lookup table) so it doesn't have to hit the DB
        // every single time for each thread-item-id in that thread. (Since it has
        // to loop through all of those).
        //
        ImportConversationsForNym(nymId, mapOfSetsOfAlreadyImportedMsgs,
                                  setNewlyAddedNymThreadAndItem); // output here, for newly-imported items.
        // ---------------------
        // Obviously this has to go under the above call.
        //
        PtrSetOfStrings pPreimportedConversationIdsForNym{
            MTContactHandler::getInstance()->selectPreimportedConversationIdsForNym(qstrMyNymId)};

        mapOfConversationsByNym.insert(std::pair<std::string,PtrSetOfStrings>(
                                            nymId,
                                            pPreimportedConversationIdsForNym)
                                      );
    }
    // By this point we've loaded up lookup tables for the nymId, threadId, and ThreadItemId.
    // We've also imported the messages that we hadn't previously imported
    // (and ONLY those -- with no re-importing).
    // That way below, when we call processImportRecord, we will be able to skip those
    // thread item IDs that have been previously imported already. (WithOUT having to
    // search our local DB for each one to see if it's already there.)
    // -------------------------------------------------------
    const int listSize = GetRecordlist().size();
    // -------------------------------------------------------
    // Delete the market receipts (since they are already archived in other places)
    // and find any finalReceipts that correspond to those, so we can add them
    // to the trade archive table as well (and delete them as well.)
    // Leave any other final receipts, since they may correspond to offers that
    // completed without a trade, or to a smart contract, or to a recurring payment, etc.
    //
    for (int ii = 0; ii < listSize; ++ii)
    {
        const int nIndex = listSize - ii - 1; // We iterate through the list in reverse. (Since we'll be deleting stuff.)

        opentxs::OTRecord record = GetRecordlist().GetRecord(nIndex);
        {
            opentxs::OTRecord& recordmt = record;

            if (false == recordmt.IsFinalReceipt()) {
                processImportRecord(recordmt, nIndex, mapOfSetsOfAlreadyImportedMsgs, mapOfConversationsByNym, setNewlyAddedNymThreadAndItem);
            }
        }
    } // for (GetRecordlist() in reverse)
    //
    // Notice that this function now does the same thing twice.
    // Only difference is, the first time around it does all NON-Final Receipts,
    // but the second time around it does all FINAL receipts. (And final receipt notices).
    // This is basically just to ensure that final receipts are always done LAST.
    //
    // -------------------------------------
    // If the above process DID remove any records, then we have to repopulate them now,
    // since every record contains its index, and so they will be wrong until re-populated.
    //
    if (listSize != GetRecordlist().size())
        populateRecords(true); //bCurrentlyModifying=false by default.
    // -------------------------------------
    // -------------------------------------
    const int newListSize = GetRecordlist().size();

    for (int iii = 0; iii < newListSize; ++iii)
    {
        const int nIndex = newListSize - iii - 1; // We iterate through the list in reverse. (Since we'll be deleting stuff.)

        opentxs::OTRecord record = GetRecordlist().GetRecord(nIndex);
        {
            opentxs::OTRecord& recordmt = record;

            if (true == recordmt.IsFinalReceipt()) {
                processImportRecord(recordmt, nIndex, mapOfSetsOfAlreadyImportedMsgs, mapOfConversationsByNym, setNewlyAddedNymThreadAndItem);
            }
        }
    } // for (GetRecordlist() in reverse)
    // -------------------------------------
    // If the above process DID remove any records, then we have to repopulate them now,
    // since every record contains its index, and so they will be wrong until re-populated.
    //
    if (listSize != GetRecordlist().size())
        populateRecords(true); //bCurrentlyModifying=false by default.
    // -------------------------------------
}


void Moneychanger::processImportRecord(
        opentxs::OTRecord& recordmt,
        const int nIndex,
        MapOfPtrSetsOfStrings & mapOfSetsOfAlreadyImportedMsgs,
        MapOfConversationsByNym & mapOfConversationsByNym,
        SetNymThreadAndItem & setNewlyAddedNymThreadAndItem
        )
{
    if (!recordmt.CanDeleteRecord())
    {
        // In this case we aren't going to delete the record, but we can still
        // save a copy of it in our local database, if it's not already there.

        if (!recordmt.IsMail() &&
            !recordmt.IsSpecialMail() &&
            !recordmt.IsExpired() )
        {
            bool bShouldImportPayment   = true;  // To preserve original logic.
            bool bShouldImportAgreement = false; // This part is new.

            if (recordmt.IsPending()
               && (recordmt.IsPaymentPlan() ||
                   recordmt.IsContract()) )
            {
                bShouldImportPayment   = true;
                bShouldImportAgreement = true;
            }

            if (bShouldImportPayment)
                AddPaymentToPmntArchive(recordmt);

            if (bShouldImportAgreement)
                AddAgreementRecord(recordmt);
        }
    }
    else // record can be deleted.
    {
        // If recordmt IsRecord() and IsReceipt() and is a "finalReceipt"
        // then try to look it up in the Trade Archive table. For all entries
        // from the same transaction, we set the final receipt text in those
        // rows.
        //
        // Then we delete the finalReceipt from the OT Record Box.
        //
        // Meanwhile, for all marketReceipts, we just deleting them since they
        // are ALREADY in the trade_archive table.
        //
        // -----------------------------------
        bool bShouldDeleteRecord = false;
        // -----------------------------------
        if (recordmt.IsMail() || recordmt.IsSpecialMail())
        {
            if (AddMailToMsgArchive(recordmt, mapOfSetsOfAlreadyImportedMsgs, mapOfConversationsByNym, setNewlyAddedNymThreadAndItem)) {
                ;
            }
                //bShouldDeleteRecord = true;  // Disabling this temporarily to see if it fixes any related startup/shutdown issues.
        }
        // -----------------------------------
        else if (recordmt.IsNotice())
        {
            bool bShouldImportPayment   = false;
            bool bShouldImportAgreement = false;
//          bool bShouldImportFinalReceiptToTradeArchive = false; // experimental. See note just below.

            if (recordmt.HasOriginType())
            {
                if (recordmt.IsOriginTypeMarketOffer()) {
                    qDebug() << __FUNCTION__ << ": This is where I almost just added a market notice to the payments table. I stopped myself.";
                    // There actually IS a finalReceipt NOTICE for market exchange. (Sent to the Nym.)
                    // (Versus the finalReceipt RECEIPTs, which are sent to 2 ACCOUNTS for each Nym.)
                    // Therefore, AddAgreementRecord needs to check for "IsNotice" in combination with "IsFinalReceipt"
                    // since in that case, it should know it hasn't received its official 4 ACCOUNT receipts yet,
                    // and more importantly, that this notice should NOT overwrite any of those, but it
                    // has still received _some_ useful information -- that the agreement itself _has_ been
                    // terminated.
                    //
                    //bShouldImportFinalReceiptToTradeArchive = true;

                    bShouldDeleteRecord = true;
                }
                else if (recordmt.IsOriginTypePaymentPlan()) {
                    bShouldImportPayment   = true;
                    bShouldImportAgreement = true;
                }
                else if (recordmt.IsOriginTypeSmartContract()) {
                    bShouldImportPayment   = true;
                    bShouldImportAgreement = true;
                }
                else {
                    qDebug() << __FUNCTION__ << ": Importing notices, found a record with an unknown origin type.";
                }
            }
            else {
                bShouldImportPayment = true;

                // QString tFinal = "false";
                // QString tContract = "false";
                // QString tPlan = "false";
                // QString tNotice = "false";
                // QString tExpired = "false";
                // QString tCanceled = "false";
                // QString tSuccess = "false";
                // bool bIsSuccess = false;
                //
                // if (recordmt.IsFinalReceipt())          tFinal = "true";
                // if (recordmt.IsContract())              tContract = "true";
                // if (recordmt.IsPaymentPlan())           tPlan = "true";
                // if (recordmt.IsNotice())                tNotice = "true";
                // if (recordmt.IsExpired())               tExpired = "true";
                // if (recordmt.IsCanceled())              tCanceled = "true";
                // if (recordmt.HasSuccess(bIsSuccess))    tSuccess = bIsSuccess ? "true" : "false";
                //
                // qstrTemp = QString(" IsFinalReceipt: %1\n IsContract: %2 \n IsPaymentPlan: %3 \n IsNotice: %4 \n IsExpired: %5 \n IsCanceled: %6 \n IsSuccess: %7 ")
                //         .arg(tFinal).arg(tContract).arg(tPlan).arg(tNotice).arg(tExpired).arg(tCanceled).arg(tSuccess);
                //
                // qDebug() << qstrTemp;
            }
            // -------------------------------------
            if (bShouldImportPayment && AddPaymentBasedOnNotice(recordmt))
                bShouldDeleteRecord = true;
            if (bShouldImportAgreement && AddAgreementRecord(recordmt))
                bShouldDeleteRecord = true;
         // if (bShouldImportFinalReceiptToTradeArchive && AddFinalReceiptToTradeArchive(recordmt))
         //       bShouldDeleteRecord = true;
        }
        // -----------------------------------
        else if (recordmt.IsRecord() && !recordmt.IsExpired())
        {
            if (recordmt.IsReceipt())
            {
                if (0 == recordmt.GetInstrumentType().compare("marketReceipt"))
                {
                    // We don't have to add these to the trade archive table because they
                    // are already there. OTClient directly adds them into the TradeDataNym object,
                    // and then Moneychanger reads that object and imports it into the trade_achive
                    // table already. So basically here all we need to do is delete the market
                    // receipt records so the user doesn't have the hassle of deleting them himself.
                    // Now they are safe in his archive and he can do whatever he wants with them.

                    bShouldDeleteRecord = true;
                } // marketReceipt
                // -----------------------------------
                // NOTE: PayDividend is possibly having its receipts go in here.
                // Todo: Fix pay dividend in UI.
                //
                else if (0 == recordmt.GetInstrumentType().compare("paymentReceipt"))
                {
                    if (recordmt.HasOriginType()
                        && (recordmt.IsOriginTypePaymentPlan() ||
                            recordmt.IsOriginTypeSmartContract())
                        && AddAgreementRecord(recordmt)) // <====== IMPORTS HERE.
                    {
                        bShouldDeleteRecord = true;
                    }
                 // else
                 //     qDebug() << __FUNCTION__ << ": Failed trying to add a receipt to the agreement archive.";

                    // Commenting this out for now.
                    // It might have been added just to hide the pay dividend problems.
                    // Now I want to see whatever pops up here.
                    //bShouldDeleteRecord = true;
                } // paymentReceipt
                // -----------------------------------
                else if (recordmt.IsFinalReceipt())
                {
                    // Notice here we only delete the record if we successfully
                    // added the final receipt to the trade archive table.
                    // Why? Because the trade archive table contains receipts
                    // of COMPLETED TRADES. So if we fail to find any of those
                    // to add the final receipt to, we don't just want to DELETE
                    // the final receipt -- the user's sole remaining copy!
                    // - So for trades that occurred, the final receipt will be stored
                    // with those archives next to the corresponding market receipts.
                    // - And for trades that did NOT occur, the final receipt will
                    // remain in the record box, so the user himself can delete those
                    // whenever he sees fit. They will be his only notice that an
                    // offer completed on the market without any trades occurring.
                    // We might even change the GUI label now for final receipt records,
                    // (in the Pending Transactions window) to explicitly say,
                    // "offer completed on market without any trades."
                    //
                    // P.S. There's another reason not to just delete a finalReceipt
                    // if we can't find any trades associated with it: because it might
                    // not be a finalReceipt for a market offer! It might correspond to
                    // a smart contract or a recurring payment plan.
                    //
                    // UPDATE: We now CAN tell for finalReceipts and paymentReceipts, whether
                    // they are for market offers, payment plans, or smart contracts. So the
                    // logic is updated now to not even TRY to import into the trade archive
                    // unless it's specifically a finalReceipt for a market offer.
                    //
                    const bool has_origin_type     = recordmt.HasOriginType();
                    const bool origin_market_offer = has_origin_type && recordmt.IsOriginTypeMarketOffer();
                    const bool added_trade_archive = origin_market_offer && AddFinalReceiptToTradeArchive(recordmt);

                    if (   has_origin_type
                        && origin_market_offer
                        && added_trade_archive) // <==== IMPORTS HERE.
                        bShouldDeleteRecord = true;
                    else if (AddAgreementRecord(recordmt)) // Okay, it's for a smart contract or recurring payment plan.
                    {
                        bShouldDeleteRecord = true;

                        // For now I'm doing this one here as well, so the normal payments screen
                        // is able to realize that the agreement has finished.
                        //
                        // UPDATE: We no longer do this. If someone activates a payment plan, and it
                        // says "activated" in the payments screen. (Or "canceled" or "expired" or whatever)
                        // then that was its state when that action occurred, and it's now historical.
                        // It "was activated." Since then, is it STILL active? If you want to know that,
                        // you have to look at the "active agreements" window where you can see its current
                        // status and its finalReceipts and paymentReceipts.
                        //
                        // UPDATE: uncommenting this for now, to see how it goes. I'm thinking that
                        // both the payments screen AND the active agreements screen should show the
                        // most recent status of the agreement.
                        //
                        // UPDATE: commented it out again. In the payments GUI, we had a sent
                        // activated, and a received activated, and you had to go to the live
                        // agreements GUI to see what happened beyond that. It's better to keep
                        // it that way, since when I uncomment this, those get replaced in the
                        // payments UI with both final receipts in the received. It's appropriate
                        // to see those appear in the live agreements, but it's confusing to have
                        // them appear in the payments UI, so I commented this out again.
                        //
                        //AddPaymentToPmntArchive(recordmt);
                    }
                    else
                        qDebug() << QString(" --- Tried to import final receipt, but it failed! has_origin_type: %1 origin_market_offer: %2 added_trade_archive: %3").arg(has_origin_type).arg(origin_market_offer).arg(added_trade_archive);
                } // finalReceipt
                else // All  other closed (deletable) receipts.
                {
                    qDebug() << __FUNCTION__ << ": FYI, IMPORTING A RECEIPT OF CLOSED 'ALL OTHER' TYPE.";

                    if (AddPaymentToPmntArchive(recordmt))
                    {
                        bShouldDeleteRecord = true;
                    }
                 // else
                 //     qDebug() << __FUNCTION__ << ": Failed trying to add a receipt to the payment archive.";
                }
            }
            // -----------------------------------
            else // All  other delete-able, non-expired records.
            {

                qDebug() << __FUNCTION__ << ": FYI, IMPORTING A NON-RECEIPT OF DELETABLE, NON-EXPIRED TYPE.";

                // For example, an incoming cheque becomes a received cheque after depositing it.
                // At that point, OT moves incoming cheque from the payments inbox, to the record box. So
                // it's not a cheque receipt (the payer gets that; you're the recipient) but it's the deletable
                // record of the incoming cheque itself, for a cheque you've since already deposited, and thus
                // are now moving to your payment receipts table.
                // There IS another relevant receipt, however -- the cheque DEPOSIT. When you deposited the cheque,
                // YOU got a deposit receipt. This is currently not recorded here but it really should be. That
                // way you can see the cheque itself, as well as your receipt from depositing that cheque. It'd just
                // be two different receipts on the same payment record, similar to what we do in the trade archive
                // table, which has potentially up to 3 different receipts for the same trade record. (Market receipt
                // for asset and currency accounts, plus final receipt.)
                //
                if ((recordmt.IsPaymentPlan() || recordmt.IsContract()) // They could be here maybe because they expired without ever being activated.
                    && AddAgreementRecord(recordmt))
                {
                    bShouldDeleteRecord = true;
                }

                if (AddPaymentToPmntArchive(recordmt))
                {
                    bShouldDeleteRecord = true;
                }
            }
        } // else if (recordmt.IsRecord() && !recordmt.IsExpired())
        // -----------------------------------
        if (bShouldDeleteRecord)
        {
            if (recordmt.DeleteRecord())
            {
                bool bRemoved = GetRecordlist().RemoveRecord(nIndex);

                if (!bRemoved)
                    qDebug() << "Moneychanger::modifyRecords: weird issue trying to remove deleted record from GetRecordlist() (record list.)\n";
            }
        }
    } // Record can be deleted.
}


// This function is used sometimes, but it's NOT called by the function below it, that
// does this stuff in a loop. Why not? Because we don't want to download the same Nym
// for EACH account he owns, when we could just download the Nym once and then download
// all his accounts once. So this function is only used for more targeted cases where you
// really prefer the faster loading time. Also, notice the "emit populatedRecordList()"
// that you see at the bottom? We don't want to have to emit that 10 times in a row, so
// again, you would only use this function in the case where that Acct and Nym really are
// the ONLY two entities being refreshed.
//
void Moneychanger::onNeedToDownloadSingleAcct(QString qstrAcctID, QString qstrOptionalAcctID)
{
    if (qstrAcctID.isEmpty())
        return;

    QString qstrErrorMsg;
    qstrErrorMsg = tr("Failed trying to contact the notary. Perhaps it is down, or there might be a network problem.");
    // ------------------------------

    std::string accountId = qstrAcctID.toStdString();
    std::string acctNymID = ot_.API().Exec().GetAccountWallet_NymID   (accountId);
    std::string acctSvrID = ot_.API().Exec().GetAccountWallet_NotaryID(accountId);
    // ------------------------------
    std::string accountIdOptional = qstrOptionalAcctID.isEmpty() ? "" : qstrOptionalAcctID.toStdString();
    std::string acctNymIDOptional = qstrOptionalAcctID.isEmpty() ? "" : ot_.API().Exec().GetAccountWallet_NymID   (accountIdOptional);
    std::string acctSvrIDOptional = qstrOptionalAcctID.isEmpty() ? "" : ot_.API().Exec().GetAccountWallet_NotaryID(accountIdOptional);
    // ------------------------------
    bool bRetrievalAttemptedNym = false;
    bool bRetrievalSucceededNym = false;
    bool bRetrievalAttemptedAcct = false;
    bool bRetrievalSucceededAcct = false;
    // ------------------------------
    if (!acctNymID.empty() && !acctSvrID.empty())
    {
        MTSpinner theSpinner;

        bRetrievalAttemptedNym = true;
        bRetrievalSucceededNym = retrieve_nym(acctSvrID, acctNymID);

        if (!ot_.API().Exec().CheckConnection(acctSvrID))
        {
            emit appendToLog(qstrErrorMsg);
            return;
        }

        // Let's download the Nym for the optional account too, but ONLY if it's not the same Nym!!
        //
        if (bRetrievalSucceededNym && !qstrOptionalAcctID.isEmpty() && (acctNymIDOptional != acctNymID))
        {
            bRetrievalSucceededNym = retrieve_nym(acctSvrIDOptional, acctNymIDOptional);

            if (!ot_.API().Exec().CheckConnection(acctSvrIDOptional))
            {
                emit appendToLog(qstrErrorMsg);
                return;
            }
        }
    }
    if (bRetrievalSucceededNym)
    {
        MTSpinner theSpinner;

        bRetrievalAttemptedAcct = true;
        const opentxs::Identifier notaryID{acctSvrID}, nymID{acctNymID}, accountID{accountId};
        bRetrievalSucceededAcct = ot_.API().ServerAction().DownloadAccount(nymID, notaryID, accountID, true);

        if (bRetrievalSucceededAcct && !qstrOptionalAcctID.isEmpty())
        {
            const opentxs::Identifier notaryIDOptional{acctSvrIDOptional},
				nymIDOptional{acctNymIDOptional}, accountIDOptional{accountIdOptional};
            bRetrievalSucceededAcct = ot_.API().ServerAction().DownloadAccount(
            		nymIDOptional, notaryIDOptional, accountIDOptional, true);
        }

        if (!ot_.API().Exec().CheckConnection(acctSvrIDOptional))
        {
            emit appendToLog(qstrErrorMsg);
            return;
        }
    }
    // ----------------------------------------------------------------
    const bool bRetrievalAttempted = (bRetrievalAttemptedNym && bRetrievalAttemptedAcct);
    const bool bRetrievalSucceeded = (bRetrievalSucceededNym && bRetrievalSucceededAcct);
    // ----------------------------------------------------------------
    if (bRetrievalAttempted)
    {
        if (!bRetrievalSucceeded) {
            Moneychanger::It()->HasUsageCredits(acctSvrID, acctNymID);
            return;
        }
        else
        {
            // ----------------------------------------------------------------
            // This refreshes any new Nym Trade Data (the receipts we just downloaded
            // may include Market Receipts, so we need to import those into the Historical Trade Archive.)
            //
            QPointer<ModelTradeArchive> pModel = DBHandler::getInstance()->getTradeArchiveModel();

            if (pModel)
            {
                pModel->updateDBFromOT();
            }
            // ----------------------------------------------------------------
            onNeedToPopulateRecordlist();
            return;
        }
    }
}

// ----------------------------------------------------------------

void Moneychanger::onNeedToPopulateRecordlist()
{
    setupRecordList();
    populateRecords(); // This updates the record list. (It assumes a download has recently occurred.)
    // ----------------------------------------------------------------
    emit populatedRecordlist();
}

// ----------------------------------------------------------------

void Moneychanger::onNeedToDownloadAccountData()
{

//    const auto nymCount = ot_.API().Exec().GetNymCount();
//    if (0 == nymCount) {
//        const std::string id =
//            opentxs::SwigWrap::CreateIndividualNym("Me", "", 0);
//        if (!id.empty()) {
//            DBHandler::getInstance()->AddressBookUpdateDefaultNym(
//                QString::fromStdString(id));
//        }
//    }
//    const auto defaultNotaryID = get_default_notary_id().toStdString();
//    if (defaultNotaryID.empty()) {
//        DBHandler::getInstance()->
//            AddressBookUpdateDefaultServer(get_notary_id_at(0));
//    }
//    const auto defaultUnitDefinitionID (get_default_asset_id().toStdString());
//    if (defaultUnitDefinitionID.empty()) {
//        DBHandler::getInstance()->
//            AddressBookUpdateDefaultAsset(get_asset_id_at(0));
//    }

    ot_.API().Sync().Refresh();

    return;
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
    int32_t  asset_count = ot_.API().Exec().GetAssetTypeCount();
    bool    bFoundPreset = false;

    for (int32_t ii = 0; ii < asset_count; ii++)
    {
        QString OT_id   = QString::fromStdString(ot_.API().Exec().GetAssetType_ID(ii));
        QString OT_name = QString::fromStdString(ot_.API().Exec().GetAssetType_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);
        // ------------------------------
        if (!qstrPresetID.isEmpty() && (0 == qstrPresetID.compare(OT_id)))
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
        mc_systrayMenu_asset->setTitle(tr("Asset contract: ")+asset_name);

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
    int32_t acct_count = ot_.API().Exec().GetAccountCount();
    bool bFoundDefault = false;

    for (int32_t ii = 0; ii < acct_count; ii++)
    {
        QString OT_id   = QString::fromStdString(ot_.API().Exec().GetAccountWallet_ID(ii));
        QString OT_name = QString::fromStdString(ot_.API().Exec().GetAccountWallet_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);
        // ------------------------------
        if (!qstrAcctID.isEmpty() && (0 == qstrAcctID.compare(OT_id)))
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
//      emit populatedRecordlist();
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
        QString result = account_name;
//      QString result = tr("Account: ") + account_name;

        int64_t     lBalance  = ot_.API().Exec().GetAccountWallet_Balance    (account_id.toStdString());
        std::string strAsset  = ot_.API().Exec().GetAccountWallet_InstrumentDefinitionID(account_id.toStdString());
        // ----------------------------------------------------------
        std::string str_amount;

        if (!strAsset.empty())
        {
            str_amount = ot_.API().Exec().FormatAmount(strAsset, lBalance);
            result += " ("+ QString::fromStdString(str_amount) +")";
        }

        mc_systrayMenu_account->setTitle(result);
        // -----------------------------------------------------------
        std::string strNym    = ot_.API().Exec().GetAccountWallet_NymID   (account_id.toStdString());
        std::string strServer = ot_.API().Exec().GetAccountWallet_NotaryID(account_id.toStdString());

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
                QString qstrAssetName = QString::fromStdString(ot_.API().Exec().GetAssetType_Name(strAsset));

                if (!qstrAssetName.isEmpty() && (mc_systrayMenu_asset))
                    setDefaultAsset(QString::fromStdString(strAsset),
                                    qstrAssetName);
            }
            // -----------------------------------------------------------
            if (!strNym.empty())
            {
                QString qstrNymName = QString::fromStdString(ot_.API().Exec().GetNym_Name(strNym));

                if (!qstrNymName.isEmpty() && (mc_systrayMenu_nym))
                    setDefaultNym(QString::fromStdString(strNym),
                                  qstrNymName);
            }
            // -----------------------------------------------------------
            if (!strServer.empty())
            {
                QString qstrServerName = QString::fromStdString(ot_.API().Exec().GetServer_Name(strServer));

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
            // ----------------------------------------------------------------
            onNeedToPopulateRecordlist();
        }
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
    int32_t server_count = ot_.API().Exec().GetServerCount();
    bool    bFoundPreset = false;

    for (int32_t ii = 0; ii < server_count; ii++)
    {
        QString OT_id   = QString::fromStdString(ot_.API().Exec().GetServer_ID(ii));
        QString OT_name = QString::fromStdString(ot_.API().Exec().GetServer_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);
        // ------------------------------
        if (!qstrPresetID.isEmpty() && (0 == qstrPresetID.compare(OT_id)))
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

    if (mc_overall_init)
    {
        if (new_server_title.isEmpty())
            mc_systrayMenu_server->setTitle(tr("Set default server..."));
        else
            mc_systrayMenu_server->setTitle(tr("Server contract: ")+new_server_title);

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

void Moneychanger::mc_request_to_acct_from_contact(QString qstrAcct, QString qstrContact)
{
    mc_requestfunds_show_dialog(qstrAcct, qstrContact);
}

void Moneychanger::mc_request_to_acct(QString qstrAcct)
{
    mc_requestfunds_show_dialog(qstrAcct);
}

void Moneychanger::mc_requestfunds_show_dialog(QString qstrAcct/*=QString("")*/, QString qstrContact/*=QString("")*/)
{
    // --------------------------------------------------
    MTRequestDlg * request_window = new MTRequestDlg(NULL);
    request_window->setAttribute(Qt::WA_DeleteOnClose);
    // --------------------------------------------------
    QString qstr_acct_id = qstrAcct.isEmpty() ? this->get_default_account_id() : qstrAcct;

    if (!qstr_acct_id.isEmpty())
        request_window->setInitialMyAcct(qstr_acct_id);
    // --------------------------------------------------
    if (!qstrContact.isEmpty())
    {
        request_window->setInitialHisContact(qstrContact);
    }
    // ---------------------------------------
    request_window->dialog();
    // --------------------------------------------------
}


#include <QSerialPort>
#include <QSerialPortInfo>
#include <QLabel>
#include <QScrollArea>

/**
  * Pair Stash Node
  **/
void Moneychanger::mc_pair_node_slot()
{
    QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("Make sure the pairing cable is attached, then click OK."));
    // ----------------------------------------
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        auto baudrates = info.standardBaudRates();
        QString qstrBaudrates;
        QList<qint32>::const_iterator i;
        for (i = baudrates.begin(); i != baudrates.end(); ++i) {
            if (i != baudrates.begin())
                qstrBaudrates += QString(", ");
            qstrBaudrates += QString::number(*i);
        }
        QString s = QObject::tr("Port: ") + info.portName() + "\n"
                    + QObject::tr("Location: ") + info.systemLocation() + "\n"
                    + QObject::tr("Description: ") + info.description() + "\n"
                    + QObject::tr("Manufacturer: ") + info.manufacturer() + "\n"
                    + QObject::tr("Serial number: ") + info.serialNumber() + "\n"
                    + QObject::tr("Vendor Identifier: ") + (info.hasVendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : QString()) + "\n"
                    + QObject::tr("Product Identifier: ") + (info.hasProductIdentifier() ? QString::number(info.productIdentifier(), 16) : QString()) + "\n"
                    + QObject::tr("StandardBaudRates: ") + qstrBaudrates + "\n"
                    + QObject::tr("Busy: ") + (info.isBusy() ? QObject::tr("Yes") : QObject::tr("No")) + "\n";
        qDebug() << s;
    }
    // ----------------------------------------
//    DlgPairNode dlgPair;

//    if (QDialog::Accepted != dlgPair.exec())
//        return;
    // ----------------------------------------
    QString serialPortName = QString("/dev/tty.usbserial-A104CNRS"); // todo
//  QString serialPortName = QString("/dev/cu.usbserial-A104CNRS");
    QSerialPort serialPort;
    serialPort.setPortName(serialPortName);

//  int serialPortBaudRate = QSerialPort::Baud38400;
//  serialPort.setBaudRate(serialPortBaudRate);

    if (!serialPort.open(QIODevice::ReadOnly)) {
        qDebug() << QObject::tr("Failed to open port %1, error: %2").arg(serialPortName).arg(serialPort.error()) << endl;
    }
    else
    {
        serialPort.setBaudRate(QSerialPort::Baud38400);
        serialPort.setDataBits(QSerialPort::Data8);
        serialPort.setStopBits(QSerialPort::OneStop);
        serialPort.setParity(QSerialPort::NoParity);
        serialPort.setFlowControl(QSerialPort::NoFlowControl);
        /**
         * Current flow control Options:
         * UsbSerialInterface.FLOW_CONTROL_OFF
         * UsbSerialInterface.FLOW_CONTROL_RTS_CTS only for CP2102 and FT232
         * UsbSerialInterface.FLOW_CONTROL_DSR_DTR only for CP2102 and FT232
         */

        // ------------------
        int nCounter{0};
        QByteArray readData;
        while (nCounter++ < 30 && serialPort.waitForReadyRead(10000)) {
            readData.append(serialPort.readAll());
        }
        auto the_error = serialPort.error();
        if (the_error == QSerialPort::ReadError) {
            qDebug() << QObject::tr("Failed to read from port %1, error: %2").arg(serialPortName).arg(serialPort.errorString());
        } else if ((serialPort.error() == QSerialPort::TimeoutError) && readData.isEmpty()) {
            qDebug() << QObject::tr("No data was currently available for reading from port: %1").arg(serialPortName);
        }
        else {
            QString qstrData = QString::fromUtf8(readData);
            qDebug() << QObject::tr("Data successfully received from port %1.").arg(serialPortName);
            //qDebug() << qstrData << endl;
            //qDebug() << readData << endl;

            QStringList strPairs = qstrData.split("\n");
            if (strPairs.length() >= 3) {
                QString strPair = strPairs[1]; // This way there's at least 3 pairs and we're grabbing the middle of the three.
                QStringList listData = strPair.split(",");

                if (2 == listData.length())
                {
                    const QString qstrBridgeNymID   = listData[0];
                    const QString qstrAdminPassword = listData[1];

                    QString qstrUserNymID = Moneychanger::It()->get_default_nym_id();
                    QString errorMessage{""};

                    if (qstrBridgeNymID.isEmpty()) {
                        errorMessage = tr("SNP Bridge Nym Id not available from pairing cable.");
                    }
                    if (qstrUserNymID.isEmpty()) {
                        errorMessage = tr("Default User NymId not available from local Opentxs wallet.");
                    }
                    if (qstrAdminPassword.isEmpty()) {
                        errorMessage = tr("SNP admin password not available from pairing cable.");
                    }
                    // -----------------------------------
                    const std::string str_introduction_notary_id{opentxs::String(ot_.API().Sync().IntroductionServer()).Get()};

                    if (str_introduction_notary_id.empty()) {
                        errorMessage = tr("Introduction Notary Id not available.");
                    }
                    // -----------------------------------
                    if (!errorMessage.isEmpty()) {
                        qDebug() << errorMessage << endl;
                        QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), errorMessage);
                        return;
                    }
                    // -----------------------------------
                    // By this point we at least know that the pre-requisites are there,
                    // so we can go ahead and start pairing. (Or see how much is already done,
                    // if it's already been started.)
                    //

                    /*

namespace opentxs
{
class Identifier;

namespace api
{
namespace client
{

class Pair
{
public:
    virtual bool AddIssuer(
        const Identifier& localNymID,
        const Identifier& issuerNymID,
        const std::string& pairingCode) const = 0;
    virtual std::string IssuerDetails(
        const Identifier& localNymID,
        const Identifier& issuerNymID) const = 0;
    virtual std::set<Identifier> IssuerList(
        const Identifier& localNymID,
        const bool onlyTrusted) const = 0;

*/
                    const opentxs::Identifier localNymID{qstrUserNymID.toStdString()};
                    const opentxs::Identifier issuerNymID{qstrBridgeNymID.toStdString()};
                    const std::string pairingCode{qstrAdminPassword.toStdString()};

                    // NEW JUSTUS API
                    const bool bPairNode = ot_.API().Pair().AddIssuer(localNymID, issuerNymID, pairingCode);
                    //const bool bPairNode = ot_.API().OTME_TOO().PairNode(qstrUserNymID.toStdString(), qstrBridgeNymID.toStdString(), qstrAdminPassword.toStdString());

                    if (!bPairNode) {
                        QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), tr("Pairing failed."));
                        return;
                    }
                    // ---------------------------------------------------
                    QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("Pairing successfully initiated in the background. You may now unplug your device."));
                }
            }
            else
            {
                qDebug() << QString("Didn't have enough pairs from the serial port. Fix the code.") << endl;
            }
        }
    }
}

/**
 * Import Cash
 **/

// TODO: Make this able to handle multiple instrument types
void Moneychanger::mc_import_slot()
{
    DlgImport dlgImport;

    if (QDialog::Accepted != dlgImport.exec())
        return;
    // ----------------------------------------
    QString qstrInstrument;
    // ----------------------------------------
    if (dlgImport.IsPasted()) // Pasted contents (not filename)
        qstrInstrument = dlgImport.GetPasted(); // Dialog prohibits empty contents, so no need to check here.
    // --------------------------------
    else  // Filename
    {
        QString fileName = dlgImport.GetFilename(); // Dialog prohibits empty filename, so no need to check here.
        // -----------------------------------------------
        QFile plainFile(fileName);

        if (plainFile.open(QIODevice::ReadOnly))//| QIODevice::Text)) // Text flag translates /n/r to /n
        {
            QTextStream in(&plainFile); // Todo security: check filesize here and place a maximum size.
            qstrInstrument = in.readAll();

            plainFile.close();
            // ----------------------------
            if (qstrInstrument.isEmpty())
            {
                QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                     QString("%1: %2").arg(tr("File was apparently empty")).arg(fileName));
                return;
            }
            // ----------------------------
        }
        else
        {
            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                 QString("%1: %2").arg(tr("Failed trying to read file")).arg(fileName));
            return;
        }
    }
    // --------------------------------
    // By this point, we have the contents of the instrument,
    // and we know they aren't empty.
    //
    std::string strInstrument{qstrInstrument.trimmed().toStdString()};
    opentxs::String otstrInstrument{strInstrument};
    qstrInstrument = QString::fromStdString(strInstrument);
    // -----------------------------------------------
    // This is for cases where the entire input is base64 (without bookends).
    //
    if (Moneychanger::is_base64(qstrInstrument))
    {
        opentxs::OTASCIIArmor ascInstrument(strInstrument.c_str());
        ascInstrument.GetString(otstrInstrument);
        strInstrument  = otstrInstrument.Get();
        qstrInstrument = QString::fromStdString(strInstrument);
    }
    // -----------------------------------------------
    // This handles "BEGIN SIGNED CHEQUE"
    // as well as "BEGIN OT ARMORED CHEQUE"
    //
    std::shared_ptr<opentxs::OTPayment> payment(new opentxs::OTPayment(otstrInstrument));

    if (!payment) {
        QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                             tr("Failed instantiation. (Should never happen)."));
        return;
    }
    else if (!payment->IsValid() || !payment->SetTempValues()) {
        QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                             tr("Failure: Unable to load financial instrument from text."));
        return;
    }
    // -----------------------------------------------
    bool bExpired{false};
    bool bWithinValidDateRange{false};
    time64_t validFrom{}, validTo{};
    QString  qstrDateFrom, qstrDateTo;
    QDateTime qdate_from, qdate_to;
    QDateTime currentDateTime = QDateTime::currentDateTime();

    QString qstrExpiration;

    if (payment->GetValidFrom(validFrom))
    {
        // ---------------------------------------------------------------------
        qdate_from = QDateTime::fromTime_t(validFrom);
        qstrDateFrom = qdate_from.toString(QString("MMM d yyyy hh:mm:ss"));
        // ---------------------------------------------------------------------
        if (currentDateTime < qdate_from)
        {
            qstrExpiration = QString("%1: %2")
                .arg(tr("Failed to import: This instrument is not valid yet, until"))
                .arg(qstrDateFrom);
        }
    }
    if (payment->GetValidTo(validTo))
    {
        qdate_to = QDateTime::fromTime_t(validTo);
        qstrDateTo = qdate_to.toString(QString("MMM d yyyy hh:mm:ss"));
    }
    // If we were able to retrieve expiration info
    // AND that info shows expired=true...
    if (payment->IsExpired(bExpired) && bExpired)
    {
        qstrExpiration = QString("%1: %2: %3")
            .arg(tr("Failed to import"))
            .arg(tr("This instrument expired on"))
            .arg(qstrDateTo);
    }
    // If we were able to retrieve date range info
    // AND that info shows we're NOT within the valid date range...
    if (payment->VerifyCurrentDate(bWithinValidDateRange) && !bWithinValidDateRange)
    {
        if (qstrExpiration.isEmpty())
            qstrExpiration = QString("%1: %2: %3 %4 %5")
                .arg(tr("Failed to import"))
                .arg(tr("This instrument is outside its valid date range of"))
                .arg(qstrDateFrom)
                .arg(tr("to"))
                .arg(qstrDateTo);
    }
    // -------------
    if (!qstrExpiration.isEmpty())
    {
        QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), qstrExpiration);
        return;
    }
    // -----------------------------------------------
    QString qstrErrorMsg;

    switch (payment->GetType()) {
        case opentxs::OTPayment::PAYMENT_PLAN: {
            qstrErrorMsg = tr("Sorry, we don't yet allow importing payment plans.");
            break;
        }
        case opentxs::OTPayment::SMART_CONTRACT: {
            qstrErrorMsg = tr("Sorry, we don't yet allow importing smart contracts.");
            break;
        }
        case opentxs::OTPayment::NOTICE: {
            qstrErrorMsg = tr("Sorry, we don't yet allow importing notices.");
            break;
        }
        case opentxs::OTPayment::CHEQUE: {
            break; // Instrument is allowed.
        }
        case opentxs::OTPayment::VOUCHER: {
            break; // Instrument is allowed.
        }
        case opentxs::OTPayment::INVOICE: {
            qstrErrorMsg = tr("Sorry, we don't yet allow importing invoices.");
            break;
        }
        case opentxs::OTPayment::PURSE: {
            break; // Instrument is allowed.
        }
        default: {
            const std::string instrument_type{payment->GetTypeString()};
            const QString qstrInstrumentType = QString::fromStdString(instrument_type);
            qstrErrorMsg = QString("%1: %2")
                .arg("Failed to import; unexpected financial instrument type")
                .arg(qstrInstrumentType);
            break;
        }
    }

    if (!qstrErrorMsg.isEmpty()) {
        QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), qstrErrorMsg);
        return;
    }
    // -----------------------------------------------
    opentxs::Identifier recipient_nym_id, sender_nym_id, remitter_nym_id, notary_id;
    const bool has_recipient_nym = payment->GetRecipientNymID(recipient_nym_id);
    const bool has_sender_nym    = payment->GetSenderNymID(sender_nym_id);
    const bool has_remitter_nym  = payment->IsVoucher() && payment->GetRemitterNymID(remitter_nym_id);
    const bool has_notary_id     = payment->GetNotaryID(notary_id);

    auto myNyms = ot_.API().OTAPI().LocalNymList();

    const bool recipientNymIsMine = has_recipient_nym && (1 == myNyms.count(recipient_nym_id));
    const bool senderNymIsMine    = has_sender_nym    && (1 == myNyms.count(sender_nym_id));
    const bool remitterNymIsMine  = has_remitter_nym  && (1 == myNyms.count(remitter_nym_id));

    const bool bISentThisThing = (senderNymIsMine || remitterNymIsMine);

    if (bISentThisThing && has_recipient_nym && !recipientNymIsMine)
    {
        // Todo: We might allow this in the future, in the case where you want
        // to CANCEL a cheque that you wrote. In this spot, the UI might pop up
        // and ask if you want to cancel the cheque.
        //
        QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                             tr("Failed: Cannot import an instrument that you yourself created, "
                                "unless you are also a valid recipient on that instrument (which you are not, in this case). "
                                "Anyway, it should already be in your sent box!"));
        return;
    }
    // -----------------------------------------------
    std::string str_notary_id;

    if (has_notary_id) {
        opentxs::String strNotaryId{notary_id};
        str_notary_id = strNotaryId.Get();
    }
    // -----------------------------------------------

//    bool bPaymentSenderIsNym {false};
//    bool bFromAcctIsAvailable{false};
//
//    Identifier theSenderNymID, theSenderAcctID;
//
//    if (thePayment.IsVoucher()) {
//        bPaymentSenderIsNym =
//            (thePayment.GetRemitterNymID(theSenderNymID) &&
//             nymfile->CompareID(theSenderNymID));
//
//        bFromAcctIsAvailable =
//            thePayment.GetRemitterAcctID(theSenderAcctID);
//    } else {
//        bPaymentSenderIsNym =
//            (thePayment.GetSenderNymID(theSenderNymID) &&
//             nymfile->CompareID(theSenderNymID));
//        bFromAcctIsAvailable =
//            thePayment.GetSenderAcctID(theSenderAcctID);
//    }


    // This is for all financial instruments EXCEPT a cash purse.
    //
//  if (false == payment->IsPurse())
    if (payment->IsCheque() || payment->IsVoucher())
    {
        if (has_recipient_nym)
        {
            if (recipientNymIsMine)
            {
                // Not ready to turn this on yet, since I need to be able to
                // run it both ways in order to test the difference in how
                // the receipts show up in the inbox. (Which was a problem
                // last time I tested it).
                // Once I can verify the receipts are showing up correctly,
                // I will uncomment this and remove the other code.
                //
                // COMING SOON:
                //
//                const opentxs::Identifier taskId = ot_.API().Sync().
//                    DepositPayment(recipient_nym_id, payment);
//                const opentxs::String strTaskId(taskId);
//                const std::string str_task_id{strTaskId.Get()};
//                const QString qstrTaskId{QString::fromStdString(str_task_id)};
//
//                const QString qstrMsg = QString("%1. "
//                                                "%2: %3"
//                                                )
//                                    .arg(tr("Successfully initiated deposit"))
//                                    .arg(tr("Make sure you write down your Task ID"))
//                                    .arg(qstrTaskId);
//
//                QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), qstrMsg);
//                return;

                const opentxs::String otstr_recip(recipient_nym_id);
                const std::string str_recipient_nym_id(otstr_recip.Get());

                opentxs::Identifier unit_type_id;
                payment->GetInstrumentDefinitionID(unit_type_id);
                const opentxs::String otstr_unit(unit_type_id);
                const std::string str_unit_type_id(otstr_unit.Get());

                // Maybe we ALREADY have an account of the correct type, that we
                // can deposit this cheque into. Let's try and look it up.
                //
                DlgChooser theChooser(this);
                theChooser.SetIsAccounts();
                // -----------------------------------------------
                mapIDName & the_map = theChooser.m_map;

                bool bFoundDefault = false;
                // -----------------------------------------------

                // NEW WAY TO ITERATE ACCOUNTS:
                //
                opentxs::OTWallet * pWallet =
                    opentxs::OT::App().API().OTAPI().GetWallet("Moneychanger::mc_import_slot");
                auto accountSet = pWallet->AccountList();

                for (const auto & accountInfo : accountSet)
                {
                    const auto & [ accountID, nymID, serverID, unitID ] = accountInfo;

                    const opentxs::String otstrAccountId{accountID};
                    const opentxs::String otstrNymId{nymID};
                    const opentxs::String otstrServerId{serverID};
                    const opentxs::String otstrUnitId{unitID};

                    const std::string str_account_id{otstrAccountId.Get()};

                    QString OT_acct_id = QString::fromStdString(str_account_id);
                    QString OT_acct_name("");

                    const std::string str_acct_nym_id    = otstrNymId.Get();
                    const std::string str_acct_asset_id  = otstrUnitId.Get();
                    const std::string str_acct_notary_id = otstrServerId.Get();

                    if (!str_recipient_nym_id.empty() && (0 != str_recipient_nym_id.compare(str_acct_nym_id)))
                        continue;
                    if (0 != str_unit_type_id.compare(str_acct_asset_id))
                        continue;
                    if (0 != str_notary_id.compare(str_acct_notary_id))
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
                // -----------------------------------------------
                if (the_map.size() < 1)
                {
//                    QMessageBox::warning(this, tr("No Matching Accounts"),
//                                         tr("The Nym doesn't have any accounts of the appropriate asset type, "
//                                            "on the appropriate server. Please create one and then try again."));

                    // This is where we can just call Justus' function.
                    // (There's no matching account, so the new API stuff can just
                    //  create that account for us).
                    //
                    // This way for now, I can test the new code (by trying it
                    // with zero accounts, which Justus should create) AND I
                    // can compare it to the behavior of the old code (by trying
                    // it when I already have accounts created). That way I can
                    // compare behavior in terms of what receipts show up in the
                    // UI, and make sure that's behaving properly in the new code.
                    //
                    const opentxs::Identifier taskId = opentxs::OT::App().API().Sync().
                        DepositPayment(recipient_nym_id, payment);
                    const opentxs::String strTaskId(taskId);
                    const std::string str_task_id{strTaskId.Get()};
                    const QString qstrTaskId{QString::fromStdString(str_task_id)};

                    const QString qstrMsg = QString("%1. "
                                                    "%2: %3"
                                                    )
                                .arg(tr("Successfully initiated deposit"))
                                .arg(tr("Make sure you write down your Task ID"))
                                .arg(qstrTaskId);

                    QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), qstrMsg);
                    return;
                }
                // -----------------------------------------------
                // There's exactly one matching account in existence already.
                // (With the right Nym, Server, and Unit type to deposit this
                //  cheque).
                //
                else if (1 == the_map.size())
                {
                    const std::string str_deposit_acct_id(the_map.firstKey().toStdString());

                    int32_t nDepositCheque = 0;
                    {
                        MTSpinner theSpinner;

                        opentxs::Identifier deposit_acct_id{str_deposit_acct_id};
                        
                        std::unique_ptr<opentxs::Cheque> cheque = std::make_unique<opentxs::Cheque>();
                        cheque->LoadContractFromString(opentxs::String(strInstrument.c_str()));
                        
                        OT_ASSERT(cheque);

                        auto action = opentxs::OT::App().API().ServerAction().DepositCheque(recipient_nym_id, 
                        		notary_id,
								deposit_acct_id,
								cheque);
                        std::string response = action->Run();
                        
                        nDepositCheque = opentxs::InterpretTransactionMsgReply(str_notary_id, str_recipient_nym_id, str_deposit_acct_id, "import_cash_or_cheque", response);
                    }
                    // --------------------------------------------
                    if (1 == nDepositCheque)
                    {
                        QMessageBox::information(this, tr("Success"), tr("Success depositing cheque or voucher."));
                        emit balancesChanged();
                    }
                    else
                    {
                        const int64_t lUsageCredits = Moneychanger::It()->HasUsageCredits(str_notary_id, str_recipient_nym_id);

                        // In the case of -2 and 0, the problem has to do with the usage credits,
                        // and it already pops up an error box. Otherwise, the user had enough usage
                        // credits, so there must have been some other problem, so we pop up an error box.
                        //
                        if ((lUsageCredits != (-2)) && (lUsageCredits != 0)) {
                            QMessageBox::information(this, tr("Import Failure"), tr("Failed trying to deposit cheque or voucher."));
                        }
                    }
                    return;
                }
                // -----------------------------------------------

//                const opentxs::Identifier taskId = opentxs::OT::App().API().Sync().
//                DepositPayment(recipient_nym_id, payment);
//                const opentxs::String strTaskId(taskId);
//                const std::string str_task_id{strTaskId.Get()};
//                const QString qstrTaskId{QString::fromStdString(str_task_id)};
//
//                const QString qstrMsg = QString("%1. "
//                                                "%2: %3"
//                                                )
//                .arg(tr("Successfully initiated deposit via Sync().DepositPayment"))
//                .arg(tr("Make sure you write down your Task ID"))
//                .arg(qstrTaskId);
//
//                QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), qstrMsg);
//                return;


                // NOTE: The above commented-out code is what we should actually
                // be using.

                if (bFoundDefault && !default_account_id.isEmpty())
                    theChooser.SetPreSelected(default_account_id);
                // -----------------------------------------------
                theChooser.setWindowTitle(tr("Deposit Cheque to Which Account?"));
                // -----------------------------------------------
                if (theChooser.exec() == QDialog::Accepted)
                {
                    if (!theChooser.m_qstrCurrentID.isEmpty())
                    {
                        const std::string str_deposit_acct_id(theChooser.m_qstrCurrentID.toStdString());

                        int32_t nDepositCheque = 0;
                        {
                            MTSpinner theSpinner;

                            opentxs::Identifier deposit_acct_id{str_deposit_acct_id};
                            
                            std::unique_ptr<opentxs::Cheque> cheque = std::make_unique<opentxs::Cheque>();
                            cheque->LoadContractFromString(opentxs::String(strInstrument.c_str()));
                            
                            OT_ASSERT(cheque);

                            auto action = opentxs::OT::App().API().ServerAction().DepositCheque(recipient_nym_id, 
                            		notary_id,
    								deposit_acct_id,
    								cheque);
                            std::string response = action->Run();
                            
                            nDepositCheque = opentxs::InterpretTransactionMsgReply(str_notary_id, str_recipient_nym_id, str_deposit_acct_id, "import_cash_or_cheque", response);
                        }
                        // --------------------------------------------
                        if (1 == nDepositCheque)
                        {
                            QMessageBox::information(this, tr("Success"), tr("Success depositing cheque."));
                            emit balancesChanged();
                        }
                        else
                        {
                            const int64_t lUsageCredits = Moneychanger::It()->HasUsageCredits(str_notary_id, str_recipient_nym_id);

                            // In the case of -2 and 0, the problem has to do with the usage credits,
                            // and it already pops up an error box. Otherwise, the user had enough usage
                            // credits, so there must have been some other problem, so we pop up an error box.
                            //
                            if ((lUsageCredits != (-2)) && (lUsageCredits != 0)) {
                                QMessageBox::information(this, tr("Import Failure"), tr("Failed trying to deposit cheque."));
                                return;
                            }
                        }
                    }
                } // dialog accepted.
            } // recipient Nym is mine.
            // ----------------------------
            //else: There's a recipient Nym, but it's not mine.
            MTNameLookupQT theLookup;
            const opentxs::String strRecipientNymId{recipient_nym_id};
            const std::string str_recipient_nym_id{strRecipientNymId.Get()};
            std::string str_recipient_name = theLookup.GetNymName(str_recipient_nym_id, str_notary_id);

            QString qstrRecipientId = str_recipient_nym_id.empty()
                ? QString("")
                : QString::fromStdString(str_recipient_nym_id);

            QString qstrRecipientName = str_recipient_name.empty()
                ? QString("")
                : QString::fromStdString(str_recipient_name);

            const QString qstrMsg = QString("%1: %2. %3: %4, \"%5\"")
                .arg(tr("Unable to import this instrument"))
                .arg(tr("It has a recipient, but the recipient is NOT you"))
                .arg(tr("Recipient ID and name (if known)"))
                .arg(qstrRecipientId)
                .arg(qstrRecipientName);

            // todo: Use a different dlg here and actually display the
            // unarmored plaintext of the instrument so the user can view it,
            // and copy to clipboard.

            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), qstrMsg);
            return;
        }
        // else has no recipient at all...


        // Here it's NOT a purse (something else)
        // and it does NOT have a recipient Nym.
        //
        // So it might be a cheque with a blank recipient,
        // because we already know it's not cash.

        QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                             tr("Sorry - it's probably a cheque with a blank recipient, and this import dialog can't handle those yet. (Soon!)"));

        //todo, resume  (invoices, plus cheques with no recipient ID).

        // "This is an invoice, requesting payment from you. Do you want to
        // pay it now?"
        //
        // "This cheque has a blank recipient. Do you want to go ahead and
        // deposit it?"

        return;
    }
    // ************************************************************************
    // else CASH PURSE...
    //
    std::string strNotaryID = ot_.API().Exec().Instrmnt_GetNotaryID(strInstrument);


    //resume

    if (strNotaryID.empty())
    {
        QMessageBox::warning(this, tr("Indeterminate Server"),
                             tr("Unable to determine server ID for this instrument. Are you sure it's properly formed?"));
        return;
    }
    // -----------------------
    std::string strInstrumentDefinitionID = ot_.API().Exec().Instrmnt_GetInstrumentDefinitionID(strInstrument);

    if (strInstrumentDefinitionID.empty())
    {
        QMessageBox::warning(this, tr("Indeterminate Asset Type"),
                             tr("Unable to determine asset ID for this instrument. Are you sure it's properly formed?"));
        return;
    }
    // -----------------------
    std::string strServerContract = ot_.API().Exec().LoadServerContract(strNotaryID);

    if (strServerContract.empty())
    {
        QMessageBox::warning(this, tr("No Server Contract Found"),
                             QString("%1 '%2'<br/>%3").arg(tr("Unable to load the server contract for server ID")).
                             arg(QString::fromStdString(strNotaryID)).arg(tr("Are you sure that server contract is even in your wallet?")));
        return;
    }
    // -----------------------
    std::string strAssetContract = ot_.API().Exec().GetAssetType_Contract(strInstrumentDefinitionID);

    if (strAssetContract.empty())
    {
        QMessageBox::warning(this, tr("No Asset Contract Found"),
                             QString("%1 '%2'<br/>%3").arg(tr("Unable to load the asset contract for asset ID")).
                             arg(QString::fromStdString(strInstrumentDefinitionID)).arg(tr("Are you sure that asset type is even in your wallet?")));
        return;
    }
    // -----------------------

    // By this point, we know it's a purse, and we know it has Server
    // and Asset IDs for contracts that are found in this wallet.
    //
    // Next, let's see if the purse is password-protected, and if not,
    // let's see if the recipient Nym is named on the instrument. (He may not be.)
    //
#if OT_CASH
    const bool  bHasPassword = ot_.API().Exec().Purse_HasPassword(strNotaryID, strInstrument);
    std::string strPurseOwner("");

    if (!bHasPassword)
    {
        // If the purse isn't password-protected, then it's DEFINITELY encrypted to
        // a specific Nym.
        //
        // The purse MAY include the NymID for this Nym, but it MAY also be blank, in
        // which case the user will have to select a Nym to TRY.
        //
        strPurseOwner = ot_.API().Exec().Instrmnt_GetRecipientNymID(strInstrument); // TRY and get the Nym ID (it may have been left blank.)

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
            const int32_t nym_count = ot_.API().Exec().GetNymCount();
            // -----------------------------------------------
            for (int32_t ii = 0; ii < nym_count; ++ii)
            {
                //Get OT Nym ID
                QString OT_nym_id = QString::fromStdString(ot_.API().Exec().GetNym_ID(ii));
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
        if (!ot_.API().Exec().IsNym_RegisteredAtServer(strPurseOwner, strNotaryID))
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

    // NEW WAY TO ITERATE ACCOUNTS:
    //
//    opentxs::OTWallet * pWallet = ot_.API().OTAPI().GetWallet("Moneychanger::mc_import_slot");
//    auto accountSet = pWallet->AccountList();
//
//    for (const auto & accountInfo : accountSet)
//    {
//        const auto & [ accountID, nymID, serverID, unitID ] = accountInfo;
//    }

    const int32_t acct_count = ot_.API().Exec().GetAccountCount();
    // -----------------------------------------------
    for (int32_t ii = 0; ii < acct_count; ++ii)
    {
        //Get OT Acct ID
        QString OT_acct_id = QString::fromStdString(ot_.API().Exec().GetAccountWallet_ID(ii));
        QString OT_acct_name("");
        // -----------------------------------------------
        if (!OT_acct_id.isEmpty())
        {
            std::string str_acct_nym_id    = ot_.API().Exec().GetAccountWallet_NymID      (OT_acct_id.toStdString());
            std::string str_acct_asset_id  = ot_.API().Exec().GetAccountWallet_InstrumentDefinitionID(OT_acct_id.toStdString());
            std::string str_acct_notary_id = ot_.API().Exec().GetAccountWallet_NotaryID   (OT_acct_id.toStdString());

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
                strPurseOwner = ot_.API().Exec().GetAccountWallet_NymID(theChooser.m_qstrCurrentID.toStdString());
            // -------------------------------------------
//          const bool bImported = ot_.API().Exec().Wallet_ImportPurse(strNotaryID, strInstrumentDefinitionID, strPurseOwner, strInstrument);

            int32_t nDepositCash = 0;
            {
                MTSpinner theSpinner;

                nDepositCash = ot_.API().Cash().deposit_cash(strNotaryID, strPurseOwner,
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
#else
    return;
#endif  // OT_CASH
}

// -----------------------------------------------
/**
 * Propose Payment Plan
 **/

void Moneychanger::mc_proposeplan_slot()
{
    mc_proposeplan_show_dialog();
}

void Moneychanger::mc_proposeplan_show_dialog(QString qstrAcct/*=QString("")*/, QString qstrContact/*=QString("")*/)
{
    // --------------------------------------------------
    ProposePlanDlg * plan_window = new ProposePlanDlg(NULL);
    plan_window->setAttribute(Qt::WA_DeleteOnClose);
    // --------------------------------------------------
    QString qstr_acct_id = qstrAcct.isEmpty() ? this->get_default_account_id() : qstrAcct;

    if (!qstr_acct_id.isEmpty())
        plan_window->setInitialMyAcct(qstr_acct_id);
    // ---------------------------------------
    plan_window->dialog();
    // --------------------------------------------------
}

void Moneychanger::mc_proposeplan_to_acct(QString qstrAcct)
{
    mc_proposeplan_show_dialog(qstrAcct);
}

void Moneychanger::mc_proposeplan_to_acct_from_contact(QString qstrAcct, QString qstrContact)
{
    mc_proposeplan_show_dialog(qstrAcct, qstrContact);
}





/**
 * Send Funds
 **/

void Moneychanger::mc_sendfunds_slot()
{
    mc_sendfunds_show_dialog();
}

void Moneychanger::mc_sendfunds_show_dialog(QString qstrAcct/*=QString("")*/, QString qstrContact/*=QString("")*/)
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

void Moneychanger::mc_send_from_acct_to_contact(QString qstrAcct, QString qstrContact)
{
    mc_sendfunds_show_dialog(qstrAcct, qstrContact);
}


/**
 * Compose Message
 **/

void Moneychanger::mc_composemessage_slot()
{
    mc_composemessage_show_dialog();
}

void Moneychanger::mc_message_contact_slot(QString qstrFromNym, QString qstrToOpentxsContact) // Compose Message to specific opentxs contact
{
    mc_composemessage_show_dialog(qstrToOpentxsContact, qstrFromNym);
}

void Moneychanger::mc_composemessage_show_dialog(QString qstrToOpentxsContact/*=""*/, QString qstrFromNym/*=""*/)
{
    // --------------------------------------------------
    MTCompose * compose_window = new MTCompose;
    compose_window->setAttribute(Qt::WA_DeleteOnClose);
    // --------------------------------------------------
    // If Moneychanger has a default Nym set, we use that for the Sender.
    // (User can always change it.)
    //
    if (qstrFromNym.isEmpty())
    {
        QString qstrDefaultNym = this->get_default_nym_id();

        if (!qstrDefaultNym.isEmpty()) // Sender Nym is set.
            qstrFromNym = qstrDefaultNym;
    }
    compose_window->setInitialSenderNym(qstrFromNym);
    // --------------------------------------------------
    if (!qstrToOpentxsContact.isEmpty()) {
        compose_window->setInitialRecipientContactID(qstrToOpentxsContact);
    }
    // --------------------------------------------------
    else {
        QString qstrDefaultServer = this->get_default_notary_id();

        if (!qstrDefaultServer.isEmpty())
            compose_window->setInitialServer(qstrDefaultServer);
    }
    // --------------------------------------------------
    compose_window->dialog();
    Focuser f(compose_window);
    f.show();
    f.focus();
    // --------------------------------------------------
}


void Moneychanger::mc_messages_slot()
{
    mc_messages_dialog();
}

void Moneychanger::mc_messages_dialog()
{
    if (!messages_window)
    {
        messages_window = new Messages(this);

        connect(messages_window, SIGNAL(needToDownloadMail()),          this,            SLOT(onNeedToDownloadMail()));
        connect(this,            SIGNAL(populatedRecordlist()),         messages_window, SLOT(onRecordlistPopulated()));
        connect(this,            SIGNAL(claimsUpdatedForNym(QString)),  messages_window, SLOT(onClaimsUpdatedForNym(QString)));
    }
    // ---------------------------------
    messages_window->dialog();
}


void Moneychanger::mc_payments_slot()
{
    mc_payments_dialog();
}

void Moneychanger::mc_show_payment_slot(int nSourceRow, int nFolder)
{
    mc_payments_dialog(nSourceRow, nFolder);
}

void Moneychanger::mc_payments_dialog(int nSourceRow/*=-1*/, int nFolder/*=-1*/)
{
    if (!payments_window)
    {
        payments_window = new Payments(this);

        connect(payments_window, SIGNAL(showDashboard()),               this,            SLOT(mc_overview_slot()));
        connect(payments_window, SIGNAL(needToDownloadAccountData()),   this,            SLOT(onNeedToDownloadAccountData()));
        connect(this,            SIGNAL(populatedRecordlist()),         payments_window, SLOT(onRecordlistPopulated()));
        connect(payments_window, SIGNAL(needToPopulateRecordlist()),    this,            SLOT(onNeedToPopulateRecordlist()));
        connect(this,            SIGNAL(balancesChanged()),             payments_window, SLOT(onBalancesChanged()));
        connect(this,            SIGNAL(claimsUpdatedForNym(QString)),  payments_window, SLOT(onClaimsUpdatedForNym(QString)));

    }
    // ---------------------------------
    payments_window->dialog(nSourceRow, nFolder);
}


void Moneychanger::mc_activity_slot()
{
    mc_activity_dialog();
}

void Moneychanger::mc_agreements_slot()
{
    mc_agreements_dialog();
}

void Moneychanger::mc_show_agreement_slot(int nSourceRow, int nFolder)
{
    mc_agreements_dialog(nSourceRow, nFolder);
}

void Moneychanger::mc_activity_dialog()
{
    if (!activity_window)
    {
        activity_window = new Activity(this);

        connect(activity_window, SIGNAL(showDashboard()),               this,              SLOT(mc_overview_slot()));
        connect(activity_window, SIGNAL(needToDownloadAccountData()),   this,              SLOT(onNeedToDownloadAccountData()));
        connect(this,            SIGNAL(populatedRecordlist()),         activity_window,   SLOT(onRecordlistPopulated()));
        connect(activity_window, SIGNAL(needToPopulateRecordlist()),    this,              SLOT(onNeedToPopulateRecordlist()));
        connect(this,            SIGNAL(balancesChanged()),             activity_window,   SLOT(onBalancesChanged()));
        connect(this,            SIGNAL(claimsUpdatedForNym(QString)),  activity_window,   SLOT(onClaimsUpdatedForNym(QString)));
    }
    // ---------------------------------
    activity_window->dialog();
}

void Moneychanger::mc_agreements_dialog(int nSourceRow/*=-1*/, int nFolder/*=-1*/)
{
    if (!agreements_window)
    {
        agreements_window = new Agreements(this);

        connect(agreements_window, SIGNAL(showDashboard()),               this,              SLOT(mc_overview_slot()));
        connect(agreements_window, SIGNAL(needToDownloadAccountData()),   this,              SLOT(onNeedToDownloadAccountData()));
        connect(this,              SIGNAL(populatedRecordlist()),         agreements_window, SLOT(onRecordlistPopulated()));
        connect(agreements_window, SIGNAL(needToPopulateRecordlist()),    this,              SLOT(onNeedToPopulateRecordlist()));
        connect(this,              SIGNAL(balancesChanged()),             agreements_window, SLOT(onBalancesChanged()));
        connect(this,              SIGNAL(claimsUpdatedForNym(QString)),  agreements_window, SLOT(onClaimsUpdatedForNym(QString)));

    }
    // ---------------------------------
    agreements_window->dialog(nSourceRow, nFolder);
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

        connect(homewindow, SIGNAL(needToPopulateRecordlist()),
                this,       SLOT(onNeedToPopulateRecordlist()));

        connect(this,       SIGNAL(populatedRecordlist()),
                homewindow, SLOT(onRecordlistPopulated()));

        connect(this,       SIGNAL(balancesChanged()),
                homewindow, SLOT(onBalancesChanged()));

        qDebug() << "Overview Opened";
    }
    // ---------------------------------
    homewindow->dialog();
}
// End Overview


void Moneychanger::onServersChanged()
{
    // Because the Nym details page has a list of servers that Nym is registered on.
    // So if we've added a new Server, we should update that page, if it's open.
    if (nullptr != nymswindow)
        nymswindow->RefreshRecords();

    if (menuwindow)
        menuwindow->refreshOptions();
}


void Moneychanger::onAssetsChanged()
{
    if (menuwindow)
        menuwindow->refreshOptions();
}


void Moneychanger::onNymsChanged()
{
    if (menuwindow)
        menuwindow->refreshOptions();
}


void Moneychanger::onAccountsChanged()
{
    if (menuwindow)
        menuwindow->refreshOptions();
}

void Moneychanger::onNewNymAdded(QString qstrID)
{
    // Add a new Contact in the Address Book for this Nym as well.
    // (When testing, it's a pain having to add my own Nyms to the address book
    // by hand for sending payments between them, every time I create a new Nym.)

    QString qstrNymName("");

    if (!qstrID.isEmpty())
    {
        MTNameLookupQT theLookup;
        qstrNymName = QString::fromStdString(theLookup.GetNymName(qstrID.toStdString(), ""));
        if (!qstrNymName.isEmpty())
            qstrNymName += QString(" (%1)").arg(tr("Me"));
//      QString qstrContactID  = MTContactHandler::getInstance()->GetOrCreateOpentxsContactBasedOnNym(qstrNymName, qstrID, "");

        int nContactId = MTContactHandler::getInstance()->CreateContactBasedOnNym(qstrID);

        if (!qstrNymName.isEmpty() && (nContactId > 0))
        {
            MTContactHandler::getInstance()->SetContactName(nContactId, qstrNymName);

            if (nullptr != contactswindow)
                mc_addressbook_show();
        }
        // --------------------------------------------------
        GetRecordlist().AddNymID(qstrID.toStdString());
    }

    onNymsChanged();
}

void Moneychanger::onNewAccountAdded(QString qstrID)
{
    GetRecordlist().AddAccountID(qstrID.toStdString());
    onAccountsChanged();
}



// --------------------------------------------------

void Moneychanger::onNewServerAdded(QString qstrID)
{
    GetRecordlist().AddNotaryID(qstrID.toStdString());

    onServersChanged();
}

void Moneychanger::onNewAssetAdded(QString qstrID)
{
    GetRecordlist().AddInstrumentDefinitionID(qstrID.toStdString());

    onAssetsChanged();
}


void Moneychanger::PublicNymNotify(std::string id)
{
        auto pNym = ot_.Wallet().Nym(opentxs::Identifier(id));

        if (pNym)
        {
            qDebug() << "I was notified that the DHT downloaded Nym "
                     << QString::fromStdString(id);
        }
        else // The Nym is not already in the wallet.
        {
            // I don't think I have to do anything at all then.
            // No need to reload the wallet, since the Nym isn't loaded
            // in the wallet, and if it ever does load that Nym in the
            // near future, it will get the latest version then.
        }
        emit nymWasJustChecked(QString::fromStdString(id));
}

void Moneychanger::ServerContractNotify(std::string id)
{
    auto pContract =
        ot_.Wallet().Server(opentxs::Identifier(id));

    if (pContract) {
        qDebug() << "I was notified that the DHT downloaded contract "
                 << QString::fromStdString(id);
        emit serversChanged();
    }
    else {
        qDebug() << "Strange: I was notified that we downloaded contract "
                 << QString::fromStdString(id)
                 << " but then failed trying to load it up.";
    }
}


void Moneychanger::AssetContractNotify(std::string id)
{
    const opentxs::Identifier ot_id(id);

    auto pContract = ot_.Wallet().UnitDefinition(ot_id);

    if (pContract) {
        qDebug() << "I was notified that the DHT downloaded contract "
                 << QString::fromStdString(id);
        emit assetsChanged();
    }
    else {
        qDebug() << "Strange: I was notified that we downloaded contract "
                 << QString::fromStdString(id)
                 << " but then failed trying to load it up.";
    }
}

// --------------------------------------------------

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

void Moneychanger::mc_main_menu_dialog(bool bShow/*=true*/)
{
    if (!menuwindow)
    {
        // --------------------------------------------------
        menuwindow = new DlgMenu(this);
        // --------------------------------------------------
        QCoreApplication * pCore = QCoreApplication::instance();

        connect(pCore,      SIGNAL(aboutToQuit()),
                menuwindow, SLOT(onAboutToQuit()));
        // --------------------------------------------------
        connect(menuwindow, SIGNAL(sig_on_toolButton_payments_clicked()),
                this,       SLOT(mc_payments_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_messages_clicked()),
                this,       SLOT(mc_messages_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_pending_clicked()),
                this,       SLOT(mc_overview_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_markets_clicked()),
                this,       SLOT(mc_market_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_trade_archive_clicked()),
                this,       SLOT(mc_trade_archive_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_secrets_clicked()),
                this,       SLOT(mc_passphrase_manager_slot()));

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
                this,       SLOT(mc_smartcontract_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_Corporations_clicked()),
                this,       SLOT(mc_corporation_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_Transport_clicked()),
                this,       SLOT(mc_transport_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_settings_clicked()),
                this,       SLOT(mc_settings_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_quit_clicked()),
                this,       SLOT(mc_shutdown_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_encrypt_clicked()),
                this,       SLOT(mc_crypto_encrypt_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_sign_clicked()),
                this,       SLOT(mc_crypto_sign_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_decrypt_clicked()),
                this,       SLOT(mc_crypto_decrypt_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_requestPayment_clicked()),
                this,       SLOT(mc_requestfunds_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_recurringPayment_clicked()),
                this,       SLOT(mc_proposeplan_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_liveAgreements_clicked()),
                this,       SLOT(mc_agreements_slot()));

        connect(menuwindow, SIGNAL(sig_on_toolButton_activity_clicked()),
                this,       SLOT(mc_activity_slot()));

    }
    // ---------------------------------
    if (bShow)
    {
        if (!menuwindow->isVisible())
            menuwindow->setVisible(true);
        menuwindow->dialog();
    }
    else
    {
        menuwindow->setVisible(false);
    }
}

// End Main Menu





/**
 * Agreement Window
 **/

void Moneychanger::mc_smartcontract_slot()
{
    mc_smartcontract_dialog();
}

void Moneychanger::mc_smartcontract_dialog()
{
    // -------------------------------------
    if (!smartcontract_window)
        smartcontract_window = new MTDetailEdit(this);
    // -------------------------------------
    mapIDName & the_map = smartcontract_window->m_map;
    // -------------------------------------
    the_map.clear();
    // -------------------------------------
    MTContactHandler::getInstance()->GetSmartContracts(the_map);
    // -------------------------------------
    smartcontract_window->setWindowTitle(tr("Smart Contracts"));
    // -------------------------------------
    smartcontract_window->dialog(MTDetailEdit::DetailEditTypeAgreement);
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
        connect(market_window, SIGNAL(needToDownloadAccountData()),
                this,       SLOT(onNeedToDownloadAccountData()));
        connect(market_window, SIGNAL(needToDownloadSingleAcct(QString, QString)),
                this,       SLOT(onNeedToDownloadSingleAcct(QString, QString)));
        connect(market_window, SIGNAL(needToDisplayTradeArchive()),
                this,          SLOT(mc_trade_archive_slot()));
        connect(this,          SIGNAL(balancesChanged()),
                market_window, SLOT(onBalancesChangedFromAbove()));
    }
    // ------------------------------------
    market_window->dialog();
}



void Moneychanger::mc_trade_archive_slot()
{
    mc_trade_archive_dialog();
}

void Moneychanger::mc_trade_archive_dialog()
{
    if (!trade_archive_window)
    {
        trade_archive_window = new DlgTradeArchive(this);

//        connect(this,                 SIGNAL(balancesChanged()),
//                trade_archive_window, SLOT(onBalancesChangedFromAbove()));
    }
    // ------------------------------------
    trade_archive_window->dialog();
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
    Focuser f(createinsurancecompany_window);
    f.show();
    f.focus();
}

void Moneychanger::onConfirmSmartContract(QString qstrTemplate, QString qstrLawyerID, int32_t index)
{
    if (qstrTemplate.isEmpty())
    {
        qDebug() << "onConfirmSmartContract: Strange, the smart contract was empty. (Failure.)";
        return;
    }

    std::string str_template = qstrTemplate.toStdString();
    // ------------------------------------------------
    if (qstrLawyerID.isEmpty())
        qstrLawyerID = get_default_nym_id();
    // ------------------------------------------------
    if (0 == ot_.API().Exec().Smart_GetPartyCount(str_template))
    {
       QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("There are no parties listed on this smart contract, so you cannot sign it as a party."));
       return;
    }
    // ------------------------------------------------
    if (ot_.API().Exec().Smart_AreAllPartiesConfirmed(str_template))
    {
        QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("Strange, all parties are already confirmed on this contract. (Failure.)"));
        return;
    }
    // ------------------------------------------------
    std::string str_server = ot_.API().Exec().Instrmnt_GetNotaryID(str_template);
    // ------------------------------------------------
    WizardConfirmSmartContract theWizard(this);

    theWizard.setWindowTitle(tr("Confirm smart contract"));

    QString qstrNotaryID("");

    if (!str_server.empty())
    {
        qstrNotaryID = QString::fromStdString(str_server);
    }
    else
    {
        qDebug() << "onConfirmSmartContract: Failed trying to get NotaryID from smart contract. (Failure.)";
        return;
    }
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

//    QString qstrNotaryID = theWizard.field("NotaryID").toString();
//    str_server = qstrNotaryID.toStdString();
    // -----------------------------------------
    QString qstrPartyName  = theWizard.field("PartyName") .toString();
    std::string str_party  = qstrPartyName.toStdString();
    // ---------------------------------------------------
    // By this point the user has selected the server ID, the Nym ID, and the Party Name.
    //
    std::string serverFromContract = ot_.API().Exec().Instrmnt_GetNotaryID(str_template);
    if ("" != serverFromContract && str_server != serverFromContract) {
        QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("Mismatched server ID in contract. (Failure.)"));
        return;
    }
    // ----------------------------------------------------
    if (!ot_.API().Exec().IsNym_RegisteredAtServer(str_lawyer_id, str_server)) {
        QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("Nym is not registered on server. (Failure.)"));
        return;
    }
    // ----------------------------------------------------
    // See if there are accounts for that party via Party_GetAcctCount.
    // If there are, confirm those accounts.
    //
    int32_t nAccountCount = ot_.API().Exec().Party_GetAcctCount(str_template, str_party);

    if (0 >= nAccountCount)
    {
        QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("Strange, the chosen party has no accounts named on this smart contract. (Failed.)"));
        return;
    }
    // ----------------------------------------------------
    // Need to LOOP here until all the accounts are confirmed.

    mapIDName mapConfirmed, mapAgents;

    while (nAccountCount > 0)
    {
        WizardPartyAcct otherWizard(this);

        otherWizard.setWindowTitle(tr("Confirm smart contract"));

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
        std::string agentName = ot_.API().Exec().Party_GetAcctAgentName(str_template, str_party, str_acct_name);

        if ("" == agentName)
            agentName = ot_.API().Exec().Party_GetAgentNameByIndex(str_template, str_party, 0);

        if ("" == agentName)
        {
            QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("Strange, but apparently this smart contract doesn't have any agents for this party. (Failed.)"));
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

        needed += ot_.API().Exec().SmartContract_CountNumsNeeded(str_template, qstrAgent.toStdString());
    }
    // --------------------------------------------

    const opentxs::Identifier notaryID{str_server}, nymID{str_lawyer_id};
    if (!ot_.API().ServerAction().GetTransactionNumbers(nymID, notaryID, needed + 1))
    {
        QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("Failed trying to reserve enough transaction numbers from the notary."));
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
        if ("" == myAcctID && (0 == qstr_default_acct_id.compare(x.value())))
        {
            myAcctID = qstr_default_acct_id.toStdString();
            QString qstrMyAcctAgentName = mapAgents[x.key()];
            myAcctAgentName = qstrMyAcctAgentName.toStdString();
        }

        QString qstrCurrentAcctName  = x.key();
        QString qstrCurrentAcctID    = x.value();
        QString qstrCurrentAgentname = mapAgents[x.key()];

        // confirm a theoretical acct by giving it a real acct id.
        std::string confirmed = ot_.API().Exec().SmartContract_ConfirmAccount(
            str_template, str_lawyer_id, str_party, qstrCurrentAcctName.toStdString(), qstrCurrentAgentname.toStdString(), qstrCurrentAcctID.toStdString());

        if ("" == confirmed)
        {
            qDebug() << "Failure while calling "
                     "OT_API_SmartContract_ConfirmAccount. Acct Name: "
                  << qstrCurrentAcctName << "  Agent Name: " << qstrCurrentAgentname
                  << "  Acct ID: " << qstrCurrentAcctID << " \n";

            QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("Failed while calling OT_API_SmartContract_ConfirmAccount."));

            ot_.API().Exec().Msg_HarvestTransactionNumbers(str_template, str_lawyer_id, false, false, false, false, false);

            return;
        }

        str_template = confirmed;
    }
    // ----------------------------------------
    // Then we try to activate it or pass on to the next party.

    std::string confirmed =
        ot_.API().Exec().SmartContract_ConfirmParty(str_template, str_party, str_lawyer_id, str_server);

    if ("" == confirmed)
    {
        qDebug() << "Error: cannot confirm smart contract party.\n";
        QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("Failed while calling SmartContract_ConfirmParty."));
        ot_.API().Exec().Msg_HarvestTransactionNumbers(str_template, str_lawyer_id, false, false, false, false, false);
        return;
    }

    if (ot_.API().Exec().Smart_AreAllPartiesConfirmed(confirmed))
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
        ot_.API().Exec().Msg_HarvestTransactionNumbers(str_template, str_lawyer_id, false, false, false, false, false);
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
        ot_.API().Exec().Msg_HarvestTransactionNumbers(str_template, str_lawyer_id, false, false, false, false, false);
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
        ot_.API().Exec().RecordPayment(str_server, str_lawyer_id, true, index, false);
    }
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
    if (0 == ot_.API().Exec().Smart_GetPartyCount(str_template))
    {
       QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("There are no parties listed on this smart contract, so you cannot sign it as a party."));
       return;
    }
    // ------------------------------------------------
    if (ot_.API().Exec().Smart_AreAllPartiesConfirmed(str_template))
    {
        QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("Strange, all parties are already confirmed on this contract. (Failure.)"));
        return;
    }
    // ------------------------------------------------
    std::string str_server = ot_.API().Exec().Instrmnt_GetNotaryID(str_template);
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
    std::string serverFromContract = ot_.API().Exec().Instrmnt_GetNotaryID(str_template);
    if ("" != serverFromContract && str_server != serverFromContract) {
        QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("Mismatched server ID in contract. (Failure.)"));
        return;
    }
    // ----------------------------------------------------
    if (!ot_.API().Exec().IsNym_RegisteredAtServer(str_lawyer_id, str_server)) {
        QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("Nym is not registered on server. (Failure.)"));
        return;
    }
    // ----------------------------------------------------
    // See if there are accounts for that party via Party_GetAcctCount.
    // If there are, confirm those accounts.
    //
    int32_t nAccountCount = ot_.API().Exec().Party_GetAcctCount(str_template, str_party);

    if (0 >= nAccountCount)
    {
        QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("Strange, the chosen party has no accounts named on this smart contract. (Failed.)"));
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
        std::string agentName = ot_.API().Exec().Party_GetAcctAgentName(str_template, str_party, str_acct_name);

        if ("" == agentName)
            agentName = ot_.API().Exec().Party_GetAgentNameByIndex(str_template, str_party, 0);

        if ("" == agentName)
        {
            QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("Strange, but apparently this smart contract doesn't have any agents for this party. (Failed.)"));
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

        needed += ot_.API().Exec().SmartContract_CountNumsNeeded(str_template, qstrAgent.toStdString());
    }
    // --------------------------------------------

    const opentxs::Identifier notaryID{str_server}, nymID{str_lawyer_id};
    if (!ot_.API().ServerAction().GetTransactionNumbers(nymID, notaryID, needed + 1))
    {
        QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("Failed trying to reserve enough transaction numbers from the notary."));
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
        if ("" == myAcctID && (0 == qstr_default_acct_id.compare(x.value())))
        {
            myAcctID = qstr_default_acct_id.toStdString();
            QString qstrMyAcctAgentName = mapAgents[x.key()];
            myAcctAgentName = qstrMyAcctAgentName.toStdString();
        }

        QString qstrCurrentAcctName  = x.key();
        QString qstrCurrentAcctID    = x.value();
        QString qstrCurrentAgentname = mapAgents[x.key()];

        // confirm a theoretical acct by giving it a real acct id.
        std::string confirmed = ot_.API().Exec().SmartContract_ConfirmAccount(
            str_template, str_lawyer_id, str_party, qstrCurrentAcctName.toStdString(), qstrCurrentAgentname.toStdString(), qstrCurrentAcctID.toStdString());

        if ("" == confirmed)
        {
            qDebug() << "Failure while calling "
                     "OT_API_SmartContract_ConfirmAccount. Acct Name: "
                  << qstrCurrentAcctName << "  Agent Name: " << qstrCurrentAgentname
                  << "  Acct ID: " << qstrCurrentAcctID << " \n";

            QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("Failed while calling OT_API_SmartContract_ConfirmAccount."));

            ot_.API().Exec().Msg_HarvestTransactionNumbers(str_template, str_lawyer_id, false, false, false, false, false);

            return;
        }

        str_template = confirmed;
    }
    // ----------------------------------------
    // Then we try to activate it or pass on to the next party.

    std::string confirmed =
        ot_.API().Exec().SmartContract_ConfirmParty(str_template, str_party, str_lawyer_id, str_server);

    if ("" == confirmed)
    {
        qDebug() << "Error: cannot confirm smart contract party.\n";
        QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("Failed while calling SmartContract_ConfirmParty."));
        ot_.API().Exec().Msg_HarvestTransactionNumbers(str_template, str_lawyer_id, false, false, false, false, false);
        return;
    }

    if (ot_.API().Exec().Smart_AreAllPartiesConfirmed(confirmed))
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
        ot_.API().Exec().Msg_HarvestTransactionNumbers(str_template, str_lawyer_id, false, false, false, false, false);
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
        ot_.API().Exec().Msg_HarvestTransactionNumbers(str_template, str_lawyer_id, false, false, false, false, false);
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
        ot_.API().Exec().RecordPayment(str_server, str_lawyer_id, true, index, false);
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
        int32_t acct_count = ot_.API().Exec().Party_GetAcctCount(contract, name);

        for (int32_t i = 0; i < acct_count; i++)
        {
            std::string acctName = ot_.API().Exec().Party_GetAcctNameByIndex(contract, name, i);
            QString qstrAcctName = QString::fromStdString(acctName);
            // -----------------------------------------------
            std::string partyAcctID = ot_.API().Exec().Party_GetAcctID(contract, name, acctName);
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
            return ot_.API().Exec().Msg_HarvestTransactionNumbers(contract, mynym, false, false, false, false, false);
        }
        // ------------------------------------------------
        std::string acctName = theChooser.m_qstrCurrentName.toStdString();
        if ("" == acctName) {
            qDebug() << "Error: account name empty on smart contract.\n";
            return ot_.API().Exec().Msg_HarvestTransactionNumbers(contract, mynym, false, false, false, false, false);
        }

        myAcctID = theChooser.m_qstrCurrentID.toStdString();

        myAcctAgentName = ot_.API().Exec().Party_GetAcctAgentName(contract, name, acctName);
        if ("" == myAcctAgentName) {
            qDebug() << "Error: account agent is not yet confirmed.\n";
            return ot_.API().Exec().Msg_HarvestTransactionNumbers(contract, mynym, false, false, false, false, false);
        }
    }

    const opentxs::Identifier notaryID{server}, myNymID{mynym}, accountID{myAcctID};
    std::unique_ptr<opentxs::OTSmartContract> smartContract = std::make_unique<opentxs::OTSmartContract>();

    OT_ASSERT(smartContract)

    smartContract->LoadContractFromString(opentxs::String(contract));
    auto action = ot_.API().ServerAction().ActivateSmartContract(myNymID, notaryID, accountID, myAcctAgentName, smartContract);
    std::string response = action->Run();
    
    if (1 != opentxs::VerifyMessageSuccess(response))
    {
        qDebug() << "Error: cannot activate smart contract.\n";
        return ot_.API().Exec().Msg_HarvestTransactionNumbers(contract, mynym, false, false, false, false, false);
    }

    // BELOW THIS POINT, the transaction has definitely processed.

    int32_t reply = opentxs::InterpretTransactionMsgReply(server, mynym, myAcctID, "activate_smart_contract", response);

    if (1 != reply) {
        return reply;
    }

    if (!ot_.API().ServerAction().DownloadAccount(myNymID, notaryID, accountID, true)) {
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

    std::unique_ptr<opentxs::OTPayment> payment =
        std::make_unique<opentxs::OTPayment>(opentxs::String(contract.c_str()));
    
    OT_ASSERT(payment);
    
    auto action = ot_.API().ServerAction().SendPayment(opentxs::Identifier(mynym), opentxs::Identifier(server), opentxs::Identifier(hisNymID), payment);
    std::string response = action->Run();
    
    if (1 != opentxs::VerifyMessageSuccess(response)) {
        qDebug() << "\nFor whatever reason, our attempt to send the instrument on "
                 "to the next user has failed.\n";
        QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("Failed while calling send_user_payment."));
        return ot_.API().Exec().Msg_HarvestTransactionNumbers(contract, mynym, false, false, false, false, false);
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
    QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("Success sending the agreement on to the next party."));

    return 1;
}


bool Moneychanger::showPartyAccounts(const std::string& contract, const std::string& name, std::string & str_output)
{
    std::ostringstream os;

    int32_t accounts = ot_.API().Exec().Party_GetAcctCount(contract, name);

    if (0 > accounts) {
        qDebug() << QString("Error: Party '%1' has bad value for number of asset accounts.").arg(QString::fromStdString(name));
        return false;
    }

    for (int32_t i = 0; i < accounts; i++)
    {
        std::string acctName =
            ot_.API().Exec().Party_GetAcctNameByIndex(contract, name, i);
        if ("" == acctName) {
            qDebug() << QString("Error: Failed retrieving Asset Account Name from party '%1' at account index: %2")
                        .arg(QString::fromStdString(name)).arg(i);
            return false;
        }

        std::string acctInstrumentDefinitionID =
            ot_.API().Exec().Party_GetAcctInstrumentDefinitionID(contract, name,
                                                            acctName);
        if ("" != acctInstrumentDefinitionID) {
            os << "-------------------\nAccount '" << acctName << "' (index "
                 << i << " on Party '" << name
                 << "') has instrument definition: "
                 << acctInstrumentDefinitionID << " ("
                 << ot_.API().Exec().GetAssetType_Name(acctInstrumentDefinitionID)
                 << ")\n";
        }

        std::string acctID = ot_.API().Exec().Party_GetAcctID(contract, name, acctName);
        if ("" != acctID) {
            os << "Account '" << acctName << "' (party '" << name
                 << "') is confirmed as Account ID: " << acctID << " ("
                 << ot_.API().Exec().GetAccountWallet_Name(acctID) << ")\n";
        }

        std::string strAcctAgentName =
            ot_.API().Exec().Party_GetAcctAgentName(contract, name, acctName);
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
    connect(settingswindow, SIGNAL(expertModeUpdated(bool)), this, SIGNAL(expertModeUpdated(bool)));
    connect(settingswindow, SIGNAL(hideNavUpdated(bool)),    this, SLOT(onHideNavUpdated(bool)));
    // ------------------------------------
    Focuser f(settingswindow);
    f.show();
    f.focus();
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
    Focuser f(bitcoinwindow);
    f.show();
    f.focus();
}

/**
  * (Bitcoin ->) Connect to wallet
  **/
void Moneychanger::mc_bitcoin_connect_slot()
{
    if(!bitcoinConnectWindow)
        bitcoinConnectWindow = new BtcConnectDlg(this);
    Focuser f(bitcoinConnectWindow);
    f.show();
    f.focus();
}

/**
  * (Bitcoin ->) Pools
  **/
void Moneychanger::mc_bitcoin_pools_slot()
{
    if(!bitcoinPoolWindow)
        bitcoinPoolWindow = new BtcPoolManager(this);
    Focuser f(bitcoinPoolWindow);
    f.show();
    f.focus();
}

/**
  * (Bitcoin ->) Transactions
  **/
void Moneychanger::mc_bitcoin_transactions_slot()
{
    if(!bitcoinTxWindow)
        bitcoinTxWindow = new BtcTransactionManager(this);
    Focuser f(bitcoinTxWindow);
    f.show();
    f.focus();
}

/**
  * (Bitcoin ->) Send
  **/
void Moneychanger::mc_bitcoin_send_slot()
{
    if(!bitcoinSendWindow)
        bitcoinSendWindow = new BtcSendDlg(this);
    Focuser f(bitcoinSendWindow);
    f.show();
    f.focus();
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
    else
        send_window->setInitialAmount("0");

    if(!qstrRecipientNym.isEmpty())
        send_window->setInitialHisNym(qstrRecipientNym);
    // ---------------------------------------
//    Focuser f(send_window);
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

void Moneychanger::mc_rpc_messages_show_dialog(){
    mc_messages_dialog();
}

void Moneychanger::mc_rpc_exchange_show_dialog(){
    mc_market_dialog();
}

//void mc_rpc_payments_show_dialog();
void Moneychanger::mc_rpc_manage_accounts_show_dialog(){
    mc_accountmanager_dialog();
}

void Moneychanger::mc_rpc_manage_nyms_show_dialog(){
    mc_nymmanager_dialog();
}

void Moneychanger::mc_rpc_manage_assets_show_dialog(){
    mc_assetmanager_dialog();
}

void Moneychanger::mc_rpc_manage_smartcontracts_show_dialog(){

}


