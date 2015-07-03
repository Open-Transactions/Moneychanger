#include "rpcserver.h"
#include "mcrpcservice.h"

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
    m_listenPort = 9500;
    readConfig();

}

bool RPCServer::startListener()
{
    if(m_httpserver.isListening()){
        qDebug() << "http service already started";
        return false;
    }
    if (!m_httpserver.listen(QHostAddress::LocalHost, m_listenPort)) {
        qDebug() << "Could not start http service" << m_httpserver.errorString();
        return false;
    }
    else{
        qDebug() << "http service started on port: " << m_listenPort;
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
        return false;
    }
}


void RPCServer::readConfig()
{

}

RPCServer::~RPCServer()
{

}
