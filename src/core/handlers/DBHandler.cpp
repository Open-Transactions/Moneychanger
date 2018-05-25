#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/handlers/DBHandler.hpp>
#include <core/handlers/modeltradearchive.hpp>
#include <core/handlers/modelmessages.hpp>
#include <core/handlers/modelpayments.hpp>

#include <opentxs/opentxs.hpp>

#include <QSqlRecord>
#include <QSqlDriver>
#include <QString>
#include <QObject>
#include <QDateTime>

#include <sstream>
#include <stdexcept>

DBHandler * DBHandler::_instance = NULL;

DBHandler* DBHandler::getInstance()
{
    if (NULL == _instance)
    {
        _instance = new DBHandler;
    }
    return _instance;
}

DBHandler::~DBHandler()
{
    dbDisconnect();
}

DBHandler::DBHandler()
{

    if (!QSqlDatabase::isDriverAvailable (dbDriverStr))
    {
        qDebug() << dbConnectErrorStr + " " + dbDriverNotExistStr;
        exit(1);
    }

    db = QSqlDatabase::addDatabase(dbDriverStr, dbConnNameStr);

    bool flag = isDbExist();
//  qDebug() << QString(opentxs::OTPaths::AppDataFolder().Get()) + dbFileNameStr;
    db.setDatabaseName( QString(opentxs::OTPaths::AppDataFolder().Get()) + dbFileNameStr);
    if(!dbConnect())
        qDebug() << "Error Opening Database";

//  if (!flag)  // The database now creates the tables if they don't exist, so we call this every time now.
    {
        qDebug() << "Running dbCreateInstance";
        dbCreateInstance();
    }
}

/*
 *connect to database
 */
bool DBHandler::dbConnect()
{

    if (!db.open())
    {
        qDebug() << dbConnectErrorStr + " " + dbCannotOpenStr;
        return false;
    }

    qDebug() << "Database opened";
    return db.isOpen();
}


bool DBHandler::dbDisconnect()
{

    db.close();
    if(!db.isOpen())
    {
        return true;
    }
    else
    {
        return false;
    }
}


bool DBHandler::isDbExist()
{
    qDebug() << QString((opentxs::OTPaths::AppDataFolder().Get()) + dbFileNameStr);
    return dbFile.isFileExist(QString(opentxs::OTPaths::AppDataFolder().Get()) + dbFileNameStr);
}


bool DBHandler::dbRemove()
{
    return dbFile.removeFile(QString(opentxs::OTPaths::AppDataFolder().Get()) + dbFileNameStr);
}

/*
 * Create our database using the appropriate schema.
 */
bool DBHandler::dbCreateInstance()
{
    QMutexLocker locker(&dbMutex);

    int error = 0; // Number of SQL queries

    QSqlQuery query(db);

    if (db.isOpen())
    {
        qDebug() << "Creating Tables";

        QString address_book_create = "CREATE TABLE IF NOT EXISTS address_book (id INTEGER PRIMARY KEY, nym_id TEXT, nym_display_name TEXT)";
        // --------------------------------------------
        QString default_nym_create = "CREATE TABLE IF NOT EXISTS default_nym (default_id INTEGER PRIMARY KEY, nym TEXT)";
        QString default_server_create = "CREATE TABLE IF NOT EXISTS default_server (default_id INTEGER PRIMARY KEY, server TEXT)";
        QString default_asset_create = "CREATE TABLE IF NOT EXISTS default_asset (default_id INTEGER PRIMARY KEY, asset TEXT)";
        QString default_account_create = "CREATE TABLE IF NOT EXISTS default_account (default_id INTEGER PRIMARY KEY, account TEXT)";
        QString settings = "CREATE TABLE IF NOT EXISTS settings (setting TEXT PRIMARY KEY, parameter1 TEXT)";
        // --------------------------------------------
        QString create_contact = "CREATE TABLE IF NOT EXISTS contact(contact_id INTEGER PRIMARY KEY, contact_display_name TEXT)";
        QString create_nym     = "CREATE TABLE IF NOT EXISTS nym(nym_id TEXT PRIMARY KEY, contact_id INTEGER, nym_display_name TEXT, nym_payment_code TEXT)";
        QString create_server  = "CREATE TABLE IF NOT EXISTS nym_server(nym_id TEXT, notary_id TEXT, PRIMARY KEY(nym_id, notary_id))";
        QString create_account = "CREATE TABLE IF NOT EXISTS nym_account(account_id TEXT PRIMARY KEY, notary_id TEXT, nym_id TEXT, asset_id TEXT, account_display_name TEXT)";
        // --------------------------------------------
        QString create_msg_method = "CREATE TABLE IF NOT EXISTS msg_method"
                " (method_id INTEGER PRIMARY KEY,"   // 1, 2, etc.
                "  method_display_name TEXT,"        // "Localhost"
                "  method_type TEXT,"                // "bitmessage"
                "  method_type_display TEXT,"        // "Bitmessage"
                "  method_connect TEXT)";            // "http://username:password@http://127.0.0.1:8332/"
        // --------------------------------------------
        // Messaging methods set for various nyms or contacts.
        //
        QString create_nym_method      = "CREATE TABLE IF NOT EXISTS nym_method(nym_id TEXT, method_id INTEGER, address TEXT, PRIMARY KEY(nym_id, method_id, address))";
        QString create_contact_method  = "CREATE TABLE IF NOT EXISTS contact_method(contact_id INTEGER, method_type TEXT, address TEXT, PRIMARY KEY(contact_id, method_type, address))";
        // --------------------------------------------
        // Smart Contracts table
        QString create_smart_contract  = "CREATE TABLE IF NOT EXISTS smart_contract(template_id INTEGER PRIMARY KEY, template_display_name TEXT, template_contents TEXT)";
        // --------------------------------------------
        // Passphrase Manager table
        QString create_managed_passphrase  = "CREATE TABLE IF NOT EXISTS managed_passphrase"
                "(passphrase_id INTEGER PRIMARY KEY,"
                " passphrase_title TEXT,"
                " passphrase_username TEXT,"
                " passphrase_passphrase TEXT,"
                " passphrase_url TEXT,"
                " passphrase_notes TEXT"
                ")";
        // --------------------------------------------
        // Trade Archive table
        QString create_trade_archive  = "CREATE TABLE IF NOT EXISTS trade_archive"
                "(is_bid INTEGER,"
                " actual_price INTEGER,"
                " scale INTEGER,"
                " actual_paid INTEGER,"
                " amount_purchased INTEGER,"
                " timestamp INTEGER,"
                " offer_id INTEGER,"
                " receipt_id INTEGER,"
                " notary_id TEXT,"
                " nym_id TEXT,"
                " asset_acct_id TEXT,"
                " currency_acct_id TEXT,"
                " asset_id TEXT,"
                " currency_id TEXT,"
                " asset_receipt TEXT,"
                " currency_receipt TEXT,"
                " final_receipt TEXT"
                ")";
        // --------------------------------------------
        QString create_message_table = "CREATE TABLE IF NOT EXISTS message"
               "(message_id INTEGER PRIMARY KEY,"
               " have_read INTEGER,"
               " have_replied INTEGER,"
               " have_forwarded INTEGER,"
               " subject TEXT,"
               " sender_nym_id TEXT,"
               " sender_address TEXT,"
               " recipient_nym_id TEXT,"
               " recipient_address TEXT,"
               " timestamp INTEGER,"
               " method_type TEXT,"
               " method_type_display TEXT,"
               " notary_id TEXT,"
               " my_nym_id TEXT,"
               " my_address TEXT,"
               " folder INTEGER,"
               " thread_item_id TEXT,"
               " archived INTEGER NOT NULL,"
               " has_subject INTEGER"
               ")";
        // --------------------------------------------
        QString create_message_body_table = "CREATE TABLE IF NOT EXISTS message_body"
               "(message_id INTEGER PRIMARY KEY,"
               " body TEXT,"
               " thread_item_id TEXT"
               ")";
        // --------------------------------------------
        // Two local Nyms may each have their own conversation with a given
        // Contact ID, and that conversation ID will be set to that Contact ID.
        // Therefore you cannot use the Conversation_ID, by itself, as a primary
        // key, since there may be two local Nyms, and thus two local conversations
        // that are both with Alice, and that both have Alice's ID as the conversation ID.
        // Therefore, to prevent bugs in Moneychanger, which supports multiple Nyms,
        // we index the conversation by Conversation ID *and* Nym ID.
        //
        QString create_conversation_table = "CREATE TABLE IF NOT EXISTS conversation"
               "(conversation_id TEXT,"
               " my_nym_id TEXT,"
               " conversation_name TEXT,"
               " PRIMARY KEY (conversation_id, my_nym_id)"
               ")";
        // --------------------------------------------
        //        message StorageThreadItem {
        //            optional uint32 version = 1;
        //            optional string id = 2;
        //            optional uint64 index = 3;
        //            optional uint64 time = 4;
        //            optional uint32 box = 5;
        //            optional string account = 6;
        //            optional bool unread = 7;
        //        }
        QString create_conversation_msg_table = "CREATE TABLE IF NOT EXISTS conversation_msg"
               "(conversation_id TEXT,"
               " my_nym_id TEXT,"
               " thread_item_id TEXT," // Note: this field is also in message_body table
               " timestamp INTEGER,"
               " box INTEGER,"
               " account TEXT,"
               " unread INTEGER,"
               " PRIMARY KEY (conversation_id, my_nym_id, thread_item_id)"
               ")";
        // --------------------------------------------
        QString create_payment_table = "CREATE TABLE IF NOT EXISTS payment"
               "(payment_id INTEGER PRIMARY KEY,"
               " have_read INTEGER,"
               " have_replied INTEGER,"
               " have_forwarded INTEGER,"
               " memo TEXT,"
               " my_asset_type_id TEXT,"
               " my_nym_id TEXT,"
               " my_acct_id TEXT,"
               " my_address TEXT,"
               " description TEXT,"
               " sender_nym_id TEXT,"
               " sender_acct_id TEXT,"
               " sender_address TEXT,"
               " recipient_nym_id TEXT,"
               " recipient_acct_id TEXT,"
               " recipient_address TEXT,"
               " amount INTEGER,"
               " pending_found INTEGER,"
               " completed_found INTEGER,"
               " timestamp INTEGER,"
               " txn_id INTEGER,"
               " txn_id_display INTEGER,"
               " method_type TEXT,"
               " method_type_display TEXT,"
               " pmnt_notary_id TEXT,"  //" pmnt_notary_id TEXT,"
               " record_name TEXT,"
               " instrument_type TEXT,"
               " folder INTEGER,"
               " flags INTEGER,"
               " msg_notary_id TEXT" //" msg_notary_id TEXT,"
               ")";
        // --------------------------------------------
        QString create_payment_body_table = "CREATE TABLE IF NOT EXISTS payment_body"
               "(payment_id INTEGER PRIMARY KEY,"
               " pending_body TEXT,"
               " body TEXT"
               ")";
        // --------------------------------------------
        QString create_claim_table = "CREATE TABLE IF NOT EXISTS claim"
               "(claim_id TEXT PRIMARY KEY,"
               " claim_nym_id TEXT,"
               " claim_section INTEGER,"
               " claim_type INTEGER,"
               " claim_value TEXT,"
               " claim_start INTEGER,"
               " claim_end INTEGER,"
               " claim_attributes TEXT,"
               " claim_att_active INTEGER,"
               " claim_att_primary INTEGER"
               ")";
        // --------------------------------------------
        QString create_claim_verification_table = "CREATE TABLE IF NOT EXISTS claim_verification"
               "(ver_id TEXT PRIMARY KEY,"
               " ver_claimant_nym_id TEXT,"
               " ver_verifier_nym_id TEXT,"
               " ver_claim_id TEXT,"
               " ver_polarity INTEGER,"
               " ver_start INTEGER,"
               " ver_end INTEGER,"
               " ver_signature TEXT,"
               " ver_signature_verified INTEGER"
               ")";
        // --------------------------------------------

        // Internal verifications are other Nym's claims that I have signed.
        // External verifications are my claims that other Nyms have signed.

        // External are only shown on my Nym Tab
        // Internal are only shown on Contact tab.

        // What you care about is that the signatures have been verified.

        // Whenever you are displaying a claim, any verification related to that claim should be displayed,
        // regardless of whether it's internal or external.


        // Let's say you have made some claims, you have no external ones yet.
        // You're looking at your own Nym and you don't see the verifications. Just maybe the implicit self-signed.
        // I get your Nym and I verify your claims and publish them.
        // At some point your Moneychanger has downloaded my latest verifications.
        // On my credential, those verifications are on the internal side.
        // As soon as your client has done a checkNym and noticed this,
        // you should see those verifications on your own Nym.
        // Now that you see there are verifications related to your claims, you should add those
        // to your own credential, as external claims, and re-publish your credentials.

        // We need a unique identifier for verifications because they can appear
        // more than once for the same claim (from different people.)

        // When Alice verified your claim, I might see that verification appear on Alice's
        // credential, and on your credential, because you republished hers.

        // A verification can either confirm or refute a claim.

        // And the signature validity is either valid or unknown.

        // Need to be able to search for claims based on claimant ID.

        // Need to be able to search for verifications, filtered optionally on Claim ID and/or Verifier ID and/or Claimaint ID.

        // Verification has its own ID, plus a Claimant ID, plus a Verifier ID, plus and Claim ID, plus a confirm/refute (polarity).

        // Column for good signature, unknown signature, invalid. Column name: Validity.

        // There is a start/end time but current unused.

        // Table should have a "known valid" column so it can keep track of which ones it has validated.

        // If someone refutes your claims you do not want to automatically republish that unless you are a troll.

        // When looking at Nym details, I'll see my claims plus other people's verifications of those.

        // When looking at Contact details, I'll see the contact's claims about himself, plus mine and other people's verifications of those claims.

        // Repudiations are just a list of verification IDs. There's also the verifier/repudiator ID.

        // In the UI I would show repudiated status on each verification.

        // --------------------------------------------
        // An agreement is an INSTANCE of a contract on a specific notary.
        // (Such as a running payment plan, or a canceled smart contract.)
        //
        QString create_agreement_table = "CREATE TABLE IF NOT EXISTS agreement"
               "(agreement_id INTEGER PRIMARY KEY," // Used purely behind the scenes, in order to make sure it's unique across notaries.
               " have_read INTEGER," // If you receive any unread receipts, this gets set to false again.
               " txn_id_display INTEGER," // If your UI says "final receipt #1168," this contains 1168. So 1168 identifies this INSTANCE of the agreement on the NOTARY for ALL parties involved.
               " notary_id TEXT," // The notary where the agreement is instantiated. This, plus txn_id_display, is unique to identify the agreement. But the autonumber is easier to work with.
               " contract_fingerprint TEXT," // the hash of the contract, or whatever fingerprinting technique is used.
               " newest_receipt_id INTEGER," // Last known receiptNum for ANYTHING related to this agreement. (For any nym.)
               " newest_known_state INTEGER,"
               " timestamp INTEGER," // Timestamp of the above newest_receipt_id.
               " memo TEXT,"
               " folder INTEGER" // 0 Payment Plan, 1 Smart Contract, 2 Entity
               ")";
        // --------------------------------------------
        // So far this table is unused. Might remove.
        QString create_agreement_nym_table = "CREATE TABLE IF NOT EXISTS agreement_nym"
             "(agreement_id INTEGER,"
             " nym_id TEXT,"
             " nym_opening_txn_id INTEGER," // The Opening txn ID for my_nym (IF KNOWN)
             " known_state INTEGER," // 0 Error, 1 Paid, 2 Payment failed, 3 Contract not active.
             " last_receipt_id INTEGER," // Last receipt where ANYTHING happened for THIS NYM.
             " timestamp INTEGER," // Timestamp of the above receipt.
             " have_read INTEGER," // If you receive any unread receipts, this gets set to false again.
             " last_paid_id INTEGER,"  // Last time this Nym PAID FUNDS, here's the receipt ID.
             " last_collected_id INTEGER," // Last time this Nym COLLECTED FUNDS, here's the receipt ID.
             " last_sent_id INTEGER,"  // Last time this Nym sent the agreement to someone, here's the receipt ID. (No funds changed hands.)
             " last_received_id INTEGER,"  // Last time this Nym received the agreement from someone, here's the receipt ID. (No funds changed hands.)
             " last_notice_id INTEGER," // Last time something happened where this Nym DIDN'T pay or collect. (Like activation notice, or pending incoming/outgoing, or final receipt.)
             " activation_id INTEGER," // the receipt_id for this nym that shows the agreement being activated OR CANCELED (a form of activation that finishes the contract before it starts.)
             " last_final_id INTEGER," // the latest known receipt_id that shows the agreement ending for this Nym. (There may be multiple notices, and final receipts, even for a single Nym. This just memorializes the most recent known one.)
             " PRIMARY KEY(agreement_id, nym_id))";
      // --------------------------------------------
        // So far this table is unused. Might remove.
        QString create_agreement_account_table = "CREATE TABLE IF NOT EXISTS agreement_account"
             "(agreement_id INTEGER,"
             " account_id TEXT,"
             " account_nym_id TEXT,"
             " account_notary_id TEXT,"
             " account_asset_id TEXT,"
             " last_receipt_id INTEGER," // Last receipt where ANYTHING happened for THIS ACCOUNT.
             " timestamp INTEGER," // Timestamp of the above receipt.
             " last_paid_id INTEGER,"  // Last time this account PAID FUNDS, here's the receipt ID.
             " last_collected_id INTEGER," // Last time this account COLLECTED FUNDS, here's the receipt ID.
             " final_receipt_id INTEGER," // the ID for the final receipt for this account. (IF KNOWN)
             " PRIMARY KEY(agreement_id, account_id))";
      // --------------------------------------------
        // Individual receipts / notices for any given agreement.
        QString create_agreement_receipt_table  = "CREATE TABLE IF NOT EXISTS agreement_receipt"
                "(agreement_receipt_key INTEGER PRIMARY KEY,"
                " agreement_id INTEGER,"
                " receipt_id INTEGER," // NOTE: for final receipts, we MUST use the closing ID here, since the "transaction ID" should really go into the event ID field. The closing ID is the only ID on a final receipt that's unique. (And even that is only unique to a notary, not globally. That's why we have the agreement_receipt_key in our local DB.)
                " timestamp INTEGER," // Timestamp on this receipt.
                " have_read INTEGER,"
                " txn_id_display INTEGER,"
                " event_id INTEGER," // Unused. In the future we might have an EVENT that generates, say, 4 receipts to different Nyms and different Accounts. They'd all be different receipts, for different transactions, and different nyms, but for the SAME EVENT.
                " memo TEXT,"
                " my_asset_type_id TEXT,"
                " my_nym_id TEXT,"
                " my_acct_id TEXT,"
                " my_address TEXT,"
                " sender_nym_id TEXT,"
                " sender_acct_id TEXT,"
                " sender_address TEXT,"
                " recipient_nym_id TEXT,"
                " recipient_acct_id TEXT,"
                " recipient_address TEXT,"
                " amount INTEGER," // May be zero. Notices, for exmaple, don't have an amount.
                " folder INTEGER," // 0 is Sent, 1 is Received.
                " method_type TEXT,"
                " method_type_display TEXT,"
                " notary_id TEXT,"
                " description TEXT,"
                " record_name TEXT,"
                " instrument_type TEXT,"
                " flags INTEGER"
                ")";
        // --------------------------------------------
        QString create_receipt_body_table = "CREATE TABLE IF NOT EXISTS receipt_body"
               "(agreement_receipt_key INTEGER PRIMARY KEY,"
               " body TEXT"
               ")";
        // --------------------------------------------
        // RPC User Manager
        QString create_rpcusers_table = "CREATE TABLE IF NOT EXISTS rpc_users(user_id TEXT PRIMARY KEY, password TEXT)";


        /* Keep track of Namecoin names registered for the purpose of
           Moneychanger.  They are always related to a Nym and credential
           hash, so those are kept here, too, so we can easily find
           corresponding names (although the name itself depends on the
           credential hash).  'active' is true if the name registration
           process has been finished; if not, 'regData' contains the
           JSON string describing the nmcrpc::NameRegistration object
           of the ongoing registration.  'updateTx' contains the
           transaction ID of the last name_update that has been issued,
           mainly to keep track of still unconfirmed update transactions
           (after the first confirmation, we can find it via the name).  */
        QString create_nmc = "CREATE TABLE IF NOT EXISTS nmc_names"
                             "  (name     TEXT PRIMARY KEY,"
                             "   nym      TEXT,"
                             "   cred     TEXT,"
                             "   active   BOOLEAN,"
                             "   regData  TEXT,"
                             "   updateTx TEXT)";
        // --------------------------------------------
        error += query.exec(address_book_create);
        error += query.exec(default_nym_create);
        error += query.exec(default_server_create);
        error += query.exec(default_asset_create);
        error += query.exec(default_account_create);
        error += query.exec(settings);
        // --------------------------------------------
        error += query.exec(create_contact);
        error += query.exec(create_nym);
        error += query.exec(create_server);
        error += query.exec(create_account);
        // ------------------------------------------
        error += query.exec(create_msg_method);
        error += query.exec(create_nym_method);
        error += query.exec(create_contact_method);
        error += query.exec(create_smart_contract);
        error += query.exec(create_managed_passphrase);
        error += query.exec(create_trade_archive);
        error += query.exec(create_message_table);
        error += query.exec(create_message_body_table);
        error += query.exec(create_payment_table);
        error += query.exec(create_payment_body_table);
        // ------------------------------------------
        error += query.exec(create_conversation_table);
        error += query.exec(create_conversation_msg_table);
        // ------------------------------------------
        error += query.exec(create_claim_table);
        error += query.exec(create_claim_verification_table);
        // ------------------------------------------
        error += query.exec(create_rpcusers_table);
        // ------------------------------------------
        error += query.exec(create_agreement_table);
        error += query.exec(create_agreement_nym_table);
        error += query.exec(create_agreement_account_table);
        error += query.exec(create_agreement_receipt_table);
        error += query.exec(create_receipt_body_table);
        // ------------------------------------------
        error += query.exec(create_nmc);
        // ------------------------------------------
        if (error != 31)  // Every query passed?
        {
            qDebug() << "dbCreateInstance exec count: " << error << ": ErrorStr: "
                     << dbConnectErrorStr + "--" + dbCreationStr;
//            FileHandler rm;
            db.close();

//          rm.removeFile(QString(opentxs::OTPaths::AppDataFolder().Get()) + dbFileNameStr);
//          rm.removeFile(QCoreApplication::applicationDirPath() + dbFileNameStr);
        }
        else
            qDebug() << "Database " + dbFileNameStr + " created.";
    }
    return error;
}

// Unused for now, but too much work to just throw away:
//    QString qstrQuery(
//                "SELECT contact.contact_id as contact_id, contact.contact_display_name as contact_display_name,"
//                " msg_method.method_type as method_type, msg_method.method_type_display as method_type_display,"
//                " contact_method.address as address"
//                " FROM contact, msg_method, contact_method"
//                " WHERE contact.contact_id = contact_method.contact_id"
//                " AND msg_method.method_type = contact_method.method_type"
//                );


//QPointer<ModelClaims> getClaimsModel(int nContactId);
//QPointer<ModelClaims> getClaimsModel(const QString & qstrNymId);

static void setup_claims_model(QPointer<ModelClaims> & pClaimsModel)
{
    if ( pClaimsModel->lastError().isValid())
        qDebug() <<  pClaimsModel->lastError();

    int column = 0;

    pClaimsModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("claim_id"));
    pClaimsModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("Nym"));
    pClaimsModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("Section"));
    pClaimsModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("Type"));
    pClaimsModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("Value"));
    pClaimsModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("Start"));
    pClaimsModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("End"));
    pClaimsModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("Attributes"));
    pClaimsModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("Active"));
    pClaimsModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("Primary"));
}

QPointer<ModelClaims> DBHandler::getClaimsModel(const QString & qstrNymId)
{
    QPointer<ModelClaims> pClaimsModel = new ModelClaims(0);

    pClaimsModel->setQuery(QString("SELECT * FROM `claim` WHERE `claim_nym_id`='%1'").arg(qstrNymId), db);

    setup_claims_model(pClaimsModel);

    return pClaimsModel;
}

QPointer<ModelClaims> DBHandler::getClaimsModel(int nContactId)
{
    QPointer<ModelClaims> pClaimsModel = new ModelClaims(0);

    QString str_select = QString("SELECT * "         // Select all rows...
                                 "FROM `claim` "     // ...from the claim table...
                                 "INNER JOIN `nym` " // ...where it matches the 'nym' table...
                                 "ON nym.nym_id=claim.claim_nym_id " // ...on the claim_nym_id column.
                                 "WHERE nym.contact_id='%1'"). //(So we only see claims that are attached to a specific contact.)
                                 arg(nContactId);

    pClaimsModel->setQuery(str_select, db);

    setup_claims_model(pClaimsModel);

    return pClaimsModel;
}


// The first case where I want to use this, is when looking at a Contact's
// Relationships, I want to grab every claim where someone claims to know
// that Contact (claims to know one of his Nyms.)
//
QPointer<ModelClaims> DBHandler::getRelationshipClaims(int nAboutContactId)
{
    QPointer<ModelClaims> pClaimsModel = new ModelClaims(0);

    QString str_select = QString("SELECT * "         // Select all rows...
                                 "FROM `claim` "     // ...from the claim table...
                                 "INNER JOIN `nym` " // ...where it matches the 'nym' table...
                                 "ON nym.nym_id=claim.claim_value " // ...on the claim_value column. (If you have a relationship about
                                                                    // a Nym, then his NymId will be in your claim_value field.)
                                 "WHERE nym.contact_id='%1' AND " // (So we only see claims that are attached to a specific contact.)
                                 " claim_section=%2").
                                 arg(nAboutContactId).arg(opentxs::proto::CONTACTSECTION_RELATIONSHIP);

    pClaimsModel->setQuery(str_select, db);

    setup_claims_model(pClaimsModel);

    return pClaimsModel;
}


QPointer<ModelClaims> DBHandler::getRelationshipClaims(const QString & qstrAboutNymId)
{
    QPointer<ModelClaims> pClaimsModel = new ModelClaims(0);

    pClaimsModel->setQuery(QString("SELECT * FROM `claim` WHERE `claim_value`='%1' AND `claim_section`=%2").
                           arg(qstrAboutNymId).arg(opentxs::proto::CONTACTSECTION_RELATIONSHIP),
                           db);

    setup_claims_model(pClaimsModel);

    return pClaimsModel;
}


QPointer<ModelAgreements> DBHandler::getAgreementModel()
{
    QString tableName("agreement");

    if (!pAgreementModel_)
    {
        pAgreementModel_ = new ModelAgreements(0, db);

        pAgreementModel_->setTable(tableName);
        pAgreementModel_->setEditStrategy(QSqlTableModel::OnManualSubmit);

        pAgreementModel_->select();
//        pAgreementModel_->sort(PMNT_SOURCE_COL_TIMESTAMP, Qt::DescendingOrder);

        if ( pAgreementModel_->lastError().isValid())
            qDebug() <<  pAgreementModel_->lastError();

        int column = 0;

        // NOTE: These header names will change.
        pAgreementModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("agreement_id"));
        pAgreementModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Read?"));
        pAgreementModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Txn #"));
        pAgreementModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Notary"));
        pAgreementModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Contract ID"));
        pAgreementModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Newest Receipt #"));
        pAgreementModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Status"));
        pAgreementModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Timestamp"));
        pAgreementModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Memo"));
        pAgreementModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Folder"));
    }

    return pAgreementModel_;
}


QPointer<ModelAgreementReceipts> DBHandler::getAgreementReceiptModel()
{
    QString tableName("agreement_receipt");

    if (!pAgreementReceiptModel_)
    {
        pAgreementReceiptModel_ = new ModelAgreementReceipts(0, db);

        pAgreementReceiptModel_->setTable(tableName);
        pAgreementReceiptModel_->setEditStrategy(QSqlTableModel::OnManualSubmit);

        pAgreementReceiptModel_->select();
//        pAgreementReceiptModel_->sort(PMNT_SOURCE_COL_TIMESTAMP, Qt::DescendingOrder);

        if ( pAgreementReceiptModel_->lastError().isValid())
            qDebug() <<  pAgreementReceiptModel_->lastError();

        int column = 0;

        // NOTE: These header names will change.
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Details"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("agreement_id"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("receipt_id"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Timestamp"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Read?"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("txn_id_display"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("event_id"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Memo"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Asset type"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Me"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("My account"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("My address"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("From"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("From account"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("From address"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("To"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("To account"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("To address"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Amount"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Folder"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("method_type"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Transport"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Notary"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Description"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("record_name"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Instrument"));
        pAgreementReceiptModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("flags"));
    }

    return pAgreementReceiptModel_;
}


QPointer<ModelPayments> DBHandler::getPaymentModel()
{
    QString tableName("payment");

    if (!pPaymentModel_)
    {
        pPaymentModel_ = new ModelPayments(0, db);

        pPaymentModel_->setTable(tableName);
        pPaymentModel_->setEditStrategy(QSqlTableModel::OnManualSubmit);

        pPaymentModel_->select();
//        pPaymentModel_->sort(PMNT_SOURCE_COL_TIMESTAMP, Qt::DescendingOrder);


        if ( pPaymentModel_->lastError().isValid())
            qDebug() <<  pPaymentModel_->lastError();

        int column = 0;

        // NOTE: These header names will change.
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Details"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Read?"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr(" ")); //replied
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr(" ")); // forwarded
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Memo"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Asset type"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Me"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("My account"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("My address"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Description"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("From"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("From account"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("From address"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("To"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("To account"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("To address"));

        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Amount"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("pending_found"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("completed_found"));

        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Timestamp"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Txn#"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("txn_id_display"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("method_type"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Transport"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Payment Notary"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("record_name"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Instrument"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Folder"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Transport Notary"));
    }

    return pPaymentModel_;
}

QPointer<ModelMessages> DBHandler::getMessageModel()
{
    QString tableName("message");

    if (!pMessageModel_)
    {
        pMessageModel_ = new ModelMessages(0, db);

        pMessageModel_->setTable(tableName);
        pMessageModel_->setEditStrategy(QSqlTableModel::OnManualSubmit);

        pMessageModel_->select();
//      pMessageModel_->sort(MSG_SOURCE_COL_TIMESTAMP, Qt::DescendingOrder);

        if ( pMessageModel_->lastError().isValid())
            qDebug() <<  pMessageModel_->lastError();

        int column = 0;

        // NOTE: These header names will change.
        pMessageModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("message_id"));
        pMessageModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Read?"));
        pMessageModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr(" ")); //replied
        pMessageModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr(" ")); // forwarded
        pMessageModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Subject"));
        pMessageModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("From"));
        pMessageModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("From address"));
        pMessageModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("To"));
        pMessageModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("To address"));
        pMessageModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Timestamp"));
        pMessageModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("method_type"));
        pMessageModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Transport"));
        pMessageModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Notary"));
        pMessageModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Me"));
        pMessageModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("My address"));
        pMessageModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Folder"));
        pMessageModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("thread_item_id"));
        pMessageModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("archived"));
        pMessageModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("has_subject"));
    }

    return pMessageModel_;
}



// --------------------------------------------
//QString create_message_table = "CREATE TABLE IF NOT EXISTS message"
//       "(message_id INTEGER PRIMARY KEY,"
//       " have_read INTEGER,"
//       " have_replied INTEGER,"
//       " have_forwarded INTEGER,"
//       " subject TEXT,"
//       " sender_nym_id TEXT,"
//       " sender_address TEXT,"
//       " recipient_nym_id TEXT,"
//       " recipient_address TEXT,"
//       " timestamp INTEGER,"
//       " method_type TEXT,"
//       " method_type_display TEXT,"
//       " notary_id TEXT,"
//       " my_nym_id TEXT,"
//       " my_address TEXT,"
//       " folder INTEGER,"
//       " thread_item_id TEXT,"
//       " archived INTEGER NOT NULL,"
//       " has_subject INTEGER"
//       ")";

//QString create_message_body_table = "CREATE TABLE IF NOT EXISTS message_body"
//       "(message_id INTEGER PRIMARY KEY,"
//       " body TEXT,"
//       " thread_item_id TEXT"
//       ")";
// --------------------------------------------
//QString create_conversation_table = "CREATE TABLE IF NOT EXISTS conversation"
//       "(conversation_id TEXT,"
//       " my_nym_id TEXT,"
//       " conversation_name TEXT,"
//       " PRIMARY KEY (conversation_id, my_nym_id)"
//       ")";

//QString create_conversation_msg_table = "CREATE TABLE IF NOT EXISTS conversation_msg"
//       "(conversation_id TEXT,"
//       " my_nym_id TEXT,"
//       " thread_item_id TEXT," // Note: this field is also in message_body table
//       " timestamp INTEGER,"
//       " box INTEGER,"
//       " account TEXT,"
//       " unread INTEGER,"
//       " PRIMARY KEY (conversation_id, my_nym_id, thread_item_id)"
//       ")";
// --------------------------------------------

QSharedPointer<QSqlQueryMessages> DBHandler::getConversationItemModel(const QString & qstrMyNymId, const QString & qstrThreadId, bool bArchived/*=false*/)
{
    const int nArchived(bArchived ? 1 : 0);
    const int nHasNoSubject = 0;  // Chat messages have no subject. So the has_subject field is always 0 for those records.

    QSharedPointer<QSqlQueryMessages> pModel{new QSqlQueryMessages(0)};

    if (!pModel)
    {
        qDebug() << "QSqlQueryMessages failed to instantiate. Should never happen. Should ASSERT here.";
        return {};
    }
    // --------------------------------------------
    QString str_select = QString( "SELECT "
        "    msg.message_id AS message_id, "
        "    msg.my_nym_id AS my_nym_id, "
        "    conv_msg.conversation_id AS thread_id, "
        "    msg.thread_item_id AS thread_item_id, "
        "    msg.timestamp AS timestamp, "
        "    msg.folder AS folder, "
        "    msg_body.body AS body "
        "  FROM "
        "     `message` AS msg "
        "     INNER JOIN `message_body` AS msg_body "
        "         ON  msg.message_id = msg_body.message_id "
        "         AND msg.thread_item_id = msg_body.thread_item_id " // This line is probably superfluous. Test removing it.
        "     INNER JOIN `conversation_msg` AS conv_msg "
        "         ON  conv_msg.thread_item_id = msg_body.thread_item_id "
        "         AND conv_msg.my_nym_id = msg.my_nym_id "
        "     WHERE "
        "           msg.archived='%1' "
        "       AND msg.has_subject='%2' "
        "       AND msg.my_nym_id='%3' "
        "       AND conv_msg.conversation_id='%4' "
        ).arg(nArchived).arg(nHasNoSubject).arg(qstrMyNymId).arg(qstrThreadId);
    // --------------------------------------------
    pModel->setQuery(str_select, db);

    if ( pModel->lastError().isValid())
        qDebug() <<  pModel->lastError();
    // --------------------------------------------
    int column = 0;

    pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("message_id"));
    pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("my_nym_id"));
    pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("thread_id"));
    pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("thread_item_id"));
    pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("timestamp"));
    pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("folder"));
    pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("body"));

    return pModel;
}


QSharedPointer<QStandardItemModel>  DBHandler::getNewConversationItemModel(
                                                                const QString & qstrMyNymId,
                                                                const QString & qstrThreadId,
                                                                bool bArchived/*=false*/)
{
    const int nArchived(bArchived ? 1 : 0);
    const int nHasNoSubject = 0;  // Chat messages have no subject. So the has_subject field is always 0 for those records.

    QSharedPointer<QStandardItemModel> pModel{new QStandardItemModel(0)};

    if (!pModel)
    {
        qDebug() << "QStandardItemModel failed to instantiate. Should never happen. Should ASSERT here.";
        return {};
    }
    // --------------------------------------------
    int column = 0;

    pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("message_id"));
    pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("my_nym_id"));
    pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("thread_id"));
    pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("thread_item_id"));
    pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("timestamp"));
    pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("folder"));
    pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("body"));
    // --------------------------------------------

//    enum class StorageBox : std::uint8_t {
//        SENTPEERREQUEST = 0,
//        INCOMINGPEERREQUEST = 1,
//        SENTPEERREPLY = 2,
//        INCOMINGPEERREPLY = 3,
//        FINISHEDPEERREQUEST = 4,
//        FINISHEDPEERREPLY = 5,
//        PROCESSEDPEERREQUEST = 6,
//        PROCESSEDPEERREPLY = 7,
//        MAILINBOX = 8,
//        MAILOUTBOX = 9,
//        INCOMINGBLOCKCHAIN = 10,
//        OUTGOINGBLOCKCHAIN = 11,
//        DRAFT = 254,
//        UNKNOWN = 255,
//    };

    int nIndex = 0;
    QString qstrThreadItemId = QString("%1%2")
      .arg(qstrThreadId).arg(QString::number(nIndex));

    const std::string str_my_nym_id      = qstrMyNymId.toStdString();
    const std::string str_thread_id      = qstrThreadId.toStdString();
    const std::string str_thread_item_id = qstrThreadItemId.toStdString();

    const auto& thread = opentxs::OT::App().UI().ActivityThread(
        opentxs::Identifier(str_my_nym_id),
        opentxs::Identifier(str_thread_id));
    const auto& first = thread.First();

//    qDebug() << QString::fromStdString(thread.DisplayName()) << "\n";

    if (false == first->Valid()) {
        return pModel;
    }

    // 0 for moneychanger's outbox, and 1 for inbox.
    int nFolder = -1;
    if (opentxs::StorageBox::MAILINBOX == first->Type()) {
        nFolder = 1;
    }
    else if (opentxs::StorageBox::MAILOUTBOX == first->Type()) {
        nFolder = 0;
    }
    else if (opentxs::StorageBox::DRAFT == first->Type()) {
        nFolder = static_cast<int>(opentxs::StorageBox::DRAFT);
    }

    QList<QStandardItem*> qlistItems;
    QDateTime dateTime;
    dateTime.setTime_t(std::chrono::system_clock::to_time_t(first->Timestamp()));

    qlistItems.append(new QStandardItem(QString::fromStdString(str_thread_item_id))); // message_id
    qlistItems.append(new QStandardItem(QString::fromStdString(str_my_nym_id)));  // my_nym_id
    qlistItems.append(new QStandardItem(QString::fromStdString(str_thread_id))); // thread_id
    qlistItems.append(new QStandardItem(QString::fromStdString(str_thread_item_id))); // thread_item_id
    qlistItems.append(new QStandardItem(dateTime.toString()));  // Timestamp
    qlistItems.append(new QStandardItem(QString::number(nFolder)));  // Folder
    if (first->Loading() && first->Text().empty()) {
        qlistItems.append(new QStandardItem(QString("%1...").arg(QObject::tr("Loading"))));  // Body
    }
    else {
        qlistItems.append(new QStandardItem(QString::fromStdString(first->Text())));  // Body
    }

    pModel->appendRow(qlistItems);

    //        For each item in line_data you create a QStandardItem on the heap
    //        (using "new") and put it into a QList<QStandardItem*>.
    //        Then you append this list to the QStandardItemModel using appendRow().

    // TODO: Populate the first thread item here.

    auto last = first->Last();

//    qDebug() << " * "
//          // << time(first->Timestamp())
//             << " " << QString::fromStdString(first->Text()) << "\n";

    while (false == last) {
        auto line = thread.Next();
        last = line->Last();

        // TODO: Populate the succeeding thread items here.

        if (false == line->Valid()) {
            return pModel;
        }

        // 0 for moneychanger's outbox, and 1 for inbox.
        int nFolder = -1;
        if (opentxs::StorageBox::MAILINBOX == line->Type()) {
            nFolder = 1;
        }
        else if (opentxs::StorageBox::MAILOUTBOX == line->Type()) {
            nFolder = 0;
        }
        else if (opentxs::StorageBox::DRAFT == line->Type()) {
            nFolder = static_cast<int>(opentxs::StorageBox::DRAFT);
        }

        QList<QStandardItem*> qlistItems;
        QDateTime dateTime;
        dateTime.setTime_t(std::chrono::system_clock::to_time_t(line->Timestamp()));
        qlistItems.append(new QStandardItem(QString::fromStdString(str_thread_item_id))); // message_id
        qlistItems.append(new QStandardItem(QString::fromStdString(str_my_nym_id)));  // my_nym_id
        qlistItems.append(new QStandardItem(QString::fromStdString(str_thread_id))); // thread_id
        qlistItems.append(new QStandardItem(QString::fromStdString(str_thread_item_id))); // thread_item_id
        qlistItems.append(new QStandardItem(dateTime.toString()));  // Timestamp
        qlistItems.append(new QStandardItem(QString::number(nFolder)));  // Folder
        if (line->Loading() && line->Text().empty()) {
            qlistItems.append(new QStandardItem(QString("%1...").arg(QObject::tr("Loading"))));  // Body
        }
        else {
            qlistItems.append(new QStandardItem(QString::fromStdString(line->Text())));  // Body
        }

        pModel->appendRow(qlistItems);

//        qDebug() << " * "
//        //<< time(line->Timestamp())
//        << " "
//        << QString::fromStdString(line->Text()) << "\n";
    }

    return pModel;
}


QPointer<ModelVerifications> DBHandler::getVerificationsModel(const QString & forClaimId)
{
    QPointer<ModelVerifications> pModel = new ModelVerifications(0);

    pModel->setQuery(QString("SELECT * FROM `claim_verification` WHERE `ver_claim_id`='%1'").
                     arg(forClaimId), db);

    if ( pModel->lastError().isValid())
        qDebug() <<  pModel->lastError();

    int column = 0;

    pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("ver_id"));
    pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("Claimant"));
    pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("Verifier"));
    pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("Claim"));
    pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("Polarity"));
    pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("Start"));
    pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("End"));
    pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("Signature"));
    pModel->setHeaderData(column++, Qt::Horizontal, QObject::tr("Signature Verified"));

    return pModel;
}


/*
SELECT contact.contact_id as contact_id, contact.contact_display_name as contact_display_name,
 msg_method.method_type as method_type, msg_method.method_type_display as method_type_display,
 contact_method.address as address
 FROM contact, msg_method, contact_method
 WHERE contact.contact_id = contact_method.contact_id
 AND msg_method.method_type = contact_method.method_type
*/

// --------------------------------------------
// contact(contact_id INTEGER PRIMARY KEY, contact_display_name TEXT)";
// --------------------------------------------
// msg_method"
//        " (method_id INTEGER PRIMARY KEY,"   // 1, 2, etc.
//        "  method_display_name TEXT,"        // "Localhost"
//        "  method_type TEXT,"                // "bitmessage"
//        "  method_type_display TEXT,"        // "Bitmessage"
//        "  method_connect TEXT)";            // "http://username:password@http://127.0.0.1:8332/"
// --------------------------------------------
//  contact_method(contact_id INTEGER, method_type TEXT, address TEXT, PRIMARY KEY(contact_id, method_type, address))";
// --------------------------------------------


QPointer<ModelTradeArchive> DBHandler::getTradeArchiveModel()
{
    QString tableName("trade_archive");

    if (!pTradeArchiveModel_)
    {
        pTradeArchiveModel_ = new ModelTradeArchive(0, db);

        pTradeArchiveModel_->setTable(tableName);
        pTradeArchiveModel_->setEditStrategy(QSqlTableModel::OnManualSubmit);
        pTradeArchiveModel_->select();

        if ( pTradeArchiveModel_->lastError().isValid())
            qDebug() <<  pTradeArchiveModel_->lastError();

        int column = 0;

        // NOTE: These header names will change.
        pTradeArchiveModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Bid / Ask")); // is_bid
        pTradeArchiveModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Final Price")); // actual_price
        pTradeArchiveModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Per Scale"));
        pTradeArchiveModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Total Price")); // actual_paid
        pTradeArchiveModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("for Asset Amount")); // amount_purchased
        pTradeArchiveModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Timestamp"));
        pTradeArchiveModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Offer#"));
        pTradeArchiveModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Receipt#"));
        pTradeArchiveModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Notary"));
        pTradeArchiveModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Identity"));
        pTradeArchiveModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Asset Account"));
        pTradeArchiveModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Currency Account"));
        pTradeArchiveModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Asset Type"));
        pTradeArchiveModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Currency Type"));
        pTradeArchiveModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Asset Receipt"));
        pTradeArchiveModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Currency Receipt"));
        pTradeArchiveModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Final Receipt"));
         // --------------------------------------------
    }

    return pTradeArchiveModel_;
}


bool DBHandler::isConnected()
{
    return db.isOpen();
}

QString DBHandler::PreparedQuery::lastQuery()
{
    return query.lastQuery();
}

DBHandler::PreparedQuery* DBHandler::prepareQuery(const QString& run)
{
  return new PreparedQuery (db, run);
}


QString DBHandler::formatValue(QSqlField & sqlField)
{
    QMutexLocker locker(&dbMutex);

    if (!db.isOpen ())
      return "";

    return db.driver()->formatValue(sqlField);
}

bool DBHandler::runQuery(PreparedQuery* query)
{
  std::unique_ptr<PreparedQuery> qu(query);
  QMutexLocker locker(&dbMutex);

  if (!db.isOpen ())
    return false;

  return qu->execute ();
}

QSqlRecord DBHandler::queryOne(PreparedQuery* query)
{
  std::unique_ptr<PreparedQuery> qu(query);
  QMutexLocker locker(&dbMutex);

  if (!db.isOpen ())
    throw std::runtime_error ("Database is not open.");

  if (!qu->execute ())
    {
      std::ostringstream msg;
      msg << "Database query failed: "
          << qu->query.lastError ().text ().toStdString ();
      throw std::runtime_error (msg.str ());
    }

  if (!qu->query.next ())
    throw std::runtime_error ("Expected at least one result"
                              " in DBHandler::queryOne.");

  QSqlRecord res = qu->query.record ();
  if (qu->query.next ())
    throw std::runtime_error ("Expected at most one result"
                              " in DBHandler::queryOne.");

  return res;
}

bool DBHandler::runQuery(const QString& run)
{
    QMutexLocker locker(&dbMutex);

    bool error = false;

    QSqlQuery query(db);

    if(db.isOpen())
    {
        error = query.exec(run);

        if(error)
            return true;
        else
        {
            qDebug() << "runQuery: QSqlQuery::lastError: " << query.lastError().text();
            qDebug() << QString("THE QUERY (that caused the error): %1").arg(run);

            return false;
        }
    }
    else
        return error;
}


int DBHandler::querySize(QString run)
{
    QMutexLocker locker(&dbMutex);

    int size = 0;
    bool noerror = false;
    QSqlQuery query(db);

    if(db.isOpen())
    {
        noerror = query.exec(run);
        //size = query.size();
        // sqlite doesn't support size.
        int recCount = 0;
        while( query.next() )
        {
            recCount++;
        }
        size = recCount;

        if(noerror)
            return size;
        else
        {
            qDebug() << "Error at query Size: query.exec returned false: " << run;
            qDebug() << "QSqlQuery::lastError: " << query.lastError().text();
            return -1;
        }
    }

    // Return -1 on Error
    else
    {
        qDebug() << "Error at query Size: database not even open.";
        return -1;
    }

}


bool DBHandler::isNext(QString run)
{
    QMutexLocker locker(&dbMutex);

    bool isnext = false;

    QSqlQuery query(db);

    if (db.isOpen())
    {
        isnext = query.exec(run);
        isnext = query.next();

        if(isnext)
            return true;
        else
            return false;
    }
    else
        return isnext;

}

int DBHandler::queryInt(QString run, int value, int at)
{
    QMutexLocker locker(&dbMutex);

    bool noerror = false;
    int queryResult;

    QSqlQuery query(db);

    if(db.isOpen())
    {
        noerror = query.exec(run);
        noerror = query.next();
        noerror = query.seek(at);
        queryResult = query.value(value).toInt();

        if (noerror)
            return queryResult;
        else
        {
            qDebug() << "queryInt: QSqlQuery::lastError: " << query.lastError().text();
            return 0;
        }
    }
    else
        return 0;
}


QString DBHandler::queryString(QString run, int value, int at)
{
    QMutexLocker locker(&dbMutex);

    bool noerror = false;
    QString queryResult;

    QSqlQuery query(db);

    if(db.isOpen())
    {
        noerror = query.exec(run);
        noerror = query.next();
        noerror = query.seek(at);
        queryResult = query.value(value).toString();

        if (noerror)
            return queryResult;
        else
        {
            qDebug() << "queryString: QSqlQuery::lastError: " << query.lastError().text();

            return "";
        }
    }
    else
        return "";
}


// -------------------------------------------------------------------------

/*
The QSqlQueryModel class provides a read-only data model for SQL result sets.
QSqlQueryModel is a high-level interface for executing SQL statements and traversing the result set.
It is built on top of the lower-level QSqlQuery and can be used to provide data to view classes such as QTableView.
For example:

    QSqlQueryModel *model = new QSqlQueryModel;
    model->setQuery("SELECT name, salary FROM employee");
    model->setHeaderData(0, Qt::Horizontal, tr("Name"));
    model->setHeaderData(1, Qt::Horizontal, tr("Salary"));

    QTableView *view = new QTableView;
    view->setModel(model);
    view->show();


    NOTE: The view does not take ownership of the model unless it is the model's parent object,
            because the model may be shared between many different views.

We set the model's query, then we set up the labels displayed in the view header.
QSqlQueryModel can also be used to access a database programmatically, without binding it to a view:

    QSqlQueryModel model;
    model.setQuery("SELECT * FROM employee");
    int salary = model.record(4).value("salary").toInt();

The code snippet above extracts the salary field from record 4 in the result set of the query SELECT * from employee.
Assuming that salary is column 2, we can rewrite the last line as follows:

    int salary = model.data(model.index(4, 2)).toInt();
*/

// TODO / RESUME:

// Add a function that returns a QSqlQueryModel*
// You pass in any SQL select you want, and you get a QSqlQueryModel instance back.
// Then attach that model to a QComboBox and/or QListBox
// Also, MTContactHandler can use it to return a model instance for any number of cases

// ---------------------------------------------------------------------------


QVariant DBHandler::AddressBookInsertNym(QString nym_id_string, QString nym_display_name_string)
{
    QMutexLocker locker(&dbMutex);

    QString queryResult;

    QSqlQuery query(db);

    if (db.isOpen())
    {
        if (query.exec(QString("INSERT INTO `address_book` (`id`, `nym_id`, `nym_display_name`) VALUES(NULL, '%1', '%2')").arg(nym_id_string).arg(nym_display_name_string)))
            return query.lastInsertId();
        else
        {
            qDebug() << "AddressBookInsertNym Error";
            return -1;
        }
    }
    else
    {
        qDebug() << "AddressBookInsertNym Error";
        return -1;
    }
}


bool DBHandler::AddressBookUpdateNym(QString nym_id_string, QString nym_display_name_string, QString index_id_string)
{
    QMutexLocker locker(&dbMutex);

    QString queryResult;

    QSqlQuery query(db);

    if (db.isOpen())
    {
        return query.exec(QString("UPDATE `address_book` SET `nym_id` = '%1', `nym_display_name` = '%2' WHERE `id`='%3'").
                          arg(nym_id_string).arg(nym_display_name_string).arg(index_id_string));
    }
    else
    {
        qDebug() << "AddressBookUpdateNym Error";
        return false;
    }

}

bool DBHandler::AddressBookRemoveID(int ID)
{
    QMutexLocker locker(&dbMutex);

    QString queryResult;

    QSqlQuery query(db);

    if (db.isOpen())
    {
        return query.exec(QString("DELETE FROM `address_book` WHERE `id` = '%1'").arg(ID));
    }
    else
    {
        qDebug() << "AddressBookRemoveID Error";
        return false;
    }
}


bool DBHandler::AddressBookUpdateDefaultNym(QString ID)
{
    QMutexLocker locker(&dbMutex);

    QString queryResult;

    QSqlQuery query(db);

    if (db.isOpen())
    {
        if(query.exec(QString("UPDATE `default_nym` SET `nym` = '%1' WHERE `default_id`='1'").arg(ID)))
            return true;
        else
        {
            qDebug() << "AddressBookUpdateDefaultNym Error";
            qDebug() << "QSqlQuery::lastError: " << query.lastError().text();
            return false;
        }
    }
    else
    {
        qDebug() << "AddressBookUpdateDefaultNym Error";
        return false;
    }

}

bool DBHandler::AddressBookUpdateDefaultAsset(QString ID)
{
    QMutexLocker locker(&dbMutex);

    QString queryResult;

    QSqlQuery query(db);

    if (db.isOpen())
    {
        if(query.exec(QString("UPDATE `default_asset` SET `asset` = '%1' WHERE `default_id`='1'").arg(ID)))
            return true;
        else
        {
            qDebug() << "AddressBookUpdateDefaultAsset Error";
            qDebug() << "QSqlQuery::lastError: " << query.lastError().text();
            return false;
        }
    }
    else
    {
        qDebug() << "AddressBookUpdateDefaultAsset Error";
        return false;
    }
}

bool DBHandler::AddressBookUpdateDefaultAccount(QString ID)
{
    QMutexLocker locker(&dbMutex);

    QString queryResult;

    QSqlQuery query(db);

    if (db.isOpen())
    {
        if(query.exec(QString("UPDATE `default_account` SET `account` = '%1' WHERE `default_id`='1'").arg(ID)))
            return true;
        else
        {
            qDebug() << "AddressBookUpdateDefaultAccount Error";
            qDebug() << "QSqlQuery::lastError: " << query.lastError().text();
            return false;
        }
    }
    else
    {
        qDebug() << "AddressBookUpdateDefaultAccount Error";
        return false;
    }
}


bool DBHandler::AddressBookUpdateDefaultServer(QString ID)
{
    QMutexLocker locker(&dbMutex);

    QString queryResult;

    QSqlQuery query(db);

    if (db.isOpen())
    {
        if(query.exec(QString("UPDATE `default_server` SET `server` = '%1' WHERE `default_id`='1'").arg(ID)))
            return true;
        else
        {
            qDebug() << "AddressBookUpdateDefaultServer Error";
            qDebug() << "QSqlQuery::lastError: " << query.lastError().text();
            return false;
        }
    }
    else
    {
        qDebug() << "AddressBookUpdateDefaultServer Error";
        return false;
    }
}

/* ************************************************************************** */

bool
DBHandler::PreparedQuery::execute ()
{
  const bool ok = query.exec ();
  if (!ok)
    {
      qDebug () << "runQuery: QSqlQuery::lastError: "
                << query.lastError ().text ();
      qDebug () << QString("THE QUERY (that caused the error): %1").arg (queryStr);
      return false;
    }

  return true;
}
