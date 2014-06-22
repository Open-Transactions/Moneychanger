#ifndef BTCHELPER_HPP
#define BTCHELPER_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/TR1_Wrapper.hpp>

#include <bitcoin-api/btcobjects.hpp>

#include _CINTTYPES
#include _MEMORY

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


class BtcModules;

class BtcHelper
{
public:
    // default min confirmations to wait for before displaying a transaction
    static int32_t MinConfirms;

    // default max confirmations
    static int32_t MaxConfirms;

    // default confirmations to wait for before accepting a transaction as confirmed
    static int32_t WaitForConfirms;

    // default fee to pay when withdrawing from multi-sig
    // to set default fee for all transactions, use BtcJson::SetTxFee()
    static int64_t FeeMultiSig;

    BtcHelper(BtcModules* modules);

    ~BtcHelper();

    // converts a double bitcoin (as received through json api) to int64 satoshis
    static int64_t CoinsToSatoshis(double value);

    // converts int64 satoshis to double bitcoin
    static double SatoshisToCoins(int64_t value);

    // returns true if we own the address or any address making up a multisig
    bool IsMine(const std::string &address);

    BtcBalancesPtr GetBalances();

    // Returns the public key of an address (addresses are just hashes)
    // Public keys need to be shared to create multi signature addresses
    // calls 'validateaddress' which is kinda slow so use in moderation.
    std::string GetPublicKey(const std::string &address);

    // checks if tx contains the raw transaction (bitcoind v0.9.0)
    // if it doesn't the transaction will be fetched from tx index
    BtcRawTransactionPtr GetDecodedRawTransaction(const BtcTransactionPtr &tx) const;

    BtcRawTransactionPtr GetDecodedRawTransaction(const std::string &txId) const;

    // Counts how many coins are sent to targetAddress through this transaction
    int64_t GetTotalOutput(const std::string &txId, const std::string &targetAddress);

    // Counts how many coins are sent to targetAddress through this transaction
    int64_t GetTotalOutput(BtcRawTransactionPtr transaction, const std::string &targetAddress);

    int32_t GetConfirmations(const std::string &txId);

    // returns a list of double spends/conflicts
    btc::stringList GetDoubleSpends(const std::string &txId);

    // Checks whether a transaction has been confirmed often enough
    bool TransactionConfirmed(BtcTransactionPtr transaction, int32_t minconfirms = WaitForConfirms);

    // Checks whether a transaction (can be non-wallet) has been confirmed often enough
    int64_t TransactionConfirmed(const std::string &txId, const int32_t &minConfirms = WaitForConfirms);

    // Checks a transaction for correct amount and confirmations.
    bool TransactionSuccessfull(int64_t amount, BtcTransactionPtr transaction, const std::string &targetAddress, int minConfirms = WaitForConfirms);

    // Checks a raw transaction for correct amount, confirmations and recipient.
    bool TransactionSuccessfull(int64_t amount, BtcRawTransactionPtr transaction, const std::string &targetAddress, int32_t minConfirms = WaitForConfirms);

    // Checks a list of outputs for correct amount, confirmations and recipient.
    // returns the first output that matches
    const BtcRawTransactionPtr TransactionSuccessfull(const int64_t &amount, BtcUnspentOutputs outputs, const std::string &targetAddress, const int32_t &minConfirms = WaitForConfirms);

    // Halts thread execution until the transaction has enough confirmations
    // timeOutSeconds is the time in seconds after which the function will fail
    // timerMS is the delay between each confirmation check
    // returns true if sufficient confirmations were received before timeout
    bool WaitTransactionSuccessfull(const int64_t &amount, BtcTransactionPtr transaction, const std::string &targetAddress, const int32_t &minConfirms = WaitForConfirms, int32_t timeOutSeconds = 7200, const int32_t &timerMS = 1000);

    bool WaitTransactionSuccessfull(const int64_t &amount, BtcRawTransactionPtr transaction, const std::string &targetAddress, const int32_t &minConfirms = WaitForConfirms, int32_t timeOutSeconds = 7200, const int32_t &timerMS = 1000);

    // Halts thread execution and returns the transaction once it arrives
    // Will only work if you have all keys of the sending/receiving address or added it as watchonly
    BtcTransactionPtr WaitGetTransaction(const std::string &txId, const int32_t &timerMS = 500, const int32_t &maxAttempts = 20);

    // Halts thread execution and returns the decoded raw transaction once it arrives
    BtcRawTransactionPtr WaitGetRawTransaction(const std::string &txId, const int32_t &timerMS = 500, int32_t maxAttempts = 20);

    // asks bitcoind for a list of unspent outputs and returns those that aren't already known
    // addresses: list of receiving addresses
    // knownOutputs: list of unspent outputs that is already known
    // returns unknown outputs
    BtcUnspentOutputs ListNewOutputs(BtcUnspentOutputs knownOutputs, const btc::stringList &addresses = btc::stringList());

    // Looks through a list of txIds to find everything we can sign, including multisig addresses
    // Only works when bitcoind received the blocks containing those transactions.
    BtcUnspentOutputs FindSignableOutputs(const btc::stringList &txIds);

    BtcUnspentOutputs FindSignableOutputs(const BtcUnspentOutputs &outputs);

    // checks a list of tx outputs if they are spent, returns unspent
    BtcUnspentOutputs FindUnspentOutputs(BtcUnspentOutputs possiblySpentOutputs);

    BtcUnspentOutputs FindUnspentOutputs(const btc::stringList &txIdsToCheck);

    // Looks through a list of txIds to find everything we can sign and haven't spent yet
    // only works with own and imported addresses and up-to-date blockchain
    BtcUnspentOutputs FindUnspentSignableOutputs(const btc::stringList &txIds);

    // Creates the signing prerequisites corresponding to those outputs
    BtcSigningPrerequisites GetSigningPrerequisites(const BtcUnspentOutputs &outputs);

    // Creates an unsigned raw transaction that sends coins to a target address, pays a fee and returns change.
    // outputs:         outputs to spend
    // amount:          amount in satoshis
    // toAddress:       send to
    // changeAddress:   receive change to
    // fee:                     optional, fee in satoshis
    // SigningPreRequisites:    optional, needed for offline transactions or when passing an array of private keys
    // signingKeys:             optional, only sign with those keys
    BtcSignedTransactionPtr CreateSpendTransaction(const BtcUnspentOutputs &outputs, const int64_t &amount, const std::string &toAddress, const std::string &changeAddress, const int64_t &fee = FeeMultiSig);

    // Creates an unsigned raw transaction that sends all unspent outputs from an address to another
    // txSourceId: transaction that sends funds to sourceAddress
    // sourceAddress: address from which you want to withdraw
    // destinationAddress: address to which to withdraw
    // signingAddress: only this address's private key will be used to sign the tx
    // redeemScript: the script needed to withdraw btc from p2sh addresses
    BtcSignedTransactionPtr WithdrawAllFromAddress(const std::string &txSourceId, const std::string &sourceAddress, const std::string &destinationAddress, const int64_t fee = FeeMultiSig, const std::string &redeemScript = "", const std::string &signingAddress = "");


private:
    BtcModules* modules;
};

typedef _SharedPtr<BtcHelper> BtcHelperPtr;


namespace btc
{
    void Sleep(time_t milliSeconds);
    template <typename T>
    std::string to_string(T number);
}

#endif // BTCHELPER_HPP
