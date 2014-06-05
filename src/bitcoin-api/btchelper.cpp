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

bool BtcHelper::IsMine(const std::string &address)
{
    BtcAddressInfoPtr addressInfo = this->modules->btcJson->ValidateAddress(address);

    // get address info
    BtcAddressInfoPtr addrInfo = this->modules->btcJson->ValidateAddress(address);
    if(!addrInfo->isvalid)
        return false;

    // is it ours and do we own the public key? then we also know the private key.
    if(addrInfo->ismine && !addrInfo->pubkey.empty())
    {
        return true;
    }

    // multisig
    if(addrInfo->isScript)
    {
        // iterate through the addresses that make up the multisig
        for(btc::stringList::const_iterator multiSigAddr = addrInfo->addresses.begin(); multiSigAddr != addrInfo->addresses.end(); multiSigAddr++)
        {
            if(IsMine((*multiSigAddr)))
            {
                return true;
            }
        }
    }
}

BtcRawTransactionPtr BtcHelper::GetDecodedRawTransaction(const std::string &txId) const
{
    if(txId.empty())
        return BtcRawTransactionPtr();

    // first check transaction database
    BtcTransactionPtr tx = this->modules->btcJson->GetTransaction(txId);
    if(tx != NULL)
        if(!tx->Hex.empty())
            return this->modules->btcJson->DecodeRawTransaction(tx->Hex);

    // otherwise check block database
    // use bitcoind -txindex to keep a complete list of all transactions, otherwise it might not find it if it's too old
    // or use 'importaddress' and bitcoind will index all relevant transactions and return them with 'gettransaction'
    return this->modules->btcJson->GetDecodedRawTransaction(txId);
}

int64_t BtcHelper::GetTotalOutput(const std::string &txId, const std::string &targetAddress)
{
    if(txId.empty() || targetAddress.empty())
            return 0;

    return GetTotalOutput(GetDecodedRawTransaction(txId), targetAddress);
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

int32_t BtcHelper::GetConfirmations(const std::string &txId)
{
    if(txId.empty())
        return 0;

    // if it wasn't a transaction to a multi-sig or if we used importaddress then we can just ask bitcoind
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
    if(txId.empty())
        return btc::stringList();

    BtcTransactionPtr tx = modules->btcJson->GetTransaction(txId);
    if(tx == NULL)
        return btc::stringList();

    return tx->walletConflicts;
}

int64_t BtcHelper::TransactionConfirmed(const std::string &txId, const int32_t &minConfirms)
{
    return GetConfirmations(txId) >= minConfirms;
}

bool BtcHelper::TransactionSuccessfull(int64_t amount, BtcTransactionPtr transaction, const std::string &targetAddress, int minConfirms)
{
    if(amount < 0 || transaction == NULL || targetAddress.empty() || minConfirms < -1)
        return false;

    // see if we got the "hex" transaction (own/watchonly addresses only)
    if(!transaction->Hex.empty())
    {
        return TransactionSuccessfull(amount,
                                      this->modules->btcJson->DecodeRawTransaction(transaction->Hex),
                                      targetAddress, minConfirms);
    }
    else
    {
        // otherwise try to find it in block database. use -txindex=1 to keep a full index.
        return TransactionSuccessfull(amount, WaitGetRawTransaction(transaction->TxId), targetAddress, minConfirms);
    }
}

bool BtcHelper::TransactionSuccessfull(int64_t amountRequested, BtcRawTransactionPtr transaction, const std::string &targetAddress, int32_t minConfirms)
{
    if(transaction == NULL || targetAddress.empty() || minConfirms < 0) // if it hasn't been received yet we will return.
        return false;       // use WaitForTransaction(txid) to prevent this.

    // check for sufficient confirms...
    if(!TransactionConfirmed(transaction->txID, minConfirms))
        return false;

    // check for sufficient amount...
    if(GetTotalOutput(transaction, targetAddress) >= amountRequested)
        return true;    // if we were sent at least as much money as requested, return true

    return false;
}

const BtcRawTransactionPtr BtcHelper::TransactionSuccessfull(const int64_t &amount, BtcUnspentOutputs outputs, const std::string &targetAddress, const int32_t &minConfirms)
{
    if(amount < 0 || outputs.empty() || targetAddress.empty() || MinConfirms < 0)
        return BtcRawTransactionPtr();

    for(size_t i = 0; i < outputs.size(); i++)
    {
        BtcRawTransactionPtr transaction =  WaitGetRawTransaction(outputs[i]->txId);
        if(TransactionSuccessfull(amount, transaction, targetAddress, minConfirms))
            return transaction;
    }

    return BtcRawTransactionPtr();
}

bool BtcHelper::WaitTransactionSuccessfull(const int64_t &amount, BtcTransactionPtr transaction, const std::string &targetAddress, const int32_t &minConfirms, int32_t timeOutSeconds, const int32_t &timerMS)
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

BtcTransactionPtr BtcHelper::WaitGetTransaction(const std::string &txId, const int32_t &timerMS, const int32_t &maxAttempts)
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

BtcRawTransactionPtr BtcHelper::WaitGetRawTransaction(const std::string &txId, const int32_t &timerMS, int32_t maxAttempts)
{
    if(txId.empty())
        return BtcRawTransactionPtr();

    BtcRawTransactionPtr rawTransaction = BtcRawTransactionPtr();

    while(maxAttempts)
    {
        rawTransaction = GetDecodedRawTransaction(txId);
        if(rawTransaction != NULL)
            return rawTransaction;

        maxAttempts--;
        btc::Sleep(timerMS);
    }

    return rawTransaction;
}

BtcUnspentOutputs BtcHelper::ListNewOutputs(const btc::stringList &addresses, BtcUnspentOutputs knownOutputs)
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

BtcUnspentOutputs BtcHelper::FindSignableOutputs(const btc::stringList &txIds)
{
    BtcUnspentOutputs outputs = BtcUnspentOutputs();

    // iterate through txids
    for(btc::stringList::const_iterator txId = txIds.begin(); txId != txIds.end(); txId++)
    {
        // get transaction details
        BtcTransactionPtr tx = this->modules->btcJson->GetTransaction((*txId));
        BtcRawTransactionPtr txRaw;

        // get raw transaction details
        if(!tx->Hex.empty())
            txRaw = this->modules->btcJson->DecodeRawTransaction(tx->Hex);
        else
            txRaw = this->GetDecodedRawTransaction((*txId));

        // iterate through raw transaction VOUT array
        for(std::vector<BtcRawTransaction::VOUT>::const_iterator vout = txRaw->outputs.begin(); vout != txRaw->outputs.end(); vout++)
        {
            // iterate through VOUT.addresses array (there should only be one address in that array)
            for(btc::stringList::const_iterator outAddr = vout->addresses.begin(); outAddr != vout->addresses.end(); outAddr++)
            {
                if(!IsMine(*outAddr))
                    continue;

                BtcUnspentOutputPtr output = BtcUnspentOutputPtr(new BtcUnspentOutput(Json::Value()));
                output->txId = (*txId);
                output->address = (*outAddr);
                output->scriptPubKey = vout->scriptPubKeyHex;
                output->amount = vout->value;
                output->vout = vout->n;
                outputs.push_back(output);
            }
        }
    }

    return outputs;
}

BtcSigningPrerequisites BtcHelper::GetSigningPrerequisites(const BtcUnspentOutputs &outputs)
{
    BtcSigningPrerequisites prereqs = BtcSigningPrerequisites();

    for(BtcUnspentOutputs::const_iterator output = outputs.begin(); output != outputs.end(); output++)
    {
        BtcAddressInfoPtr addrInfo = this->modules->btcJson->ValidateAddress((*output)->address);
        if(addrInfo == NULL)
            addrInfo = BtcAddressInfoPtr(new BtcAddressInfo(Json::Value(Json::objectValue)));
        prereqs.push_back(BtcSigningPrerequisitePtr(new BtcSigningPrerequisite((*output)->txId, (*output)->vout, (*output)->scriptPubKey, addrInfo->redeemScript)));
    }

    return prereqs;
}

BtcSignedTransactionPtr BtcHelper::CreateSpendTransaction(const BtcUnspentOutputs &outputs, const int64_t &amount, const std::string &toAddress, const std::string &changeAddress,
                                                const int64_t &fee)
{
    BtcTxIdVouts vouts = BtcTxIdVouts();
    BtcTxTargets targets = BtcTxTargets();
    int64_t amountAvailable = 0;
    for(BtcUnspentOutputs::const_iterator output = outputs.begin(); output != outputs.end(); output++)
    {
        vouts.push_back(BtcTxIdVoutPtr(new BtcTxIdVout((*output)->txId, (*output)->vout)));
        amountAvailable += (*output)->amount;
    }

    int64_t change = amountAvailable - amount - fee;
    if(change < 0)
       return BtcSignedTransactionPtr();  // invalid amount

    // set amount=0 to sweep all funds to change address
    if(amount > 0)
        targets.SetTarget(toAddress, amount);
    if(change > 0)
        targets.SetTarget(changeAddress, change);

    BtcSignedTransactionPtr transactionPtr = BtcSignedTransactionPtr(new BtcSignedTransaction(Json::Value(Json::objectValue)));
    transactionPtr->signedTransaction = this->modules->btcJson->CreateRawTransaction(vouts, targets);

    return transactionPtr;
}

BtcSignedTransactionPtr BtcHelper::WithdrawAllFromAddress(const std::string &txToSourceId, const std::string &sourceAddress, const std::string &destinationAddress, const int64_t fee, const std::string &redeemScript /* = "" */, const std::string &signingAddress /* = "" */)
{
    // This function will check a txId for outputs leading to sourceAddress
    // and then create and sign a raw transaction sending those outputs to destinationAddress.
    // This will only work when none of the outputs (usually it's just one) have been spent yet

    // retrieve decoded raw transaction sending funds to our sourceAddress
    BtcRawTransactionPtr rawTransaction = GetDecodedRawTransaction(txToSourceId);
    if(rawTransaction == NULL)
        return BtcSignedTransactionPtr();   // return NULL

    // count funds in source address and list outputs leading to it
    int64_t funds = 0;
    BtcTxIdVouts unspentOutputs = BtcTxIdVouts();
    BtcSigningPrerequisites signingPrerequisites = BtcSigningPrerequisites();
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
                signingPrerequisites.push_back(BtcSigningPrerequisitePtr(new BtcSigningPrerequisite(txToSourceId, output.n, output.scriptPubKeyHex, redeemScript)));
        }
    }

    // substract fee
    if((funds -= fee) <= 0)
        return BtcSignedTransactionPtr();

    // map of output addresses and the amount each of them receives.
    BtcTxTargets txTargets = BtcTxTargets();
    txTargets.SetTarget(destinationAddress, funds);

    // create raw transaction to send outputs to target address
    std::string withdrawTransaction = this->modules->btcJson->CreateRawTransaction(unspentOutputs, txTargets);

    // only sign transaction with signingAddress's privkey
    // in multi-sig signing will fail unless redeemScript is passed aswell, but we can try anyway...
    btc::stringList signingKeys;
    if(!signingAddress.empty() /*&& !redeemScript.empty()*/)
    {
        signingKeys.push_back(this->modules->btcJson->GetPrivateKey(signingAddress));
    }

    // Note: signingPrerequisites can be empty if the input transaction is known and if signingKeys is empty too,
    // in that case bitcoin will sign with any key that fits.
    // this should only be done with locally generated transactions or transactions whose outputs were checked first
    // because otherwise someone could give you a transaction to send funds from your wallet to his and you'd blindly sign it.

    // sign raw transaction
    // as we just created this tx ourselves, we can assume that it is safe to sign
    BtcSignedTransactionPtr signedTransact = this->modules->btcJson->SignRawTransaction(withdrawTransaction, signingPrerequisites, signingKeys);

    return signedTransact;
}
