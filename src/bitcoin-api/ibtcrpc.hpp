#ifndef IBTCRPC_HPP
#define IBTCRPC_HPP

#include <MemoryWrapper.hpp>

#include <bitcoin-api/btcobjects.hpp>

#include <string>





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


struct BitcoinServer
{
    std::string user;
    std::string password;
    std::string url;
    int port;

    BitcoinServer(const std::string account, std::string password, std::string url, int port):
        user(account), password(password), url(url), port(port)
    {}
};

#ifndef OT_USE_TR1
    typedef std::shared_ptr<BitcoinServer> BitcoinServerPtr;
#else
    typedef std::tr1::shared_ptr<BitcoinServer> BitcoinServerPtr;
#endif // OT_USE_TR1


class IBtcRpc
{
public:
    // Returns whether we're connected to bitcoin's http interface
    // Not implemented properly! And probably not necessary. Will have to check to be sure.
    virtual bool IsConnected() = 0;

    // Connects to bitcoin, sets some http header information and sends a test query (getinfo)
    // The whole connected/disconnected implementation is poorly done (my fault) but it works.
    // This function can be called again and again and nothing will crash if it fails.
    virtual bool ConnectToBitcoin(const std::string &user, const std::string &password, const std::string &url, int port) = 0;

    // Overload to make code that switches between bitcoin servers more readable
    virtual bool ConnectToBitcoin(BitcoinServerPtr server) = 0;

    // Sends a string over the network
    // This string should be a json-rpc call if we're talking to bitcoin,
    // but we could send anything and expand this class to also connect to other http(s) interfaces.
    // returns reply
    virtual BtcRpcPacketPtr SendRpc(const std::string &jsonString) = 0;

    // Sends a byte array over the network
    // Should be json-rpc if talking to bitcoin
    // returns reply
    virtual BtcRpcPacketPtr SendRpc(const char* jsonString) = 0;

    // sends an array of a certain size over the network
    // returns reply
    virtual BtcRpcPacketPtr SendRpc(BtcRpcPacketPtr jsonString) = 0;
};

#ifndef OT_USE_TR1
    typedef std::shared_ptr<IBtcRpc> IBtcRpcPtr;
#else
    typedef std::tr1::shared_ptr<IBtcRpc> IBtcRpcPtr;
#endif // OT_USE_TR1


#endif // IBTCRPC_HPP
