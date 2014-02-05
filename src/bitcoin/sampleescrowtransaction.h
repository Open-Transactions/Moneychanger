#ifndef SAMPLEESCROWTRANSACTION_H
#define SAMPLEESCROWTRANSACTION_H

#include <string>
#include "btcobjects.h"
#include "btcmodules.h"
#include <tr1/memory>
//#include "sampleescrowclient.h"
//#include "sampleescrowserver.h"


// this class holds some information about pending transactions
// to save/restore pending transactions we need txId, targetAddr and amountToSend
class SampleEscrowTransaction
{
public:
    SampleEscrowTransaction(int64_t amountToSend);

    bool SendToTarget();

    // creates a raw transaction to send funds from source tx and address to target address
    // returns true if signing is complete
    bool CreateWithdrawalTransaction(const std::string &sourceTxId, const std::string &multiSigSourceAddress, const std::string &targetAddr);

    // combines raw transactions and their signatures
    // returns true if signing is complete
    bool AddWithdrawalTransaction(const std::string& partiallySignedTx);

    bool SendWithdrawalTransaction();

    // checks if transaction is confirmed and sends the correct value to the correct address
    void CheckTransaction(int minConfirms);

    int64_t amountToSend;
    std::string targetAddr;             // address to which funds are sent
    std::string txId;                   // id of the pending transaction
    int64_t confirmations;              // confirmations of pending transaction

    std::string sourceTxId;             // transaction id from which to withdraw the outputs
    std::string withdrawalTransaction;  // a (partially) signed raw transaction to withdraw funds from sourceTxId to targetAddr

    enum SUCCESS
    {
        NotStarted,
        Pending,
        Successfull,
        Failed
    }status;

    //SampleEscrowClientPtr client;

private:
    BtcModules* modules;
    //QList<SampleEscrowServerPtr> servers;
};

#ifndef OT_USE_TR1
    typedef std::shared_ptr<SampleEscrowTransaction> SampleEscrowTransactionPtr;
#else
    typedef std::tr1::shared_ptr<SampleEscrowTransaction> SampleEscrowTransactionPtr;
#endif // OT_USE_TR1

#endif // SAMPLEESCROWTRANSACTION_H
