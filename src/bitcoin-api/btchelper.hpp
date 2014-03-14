#ifndef BTCHELPER_HPP
#define BTCHELPER_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/MemoryWrapper.hpp>

#include <bitcoin-api/btcobjects.hpp>

#include <sys/types.h>
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
    // default amount of confirmations to wait for
    static int MinConfirms;

    // default fee to pay when withdrawing from multi-sig
    // to set default fee for all transactions, use BtcJson::SetTxFee()
    static int FeeMultiSig;

    BtcHelper(BtcModules* modules);

    ~BtcHelper();

    // converts a double bitcoin (as received through json api) to int64 satoshis
    static int64_t CoinsToSatoshis(double value);

    // converts int64 satoshis to double bitcoin
    static double SatoshisToCoins(int64_t value);

    // Returns the public key of an address (addresses are just hashes)
    // Pub keys need to be shared to create multi signature addresses
    std::string GetPublicKey(const std::string &address);

    // Counts how many coins are sent to targetAddress through this transaction
    int64_t GetTotalOutput(const std::string &transactionId, const std::string &targetAddress);

    // Counts how many coins are sent to targetAddress through this transaction
    int64_t GetTotalOutput(BtcRawTransactionPtr transaction, const std::string &targetAddress);

    int64_t GetConfirmations(const std::string &txId);

    // Checks whether a transaction has been confirmed often enough
    bool TransactionConfirmed(BtcTransactionPtr transaction, int minconfirms = MinConfirms);

    // Checks whether a transaction (can be non-wallet) has been confirmed often enough
    int64_t TransactionConfirmed(const std::string &txId, int minConfirms = MinConfirms);

    // Checks a transaction for correct amount and confirmations.
    bool TransactionSuccessfull(int64_t amount, BtcTransactionPtr transaction, int minConfirms = MinConfirms);

    // Checks a raw transaction for correct amount, confirmations and recipient.
    // We need this because bitcoin-qt offers no good way to watch multi-sig addresses if we don't own all the keys
    bool TransactionSuccessfull(int64_t amount, BtcRawTransactionPtr transaction, const std::string &targetAddress, int minConfirms = MinConfirms);


    // Halts thread execution until the transaction has enough confirmations
    // timeOutSeconds is the time in seconds after which the function will fail
    // timerMS is the delay between each confirmation check
    // returns true if sufficient confirmations were received before timeout
    bool WaitTransactionSuccessfull(int64_t amount, BtcTransactionPtr transaction, int minConfirms = MinConfirms, double timeOutSeconds = 7200, double timerSeconds = 1);

    bool WaitTransactionSuccessfull(int64_t amount, BtcRawTransactionPtr transaction, const std::string &targetAddress, int minConfirms = MinConfirms, double timeOutSeconds = 7200, double timerSeconds = 1);

    // Halts thread execution until the bitcoin client learns about a new transaction
    bool WaitForTransaction(const std::string &txID, int timerMS = 500, int maxAttempts = 20);

    // Halts thread execution and returns the transaction once it arrives
    // Will only work if you have all keys of the receiving address.
    BtcTransactionPtr WaitGetTransaction(const std::string &txID, int timerMS = 500, int maxAttempts = 20);

    // Halts thread execution and returns the decoded raw transaction once it arrives
    BtcRawTransactionPtr WaitGetRawTransaction(const std::string &txID, int timerMS = 500, int maxAttempts = 20);

    // Creates a raw transaction that sends all unspent outputs from an address to another
    // txSourceId: transaction that sends funds to sourceAddress
    // sourceAddress: address from which you want to withdraw
    // destinationAddress: address to which to withdraw
    // signingAddress: only this address's private key will be used to sign the tx
    // redeemScript: the script needed to withdraw btc from p2sh addresses
    BtcSignedTransactionPtr WithdrawAllFromAddress(const std::string &txSourceId, const std::string &sourceAddress, const std::string &destinationAddress, int64_t fee = BtcHelper::CoinsToSatoshis(0.0001), const std::string &redeemScript = "", const std::string &signingAddress = "");


private:
    BtcModules* modules;
};

#ifndef OT_USE_TR1
    typedef std::shared_ptr<BtcHelper> BtcHelperPtr;
#else
    typedef std::tr1::shared_ptr<BtcHelper> BtcHelperPtr;
#endif // OT_USE_TR1


namespace btc
{
    template <typename T>
    std::string to_string(T number);
}

#endif // BTCHELPER_HPP
