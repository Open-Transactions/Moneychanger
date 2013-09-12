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
    bool error = false;

    QSqlQuery query(db);

    if(db.isOpen())
    {
        qDebug() << "Creating Tables";

        QString address_book_create = "CREATE TABLE address_book (id INTEGER PRIMARY KEY, nym_id TEXT, nym_display_name TEXT)";
        QString default_nym_create = "CREATE TABLE default_nym (nym TEXT)";
        QString default_server_create = "CREATE TABLE default_server (server TEXT)";
        
        error = query.exec(address_book_create);
        error = query.exec(default_nym_create);
        error = query.exec(default_server_create);
        
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
    bool error = false;
    QSqlQuery query(db);
    
    if(db.isOpen())
    {
        error = query.exec(run);
        size = query.size();
        if(!error)
            return size;
        else
            return -1;
    }
    
    // Return -1 on Error
    else
        return -1;
    
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

QString DBHandler::queryString(QString run, int value)
{
    QMutexLocker locker(&dbMutex);
    
    bool noerror = false;
    QString queryResult;
    
    QSqlQuery query(db);
    
    if(db.isOpen())
    {
        noerror = query.exec(run);
        queryResult = query.value(value).toString();
        if(noerror)
            return queryResult;
        else
            return "";
    }
    else
        return "";
}