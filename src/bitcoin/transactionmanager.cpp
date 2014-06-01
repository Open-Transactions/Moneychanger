#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin/transactionmanager.hpp>
#include <bitcoin/sampleescrowtransaction.hpp>

#include <string>


TransactionManager::TransactionManager()
{
    this->clientTransactionMap = ClientTransactionMap();
}

void TransactionManager::AddTransaction(const std::string &clientName, SampleEscrowTransactionPtr transaction)
{
    if(transaction == NULL)
        return;

    if(!FindTransaction(transaction->txId, transaction->targetAddr).empty())
        return;

    ClientTransactionMap::iterator clientList = this->clientTransactionMap.find(clientName);
    if(clientList == this->clientTransactionMap.end())
        clientList = this->clientTransactionMap.insert(clientName, EscrowTransactionList());

    clientList.value().push_back(transaction);
}

void TransactionManager::RemoveTransaction(const std::string &clientName, SampleEscrowTransactionPtr transaction)
{
    if(clientName.empty() || transaction == NULL)
        return;

    ClientTransactionMap::iterator transactionList = this->clientTransactionMap.find(clientName);
    if(transactionList == this->clientTransactionMap.end())
        return;
    foreach(SampleEscrowTransactionPtr tx, transactionList.value())
    {
        if(tx->txId == transaction->txId && tx->targetAddr == transaction->targetAddr)
        {
            transactionList.value().remove(tx);
            break;
        }
    }
}

EscrowTransactionList TransactionManager::FindTransaction(const std::string &txId, const std::string targetAddress)
{
    EscrowTransactionList outputs = EscrowTransactionList();
    foreach(EscrowTransactionList list, this->clientTransactionMap)
    {
        foreach(SampleEscrowTransactionPtr tx, list)
        {
            if(tx->txId == txId && tx->targetAddr == targetAddress)
                outputs.push_back(tx);
        }
    }

    return EscrowTransactionList();
}
