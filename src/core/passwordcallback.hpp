#ifndef PASSWORDCALLBACK_HPP
#define PASSWORDCALLBACK_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <opentxs/opentxs.hpp>

#ifndef OT_NO_PASSWORD
class MTPasswordCallback : public opentxs::OTCallback
{
public:
    void runOne(const char * szDisplay, opentxs::OTPassword & theOutput) const;
    void runTwo(const char * szDisplay, opentxs::OTPassword & theOutput) const;
};
#endif

#endif // PASSWORDCALLBACK_HPP
