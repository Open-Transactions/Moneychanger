#ifndef SAMPLEESCROWSERVER_HPP
#define SAMPLEESCROWSERVER_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/TR1_Wrapper.hpp>

#include <bitcoin/escrowpool.hpp>
#include <bitcoin/sampleescrowtransaction.hpp>

#include _CINTTYPES
#include _MEMORY

#include <map>

class SampleEscrowClient;

typedef _SharedPtr<SampleEscrowClient> SampleEscrowClientPtr;

#include <map>

class SampleEscrowServer
{
public:
    SampleEscrowServer(BitcoinServerPtr rpcServer);

    virtual void OnClientConnected(SampleEscrowClient* client);

    // called when someone wants to make a deposit
    virtual std::string OnRequestEscrowDeposit(const std::string &sender, int64_t amount);

    // called by other servers in the pool so they get eachothers' public keys for multi-sig
    virtual std::string GetMultiSigPubKey();

    virtual std::string OnGetPubKey(const std::string &sender);

    virtual void AddPubKey(const std::string &sender, const std::string &key);

    //virtual void OnReceivePubKey(const std::string &sender, const std::string &key);

    // called when the client tells the server the tx id of the incoming transaction
    virtual void OnIncomingDeposit(const std::string sender, const std::string txId, int64_t amount);

    virtual bool CheckTransaction(const std::string &sender, const std::string txId, const std::string &targetAddress);

    virtual std::string OnRequestEscrowWithdrawal(const std::string &sender, const std::string &txId, const std::string &fromAddress, const std::string &toAddress);

    // we get notified that a client's deposit is being released
    virtual void ReleasingFunds(const std::string &client, const std::string &txId, const std::string &sourceTxId, const std::string &sourceAddress, const std::string &toAddress);

    std::string serverName;             // name of this server

    EscrowPoolPtr serverPool;           // the pool that this server is part of

    std::string addressForMultiSig;     // this server's address used to create the multi-sig

    std::string pubKeyForMultiSig;      // public key of the address used to create the multi-sig

    std::list<std::string> publicKeys;  // public keys of this and all servers to create multi-sig

    std::string multiSigAddress;        // the actual multisig address

    BtcMultiSigAddressPtr multiSigAddrInfo; // info required to withdraw from the address

    //SampleEscrowTransactionPtr transactionDeposit;      // info about deposit
    //SampleEscrowTransactionPtr transactionWithdrawal;   // info about withdrawal

protected:
    void AddClientDeposit(const std::string &client, SampleEscrowTransactionPtr transaction);
    void RemoveClientDeposit(const std::string &client, SampleEscrowTransactionPtr transaction);
    SampleEscrowTransactionPtr FindClientTransaction(const std::string &targetAddress, const std::string &txId, const std::string &client);
    SampleEscrowTransactionPtr FindClientTransaction(const std::string &targetAddress, const std::string txId);

private:
    BitcoinServerPtr rpcServer;     // login info for bitcoin-qt rpc

    BtcModules* modules;

    int minSignatures;          // minimum required signatures

    int minConfirms;            // minimum required confirmations

    std::map<std::string, SampleEscrowClientPtr> clientList;
    typedef std::map<std::string, std::list<SampleEscrowTransactionPtr> > BalanceMap;
    BalanceMap clientBalances;  // list of transactions
};

typedef _SharedPtr<SampleEscrowServer> SampleEscrowServerPtr;


#endif // SAMPLEESCROWSERVER_HPP
