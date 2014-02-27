#ifndef HTTPINTERFACE_HPP
#define HTTPINTERFACE_HPP

#include <WinsockWrapper.h>
#include <ExportWrapper.h>


#ifdef _WIN32
#include <otapi/OT_ME.hpp>
#else
#include <opentxs/OT_ME.hpp>
#endif

#include <qjsonrpc/qjsonrpcmessage.h>

class HTTPInterface
{
public:
    HTTPInterface();

private:
    OT_ME   mMadeEasy;
};

#endif // HTTPINTERFACE_HPP
