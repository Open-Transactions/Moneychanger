#include "sampleescrowserverzmq.hpp"
#include "sampleescrowclientzmq.hpp"

#include <zmq.hpp>

#include <string>

#include <opentxs/OTLog.hpp>


SampleEscrowServerZmq::SampleEscrowServerZmq(BitcoinServerPtr bitcoind, int port)
    :SampleEscrowServer(bitcoind)
{
    this->serverInfo = BitcoinServerPtr(new BitcoinServer(this->serverName, "", "127.0.0.1", port));

    InitNetMessages();
}

SampleEscrowServerZmq::SampleEscrowServerZmq(BitcoinServerPtr bitcoind, const std::string &url, int port)
    :SampleEscrowServer(bitcoind)
{
    this->serverInfo = BitcoinServerPtr(new BitcoinServer(this->serverName, "", url, port));

    InitNetMessages();
}

void SampleEscrowServerZmq::ClientConnected(SampleEscrowClient *client)
{
    BtcNetMsgPtr clientMsg = BtcNetMsgPtr(new BtcNetMsg());
    memccpy(clientMsg->sender, client->clientName.c_str(), 0, 20);
    SendData(static_cast<BtcNetMsg*>(static_cast<void*>(clientMsg.get())));
}

void SampleEscrowServerZmq::ClientConnected(BtcNetMsg clientMsg)
{
    SampleEscrowClient* client = new SampleEscrowClientZmq();
    SampleEscrowServer::ClientConnected(client);
}

bool SampleEscrowServerZmq::RequestEscrowDeposit(const std::string &sender, const int64_t &amount)
{
    BtcNetMsgReqDepositPtr reqDeposit = BtcNetMsgReqDepositPtr(new BtcNetMsgReqDeposit());
    memccpy(reqDeposit->sender, sender.c_str(), 0, 20);
    reqDeposit->amount = amount;
    SendData(static_cast<BtcNetMsg*>(static_cast<void*>(reqDeposit.get())));
}

bool SampleEscrowServerZmq::RequestEscrowDeposit(BtcNetMsgReqDepositPtr deposit)
{
    return SampleEscrowServer::RequestEscrowDeposit(deposit->sender, deposit->amount);
}

std::string SampleEscrowServerZmq::CreatePubKey(const std::string &client)
{
    return SampleEscrowServer::CreatePubKey(client);
}

std::string SampleEscrowServerZmq::GetPubKey(const std::string &client)
{

    return SampleEscrowServer::GetPubKey(client);


    if(!client.empty())
    {
        return this->serverPool->serverNameMap[client]->GetPubKey(client);
    }

    BtcNetMsgPubKey msg;
    std::string pubKey = CreatePubKey(client);

    memccpy(msg.pubKey, pubKey.c_str(), 0, pubKey.size() < 66 ? pubKey.size() : 66);
    msg.pubKey[66] = 0;
    SendData((BtcNetMsg*)&msg);
}

void SampleEscrowServerZmq::AddPubKey(const std::string &client, const std::string &key)
{
    return SampleEscrowServer::AddPubKey(client, key);
}

bool SampleEscrowServerZmq::RequestEscrowWithdrawal(const std::string &client, const int64_t &amount, const std::string &toAddress)
{
    return SampleEscrowServer::RequestEscrowWithdrawal(client, amount, toAddress);
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
        int size = NetMessageSizes[static_cast<NetMessageType>(message->MessageType)];
        zmq::message_t request (size);
        memcpy ((void *) request.data (), message->data, size);
        OTLog::Output(0, "Sending data…\n");
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

    while (true)
    {
        zmq::message_t request;

        // Wait for next request from client
        socket.recv (&request);
        OTLog::Output(0, "Received data\n");

        // Send reply back to client
        int size = NetMessageSizes[Unknown];
        zmq::message_t reply (size);
        BtcNetMsg replyMsg;
        memcpy ((void *) reply.data (), replyMsg.data, 5);
        socket.send (reply);

        if(request.size() < NetMessageSizes[Unknown])
            continue;

        int messageType = static_cast<BtcNetMsg*>(request.data())->MessageType;

        if(request.size() < NetMessageSizes[(NetMessageType)messageType])
            continue;

        switch(messageType)
        {
        case (int)Unknown:
            break;
        case (int)GetMultiSigKey:
            BtcNetMsgGetKey message;
            memcpy(&message, reply.data(), NetMessageSizes[GetMultiSigKey]);
            GetPubKey(message.client);
        }

        // Do some 'work'
        /*#ifndef _WIN32
            sleep(1);
        #else
            Sleep (1);
        #endif*/
    }

   return;
}
