#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/modules.hpp>

#include "core/TR1_Wrapper.hpp"

#include <bitcoin/sampleescrowmanager.hpp>
#include <bitcoin/poolmanager.hpp>
#include <bitcoin/transactionmanager.hpp>
#include <bitcoin/sampleescrowclient.hpp>
#include <bitcoin-api/btcmodules.hpp>
#include <gui/widgets/btcconnectdlg.hpp>
#include <gui/widgets/btcwalletpwdlg.hpp>


// "SampleEscrowManager" is causing some kind of Crash in Moneychanger
// So for now, I'm just commenting it out. It's used by the internal Bitcoin
// wallet and we don't use that currently anyway.
//
_SharedPtr<SampleEscrowManager> Modules::sampleEscrowManager;
_SharedPtr<PoolManager> Modules::poolManager;
_SharedPtr<TransactionManager> Modules::transactionManager;
QPointer<BtcConnectDlg> Modules::connectionManager;
_SharedPtr<SampleEscrowClient> Modules::sampleEscrowClient;
_SharedPtr<BtcModules> Modules::btcModules;
BtcWalletPwDlgPtr Modules::walletPwDlg;

bool Modules::shutDown = false;


Modules::Modules()
{
    sampleEscrowManager.reset(new SampleEscrowManager());
    poolManager.reset(new PoolManager());
    transactionManager.reset(new TransactionManager());
    connectionManager = new BtcConnectDlg();
    walletPwDlg.reset(new BtcWalletPwDlg());
    btcModules.reset(new BtcModules());
    sampleEscrowClient.reset(new SampleEscrowClient(btcModules));

    shutDown = false;

    btcModules->btcJson->SetPasswordCallback(fastdelegate::MakeDelegate(walletPwDlg.get(), &BtcWalletPwDlg::WaitForPassword));
}

Modules::~Modules()
{

}
