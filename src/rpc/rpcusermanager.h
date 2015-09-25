#ifndef RPCUSERMANAGER_H
#define RPCUSERMANAGER_H

#include <vector>
#include <string>
#include "core/handlers/DBHandler.hpp"
#include "rpcuser.h"

class RPCUserManager
{
public:
    RPCUserManager();
    ~RPCUserManager();

    bool activateUserAccount(QString Username, QString Password);
    bool deactivateUserAccount(QString Username);

    bool validateAPIKey(QString Username, QString APIKey);

private:

    std::vector<RPCUser> m_userList;

    bool validateUserInDatabase(QString Username, QString Password);
    bool checkUserExistsInDatabase(QString Username);
    bool addUserToDatabase(QString Username, QString Password);



};

#endif // RPCUSERMANAGER_H
