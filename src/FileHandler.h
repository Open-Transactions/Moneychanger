#ifndef FILEHANDLER_H
#define FILEHANDLER_H


#include <QMessageBox>
#include <QString>
#include <QFile>


/* GUI string messages. */
static const QString fileHandlerstr     = QObject::tr("File Handler");
static const QString fileNotExistStr    = QObject::tr("File does not Exist.");
static const QString fileRemovedStr     = QObject::tr("File was removed.");
static const QString fileNotRemovedStr  = QObject::tr("File wasn't removed.");

class FileHandler
{
public:

    /*chech if the file exist*/
    bool isFileExist(QString fileName);
    /*remove file*/
    bool removeFile(QString fileName);
};

#endif // FILEHANDLER_H
