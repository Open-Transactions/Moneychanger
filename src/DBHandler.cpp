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
    bool ret = false;

    QSqlQuery query(db);

    if(db.isOpen())
    {
        qDebug() << "Creating Tables";

        QString address_book_create = "CREATE TABLE address_book (id INTEGER PRIMARY KEY, nym_id TEXT, nym_display_name TEXT)";
        QString default_nym_create = "CREATE TABLE default_nym (nym TEXT)";
        QString default_server_create = "CREATE TABLE default_server (server TEXT)";
        
        ret = query.exec(address_book_create);
        ret = query.exec(default_nym_create);
        ret = query.exec(default_server_create);
        
        if(!ret)
        {
            qDebug() << dbConnectErrorStr + " " + dbCreationStr;
            FileHandler rm;
            db.close();
            rm.removeFile(QCoreApplication::applicationDirPath() + dbFileNameStr);
        }
        else
            qDebug() << "Database " + dbFileNameStr + " created.";
    }
    return ret;
}

bool DBHandler::isConnected()
{
    return db.isOpen();
}
