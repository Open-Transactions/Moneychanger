#ifndef RPCSERVER_H
#define RPCSERVER_H

#include <qjsonrpchttpserver.h>

class RPCServer
{
  private:
    static RPCServer * _instance;

    QJsonRpcHttpServer m_httpserver;
    int m_listenPort;

  protected:
    RPCServer();

  public:
    static RPCServer * getInstance();

    bool startListener();
    bool stopListener();

    void readConfig();

    ~RPCServer();
};

#endif // RPCSERVER_H
