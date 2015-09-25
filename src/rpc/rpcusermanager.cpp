#include "rpcusermanager.h"

RPCUserManager::RPCUserManager()
{

}


RPCUserManager::~RPCUserManager()
{

}


bool RPCUserManager::activateUserAccount(QString Username, QString Password)
{

    return false;
}


bool RPCUserManager::deactivateUserAccount(QString Username)
{

    return false;
}


bool RPCUserManager::validateAPIKey(QString Username, QString APIKey)
{

    if(checkUserExistsInDatabase(Username))
    {
        return true;
    }
    else
        return false;

}


bool RPCUserManager::validateUserInDatabase(QString Username, QString Password)
{

    auto user_check = DBHandler::getInstance()->runQuery(QString("SELECT `user_id` FROM `rpc_users` WHERE `user_id`='%1' AND `password`='%2'").arg(Username).arg(Password));

    if(user_check)
        return true;
    else
        return false;

}


bool RPCUserManager::checkUserExistsInDatabase(QString Username)
{
    auto user_check = DBHandler::getInstance()->queryString(QString("SELECT `user_id` FROM `rpc_users` WHERE `user_id`='%1'").arg(Username), 0, 0);

    if(user_check.isEmpty())
        return false;
    else
        return true;
}


bool RPCUserManager::addUserToDatabase(QString Username, QString Password)
{
    auto added_user = DBHandler::getInstance()->runQuery(QString("INSERT INTO `rpc_users` (`user_id`,`rpc_users`) VALUES('%1','%2')").arg(Username).arg(Password));

    if(added_user)
        return true;
    else
        return false;
}
