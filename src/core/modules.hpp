#ifndef MODULES_HPP
#define MODULES_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/TR1_Wrapper.hpp>

#include <QScopedPointer>

// This class will hold pointers to various modules so they can access eachother.
// Hierarchic layout would be possible too: BtcInterface -> BtcJson -> BtcRpc
class SampleEscrowManager;
class PoolManager; // has a list of all available pools
class TransactionManager; // has a list of pending and finished transactions
class BtcConnectionManager;
class SampleEscrowClient;
class BtcModules;
//class MTBitcoin;

class Modules
{
public:
    Modules();
    ~Modules();

    // TODO: _maybe_ overload the :: operator to check if the pointer isn't NULL
    // and maybe use QSharedPointer?
    static _SharedPtr<SampleEscrowManager> sampleEscrowManager;
    static _SharedPtr<PoolManager> poolManager;
    static _SharedPtr<TransactionManager> transactionManager;
    static _SharedPtr<BtcConnectionManager> connectionManager;
    static _SharedPtr<SampleEscrowClient> sampleEscrowClient;
    static _SharedPtr<BtcModules> btcModules;
    //static QScopedPointer<MTBitcoin> mtBitcoin;
};

#endif // MODULES_HPP
