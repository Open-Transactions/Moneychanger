#include "rpcuser.h"
#include <QDebug>

RPCUser::RPCUser(QString Username, QString Password)
{
    m_username = Username;
    m_password = Password;
    m_keyLength = 32;
    setKeyTimeout(300); // 5 Minute Default
}

RPCUser::~RPCUser()
{

}


bool RPCUser::checkAPIKey(QString APIKey)
{
    if(APIKey != ""){
        if(m_APIKey == ""){
            qDebug() << "Error: User API Key not Initialized!";
            return false;
        }
        else{
            if(APIKey == m_APIKey){
                // 300,000 ms to 5 minutes
                if(m_APIKeyTimestamp.elapsed() < m_keyTimeout)
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


QString RPCUser::generateAPIKey(int length)
{
    m_keyLength = length;

    opentxs::OTPassword::BlockSize passwordSize = opentxs::OTPassword::BlockSize(length);
    opentxs::OTPassword password(passwordSize);


    password.randomizePassword(length);

    QString outputString(password.getPassword());

    m_APIKeyTimestamp.start();

    m_APIKey = outputString;

    return outputString;

}

void RPCUser::refreshAPIKey()
{
    m_APIKey = generateAPIKey(m_keyLength);
}

void RPCUser::setKeyTimeout(int seconds)
{
    // m_keyTimeout is time in milliseconds
    m_keyTimeout = seconds * 1000;

}
