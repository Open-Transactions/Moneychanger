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
#include <core/handlers/modeltradearchive.hpp>

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
#include <gui/ui/dlgtradearchive.hpp>
#include <gui/ui/dlgencrypt.hpp>
#include <gui/ui/dlgdecrypt.hpp>
#include <gui/ui/dlgpassphrasemanager.hpp>
#include <gui/ui/messages.hpp>
#include <gui/ui/payments.hpp>



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
#include <QFlags>

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
    mc_systrayIcon_trade_archive = QIcon(":/icons/overview");



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
    setupRecordList();

    mc_overall_init = true;
}


Moneychanger::~Moneychanger()
{
    delete nmc_update_timer;
    delete nmc_names;
    delete nmc;
}

opentxs::OTRecordList & Moneychanger::GetRecordlist()
{
    return m_list;
}

void Moneychanger::setupRecordList()
{
    int nServerCount  = opentxs::OTAPI_Wrap::It()->GetServerCount();
    int nAssetCount   = opentxs::OTAPI_Wrap::It()->GetAssetTypeCount();
    int nNymCount     = opentxs::OTAPI_Wrap::It()->GetNymCount();
    int nAccountCount = opentxs::OTAPI_Wrap::It()->GetAccountCount();
    // ----------------------------------------------------
    GetRecordlist().ClearServers();
    GetRecordlist().ClearAssets();
    GetRecordlist().ClearNyms();
    GetRecordlist().ClearAccounts();
    // ----------------------------------------------------
    for (int ii = 0; ii < nServerCount; ++ii)
    {
        std::string NotaryID = opentxs::OTAPI_Wrap::It()->GetServer_ID(ii);
        GetRecordlist().AddNotaryID(NotaryID);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nAssetCount; ++ii)
    {
        std::string InstrumentDefinitionID = opentxs::OTAPI_Wrap::It()->GetAssetType_ID(ii);
        GetRecordlist().AddInstrumentDefinitionID(InstrumentDefinitionID);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nNymCount; ++ii)
    {
        std::string nymId = opentxs::OTAPI_Wrap::It()->GetNym_ID(ii);
        GetRecordlist().AddNymID(nymId);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nAccountCount; ++ii)
    {
        std::string accountID = opentxs::OTAPI_Wrap::It()->GetAccountWallet_ID(ii);
        GetRecordlist().AddAccountID(accountID);
    }
    // ----------------------------------------------------
    GetRecordlist().AcceptChequesAutomatically  (true);
    GetRecordlist().AcceptReceiptsAutomatically (true);
    GetRecordlist().AcceptTransfersAutomatically(false);
}

// Calls OTRecordList::Populate(), and then additionally adds records from Bitmessage, etc.
//
void Moneychanger::populateRecords()
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
    // Pop up the payments screen.
    mc_payments_dialog();
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
//    mc_systrayMenu_headertext = new QAction(tr("Moneychanger"), mc_systrayMenu);
//    mc_systrayMenu_headertext->setDisabled(1);
//    mc_systrayMenu->addAction(mc_systrayMenu_headertext);
    // --------------------------------------------------------------
    SetupAccountMenu(mc_systrayMenu);
    SetupNymMenu(mc_systrayMenu);
    // --------------------------------------------------------------
    mc_systrayMenu->addSeparator();
    // --------------------------------------------------------------
    SetupPaymentsMenu(mc_systrayMenu);
    SetupMessagingMenu(mc_systrayMenu);
    // --------------------------------------------------------------
    mc_systrayMenu->addSeparator();
    // --------------------------------------------------------------
    //Passphrase Manager
    mc_systrayMenu_passphrase_manager = new QAction(mc_systrayIcon_crypto, tr("Secrets"), mc_systrayMenu);
    mc_systrayMenu->addAction(mc_systrayMenu_passphrase_manager);
    connect(mc_systrayMenu_passphrase_manager, SIGNAL(triggered()), this, SLOT(mc_passphrase_manager_slot()));
    // --------------------------------------------------------------
    mc_systrayMenu->addSeparator();
    // --------------------------------------------------------------
    //Blank
    //            mc_systrayMenu_aboveBlank = new QAction(" ", 0);
    //            mc_systrayMenu_aboveBlank->setDisabled(1);
    //            mc_systrayMenu->addAction(mc_systrayMenu_aboveBlank);
    // --------------------------------------------------------------
    //Separator
    mc_systrayMenu->addSeparator();
    // --------------------------------------------------------------
    SetupExchangeMenu(mc_systrayMenu);
    SetupContractsMenu(mc_systrayMenu);
    // --------------------------------------------------------------
    mc_systrayMenu->addSeparator();
    // --------------------------------------------------------------
    SetupToolsMenu(mc_systrayMenu);
    // --------------------------------------------------------------
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
    mc_systrayMenu_advanced = new QMenu(tr("Experimental"), mc_systrayMenu);
    mc_systrayMenu_advanced->setIcon(mc_systrayIcon_advanced);
    mc_systrayMenu->addMenu(mc_systrayMenu_advanced);
    //Advanced submenu
    // --------------------------------------------------------------
    // Settings

    mc_systrayMenu_settings = new QAction(mc_systrayIcon_advanced_settings, tr("Settings"), mc_systrayMenu_advanced);
    mc_systrayMenu_settings->setMenuRole(QAction::NoRole);
    mc_systrayMenu_advanced->addAction(mc_systrayMenu_settings);
    connect(mc_systrayMenu_settings, SIGNAL(triggered()), this, SLOT(mc_settings_slot()));
    // --------------------------------------------------------------
    //Separator
    mc_systrayMenu_advanced->addSeparator();
    // ------------------------------------------------
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


void Moneychanger::SetupAssetMenu(QPointer<QMenu> & parent_menu)
{
    mc_systrayMenu_asset = new QMenu(tr("Set default asset..."), parent_menu);
    mc_systrayMenu_asset->setIcon(mc_systrayIcon_purse);
    parent_menu->addMenu(mc_systrayMenu_asset);
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

void Moneychanger::SetupServerMenu(QPointer<QMenu> & parent_menu)
{
    mc_systrayMenu_server = new QMenu(tr("Set default server..."), parent_menu);
    mc_systrayMenu_server->setIcon(mc_systrayIcon_server);
    parent_menu->addMenu(mc_systrayMenu_server);
    // --------------------------------------------------
    //Add a "Manage Servers" action button (and connection)
    QAction * manage_servers = new QAction(tr("Manage Server Contracts..."), mc_systrayMenu_server);
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

void Moneychanger::SetupNymMenu(QPointer<QMenu> & parent_menu)
{
    mc_systrayMenu_nym = new QMenu("Set default identity...", parent_menu);
    mc_systrayMenu_nym->setIcon(mc_systrayIcon_nym);
    parent_menu->addMenu(mc_systrayMenu_nym);

    //Add a "Manage pseudonym" action button (and connection)
    QAction * manage_nyms = new QAction(tr("Manage My Identities..."), mc_systrayMenu_nym);
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


void Moneychanger::SetupPaymentsMenu(QPointer<QMenu> & parent_menu)
{
    mc_systrayMenu_payments = new QMenu(tr("Payments"), parent_menu);
    mc_systrayMenu_payments->setIcon(mc_systrayIcon_sendfunds);
    parent_menu->addMenu(mc_systrayMenu_payments);
    // --------------------------------------------------------------
    //Send funds
    mc_systrayMenu_sendfunds = new QAction(mc_systrayIcon_sendfunds, tr("Pay Funds..."), mc_systrayMenu_payments);
    mc_systrayMenu_payments->addAction(mc_systrayMenu_sendfunds);
    connect(mc_systrayMenu_sendfunds, SIGNAL(triggered()), this, SLOT(mc_sendfunds_slot()));
    // -------------------------------------------------
    mc_systrayMenu_payments->addSeparator();
    // -------------------------------------------------
    //Payment History
    mc_systrayMenu_receipts = new QAction(mc_systrayIcon_overview, tr("Payment History"), mc_systrayMenu_payments);
    mc_systrayMenu_payments->addAction(mc_systrayMenu_receipts);
    connect(mc_systrayMenu_receipts, SIGNAL(triggered()), this, SLOT(mc_payments_slot()));
    // --------------------------------------------------------------
    //Pending Transactions
    mc_systrayMenu_overview = new QAction(mc_systrayIcon_overview, tr("Pending Transactions"), mc_systrayMenu_payments);
    mc_systrayMenu_payments->addAction(mc_systrayMenu_overview);
    connect(mc_systrayMenu_overview, SIGNAL(triggered()), this, SLOT(mc_overview_slot()));
    // -------------------------------------------------
    mc_systrayMenu_payments->addSeparator();
    // -------------------------------------------------
    //Request payment
    mc_systrayMenu_requestfunds = new QAction(mc_systrayIcon_requestfunds, tr("Request Payment..."), mc_systrayMenu_payments);
    mc_systrayMenu_payments->addAction(mc_systrayMenu_requestfunds);
    connect(mc_systrayMenu_requestfunds, SIGNAL(triggered()), this, SLOT(mc_requestfunds_slot()));
    // --------------------------------------------------------------
    mc_systrayMenu_import_cash = new QAction(mc_systrayIcon_advanced_import, tr("Import Cash..."), mc_systrayMenu_payments);
    mc_systrayMenu_payments->addAction(mc_systrayMenu_import_cash);
    connect(mc_systrayMenu_import_cash, SIGNAL(triggered()), this, SLOT(mc_import_slot()));
}

// --------------------------------------------------------------

void Moneychanger::SetupExchangeMenu(QPointer<QMenu> & parent_menu)
{
    mc_systrayMenu_exchange = new QMenu(tr("Exchange"), parent_menu);
    mc_systrayMenu_exchange->setIcon(mc_systrayIcon_markets);
    parent_menu->addMenu(mc_systrayMenu_exchange);
    // --------------------------------------------------------------
    mc_systrayMenu_markets = new QAction(mc_systrayIcon_markets, tr("Live Offers"), mc_systrayMenu_exchange);
    mc_systrayMenu_exchange->addAction(mc_systrayMenu_markets);
    connect(mc_systrayMenu_markets, SIGNAL(triggered()), this, SLOT(mc_market_slot()));
    // --------------------------------------------------------------
    mc_systrayMenu_trade_archive = new QAction(mc_systrayIcon_trade_archive, tr("Historical Trades"), mc_systrayMenu_exchange);
    mc_systrayMenu_exchange->addAction(mc_systrayMenu_trade_archive);
    connect(mc_systrayMenu_trade_archive, SIGNAL(triggered()), this, SLOT(mc_trade_archive_slot()));
}


void Moneychanger::SetupContractsMenu(QPointer<QMenu> & parent_menu)
{
    mc_systrayMenu_contracts = new QMenu(tr("Contracts"), parent_menu);
    mc_systrayMenu_contracts->setIcon(mc_systrayIcon_advanced_agreements);
    parent_menu->addMenu(mc_systrayMenu_contracts);
    // --------------------------------------------------------------
    SetupAssetMenu(mc_systrayMenu_contracts);
    SetupServerMenu(mc_systrayMenu_contracts);
    // -------------------------------------------------
    mc_systrayMenu_contracts->addSeparator();
    // -------------------------------------------------
    mc_systrayMenu_smart_contracts = new QAction(mc_systrayIcon_advanced_agreements, tr("Smart Contract Editor"), mc_systrayMenu_contracts);
    mc_systrayMenu_contracts->addAction(mc_systrayMenu_smart_contracts);
    connect(mc_systrayMenu_smart_contracts, SIGNAL(triggered()), this, SLOT(mc_agreement_slot()));
}

void Moneychanger::SetupToolsMenu(QPointer<QMenu> & parent_menu)
{
    mc_systrayMenu_tools = new QMenu(tr("Crypto Tools"), parent_menu);
    mc_systrayMenu_tools->setIcon(mc_systrayIcon_crypto_sign);
    parent_menu->addMenu(mc_systrayMenu_tools);
    // --------------------------------------------------------------
    mc_systrayMenu_crypto_sign = new QAction(mc_systrayIcon_crypto_sign, tr("Sign"), mc_systrayMenu_tools);
    mc_systrayMenu_tools->addAction(mc_systrayMenu_crypto_sign);
    connect(mc_systrayMenu_crypto_sign, SIGNAL(triggered()), this, SLOT(mc_crypto_sign_slot()));
    // --------------------------------------------------------------
    mc_systrayMenu_crypto_encrypt = new QAction(mc_systrayIcon_crypto_encrypt, tr("Encrypt"), mc_systrayMenu_tools);
    mc_systrayMenu_tools->addAction(mc_systrayMenu_crypto_encrypt);
    connect(mc_systrayMenu_crypto_encrypt, SIGNAL(triggered()), this, SLOT(mc_crypto_encrypt_slot()));
    // --------------------------------------------------------------
    //Separator
    mc_systrayMenu_tools->addSeparator();
    // --------------------------------------------------------------
    mc_systrayMenu_crypto_decrypt = new QAction(mc_systrayIcon_crypto_decrypt, tr("Decrypt / Verify"), mc_systrayMenu_tools);
    mc_systrayMenu_tools->addAction(mc_systrayMenu_crypto_decrypt);
    connect(mc_systrayMenu_crypto_decrypt, SIGNAL(triggered()), this, SLOT(mc_crypto_decrypt_slot()));
    // --------------------------------------------------------------
}

void Moneychanger::SetupMessagingMenu(QPointer<QMenu> & parent_menu)
{
    mc_systrayMenu_messaging = new QMenu(tr("Messages"), parent_menu);
    mc_systrayMenu_messaging->setIcon(mc_systrayIcon_composemessage);
    parent_menu->addMenu(mc_systrayMenu_messaging);
    // -------------------------------------------------
    //Compose Message
    mc_systrayMenu_composemessage = new QAction(mc_systrayIcon_composemessage, tr("Compose Message"), mc_systrayMenu_messaging);
    mc_systrayMenu_messaging->addAction(mc_systrayMenu_composemessage);
    connect(mc_systrayMenu_composemessage, SIGNAL(triggered()), this, SLOT(mc_composemessage_slot()));
    // --------------------------------------------------------------
    mc_systrayMenu_messaging->addSeparator();
    // --------------------------------------------------------------
    //Message History
    mc_systrayMenu_messages = new QAction(mc_systrayIcon_overview, tr("Message History"), mc_systrayMenu_messaging);
    mc_systrayMenu_messaging->addAction(mc_systrayMenu_messages);
    connect(mc_systrayMenu_messages, SIGNAL(triggered()), this, SLOT(mc_messages_slot()));
    // --------------------------------------------------------------
    //Address Book
    mc_systrayMenu_contacts = new QAction(mc_systrayIcon_contacts, tr("Address Book"), mc_systrayMenu_messaging);
    mc_systrayMenu_messaging->addAction(mc_systrayMenu_contacts);
    connect(mc_systrayMenu_contacts, SIGNAL(triggered()), this, SLOT(mc_addressbook_slot()));
    // --------------------------------------------------------------
    mc_systrayMenu_messaging->addSeparator();
    // --------------------------------------------------------------
    // Transport
    mc_systrayMenu_p2p_transport = new QAction(mc_systrayIcon_advanced_transport, tr("P2P Transport"), mc_systrayMenu_messaging);
    mc_systrayMenu_messaging->addAction(mc_systrayMenu_p2p_transport);
    connect(mc_systrayMenu_p2p_transport, SIGNAL(triggered()), this, SLOT(mc_transport_slot()));
    // --------------------------------------------------------------
    // Error Log
    mc_systrayMenu_error_log = new QAction(mc_systrayIcon_overview, tr("Error Log"), mc_systrayMenu_messaging);
    mc_systrayMenu_messaging->addAction(mc_systrayMenu_error_log);
    connect(mc_systrayMenu_error_log, SIGNAL(triggered()), this, SLOT(mc_log_slot()));

    connect(this, SIGNAL(appendToLog(QString)),
            this, SLOT(mc_showlog_slot(QString)));
    // --------------------------------------------------------------
}

void Moneychanger::SetupAccountMenu(QPointer<QMenu> & parent_menu)
{
    mc_systrayMenu_account = new QMenu(tr("Set default account..."), parent_menu);
    mc_systrayMenu_account->setIcon(mc_systrayIcon_goldaccount);
    parent_menu->addMenu(mc_systrayMenu_account);

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
        if (!qstrPresetID.isEmpty() && (0 == qstrPresetID.compare(OT_id)))
            bFoundPreset = true;
        // ------------------------------
    } // for
    // -------------------------------------
    nymswindow->setWindowTitle(tr("Manage My Identities"));
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



void Moneychanger::onNeedToDownloadMail()
{
    QString qstrErrorMsg;
    qstrErrorMsg = tr("Failed trying to contact the notary. Perhaps it is down, or there might be a network problem.");
    // -----------------------------
    opentxs::OT_ME madeEasy;

    int32_t nymCount = opentxs::OTAPI_Wrap::It()->GetNymCount();

    if (0 == nymCount)
    {
        qDebug() << "Making 'Me' Nym";

        std::string strSource("");
        std::string newNymId = madeEasy.create_nym_ecdsa(strSource);

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


// Todo someday: Add a setting to the configuration so a user can choose whether or not to import Bitmessages.
// In which case they might never be added to the database here, or deleting from Bitmessage here (as they are now in both cases), unless that setting was set to true.
//
bool Moneychanger::AddMailToMsgArchive(opentxs::OTRecord& recordmt)
{
    QPointer<ModelMessages> pModel = DBHandler::getInstance()->getMessageModel();
    bool bSuccessAddingMsg = false;
    QString qstrBody("");

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
        // ---------------------------------
        time64_t tDate = static_cast<time64_t>(opentxs::OTAPI_Wrap::It()->StringToLong(recordmt.GetDate()));
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
        if (!mailDescription.isEmpty())
            record.setValue("subject", mailDescription);
        record.setValue("folder", nFolder);

        pModel->insertRecord(0, record);
        // ---------------------------------
        if (pModel->submitAll())
        {
            if (pModel->database().commit())
            {
                // Success.
                bSuccessAddingMsg = true;
                qstrBody = QString::fromStdString(recordmt.GetContents());
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
        if (!MTContactHandler::getInstance()->CreateMessageBody(qstrBody))
        {
            qDebug() << "AddMailToMsgArchive: Succeeded adding message record to database, but then failed writing message body.\n";
            return false;
        }
        // -------------------------------------------------------
        // Now that we've added it to our database, we need to delete it from Bitmessage.
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
        } // special mail

        if (!bSuccessDeletingSpecial)
            qDebug() << "AddMailToMsgArchive: FYI, Failed while trying to delete special mail (probably bitmessage) from its native source.";
        else
            qDebug() << "AddMailToMsgArchive: FYI, SUCCESS deleting special mail (probably bitmessage) from its native source.";
        // -----------------------------------
    }

    return bSuccessAddingMsg;
}

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
        if (recordmt.IsMail())                  flags |= ModelPayments::IsMail;
        if (recordmt.IsTransfer())              flags |= ModelPayments::IsTransfer;
        if (recordmt.IsCheque())                flags |= ModelPayments::IsCheque;
        if (recordmt.IsInvoice())               flags |= ModelPayments::IsInvoice;
        if (recordmt.IsVoucher())               flags |= ModelPayments::IsVoucher;
        if (recordmt.IsContract())              flags |= ModelPayments::IsContract;
        if (recordmt.IsPaymentPlan())           flags |= ModelPayments::IsPaymentPlan;
        if (recordmt.IsCash())                  flags |= ModelPayments::IsCash;
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
        time64_t tDate = static_cast<time64_t>(opentxs::OTAPI_Wrap::It()->StringToLong(recordmt.GetDate()));

        int64_t transNum        = recordmt.GetTransactionNum();
        int64_t transNumDisplay = recordmt.GetTransNumForDisplay();

        int64_t lAmount = opentxs::OTAPI_Wrap::It()->StringToLong(recordmt.GetAmount());

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
        nPreExistingPaymentId = MTContactHandler::getInstance()->GetPaymentIdByTxnDisplayId(transNumDisplay);
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
        if (!bCanDeleteRecord) qstrPendingBody = QString::fromStdString(recordmt.GetContents());
        else                   qstrBody        = QString::fromStdString(recordmt.GetContents());
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



void Moneychanger::modifyRecords()
{
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

            if (!recordmt.CanDeleteRecord())
            {
                // In this case we aren't going to delete the record, but we can still
                // save a copy of it in our local database, if it's not already there.

                if (!recordmt.IsMail() &&
                    !recordmt.IsSpecialMail() &&
                    !recordmt.IsExpired() )
                    AddPaymentToPmntArchive(recordmt);

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
                    if (AddMailToMsgArchive(recordmt))
                        bShouldDeleteRecord = true;
                }
                // -----------------------------------
                else if (recordmt.IsRecord() && !recordmt.IsExpired())
                {
                    if (recordmt.IsReceipt())
                    {
                        if (0 == recordmt.GetInstrumentType().compare("marketReceipt"))
                        {
                            // We don't have to add these to the trade archive table because they
                            // are already there. OTClient directly adds them into the TradeNymData object,
                            // and then Moneychanger reads that object and imports it into the trade_achive
                            // table already. So basically here all we need to do is delete the market
                            // receipt records so the user doesn't have the hassle of deleting them himself.
                            // Now they are safe in his archive and he can do whatever he wants with them.

                            bShouldDeleteRecord = true;
                        } // marketReceipt
                        // -----------------------------------
                        else if (0 == recordmt.GetInstrumentType().compare("finalReceipt"))
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
                            if (AddFinalReceiptToTradeArchive(recordmt))
                                bShouldDeleteRecord = true;
                        } // finalReceipt
                        else // All  other closed (deletable) receipts.
                        {
                            if (AddPaymentToPmntArchive(recordmt))
                                bShouldDeleteRecord = true;
                        }
                    }
                    // -----------------------------------
                    else // All  other delete-able, non-expired records.
                    {
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
                        if (AddPaymentToPmntArchive(recordmt))
                            bShouldDeleteRecord = true;
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
    } // for (GetRecordlist() in reverse)
    // -------------------------------------
    // If the above process DID remove any records, then we have to repopulate them now,
    // since every record contains its index, and so they will be wrong until re-populated.
    //
    if (listSize != GetRecordlist().size())
        populateRecords();
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
    opentxs::OT_ME madeEasy;

    std::string accountId = qstrAcctID.toStdString();
    std::string acctNymID = opentxs::OTAPI_Wrap::It()->GetAccountWallet_NymID   (accountId);
    std::string acctSvrID = opentxs::OTAPI_Wrap::It()->GetAccountWallet_NotaryID(accountId);
    // ------------------------------
    std::string accountIdOptional = qstrOptionalAcctID.isEmpty() ? "" : qstrOptionalAcctID.toStdString();
    std::string acctNymIDOptional = qstrOptionalAcctID.isEmpty() ? "" : opentxs::OTAPI_Wrap::It()->GetAccountWallet_NymID   (accountIdOptional);
    std::string acctSvrIDOptional = qstrOptionalAcctID.isEmpty() ? "" : opentxs::OTAPI_Wrap::It()->GetAccountWallet_NotaryID(accountIdOptional);
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
        bRetrievalSucceededNym = madeEasy.retrieve_nym(acctSvrID, acctNymID, true);

        if (opentxs::OTAPI_Wrap::networkFailure())
        {
            emit appendToLog(qstrErrorMsg);
            return;
        }

        // Let's download the Nym for the optional account too, but ONLY if it's not the same Nym!!
        //
        if (bRetrievalSucceededNym && !qstrOptionalAcctID.isEmpty() && (acctNymIDOptional != acctNymID))
        {
            bRetrievalSucceededNym = madeEasy.retrieve_nym(acctSvrIDOptional, acctNymIDOptional, true);

            if (opentxs::OTAPI_Wrap::networkFailure())
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
        bRetrievalSucceededAcct = madeEasy.retrieve_account(acctSvrID, acctNymID, accountId, true);

        if (bRetrievalSucceededAcct && !qstrOptionalAcctID.isEmpty())
        {
            bRetrievalSucceededAcct = madeEasy.retrieve_account(acctSvrIDOptional, acctNymIDOptional, accountIdOptional, true);
        }

        if (opentxs::OTAPI_Wrap::networkFailure())
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
    populateRecords(); // This updates the record list. (It assumes a download has recently occurred.)
    // ----------------------------------------------------------------
    emit populatedRecordlist();
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

            std::string strSource("");

            std::string newNymId = madeEasy.create_nym_ecdsa(strSource);

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

//      qDebug() << QString("Account Count: %1").arg(accountCount);

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
    int32_t acct_count = opentxs::OTAPI_Wrap::It()->GetAccountCount();
    bool bFoundDefault = false;

    for (int32_t ii = 0; ii < acct_count; ii++)
    {
        QString OT_id   = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAccountWallet_ID(ii));
        QString OT_name = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetAccountWallet_Name(OT_id.toStdString()));

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
    int32_t server_count = opentxs::OTAPI_Wrap::It()->GetServerCount();
    bool    bFoundPreset = false;

    for (int32_t ii = 0; ii < server_count; ii++)
    {
        QString OT_id   = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetServer_ID(ii));
        QString OT_name = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetServer_Name(OT_id.toStdString()));

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

    if (new_server_title.isEmpty())
    {
        new_server_title = tr("Set Default...");
    }

    if (mc_overall_init)
    {
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

        connect(messages_window, SIGNAL(needToDownloadMail()),
                this,            SLOT(onNeedToDownloadMail()));

        connect(this,            SIGNAL(populatedRecordlist()),
                messages_window, SLOT(onRecordlistPopulated()));
    }
    // ---------------------------------
    messages_window->dialog();
}


void Moneychanger::mc_payments_slot()
{
    mc_payments_dialog();
}

void Moneychanger::mc_payments_dialog()
{
    if (!payments_window)
    {
        payments_window = new Payments(this);

        connect(payments_window, SIGNAL(showDashboard()),
                this,            SLOT(mc_overview_slot()));

        connect(payments_window, SIGNAL(needToDownloadAccountData()),
                this,            SLOT(onNeedToDownloadAccountData()));

        connect(this,            SIGNAL(populatedRecordlist()),
                payments_window, SLOT(onRecordlistPopulated()));

        connect(payments_window, SIGNAL(needToPopulateRecordlist()),
                this,            SLOT(onNeedToPopulateRecordlist()));

        connect(this,            SIGNAL(balancesChanged()),
                payments_window, SLOT(onBalancesChanged()));

    }
    // ---------------------------------
    payments_window->dialog();
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
}


void Moneychanger::onAssetsChanged()
{

}


void Moneychanger::onNymsChanged()
{

}


void Moneychanger::onAccountsChanged()
{

}

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

void Moneychanger::onNewNymAdded(QString qstrID)
{
    // Add a new Contact in the Address Book for this Nym as well.
    // It's a pain having to add my own Nyms to the address book
    // by hand for sending payments between them.

    QString qstrNymName("");

    if (!qstrID.isEmpty())
    {
        MTNameLookupQT theLookup;
        qstrNymName = QString::fromStdString(theLookup.GetNymName(qstrID.toStdString(), ""));
        int nContactID  = MTContactHandler::getInstance()->CreateContactBasedOnNym(qstrID, "");
//      QString qstrContactID = QString::number(nContactID);

        if (!qstrNymName.isEmpty() && (nContactID > 0))
        {
            qstrNymName += tr(" (local wallet)");
            MTContactHandler::getInstance()->SetContactName(nContactID, qstrNymName);

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
                this,       SLOT(mc_agreement_slot()));

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

