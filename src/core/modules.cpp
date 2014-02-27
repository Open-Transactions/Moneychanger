#ifndef STABLE_H
#include <core/stable.hpp>
#endif

#include "modules.h"


QScopedPointer<SampleEscrowManager> Modules::sampleEscrowManager;
QScopedPointer<PoolManager> Modules::poolManager;
QScopedPointer<TransactionManager> Modules::transactionManager;
//QScopedPointer<MTBitcoin> Modules::mtBitcoin;


Modules::Modules()
{
    /*
    Modules::btcRpcQt.reset(new BtcRpc());
    Modules::btcJsonQt.reset(new BtcJsonQt());
    Modules::btcInterface.reset(new BtcInterface());
    */
    Modules::sampleEscrowManager.reset(new SampleEscrowManager());
    Modules::poolManager.reset(new PoolManager());
    Modules::transactionManager.reset(new TransactionManager());
    //Modules::mtBitcoin.reset(new MTBitcoin());
    //btcJsonQt->Initialize();
}

Modules::~Modules()
{

}
