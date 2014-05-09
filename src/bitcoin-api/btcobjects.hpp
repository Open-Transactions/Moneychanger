#ifndef BTCOBJECTS_HPP
#define BTCOBJECTS_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/TR1_Wrapper.hpp>

#include <json/json.h>

#include _CINTTYPES
#include _MEMORY

#include <list>
#include <vector>

#include <cstddef>



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


namespace std
{   // might make things easier to read. also maybe i should use vectors instead.
    typedef list<std::string> stringList;
}



// TODO: error checking


// if a query returns a complex object we should convert it to a struct instead of
// passing around the Json::Value

// This struct holds various information about a bitcoin transaction.
// Its constructor accepts the Json::Value returned by "gettransaction".
// Does not take into account block rewards.
struct BtcTransaction
{
    int64_t Confirmations;
    // all amounts are satoshis
    int64_t AmountReceived;  // amount received
    int64_t AmountSent;      // amount sent
    int64_t Amount;          // +received -sent
    int64_t Fee;             // only displayed when sender
    std::string TxID;
    time_t Time;
    //std::string Account;
    std::list<std::string> AddressesRecv;   // received to addresses
    std::list<std::string> AddressesSent;   // sent to addresses
    //std::string Category;           // "send", "receive", "immature" (unconfirmed block reward), ...?
    std::vector<std::string> walletConflicts;
    std::string rawTransaction;
    bool involvesWatchonly;

    BtcTransaction(Json::Value reply);

private:
    void SetDefaults();
};

struct BtcRawTransaction
{
    std::string txID;

    struct VIN
    {
        std::string txInID;
        int64_t vout;   // number of txInID's output to be used as input

        VIN(const std::string &txInID, const int64_t &vout)
            :txInID(txInID), vout(vout)
        {}
    };
    std::vector<VIN> inputs;

    struct VOUT
    {
        int64_t value;      // amount of satoshis to be sent
        int64_t n ;             // outputs array index
        uint32_t reqSigs;        // signatures required to spend the output I think?
        std::vector<std::string> addresses; // an array of addresses receiving the value.
        std::string scriptPubKeyHex;        // needed to spend offline transactions

        VOUT()
        {
            this->value = 0;
            this->n = -1;
            this->reqSigs = 0;
            this->addresses = std::vector<std::string>();
            this->scriptPubKeyHex = "";
        }

        VOUT(const int64_t &value, const int64_t &n, const uint32_t &reqSigs, const std::vector<std::string> &addresses, const std::string &scriptPubKeyHex)
            :value(value), n(n), reqSigs(reqSigs), addresses(addresses), scriptPubKeyHex(scriptPubKeyHex)
        {}

    };
    std::vector<VOUT> outputs;

    BtcRawTransaction(Json::Value rawTx);
};

// returned by listunspent
struct BtcUnspentOutput
{
    std::string txId;
    int64_t vout;
    std::string address;
    std::string account;
    std::string scriptPubKey;
    int64_t amount;
    uint32_t confirmations;
    bool spendable;

    BtcUnspentOutput(Json::Value unspentOutput);
};

struct BtcAddressInfo
{
    std::string address;
    std::string pubkey;
    std::string account;
    bool ismine;
    bool isvalid;
    bool isScript;
    Json::Value addresses;       // shows addresses which a multi-sig is composed of
    uint32_t sigsRequired;

    BtcAddressInfo(Json::Value result);
};

struct BtcAddressAmount
{
    std::string Address;
    double Amount;              // amount in bitcoins
};

struct BtcMultiSigAddress
{
    std::string address;
    std::string redeemScript;
    std::stringList publicKeys;     // this will make everything so much more convenient

    BtcMultiSigAddress(Json::Value result, const std::stringList& publicKeys);
};

struct BtcBlock
{
    int64_t confirmations;
    std::list<std::string> transactions;
    int64_t height;
    std::string hash;
    std::string previousHash;

    BtcBlock();

    BtcBlock(Json::Value block);
};

// a json object containing txid and vout
// used in CreateRawTransaction
struct BtcTxIdVout : Json::Value
{
    BtcTxIdVout(const std::string &txID, const int64_t &vout);
};

// a json object mapping amounts to addresses
// used in raw and sendmany transactions
struct BtcTxTarget : Json::Value
{
    BtcTxTarget();
    BtcTxTarget(const std::string &toAddress, int64_t amount);
    void ConvertSatoshisToBitcoin();
};

struct BtcSignedTransaction
{
    std::string signedTransaction;
    bool complete;              // true if all (enough?) signatures were collected

    BtcSignedTransaction(Json::Value signedTransactionObj);
};

// used to sign some raw transactions
struct BtcSigningPrerequisite : Json::Value
{
    // an array of BtcSigningPrerequisites can be passed to bitcoin- as an optional argument to signrawtransaction
    // [{"txid":txid,"vout":n,"scriptPubKey":hex,"redeemScript":hex},...]
    // this is required if we
    //      a) didn't add the address to our wallet (createmultisig instead of addmultisigaddress)
    //      b) want to sign a raw tx only with a particular address's private key (the privkey is passed in another argument)

    BtcSigningPrerequisite();

    BtcSigningPrerequisite(const std::string &txId, const int64_t &vout, const std::string &scriptPubKey, const std::string &redeemScript);

    // all of these values must be set or else prerequisite is invalid
    void SetTxId(std::string txId);

    // all of these values must be set or else prerequisite is invalid
    void SetVout(Json::Int64 vout);

    // all of these values must be set or else prerequisite is invalid
    void SetScriptPubKey(std::string scriptPubKey);

    // all of these values must be set or else prerequisite is invalid
    void SetRedeemScript(std::string redeemScript);
};

// not really a packet.
struct BtcRpcPacket
{
    BtcRpcPacket();

    BtcRpcPacket(const std::string &strData);

    typedef _SharedPtr<BtcRpcPacket> BtcRpcPacketPtr;
    BtcRpcPacket(const BtcRpcPacketPtr packet);

    ~BtcRpcPacket();

    // appends data to data
    bool AddData(const std::string strData);

    // returns char and offsets the data pointer (makes no sense, will fix sometime)
    const char* ReadNextChar();

    const char* GetData();

    size_t size();

private:
    void SetDefaults();

    std::vector<char> data;     // received data or data to send
    size_t pointerOffset;       // need that for curl's way to send data
};


typedef _SharedPtr<BtcTransaction>         BtcTransactionPtr;
typedef _SharedPtr<BtcRawTransaction>      BtcRawTransactionPtr;
typedef _SharedPtr<BtcUnspentOutput>       BtcUnspentOutputPtr;
typedef _SharedPtr<BtcAddressInfo>         BtcAddressInfoPtr;
typedef _SharedPtr<BtcMultiSigAddress>     BtcMultiSigAddressPtr;
typedef _SharedPtr<BtcBlock>               BtcBlockPtr;
typedef _SharedPtr<BtcTxIdVout>            BtcTxIdVoutPtr;
typedef _SharedPtr<BtcTxTarget>            BtcTxTargetPtr;
typedef _SharedPtr<BtcSignedTransaction>   BtcSignedTransactionPtr;
typedef _SharedPtr<BtcSigningPrerequisite> BtcSigningPrerequisitePtr;
typedef _SharedPtr<BtcRpcPacket>           BtcRpcPacketPtr;


typedef std::vector<BtcUnspentOutputPtr> BtcUnspentOutputs;
typedef std::vector<BtcTxIdVoutPtr> BtcTxIdVouts;
//typedef std::vector<BtcTxTargetPtr> BtcTxTargets;






#endif // BTCOBJECTS_HPP
