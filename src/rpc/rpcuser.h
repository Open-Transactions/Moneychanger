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

    RPCUser(std::string Username, std::string Password);
    ~RPCUser();

    std::string generateAPIKey(int length=32);
    bool checkAPIKey(std::string APIKey);
    std::string getUsername(){return m_username;}
    std::string getPassword(){return m_password;}
    void resetTimeStamp();


private:

    std::string m_username;
    std::string m_password;

    QTime m_APIKeyTimestamp;
    std::string m_APIKey;

};

#endif // RPCUSER_H
