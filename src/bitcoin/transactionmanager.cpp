#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin/transactionmanager.hpp>


TransactionManager::TransactionManager()
{
    this->transactions = QList<SampleEscrowTransactionPtr>();
}

void TransactionManager::AddTransaction(SampleEscrowTransactionPtr transaction)
{
    if(transaction == NULL || this->transactions.contains(transaction))
        return;

    this->transactions.append(transaction);
}

void TransactionManager::RemoveTransaction(SampleEscrowTransactionPtr transaction)
{
    this->transactions.removeAll(transaction);
}
