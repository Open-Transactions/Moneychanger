#ifndef RPCSERVER_H
#define RPCSERVER_H

#include <qjsonrpchttpserver.h>

class RPCServer
{
  private:

    static RPCServer * _instance;

    QJsonRpcHttpServer m_httpserver;

    // Our Config Settings
    QString m_rpcserver_autorun;
    int m_rpcserver_listenPort;


  protected:
    RPCServer();

  public:
    static RPCServer * getInstance();

    // This is an empty function as initialization
    // Will occur at the class construction time
    void init(){};

    bool startListener();
    bool stopListener();

    void readConfig();

    void readAutorun();
    void setAutorun(bool setting);

    int readListenPort();
    void setListenPort(int port);

    void setupDebugUser();

    ~RPCServer();
};

#endif // RPCSERVER_H
