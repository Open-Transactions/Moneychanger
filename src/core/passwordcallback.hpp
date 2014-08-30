#ifndef PASSWORDCALLBACK_HPP
#define PASSWORDCALLBACK_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <opentxs/core/OTPassword.hpp>
#include <opentxs/core/OTCallback.hpp>

class MTPasswordCallback : public opentxs::OTCallback
{
public:
    void runOne(const char * szDisplay, opentxs::OTPassword & theOutput);
    void runTwo(const char * szDisplay, opentxs::OTPassword & theOutput);
};

#endif // PASSWORDCALLBACK_HPP
