#ifndef TRANSACTIONMANAGER_HPP
#define TRANSACTIONMANAGER_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <bitcoin/sampleescrowtransaction.hpp>

#include <QList>


// TransactionManager will watch pending transactions in the background and notify on event
class TransactionManager
{
public:
    TransactionManager();

    void AddTransaction(SampleEscrowTransactionPtr transaction);

    void RemoveTransaction(SampleEscrowTransactionPtr transaction);

private:
    // not started, pending and finished transactions
    QList<SampleEscrowTransactionPtr> transactions;
};

#endif // TRANSACTIONMANAGER_HPP
