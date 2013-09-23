#include "DBHandler.h"
#include <QDebug>

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
    qDebug() << QCoreApplication::applicationDirPath() + dbFileNameStr;
    db.setDatabaseName( QCoreApplication::applicationDirPath() + dbFileNameStr);
    if(!dbConnect())
        qDebug() << "Error Opening Database";
    
    if (!flag)
    {
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
    return dbFile.isFileExist(QCoreApplication::applicationDirPath() + dbFileNameStr);
}


bool DBHandler::dbRemove()
{
    return dbFile.removeFile(QCoreApplication::applicationDirPath() + dbFileNameStr);
}

/*
 * Create our database using the appropriate schema.
 */
bool DBHandler::dbCreateInstance()
{
    QMutexLocker locker(&dbMutex);

    bool error = false;

    QSqlQuery query(db);

    if(db.isOpen())
    {
        qDebug() << "Creating Tables";

        QString address_book_create = "CREATE TABLE address_book (id INTEGER PRIMARY KEY, nym_id TEXT, nym_display_name TEXT)";
        // --------------------------------------------
        QString default_nym_create = "CREATE TABLE default_nym (nym TEXT)";
        QString default_server_create = "CREATE TABLE default_server (server TEXT)";
        QString default_asset_create = "CREATE TABLE default_asset (asset TEXT)";
        QString default_account_create = "CREATE TABLE default_account (account TEXT)";
        // --------------------------------------------
        QString create_contact = "CREATE TABLE contact(contact_id INTEGER PRIMARY KEY, contact_display_name TEXT)";
        QString create_nym     = "CREATE TABLE nym(nym_id TEXT PRIMARY KEY, contact_id INTEGER, nym_display_name)";
        QString create_server  = "CREATE TABLE nym_server(nym_id TEXT, server_id TEXT, PRIMARY KEY(nym_id, server_id))";
        QString create_account = "CREATE TABLE nym_account(account_id TEXT PRIMARY KEY, server_id TEXT, nym_id TEXT, asset_id TEXT, account_display_name TEXT)";
        // --------------------------------------------
        error = query.exec(address_book_create);
        error = query.exec(default_nym_create);
        error = query.exec(default_server_create);
        error = query.exec(default_asset_create);
        error = query.exec(default_account_create);
        // --------------------------------------------
        error = query.exec(create_contact);
        error = query.exec(create_nym);
        error = query.exec(create_server);
        error = query.exec(create_account);
        // ------------------------------------------
        if(!error)
        {
            qDebug() << dbConnectErrorStr + " " + dbCreationStr;
            FileHandler rm;
            db.close();
            rm.removeFile(QCoreApplication::applicationDirPath() + dbFileNameStr);
        }
        else
            qDebug() << "Database " + dbFileNameStr + " created.";
    }
    return error;
}

bool DBHandler::isConnected()
{
    return db.isOpen();
}

bool DBHandler::runQuery(QString run)
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
            return false;
    }
    else
        return error;
    
}


int DBHandler::querySize(QString run)
{
    QMutexLocker locker(&dbMutex);

    int size;
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
            qDebug() << "Error at query Size";
            return -1;
        }
    }
    
    // Return -1 on Error
    else
    {
        qDebug() << "Error at query Size";
        return -1;
    }
    
}


bool DBHandler::isNext(QString run)
{
    QMutexLocker locker(&dbMutex);
    
    bool isnext = false;
    
    QSqlQuery query(db);
    
    if(db.isOpen())
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
        if(!noerror)
            return queryResult;
        else
            return 0;
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
        if(!noerror)
            return queryResult;
        else
            return "";
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
    
    if(db.isOpen())
    {
        if(query.exec(QString("INSERT INTO `address_book` (`id`, `nym_id`, `nym_display_name`) VALUES(NULL, '%1', '%2')").arg(nym_id_string).arg(nym_display_name_string)))
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
    
    if(db.isOpen())
    {
        return query.exec(QString("UPDATE `address_book` SET `nym_id` = '%1', `nym_display_name` = '%2' WHERE `id` = %3").arg(nym_id_string).arg(nym_display_name_string).arg(index_id_string));

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
    if(db.isOpen())
    {
        return query.exec(QString("DELETE FROM `address_book` WHERE `id` = %1").arg(ID));
        
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
    if(db.isOpen())
    {
        if(query.exec(QString("UPDATE `default_nym` SET `nym` = '%1'").arg(ID)))
            return true;
        else
        {
            qDebug() << "AddressBookUpdateDefaultNym Error";
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
    if(db.isOpen())
    {
        if(query.exec(QString("UPDATE `default_asset` SET `asset` = '%1'").arg(ID)))
            return true;
        else
        {
            qDebug() << "AddressBookUpdateDefaultAsset Error";
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
    if(db.isOpen())
    {
        if(query.exec(QString("UPDATE `default_account` SET `account` = '%1'").arg(ID)))
            return true;
        else
        {
            qDebug() << "AddressBookUpdateDefaultAccount Error";
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
    if(db.isOpen())
    {
        if(query.exec(QString("UPDATE `default_server` SET `server` = '%1'").arg(ID)))
            return true;
        else
        {
            qDebug() << "AddressBookUpdateDefaultServer Error";
            return false;
        }
    }
    else
    {
        qDebug() << "AddressBookUpdateDefaultServer Error";
        return false;
    }
}
