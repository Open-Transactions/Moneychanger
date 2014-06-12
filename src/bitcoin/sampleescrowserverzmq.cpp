#include "sampleescrowserverzmq.hpp"
#include "sampleescrowclientzmq.hpp"

#include <core/modules.hpp>

#include <opentxs/OTLog.hpp>

#include <zmq.hpp>

#include <string>
#include <cstdlib>
#include <QTime>


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

    if(this->serverSocket != NULL && this->serverSocket->connected())
        this->serverSocket->close();
    if(this->context != NULL)
        this->context->close();
    delete this->serverSocket;
    delete this->context;
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

    delete rawReply;

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

    delete rawReply;

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

    delete rawReply;

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

    delete rawReply;

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

    delete rawReply;

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

    delete rawReply;

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

    delete rawReply;

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

    delete rawReply;

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

    delete rawReply;

    return reply->rawTx;
}

std::string SampleEscrowServerZmq::RequestSignedWithdrawal(BtcNetMsgReqSignedTxPtr message)
{
    return SampleEscrowServer::RequestSignedWithdrawal(message->client);
}

BtcNetMsg* SampleEscrowServerZmq::SendData(BtcNetMsg* message)
{
    // Prepare our context and socket
    zmq::context_t* context = new zmq::context_t(1);
    zmq::socket_t socket (*context, ZMQ_REQ);

    // Configure socket to not wait at close time
    int timeOut = 3000;
    socket.setsockopt(ZMQ_RCVTIMEO, &timeOut, sizeof(timeOut));
    socket.setsockopt(ZMQ_SNDTIMEO, &timeOut, sizeof(timeOut));
    int linger = 0;
    socket.setsockopt(ZMQ_LINGER, &linger, sizeof (linger));

    if(zmq_connect(socket, this->connectString.c_str()) != 0)
    {
        int error = zmq_errno();

        if(socket.connected())
            socket.close();
        zmq_ctx_destroy(context);
        delete context;
        return NULL;
    }

    size_t size = NetMessageSizes[static_cast<NetMessageType>(message->MessageType)];
    zmq::message_t* request = new zmq::message_t(size);
    memcpy ((void *) request->data (), message->data, size);

    socket.send (*request);

    zmq::message_t* reply = new zmq::message_t();

    bool incomingData = false;
    for(int i = 0; i < 3; i++)
    {
        // Wait for response from server
        zmq::pollitem_t items[] = { { socket, 0, ZMQ_POLLIN, 0 } };
        zmq::poll (&items[0], 1, 1000);

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
        if(socket.connected())
            socket.close();
        zmq_ctx_destroy(context);
        delete context;
        delete reply;
        return NULL;
    }

    socket.recv (reply);

    if(reply->size() < NetMessageSizes[Unknown])
    {
        if(socket.connected())
            socket.close();
        zmq_ctx_destroy(context);
        delete context;
        delete reply;
        return NULL;
    }

    NetMessageType messageType = static_cast<NetMessageType>(static_cast<BtcNetMsg*>(reply->data())->MessageType);

    if(messageType == Unknown || reply->size() < NetMessageSizes[messageType])
    {
        if(socket.connected())
            socket.close();
        zmq_ctx_destroy(context);
        delete context;
        delete reply;
        return NULL;
    }

    BtcNetMsg* replyMsg = NULL;

    char* data = new char[NetMessageSizes[messageType]];
    memcpy(data, reply->data(), NetMessageSizes[messageType]);

    if(socket.connected())
        socket.close();
    zmq_ctx_destroy(context);
    delete context;
    delete reply;

    return (BtcNetMsg*) data;

}

void SampleEscrowServerZmq::StartServer()
{
    std::printf("starting server %s on port %d\n", this->serverName.c_str(), this->serverInfo->port);
    std::cout.flush();

    // Prepare our context and socket
    this->context = new zmq::context_t(1);
    this->serverSocket = new zmq::socket_t(*this->context, ZMQ_REP);

    // Configure socket to not wait at close time
    int timeOut = 3000;
    zmq_setsockopt(this->serverSocket, ZMQ_RCVTIMEO, &timeOut, sizeof(timeOut));
    zmq_setsockopt(this->serverSocket, ZMQ_SNDTIMEO, &timeOut, sizeof(timeOut));
    int linger = 0;
    this->serverSocket->setsockopt (ZMQ_LINGER, &linger, sizeof (linger));

    std::string connectString = "tcp://";
    connectString += this->serverInfo->url;
    connectString += ":";
    connectString += btc::to_string(this->serverInfo->port);
    this->serverSocket->bind(connectString.c_str());
}

void SampleEscrowServerZmq::UpdateServer()
{
    //while (!Modules::shutDown)
    {
        zmq::message_t* request = new zmq::message_t();

        // Wait for next request from client
        zmq::pollitem_t items[] = { { *this->serverSocket, 0, ZMQ_POLLIN, 0 } };
        zmq::poll (&items[0], 1, 1000);

        // Return if no request
        if (!(items[0].revents & ZMQ_POLLIN))
        {
            delete request;
            return;
        }

        this->serverSocket->recv (request);

        if(request->size() < NetMessageSizes[Unknown])
        {
            delete request;
            return;
        }

        NetMessageType messageType = static_cast<NetMessageType>(static_cast<BtcNetMsg*>(request->data())->MessageType);
        if(request->size() != NetMessageSizes[messageType])
        {
            delete request;
            return;
        }

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
            memcpy(message->data, request->data(), NetMessageSizes[messageType]);
            ClientConnected(message);
            printf("client connected\n");
            std::cout.flush();
            break;
        }
        case ReqDeposit:
        {
            BtcNetMsgReqDepositPtr message = BtcNetMsgReqDepositPtr(new BtcNetMsgReqDeposit());
            memcpy(message->data, request->data(), NetMessageSizes[messageType]);
            bool accepted = RequestEscrowDeposit(message);

            BtcNetMsgDepositReply* replyMsg = new BtcNetMsgDepositReply();
            replyMsg->accepted = static_cast<int8_t>(accepted);
            replyPtr.reset((BtcNetMsg*)replyMsg);
            break;
        }
        case GetMultiSigAddr:
        {
            BtcNetMsgGetDepositAddrPtr message = BtcNetMsgGetDepositAddrPtr(new BtcNetMsgGetDepositAddr());
            memcpy(message->data, request->data(), NetMessageSizes[messageType]);
            std::string multiSigAddr = RequestDepositAddress(message);

            if(multiSigAddr.empty())
                break;

            BtcNetMsgDepositAddr* replyMsg = new BtcNetMsgDepositAddr();
            memcpy(replyMsg->address, multiSigAddr.c_str(), std::min(multiSigAddr.size(), sizeof(replyMsg->address)));
            replyPtr.reset((BtcNetMsg*)replyMsg);
            break;
        }
        case GetMultiSigKey:
        {
            BtcNetMsgGetKeyPtr message = BtcNetMsgGetKeyPtr(new BtcNetMsgGetKey());
            memcpy(message->data, request->data(), NetMessageSizes[messageType]);
            std::string pubKey = GetPubKey(message);

            if(pubKey.empty())
                break;

            BtcNetMsgPubKey* msgPubKey = new BtcNetMsgPubKey();
            memcpy(msgPubKey->pubKey, pubKey.c_str(), std::min(pubKey.size(), sizeof(msgPubKey->pubKey)));
            replyPtr.reset((BtcNetMsg*)msgPubKey);
            break;
        }
        case GetBalance:
        {
            BtcNetMsgGetBalancePtr message = BtcNetMsgGetBalancePtr(new BtcNetMsgGetBalance());
            memcpy(message->data, request->data(), NetMessageSizes[messageType]);
            int64_t balance = GetClientBalance(message);

            BtcNetMsgBalance* replyMsg = new BtcNetMsgBalance();
            replyMsg->balance = balance;
            replyPtr.reset((BtcNetMsg*)replyMsg);
            break;
        }
        case GetTxCount:
        {
            BtcNetMsgGetTxCountPtr message = BtcNetMsgGetTxCountPtr(new BtcNetMsgGetTxCount());
            memcpy(message->data, request->data(), NetMessageSizes[messageType]);
            int32_t txCount = GetClientTransactionCount(message);

            BtcNetMsgTxCount* replyMsg = new BtcNetMsgTxCount();
            replyMsg->txCount = txCount;
            replyPtr.reset((BtcNetMsg*)replyMsg);
            break;
        }
        case GetTx:
        {
            BtcNetMsgGetTxPtr message = BtcNetMsgGetTxPtr(new BtcNetMsgGetTx());
            memcpy(message->data, request->data(), NetMessageSizes[messageType]);
            SampleEscrowTransactionPtr tx = GetClientTransaction(message);

            if(tx == NULL)
                break;

            BtcNetMsgTx* replyMsg = new BtcNetMsgTx();
            memcpy(replyMsg->txId, tx->txId.c_str(), std::min(tx->txId.size(), sizeof(replyMsg->txId)));
            memcpy(replyMsg->toAddress, tx->targetAddr.c_str(), std::min(tx->targetAddr.size(), sizeof(replyMsg->toAddress)));
            replyMsg->amount = tx->amountToSend;
            replyMsg->type = static_cast<int8_t>(tx->type);
            replyMsg->status = static_cast<int8_t>(tx->status);

            replyPtr.reset((BtcNetMsg*)replyMsg);            
            break;
        }
        case RequestRelease:
        {      
            BtcNetMsgReqWithdrawPtr message = BtcNetMsgReqWithdrawPtr(new BtcNetMsgReqWithdraw());
            memcpy(message->data, request->data(), NetMessageSizes[messageType]);
            bool accepted = RequestEscrowWithdrawal(message);

            BtcNetMsgWithdrawReply* replyMsg = new BtcNetMsgWithdrawReply();
            replyMsg->accepted = static_cast<int8_t>(accepted);
            replyPtr.reset((BtcNetMsg*)replyMsg);
            break;
        }
        case ReqSignedTx:
        {
            BtcNetMsgReqSignedTxPtr message = BtcNetMsgReqSignedTxPtr(new BtcNetMsgReqSignedTx());
            memcpy(message->data, request->data(), NetMessageSizes[messageType]);
            std::string partiallySignedTx = RequestSignedWithdrawal(message);

            if(partiallySignedTx.empty())
                break;

            BtcNetMsgSignedTx* replyMsg = new BtcNetMsgSignedTx();
            memcpy(replyMsg->rawTx, partiallySignedTx.c_str(), std::min(partiallySignedTx.size(), sizeof(replyMsg->rawTx)));
            replyPtr.reset((BtcNetMsg*)replyMsg);
            break;
        }
        default:
            printf("received malformed message\n");
            std::cout.flush();
            break;
        }

        // Send reply back to client
        size_t size = NetMessageSizes[(NetMessageType)replyPtr->MessageType];
        zmq::message_t* reply = new zmq::message_t(size);

        memcpy ((void *) reply->data(), replyPtr->data, size);
        this->serverSocket->send(*reply);

        delete request;
        delete reply;
    }
}
