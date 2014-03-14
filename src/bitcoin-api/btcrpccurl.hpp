#ifndef BTCRPCZMQ_HPP
#define BTCRPCZMQ_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/MemoryWrapper.hpp>

#include <bitcoin-api/ibtcrpc.hpp>
#include <bitcoin-api/btcmodules.hpp>

#include <curl/curl.h>



/*
 *
 * * MTBitcoin provides a clean interface for all important bitcoin functions
 *   it is mostly a wrapper for BtcHelper
 *
 * * BtcHelper automates abstract tasks like verifying multisig transactions
 *   by calling BtcJson functions and processing the results
 *
 * * BtcJson provides Bitcoin API functions
 *   it creates Json strings, sends them to bitcoin's http interface via BtcRpcCurl and parses the results
 *   it returns strings, int64 or smart pointers to structures if Json objects are returned
 *
 * * BtcRpcCurl uses libcurl to connect to bitcoin's http interface
 *   it only has a few functions so should be easy to replace with a class using zmq
 *
 * * BtcModules is a class holding pointers to the instances of the other classes
 *   it isn't really good for anything but makes things a bit easier imho
 *
 * * btcobjects.h is a collection of various structs
 *   most of them imitate the objects returned in bitcoind's replies
 *
*/


class BtcRpcCurl : public IBtcRpc
{
public:
    BtcRpcCurl(BtcModules* modules);
    ~BtcRpcCurl();

    // Returns whether we're connected to bitcoin's http interface
    // Not implemented properly! And probably not necessary. Will have to check to be sure.
    virtual bool IsConnected();

    // Connects to bitcoin, sets some http header information and sends a test query (getinfo)
    // The whole connected/disconnected implementation is poorly done (my fault) but it works.
    // This function can be called again and again and nothing will crash if it fails.
    virtual bool ConnectToBitcoin(const std::string &user, const std::string &password, const std::string &url = "http://127.0.0.1", int port = 8332);

    // Overload to make code that switches between bitcoin servers more readable
    virtual bool ConnectToBitcoin(BitcoinServerPtr server);

    // Sends a string over the network
    // This string should be a json-rpc call if we're talking to bitcoin,
    // but we could send anything and expand this class to also connect to other http(s) interfaces.
    virtual BtcRpcPacketPtr SendRpc(const std::string &jsonString);

    // Sends a byte array over the network
    // Should be json-rpc if talking to bitcoin
    virtual BtcRpcPacketPtr SendRpc(const char* jsonString);

    // sends an array of a certain size over the network
    // returns reply
    virtual BtcRpcPacketPtr SendRpc(BtcRpcPacketPtr jsonString);

private:
    void InitSession();         // Called in constructor, makes sure we have a network interface or something
    void InitNAM();             // Also called in constr, creates the Network Access Manager.
    void SetHeaderInformation(); // Called by ConnectToBitcoin, sets the HTTP header

    void CleanUpCurl();

    // Processes a network reply
    // Probably not needed anymore
    //void ProcessReply(QSharedPointer<QByteArray> replyContType, const QSharedPointer<QByteArray> replyContent);

    // Processes an error message
    // These can come from a failed network connection, erroneus json-rpc calls or meta-errors like lack of funds.
    //void ProcessErrorMessage(const QNetworkReply *reply);

    //QPointer<QNetworkSession> session;

    //QScopedPointer<QNetworkAccessManager> rpcNAM;      // this is used to send http packets to bitcoin-qt
    //QScopedPointer<QNetworkRequest> rpcRequest;
    //QSharedPointer<QByteArray> rpcReplyContent;

    BitcoinServerPtr currentServer;
    CURLcode res;
    CURL* curl;

    BtcModules* modules;

    static BtcRpcPacketPtr connectString;


    /*
    Q_OBJECT
public slots:

    // Called when there's a network reply from bitcoin
    void OnNetReplySlot(QNetworkReply* reply);
    void OnNetReplySlot();

    // Called if bitcoin requires authentication (user/pw)
    void OnAuthReqSlot(QNetworkReply* reply, QAuthenticator* authenticator);
    */
};

#ifndef OT_USE_TR1
    typedef std::shared_ptr<BtcRpcCurl> BtcRpcCurlPtr;
#else
    typedef std::tr1::shared_ptr<BtcRpcCurl> BtcRpcCurlPtr;
#endif // OT_USE_TR1


#endif // BTCRPCZMQ_HPP
