#ifndef SAMPLEESCROWSERVERZMQ_H
#define SAMPLEESCROWSERVERZMQ_H

#include "sampleescrowserver.hpp"
#include "samplenetmessages.hpp"

class SampleEscrowServerZmq : public SampleEscrowServer
{
public:
    BitcoinServerPtr serverInfo;

    SampleEscrowServerZmq(BitcoinServerPtr bitcoind, EscrowPoolPtr pool, int port);

    SampleEscrowServerZmq(BitcoinServerPtr bitcoind, EscrowPoolPtr pool, const std::string &url, int port);

    void StartServer();

    void ClientConnected(SampleEscrowClient *client);
    void ClientConnected(BtcNetMsgConnectPtr clientMsg);

    // called when someone wants to make a deposit
    virtual bool RequestEscrowDeposit(const std::string &client, const int64_t &amount);
    virtual bool RequestEscrowDeposit(BtcNetMsgReqDepositPtr message);

    virtual std::string RequestDepositAddress(const std::string &client);
    virtual std::string RequestDepositAddress(BtcNetMsgGetDepositAddrPtr message);

    // called by other servers in the pool so they get eachothers' public keys for multi-sig
    virtual std::string GetPubKey(const std::string &client);
    virtual std::string GetPubKey(BtcNetMsgGetKeyPtr message);

    virtual int64_t GetClientBalance(const std::string& client);
    virtual int64_t GetClientBalance(BtcNetMsgGetBalancePtr message);

    virtual int32_t GetClientTransactionCount(const std::string &client);
    virtual int32_t GetClientTransactionCount(BtcNetMsgGetTxCountPtr message);

    virtual SampleEscrowTransactionPtr GetClientTransaction(const std::string &client, const uint32_t txIndex);
    virtual SampleEscrowTransactionPtr GetClientTransaction(BtcNetMsgGetTxPtr message);

    virtual bool RequestEscrowWithdrawal(const std::string &sender, const int64_t &amount, const std::string &toAddress);
    virtual bool RequestEscrowWithdrawal(BtcNetMsgReqWithdrawPtr message);

    // called from server to server
    // returns a partially signed raw transaction
    virtual std::string RequestSignedWithdrawal(const std::string &client);
    virtual std::string RequestSignedWithdrawal(BtcNetMsgReqSignedTxPtr message);

    void SendData(BtcNetMsg *message);
};

typedef _SharedPtr<SampleEscrowServerZmq> SampleEscrowServerZmqPtr;

#endif // SAMPLEESCROWSERVERZMQ_H
