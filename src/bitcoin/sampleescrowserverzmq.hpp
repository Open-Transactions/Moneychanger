#ifndef SAMPLEESCROWSERVERZMQ_H
#define SAMPLEESCROWSERVERZMQ_H

#include "sampleescrowserver.h"
#include "samplenetmessages.h"

class SampleEscrowServerZmq : public SampleEscrowServer
{
public:
    BitcoinServerPtr serverInfo;

    SampleEscrowServerZmq(BitcoinServerPtr bitcoind, int port);

    SampleEscrowServerZmq(BitcoinServerPtr bitcoind, const std::string &url, int port);

    void StartServer();

    void OnGetMultiSigPubKey(const std::string &sender);

    void OnRequestEscrowDeposit(SampleEscrowClient *client);

    void OnRequestEscrowDeposit(const std::string &sender, int64_t amount);

    void OnIncomingDeposit(std::string txId);

    void SendData(BtcNetMsg *message);
};

#endif // SAMPLEESCROWSERVERZMQ_H
