#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin-api/btchelper.hpp>

#include <bitcoin-api/btcmodules.hpp>

#include <stdio.h>
#include <iostream>
#include <vector>
#include <algorithm>    // std::find

#ifndef OT_USE_TR1
    #include <thread>   // sleep()
#else
    #include <unistd.h> // usleep
    #include <sstream>  // ostringstream
#endif // OT_USE_TR1

namespace btc   // will probably put everything into this namespace
{
    void Sleep(int32_t milliSeconds)
    {
    #ifndef OT_USE_TR1
        std::this_thread::sleep_for(std::chrono::milliseconds(milliSeconds));
        std::this_thread::yield();  // let's also free some CPU because who cares about a few ms here?
    #else
        usleep(milliSeconds * 1000);
    #endif // OT_USE_TR1
    }

    template <typename T>
    std::string to_string(T number)
    {
    #ifndef OT_USE_TR1
        return std::to_string(number);
    #else
         std::ostringstream oOStrStream;
         oOStrStream << number;
         return oOStrStream.str();
    #endif
    }
}


int32_t BtcHelper::MinConfirms = 0;         // used for listunspent, should be zero or same as WaitForConfirms imho
int32_t BtcHelper::MaxConfirms = 9999999;   // used for listunspent, it's 9999999 in bitcoin-qt
int32_t BtcHelper::WaitForConfirms = 1;     // confirmations to wait for before accepting a transaction as confirmed. should be higher.
int64_t BtcHelper::FeeMultiSig = BtcHelper::CoinsToSatoshis(0.001);

BtcHelper::BtcHelper(BtcModules *modules)
{
    this->modules = modules;

    std::string str = btc::to_string(0);
}

BtcHelper::~BtcHelper()
{
    this->modules = NULL;
}

// converts a double bitcoin (as received through json api) to int64 satoshis
int64_t BtcHelper::CoinsToSatoshis(double value)
{
    // copied from the wiki
    return (int64_t)(value * 1e8 + (value < 0.0 ? -.5 : .5));
}

// converts int64 satoshis to double bitcoin
double BtcHelper::SatoshisToCoins(int64_t value)
{
    // also copied from the wiki
    return (double)value / 1e8;
}

int64_t BtcHelper::GetTotalOutput(const std::string &transactionId, const std::string &targetAddress)
{
    return GetTotalOutput(this->modules->btcJson->GetDecodedRawTransaction(transactionId), targetAddress);
}

int64_t BtcHelper::GetTotalOutput(BtcRawTransactionPtr transaction, const std::string &targetAddress)
{
    if(transaction == NULL || targetAddress.empty())
        return 0;

    int64_t amountReceived = 0.0;
    for(uint64_t i = 0; i < transaction->outputs.size(); i++)
    {
        // I don't know what outputs to multiple addresses mean so I'm not gonna trust them for now.
        if(transaction->outputs[i].addresses.size() > 1)
        {
            printf("Multiple output addresses per output detected.");
            std::cout.flush();
            continue;
        }

        // TODO: vulnerability fix
        // I don't know much about scriptPubKey but I think a malicious buyer could create a
        // transaction that isn't spendable by anyone, see
        // https://en.bitcoin.it/wiki/Script#Provably_Unspendable.2FPrunable_Outputs
        // I think the easiest solution would be to check
        // if scriptPubKey.hex != "76a914<pub key hash>88ac" return false
        // as this seems to be the hex representation of the most basic standard transaction.


        if(std::find(transaction->outputs[i].addresses.begin(), transaction->outputs[i].addresses.end(), targetAddress) != transaction->outputs[i].addresses.end())
        {
            // if output leads to target address, add value to total received bitcoins
            amountReceived += transaction->outputs[i].value;
        }
    }

    return amountReceived;
}

int64_t BtcHelper::GetConfirmations(const std::string &txId)
{
    if(txId.empty())
        return 0;

    // if it wasn't a transaction to a multi-sig then we can just ask bitcoind
    BtcTransactionPtr transaction = this->modules->btcJson->GetTransaction(txId);
    if(transaction != NULL)
        return transaction->Confirmations;


    // otherwise we will have to crawl through the blockchain:

    // firstly, see if the transaction isn't included in a block yet
    std::vector<std::string> rawMemPool = this->modules->btcJson->GetRawMemPool();
    if(std::find(rawMemPool.begin(), rawMemPool.end(), txId) != rawMemPool.end())
        return 0;    // 0 confirmations if still in mempool

    // if it is, we'll need to loop through the latest blocks until we find it
    // getblockcount --> getblockhash(count) --> getblock(hash) --> getblock(block->previous) -->...
    // get number of latest block (this is also called 'height')
    int latestBlock = this->modules->btcJson->GetBlockCount();
    // get hash of latest block
    std::string blockHash = this->modules->btcJson->GetBlockHash(latestBlock);
    // get the actual block
    BtcBlockPtr currentBlock = this->modules->btcJson->GetBlock(blockHash);
    // the block might not be downloaded yet, in that case return
    if(currentBlock == NULL)    // I'm not sure how this can happen but i think it did.
        return 0;

    int64_t confirmations = 1;  // first block = first confirmation

    // see if txId is NOT in the block's transaction list
    while(std::find(currentBlock->transactions.begin(), currentBlock->transactions.end(), txId) == currentBlock->transactions.end())
    {
        confirmations++;

        currentBlock = this->modules->btcJson->GetBlock(currentBlock->previousHash);
        if(currentBlock == NULL)
            return 0;   // Genesis block - this should never happen I think. except maybe during forks
                        // oh shit this could also happen if we check for a bogus txId
                        // ==> DoS
                        // TODO: optional argument to limit number of blocks to check?
    }

    // if we find it in an old enough block, return number of confirmations
    return confirmations;
}

btc::stringList BtcHelper::GetDoubleSpends(const std::string &txId)
{
    BtcTransactionPtr tx = modules->btcJson->GetTransaction(txId);
    if(tx == NULL)
        return btc::stringList();

    return tx->walletConflicts;
}

int64_t BtcHelper::TransactionConfirmed(const std::string &txId, int minConfirms)
{
    return GetConfirmations(txId) >= minConfirms;
}

bool BtcHelper::TransactionSuccessfull(int64_t amount, BtcTransactionPtr transaction, const std::string &targetAddress, int minConfirms)
{
    if(amount < 0 || transaction == NULL || targetAddress.empty() || minConfirms < 0)
        return false;

    // see if we got the "hex" transaction (own/watchonly addresses only)
    if(!transaction->rawTransaction.empty())
    {
        return TransactionSuccessfull(amount,
                                      this->modules->btcJson->DecodeRawTransaction(transaction->rawTransaction),
                                      targetAddress, minConfirms);
    }
    else
    {
        // otherwise try to find it in block database. use -txindex=1 to keep a full index.
        return TransactionSuccessfull(amount, WaitGetRawTransaction(transaction->TxID), targetAddress, minConfirms);
    }
}

bool BtcHelper::TransactionSuccessfull(int64_t amountRequested, BtcRawTransactionPtr transaction, const std::string &targetAddress, int minConfirms)
{
    if(transaction == NULL) // if it hasn't been received yet we will return.
        return false;       // use WaitForTransaction(txid) to prevent this.

    // check for sufficient confirms...
    if(!TransactionConfirmed(transaction->txID, minConfirms))
        return false;

    // check for sufficient amount...
    if(GetTotalOutput(transaction, targetAddress) >= amountRequested)
        return true;    // if we were sent at least as much money as requested, return true

    return false;
}

const BtcRawTransactionPtr BtcHelper::TransactionSuccessfull(const int64_t &amount, BtcUnspentOutputs outputs, const std::string &targetAddress, const int32_t &MinConfirms)
{
    if(amount < 0 || outputs.empty() || targetAddress.empty() || MinConfirms < 0)
        return BtcRawTransactionPtr();

    for(size_t i = 0; i < outputs.size(); i++)
    {
        BtcRawTransactionPtr transaction =  WaitGetRawTransaction(outputs[i]->txId);
        if(TransactionSuccessfull(amount, transaction, targetAddress))
            return transaction;
    }

    return BtcRawTransactionPtr();
}

bool BtcHelper::WaitTransactionSuccessfull(const int64_t &amount, BtcTransactionPtr transaction, const std::string &targetAddress, const int32_t &minConfirms, int timeOutSeconds, const int &timerMS)
{
    if(amount < 0 || transaction == NULL || minConfirms < 0 || timeOutSeconds <= 0 || timerMS < 0)
        return false;

    // convert to milliseconds
    timeOutSeconds *= 1000;

    while(timeOutSeconds)
    {
        if(TransactionSuccessfull(amount, transaction, targetAddress, minConfirms))
            return true;

        btc::Sleep(timerMS);
        timeOutSeconds -= timerMS;
    }

    return false;
}

bool BtcHelper::WaitTransactionSuccessfull(const int64_t &amount, BtcRawTransactionPtr transaction, const std::string &targetAddress, const int32_t &minConfirms, int timeOutSeconds, const int &timerMS)
{
    if(amount < 0 || transaction == NULL || minConfirms < 0 || timeOutSeconds < 0 || timerMS < 0)
        return false;

    // convert to milliseconds
    timeOutSeconds *= 1000;

    while(timeOutSeconds)
    {
        if(TransactionSuccessfull(amount, transaction, targetAddress, minConfirms))
            return true;

        btc::Sleep(timerMS);
        timeOutSeconds -= timerMS;
    }

    return false;
}

BtcTransactionPtr BtcHelper::WaitGetTransaction(const std::string &txId, int timerMS, int maxAttempts)
{
    if(txId.empty())
        return BtcTransactionPtr();

    BtcTransactionPtr transaction = BtcTransactionPtr();
    while(maxAttempts)
    {
        transaction = this->modules->btcJson->GetTransaction(txId);
        if(transaction != NULL)
            return transaction;

        btc::Sleep(timerMS);
    }

    return transaction;
}

BtcRawTransactionPtr BtcHelper::WaitGetRawTransaction(const std::string &txId, int timerMS, int maxAttempts)
{
    if(txId.empty())
        return BtcRawTransactionPtr();

    BtcRawTransactionPtr rawTransaction = BtcRawTransactionPtr();

    // TODO: see WaitGetTransaction()

    // first, see if we have it in our transaction database
    BtcTransactionPtr transaction = this->modules->btcJson->GetTransaction(txId);
    if(transaction != NULL)
    {
        if(!transaction->rawTransaction.empty())
        {
            rawTransaction = this->modules->btcJson->DecodeRawTransaction(transaction->rawTransaction);
            return rawTransaction;
        }
    }

    // otherwise see if we have it the in our block database
    while(maxAttempts)
    {
        rawTransaction = this->modules->btcJson->GetDecodedRawTransaction(txId);
        if(rawTransaction != NULL)
            return rawTransaction;

        maxAttempts--;
        btc::Sleep(timerMS);
    }

    return rawTransaction;
}

BtcUnspentOutputs BtcHelper::ListNewOutputs(const std::vector<std::string> &addresses, BtcUnspentOutputs knownOutputs)
{
    if(addresses.empty())
        return BtcUnspentOutputs();

    // extract txids from unspent outputs
    std::vector<std::string> knownOutputTxIds = std::vector<std::string>();
    for(size_t i = 0; i < knownOutputs.size(); i++)
    {
        knownOutputTxIds.push_back(knownOutputs[i]->txId);
    }

    // get list of all unspent outputs
    BtcUnspentOutputs outputs = this->modules->btcJson->ListUnspent(MinConfirms, MaxConfirms, addresses);

    // iterate through them
    BtcUnspentOutputs newOutputs = BtcUnspentOutputs();
    for(size_t i = 0; i < outputs.size(); i++)
    {
        // check if we already know about them
        std::vector<std::string>::const_iterator it;
        it = find (knownOutputTxIds.begin(), knownOutputTxIds.end(), outputs[i]->txId);
        if(it == knownOutputTxIds.end())
            newOutputs.push_back(outputs[i]);   // and add them if we don't
    }

    return newOutputs;
}

BtcSignedTransactionPtr BtcHelper::WithdrawAllFromAddress(const std::string &txToSourceId, const std::string &sourceAddress, const std::string &destinationAddress, int64_t fee, const std::string &redeemScript /* = "" */, const std::string &signingAddress /* = "" */)
{
    // This function will check a txId for outputs leading to sourceAddress
    // and then create and sign a raw transaction sending those outputs to destinationAddress.
    // This will only work when none of the outputs (usually it's just one) have been spent yet

    // retrieve decoded raw transaction sending funds to our sourceAddress
    BtcRawTransactionPtr rawTransaction = this->modules->btcJson->GetDecodedRawTransaction(txToSourceId);
    if(rawTransaction == NULL)
        return BtcSignedTransactionPtr();   // return NULL

    // count funds in source address and list outputs leading to it
    int64_t funds = 0;
    BtcTxIdVouts unspentOutputs = BtcTxIdVouts();
    std::list<BtcSigningPrerequisite> signingPrerequisites;
    for(uint64_t i = 0; i < rawTransaction->outputs.size(); i++)
    {
        BtcRawTransaction::VOUT output = rawTransaction->outputs[i];

        // check if output leads to sourceAddess
        // idk what multiple addresses per output means so we'll skip those cases
        if(output.addresses.size() == 1 && output.addresses[0] == sourceAddress)
        {
            funds += output.value;
            unspentOutputs.push_back(BtcTxIdVoutPtr(new BtcTxIdVout(txToSourceId, output.n)));

            // create signing prerequisite (optional, needed for offline signing)
            if(!signingAddress.empty() && !redeemScript.empty())
                signingPrerequisites.push_back(BtcSigningPrerequisite(txToSourceId, output.n, output.scriptPubKeyHex, redeemScript));
        }
    }

    // substract fee
    if((funds -= fee) <= 0)
        return BtcSignedTransactionPtr();

    // map of output addresses and the amount each of them receives.
    BtcTxTarget txTargets = BtcTxTarget();
    txTargets[destinationAddress] = Json::Value((Json::Value::Int64)funds);

    // create raw transaction to send outputs to target address
    std::string withdrawTransaction = this->modules->btcJson->CreateRawTransaction(unspentOutputs, txTargets);

    // only sign transaction with signingAddress's privkey
    // in multi-sig signing will fail unless redeemScript is passed aswell, but we can try anyway...
    btc::stringList signingKeys;
    if(!signingAddress.empty() /*&& !redeemScript.empty()*/)
    {
        signingKeys.push_back(this->modules->btcJson->GetPrivateKey(signingAddress));
    }

    // Note: signingPrerequisites can be empty, in that case bitcoin will sign with any key that fits.
    // this should only be done with locally generated transactions or transactions whose outputs were checked first
    // because otherwise someone could give you a transaction to send funds from your wallet to his and you'd blindly sign it.
    // if a signingAddress is passed, redeemScript is required aswell, at least for p2sh.

    // sign raw transaction
    // as we just created this tx ourselves, we can assume that it is safe to sign
    BtcSignedTransactionPtr signedTransact = this->modules->btcJson->SignRawTransaction(withdrawTransaction, signingPrerequisites, signingKeys);

    return signedTransact;
}
