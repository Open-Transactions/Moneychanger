#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin/sampleescrowtransaction.hpp>


SampleEscrowTransaction::SampleEscrowTransaction(int64_t amountToSend, BtcModulesPtr modules)
{
    this->amountToSend = amountToSend;

    this->status = NotStarted;
    this->txId = "";
    this->targetAddr = "";
    this->confirmations = 0;

    this->modules = modules;
}

bool SampleEscrowTransaction::SendToTarget()
{
    std::printf("sending %f.8 BTC to %s\n", BtcHelper::SatoshisToCoins(this->amountToSend), this->targetAddr.c_str());
    std::cout.flush();

    // send to this address, get transaction id
    this->txId = this->modules->mtBitcoin->SendToAddress(this->targetAddr, this->amountToSend);

    // check if we got a tx id
    if(this->txId == "")
    {
        this->status = Failed;
        return false;
    }

    this->status = Pending;
    return true;
}

void SampleEscrowTransaction::CheckTransaction(int minConfirms)
{
    //if(this->status == Successfull)
    //    return;

    // wait for the transaction to be broadcasted over the network
    // and get an object containing info
    // we have to use raw transactions here because bitcoin- doesn't properly support multi-sig yet
    BtcTransactionPtr tx = this->modules->mtBitcoin->GetTransaction(this->txId);
    if(tx == NULL)
    {
        this->status = Failed;
        return;
    }

    // this only works with multisig transactions that were 'importaddress'd
    BtcRawTransactionPtr rawTx = this->modules->btcJson->DecodeRawTransaction(tx->Hex);
    if(rawTx == NULL)
        return;

    // check if transaction has enough confirmations
    if(this->modules->mtBitcoin->TransactionSuccessfull(this->amountToSend, rawTx, this->targetAddr, minConfirms))
        this->status = Successfull;
    else
    {
        this->status = Pending;
        if(tx->walletConflicts.size() > 0)
            this->status = Conflicted;
    }

    this->confirmations = this->modules->mtBitcoin->GetConfirmations(rawTx->txID);
}
