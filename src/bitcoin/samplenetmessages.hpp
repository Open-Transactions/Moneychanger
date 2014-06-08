#ifndef SAMPLENETMESSAGES_H
#define SAMPLENETMESSAGES_H

#include <map>
#include <opentxs/TR1_Wrapper.hpp>


enum NetMessageType
{
    Unknown = 0,
    ReqDeposit,
    GetMultiSigKey,
    MultiSigKey,
    DepositId,
    RequestRelease
};

#define NetMsgSize 4 + 20
union BtcNetMsg
{
    struct
    {
        int32_t MessageType;
        char sender[20];
    };
    char data[NetMsgSize];

    BtcNetMsg();    // wow, unions can contain constructors.
};

#define NetMsgReqDepositSize 4 + 20 + 8
union BtcNetMsgReqDeposit
{
    struct
    {
        int32_t MessageType;
        char sender[20];
        int64_t amount;
    };
    char data[NetMsgSize];

    BtcNetMsgReqDeposit();
};

#define NetMsgGetKeySize 4 + 20 + 20
union BtcNetMsgGetKey
{
    struct
    {
        int32_t MessageType;
        char sender[20];
        char client[20];
    };
    char data[NetMsgGetKeySize];

    BtcNetMsgGetKey();
};

#define NetMsgPubKeySize 4 + 20 + 67 + 4
union BtcNetMsgPubKey
{
    struct
    {
        int32_t MessageType;
        char sender[20];
        char pubKey[67];    // public keys are usually 66 characters
        int32_t minSignatures;
    };

    BtcNetMsgPubKey();
};

extern std::map<NetMessageType, unsigned int> NetMessageSizes;

void InitNetMessages();

typedef _SharedPtr<BtcNetMsg>           BtcNetMsgPtr;
typedef _SharedPtr<BtcNetMsgReqDeposit> BtcNetMsgReqDepositPtr;
typedef _SharedPtr<BtcNetMsgGetKey>     BtcNetMsgGetKeyPtr;


#endif // SAMPLENETMESSAGES_H
