#ifndef SAMPLENETMESSAGES_H
#define SAMPLENETMESSAGES_H

#include <map>
#include <opentxs/TR1_Wrapper.hpp>


enum NetMessageType
{
    Unknown = 0,
    Ping,
    Connect,
    ReqDeposit,
    DepositReply,
    GetMultiSigAddr,
    MultiSigAddr,
    GetMultiSigKey,
    MultiSigKey,
    GetBalance,
    Balance,
    GetTxCount,
    TxCount,
    GetTx,
    Tx,
    RequestRelease,
    WithdrawReply,
    ReqSignedTx,
    SignedTx
};


// note: on my system if if an int64 follows an int32 the int32 will still take 8 bytes of memory

#define NetMsgSize 8
union BtcNetMsg
{
    struct
    {
        int64_t MessageType;
    };
    char data[NetMsgSize];

    BtcNetMsg();    // wow, unions can contain constructors.
};

#define NetMsgConnectSize 8 + 20
union BtcNetMsgConnect
{
    struct
    {
        int64_t MessageType;
        char client[20];
    };
    char data[NetMsgConnectSize];

    BtcNetMsgConnect();
};

#define NetMsgReqDepositSize 8 + 8 + 20
union BtcNetMsgReqDeposit
{
    struct
    {
        int64_t MessageType;
        int64_t amount;
        char client[20];
    };
    char data[NetMsgReqDepositSize];

    BtcNetMsgReqDeposit();
};

#define NetMsgDepositReplySize 8 + 1
union BtcNetMsgDepositReply
{
    struct
    {
        int64_t MessageType;
        int8_t accepted;
    };
    char data[NetMsgDepositReplySize];

    BtcNetMsgDepositReply();
};

#define NetMsgGetDepositAddrSize 8 + 20
union BtcNetMsgGetDepositAddr
{
    struct
    {
        int64_t MessageType;
        char client[20];
    };
    char data[NetMsgGetDepositAddrSize];

    BtcNetMsgGetDepositAddr();
};

#define NetMsgDepositAddrSize 8 + 72
union BtcNetMsgDepositAddr
{
    struct
    {
        int64_t MessageType;
        char address[72];
    };
    char data[NetMsgDepositAddrSize];

    BtcNetMsgDepositAddr();
};

#define NetMsgGetKeySize 8 + 20
union BtcNetMsgGetKey
{
    struct
    {
        int64_t MessageType;
        char client[20];
    };
    char data[NetMsgGetKeySize];

    BtcNetMsgGetKey();
};

#define NetMsgPubKeySize 8 + 200 + 8
union BtcNetMsgPubKey
{
    struct
    {
        int64_t MessageType;
        char pubKey[200];    // public keys are usually 66 characters
        int64_t minSignatures;
    };
    char data[NetMsgPubKeySize];

    BtcNetMsgPubKey();
};

#define NetMsgGetBalanceSize 8 + 20
union BtcNetMsgGetBalance
{
    struct
    {
        int64_t MessageType;
        char client[20];
    };
    char data[NetMsgGetBalanceSize];

    BtcNetMsgGetBalance();
};

#define NetMsgBalanceSize 8 + 8
union BtcNetMsgBalance
{
    struct
    {
        int64_t MessageType;
        int64_t balance;
    };
    char data[NetMsgBalanceSize];

    BtcNetMsgBalance();
};

#define NetMsgGetTxCountSize 8 + 20
union BtcNetMsgGetTxCount
{
    struct
    {
        u_int64_t MessageType;
        char client[20];
    };
    char data[NetMsgGetTxCountSize];

    BtcNetMsgGetTxCount();
};

#define NetMsgTxCountSize 8 + 8
union BtcNetMsgTxCount
{
    struct
    {
        int64_t MessageType;
        u_int64_t txCount;
    };
    char data[NetMsgTxCountSize];

    BtcNetMsgTxCount();
};

#define NetMsgGetTxSize 8 + 8 + 20
union BtcNetMsgGetTx
{
    struct
    {
        int64_t MessageType;
        int64_t txIndex;
        char client[20];
    };
    char data[NetMsgGetTxSize];

    BtcNetMsgGetTx();
};

#define NetMsgTxSize 8 + 72 + 72 + 8 + 1 + 1
union BtcNetMsgTx
{
    struct
    {
        int64_t MessageType;
        char txId[72];
        char toAddress[72];
        int64_t amount;
        int8_t type;
        int8_t status;
    };
    char data[NetMsgTxSize];

    BtcNetMsgTx();
};

#define NetMsgReqWithdrawSize 8 + 8 + 72 + 20
union BtcNetMsgReqWithdraw
{
    struct
    {
        int64_t MessageType;
        int64_t amount;
        char toAddress[72];
        char client[20];
    };
    char data[NetMsgReqWithdrawSize];

    BtcNetMsgReqWithdraw();
};

#define NetMsgWithdrawReplySize 8 + 1
union BtcNetMsgWithdrawReply
{
    struct
    {
        int64_t MessageType;
        int8_t accepted;
    };
    char data[NetMsgWithdrawReplySize];

    BtcNetMsgWithdrawReply();
};

#define NetMsgReqSignedTxSize 8 + 20
union BtcNetMsgReqSignedTx
{
    struct
    {
        int64_t MessageType;
        char client[20];
    };
    char data[NetMsgReqSignedTxSize];

    BtcNetMsgReqSignedTx();
};

#define NetMsgSignedTxSize 8 + 10000
union BtcNetMsgSignedTx
{
    struct
    {
        int64_t MessageType;
        char rawTx[10000];
    };
    char data[NetMsgSignedTxSize];

    BtcNetMsgSignedTx();
};

extern std::map<NetMessageType, size_t> NetMessageSizes;

void InitNetMessages();

typedef _SharedPtr<BtcNetMsg>               BtcNetMsgPtr;
typedef _SharedPtr<BtcNetMsgConnect>        BtcNetMsgConnectPtr;
typedef _SharedPtr<BtcNetMsgReqDeposit>     BtcNetMsgReqDepositPtr;
typedef _SharedPtr<BtcNetMsgDepositReply>   BtcNetMsgDepositReplyPtr;
typedef _SharedPtr<BtcNetMsgGetDepositAddr> BtcNetMsgGetDepositAddrPtr;
typedef _SharedPtr<BtcNetMsgDepositAddr>    BtcNetMsgDepositAddrPtr;
typedef _SharedPtr<BtcNetMsgGetKey>         BtcNetMsgGetKeyPtr;
typedef _SharedPtr<BtcNetMsgPubKey>         BtcNetMsgPubKeyPtr;
typedef _SharedPtr<BtcNetMsgGetBalance>     BtcNetMsgGetBalancePtr;
typedef _SharedPtr<BtcNetMsgBalance>        BtcNetMsgBalancePtr;
typedef _SharedPtr<BtcNetMsgGetTxCount>     BtcNetMsgGetTxCountPtr;
typedef _SharedPtr<BtcNetMsgTxCount>        BtcNetMsgTxCountPtr;
typedef _SharedPtr<BtcNetMsgGetTx>          BtcNetMsgGetTxPtr;
typedef _SharedPtr<BtcNetMsgTx>             BtcNetMsgTxPtr;
typedef _SharedPtr<BtcNetMsgReqWithdraw>    BtcNetMsgReqWithdrawPtr;
typedef _SharedPtr<BtcNetMsgWithdrawReply>  BtcNetMsgWithdrawReplyPtr;
typedef _SharedPtr<BtcNetMsgReqSignedTx>    BtcNetMsgReqSignedTxPtr;
typedef _SharedPtr<BtcNetMsgSignedTx>       BtcNetMsgSignedTxPtr;


#endif // SAMPLENETMESSAGES_H
