#include "rpcusermanager.h"

RPCUserManager::RPCUserManager()
{

}


RPCUserManager::~RPCUserManager()
{

}


bool RPCUserManager::activateUserAccount(QString Username, QString Password)
{

    if(checkUserExistsInDatabase(Username)){
        if(validateUserInDatabase(Username, Password)){
            if(!checkUserActivated(Username))
                m_userList.push_back(RPCUser(Username, Password));
            return true;
        }
        else{
            qDebug() << "RPC Username / Password combination incorrect";
            return false;
        }
    }
    else{
        /*
        if(addUserToDatabase(Username, Password)){
            if(!checkUserActivated(Username))
                m_userList.push_back(RPCUser(Username, Password));
            return true;
        }
        else{
            qDebug() << "Error adding User to Database";
            return false;
        }
        */
        qDebug() << "Error: activateUserAccount() called for User that does not exist in Database: " + Username;
        return false;
    }
}


bool RPCUserManager::deactivateUserAccount(QString Username)
{
    if(checkUserActivated(Username)){
        for(size_t x = 0; x < m_userList.size(); x++){
            if(m_userList.at(x).getUsername() == Username){
                m_userList.erase(m_userList.begin() + x);
                return true;
            }
        }
        qDebug() << "Error: Could not find User - " + Username + " in Userlist";
        return false;
    }
    else{
        qDebug() << "Error: deactivateUserAccount() called for inactive user";
        return false;
    }
}


bool RPCUserManager::validateAPIKey(QString Username, QString APIKey)
{
    if(checkUserActivated(Username))
    {
        for(size_t x = 0; x < m_userList.size(); x++){
            if(m_userList.at(x).getUsername() == Username){
                if(m_userList.at(x).checkAPIKey(APIKey))
                    return true;
                else
                    return false;
            }
        }
        return true;
    }
    else{
        qDebug() << "Error: validateAPIKey() called on inactive User: " + Username;
        return false;
    }

}

QString RPCUserManager::getAPIKey(QString Username)
{
    if(!checkUserActivated(Username))
        return "Error: User Not Activated";

    else{
        for(size_t x = 0; x < m_userList.size(); x++){
            if(m_userList.at(x).getUsername() == Username){
                return m_userList.at(x).getAPIKey();
            }
        }
        return "Error: RPCUserManager::getAPIKey("+Username+") - User Not Found";
    }
}


void RPCUserManager::setTimeoutForUser(QString Username, int seconds)
{
    for(size_t x = 0; x < m_userList.size(); x++){
        if(m_userList.at(x).getUsername() == Username){
            m_userList.at(x).setKeyTimeout(seconds);
            return;
        }
    }
}


void RPCUserManager::setGlobalTimeout(int seconds)
{
    for(size_t x = 0; x < m_userList.size(); x++){
        m_userList.at(x).setKeyTimeout(seconds);
    }
}


bool RPCUserManager::validateUserInDatabase(QString Username, QString Password)
{
    if(!isStringSanitized(Username))
        return false;

    auto user_check = DBHandler::getInstance()->runQuery(QString("SELECT `user_id` FROM `rpc_users` WHERE `user_id`='%1' AND `password`='%2'").arg(Username).arg(Password));

    if(user_check)
        return true;
    else
        return false;
}

bool RPCUserManager::checkUserActivated(QString Username)
{
    for(size_t x = 0; x < m_userList.size(); x++){
        if(m_userList.at(x).getUsername() == Username){
            return true;
        }
    }
    return false;
}


bool RPCUserManager::isStringSanitized(QString input)
{
    std::string s(input.toStdString());
    std::string illegalChars = "\\/:?\"<>|";
    for (auto it = s.begin(); it < s.end() ; ++it){
        bool found = illegalChars.find(*it) != std::string::npos;
        if(found){
            return false;
        }
    }
    return true;
}

bool RPCUserManager::checkUserExistsInDatabase(QString Username)
{
    if(!isStringSanitized(Username))
        return false;

    auto user_check = DBHandler::getInstance()->queryString(QString("SELECT `user_id` FROM `rpc_users` WHERE `user_id`='%1'").arg(Username), 0, 0);

    if(user_check.isEmpty())
        return false;
    else
        return true;
}


bool RPCUserManager::addUserToDatabase(QString Username, QString Password)
{
    if(!isStringSanitized(Username))
        return false;

    auto added_user = DBHandler::getInstance()->runQuery(QString("INSERT INTO `rpc_users` (`user_id`,`rpc_users`) VALUES('%1','%2')").arg(Username).arg(Password));

    if(added_user)
        return true;
    else
        return false;
}

