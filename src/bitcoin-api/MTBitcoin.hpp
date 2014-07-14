#ifndef MTBITCOIN_HPP
#define MTBITCOIN_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/TR1_Wrapper.hpp>

#include <bitcoin-api/imtbitcoin.hpp>
#include <bitcoin-api/btcobjects.hpp>
#include <bitcoin-api/btcmodules.hpp>

#include _CINTTYPES
#include _MEMORY

#include <map>
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


class MTBitcoin : public IMTBitcoin
{
public:
    MTBitcoin(BtcModules* modules);
    ~MTBitcoin();


    // account (optional): the account whose balance should be checked
    // returns total balance of all addresses.
    virtual int64_t GetBalance(const std::string &account = "*");

    // account (optional): the account to which the address should be added.
    // returns new address
    virtual std::string GetNewAddress(const std::string &account = "moneychanger");

    // address: watchonly address to import
    // label:   the name of the address and its accounts
    // rescan:  rescan the entire blockchain for old transactions
    // Can be called multiple times to change the label
    virtual bool ImportAddress(const std::string &address, const std::string &label = "watchonly", bool rescan = false);

    // returns public key for address (works only if public key is known)
    virtual std::string GetPublicKey(const std::string &address);

    // Creates a multi-sig address from public keys
    // returns the address string
    virtual std::string GetMultiSigAddress(int32_t minSignatures, const btc::stringList& publicKeys, bool addToWallet = true, const std::string &account = NULL);

    // Creates a multi-sig address from public keys
    // returns an object containing info required to withdraw from that address
    virtual BtcMultiSigAddressPtr GetMultiSigAddressInfo(int32_t minSignatures, const btc::stringList& publicKeys, bool addToWallet = true, const std::string &account = NULL);

    // Returns information about a transaction
    // Only works for your own and imported watchonly addresses
    virtual BtcTransactionPtr GetTransaction(const std::string &txId);

    // Returns an object containing information about a raw transaction
    virtual BtcRawTransactionPtr GetRawTransaction(const std::string &txId);

    // Returns an object containing information about a raw transaction
    // times out after 10 seconds
    virtual BtcRawTransactionPtr WaitGetRawTransaction(const std::string &txId);

    // Returns the number of confirmations of a raw transaction
    virtual int32_t GetConfirmations(const std::string &txId);

    virtual bool TransactionSuccessful(const int64_t &amount, BtcRawTransactionPtr rawTransaction, const std::string &targetAddress, const int32_t &confirmations = BtcHelper::WaitForConfirms);

    // sends funds from your wallet to targetAddress
    // returns the transaction id string or NULL
    virtual std::string SendToAddress(const std::string &to_address, int64_t lAmount);

    // Creates a multi-sig address using the public keys (not addresses, unless their public keys are known to bitcoin-qt!)
    // and sends bitcoin to that address.
    // If the multisig address is already known, we can instead just call SendToAddress(multiSigAddress)
    // as bitcoin makes no difference between those and regular addresses.
    // returns the transaction id string or NULL
    virtual std::string SendToMultisig(int64_t lAmount, int nRequired, const btc::stringList &to_publicKeys);

    // creates a partially signed raw transaction to withdraw funds
    // txToSourceId: transaction id that was used to fund the pool
    // sourceAddress: used to know which outputs of the tx we can spend (in case it came from a SendMany() tx)
    // redeemScript [optional]: is passed to the API to do some bitcoin magic. required if unknown or if passing a signingAddress
    // signingAddress [optional]: only this address's private key will be used to sign the transaction (kinda pointless in our case)
    virtual BtcSignedTransactionPtr VoteMultiSigRelease(const std::string &txToSourceId, const std::string &sourceAddress, const std::string &destinationAddress, int64_t fee = BtcHelper::FeeMultiSig, const std::string &redeemScript = "", const std::string &signingAddress = "");

    // combines concatenated raw transactions to one raw transaction
    // used to combine the partially signed raw tx's when releasing multi-sig
    virtual BtcSignedTransactionPtr CombineTransactions(const std::string &concatenatedRawTransactions);

    virtual std::string SendRawTransaction(const std::string &rawTransaction);

    // Lists unspent outputs, including watchonly addresses
    // addresses [optional]: limit results to addresses
    virtual BtcUnspentOutputs ListUnspentOutputs(const btc::stringList &addresses = btc::stringList());

    /*
        std::map<std::string, int64_t> GetAddressesAndBalances();
        int64_t GetBalanceAtAddress(const std::string & str_address);
        not implemented yet, but bitcoin now offers a nice API function to get that, so if we really need it I can add it almost instantly

        int64_t GetMultisigBalance(  );
        not yet implemented, would be nice if it isn't needed.
        instead we could/should keep track of transaction Ids and their value to know the balance of multi-sig addresses
        and clients should tell the server what the tx id is
    */

private:
    BtcModules* modules;
};

typedef _SharedPtr<MTBitcoin> MTBitcoinPtr;

#endif // MTBITCOIN_HPP
