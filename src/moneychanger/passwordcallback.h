#ifndef PASSWORDCALLBACK_H
#define PASSWORDCALLBACK_H

#include <opentxs/OTPassword.h>

class MTPasswordCallback : public OTCallback
{
public:
    void runOne(const char * szDisplay, OTPassword & theOutput);
    void runTwo(const char * szDisplay, OTPassword & theOutput);
};

#endif // PASSWORDCALLBACK_H
