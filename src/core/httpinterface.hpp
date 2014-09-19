#ifndef HTTPINTERFACE_HPP
#define HTTPINTERFACE_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <opentxs/client/OT_ME.hpp>

class HTTPInterface
{
public:
    HTTPInterface();

private:
    opentxs::OT_ME   mMadeEasy;
};

#endif // HTTPINTERFACE_HPP
