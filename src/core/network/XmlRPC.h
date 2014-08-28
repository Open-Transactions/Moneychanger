//
//  XmlRPC.h
//

#include <cassert>
#include <cstdlib>
#include <string>
#include <iostream>
#include <vector>
#include <utility>

#include <xmlrpc-c/girerr.hpp>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/client.hpp>
#include <xmlrpc-c/timeout.hpp>
#include <xmlrpc-c/xml.hpp>

#if MSVCRT
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  include <process.h>
#else
#  include <unistd.h>
#endif

typedef std::pair<bool, xmlrpc_c::value> XmlResponse;

class XmlRPC {
    
public:
    
    XmlRPC(std::string serverurl, int port=80, bool authrequired=false, int Timeout=10000);
    ~XmlRPC(){};
    
    XmlResponse run(std::string methodName, std::vector<xmlrpc_c::value> parameters);
    void setTimeout(int Timeout);
    void setAuth(std::string user, std::string pass);
    void toggleAuth(bool toggle);
    
private:
    
    // Address Settings
    std::string m_serverurl;
    int m_port;
    
    // Transport Settings
    int m_timeout;
    xmlrpc_c::clientXmlTransport_curl transport;
    
    // Auth Variables
    bool m_authrequired;
    bool m_authset;
    
    std::string m_authuser;
    std::string m_authpass;
    
    void xmlrpc_millisecond_sleep(unsigned int const milliseconds);
    
};