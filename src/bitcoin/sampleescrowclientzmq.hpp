#ifndef SAMPLEESCROWCLIENTZMQ_H
#define SAMPLEESCROWCLIENTZMQ_H

#include "sampleescrowclient.hpp"
#include "samplenetmessages.hpp"

#include <opentxs/TR1_Wrapper.hpp>

class SampleEscrowClientZmq : public SampleEscrowClient
{
public:
    SampleEscrowClientZmq();

    SampleEscrowClientZmq(int port);

    void SendData(BtcNetMsg* message);

    void Startserver();

    BitcoinServerPtr serverInfo;
};

    typedef _SharedPtr<SampleEscrowClientZmq> SampleEscrowClientZmqPtr;

#endif // SAMPLEESCROWCLIENTZMQ_H
