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
    void refreshAPIKey();
    QString getAPIKey();
    QString getUsername(){return m_username;}
    QString getPassword(){return m_password;}

    void setKeyTimeout(int seconds); // timeout in seconds
    bool isKeyActive(){return m_keyActive;}


private:

    QString m_username;
    QString m_password;

    QTime m_APIKeyTimestamp;
    int m_keyLength;
    int m_keyTimeout;
    QString m_APIKey;

    bool m_keyActive;
    void resetTimeStamp();

};

#endif // RPCUSER_H
