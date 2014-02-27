#ifndef MODULES_HPP
#define MODULES_HPP

#include <WinsockWrapper.h>
#include <ExportWrapper.h>

#include <QScopedPointer>

// This class will hold pointers to various modules so they can access eachother.
// Hierarchic layout would be possible too: BtcInterface -> BtcJson -> BtcRpc
class SampleEscrowManager;
class PoolManager; // has a list of all available pools
class TransactionManager; // has a list of pending and finished transactions
//class MTBitcoin;

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

#endif // MODULES_HPP
