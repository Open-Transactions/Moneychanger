#include "FileHandler.h"


bool FileHandler::removeFile(QString fileName)
{
    bool ret = false;
    ret = QFile::remove(fileName);
    
    if(ret == false)
    {
        qDebug() << fileHandlerstr + " " + fileNotRemovedStr;
        return ret;
    }
    
    qDebug() << fileHandlerstr + " " + fileRemovedStr;
    return ret;
    
}


bool FileHandler::isFileExist(QString fileName)
{
    bool ret= false;
    ret = QFile::exists(fileName);
    
    if(ret == false)
    {
        qDebug() << fileHandlerstr + " " + fileNotExistStr;
        return ret;
    }
    return ret;
}
