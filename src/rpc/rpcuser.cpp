#include "rpcuser.h"
#include <QDebug>

RPCUser::RPCUser(std::string Username, std::string Password)
{
    m_username = Username;
    m_password = Password;
}

RPCUser::~RPCUser()
{

}


bool RPCUser::checkAPIKey(std::string APIKey)
{
    if(APIKey != ""){
        if(m_APIKey == ""){
            qDebug() << "Error: User API Key not Initialized!";
            return false;
        }
        else{
            if(APIKey == m_APIKey){
                // 300,000 ms to 5 minutes
                if(m_APIKeyTimestamp.elapsed() < 300000)
                {
                    resetTimeStamp();
                    return true;
                }
                else
                    return false;
            }
            else
                return false;
        }
    }
    else{
        qDebug() << "Error: checkAPIKey() on Empty Value!";
        return false;
    }
}


void RPCUser::resetTimeStamp()
{
    // Note that the counter wraps to zero
    // 24 hours after the last call to start() or restart.

    m_APIKeyTimestamp.restart();
}


std::string RPCUser::generateAPIKey(int length)
{

    opentxs::OTPassword::BlockSize passwordSize = opentxs::OTPassword::BlockSize(length);
    opentxs::OTPassword password(passwordSize);


    password.randomizePassword(length);

    std::string outputString(password.getPassword());

    m_APIKeyTimestamp.start();

    m_APIKey = outputString;

    return outputString;

}
