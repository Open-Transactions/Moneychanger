#include "sampleescrowserverzmq.h"
#include "sampleescrowclientzmq.h"
#include <zmq.hpp>
#include <string>
#include <OTLog.h>


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

void SampleEscrowServerZmq::OnRequestEscrowDeposit(SampleEscrowClient *client)
{
    SampleEscrowServer::OnRequestEscrowDeposit(client);
}

void SampleEscrowServerZmq::OnRequestEscrowDeposit(const std::string &sender, int64_t amount)
{
    SampleEscrowClientZmqPtr client = SampleEscrowClientZmqPtr(new SampleEscrowClientZmq());
    OnRequestEscrowDeposit(client.get());
}

void SampleEscrowServerZmq::OnGetMultiSigPubKey(const std::string &sender)
{
    if(!sender.empty())
    {
        this->serverPool->serverNameMap[sender]->OnGetMultiSigPubKey("");
        return;
    }

    BtcNetMsgPubKey msg;
    std::string pubKey = GetPubKey();

    memccpy(msg.pubKey, pubKey.c_str(), 0, pubKey.size() < 66 ? pubKey.size() : 66);
    msg.pubKey[66] = 0;
    SendData((BtcNetMsg*)&msg);
}

void SampleEscrowServerZmq::OnIncomingDeposit(std::string txId)
{

}

void SampleEscrowServerZmq::SendData(BtcNetMsg *message)
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
        int size = NetMessageSizes[(NetMessageType)message->MessageType];
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

        int messageType = ((BtcNetMsg*)request.data())->MessageType;

        if(request.size() < NetMessageSizes[(NetMessageType)messageType])
            continue;

        switch(messageType)
        {
        case (int)Unknown:
            break;
        case (int)GetMultiSigKey:
            BtcNetMsgGetKey message;
            memcpy(&message, reply.data(), NetMessageSizes[GetMultiSigKey]);
            OnGetMultiSigPubKey(message.sender);
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
