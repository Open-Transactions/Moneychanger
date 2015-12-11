#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/handlers/DBHandler.hpp>
#include <core/handlers/modeltradearchive.hpp>
#include <core/handlers/modelmessages.hpp>
#include <core/handlers/modelpayments.hpp>

#include <opentxs/core/util/OTPaths.hpp>

#include <QSqlRecord>
#include <QSqlDriver>
#include <QString>
#include <QObject>

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
    
    qDebug() << "Adding DB";
    db = QSqlDatabase::addDatabase(dbDriverStr, dbConnNameStr);
    
    bool flag = isDbExist();
    qDebug() << QString(opentxs::OTPaths::AppDataFolder().Get()) + dbFileNameStr;
    db.setDatabaseName( QString(opentxs::OTPaths::AppDataFolder().Get()) + dbFileNameStr);
    if(!dbConnect())
        qDebug() << "Error Opening Database";
    
//    if (!flag)  // The database now creates the tables if they don't exist, so we call this every time now.
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
        QString create_nym     = "CREATE TABLE IF NOT EXISTS nym(nym_id TEXT PRIMARY KEY, contact_id INTEGER, nym_display_name TEXT)";
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
               " folder INTEGER"
               ")";
        // --------------------------------------------
        QString create_message_body_table = "CREATE TABLE IF NOT EXISTS message_body"
               "(message_id INTEGER PRIMARY KEY,"
               " body TEXT"
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
               " notary_id TEXT,"
               " record_name TEXT,"
               " instrument_type TEXT,"
               " folder INTEGER"
               ")";
        // --------------------------------------------
        QString create_payment_body_table = "CREATE TABLE IF NOT EXISTS payment_body"
               "(payment_id INTEGER PRIMARY KEY,"
               " pending_body TEXT,"
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
        error += query.exec(create_rpcusers_table);
        // ------------------------------------------
        error += query.exec(create_nmc);
        // ------------------------------------------
        if (error != 22)  //every query passed?
        {
            qDebug() << "dbCreateInstance Error: " << dbConnectErrorStr + " " + dbCreationStr;
            FileHandler rm;
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
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Notary"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("record_name"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Instrument"));
        pPaymentModel_->setHeaderData(column++, Qt::Horizontal, QObject::tr("Folder"));
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
//        pMessageModel_->sort(MSG_SOURCE_COL_TIMESTAMP, Qt::DescendingOrder);


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
    }

    return pMessageModel_;
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
#ifdef CXX_11
  std::unique_ptr<PreparedQuery> qu(query);
#else /* CXX_11?  */
  std::auto_ptr<PreparedQuery> qu(query);
#endif /* CXX_11?  */

  QMutexLocker locker(&dbMutex);
  if (!db.isOpen ())
    return false;

  return qu->execute ();
}

QSqlRecord DBHandler::queryOne(PreparedQuery* query)
{
#ifdef CXX_11
  std::unique_ptr<PreparedQuery> qu(query);
#else /* CXX_11?  */
  std::auto_ptr<PreparedQuery> qu(query);
#endif /* CXX_11?  */

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
