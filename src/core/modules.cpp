#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/modules.hpp>

#include <bitcoin/sampleescrowmanager.hpp>
#include <bitcoin/poolmanager.hpp>
#include <bitcoin/transactionmanager.hpp>



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
