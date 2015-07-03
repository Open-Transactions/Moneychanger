#ifndef MCRPCSERVICE_H
#define MCRPCSERVICE_H

#include <qjsonrpcservice.h>

class MCRPCService : public QJsonRpcService
{
private:
    Q_OBJECT
    Q_CLASSINFO("serviceName", "moneychanger")

public:
    MCRPCService(QObject *parent = 0);

public Q_SLOTS:

    void testMethod();

};

#endif // MCRPCSERVICE_H
