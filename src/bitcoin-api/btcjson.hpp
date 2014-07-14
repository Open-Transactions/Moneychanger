#ifndef BTCJSON_HPP
#define BTCJSON_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/TR1_Wrapper.hpp>

#include <bitcoin-api/ibtcjson.hpp>
#include <bitcoin-api/btcmodules.hpp>

#include "FastDelegate.hpp"

#include _CINTTYPES
#include _MEMORY

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

///* https://en.bitcoin.it/wiki/Original_Bitcoin_client/API_Calls_list *\\\


class BtcJson : public IBtcJson
{
public:
    BtcJson(BtcModules* modules);
    ~BtcJson();

    virtual void Initialize();       // should make this part of all modules

    // when a function requires a password getPasswordFunc is called and expected to return the pw
    virtual void SetPasswordCallback(fastdelegate::FastDelegate0<std::string> callbackFunction);

    // asks bitcoind for details but doesn't do anything with the result yet
    virtual BtcInfoPtr GetInfo();

    // returns account balance
    // account: use NULL or "*" for all accounts and "" for the default account
    // minConfirmations: duh.
    // includeWatchonly: whether to include watchonly addresses in balance calculations (default = true)
    virtual int64_t GetBalance(const std::string &account = "*", const int32_t &minConfirmations = BtcHelper::WaitForConfirms, const bool &includeWatchonly = true);

    // Gets the default address for the specified account
    virtual std::string GetAccountAddress(const std::string &account = "");

    // Returns list of all addresses belonging to account
    // account "" is default
    virtual btc::stringList GetAddressesByAccount(const std::string &account = "");

    // Add new address to account
    virtual std::string GetNewAddress(const std::string &account = "btcapi");

    // Import a watch-only address
    virtual bool ImportAddress(const std::string &address, const std::string &account = "watchonly", const bool &rescan = false);

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
    virtual BtcMultiSigAddressPtr AddMultiSigAddress(const uint32_t &nRequired, const btc::stringList &keys, const std::string &account = "multisig");

    // Creates a multi-sig address without adding it to the wallet
    // nRequired: signatures required
    // keys: list of public keys (addresses work too, if the public key is known)
    virtual BtcMultiSigAddressPtr CreateMultiSigAddress(const uint32_t &nRequired, const btc::stringList &keys);

    // deprecated, use 'validateaddress' instead
    // Creates a multi-sig address and returns its redeemScript
    // the address will not be added to your address list, use AddMultiSigAddress for that
    virtual std::string GetRedeemScript(const uint32_t &nRequired, const btc::stringList &keys);

    // Returns list of account names
    // Could also return the balance of each account
    virtual btc::stringList ListAccounts(const int32_t &minConf = BtcHelper::MinConfirms, const bool &includeWatchonly = true);

    // Returns list of addresses, their balances and txids
    virtual BtcAddressBalances ListReceivedByAddress(const int32_t &minConf = BtcHelper::MinConfirms, const bool &includeEmpty = false, const bool &includeWatchonly = true);

    // lists transactions
    // account: "*" for all accounts, "" for default account, "<whatever> for <whatever>
    virtual BtcTransactions ListTransactions(const std::string &account = "*", const int32_t &count = 20, const int32_t &from = 0, const bool &includeWatchonly = true);

    virtual BtcUnspentOutputs ListUnspent(const int32_t &minConf = BtcHelper::MinConfirms, const int32_t &maxConf = BtcHelper::MaxConfirms, const btc::stringList &addresses = btc::stringList());

    virtual std::string SendToAddress(const std::string &btcAddress, const int64_t &amount);

    // Send to multiple addresses at once
    // txTargets maps amounts (int64 satoshis) to addresses (QString)
    virtual std::string SendMany(BtcTxTargets txTargets, const std::string &fromAccount = "");

    virtual bool SetTxFee(const int64_t &fee);

    BtcUnspentOutputPtr GetTxOut(const std::string &txId, const int64_t &vout);

    virtual BtcTransactionPtr GetTransaction(const std::string &txId, const bool& includeWatchonly = true);

    virtual std::string GetRawTransaction(const std::string &txId);

    virtual BtcRawTransactionPtr GetDecodedRawTransaction(const std::string &txId);

    virtual BtcRawTransactionPtr DecodeRawTransaction(const std::string &rawTransaction);

    virtual std::string CreateRawTransaction(BtcTxIdVouts unspentOutputs, BtcTxTargets txTargets);

    // sign a raw transaction
    // rawTransaction: hex
    // previousTransactions: array of json objects containing input's txid, vout, redeemScript (if p2sh) and scriptPubKey
    // privateKeys: only sign with the keys provided (makes signing transactions too bcreated by someone else slightly more secure)
    // i think bitcoind requires previousTransactions if privateKeys are given.
    virtual BtcSignedTransactionPtr SignRawTransaction(const std::string &rawTransaction, const BtcSigningPrerequisites &previousTransactions = BtcSigningPrerequisites(), const btc::stringList &privateKeys = btc::stringList());

    // combines multiple transactions to one, does not do any signing for security reasons.
    // used when various parties sign a multisig transaction.
    // rawTransaction: concatenated string of the raw hex transactions
    virtual BtcSignedTransactionPtr CombineSignedTransactions(const std::string &rawTransaction);

    virtual std::string SendRawTransaction(const std::string &rawTransaction, const bool &allowHighFees = false);

    // list of unconfirmed transactions in memory
    // returns txIds
    virtual btc::stringList GetRawMemPool();

    virtual int GetBlockCount();

    virtual std::string GetBlockHash(const int32_t &blockNumber);

    virtual BtcBlockPtr GetBlock(const std::string &blockHash);

    virtual bool SetGenerate(const bool &generate);

    virtual bool WalletPassphrase(const std::string &password, const time_t &unlockTime);

protected:
    virtual BtcRpcPacketPtr CreateJsonQuery(const std::string &command, const Json::Value &params = Json::Value(), std::string id = std::string());

    // sends a query and processes errors. useless now but maybe not in the future.
    virtual bool SendJsonQuery(BtcRpcPacketPtr jsonString, Json::Value &result);

     // Checks the reply object received from bitcoin-qt for errors and returns the reply
    virtual bool ProcessRpcString(BtcRpcPacketPtr jsonString, Json::Value &result);
    // Splits the reply object received from bitcoin-qt into error and result objects
    virtual void ProcessRpcString(BtcRpcPacketPtr jsonString, std::string &id, Json::Value& error, Json::Value& result);

    virtual bool ProcessError(const Json::Value &error, BtcRpcPacketPtr jsonString, Json::Value &result);

    void UnlockWallet();

    BtcModules* modules;

    fastdelegate::FastDelegate0<std::string> passwordCallback;

    // default time for which to unlock the wallet
    static int32_t walletUnlockTime;
};

typedef _SharedPtr<BtcJson> BtcJsonPtr;


namespace btc
{
    namespace json
    {
        // Bitcoin RPC error codes
        enum RPCErrorCode
        {
            // Standard JSON-RPC 2.0 errors
            RPC_INVALID_REQUEST = -32600,
            RPC_METHOD_NOT_FOUND = -32601,
            RPC_INVALID_PARAMS = -32602,
            RPC_INTERNAL_ERROR = -32603,
            RPC_PARSE_ERROR = -32700,

            // General application defined errors
            RPC_MISC_ERROR = -1,                    // std::exception thrown in command handling
            RPC_FORBIDDEN_BY_SAFE_MODE = -2,        // Server is in safe mode, and command is not allowed in safe mode
            RPC_TYPE_ERROR = -3,                    // Unexpected type was passed as parameter
            RPC_INVALID_ADDRESS_OR_KEY = -5,        // Invalid address or key
            RPC_OUT_OF_MEMORY = -7,                 // Ran out of memory during operation
            RPC_INVALID_PARAMETER = -8,             // Invalid, missing or duplicate parameter
            RPC_DATABASE_ERROR = -20,               // Database error
            RPC_DESERIALIZATION_ERROR = -22,        // Error parsing or validating structure in raw format
            RPC_TRANSACTION_ERROR = -25,            // General error during transaction submission
            RPC_TRANSACTION_REJECTED = -26,         // Transaction was rejected by network rules
            RPC_TRANSACTION_ALREADY_IN_CHAIN= -27,  // Transaction already in chain

            // P2P client errors
            RPC_CLIENT_NOT_CONNECTED = -9,          // Bitcoin is not connected
            RPC_CLIENT_IN_INITIAL_DOWNLOAD = -10,   // Still downloading initial blocks
            RPC_CLIENT_NODE_ALREADY_ADDED = -23,    // Node is already added
            RPC_CLIENT_NODE_NOT_ADDED = -24,        // Node has not been added before

            // Wallet errors
            RPC_WALLET_ERROR = -4,                  // Unspecified problem with wallet (key not found etc.)
            RPC_WALLET_INSUFFICIENT_FUNDS = -6,     // Not enough funds in wallet or account
            RPC_WALLET_INVALID_ACCOUNT_NAME = -11,  // Invalid account name
            RPC_WALLET_KEYPOOL_RAN_OUT = -12,       // Keypool ran out, call keypoolrefill first
            RPC_WALLET_UNLOCK_NEEDED = -13,         // Enter the wallet passphrase with walletpassphrase first
            RPC_WALLET_PASSPHRASE_INCORRECT = -14,  // The wallet passphrase entered was incorrect
            RPC_WALLET_WRONG_ENC_STATE = -15,       // Command given in wrong wallet encryption state (encrypting an encrypted wallet etc.)
            RPC_WALLET_ENCRYPTION_FAILED = -16,     // Failed to encrypt the wallet
            RPC_WALLET_ALREADY_UNLOCKED = -17       // Wallet is already unlocked
        };
    }
}


#endif // BTCJSON_HPP
