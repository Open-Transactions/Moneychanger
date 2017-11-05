#include "sampleescrowclientzmq.hpp"

#include <zmq.hpp>

#include <opentxs/OTLog.hpp>

SampleEscrowClientZmq::SampleEscrowClientZmq()
{
    // try ports
}

SampleEscrowClientZmq::SampleEscrowClientZmq(int port)
    :SampleEscrowClient()
{
    this->serverInfo = BitcoinServerPtr(new BitcoinServer(this->clientName, "", "127.0.0.1", port));
}

void SampleEscrowClientZmq::SendData(BtcNetMsg *message)
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

void SampleEscrowClientZmq::Startserver()
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
            break;
        case (int)MultiSigKey:
            BtcNetMsgPubKey message;
            memcpy(&message, reply.data(), NetMessageSizes[MultiSigKey]);

            break;
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
