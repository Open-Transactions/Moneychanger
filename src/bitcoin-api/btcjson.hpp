#ifndef BTCJSON_HPP
#define BTCJSON_HPP

#include <opentxs/MemoryWrapper.hpp>

#include <bitcoin-api/ibtcjson.hpp>
#include <bitcoin-api/btcmodules.hpp>


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


class BtcJson : public IBtcJson
{
public:
    BtcJson(BtcModules* modules);
    ~BtcJson();

    virtual void Initialize();       // should make this part of all modules

    virtual void GetInfo();

    virtual int64_t GetBalance(const char *account = NULL /*TODO: int minConfirmations*/);

    // Gets the default address for the specified account
    virtual std::string GetAccountAddress(const std::string &account = "");

    // Returns list of all addresses belonging to account
    // account "" is default
    virtual std::list<std::string> GetAddressesByAccount(const std::string &account = "");

    // Add new address to account
    virtual std::string GetNewAddress(const std::string &account = "");

    // Validate an address
    virtual BtcAddressInfoPtr ValidateAddress(const std::string &address);

    virtual std::string GetPublicKey(const std::string& address);

    // Get private key for address (calls DumpPrivKey())
    virtual std::string GetPrivateKey(const std::string &address);

    // Get private key for address
    virtual std::string DumpPrivKey(const std::string &address);

    // Adds an address requiring n keys to sign before it can spend its inputs
    // nRequired: number of signatures required
    // keys: list of public keys (addresses work too, if the public key is known)
    // account [optional]: account to add the address to
    // Returns the multi-sig address
    virtual BtcMultiSigAddressPtr AddMultiSigAddress(int nRequired, const std::list<std::string> &keys, const std::string &account = "");

    // Creates a multi-sig address without adding it to the wallet
    // nRequired: signatures required
    // keys: list of public keys (addresses work too, if the public key is known)
    virtual BtcMultiSigAddressPtr CreateMultiSigAddress(int nRequired, const std::list<std::string> &keys);

    // Creates a multi-sig address and returns its redeemScript
    // the address will not be added to your address list, use AddMultiSigAddress for that
    virtual std::string GetRedeemScript(int nRequired, std::list<std::string> keys);

    // Returns list of account names
    // Could also return the balance of each account
    virtual std::vector<std::string> ListAccounts();

    virtual BtcUnspentOutputs ListUnspent(int minConf = 1, int maxConf = 9999999, std::vector<std::string> addresses = std::vector<std::string>());

    virtual std::string SendToAddress(const std::string &btcAddress, int64_t amount);

    // Send to multiple addresses at once
    // txTargets maps amounts (int64 satoshis) to addresses (QString)
    virtual std::string SendMany(BtcTxTarget txTargets, const std::string &fromAccount = "");

    virtual bool SetTxFee(int64_t fee);

    virtual BtcTransactionPtr GetTransaction(std::string txID);

    virtual std::string GetRawTransaction(std::string txID);

    virtual BtcRawTransactionPtr GetDecodedRawTransaction(std::string txID);

    virtual BtcRawTransactionPtr DecodeRawTransaction(std::string rawTransaction);

    virtual std::string CreateRawTransaction(BtcTxIdVouts unspentOutputs, BtcTxTarget txTargets);

    virtual BtcSignedTransactionPtr SignRawTransaction(const std::string &rawTransaction, const std::list<BtcSigningPrequisite> &previousTransactions = std::list<BtcSigningPrequisite>(), const std::stringList &privateKeys = std::list<std::string>());

    virtual BtcSignedTransactionPtr CombineSignedTransactions(std::string rawTransaction);

    virtual std::string SendRawTransaction(const std::string &rawTransaction);

    virtual std::vector<std::string> GetRawMemPool();

    virtual int GetBlockCount();

    virtual std::string GetBlockHash(int blockNumber);

    virtual BtcBlockPtr GetBlock(const std::string &blockHash);

private:
     virtual BtcRpcPacketPtr CreateJsonQuery(const std::string &command, Json::Value params = Json::Value(), std::string id = "");

     // Checks the reply object received from bitcoin-qt for errors and returns the reply
    virtual bool ProcessRpcString(BtcRpcPacketPtr jsonString, Json::Value &result);
    // Splits the reply object received from bitcoin-qt into error and result objects
    virtual void ProcessRpcString(BtcRpcPacketPtr jsonString, std::string &id, Json::Value& error, Json::Value& result);

    BtcModules* modules;
};

#ifndef OT_USE_TR1
    typedef std::shared_ptr<BtcJson> BtcJsonPtr;
#else
    typedef std::tr1::shared_ptr<BtcJson> BtcJsonPtr;
#endif // OT_USE_TR1


#endif // BTCJSON_HPP
