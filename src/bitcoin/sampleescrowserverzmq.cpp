#include "sampleescrowserverzmq.hpp"
#include "sampleescrowclientzmq.hpp"

#include <core/modules.hpp>

#include <bitcoin-api/btchelper.hpp>

#include <opentxs/core/OTLog.hpp>

//#ifdef OT_USE_ZMQ4
    #include <zmq.h>
//#else
//    #include <zmq.hpp>
//#endif

#include <string>
#include <cstdio>
#include <QTime>


void DeleteNetMsg(void* data, void* netMsg)
{
    // delete pointer after message has been sent
    delete (BtcNetMsg*)netMsg;
}

SampleEscrowServerZmq::SampleEscrowServerZmq(BitcoinServerPtr bitcoind, EscrowPoolPtr pool, int port)
    :SampleEscrowServer(bitcoind, pool)
{
    this->serverInfo = BitcoinServerPtr(new BitcoinServer(this->serverName, "", "*", port));

    this->isClient = false;

    this->master = SampleEscrowServerZmqPtr();

    InitNetMessages();

    StartServer();
}

SampleEscrowServerZmq::SampleEscrowServerZmq(BitcoinServerPtr bitcoind, EscrowPoolPtr pool, const std::string &url, int port, SampleEscrowServerZmqPtr master)
    :SampleEscrowServer(bitcoind, pool)
{
    this->serverInfo = BitcoinServerPtr(new BitcoinServer(this->serverName, "", url, port));

    this->isClient = true;
    this->serverSocket = NULL;
    this->context = NULL;

    this->master = master;

    this->connectString = "tcp://";
    this->connectString += this->serverInfo->url + ":";
    this->connectString += btc::to_string(this->serverInfo->port);

    InitNetMessages();
}

SampleEscrowServerZmq::~SampleEscrowServerZmq()
{
    this->shutDown = true;
    wait();

    zmq_close(this->serverSocket);
#ifdef OT_USE_ZMQ4
    zmq_ctx_term(this->context);
#else
    zmq_term(this->context);
#endif
}

void SampleEscrowServerZmq::Update()
{
    SampleEscrowServer::Update();
    UpdateServer();
}

bool SampleEscrowServerZmq::ClientConnected(SampleEscrowClient *client)
{
    if(!this->isClient)
        return SampleEscrowServer::ClientConnected(client);


    std::printf("Connecting client %s from serverclient %s\n", client->clientName.c_str(), this->serverName.c_str());
    std::cout.flush();

    BtcNetMsgConnectPtr clientMsg = BtcNetMsgConnectPtr(new BtcNetMsgConnect());
    memcpy(clientMsg->client, client->clientName.c_str(), 20);
    BtcNetMsg* rawReply = SendData((BtcNetMsg*)clientMsg.get());

    delete[] rawReply;

    return rawReply != NULL;    // currently the server returns the NULL packet either way
}

bool SampleEscrowServerZmq::ClientConnected(BtcNetMsgConnectPtr clientMsg)
{ 
    SampleEscrowClient* client = new SampleEscrowClient();
    client->clientName = clientMsg->client;
    SampleEscrowServer::ClientConnected(client);
}

bool SampleEscrowServerZmq::RequestEscrowDeposit(const std::string &client, const int64_t &amount)
{
    if(!this->isClient)
        return SampleEscrowServer::RequestEscrowDeposit(client, amount);

    BtcNetMsgReqDepositPtr message = BtcNetMsgReqDepositPtr(new BtcNetMsgReqDeposit());
    memcpy(message->client, client.c_str(), 20);
    message->amount = amount;

    BtcNetMsg* rawReply = SendData((BtcNetMsg*)message.get());
    if(rawReply == NULL)
        return false;

    BtcNetMsgDepositReplyPtr reply = BtcNetMsgDepositReplyPtr(new BtcNetMsgDepositReply());
    memcpy(reply->data, rawReply->data, NetMsgDepositReplySize);

    delete[] rawReply;

    return static_cast<bool>(reply->accepted);   
}

bool SampleEscrowServerZmq::RequestEscrowDeposit(BtcNetMsgReqDepositPtr message)
{
    std::printf("Client %s wants to deposit %f\n", message->client, BtcHelper::SatoshisToCoins(message->amount));
    std::cout.flush();

    return SampleEscrowServer::RequestEscrowDeposit(message->client, message->amount);
}

std::string SampleEscrowServerZmq::RequestDepositAddress(const std::string &client)
{
    if(!this->isClient)
        return  SampleEscrowServer::RequestDepositAddress(client);

    BtcNetMsgGetDepositAddrPtr message = BtcNetMsgGetDepositAddrPtr(new BtcNetMsgGetDepositAddr());
    memcpy(message->client, client.c_str(), 20);

    BtcNetMsg* rawReply = SendData((BtcNetMsg*)message.get());
    if(rawReply == NULL)
        return std::string();

    BtcNetMsgDepositAddrPtr reply = BtcNetMsgDepositAddrPtr(new BtcNetMsgDepositAddr());
    memcpy(reply->data, rawReply->data, NetMsgDepositAddrSize);

    std::printf("received deposit address %s\n", reply->address);
    std::cout.flush();

    delete[] rawReply;

    return reply->address;
}

std::string SampleEscrowServerZmq::RequestDepositAddress(BtcNetMsgGetDepositAddrPtr message)
{
    return SampleEscrowServer::RequestDepositAddress(message->client);
}

std::string SampleEscrowServerZmq::GetPubKey(const std::string &client)
{
    if(!this->isClient)
        return SampleEscrowServer::GetPubKey(client);

    BtcNetMsgGetKeyPtr msg = BtcNetMsgGetKeyPtr(new BtcNetMsgGetKey());
    memcpy(msg->client, client.c_str(), 20);

    BtcNetMsg* rawReply = SendData((BtcNetMsg*)msg.get());
    if(rawReply == NULL)
        return std::string();

    BtcNetMsgPubKeyPtr reply = BtcNetMsgPubKeyPtr(new BtcNetMsgPubKey());
    memcpy(reply->data, rawReply->data, NetMsgPubKeySize);

    delete[] rawReply;

    return reply->pubKey;
}

std::string SampleEscrowServerZmq::GetPubKey(BtcNetMsgGetKeyPtr message)
{
    return SampleEscrowServer::GetPubKey(message->client);
}

int64_t SampleEscrowServerZmq::GetClientBalance(const std::string &client)
{
    if(!this->isClient)
        return SampleEscrowServer::GetClientBalance(client);

    BtcNetMsgGetBalancePtr message = BtcNetMsgGetBalancePtr(new BtcNetMsgGetBalance());
    memcpy(message->client, client.c_str(), 20);

    BtcNetMsg* rawReply = SendData((BtcNetMsg*)message.get());
    if(rawReply == NULL)
        return int64_t(0);

    BtcNetMsgBalancePtr reply = BtcNetMsgBalancePtr(new BtcNetMsgBalance());
    memcpy(reply->data, rawReply->data, NetMsgBalanceSize);

    delete[] rawReply;

    return reply->balance;
}

int64_t SampleEscrowServerZmq::GetClientBalance(BtcNetMsgGetBalancePtr message)
{
    return SampleEscrowServer::GetClientBalance(message->client);
}

u_int64_t SampleEscrowServerZmq::GetClientTransactionCount(const std::string &client)
{
    if(!this->isClient)
        return SampleEscrowServer::GetClientTransactionCount(client);

    BtcNetMsgGetTxCountPtr message = BtcNetMsgGetTxCountPtr(new BtcNetMsgGetTxCount());
    memcpy(message->client, client.c_str(), 20);

    BtcNetMsg* rawReply = SendData((BtcNetMsg*)message.get());
    if(rawReply == NULL)
        return 0;

    BtcNetMsgTxCountPtr reply = BtcNetMsgTxCountPtr(new BtcNetMsgTxCount());
    memcpy(reply->data, rawReply->data, NetMsgTxCountSize);

    delete[] rawReply;

    return reply->txCount;
}

u_int64_t SampleEscrowServerZmq::GetClientTransactionCount(BtcNetMsgGetTxCountPtr message)
{
    return SampleEscrowServer::GetClientTransactionCount(message->client);
}

SampleEscrowTransactionPtr SampleEscrowServerZmq::GetClientTransaction(const std::string &client, const u_int64_t txIndex)
{
    if(!this->isClient)
        return SampleEscrowServer::GetClientTransaction(client, txIndex);

    BtcNetMsgGetTxPtr message = BtcNetMsgGetTxPtr(new BtcNetMsgGetTx());
    memcpy(message->client, client.c_str(), 20);
    message->txIndex = txIndex;

    BtcNetMsg* rawReply = SendData((BtcNetMsg*)message.get());
    if(rawReply == NULL)
        return SampleEscrowTransactionPtr();

    BtcNetMsgTxPtr reply = BtcNetMsgTxPtr(new BtcNetMsgTx());
    memcpy(reply->data, rawReply->data, NetMsgTxSize);

    SampleEscrowTransactionPtr tx = SampleEscrowTransactionPtr(new SampleEscrowTransaction(reply->amount, BtcModulesPtr()));
    tx->txId = reply->txId;
    tx->targetAddr = reply->toAddress;
    tx->type = (SampleEscrowTransaction::Type)reply->type;
    tx->status = (SampleEscrowTransaction::SUCCESS)reply->status;

    delete[] rawReply;

    return tx;
}

SampleEscrowTransactionPtr SampleEscrowServerZmq::GetClientTransaction(BtcNetMsgGetTxPtr message)
{
    return SampleEscrowServer::GetClientTransaction(message->client, message->txIndex);
}

bool SampleEscrowServerZmq::RequestEscrowWithdrawal(const std::string &client, const int64_t &amount, const std::string &toAddress)
{
    if(!this->isClient)
        return SampleEscrowServer::RequestEscrowWithdrawal(client, amount, toAddress);

    BtcNetMsgReqWithdrawPtr message = BtcNetMsgReqWithdrawPtr(new BtcNetMsgReqWithdraw());
    memcpy(message->client, client.c_str(), 20);
    memcpy(message->toAddress, toAddress.c_str(), std::min(toAddress.size(), sizeof(message->toAddress)));
    message->amount = amount;

    BtcNetMsg* rawReply = SendData((BtcNetMsg*)message.get());
    if(rawReply == NULL)
        return false;

    BtcNetMsgWithdrawReplyPtr reply = BtcNetMsgWithdrawReplyPtr(new BtcNetMsgWithdrawReply());
    memcpy(reply->data, rawReply->data, NetMsgWithdrawReplySize);

    delete[] rawReply;

    return static_cast<bool>(reply->accepted);
}

bool SampleEscrowServerZmq::RequestEscrowWithdrawal(BtcNetMsgReqWithdrawPtr message)
{
    return SampleEscrowServer::RequestEscrowWithdrawal(message->client, message->amount, message->toAddress);
}

std::string SampleEscrowServerZmq::RequestSignedWithdrawal(const std::string &client)
{
    if(!this->isClient)
        return SampleEscrowServer::RequestSignedWithdrawal(client);

    BtcNetMsgReqSignedTxPtr message = BtcNetMsgReqSignedTxPtr(new BtcNetMsgReqSignedTx());
    memcpy(message->client, client.c_str(), 20);

    BtcNetMsg* rawReply = SendData((BtcNetMsg*)message.get());
    if(rawReply == NULL)
        return std::string();

    BtcNetMsgSignedTxPtr reply = BtcNetMsgSignedTxPtr(new BtcNetMsgSignedTx());
    memcpy(reply->data, rawReply->data, NetMsgSignedTxSize);

    delete[] rawReply;

    return reply->rawTx;
}

std::string SampleEscrowServerZmq::RequestSignedWithdrawal(BtcNetMsgReqSignedTxPtr message)
{
    return SampleEscrowServer::RequestSignedWithdrawal(message->client);
}

BtcNetMsg* SampleEscrowServerZmq::SendData(BtcNetMsg* message)
{
    // Prepare our context and socket
    zmq_context_t* context = zmq_init(1);
    zmq_socket_t* socket = zmq_socket(context, ZMQ_REQ);

    // Configure socket to not wait at close time
    int timeOut = 3000;
#ifdef OT_USE_ZMQ4
    zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeOut, sizeof(timeOut));
    zmq_setsockopt(socket, ZMQ_SNDTIMEO, &timeOut, sizeof(timeOut));
#endif
    int linger = 0;
    zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof (linger));

    if(zmq_connect(socket, this->connectString.c_str()) != 0)
    {
        int error = zmq_errno();

        zmq_close(socket);
        zmq_term(context);
        return NULL;
    }

    size_t size = NetMessageSizes[static_cast<NetMessageType>(message->MessageType)];
    zmq_msg_t* request = new zmq_msg_t();
    zmq_msg_init_size(request, size);
    zmq_msg_init_data(request, message->data, size, NULL, NULL);

//#ifdef OT_USE_ZMQ4
    if(zmq_msg_send(request, socket, 0) == -1)
//#else
//    if(zmq_send(socket, request, 0 ) == -1)
//#endif
    {
        zmq_close(socket);
        zmq_term(context);
        return NULL;
    }

    zmq_msg_t reply;
    zmq_msg_init(&reply);

    bool incomingData = false;
    for(int i = 0; i < 3; i++)
    {
        // Wait for response from server
        zmq_pollitem_t items[] = { { socket, 0, ZMQ_POLLIN, 0 } };
        zmq_poll (&items[0], 1, 1000);

        // Cancel if there is no response
        if ((items[0].revents & ZMQ_POLLIN))
        {
            incomingData = true;
            break;
        }

        if(master != NULL)
        {
            master->UpdateServer();
        }
    }

    if(!incomingData)
    {
        zmq_close(socket);
        zmq_term(context);
        zmq_msg_close(&reply);
        return NULL;
    }

//#ifdef OT_USE_ZMQ4
    zmq_msg_recv(&reply, socket, 0);
//#else
//    zmq_recv(socket, &reply, 0);
//#endif

    if(zmq_msg_size(&reply) < NetMessageSizes[Unknown])
    {
        zmq_close(socket);
        zmq_term(context);
        zmq_msg_close(&reply);
        return NULL;
    }

    NetMessageType messageType = static_cast<NetMessageType>(static_cast<BtcNetMsg*>(zmq_msg_data(&reply))->MessageType);

    if(messageType == Unknown || zmq_msg_size(&reply) < NetMessageSizes[messageType])
    {
        zmq_close(socket);
        zmq_term(context);
        zmq_msg_close(&reply);
        return NULL;
    }

    char* data = new char[NetMessageSizes[messageType]];
    memcpy(data, zmq_msg_data(&reply), NetMessageSizes[messageType]);

    zmq_close(socket);
    zmq_term(context);
    zmq_msg_close(&reply);

    return (BtcNetMsg*) data;

}

void SampleEscrowServerZmq::StartServer()
{
    std::printf("starting server %s on port %d\n", this->serverName.c_str(), this->serverInfo->port);
    std::cout.flush();

    // Prepare our context and socket
    this->context = zmq_init(1);
    this->serverSocket = zmq_socket(this->context, ZMQ_REP);

    // Configure socket to not wait at close time
    int timeOut = 3000;
#ifdef OT_USE_ZMQ4
    zmq_setsockopt(this->serverSocket, ZMQ_RCVTIMEO, &timeOut, sizeof(timeOut));
    zmq_setsockopt(this->serverSocket, ZMQ_SNDTIMEO, &timeOut, sizeof(timeOut));
#endif
    int linger = 0;
    zmq_setsockopt(this->serverSocket, ZMQ_LINGER, &linger, sizeof (linger));

    std::string connectString = "tcp://";
    connectString += this->serverInfo->url;
    connectString += ":";
    //connectString += btc::to_string(this->serverInfo->port);
    connectString += QString::number(this->serverInfo->port).toStdString();
    zmq_bind(serverSocket, connectString.c_str());
}

void SampleEscrowServerZmq::UpdateServer()
{
    //while (!Modules::shutDown)
    {
        zmq_msg_t request; // = new zmq_msg_t();
        zmq_msg_init(&request);

        // Wait for next request from client
        zmq_pollitem_t item;
        item.socket = this->serverSocket;
        item.fd = 0;
        item.events = ZMQ_POLLIN;
        item.revents = 0;
        /*zmq_pollitem_t items[] = { item }; //{ this->serverSocket, 0, ZMQ_POLLIN, 0 } };
        zmq_poll(&items[0], 1, 1000);

        // Return if no request
        if (!(items[0].revents & ZMQ_POLLIN))
        {
            zmq_msg_close(&request);
            return;
        }*/

//#ifdef OT_USE_ZMQ4
        if(zmq_msg_recv(&request, this->serverSocket, ZMQ_DONTWAIT) == -1)
//#else
//        if(zmq_recv(this->serverSocket, &request, ZMQ_NOBLOCK) == -1)
//#endif
        {
            zmq_msg_close(&request);
            return;
        }

        if(zmq_msg_size(&request) < NetMessageSizes[Unknown])
        {
            zmq_msg_close(&request);
            return;
        }

        NetMessageType messageType = static_cast<NetMessageType>(static_cast<BtcNetMsg*>(zmq_msg_data(&request))->MessageType);
        if(zmq_msg_size(&request) != NetMessageSizes[messageType])
        {
            zmq_msg_close(&request);
            return;
        }

        BtcNetMsg* replyPtr = new BtcNetMsg();

        switch(messageType)
        {
        case Unknown:
        {
            break;
        }
        case Connect:
        {
            BtcNetMsgConnectPtr message = BtcNetMsgConnectPtr(new BtcNetMsgConnect());
            memcpy(message->data, zmq_msg_data(&request), NetMessageSizes[messageType]);
            ClientConnected(message);
            std::printf("client connected\n");
            std::cout.flush();
            break;
        }
        case ReqDeposit:
        {
            BtcNetMsgReqDepositPtr message = BtcNetMsgReqDepositPtr(new BtcNetMsgReqDeposit());
            memcpy(message->data, zmq_msg_data(&request), NetMessageSizes[messageType]);
            bool accepted = RequestEscrowDeposit(message);

            BtcNetMsgDepositReply* replyMsg = new BtcNetMsgDepositReply();
            replyMsg->accepted = static_cast<int8_t>(accepted);
            replyPtr = (BtcNetMsg*)replyMsg;
            break;
        }
        case GetMultiSigAddr:
        {
            BtcNetMsgGetDepositAddrPtr message = BtcNetMsgGetDepositAddrPtr(new BtcNetMsgGetDepositAddr());
            memcpy(message->data, zmq_msg_data(&request), NetMessageSizes[messageType]);
            std::string multiSigAddr = RequestDepositAddress(message);

            if(multiSigAddr.empty())
                break;

            BtcNetMsgDepositAddr* replyMsg = new BtcNetMsgDepositAddr();
            memcpy(replyMsg->address, multiSigAddr.c_str(), std::min(multiSigAddr.size(), sizeof(replyMsg->address)));
            std::printf("server %s sending multisig addr %s\n", this->serverName.c_str(), replyMsg->address);
            std::cout.flush();
            replyPtr = (BtcNetMsg*)replyMsg;
            break;
        }
        case GetMultiSigKey:
        {
            BtcNetMsgGetKeyPtr message = BtcNetMsgGetKeyPtr(new BtcNetMsgGetKey());
            memcpy(message->data, zmq_msg_data(&request), NetMessageSizes[messageType]);
            std::string pubKey = GetPubKey(message);

            if(pubKey.empty())
                break;

            BtcNetMsgPubKey* replyMsg = new BtcNetMsgPubKey();
            memcpy(replyMsg->pubKey, pubKey.c_str(), std::min(pubKey.size(), sizeof(replyMsg->pubKey)));
            replyPtr = (BtcNetMsg*)replyMsg;
            break;
        }
        case GetBalance:
        {
            BtcNetMsgGetBalancePtr message = BtcNetMsgGetBalancePtr(new BtcNetMsgGetBalance());
            memcpy(message->data, zmq_msg_data(&request), NetMessageSizes[messageType]);
            int64_t balance = GetClientBalance(message);

            BtcNetMsgBalance* replyMsg = new BtcNetMsgBalance();
            replyMsg->balance = balance;
            replyPtr = (BtcNetMsg*)replyMsg;
            break;
        }
        case GetTxCount:
        {
            BtcNetMsgGetTxCountPtr message = BtcNetMsgGetTxCountPtr(new BtcNetMsgGetTxCount());
            memcpy(message->data, zmq_msg_data(&request), NetMessageSizes[messageType]);
            int32_t txCount = GetClientTransactionCount(message);

            BtcNetMsgTxCount* replyMsg = new BtcNetMsgTxCount();
            replyMsg->txCount = txCount;
            replyPtr = (BtcNetMsg*)replyMsg;
            break;
        }
        case GetTx:
        {
            BtcNetMsgGetTxPtr message = BtcNetMsgGetTxPtr(new BtcNetMsgGetTx());
            memcpy(message->data, zmq_msg_data(&request), NetMessageSizes[messageType]);
            SampleEscrowTransactionPtr tx = GetClientTransaction(message);

            if(tx == NULL)
                break;

            BtcNetMsgTx* replyMsg = new BtcNetMsgTx();
            memcpy(replyMsg->txId, tx->txId.c_str(), std::min(tx->txId.size(), sizeof(replyMsg->txId)));
            memcpy(replyMsg->toAddress, tx->targetAddr.c_str(), std::min(tx->targetAddr.size(), sizeof(replyMsg->toAddress)));
            replyMsg->amount = tx->amountToSend;
            replyMsg->type = static_cast<int8_t>(tx->type);
            replyMsg->status = static_cast<int8_t>(tx->status);

            replyPtr = (BtcNetMsg*)replyMsg;
            break;
        }
        case RequestRelease:
        {      
            BtcNetMsgReqWithdrawPtr message = BtcNetMsgReqWithdrawPtr(new BtcNetMsgReqWithdraw());
            memcpy(message->data, zmq_msg_data(&request), NetMessageSizes[messageType]);
            bool accepted = RequestEscrowWithdrawal(message);

            BtcNetMsgWithdrawReply* replyMsg = new BtcNetMsgWithdrawReply();
            replyMsg->accepted = static_cast<int8_t>(accepted);
            replyPtr = (BtcNetMsg*)replyMsg;
            break;
        }
        case ReqSignedTx:
        {
            BtcNetMsgReqSignedTxPtr message = BtcNetMsgReqSignedTxPtr(new BtcNetMsgReqSignedTx());
            memcpy(message->data, zmq_msg_data(&request), NetMessageSizes[messageType]);
            std::string partiallySignedTx = RequestSignedWithdrawal(message);

            if(partiallySignedTx.empty())
                break;

            BtcNetMsgSignedTx* replyMsg = new BtcNetMsgSignedTx();
            memcpy(replyMsg->rawTx, partiallySignedTx.c_str(), std::min(partiallySignedTx.size(), sizeof(replyMsg->rawTx)));
            replyPtr = (BtcNetMsg*)replyMsg;
            break;
        }
        default:
            std::printf("received malformed message\n");
            std::cout.flush();
            break;
        }

        zmq_msg_close(&request);

        // Send reply back to client
        size_t size = NetMessageSizes[(NetMessageType)replyPtr->MessageType];
        zmq_msg_t reply;
        zmq_msg_init_size(&reply, size);
        zmq_msg_init_data(&reply, replyPtr->data, size, &DeleteNetMsg, replyPtr);

//#ifdef OT_USE_ZMQ4
        zmq_msg_send(&reply, this->serverSocket, 0);
//#else
//        zmq_send(this->serverSocket, &reply, 0);
//#endif

        // note: replyPtr is not deleted on purpose, see DeleteNetMsg()
    }
}
