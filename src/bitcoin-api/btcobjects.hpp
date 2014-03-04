#ifndef BTCOBJECTS_HPP
#define BTCOBJECTS_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/MemoryWrapper.hpp>

#include <json/json.h>

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

        VIN(std::string txInID, int64_t vout)
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

        VOUT(int64_t value, int64_t n, uint32_t reqSigs, std::vector<std::string> addresses, std::string scriptPubKeyHex)
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
    double Amount;
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
    BtcTxIdVout(std::string txID, int64_t vout);
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
struct BtcSigningPrequisite : Json::Value
{
    // an array of BtcSigningPrequisites can be passed to bitcoin- as an optional argument to signrawtransaction
    // [{"txid":txid,"vout":n,"scriptPubKey":hex,"redeemScript":hex},...]
    // this is required if we
    //      a) didn't add the address to our wallet (createmultisig instead of addmultisigaddress)
    //      b) want to sign a raw tx only with a particular address's private key (the privkey is passed in another argument)

    BtcSigningPrequisite();

    BtcSigningPrequisite(std::string txId, int64_t vout, std::string scriptPubKey, std::string redeemScript);

    // all of these values must be set or else prequisite is invalid
    void SetTxId(std::string txId);

    // all of these values must be set or else prequisite is invalid
    void SetVout(Json::Int64 vout);

    // all of these values must be set or else prequisite is invalid
    void SetScriptPubKey(std::string scriptPubKey);

    // all of these values must be set or else prequisite is invalid
    void SetRedeemScript(std::string redeemScript);
};

// not really a packet.
struct BtcRpcPacket
{
    BtcRpcPacket();

    BtcRpcPacket(const std::string &strData);

#ifndef OT_USE_TR1
    typedef std::shared_ptr<BtcRpcPacket> BtcRpcPacketPtr;
#else
    typedef std::tr1::shared_ptr<BtcRpcPacket> BtcRpcPacketPtr;
#endif // OT_USE_TR1
    BtcRpcPacket(const BtcRpcPacketPtr packet);

    ~BtcRpcPacket();

    // appends data to data
    bool AddData(const std::string &strData);

    // returns char and offsets the data pointer (makes no sense, will fix sometime)
    const char* ReadNextChar();

    const char* GetData();

    size_t size();

private:
    void SetDefaults();

    std::vector<char> data;     // received data or data to send
    ptrdiff_t pointerOffset;  // need that for curl's way to send data
};


#ifndef OT_USE_TR1
typedef std::shared_ptr<BtcTransaction>         BtcTransactionPtr;
typedef std::shared_ptr<BtcRawTransaction>      BtcRawTransactionPtr;
typedef std::shared_ptr<BtcUnspentOutput>       BtcUnspentOutputPtr;
typedef std::shared_ptr<BtcAddressInfo>         BtcAddressInfoPtr;
typedef std::shared_ptr<BtcMultiSigAddress>     BtcMultiSigAddressPtr;
typedef std::shared_ptr<BtcBlock>               BtcBlockPtr;
typedef std::shared_ptr<BtcTxIdVout>            BtcTxIdVoutPtr;
typedef std::shared_ptr<BtcTxTarget>            BtcTxTargetPtr;
typedef std::shared_ptr<BtcSignedTransaction>   BtcSignedTransactionPtr;
typedef std::shared_ptr<BtcSigningPrequisite>   BtcSigningPrequisitePtr;
typedef std::shared_ptr<BtcRpcPacket>           BtcRpcPacketPtr;
#else
typedef std::tr1::shared_ptr<BtcTransaction>        BtcTransactionPtr;
typedef std::tr1::shared_ptr<BtcRawTransaction>     BtcRawTransactionPtr;
typedef std::tr1::shared_ptr<BtcUnspentOutput>      BtcUnspentOutputPtr;
typedef std::tr1::shared_ptr<BtcAddressInfo>        BtcAddressInfoPtr;
typedef std::tr1::shared_ptr<BtcMultiSigAddress>    BtcMultiSigAddressPtr;
typedef std::tr1::shared_ptr<BtcBlock>              BtcBlockPtr;
typedef std::tr1::shared_ptr<BtcTxIdVout>           BtcTxIdVoutPtr;
typedef std::tr1::shared_ptr<BtcTxTarget>           BtcTxTargetPtr;
typedef std::tr1::shared_ptr<BtcSignedTransaction>  BtcSignedTransactionPtr;
typedef std::tr1::shared_ptr<BtcSigningPrequisite>  BtcSigningPrequisitePtr;
typedef std::tr1::shared_ptr<BtcRpcPacket>          BtcRpcPacketPtr;
#endif

typedef std::vector<BtcUnspentOutputPtr> BtcUnspentOutputs;
typedef std::vector<BtcTxIdVoutPtr> BtcTxIdVouts;
//typedef std::vector<BtcTxTargetPtr> BtcTxTargets;






#endif // BTCOBJECTS_HPP
