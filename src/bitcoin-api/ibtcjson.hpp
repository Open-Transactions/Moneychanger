#ifndef IBTCJSON_HPP
#define IBTCJSON_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/TR1_Wrapper.hpp>

#include <bitcoin-api/btcobjects.hpp>
#include <bitcoin-api/btchelper.hpp>

#include <json/json.h>

#include _CINTTYPES
#include _MEMORY

#include <string>
#include <list>
#include <map>

#include "FastDelegate.hpp"



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


class IBtcJson
{
public:
    virtual void Initialize() = 0;       // should make this part of all modules

    // when a function requires a password getPasswordFunc() is called and expected to return the pw
    virtual void SetPasswordCallback(fastdelegate::FastDelegate0<std::string> getPasswordFunc) = 0;

    virtual BtcInfoPtr GetInfo() = 0;

    // returns account balance
    // account: use NULL or "*" for all accounts and "" for the default account
    // minConfirmations: duh.
    // includeWatchonly: whether to include watchonly addresses in balance calculations (default = true)
    virtual int64_t GetBalance(const std::string &account = "*", const int32_t &minConfirmations = BtcHelper::MinConfirms, const bool &includeWatchonly = true) = 0;

    // Gets the default address for the specified account
    virtual std::string GetAccountAddress(const std::string &account = "") = 0;

    // Returns list of all addresses belonging to account
    virtual btc::stringList GetAddressesByAccount(const std::string &account = "") = 0;

    // Add new address to account
    virtual std::string GetNewAddress(const std::string &account = "btcapi") = 0;

    // Import a watch-only address
    virtual bool ImportAddress(const std::string &address, const std::string &account = "watchonly", const bool &rescan = false) = 0;

    // Validate an address
    // returns some useful info
    // This function is quite slow so use in moderation
    virtual BtcAddressInfoPtr ValidateAddress(const std::string &address) = 0;

    virtual std::string GetPublicKey(const std::string& address) = 0;

    // Get private key for address (calls DumpPrivKey())
    virtual std::string GetPrivateKey(const std::string &address) = 0;

    // Get private key for address
    virtual std::string DumpPrivKey(const std::string &address) = 0;

    // Adds an address requiring n keys to sign before it can spend its inputs
    // nRequired: number of signatures required
    // keys: list of public keys (addresses work too, if the public key is known)
    // account [optional]: account to add the address to
    // Returns the multi-sig address
    virtual BtcMultiSigAddressPtr AddMultiSigAddress(const uint32_t &nRequired, const btc::stringList &keys, const std::string &account = "multisig") = 0;

    // Creates a multi-sig address without adding it to the wallet
    // nRequired: signatures required
    // keys: list of public keys (addresses work too, if the public key is known)
    virtual BtcMultiSigAddressPtr CreateMultiSigAddress(const uint32_t &nRequired, const btc::stringList &keys) = 0;

    // Creates a multi-sig address and returns its redeemScript
    // the address will not be added to your address list, use AddMultiSigAddress for that
    virtual std::string GetRedeemScript(const uint32_t &nRequired, const btc::stringList &keys) = 0;

    // Returns list of account names
    virtual btc::stringList ListAccounts(const int32_t &minConf = BtcHelper::MinConfirms, const bool &includeWatchonly = true) = 0;

    // Returns list of addresses, their balances and txids
    virtual BtcAddressBalances ListReceivedByAddress(const int32_t &minConf = BtcHelper::MinConfirms, const bool &includeEmpty = false, const bool &includeWatchonly = true) = 0;

    // Returns list of transactions
    virtual BtcTransactions ListTransactions(const std::string &account = "*", const int32_t &count = 20, const int32_t &from = 0, const bool &includeWatchonly = true) = 0;

    // Returns vector of unspent outputs
    // does not work with non-wallet addresses (multisig)
    virtual BtcUnspentOutputs ListUnspent(const int32_t &minConf = BtcHelper::MinConfirms, const int32_t &maxConf = BtcHelper::MaxConfirms, const btc::stringList &addresses = btc::stringList()) = 0;

    virtual std::string SendToAddress(const std::string &btcAddress, const int64_t &amount) = 0;

    // Send to multiple addresses at once
    // txTargets maps amounts (int64 satoshis) to addresses (QString)
    virtual std::string SendMany(BtcTxTargets txTargets, const std::string &fromAccount = "") = 0;

    virtual bool SetTxFee(const int64_t &fee) = 0;

    virtual BtcUnspentOutputPtr GetTxOut(const std::string &txId, const int64_t &vout) = 0;

    virtual BtcTransactionPtr GetTransaction(const std::string &txId, const bool& includeWatchonly = true) = 0;

    virtual std::string GetRawTransaction(const std::string &txId) = 0;

    virtual BtcRawTransactionPtr GetDecodedRawTransaction(const std::string &txId) = 0;

    virtual BtcRawTransactionPtr DecodeRawTransaction(const std::string &rawTransaction) = 0;

    virtual std::string CreateRawTransaction(BtcTxIdVouts unspentOutputs, BtcTxTargets txTargets) = 0;

    virtual BtcSignedTransactionPtr SignRawTransaction(const std::string &rawTransaction, const BtcSigningPrerequisites &previousTransactions = BtcSigningPrerequisites(), const btc::stringList &privateKeys = btc::stringList()) = 0;

    virtual BtcSignedTransactionPtr CombineSignedTransactions(const std::string &rawTransaction) = 0;

    virtual std::string SendRawTransaction(const std::string &rawTransaction, const bool &acceptHighFee = false) = 0;

    virtual btc::stringList GetRawMemPool() = 0;

    virtual int32_t GetBlockCount() = 0;

    virtual std::string GetBlockHash(const int32_t &blockNumber) = 0;

    virtual BtcBlockPtr GetBlock(const std::string &blockHash) = 0;

    virtual bool SetGenerate(const bool &generate) = 0;

    virtual bool WalletPassphrase(const std::string &password, const time_t &unlockTime) = 0;
};

typedef _SharedPtr<IBtcJson> IBtcJsonPtr;

#endif // IBTCJSON_HPP
