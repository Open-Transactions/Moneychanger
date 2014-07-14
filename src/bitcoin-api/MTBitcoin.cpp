#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin-api/MTBitcoin.hpp>

#include <bitcoin-api/btcmodules.hpp>


MTBitcoin::MTBitcoin(BtcModules *modules)
{
    this->modules = modules;
}

MTBitcoin::~MTBitcoin()
{
    this->modules = NULL;
}

int64_t MTBitcoin::GetBalance(const std::string &account)
{
    return this->modules->btcJson->GetBalance(account);
}

std::string MTBitcoin::GetNewAddress(const std::string &account)
{
    return this->modules->btcJson->GetNewAddress(account);
}

bool MTBitcoin::ImportAddress(const std::string &address, const std::string &label, bool rescan)
{
    return this->modules->btcJson->ImportAddress(address, label, rescan);
}

std::string MTBitcoin::GetPublicKey(const std::string& address)
{
    return this->modules->btcJson->GetPublicKey(address);
}

std::string MTBitcoin::GetMultiSigAddress(int32_t minSignatures, const btc::stringList& publicKeys, bool addToWallet, const std::string &account)
{
    BtcMultiSigAddressPtr multiSigAddr;
    return (multiSigAddr = GetMultiSigAddressInfo(minSignatures, publicKeys, addToWallet, account)) == NULL ? "" : multiSigAddr->address;
}

BtcMultiSigAddressPtr MTBitcoin::GetMultiSigAddressInfo(int32_t minSignatures, const btc::stringList &publicKeys, bool addToWallet, const std::string &account)
{
    BtcMultiSigAddressPtr multiSigAddr;
    if(addToWallet)
    {
        multiSigAddr = this->modules->btcJson->AddMultiSigAddress(minSignatures, publicKeys, account);
    }
    else
    {
        multiSigAddr = this->modules->btcJson->CreateMultiSigAddress(minSignatures, publicKeys);
    }

    return multiSigAddr;
}

BtcTransactionPtr MTBitcoin::GetTransaction(const std::string &txId)
{
    return this->modules->btcJson->GetTransaction(txId);
}

BtcRawTransactionPtr MTBitcoin::GetRawTransaction(const std::string &txId)
{
    return this->modules->btcJson->GetDecodedRawTransaction(txId);
}

BtcRawTransactionPtr MTBitcoin::WaitGetRawTransaction(const std::string &txId)
{
    // wait for transaction and return it
    // times out after 10 seconds by default
    return this->modules->btcHelper->WaitGetRawTransaction(txId, 500, 20);
}

int32_t MTBitcoin::GetConfirmations(const std::string &txId)
{
    return this->modules->btcHelper->GetConfirmations(txId);
}

bool MTBitcoin::TransactionSuccessful(const int64_t &amount, BtcRawTransactionPtr rawTransaction, const std::string &targetAddress, const int32_t &confirmations)
{
    return this->modules->btcHelper->TransactionSuccessfull(amount, rawTransaction, targetAddress, confirmations);
}

std::string MTBitcoin::SendToAddress(const std::string &to_address, int64_t lAmount)
{
    return this->modules->btcJson->SendToAddress(to_address, lAmount);
}

std::string MTBitcoin::SendToMultisig(int64_t lAmount, int nRequired, const btc::stringList &to_publicKeys)
{
    // will send to a multi-sig address without adding it to the wallet.
    // if you want the address to be added to your wallet, use AddMultiSigAddress() and SendToAddress()

    // generate the multi-sig address
    BtcMultiSigAddressPtr multiSigAddr = this->modules->btcJson->CreateMultiSigAddress(nRequired, to_publicKeys);

    // send to the address
    return this->modules->btcJson->SendToAddress(multiSigAddr->address, lAmount);
}

BtcSignedTransactionPtr MTBitcoin::VoteMultiSigRelease(const std::string &txToSourceId, const std::string &sourceAddress, const std::string &destinationAddress, int64_t fee, const std::string &redeemScript, const std::string &signingAddress)
{
    return this->modules->btcHelper->WithdrawAllFromAddress(txToSourceId,
                                                         sourceAddress,
                                                         destinationAddress,
                                                         fee,
                                                         redeemScript,
                                                         signingAddress);
}

BtcSignedTransactionPtr MTBitcoin::CombineTransactions(const std::string &concatenatedRawTransactions)
{
    return this->modules->btcJson->CombineSignedTransactions(concatenatedRawTransactions);
}

std::string MTBitcoin::SendRawTransaction(const std::string &rawTransaction)
{
    return this->modules->btcJson->SendRawTransaction(rawTransaction);
}

BtcUnspentOutputs MTBitcoin::ListUnspentOutputs(const btc::stringList &addresses)
{
    return this->modules->btcJson->ListUnspent(BtcHelper::MinConfirms, BtcHelper::MaxConfirms, addresses);
}





