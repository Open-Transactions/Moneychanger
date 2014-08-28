#include <QObject>
#include <QString>
#include <QDebug>

#include "mtcomms.h"

#include <core/network/Network.h>
#include <core/network/BitMessage.h>

MTComms::MTComms()
{

}

MTComms::~MTComms()
{
    it_clearmodules();
}

void MTComms::it_clearmodules()
{
    while (m_mapModules.size() > 0)
    {
        mapOfNetModules::iterator it = m_mapModules.begin();

        NetworkModule * pModule = it->second;

        if (NULL != pModule)
        {
            m_mapModules.erase(it);
            delete pModule;
        }
    }
}


//typedef mapOfNetModules  std::map<std::string, NetworkModule *>;

//static
MTComms * MTComms::s_pIt = NULL;




NetworkModule * MTComms::it_find(const std::string commstring)
{
    for (mapOfNetModules::iterator it = m_mapModules.begin(); it != m_mapModules.end(); ++it)
    {
        NetworkModule * pModule = it->second;

        if (NULL != pModule)
        {
            if (0 == commstring.compare(pModule->getCommstring()))
                return pModule;
        }
    }
    return NULL;
}

bool MTComms::it_get(const std::string type, mapOfNetModules & mapOutput)
{
    bool bFoundAny = false;

    for(mapOfNetModules::iterator it = m_mapModules.begin(); it != m_mapModules.end(); ++it)
    {
        // -----------------------------
        const std::string & current_type = it->first;
        // -----------------------
        if (current_type != type)
            continue;
        // -----------------------
        NetworkModule * pModule = it->second;

        if (NULL != pModule)
        {
            bFoundAny = true;

            mapOutput.insert(std::pair<std::string, NetworkModule *>(current_type, pModule));
        }
    }

    return bFoundAny;
}



std::string MTComms::it_connectPlaceholder(const std::string strType)
{
    if (0 == strType.compare("bitmessage"))
        return "127.0.0.1,8442,username,password"; // todo hardcoded

    return "";
}

std::string MTComms::it_displayName(const std::string strType)
{
    mapOfCommTypes mapTypes;

    if (this->it_types(mapTypes))
    {
        for(mapOfCommTypes::iterator it = mapTypes.begin(); it != mapTypes.end(); ++it)
        {
            std::string str_type = it->first;
            std::string str_name = it->second;

            if (!str_type.empty() && (0 == str_type.compare(strType)))
                return str_name;
        }
    }

    return "";
}

bool MTComms::it_types(mapOfCommTypes & mapTypes)
{
    QString    qstrBitmessage = QObject::tr("Bitmessage");
    std::string strBitmessage = qstrBitmessage.toStdString();

    mapTypes.insert(std::pair<std::string, std::string>("bitmessage", strBitmessage));

    return true;
}

bool MTComms::it_add(const std::string type, const std::string commstring)
{
    // See if it's already there with the same type and commstring.
    //
    mapOfNetModules mapOutput;

    if (this->it_get(type, mapOutput))
    {
        // Next we loop through all comms of a specific type, to see if there's one
        // with the same commstring.
        //
        for(mapOfNetModules::iterator it = mapOutput.begin(); it != mapOutput.end(); ++it)
        {
            NetworkModule * pModule = it->second;

            if (NULL != pModule)
            {
                if (0 == commstring.compare(pModule->getCommstring())) // pModule's commstring matches the one passed in (we already have it.)
                    return false;
            }
        }
    }
    // ----------------------------------------------------
    // If we got this far, that means it wasn't already there,
    // so we can add it.
    //
    if (0 == type.compare("bitmessage"))
    {
//      qDebug() << QString("Trying to add Bitmessage module with commstring: %1").arg(QString::fromStdString(commstring));

        NetworkModule * pModule = new BitMessage(commstring);

        if (NULL != pModule)
        {
            m_mapModules.insert(std::pair<std::string, NetworkModule *>(type, pModule));
            return true;
        }
    }
    // ----------------------------------------------------
    return false;
}

//static
bool MTComms::add(const std::string type, const std::string commstring)
{
    return MTComms::it()->it_add(type, commstring);
}

//static
bool MTComms::get(const std::string type, mapOfNetModules & mapOutput)
{
    return MTComms::it()->it_get(type, mapOutput);
}

//static
NetworkModule * MTComms::find(const std::string commstring)
{
    return MTComms::it()->it_find(commstring);
}

//static
std::string MTComms::displayName(const std::string strType)
{
    return MTComms::it()->it_displayName(strType);
}

//static
std::string MTComms::connectPlaceholder(const std::string strType)
{
    return MTComms::it()->it_connectPlaceholder(strType);
}


//static
bool MTComms::types(mapOfCommTypes & mapTypes)
{
    return MTComms::it()->it_types(mapTypes);
}


//static
MTComms * MTComms::it()
{
    if (NULL == MTComms::s_pIt)
        MTComms::s_pIt = new MTComms;

    return MTComms::s_pIt;
}
