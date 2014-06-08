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


namespace btc
{   // might make things easier to read. also maybe i should use vectors instead.
    typedef std::vector<std::string> stringList;
}



// TODO: error checking


struct BtcTxDetail
{
    bool involvesWatchonly;
    std::string account;
    std::string address;
    std::string category;   // send, receive, coinbase
    int64_t amount;
    int64_t fee;

    BtcTxDetail(const Json::Value &detail);
};
typedef _SharedPtr<BtcTxDetail>     BtcTxDetailPtr;
typedef std::vector<BtcTxDetailPtr> BtcTxDetails;

// This struct holds various information about a bitcoin transaction.
// Its constructor accepts the Json::Value returned by "gettransaction".
// returned by 'gettransaction' and 'listtransactions'
struct BtcTransaction
{
    int64_t Amount;         // the sum of received/sent in Details[]
    int64_t Fee;            // optional
    int32_t Confirmations;  // can be -1
    std::string Blockhash;  // optional
    int32_t BlockIndex;     // optional
    time_t Blocktime;       // optional
    std::string TxId;
    btc::stringList walletConflicts;
    time_t Time;
    time_t TimeReceived;
    BtcTxDetails Details;
    std::string Hex;        // optional, returned by gettransaction

    BtcTransaction(Json::Value reply);

private:
    void SetDefaults();
};

struct BtcRawTransaction
{
    std::string txID;
    int32_t Version;
    time_t LockTime;

    struct VIN
    {
        std::string txInID;
        int64_t vout;                   // number of txInID's output to be used as input
        // std::string ScriptSigAsm;    // add this when you need it
        std::string ScriptSigHex;
        int64_t Sequence;

        VIN(const std::string &txInID, const int64_t &vout, const std::string &scriptSigHex, const int64_t &sequence)
            :txInID(txInID), vout(vout), ScriptSigHex(scriptSigHex), Sequence(sequence)
        {}
    };
    std::vector<VIN> inputs;

    struct VOUT
    {
        int64_t value;                  // amount of satoshis to be sent
        uint32_t n ;                     // outputs array index
        // std::string ScriptPubKeyAsm; // add this when you need it
        std::string scriptPubKeyHex;    // needed to spend offline transactions
        uint32_t reqSigs;               // signatures required to spend the output I think?
        std::string Type;               // scripthash, pubkeyhash, ..
        btc::stringList addresses;      // an array of addresses receiving the value.

        int64_t sequence;

        VOUT()
        {
            this->value = 0;
            this->n = -1;
            this->reqSigs = 0;
            this->addresses = btc::stringList();
            this->scriptPubKeyHex = "";
        }

        VOUT(const int64_t &value, const int64_t &n, const std::string &scriptPubKeyHex, const uint32_t &reqSigs, const std::string &type, const btc::stringList &addresses)
            :value(value), n(n), scriptPubKeyHex(scriptPubKeyHex), reqSigs(reqSigs), Type(type), addresses(addresses)
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

// returned by listreceivedbyaddress
struct BtcAddressBalance
{
    bool involvesWatchonly;
    std::string address;
    std::string account;
    int64_t amount;
    int32_t confirmations;
    btc::stringList txIds;

    BtcAddressBalance(Json::Value addressBalance);
};

struct BtcAddressInfo
{
    bool isvalid;
    std::string address;
    bool ismine;
    bool isWatchonly;
    bool isScript;

    // regular addresses:
    std::string pubkey;
    bool isCompressed;

    // p2sh:
    std::string script;         // "multisig"
    std::string redeemScript;   // hex
    btc::stringList addresses;  // shows addresses which a multi-sig is composed of
    uint32_t sigsRequired;

    std::string account;

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
    btc::stringList publicKeys;     // this will make everything so much more convenient

    BtcMultiSigAddress(Json::Value result, const btc::stringList& publicKeys);
};

struct BtcBlock
{
    int64_t confirmations;
    btc::stringList transactions;
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
struct BtcTxTargets : Json::Value
{
    BtcTxTargets();
    BtcTxTargets(const std::string &toAddress, int64_t amount);
    void SetTarget(const std::string &toAddress, int64_t amount);
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


typedef _SharedPtr<BtcTxDetail>            BtcTxDetailPtr;
typedef _SharedPtr<BtcTransaction>         BtcTransactionPtr;
typedef _SharedPtr<BtcRawTransaction>      BtcRawTransactionPtr;
typedef _SharedPtr<BtcUnspentOutput>       BtcUnspentOutputPtr;
typedef _SharedPtr<BtcAddressBalance>      BtcAddressBalancePtr;
typedef _SharedPtr<BtcAddressInfo>         BtcAddressInfoPtr;
typedef _SharedPtr<BtcMultiSigAddress>     BtcMultiSigAddressPtr;
typedef _SharedPtr<BtcBlock>               BtcBlockPtr;
typedef _SharedPtr<BtcTxIdVout>            BtcTxIdVoutPtr;
typedef _SharedPtr<BtcTxTargets>           BtcTxTargetPtr;
typedef _SharedPtr<BtcSignedTransaction>   BtcSignedTransactionPtr;
typedef _SharedPtr<BtcSigningPrerequisite> BtcSigningPrerequisitePtr;
typedef _SharedPtr<BtcRpcPacket>           BtcRpcPacketPtr;

typedef std::vector<BtcTxDetailPtr>        BtcTxDetails;
typedef std::vector<BtcUnspentOutputPtr>   BtcUnspentOutputs;
typedef std::vector<BtcAddressBalancePtr>  BtcAddressBalances;
typedef std::vector<BtcTxIdVoutPtr>        BtcTxIdVouts;
typedef std::vector<BtcTransactionPtr>     BtcTransactions;
typedef std::list<BtcSigningPrerequisitePtr> BtcSigningPrerequisites;   // must be a list so the json lib accepts it
//typedef std::vector<BtcTxTargetPtr> BtcTxTargets;






#endif // BTCOBJECTS_HPP
