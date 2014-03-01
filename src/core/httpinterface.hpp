#ifndef HTTPINTERFACE_HPP
#define HTTPINTERFACE_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <opentxs/OT_ME.hpp>

class HTTPInterface
{
public:
    HTTPInterface();

private:
    OT_ME   mMadeEasy;
};

#endif // HTTPINTERFACE_HPP
