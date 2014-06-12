#include "samplenetmessages.hpp"
#include <cstring>

std::map<NetMessageType, size_t> NetMessageSizes = std::map<NetMessageType, size_t>();

void InitNetMessages()
{
    NetMessageSizes[Unknown] = NetMsgSize;
    NetMessageSizes[Connect] = NetMsgConnectSize;
    NetMessageSizes[ReqDeposit] = NetMsgReqDepositSize;
    NetMessageSizes[DepositReply] = NetMsgDepositReplySize;
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
    NetMessageSizes[ReqSignedTx] = NetMsgReqSignedTxSize;
    NetMessageSizes[SignedTx] = NetMsgSignedTxSize;
}

BtcNetMsg::BtcNetMsg()
{
    memset(this->data, 0, NetMsgSize);
    this->MessageType = Unknown;
}

BtcNetMsgConnect::BtcNetMsgConnect()
{
    memset(this->data, 0, NetMsgConnectSize);
    this->MessageType = Connect;
}

BtcNetMsgReqDeposit::BtcNetMsgReqDeposit()
{
    memset(this->data, 0, NetMsgReqDepositSize);
    this->MessageType = ReqDeposit;
}

BtcNetMsgDepositReply::BtcNetMsgDepositReply()
{
    memset(this->data, 0, NetMsgDepositReplySize);
    this->MessageType = DepositReply;
}

BtcNetMsgGetDepositAddr::BtcNetMsgGetDepositAddr()
{
    memset(this->data, 0, NetMsgGetDepositAddrSize);
    this->MessageType = GetMultiSigAddr;
}

BtcNetMsgDepositAddr::BtcNetMsgDepositAddr()
{
    memset(this->data, 0, NetMsgDepositAddrSize);
    this->MessageType = MultiSigAddr;
}

BtcNetMsgGetKey::BtcNetMsgGetKey()
{
    memset(this->data, 0, NetMsgGetKeySize);
    this->MessageType = GetMultiSigKey;
}

BtcNetMsgPubKey::BtcNetMsgPubKey()
{
    memset(this->data, 0, NetMsgPubKeySize);
    this->MessageType = MultiSigKey;
}

BtcNetMsgGetBalance::BtcNetMsgGetBalance()
{
    memset(this->data, 0, NetMsgGetBalanceSize);
    this->MessageType = GetBalance;
}

BtcNetMsgBalance::BtcNetMsgBalance()
{
    memset(this->data, 0, NetMsgBalanceSize);
    this->MessageType = Balance;
}

BtcNetMsgGetTxCount::BtcNetMsgGetTxCount()
{
    memset(this->data, 0, NetMsgGetTxCountSize);
    this->MessageType = GetTxCount;
}

BtcNetMsgTxCount::BtcNetMsgTxCount()
{
    memset(this->data, 0, NetMsgTxCountSize);
    this->MessageType = TxCount;
}

BtcNetMsgGetTx::BtcNetMsgGetTx()
{
    memset(this->data, 0, NetMsgGetTxSize);
    this->MessageType = GetTx;
}

BtcNetMsgTx::BtcNetMsgTx()
{
    memset(this->data, 0, NetMsgTxSize);
    this->MessageType = Tx;
}

BtcNetMsgReqWithdraw::BtcNetMsgReqWithdraw()
{
    memset(this->data, 0, NetMsgReqWithdrawSize);
    this->MessageType = RequestRelease;
}

BtcNetMsgWithdrawReply::BtcNetMsgWithdrawReply()
{
    memset(this->data, 0, NetMsgWithdrawReplySize);
    this->MessageType = WithdrawReply;
}

BtcNetMsgReqSignedTx::BtcNetMsgReqSignedTx()
{
    memset(this->data, 0, NetMsgReqSignedTxSize);
    this->MessageType = ReqSignedTx;
}

BtcNetMsgSignedTx::BtcNetMsgSignedTx()
{
    memset(this->data, 0, NetMsgSignedTxSize);
    this->MessageType = SignedTx;
}
