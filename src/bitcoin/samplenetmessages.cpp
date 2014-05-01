#include "samplenetmessages.h"

std::map<NetMessageType, unsigned int> NetMessageSizes = std::map<NetMessageType, unsigned int>();

void InitNetMessages()
{
    NetMessageSizes[Unknown] = NetMsgSize;
    NetMessageSizes[GetMultiSigKey] = NetMsgGetKeySize;
}

BtcNetMsg::BtcNetMsg()
{
    this->MessageType = Unknown;
}

BtcNetMsgGetKey::BtcNetMsgGetKey()
{
    this->MessageType = GetMultiSigKey;
}

BtcNetMsgPubKey::BtcNetMsgPubKey()
{
    this->MessageType = MultiSigKey;
}
