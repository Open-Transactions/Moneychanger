#ifndef SAMPLEESCROWSERVER_HPP
#define SAMPLEESCROWSERVER_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/TR1_Wrapper.hpp>

#include <bitcoin/escrowpool.hpp>
#include <bitcoin/sampleescrowtransaction.hpp>

#include <QObject>
#include <QThread>

#include _CINTTYPES
#include _MEMORY

#include <map>

class QTimer;
class QMutex;

class SampleEscrowClient;
typedef _SharedPtr<SampleEscrowClient> SampleEscrowClientPtr;

class CheckTxDaemon : public QObject
{
    Q_OBJECT

public:
    CheckTxDaemon(SampleEscrowServer* master, QObject* parent = NULL);
public slots:
    void StartDaemon();

private:
    SampleEscrowServer* master;
};

#include <map>

class SampleEscrowServer : public QThread
{
    friend class CheckTxDaemon;
    Q_OBJECT
public:
    SampleEscrowServer(BitcoinServerPtr rpcServer, EscrowPoolPtr pool, QObject* parent = NULL);
    ~SampleEscrowServer();

    virtual bool ClientConnected(SampleEscrowClient* client);

    // called when someone wants to make a deposit
    virtual bool RequestEscrowDeposit(const std::string &client, const int64_t &amount);

    // returns the multisig address to deposit to
    virtual std::string RequestDepositAddress(const std::string &client);

    // called by other servers in the pool so they get eachothers' public keys for multi-sig
    virtual std::string GetPubKey(const std::string &client);

    virtual int64_t GetClientBalance(const std::string& client);

    virtual u_int64_t GetClientTransactionCount(const std::string &client);

    virtual SampleEscrowTransactionPtr GetClientTransaction(const std::string &client, u_int64_t txIndex);

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

    btc::stringList multiSigAddresses;

    bool isClient;
    bool shutDown;

protected:
    void InitializeClient(const std::string& client);
    void AddClientDeposit(const std::string &client, SampleEscrowTransactionPtr transaction, bool oldTx);
    void RemoveClientDeposit(const std::string &client, SampleEscrowTransactionPtr transaction);
    SampleEscrowTransactionPtr FindClientTransaction(const std::string &targetAddress, const std::string &txId, const std::string &client);
    BtcUnspentOutputs GetOutputsToSpend(const std::string &client, const int64_t &amountToSpend);

    struct ClientRequest;
    typedef _SharedPtr<ClientRequest> ClientRequestPtr;
    typedef std::list<ClientRequestPtr> ClientRequests;
    ClientRequests clientRequests;

    BtcModulesPtr modules;

private:
    virtual std::string CreatePubKey(const std::string &client);
    virtual void AddPubKey(const std::string &client, const std::string &key);

    BitcoinServerPtr rpcServer;     // login info for bitcoin-qt rpc

    int32_t minSignatures;          // minimum required signatures

    int minConfirms;            // minimum required confirmations

    std::map<std::string, SampleEscrowClientPtr> clientList;
    typedef std::map<std::string, SampleEscrowTransactions> ClientBalanceMap;
    ClientBalanceMap clientBalancesMap;  // list of deposit transactions
    ClientBalanceMap clientHistoryMap;     // list of all transactions

    QMutex* mutex;

public slots:
    virtual void Update();
    void StartServerLoop();
    void CheckTransactions();
};

typedef _SharedPtr<SampleEscrowServer> SampleEscrowServerPtr;


#endif // SAMPLEESCROWSERVER_HPP
