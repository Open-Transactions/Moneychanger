#ifndef SAMPLENETMESSAGES_H
#define SAMPLENETMESSAGES_H

#include <map>
#include <opentxs/TR1_Wrapper.hpp>


enum NetMessageType
{
    Unknown = 0,
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

#define NetMsgSize 4 + 20
union BtcNetMsg
{
    struct
    {
        int32_t MessageType;
    };
    char data[NetMsgSize];

    BtcNetMsg();    // wow, unions can contain constructors.
};

#define NetMsgConnectSize 4 + 20
union BtcNetMsgConnect
{
    struct
    {
        int32_t MessageType;
        char client[20];
    };
    char data[NetMsgConnectSize];

    BtcNetMsgConnect();
};

#define NetMsgReqDepositSize 4 + 20 + 8
union BtcNetMsgReqDeposit
{
    struct
    {
        int32_t MessageType;
        char client[20];
        int64_t amount;
    };
    char data[NetMsgSize];

    BtcNetMsgReqDeposit();
};

#define NetMsgDepositReplySize 4 + 1
union BtcNetMsgDepositReply
{
    struct
    {
        int32_t MessageType;
        int8_t accepted;
    };
    char data[NetMsgDepositReplySize];

    BtcNetMsgDepositReply();
};

#define NetMsgGetDepositAddrSize 4 + 20
union BtcNetMsgGetDepositAddr
{
    struct
    {
        int32_t MessageType;
        char client[20];
    };
    char data[NetMsgGetDepositAddrSize];

    BtcNetMsgGetDepositAddr();
};

#define NetMsgDepositAddrSize 4 + 100
union BtcNetMsgDepositAddr
{
    struct
    {
        int32_t MessageType;
        char address[100];
    };
    char data[NetMsgDepositAddrSize];

    BtcNetMsgDepositAddr();
};

#define NetMsgGetKeySize 4 + 20
union BtcNetMsgGetKey
{
    struct
    {
        int32_t MessageType;
        char client[20];
    };
    char data[NetMsgGetKeySize];

    BtcNetMsgGetKey();
};

#define NetMsgPubKeySize 4 + 200 + 4
union BtcNetMsgPubKey
{
    struct
    {
        int32_t MessageType;
        char pubKey[200];    // public keys are usually 66 characters
        int32_t minSignatures;
    };
    char data[NetMsgPubKeySize];

    BtcNetMsgPubKey();
};

#define NetMsgGetBalanceSize 4 + 20
union BtcNetMsgGetBalance
{
    struct
    {
        int32_t MessageType;
        char client[20];
    };
    char data[NetMsgGetBalanceSize];

    BtcNetMsgGetBalance();
};

#define NetMsgBalanceSize 4 + 8
union BtcNetMsgBalance
{
    struct
    {
        int32_t MessageType;
        int64_t balance;
    };
    char data[NetMsgBalanceSize];

    BtcNetMsgBalance();
};

#define NetMsgGetTxCountSize 4 + 20
union BtcNetMsgGetTxCount
{
    struct
    {
        int32_t MessageType;
        char client[20];
    };
    char data[NetMsgGetTxCountSize];

    BtcNetMsgGetTxCount();
};

#define NetMsgTxCountSize 4 + 4
union BtcNetMsgTxCount
{
    struct
    {
        int32_t MessageType;
        int32_t txCount;
    };
    char data[NetMsgTxCountSize];

    BtcNetMsgTxCount();
};

#define NetMsgGetTxSize 4 + 20 + 4
union BtcNetMsgGetTx
{
    struct
    {
        int32_t MessageType;
        char client[20];
        int32_t txIndex;
    };
    char data[NetMsgGetTxSize];

    BtcNetMsgGetTx();
};

#define NetMsgTxSize 4 + 66 + 100 + 4 + 1 + 1
union BtcNetMsgTx
{
    struct
    {
        int32_t MessageType;
        char txId[66];
        char toAddress[100];
        int64_t amount;
        int8_t type;
        int8_t status;
    };
    char data[NetMsgTxSize];

    BtcNetMsgTx();
};

#define NetMsgReqWithdrawSize 4 + 20 + 100 + 4
union BtcNetMsgReqWithdraw
{
    struct
    {
        int32_t MessageType;
        char client[20];
        char toAddress[100];
        int64_t amount;
    };
    char data[NetMsgReqWithdrawSize];

    BtcNetMsgReqWithdraw();
};

#define NetMsgWithdrawReplySize 4 + 1
union BtcNetMsgWithdrawReply
{
    struct
    {
        int32_t MessageType;
        int8_t accepted;
    };
    char data[NetMsgWithdrawReplySize];

    BtcNetMsgWithdrawReply();
};

#define NetMsgReqSignedTxSize 4 + 20
union BtcNetMsgReqSignedTx
{
    struct
    {
        int32_t MessageType;
        char client[20];
    };
    char data[NetMsgReqSignedTxSize];

    BtcNetMsgReqSignedTx();
};

#define NetMsgSignedTxSize 4 + 10000
union BtcNetMsgSignedTx
{
    struct
    {
        int32_t MessageType;
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
