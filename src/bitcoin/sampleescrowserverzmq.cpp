#include "sampleescrowserverzmq.hpp"
#include "sampleescrowclientzmq.hpp"

#include <core/modules.hpp>

#include <opentxs/OTLog.hpp>

#include <zmq.hpp>

#include <string>

SampleEscrowServerZmq::SampleEscrowServerZmq(BitcoinServerPtr bitcoind, EscrowPoolPtr pool, int port)
    :SampleEscrowServer(bitcoind, pool)
{
    this->serverInfo = BitcoinServerPtr(new BitcoinServer(this->serverName, "", "127.0.0.1", port));

    InitNetMessages();
}

SampleEscrowServerZmq::SampleEscrowServerZmq(BitcoinServerPtr bitcoind, EscrowPoolPtr pool, const std::string &url, int port)
    :SampleEscrowServer(bitcoind, pool)
{
    this->serverInfo = BitcoinServerPtr(new BitcoinServer(this->serverName, "", url, port));

    InitNetMessages();
}

void SampleEscrowServerZmq::ClientConnected(SampleEscrowClient *client)
{
    BtcNetMsgConnectPtr clientMsg = BtcNetMsgConnectPtr(new BtcNetMsgConnect());
    memccpy(clientMsg->client, client->clientName.c_str(), 0, 20);
    SendData((BtcNetMsg*)clientMsg.get());
}

void SampleEscrowServerZmq::ClientConnected(BtcNetMsgConnectPtr clientMsg)
{
    SampleEscrowClient* client = new SampleEscrowClientZmq();
    client->clientName = clientMsg->client;
    SampleEscrowServer::ClientConnected(client);
}

bool SampleEscrowServerZmq::RequestEscrowDeposit(const std::string &client, const int64_t &amount)
{
    BtcNetMsgReqDepositPtr reqDeposit = BtcNetMsgReqDepositPtr(new BtcNetMsgReqDeposit());
    memccpy(reqDeposit->client, client.c_str(), 0, 20);
    reqDeposit->amount = amount;
    SendData((BtcNetMsg*)reqDeposit.get());
}

bool SampleEscrowServerZmq::RequestEscrowDeposit(BtcNetMsgReqDepositPtr message)
{
    return SampleEscrowServer::RequestEscrowDeposit(message->client, message->amount);
}

std::string SampleEscrowServerZmq::RequestDepositAddress(const std::string &client)
{
    BtcNetMsgGetDepositAddrPtr message = BtcNetMsgGetDepositAddrPtr(new BtcNetMsgGetDepositAddr());
    memccpy(message->client, client.c_str(), 0, 20);

    SendData((BtcNetMsg*)message.get());
}

std::string SampleEscrowServerZmq::RequestDepositAddress(BtcNetMsgGetDepositAddrPtr message)
{
    return SampleEscrowServer::RequestDepositAddress(message->client);
}

std::string SampleEscrowServerZmq::GetPubKey(const std::string &client)
{
    BtcNetMsgGetKeyPtr msg = BtcNetMsgGetKeyPtr(new BtcNetMsgGetKey());
    memccpy(msg->client, client.c_str(), 0, 20);

    SendData((BtcNetMsg*)msg.get());
}

std::string SampleEscrowServerZmq::GetPubKey(BtcNetMsgGetKeyPtr message)
{
    return SampleEscrowServer::GetPubKey(message->client);
}

int64_t SampleEscrowServerZmq::GetClientBalance(const std::string &client)
{
    BtcNetMsgGetBalancePtr message = BtcNetMsgGetBalancePtr(new BtcNetMsgGetBalance());
    memccpy(message->client, client.c_str(), 0, 20);

    SendData((BtcNetMsg*)message.get());
}

int64_t SampleEscrowServerZmq::GetClientBalance(BtcNetMsgGetBalancePtr message)
{
    return SampleEscrowServer::GetClientBalance(message->client);
}

int32_t SampleEscrowServerZmq::GetClientTransactionCount(const std::string &client)
{
    BtcNetMsgGetTxCountPtr message = BtcNetMsgGetTxCountPtr(new BtcNetMsgGetTxCount());
    memccpy(message->client, client.c_str(), 0, 20);

    SendData((BtcNetMsg*)message.get());
}

int32_t SampleEscrowServerZmq::GetClientTransactionCount(BtcNetMsgGetTxCountPtr message)
{
    return SampleEscrowServer::GetClientTransactionCount(message->client);
}

SampleEscrowTransactionPtr SampleEscrowServerZmq::GetClientTransaction(const std::string &client, const uint32_t txIndex)
{
    BtcNetMsgGetTxPtr message = BtcNetMsgGetTxPtr(new BtcNetMsgGetTx());
    memccpy(message->client, client.c_str(), 0, 20);
    message->txIndex = txIndex;

    SendData((BtcNetMsg*)message.get());
}

SampleEscrowTransactionPtr SampleEscrowServerZmq::GetClientTransaction(BtcNetMsgGetTxPtr message)
{
    return SampleEscrowServer::GetClientTransaction(message->client, message->txIndex);
}

bool SampleEscrowServerZmq::RequestEscrowWithdrawal(const std::string &client, const int64_t &amount, const std::string &toAddress)
{
    BtcNetMsgReqWithdrawPtr release = BtcNetMsgReqWithdrawPtr(new BtcNetMsgReqWithdraw());
    memccpy(release->client, client.c_str(), 0, 20);
    memccpy(release->toAddress, toAddress.c_str(), 0, std::min(toAddress.size(), sizeof(release->toAddress)));
    release->amount = amount;

    SendData((BtcNetMsg*)release.get());
}

bool SampleEscrowServerZmq::RequestEscrowWithdrawal(BtcNetMsgReqWithdrawPtr message)
{
    return SampleEscrowServer::RequestEscrowWithdrawal(message->client, message->amount, message->toAddress);
}

std::string SampleEscrowServerZmq::RequestSignedWithdrawal(const std::string &client)
{
    BtcNetMsgReqSignedTxPtr message = BtcNetMsgReqSignedTxPtr(new BtcNetMsgReqSignedTx());
    memccpy(message->client, client.c_str(), 0, 20);

    SendData((BtcNetMsg*)message.get());
}

std::string SampleEscrowServerZmq::RequestSignedWithdrawal(BtcNetMsgReqSignedTxPtr message)
{
    return SampleEscrowServer::RequestSignedWithdrawal(message->client);
}

void SampleEscrowServerZmq::SendData(BtcNetMsg* message)
{
    // Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REQ);

    OTLog::Output(0, "Connecting to hello world server…\n");
    std::string connectString = "tcp://";
    connectString += this->serverInfo->url + ":";
    connectString += btc::to_string(this->serverInfo->port);
    socket.connect(connectString.c_str());

    // Do 10 requests, waiting each time for a response
    //for (int request_nbr = 0; request_nbr != 10; request_nbr++)
    //{
        size_t size = NetMessageSizes[static_cast<NetMessageType>(message->MessageType)];
        zmq::message_t request (size);
        memcpy ((void *) request.data (), message->data, size);
        OTLog::Output(0, "Sending data…\n");
        int timeOut = 3000;
        zmq_setsockopt(&socket, ZMQ_RCVTIMEO, &timeOut, sizeof(timeOut));
        zmq_setsockopt(&socket, ZMQ_SNDTIMEO, &timeOut, sizeof(timeOut));
        socket.send (request);

        // Get the reply.
        zmq::message_t reply;
        socket.recv (&reply);
        OTLog::Output(0, "Received reply\n");
    //}
    return;
}

void SampleEscrowServerZmq::StartServer()
{
    // Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);
    std::string connectString = "tcp://";
    connectString += this->serverInfo->url + ":";
    connectString += btc::to_string(this->serverInfo->port);
    socket.bind(connectString.c_str());

    while (!Modules::shutDown)
    {
        zmq::message_t request;

        // Wait for next request from client
        socket.recv (&request);
        OTLog::Output(0, "Received data\n");

        if(request.size() < NetMessageSizes[Unknown])
            continue;

        NetMessageType messageType = static_cast<NetMessageType>(static_cast<BtcNetMsg*>(request.data())->MessageType);

        if(request.size() < NetMessageSizes[messageType])
            continue;

        BtcNetMsgPtr replyPtr = BtcNetMsgPtr(new BtcNetMsg());

        switch(messageType)
        {
        case Unknown:
        {
            break;
        }
        case Connect:
        {
            BtcNetMsgConnectPtr message = BtcNetMsgConnectPtr(new BtcNetMsgConnect());
            memcpy(message->data, request.data(), NetMessageSizes[messageType]);
            ClientConnected(message);
            break;
        }
        case ReqDeposit:
        {
            BtcNetMsgReqDepositPtr message = BtcNetMsgReqDepositPtr(new BtcNetMsgReqDeposit());
            memcpy(message->data, request.data(), NetMessageSizes[messageType]);
            bool accepted = RequestEscrowDeposit(message);

            BtcNetMsgDepositReply* replyMsg = new BtcNetMsgDepositReply();
            replyMsg->accepted = static_cast<int8_t>(accepted);
            replyPtr.reset((BtcNetMsg*)replyMsg);
            break;
        }
        case GetMultiSigAddr:
        {
            BtcNetMsgGetDepositAddrPtr message = BtcNetMsgGetDepositAddrPtr(new BtcNetMsgGetDepositAddr());
            memcpy(message->data, request.data(), NetMessageSizes[messageType]);
            std::string multiSigAddr = RequestDepositAddress(message);

            BtcNetMsgDepositAddr* replyMsg = new BtcNetMsgDepositAddr();
            memccpy(replyMsg->address, multiSigAddr.c_str(), 0, std::min(multiSigAddr.size(), sizeof(replyMsg->address)));
            replyPtr.reset((BtcNetMsg*)replyMsg);
            break;
        }
        case GetMultiSigKey:
        {
            BtcNetMsgGetKeyPtr message = BtcNetMsgGetKeyPtr(new BtcNetMsgGetKey());
            memcpy(message->data, request.data(), NetMessageSizes[messageType]);
            std::string pubKey = GetPubKey(message);

            BtcNetMsgPubKey* msgPubKey = new BtcNetMsgPubKey();
            memccpy(msgPubKey->pubKey, pubKey.c_str(), 0, std::min(pubKey.size(), sizeof(msgPubKey->pubKey)));
            replyPtr.reset((BtcNetMsg*)msgPubKey);
            break;
        }
        case GetBalance:
        {
            BtcNetMsgGetBalancePtr message = BtcNetMsgGetBalancePtr(new BtcNetMsgGetBalance());
            memcpy(message->data, request.data(), NetMessageSizes[messageType]);
            int64_t balance = GetClientBalance(message);

            BtcNetMsgBalance* replyMsg = new BtcNetMsgBalance();
            replyMsg->balance = balance;
            replyPtr.reset((BtcNetMsg*)replyMsg);
            break;
        }
        case GetTxCount:
        {
            BtcNetMsgGetTxCountPtr message = BtcNetMsgGetTxCountPtr(new BtcNetMsgGetTxCount());
            memcpy(message->data, request.data(), NetMessageSizes[messageType]);
            int32_t txCount = GetClientTransactionCount(message);

            BtcNetMsgTxCount* replyMsg = new BtcNetMsgTxCount();
            replyMsg->txCount = txCount;
            replyPtr.reset((BtcNetMsg*)replyMsg);
            break;
        }
        case GetTx:
        {
            BtcNetMsgGetTxPtr message = BtcNetMsgGetTxPtr(new BtcNetMsgGetTx());
            memcpy(message->data, request.data(), NetMessageSizes[messageType]);
            SampleEscrowTransactionPtr tx = GetClientTransaction(message);

            BtcNetMsgTx* replyMsg = new BtcNetMsgTx();
            memccpy(replyMsg->txId, tx->txId.c_str(), 0, std::min(tx->txId.size(), sizeof(replyMsg->txId)));
            memccpy(replyMsg->toAddress, tx->targetAddr.c_str(), 0, std::min(tx->targetAddr.size(), sizeof(replyMsg->toAddress)));
            replyMsg->amount = tx->amountToSend;
            replyMsg->type = static_cast<int8_t>(tx->type);
            replyMsg->status = static_cast<int8_t>(tx->status);
            replyPtr.reset((BtcNetMsg*)replyMsg);
            break;
        }
        case RequestRelease:
        {
            BtcNetMsgReqWithdrawPtr message = BtcNetMsgReqWithdrawPtr(new BtcNetMsgReqWithdraw());
            memcpy(message->data, request.data(), NetMessageSizes[messageType]);
            bool accepted = RequestEscrowWithdrawal(message);

            BtcNetMsgWithdrawReply* replyMsg = new BtcNetMsgWithdrawReply();
            replyMsg->accepted = static_cast<int8_t>(accepted);
            replyPtr.reset((BtcNetMsg*)replyMsg);
            break;
        }
        case ReqSignedTx:
        {
            BtcNetMsgReqSignedTxPtr message = BtcNetMsgReqSignedTxPtr(new BtcNetMsgReqSignedTx());
            memcpy(message->data, request.data(), NetMessageSizes[messageType]);
            std::string partiallySignedTx = RequestSignedWithdrawal(message);

            BtcNetMsgSignedTx* replyMsg = new BtcNetMsgSignedTx();
            memccpy(replyMsg->rawTx, partiallySignedTx.c_str(), 0, std::min(partiallySignedTx.size(), sizeof(replyMsg->rawTx)));
            replyPtr.reset((BtcNetMsg*)replyMsg);
            break;
        }
        }

        // Send reply back to client
        int size = NetMessageSizes[(NetMessageType)replyPtr->MessageType];
        zmq::message_t reply (size);

        memcpy ((void *) reply.data (), replyPtr->data, size);
        socket.send (reply);
    }

   return;
}
