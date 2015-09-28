#ifndef RPCUSER_H
#define RPCUSER_H

#include <string>
#include <QTime>

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include "opentxs/core/crypto/OTPassword.hpp"


class RPCUser
{

public:

    RPCUser(QString Username, QString Password);
    ~RPCUser();

    QString generateAPIKey(int length=32);
    bool checkAPIKey(QString APIKey);
    QString getUsername(){return m_username;}
    QString getPassword(){return m_password;}
    void resetTimeStamp();


private:

    QString m_username;
    QString m_password;

    QTime m_APIKeyTimestamp;
    QString m_APIKey;

};

#endif // RPCUSER_H
