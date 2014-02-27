#ifndef MODULES_H
#define MODULES_H

#include <QScopedPointer>
#include "sampleescrowmanager.h"
#include "poolmanager.h"            // has a list of all available pools
#include "transactionmanager.h"     // has a list of pending and finished transactions
//#include "MTBitcoin.h"

// This class will hold pointers to various modules so they can access eachother.
// Hierarchic layout would be possible too: BtcInterface -> BtcJson -> BtcRpc
class Modules
{
public:
    Modules();
    ~Modules();

    // TODO: _maybe_ overload the :: operator to check if the pointer isn't NULL
    // and maybe use QSharedPointer?
    static QScopedPointer<SampleEscrowManager> sampleEscrowManager;
    static QScopedPointer<PoolManager> poolManager;
    static QScopedPointer<TransactionManager> transactionManager;
    //static QScopedPointer<MTBitcoin> mtBitcoin;
};

#endif // MODULES_H
