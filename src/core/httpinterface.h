#ifndef HTTPINTERFACE_H
#define HTTPINTERFACE_H


#ifdef _WIN32
#include <otapi/OT_ME.h>
#else
#include <opentxs/OT_ME.h>
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
