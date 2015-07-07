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

    // opentxs::OTAPI methods
    QJsonValue getAccountCount();

    // Moneychanger::It() methods
    QString mcSendDialog(QString Account, QString Recipient,
                         QString Asset, QString Amount);


};

#endif // MCRPCSERVICE_H
