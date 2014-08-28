#ifndef MODULES_HPP
#define MODULES_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/TR1_Wrapper.hpp>

#include <QScopedPointer>
#include <QPointer>

// This class will hold pointers to various modules so they can access eachother.
// Hierarchic layout would be possible too: BtcInterface -> BtcJson -> BtcRpc
class SampleEscrowManager;
class PoolManager; // has a list of all available pools
class TransactionManager; // has a list of pending and finished transactions
class BtcConnectDlg;
class SampleEscrowClient;
class BtcWalletPwDlg;

class BtcModules;


class Modules
{
public:
    Modules();
    ~Modules();

    static _SharedPtr<SampleEscrowManager> sampleEscrowManager;
    static _SharedPtr<PoolManager> poolManager;
    static _SharedPtr<TransactionManager> transactionManager;
    static  QPointer<BtcConnectDlg> connectionManager;
    static _SharedPtr<BtcWalletPwDlg> walletPwDlg;
    static _SharedPtr<SampleEscrowClient> sampleEscrowClient;
    static _SharedPtr<BtcModules> btcModules;

    static bool shutDown;

};

#endif // MODULES_HPP
