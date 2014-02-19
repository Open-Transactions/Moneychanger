#ifndef SAMPLEESCROWCLIENT_H
#define SAMPLEESCROWCLIENT_H

#include "btcobjects.h"
#include "btcmodules.h"
#include "widgets/btcguitest.h"
//#include "sampletypedefs.h"
//#include "sampleescrowserver.h"
//#include "sampleescrowtransaction.h"
#include "poolmanager.h"


class SampleEscrowClient : public QObject
{
        Q_OBJECT
public:
    explicit SampleEscrowClient(QObject* parent = 0);
    ~SampleEscrowClient();


    /** Deposit **/
    // client asks servers for public keys
    // amountToSend: amount in satoshis
    void StartDeposit(int64_t amountToSend, EscrowPoolPtr targetPool);

    // client receives public keys
    void OnReceivePubKey(const std::string& publicKey, int minSignatures);

    // returns true when the transaction finished
    bool CheckDepositFinished();

    /** Release **/
    // client asks server for release of escrow
    void StartWithdrawal();

    // client receives partially signed tx
    void OnReceiveSignedTx(const std::string &signedTx);

    bool CheckWithdrawalFinished();

    bool CheckTransactionFinished(SampleEscrowTransactionPtr transaction);


    std::list<std::string> pubKeyList;
    int minSignatures;

    int minConfirms;  // minimum number of confirmations before tx is considered successfull

    SampleEscrowTransactionPtr transactionDeposit;      // holds info about the transaction to the pool
    SampleEscrowTransactionPtr transactionWithdrawal;   // holds info about the transaction from the pool

private:
    BitcoinServerPtr rpcServer;
    EscrowPoolPtr targetPool;

    BtcModules* modules;

    // client sends bitcoin to escrow
    void SendToEscrow();

    signals:    // used to update the GUI
        // deposit
        void SetMultiSigAddress(const std::string& address);
        void SetTxIdDeposit(const std::string& txId);
        void SetConfirmationsDeposit(int confirms);
        void SetStatusDeposit(SampleEscrowTransaction::SUCCESS status);
        // withdrawal
        void SetWithdrawalAddress(const std::string& address);
        void SetTxIdWithdrawal(const std::string& txId);
        void SetConfirmationsWithdrawal(int confirms);
        void SetStatusWithdrawal(SampleEscrowTransaction::SUCCESS status);
};

#ifndef OT_USE_TR1
    typedef std::shared_ptr<SampleEscrowClient> SampleEscrowClientPtr;
#else
    typedef std::tr1::shared_ptr<SampleEscrowClient> SampleEscrowClientPtr;
#endif // OT_USE_TR1




#endif // SAMPLEESCROWCLIENT_H
