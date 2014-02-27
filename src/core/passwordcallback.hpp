#ifndef PASSWORDCALLBACK_H
#define PASSWORDCALLBACK_H

#ifdef _WIN32
#include <otlib/OTPassword.hpp>
#else
#include <opentxs/OTPassword.hpp>
#endif

class MTPasswordCallback : public OTCallback
{
public:
    void runOne(const char * szDisplay, OTPassword & theOutput);
    void runTwo(const char * szDisplay, OTPassword & theOutput);
};

#endif // PASSWORDCALLBACK_H
