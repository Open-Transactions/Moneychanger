#ifndef IMTBITCOIN_HPP
#define IMTBITCOIN_HPP

#include <opentxs/MemoryWrapper.hpp>

#include <bitcoin-api/btcobjects.hpp>
#include <bitcoin-api/btchelper.hpp>

#include <map>
#include <sys/types.h>
#include <string>
#include <list>



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

// interface
class IMTBitcoin
{
public:
    // account [optional]: the account whose balance should be checked
    // returns total balance of all addresses.
    virtual int64_t GetBalance(const char *account = NULL) = 0;

    // account [optional]: the account to which the address should be added.
    // returns new address
    virtual std::string GetNewAddress(const std::string &account = "") = 0;

    // returns public key for address (works only if public key is known...)
    virtual std::string GetPublicKey(const std::string &address) = 0;

    // Creates a multi-sig address from public keys
    // minSignatures:           signatures required to release funds
    // publicKeys:              an array containing the public keys of the addresses
    // addToWallet [optional]:  whether or not to add address to wallet
    // account [optional]:      account to which the address should be added
    // returns the multi-sig address
    virtual std::string GetMultiSigAddress(int minSignatures, const std::list<std::string>& publicKeys, bool addToWallet = false, const std::string &account = "") = 0;

    // Creates a multi-sig address from public keys
    // minSignatures:           signatures required to release funds
    // publicKeys:              an array containing the public keys of the addresses
    // addToWallet [optional]:  whether or not to add address to wallet
    // account [optional]:      account to which the address should be added
    // returns an object containing the address and additional info
    virtual BtcMultiSigAddressPtr GetMultiSigAddressInfo(int minSignatures, const std::list<std::string>& publicKeys, bool addToWallet = true, const std::string &account = "") = 0;

    // Returns an object containing information about a raw transaction
    virtual BtcRawTransactionPtr GetRawTransaction(const std::string &txId) = 0;

    // Returns an object containing information about a raw transaction
    // times out after 10 seconds
    virtual BtcRawTransactionPtr WaitGetRawTransaction(const std::string &txId) = 0;

    // Returns the number of confirmations of a raw transaction
    virtual int GetConfirmations(const std::string &txId) = 0;

    // Checks whether transaction sends correct amount and is confirmed
    // amount: amount (in satoshis)
    // rawTransaction:              transaction that was used to deposit funds
    // targetAddress:               the address into which funds were deposited (in case someone sent to multiple unrelated addresses in one tx)
    // confirmations [optional]:    minimum amount of confirmations (default: 1 (we should increase this))
    // returns true if amount and confirmations are equal or greater than required
    virtual bool TransactionSuccessfull(int64_t amount, BtcRawTransactionPtr rawTransaction, const std::string &targetAddress, int64_t confirmations = 1) = 0;

    // sends funds from your wallet to targetAddress
    // lAmount: integer containing amount in satoshis
    // returns the transaction id string or NULL
    virtual std::string SendToAddress(const std::string &to_address, int64_t lAmount) = 0;

    // Creates a multi-sig address using the public keys (not addresses, unless their public keys are known to bitcoind!)
    // and sends btc to that address.
    // lAmount:         integer containing amount (in satoshis)
    // nRequired:       number of signatures required
    // to_publicKeys:   an array containing the public keys of the addresses
    // returns the transaction id string or ""
    virtual std::string SendToMultisig(int64_t lAmount, int nRequired, const std::list<std::string> &to_publicKeys) = 0;

    // creates a partially signed raw transaction to withdraw funds
    // every escrow participant who wants to vote "yes" has to call this
    // then their results have to be concatenated and fed to CombineTransactions()
    // txToSourceId:                transaction id that was used to fund the pool
    // sourceAddress:               used to know which outputs of the tx we can spend (in case it came from a SendMany() tx)
    // redeemScript [optional]:     is passed to the API to do some bitcoin magic. required if unknown or if passing a signingAddress
    // signingAddress [optional]:   only this address's private key will be used to sign the transaction (kinda pointless in our case)
    virtual BtcSignedTransactionPtr VoteMultiSigRelease(const std::string &txToSourceId, const std::string &sourceAddress, const std::string &destinationAddress, int64_t fee = BtcHelper::FeeMultiSig, const std::string &redeemScript = "", const std::string &signingAddress = "") = 0;

    // combines concatenated raw transactions to one raw transaction
    // used to combine the partially signed raw tx's when releasing multi-sig
    virtual BtcSignedTransactionPtr CombineTransactions(const std::string &concatenatedRawTransactions) = 0;

    // Broadcasts a raw transaction to the network
    // returns txId or ""
    virtual std::string SendRawTransaction(const std::string &rawTransaction) = 0;

    /*
        std::map<std::string, int64_t> GetAddressesAndBalances() = 0;
        int64_t GetBalanceAtAddress(const std::string & str_address) = 0;
        not implemented yet, but bitcoin now offers a nice API function to get that, so if we really need it I can add it almost instantly
        but it does not work for multi-sig

        int64_t GetMultisigBalance(  ) = 0;
        not yet implemented, would be nice if it isn't needed.
        instead we could/should keep track of transaction Ids and their value to know the balance of multi-sig addresses
        and clients should tell the server what the tx id is
        but if needed i can create a function to crawl through the blockchain. will be very CPU intensive but we could tweak it to only go back X days or something like that.

    */
};

#ifndef OT_USE_TR1
    typedef std::shared_ptr<IMTBitcoin> IMTBitcoinPtr;
#else
    typedef std::tr1::shared_ptr<IMTBitcoin> IMTBitcoinPtr;
#endif // OT_USE_TR1


#endif // IMTBITCOIN_HPP
