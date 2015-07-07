#include "rpcserver.h"
#include "mcrpcservice.h"

#include <core/handlers/DBHandler.hpp>

#include <stdexcept>

RPCServer * RPCServer::_instance = NULL;

RPCServer* RPCServer::getInstance()
{
    if (NULL == _instance)
    {
        _instance = new RPCServer;
    }
    return _instance;
}

RPCServer::RPCServer()
{
    m_httpserver.addService(new MCRPCService);

    readConfig();

    if(m_rpcserver_autorun == "true")
        startListener();

}

bool RPCServer::startListener()
{
    if(m_httpserver.isListening()){
        qDebug() << "http service already started";
        return true;
    }
    if (!m_httpserver.listen(QHostAddress::LocalHost, m_rpcserver_listenPort)) {
        qDebug() << "Could not start http service" << m_httpserver.errorString();
        return false;
    }
    else{
        qDebug() << "http service started on port: " << m_rpcserver_listenPort;
        return true;
    }

}

bool RPCServer::stopListener()
{
    if(m_httpserver.isListening()){
        try{
            m_httpserver.close();
            return true;
        }
        catch(...){
            qDebug() << "Error Stopping http service";
            return false;
        }
    }
    else{
        qDebug() << "http service was already stopped";
        return true;
    }
}


void RPCServer::readConfig()
{
    readAutorun();
    readListenPort();
}


void RPCServer::readAutorun()
{
    if (DBHandler::getInstance()->querySize("SELECT `setting`,`parameter1` FROM `settings` WHERE `setting`='rpcserver_autorun'") <= 0)
    {
        DBHandler::getInstance()->runQuery(QString("INSERT INTO `settings` (`setting`, `parameter1`) VALUES('rpcserver_autorun','false')"));
        qDebug() << "rpcserver_autorun setting wasn't set in the database. Inserting default: false";
    }
    else
    {
        if (DBHandler::getInstance()->runQuery("SELECT `setting`,`parameter1` FROM `settings` WHERE `setting`='rpcserver_autorun'"))
        {
            m_rpcserver_autorun = DBHandler::getInstance()->queryString("SELECT `parameter1` FROM `settings` WHERE `setting`='rpcserver_autorun'", 0, 0);
        }
        if (m_rpcserver_autorun.isEmpty())
        {
            m_rpcserver_autorun = "false";
            qDebug() << "Error loading rpcserver_autorun setting from SQL, using default: false";
        }
    }
}

void RPCServer::setAutorun(bool setting)
{
    QString l_setting;
    if(setting)
        l_setting = "true";
    else
        l_setting = "false";

    if (DBHandler::getInstance()->querySize("SELECT `setting`,`parameter1` FROM `settings` WHERE `setting`='rpcserver_autorun'") <= 0)
    {
        DBHandler::getInstance()->runQuery(QString("INSERT INTO `settings` (`setting`, `parameter1`) VALUES('rpcserver_autorun','%1')").arg(l_setting));
        qDebug() << "rpcserver_autorun setting wasn't set in the database. Inserting: " << l_setting;
    }
    else
    {
        DBHandler::getInstance()->runQuery(QString("INSERT OR REPLACE INTO 'settings' VALUES('rpcserver_autorun','%1')").arg(l_setting));
    }

    readAutorun();
}


int RPCServer::readListenPort()
{
    if (DBHandler::getInstance()->querySize("SELECT `setting`,`parameter1` FROM `settings` WHERE `setting`='rpcserver_listenport'") <= 0)
    {
        DBHandler::getInstance()->runQuery(QString("INSERT INTO `settings` (`setting`, `parameter1`) VALUES('rpcserver_listenport','9500')"));
        qDebug() << "rpcserver_listenport setting wasn't set in the database. Inserting default: 9500";
        m_rpcserver_listenPort = 9500;
    }
    else
    {
        if (DBHandler::getInstance()->runQuery("SELECT `setting`,`parameter1` FROM `settings` WHERE `setting`='rpcserver_listenport'"))
        {
            m_rpcserver_listenPort = DBHandler::getInstance()->queryString("SELECT `parameter1` FROM `settings` WHERE `setting`='rpcserver_listenport'", 0, 0).toInt();
        }
        if (m_rpcserver_listenPort <= 0 || m_rpcserver_listenPort > 65535)
        {
            m_rpcserver_listenPort = 9500;
            qDebug() << "Error loading rpcserver_listenport setting from SQL, using default: 9500";
        }
    }

    return m_rpcserver_listenPort;
}

void RPCServer::setListenPort(int port)
{

    if(port <= 0 || port > 65535)
    {
        qDebug() << "rpcserver_listenport number selection not in valid range (1-65535). Attempted: " << port;
        return;
    }

    if (DBHandler::getInstance()->querySize("SELECT `setting`,`parameter1` FROM `settings` WHERE `setting`='rpcserver_listenport'") <= 0)
    {
        DBHandler::getInstance()->runQuery(QString("INSERT INTO `settings` (`setting`, `parameter1`) VALUES('rpcserver_listenport','%1')").arg(port));
        qDebug() << "rpcserver_listenport setting wasn't set in the database. Inserting: " << port;
    }
    else
    {
        DBHandler::getInstance()->runQuery(QString("INSERT OR REPLACE INTO 'settings' VALUES('rpcserver_listenport','%1')").arg(port));
    }

    readListenPort();
}




RPCServer::~RPCServer()
{

}
