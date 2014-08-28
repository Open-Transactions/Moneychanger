#ifndef PASSWORDCALLBACK_HPP
#define PASSWORDCALLBACK_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <opentxs/OTPassword.hpp>
#include <opentxs/OTCallback.hpp>

class MTPasswordCallback : public OTCallback
{
public:
    void runOne(const char * szDisplay, OTPassword & theOutput);
    void runTwo(const char * szDisplay, OTPassword & theOutput);
};

#endif // PASSWORDCALLBACK_HPP
