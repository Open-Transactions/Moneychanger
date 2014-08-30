#ifndef SAMPLEESCROWSERVERZMQ_H
#define SAMPLEESCROWSERVERZMQ_H

#include "sampleescrowserver.hpp"
#include "samplenetmessages.hpp"

//#ifdef OT_USE_ZMQ4
    #include <zmq.h>
//#else
//    #include <zmq.hpp>
//#endif

class QTime;

class SampleEscrowServerZmq : public SampleEscrowServer
{
public:
    BitcoinServerPtr serverInfo;

    SampleEscrowServerZmq(BitcoinServerPtr bitcoind, EscrowPoolPtr pool, int port);

    typedef _SharedPtr<SampleEscrowServerZmq> SampleEscrowServerZmqPtr;
    SampleEscrowServerZmq(BitcoinServerPtr bitcoind, EscrowPoolPtr pool, const std::string &url, int port, SampleEscrowServerZmqPtr master = SampleEscrowServerZmqPtr());

    ~SampleEscrowServerZmq();

    void StartServer();
    void UpdateServer();

    bool ClientConnected(SampleEscrowClient *client);
    bool ClientConnected(BtcNetMsgConnectPtr clientMsg);

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

    virtual u_int64_t GetClientTransactionCount(const std::string &client);
    virtual u_int64_t GetClientTransactionCount(BtcNetMsgGetTxCountPtr message);

    virtual SampleEscrowTransactionPtr GetClientTransaction(const std::string &client, const u_int64_t txIndex);
    virtual SampleEscrowTransactionPtr GetClientTransaction(BtcNetMsgGetTxPtr message);

    virtual bool RequestEscrowWithdrawal(const std::string &sender, const int64_t &amount, const std::string &toAddress);
    virtual bool RequestEscrowWithdrawal(BtcNetMsgReqWithdrawPtr message);

    // called from server to server
    // returns a partially signed raw transaction
    virtual std::string RequestSignedWithdrawal(const std::string &client);
    virtual std::string RequestSignedWithdrawal(BtcNetMsgReqSignedTxPtr message);

    BtcNetMsg *SendData(BtcNetMsg *message);

private:
    typedef void zmq_socket_t;
    typedef void zmq_context_t;
    zmq_socket_t* serverSocket;
    zmq_context_t* context;
    SampleEscrowServerZmqPtr master;
    std::string connectString;

public slots:
    virtual void Update();
};

typedef _SharedPtr<SampleEscrowServerZmq> SampleEscrowServerZmqPtr;

#endif // SAMPLEESCROWSERVERZMQ_H
