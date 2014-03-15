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

class SampleEscrowServer
{
public:
    SampleEscrowServer(BitcoinServerPtr rpcServer);

    // called when someone wants to make a deposit
    void OnRequestEscrowDeposit(SampleEscrowClient* client);

    // called by other servers in the pool so they get eachothers' public keys for multi-sig
    std::string GetMultiSigPubKey();

    // called when the client tells the server the tx id of the incoming transaction
    void OnIncomingDeposit(std::string txId);

    bool CheckIncomingTransaction();

    void OnRequestEscrowWithdrawal(SampleEscrowClient* client);

    EscrowPoolPtr serverPool;           // the pool that this server is part of

    std::string addressForMultiSig;     // this server's address used to create the multi-sig

    std::string pubKeyForMultiSig;      // public key of the address used to create the multi-sig

    std::list<std::string> publicKeys;  // public keys of this and all servers to create multi-sig

    std::string multiSigAddress;        // the actual multisig address

    BtcMultiSigAddressPtr multiSigAddrInfo; // info required to withdraw from the address

    SampleEscrowTransactionPtr transactionDeposit;      // info about deposit
    SampleEscrowTransactionPtr transactionWithdrawal;   // info about withdrawal


private:
    BitcoinServerPtr rpcServer;     // login info for bitcoin-qt rpc

    BtcModules* modules;

    int minSignatures;          // minimum required signatures

    int minConfirms;            // minimum required confirmations
};

typedef _SharedPtr<SampleEscrowServer> SampleEscrowServerPtr;


#endif // SAMPLEESCROWSERVER_HPP
