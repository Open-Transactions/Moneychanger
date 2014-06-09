#include "samplenetmessages.hpp"

std::map<NetMessageType, size_t> NetMessageSizes = std::map<NetMessageType, size_t>();

void InitNetMessages()
{
    NetMessageSizes[Unknown] = NetMsgSize;
    NetMessageSizes[Connect] = NetMsgConnectSize;
    NetMessageSizes[ReqDeposit] = NetMsgReqDepositSize;
    NetMessageSizes[GetMultiSigAddr] = NetMsgGetDepositAddrSize;
    NetMessageSizes[MultiSigAddr] = NetMsgDepositAddrSize;
    NetMessageSizes[GetMultiSigKey] = NetMsgGetKeySize;
    NetMessageSizes[MultiSigKey] = NetMsgPubKeySize;
    NetMessageSizes[GetBalance] = NetMsgGetBalanceSize;
    NetMessageSizes[Balance] = NetMsgBalanceSize;
    NetMessageSizes[GetTxCount] = NetMsgGetTxCountSize;
    NetMessageSizes[TxCount] = NetMsgTxCountSize;
    NetMessageSizes[GetTx] = NetMsgGetTxSize;
    NetMessageSizes[Tx] = NetMsgTxSize;
    NetMessageSizes[RequestRelease] = NetMsgReqWithdrawSize;
    NetMessageSizes[WithdrawReply] = NetMsgWithdrawReplySize;
}

BtcNetMsg::BtcNetMsg()
{
    this->MessageType = Unknown;
}

BtcNetMsgConnect::BtcNetMsgConnect()
{
    this->MessageType = Connect;
}

BtcNetMsgReqDeposit::BtcNetMsgReqDeposit()
{
    this->MessageType = ReqDeposit;
}

BtcNetMsgDepositReply::BtcNetMsgDepositReply()
{
    this->MessageType = DepositReply;
}

BtcNetMsgGetDepositAddr::BtcNetMsgGetDepositAddr()
{
    this->MessageType = GetMultiSigAddr;
}

BtcNetMsgDepositAddr::BtcNetMsgDepositAddr()
{
    this->MessageType = MultiSigAddr;
}

BtcNetMsgGetKey::BtcNetMsgGetKey()
{
    this->MessageType = GetMultiSigKey;
}

BtcNetMsgPubKey::BtcNetMsgPubKey()
{
    this->MessageType = MultiSigKey;
}

BtcNetMsgGetBalance::BtcNetMsgGetBalance()
{
    this->MessageType = GetBalance;
}

BtcNetMsgBalance::BtcNetMsgBalance()
{
    this->MessageType = Balance;
}

BtcNetMsgGetTxCount::BtcNetMsgGetTxCount()
{
    this->MessageType = GetTxCount;
}

BtcNetMsgTxCount::BtcNetMsgTxCount()
{
    this->MessageType = TxCount;
}

BtcNetMsgGetTx::BtcNetMsgGetTx()
{
    this->MessageType = GetTx;
}

BtcNetMsgTx::BtcNetMsgTx()
{
    this->MessageType = Tx;
}

BtcNetMsgReqWithdraw::BtcNetMsgReqWithdraw()
{
    this->MessageType = RequestRelease;
}

BtcNetMsgWithdrawReply::BtcNetMsgWithdrawReply()
{
    this->MessageType = WithdrawReply;
}

BtcNetMsgReqSignedTx::BtcNetMsgReqSignedTx()
{
    this->MessageType = ReqSignedTx;
}

BtcNetMsgSignedTx::BtcNetMsgSignedTx()
{
    this->MessageType = SignedTx;
}
