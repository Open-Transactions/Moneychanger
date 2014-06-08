#include "samplenetmessages.hpp"

std::map<NetMessageType, unsigned int> NetMessageSizes = std::map<NetMessageType, unsigned int>();

void InitNetMessages()
{
    NetMessageSizes[Unknown] = NetMsgSize;
    NetMessageSizes[ReqDeposit] = NetMsgReqDepositSize;
    NetMessageSizes[GetMultiSigKey] = NetMsgGetKeySize;
}

BtcNetMsg::BtcNetMsg()
{
    this->MessageType = Unknown;
}

BtcNetMsgReqDeposit::BtcNetMsgReqDeposit()
{
    this->MessageType = ReqDeposit;
}

BtcNetMsgGetKey::BtcNetMsgGetKey()
{
    this->MessageType = GetMultiSigKey;
}

BtcNetMsgPubKey::BtcNetMsgPubKey()
{
    this->MessageType = MultiSigKey;
}
