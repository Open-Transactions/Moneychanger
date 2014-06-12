#ifndef TRANSACTIONMANAGER_HPP
#define TRANSACTIONMANAGER_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <bitcoin/sampletypedefs.hpp>

#include <QList>
#include <QMap>


// TransactionManager will watch pending transactions in the background and notify on event
// currently it isn't used
class TransactionManager
{
public:
    TransactionManager();

    void AddTransaction(const std::string &clientName, SampleEscrowTransactionPtr transaction);

    void RemoveTransaction(const std::string &clientName, SampleEscrowTransactionPtr transaction);

    EscrowTransactionList FindTransaction(const std::string &txId, const std::string targetAddress);

private:
    // maps client names to transactions
    typedef QMap<std::string, EscrowTransactionList> ClientTransactionMap;
    ClientTransactionMap clientTransactionMap;
};

#endif // TRANSACTIONMANAGER_HPP
