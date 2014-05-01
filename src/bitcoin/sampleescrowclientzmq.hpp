#ifndef SAMPLEESCROWCLIENTZMQ_H
#define SAMPLEESCROWCLIENTZMQ_H

#include "sampleescrowclient.h"
#include "samplenetmessages.h"

class SampleEscrowClientZmq : public SampleEscrowClient
{
public:
    SampleEscrowClientZmq();

    SampleEscrowClientZmq(int port);

    void SendData(BtcNetMsg* message);

    void Startserver();

    BitcoinServerPtr serverInfo;
};

#ifndef OT_USE_TR1
    typedef std::shared_ptr<SampleEscrowClientZmq> SampleEscrowClientZmqPtr;
#else
    typedef std::tr1::shared_ptr<SampleEscrowClientZmq> SampleEscrowClientZmqPtr;
#endif

#endif // SAMPLEESCROWCLIENTZMQ_H
