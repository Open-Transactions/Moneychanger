#ifndef HTTPINTERFACE_H
#define HTTPINTERFACE_H

#include <opentxs/OT_ME.h>

#include "qjsonrpchttpclient.h"
#include "qjsonrpcmessage.h"

class HTTPInterface
{
public:
    HTTPInterface();

private:
    OT_ME   mMadeEasy;
};

#endif // HTTPINTERFACE_H
