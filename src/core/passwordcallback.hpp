#ifndef PASSWORDCALLBACK_HPP
#define PASSWORDCALLBACK_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <opentxs/core/Version.hpp>
#include <opentxs/core/crypto/OTCallback.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>

class MTPasswordCallback : public opentxs::OTCallback
{
public:
    void runOne(const char * szDisplay, opentxs::OTPassword & theOutput) const;
    void runTwo(const char * szDisplay, opentxs::OTPassword & theOutput) const;
};

#endif // PASSWORDCALLBACK_HPP
