#ifndef TRANSACTIONMANAGER_H
#define TRANSACTIONMANAGER_H

#include <QSharedPointer>
#include <QList>
#include "sampleescrowtransaction.h"


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

#endif // TRANSACTIONMANAGER_H
