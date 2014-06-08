#ifndef SAMPLEESCROWSERVERZMQ_H
#define SAMPLEESCROWSERVERZMQ_H

#include "sampleescrowserver.hpp"
#include "samplenetmessages.hpp"

class SampleEscrowServerZmq : public SampleEscrowServer
{
public:
    BitcoinServerPtr serverInfo;

    SampleEscrowServerZmq(BitcoinServerPtr bitcoind, int port);

    SampleEscrowServerZmq(BitcoinServerPtr bitcoind, const std::string &url, int port);

    void StartServer();

    void ClientConnected(SampleEscrowClient *client);
    void ClientConnected(BtcNetMsg clientMsg);

    // called when someone wants to make a deposit
    virtual bool RequestEscrowDeposit(const std::string &sender, const int64_t &amount);
    bool RequestEscrowDeposit(BtcNetMsgReqDepositPtr deposit);

    virtual std::string CreatePubKey(const std::string &client);

    // called by other servers in the pool so they get eachothers' public keys for multi-sig
    virtual std::string GetPubKey(const std::string &client);
    std::string GeGetPubKey(BtcNetMsgGetKeyPtr sender);

    virtual void AddPubKey(const std::string &client, const std::string &key);

    //virtual void OnReceivePubKey(const std::string &sender, const std::string &key);

    virtual bool RequestEscrowWithdrawal(const std::string &sender, const int64_t &amount, const std::string &toAddress);

    void SendData(BtcNetMsg *message);
};

#endif // SAMPLEESCROWSERVERZMQ_H
