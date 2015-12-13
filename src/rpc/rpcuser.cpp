#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include "rpcuser.h"

#include <opentxs/core/crypto/OTPassword.hpp>

#include <QDebug>

#include <string>


RPCUser::RPCUser(QString Username, QString Password)
{
    m_username = Username;
    m_password = Password;
    m_keyLength = 32;
    setKeyTimeout(300); // 5 Minute Default
    m_keyActive = false;
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
                else{
                    m_keyActive = false;
                    return false;
                }
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

    //opentxs::OTPassword::BlockSize passwordSize = opentxs::OTPassword::BlockSize(length);
    opentxs::OTPassword password;

    password.randomizePassword(length);

    std::string sanitizedString(password.getPassword());
    std::string illegalChars = "\\/:;%$!@*`'?\"<>|";
    for (auto it = sanitizedString.begin(); it < sanitizedString.end() ; ++it){
        bool found = illegalChars.find(*it) != std::string::npos;
        if(found){
            sanitizedString.erase(it);
        }
    }

    QString outputString(sanitizedString.c_str());

    m_APIKeyTimestamp.start();

    m_APIKey = outputString;

    qDebug() << "User API Key: " + QString(outputString);

    m_keyActive = true;

    return outputString;

}

QString RPCUser::getAPIKey()
{
    if(!m_keyActive)
        generateAPIKey();

    return m_APIKey;
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
