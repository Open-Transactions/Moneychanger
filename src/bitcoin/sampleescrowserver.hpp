#ifndef SAMPLEESCROWSERVER_HPP
#define SAMPLEESCROWSERVER_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/MemoryWrapper.hpp>

#include <bitcoin/escrowpool.hpp>
#include <bitcoin/sampleescrowtransaction.hpp>

#include <map>

class SampleEscrowClient;

#ifndef OT_USE_TR1
typedef std::shared_ptr<SampleEscrowClient> SampleEscrowClientPtr;
#else
typedef std::tr1::shared_ptr<SampleEscrowClient> SampleEscrowClientPtr;
#endif // OT_USE_TR1

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

#ifndef OT_USE_TR1
    typedef std::shared_ptr<SampleEscrowServer> SampleEscrowServerPtr;
#else
    typedef std::tr1::shared_ptr<SampleEscrowServer> SampleEscrowServerPtr;
#endif // OT_USE_TR1


#endif // SAMPLEESCROWSERVER_HPP
