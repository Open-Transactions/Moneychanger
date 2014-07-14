#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin-api/btchelper.hpp>

#include <bitcoin-api/btcmodules.hpp>

#include <vector>
#include <stdio.h>
#include <iostream>
#include <iomanip>      // setprecision
#include <sstream>      // ostringstream
#include <cstdio>       // std::printf
#include <cstdlib>      // std::find
#include <algorithm>    // std::find

#ifndef OT_USE_TR1
    #include <thread>   // sleep()
    //#include <string>   // to_string
#else
    #include <unistd.h> // usleep
#endif // OT_USE_TR1

void btc::Sleep(time_t milliSeconds)
{
#ifndef OT_USE_TR1
    std::this_thread::sleep_for(std::chrono::milliseconds(milliSeconds));
#else
    usleep(milliSeconds * 1000);
#endif // OT_USE_TR1
}

template <typename T>
std::string btc::to_string(T number)
{
    std::ostringstream oOStrStream;
    oOStrStream << std::setprecision(8) << number;      // precision 8 because satoshis
    return oOStrStream.str();
}
template std::string btc::to_string<int32_t>(int32_t);
template std::string btc::to_string<uint32_t>(uint32_t);
template std::string btc::to_string<int64_t>(int64_t);
template std::string btc::to_string<uint64_t>(uint64_t);
template std::string btc::to_string<double>(double);


int32_t BtcHelper::MinConfirms = 0;         // used for listunspent, should be zero or same as WaitForConfirms imho
int32_t BtcHelper::MaxConfirms = 9999999;   // used for listunspent, it's 9999999 in bitcoin-qt
int32_t BtcHelper::WaitForConfirms = 2;     // confirmations to wait for before accepting a transaction as confirmed. should be higher.
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
    // get address info
    BtcAddressInfoPtr addrInfo = this->modules->btcJson->ValidateAddress(address);
    if(addrInfo == NULL || !addrInfo->isvalid)
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
        for(btc::stringList::const_iterator multiSigSigningAddr = addrInfo->addresses.begin(); multiSigSigningAddr != addrInfo->addresses.end(); multiSigSigningAddr++)
        {
            // i hate recursions but this one is limited to one level because multisig addresses can't be made from multisig addresses
            if(IsMine((*multiSigSigningAddr)))
            {
                return true;
            }
        }
    }

    return false;
}

BtcBalancesPtr BtcHelper::GetBalances()
{
    BtcBalancesPtr balances = BtcBalancesPtr(new BtcBalances());

    int64_t allConf = modules->btcJson->GetBalance("*", WaitForConfirms, true); // include watchonly
    int64_t all = modules->btcJson->GetBalance("*", 0, true);                   // include watchonly
    int64_t allMine = modules->btcJson->GetBalance("*", 0, false);              // spendable total, exclude watchonly

    balances->confirmed = this->modules->btcJson->GetBalance("*", WaitForConfirms, false);    // spendable confirmed
    balances->pending = allMine - balances->confirmed;                          // spendable total - spendable confirmed
    balances->watchConfirmed = allConf - balances->confirmed;                   // all confirmed - confirmed total
    balances->watchPending = all - allMine - balances->watchConfirmed;          // all - allMine - watch confirmed

    return balances;
}

BtcRawTransactionPtr BtcHelper::GetDecodedRawTransaction(const BtcTransactionPtr &tx) const
{
    if(tx == NULL)
        return BtcRawTransactionPtr();

    if(!tx->Hex.empty())
        return this->modules->btcJson->DecodeRawTransaction(tx->Hex);
    return this->modules->btcJson->GetDecodedRawTransaction(tx->TxId);
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
            std::printf("Multiple output addresses per output detected.");
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
    if(!TransactionConfirmed(transaction->txId, minConfirms))
        return false;

    // check for sufficient amount...
    if(GetTotalOutput(transaction, targetAddress) >= amountRequested)
        return true;    // if we were sent at least as much money as requested, return true

    btc::stringList doubleSpends = GetDoubleSpends(transaction->txId);
    if(doubleSpends.size() <= 0)
        return true;

    BtcTransactionPtr txToCheck = modules->btcJson->GetTransaction(transaction->txId);
    for(btc::stringList::const_iterator txId = doubleSpends.begin(); txId != doubleSpends.end(); txId++)
    {
        BtcTransactionPtr doubleTx = this->modules->btcJson->GetTransaction((*txId));
        if(doubleTx->Confirmations > txToCheck->Confirmations)
            return false;   // double spend with higher confirmations, or maybe just tx malleability
    }

    return false;
}

const BtcRawTransactionPtr BtcHelper::TransactionSuccessfull(const int64_t &amount, BtcUnspentOutputs outputs, const std::string &targetAddress, const int32_t &minConfirms)
{
    if(amount < 0 || outputs.empty() || targetAddress.empty() || minConfirms < 0)
        return BtcRawTransactionPtr();

    for(BtcUnspentOutputs::iterator output = outputs.begin(); output != outputs.end(); output++)
    {
        BtcRawTransactionPtr transaction =  WaitGetRawTransaction((*output)->txId);
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

    BtcTransactionPtr transaction;
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

    BtcRawTransactionPtr rawTransaction;

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

BtcUnspentOutputs BtcHelper::ListNewOutputs(BtcUnspentOutputs knownOutputs, const btc::stringList &addresses)
{
    if(addresses.empty())
        return BtcUnspentOutputs();

    // extract txids from unspent outputs
    btc::stringList knownOutputTxIds = btc::stringList();
    for(BtcUnspentOutputs::iterator output = knownOutputs.begin(); output != knownOutputs.end(); output++)
    {
        knownOutputTxIds.push_back((*output)->txId);
    }

    // get list of all unspent outputs
    BtcUnspentOutputs outputs = this->modules->btcJson->ListUnspent(MinConfirms, MaxConfirms, addresses);

    // iterate through them
    BtcUnspentOutputs newOutputs;
    for(BtcUnspentOutputs::iterator output = outputs.begin(); output != outputs.end(); output++)
    {
        // check if we already know about them
        btc::stringList::iterator it;
        it = std::find (knownOutputTxIds.begin(), knownOutputTxIds.end(), (*output)->txId);
        if(it == knownOutputTxIds.end())
            newOutputs.push_back((*output));   // add them if we don't
        else
            knownOutputTxIds.erase(it, it);
    }

    return newOutputs;
}

BtcUnspentOutputs BtcHelper::FindSignableOutputs(const btc::stringList &txIds)
{
    BtcUnspentOutputs outputsToCheck;
    std::string lastTxId = std::string();   // prevent double inserts

    // iterate through txids
    for(btc::stringList::const_iterator txId = txIds.begin(); txId != txIds.end(); txId++)
    {
        if((*txId) == lastTxId)
            continue;
        lastTxId = (*txId);

        // get transaction details
        BtcRawTransactionPtr txRaw = GetDecodedRawTransaction((*txId));
        // iterate through raw transaction VOUT array
        for(std::vector<BtcRawTransaction::VOUT>::const_iterator vout = txRaw->outputs.begin(); vout != txRaw->outputs.end(); vout++)
        {
            BtcUnspentOutputPtr output = BtcUnspentOutputPtr(new BtcUnspentOutput(Json::Value()));
            output->txId = (*txId);
            if(vout->addresses.size() == 1)
                output->address = vout->addresses.front();
            output->scriptPubKey = vout->scriptPubKeyHex;
            output->amount = vout->value;
            output->vout = vout->n;
            outputsToCheck.push_back(output);
        }
    }

    return FindSignableOutputs(outputsToCheck);
}

BtcUnspentOutputs BtcHelper::FindSignableOutputs(const BtcUnspentOutputs &outputs)
{
    BtcUnspentOutputs signableOutputs;

    for(BtcUnspentOutputs::const_iterator output = outputs.begin(); output != outputs.end(); output++)
    {
        if(IsMine((*output)->address))
            signableOutputs.push_back((*output));
    }

    return signableOutputs;
}

BtcUnspentOutputs BtcHelper::FindUnspentOutputs(BtcUnspentOutputs possiblySpentOutputs)
{
    BtcUnspentOutputs unspentOutputs;
    for(BtcUnspentOutputs::iterator outputToCheck = possiblySpentOutputs.begin(); outputToCheck != possiblySpentOutputs.end(); outputToCheck++)
    {
        BtcUnspentOutputPtr output = this->modules->btcJson->GetTxOut((*outputToCheck)->txId, (*outputToCheck)->vout);
        if(output != NULL)
        {
            std::printf ("found unspent output %s : %ld, %f BTC\n", output->txId.c_str(), output->vout, SatoshisToCoins(output->amount));
            unspentOutputs.push_back(output);
        }
    }

    return unspentOutputs;
}

BtcUnspentOutputs BtcHelper::FindUnspentOutputs(const btc::stringList &txIdsToCheck)
{
    BtcUnspentOutputs unspentOutputs;
    std::string lastTxId = std::string();   // prevent double inserts

    for(btc::stringList::const_iterator txId = txIdsToCheck.begin(); txId != txIdsToCheck.end(); txId++)
    {
        if((*txId) == lastTxId)
            continue;
        lastTxId = (*txId);

        BtcRawTransactionPtr rawTx = GetDecodedRawTransaction((*txId));
        if(rawTx == NULL)
            continue;

        // iterate through raw transaction VOUT array
        for(std::vector<BtcRawTransaction::VOUT>::const_iterator vout = rawTx->outputs.begin(); vout != rawTx->outputs.end(); vout++)
        {
            BtcUnspentOutputPtr output = this->modules->btcJson->GetTxOut(rawTx->txId, vout->n);
            if(output == NULL)
                continue;

            if(!output->address.empty())
                unspentOutputs.push_back(output);
        }
    }

    return unspentOutputs;
}

BtcUnspentOutputs BtcHelper::FindUnspentSignableOutputs(const btc::stringList &txIds)
{
    BtcUnspentOutputs signableOutputs = FindSignableOutputs(txIds);
    if(signableOutputs.empty())
        return BtcUnspentOutputs();

    BtcUnspentOutputs unspentSignableOutputs = FindUnspentOutputs(signableOutputs);
    return unspentSignableOutputs;
}

BtcSigningPrerequisites BtcHelper::GetSigningPrerequisites(const BtcUnspentOutputs &outputs)
{
    BtcSigningPrerequisites prereqs;

    for(BtcUnspentOutputs::const_iterator output = outputs.begin(); output != outputs.end(); output++)
    {
        std::string redeemScript = (*output)->redeemScript;

        if(redeemScript.empty())
        {
            BtcAddressInfoPtr addrInfo = this->modules->btcJson->ValidateAddress((*output)->address);
            if(addrInfo != NULL)
                redeemScript = addrInfo->redeemScript;
        }

        prereqs.push_back(BtcSigningPrerequisitePtr(new BtcSigningPrerequisite((*output)->txId, (*output)->vout, (*output)->scriptPubKey, redeemScript)));
    }

    return prereqs;
}

BtcSignedTransactionPtr BtcHelper::CreateSpendTransaction(const BtcUnspentOutputs &outputs, const int64_t &amount, const std::string &toAddress, const std::string &changeAddress,
                                                const int64_t &fee)
{
    BtcTxIdVouts vouts;
    BtcTxTargets targets;
    int64_t amountAvailable = 0;
    for(BtcUnspentOutputs::const_iterator output = outputs.begin(); output != outputs.end(); output++)
    {
        vouts.push_back(BtcTxIdVoutPtr(new BtcTxIdVout((*output)->txId, (*output)->vout)));
        amountAvailable += (*output)->amount;
    }

    int64_t change = amountAvailable - amount - fee;
    std::printf("Creating spend transaction:\n To: %s\n Amount: %f\n Fee: %f\n Available: %f\n Change: %f\n", toAddress.c_str(), SatoshisToCoins(amount), SatoshisToCoins(fee), SatoshisToCoins(amountAvailable), SatoshisToCoins(change));
    std::cout.flush();
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
    BtcTxIdVouts unspentOutputs;
    BtcSigningPrerequisites signingPrerequisites;
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
    BtcTxTargets txTargets;
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
