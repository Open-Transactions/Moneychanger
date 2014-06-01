#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/modules.hpp>

#include <bitcoin/sampleescrowmanager.hpp>
#include <bitcoin/poolmanager.hpp>
#include <bitcoin/transactionmanager.hpp>
#include <bitcoin-api/btcmodules.hpp>



QScopedPointer<SampleEscrowManager> Modules::sampleEscrowManager;
QScopedPointer<PoolManager> Modules::poolManager;
QScopedPointer<TransactionManager> Modules::transactionManager;
QScopedPointer<BtcModules> Modules::btcModules;


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
    Modules::btcModules.reset(new BtcModules());
    //Modules::mtBitcoin.reset(new MTBitcoin());
    //btcJsonQt->Initialize();
}

Modules::~Modules()
{

}
