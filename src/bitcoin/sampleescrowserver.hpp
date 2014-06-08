#ifndef SAMPLEESCROWSERVER_HPP
#define SAMPLEESCROWSERVER_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/TR1_Wrapper.hpp>

#include <bitcoin/escrowpool.hpp>
#include <bitcoin/sampleescrowtransaction.hpp>

#include <QObject>

#include _CINTTYPES
#include _MEMORY

#include <map>

class SampleEscrowClient;
class QTimer;
class QMutex;

typedef _SharedPtr<SampleEscrowClient> SampleEscrowClientPtr;

#include <map>

class SampleEscrowServer : public QObject
{
    Q_OBJECT
public:
    SampleEscrowServer(BitcoinServerPtr rpcServer, QObject* parent = NULL);
    ~SampleEscrowServer();

    virtual void ClientConnected(SampleEscrowClient* client);

    // called when someone wants to make a deposit
    virtual bool RequestEscrowDeposit(const std::string &client, const int64_t &amount);

    // returns the multisig address to deposit to
    virtual std::string RequestDepositAddress(const std::string &client);

    virtual std::string CreatePubKey(const std::string &client);

    // called by other servers in the pool so they get eachothers' public keys for multi-sig
    virtual std::string GetPubKey(const std::string &client);

    virtual void AddPubKey(const std::string &client, const std::string &key);

    //virtual void OnReceivePubKey(const std::string &sender, const std::string &key);

    virtual bool RequestEscrowWithdrawal(const std::string &client, const int64_t &amount, const std::string &toAddress);

    // called from server to server
    // returns a partially signed raw transaction
    virtual std::string RequestSignedWithdrawal(const std::string &client);

    std::string serverName;             // name of this server

    EscrowPoolPtr serverPool;           // the pool that this server is part of

    // maps clients to the address the server created for this client
    typedef std::map<std::string, std::string> ClientAddressMap;
    ClientAddressMap addressForMultiSig;     // this server's address used to create the multi-sig

    // maps clients to the public key the server created for this client
    typedef std::map<std::string, std::string> ClientKeyMap;
    ClientKeyMap pubKeyForMultiSig;      // public key of the address used to create the multi-sig

    // maps client ids to lists of public keys,
    // each client pays to a multisig made from those keys
    typedef std::map<std::string, btc::stringList> ClientKeyListMap;
    ClientKeyListMap publicKeys;  // public keys of this and all servers to create multi-sig

    // maps clients to the current multisig address for this client
    typedef std::map<std::string, std::string> ClientMultiSigMap;
    ClientMultiSigMap multiSigAddress;        // the actual multisig address

    typedef std::map<std::string, BtcSignedTransactionPtr> ClientReleaseTxMap;
    ClientReleaseTxMap clientReleaseTxMap;

    typedef std::map<std::string, std::string> AddressClientMap;
    AddressClientMap addressToClientMap;    // maps clients to their multisig addresses

    //BtcMultiSigAddressPtr multiSigAddrInfo; // info required to withdraw from the address

    //SampleEscrowTransactionPtr transactionDeposit;      // info about deposit
    //SampleEscrowTransactionPtr transactionWithdrawal;   // info about withdrawal

protected:
    void Initialize(const std::string& client);
    void AddClientDeposit(const std::string &client, SampleEscrowTransactionPtr transaction);
    void RemoveClientDeposit(const std::string &client, SampleEscrowTransactionPtr transaction);
    void CheckTransactions();
    SampleEscrowTransactionPtr FindClientTransaction(const std::string &targetAddress, const std::string &txId, const std::string &client);
    int64_t GetClientBalance(const std::string& client);
    BtcUnspentOutputs GetOutputsToSpend(const std::string &client, const int64_t &amountToSpend);

    struct ClientRequest;
    typedef _SharedPtr<ClientRequest> ClientRequestPtr;
    typedef std::list<ClientRequestPtr> ClientRequests;
    ClientRequests clientRequests;

private:
    BitcoinServerPtr rpcServer;     // login info for bitcoin-qt rpc

    BtcModulesPtr modules;

    int minSignatures;          // minimum required signatures

    int minConfirms;            // minimum required confirmations

    std::map<std::string, SampleEscrowClientPtr> clientList;
    typedef std::map<std::string, SampleEscrowTransactions> ClientBalanceMap;
    ClientBalanceMap clientBalances;  // list of transactions

    QTimer* updateTimer;
    QMutex* mutex;

public slots:
    void Update();
};

typedef _SharedPtr<SampleEscrowServer> SampleEscrowServerPtr;


#endif // SAMPLEESCROWSERVER_HPP
