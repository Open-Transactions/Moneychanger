#ifndef HTTPINTERFACE_H
#define HTTPINTERFACE_H


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

#endif // HTTPINTERFACE_H
