#include "DBHandler.h"
#include <QDebug>

DBHandler * DBHandler::_instance = 0;

DBHandler* DBHandler::getInstance()
{
    if (_instance == 0)
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
        QString default_nym_create = "CREATE TABLE default_nym (nym TEXT)";
        QString default_server_create = "CREATE TABLE default_server (server TEXT)";
        QString default_asset_create = "CREATE TABLE default_asset (asset TEXT)";
        QString default_account_create = "CREATE TABLE default_account (account TEXT)";

        
        error = query.exec(address_book_create);
        error = query.exec(default_nym_create);
        error = query.exec(default_server_create);
        error = query.exec(default_asset_create);
        error = query.exec(default_account_create);
        
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
