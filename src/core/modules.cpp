#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/modules.hpp>

#include <bitcoin/sampleescrowmanager.hpp>
#include <bitcoin/poolmanager.hpp>
#include <bitcoin/transactionmanager.hpp>
#include <bitcoin/sampleescrowclient.hpp>
#include <bitcoin-api/btcmodules.hpp>



_SharedPtr<SampleEscrowManager> Modules::sampleEscrowManager;
_SharedPtr<PoolManager> Modules::poolManager;
_SharedPtr<TransactionManager> Modules::transactionManager;
_SharedPtr<SampleEscrowClient> Modules::sampleEscrowClient;
_SharedPtr<BtcModules> Modules::btcModules;


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
    Modules::sampleEscrowClient.reset(new SampleEscrowClient(btcModules));
    //Modules::mtBitcoin.reset(new MTBitcoin());
    //btcJsonQt->Initialize();
}

Modules::~Modules()
{

}
