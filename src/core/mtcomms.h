#ifndef MTCOMMS_H
#define MTCOMMS_H

#include <string>
#include <map>

class NetworkModule;

typedef  std::multimap<std::string, NetworkModule *>  mapOfNetModules;  // May be multiple "bitmessage" keys.
typedef  std::map<std::string, std::string>           mapOfCommTypes;   // Can only be one "bitmessage" key.


class MTComms
{
    MTComms();
    ~MTComms();

    mapOfNetModules m_mapModules;

    static MTComms * s_pIt;

public:
    void            it_clearmodules();
    bool            it_add(const std::string type, const std::string commstring);
    bool            it_get(const std::string type, mapOfNetModules & mapOutput);
    NetworkModule * it_find(const std::string commstring);
    bool            it_types(mapOfCommTypes & mapTypes);
    std::string     it_displayName(const std::string strType);
    std::string     it_connectPlaceholder(const std::string strType);

    static bool            add(const std::string type, const std::string commstring);
    static bool            get(const std::string type, mapOfNetModules & mapOutput);
    static NetworkModule * find(const std::string commstring);
    static bool            types(mapOfCommTypes & mapTypes);
    static std::string     displayName(const std::string strType);
    static std::string     connectPlaceholder(const std::string strType);

    static MTComms       * it ();
};

#endif // MTCOMMS_H

