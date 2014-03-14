#ifndef BTCTEST_HPP
#define BTCTEST_HPP

#include <bitcoin-api/btcmodules.hpp>

// will be used as a sort of unit test
// currently only checks the most important things.
class BtcTest
{
public:
    BtcTest();

    static bool TestBitcoinFunctions();

private:
    static bool TestBtcRpc();

    static bool TestBtcJson();

    static bool TestRawTransactions();

    static bool TestMultiSig();

    static bool TestMultiSigDeposit(int minConfirms);

    static bool TestMultiSigWithdrawal(int minConfirms);


    static BtcModulesPtr modules;

    static std::string multiSigAddress;
    static std::string depositTxId;

};

#endif // BTCTEST_HPP
