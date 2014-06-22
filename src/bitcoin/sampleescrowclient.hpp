#ifndef SAMPLEESCROWCLIENT_HPP
#define SAMPLEESCROWCLIENT_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/TR1_Wrapper.hpp>

#include <bitcoin/sampleescrowtransaction.hpp>
#include <bitcoin/escrowpool.hpp>

#include _CINTTYPES
#include _MEMORY

#include <QObject>

class QTimer;

class SampleEscrowClient : public QObject
{
        Q_OBJECT
public:
    explicit SampleEscrowClient(QObject* parent = 0);
    SampleEscrowClient(BitcoinServerPtr rpcServer, QObject *parent = NULL);
    SampleEscrowClient(BtcModulesPtr modules, QObject *parent = NULL);
    ~SampleEscrowClient();

    virtual void Initialize();

    virtual void Reset();

    /** Deposit **/
    // client asks servers for public keys
    // amountToSend: amount in satoshis
    void StartDeposit(int64_t amountToSend, EscrowPoolPtr targetPool);

    /** Release **/
    // client asks server for release of escrow
    void StartWithdrawal(const int64_t &amountToWithdraw, const std::string &toAddress, EscrowPoolPtr fromPool);

    /** Balance **/
    virtual void CheckPoolBalance(EscrowPoolPtr pool);
    virtual void CheckPoolTransactions(EscrowPoolPtr pool);

    typedef std::map<std::string, std::string> PoolAddressMap;
    PoolAddressMap poolAddressMap;
    typedef std::map<std::string, int64_t> PoolBalanceMap;
    PoolBalanceMap poolBalanceMap;    
    typedef std::map<std::string, SampleEscrowTransactions> PoolTxMap;
    PoolTxMap poolTxMap;
    typedef std::map<std::string, u_int64_t> PoolTxCountMap;
    PoolTxCountMap poolTxCountMap;

    std::string clientName;


protected:
    BitcoinServerPtr rpcServer;

    BtcModulesPtr modules;

    _SharedPtr<QTimer> updateTimer;

private:
    struct Action;
    typedef _SharedPtr<Action> ActionPtr;
    typedef std::list<ActionPtr> Actions;
    Actions actionsToDo;

    virtual void ContactServer(ActionPtr action);
    virtual void RequestDeposit(ActionPtr action);
    virtual void AskForDepositAddress(ActionPtr action);
    virtual void SendToEscrow(ActionPtr action);
    virtual void RequestRelease(ActionPtr action);
    virtual void CheckBalance(ActionPtr action);
    virtual void GetPoolTxCount(ActionPtr action);
    virtual void FetchPoolTx(ActionPtr action);

    void InitializePool(EscrowPoolPtr pool);


    int minConfirms;  // minimum number of confirmations before tx is considered successfull

public slots:
    void Update();
};

typedef _SharedPtr<SampleEscrowClient> SampleEscrowClientPtr;


// random name generator
void gen_random(char *s, const int len);



#endif // SAMPLEESCROWCLIENT_HPP
