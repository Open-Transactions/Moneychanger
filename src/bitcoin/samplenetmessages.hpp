#ifndef SAMPLENETMESSAGES_H
#define SAMPLENETMESSAGES_H

#include <map>


enum NetMessageType
{
    Unknown = 0,
    GetMultiSigKey,
    MultiSigKey,
    RequestDeposit,
    DepositId,
    RequestRelease
};

#define NetMsgSize 4 + 20
union BtcNetMsg
{
    struct
    {
        int MessageType;
        char sender[20];
    };
    char data[NetMsgSize];

    BtcNetMsg();    // wow, unions can contain constructors.
};

#define NetMsgGetKeySize 4 + 20
union BtcNetMsgGetKey
{
    struct
    {
        int MessageType;
        char sender[20];
    };
    char data[NetMsgGetKeySize];

    BtcNetMsgGetKey();
};

#define NetMsgPubKeySize 4 + 20 + 67 + 4
union BtcNetMsgPubKey
{
    struct
    {
        int MessageType;
        char sender[20];
        char pubKey[67];    // public keys are usually 66 characters
        int minSignatures;
    };

    BtcNetMsgPubKey();
};

extern std::map<NetMessageType, unsigned int> NetMessageSizes;

void InitNetMessages();


#endif // SAMPLENETMESSAGES_H
