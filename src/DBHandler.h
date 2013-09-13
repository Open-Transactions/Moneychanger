#ifndef DBHANDLER_H
#define DBHANDLER_H

#include <QSqlDatabase>
#include <QMessageBox>
#include <QString>
#include <QSqlQuery>
#include <QProgressDialog>
#include <QCoreApplication>
#include <QMutex>
#include <QMutexLocker>
#include <QSqlError>
#include <QVariant>

#include "filehandler.h"

/*
 * declare DB driver and filename.
 * Note that the filename path assignment is going to need to be fixed.
 */
static const QString dbDriverStr = "QSQLITE";
static const QString dbFileNameStr = "/db/mc_db";
static const QString dbConnNameStr = "addressBook";

static const int PBAR_MAX_STEPS =7;

/*
 * Error messages.
 */
static QString dbConnectErrorStr =
       QObject::tr("Database Connection Error");
static QString dbDriverNotExistStr =
       QObject::tr("Database driver is not available.");
static QString dbCannotOpenStr =
       QObject::tr("The database cannot open.");
static const QString dbCreationStr =
       QObject::tr("Creating a database instance failed");

class DBHandler
{
  private:
    static DBHandler * _instance;

  protected:
    DBHandler();

    QSqlDatabase db;
    FileHandler dbFile;
    QMutex dbMutex;
    
    bool dbConnect();
    bool isConnected();
    bool dbDisconnect();
    bool isDbExist();
    bool dbRemove();
    bool dbCreateInstance();

  public:
    static DBHandler * getInstance();
    
    bool runQuery(QString run);
    int querySize(QString run);
    bool isNext(QString run);
    QString queryString(QString run, int value, int at=0);
    QVariant AddressBookInsertNym(QString nym_id_string, QString nym_display_name_string);
    bool AddressBookUpdateNym(QString nym_id_string, QString nym_display_name_string, QString index_id_string);
    bool AddressBookRemoveID(int ID);
    bool AddressBookUpdateDefaultNym(QString ID);
    bool AddressBookUpdateDefaultAsset(QString ID);
    bool AddressBookUpdateDefaultAccount(QString ID);
    bool AddressBookUpdateDefaultServer(QString ID);



    ~DBHandler();
};

#endif // DBHANDLER_H
