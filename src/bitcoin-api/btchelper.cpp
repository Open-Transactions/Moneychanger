#include "btchelper.h"
#include "btcmodules.h"
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <string>

#ifndef OT_USE_TR1
    #include <thread>   // for sleep()
#else
    #include <unistd.h> // usleep
    #include <sstream>  // ostringstream
#endif // OT_USE_TR1

void Sleep(int milliSeconds)
{
#ifndef OT_USE_TR1
    std::this_thread::sleep_for(std::chrono::milliseconds(milliSeconds));
    std::this_thread::yield();  // let's also free some CPU because who cares about a few ms here?
#else
    usleep(milliSeconds * 1000);
#endif // OT_USE_TR1
}

namespace btc   // will probably put everything into this namespace
{
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


int BtcHelper::MinConfirms = 1;

int BtcHelper::FeeMultiSig = BtcHelper::CoinsToSatoshis(0.001);

BtcHelper::BtcHelper(BtcModules *modules)
{
    this->modules = modules;

    // i think the compiler removes the function body if it is not referenced at least once.
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
    for(uint i = 0; i < transaction->outputs.size(); i++)
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

int64_t BtcHelper::TransactionConfirmed(const std::string &txId, int minConfirms)
{
    return GetConfirmations(txId) >= minConfirms;
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

BtcRawTransactionPtr BtcHelper::WaitGetRawTransaction(const std::string &txID, int timerMS, int maxAttempts)
{
    BtcRawTransactionPtr transaction;

    // TODO: see WaitGetTransaction()
    while((transaction = this->modules->btcJson->GetDecodedRawTransaction(txID)) == NULL && maxAttempts--)
    {
        Sleep(timerMS);
    }

    return transaction;
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
    std::list<BtcSigningPrequisite> signingPrequisites;
    for(uint i = 0; i < rawTransaction->outputs.size(); i++)
    {
        BtcRawTransaction::VOUT output = rawTransaction->outputs[i];

        // check if output leads to sourceAddess
        // idk what multiple addresses per output means so we'll skip those cases
        if(output.addresses.size() == 1 && output.addresses[0] == sourceAddress)
        {
            funds += output.value;
            unspentOutputs.push_back(BtcTxIdVoutPtr(new BtcTxIdVout(txToSourceId, output.n)));

            // create signing prequisite (optional, needed for offline signing)
            if(!signingAddress.empty() && !redeemScript.empty())
                signingPrequisites.push_back(BtcSigningPrequisite(txToSourceId, output.n, output.scriptPubKeyHex, redeemScript));
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
    std::stringList signingKeys;
    if(!signingAddress.empty() /*&& !redeemScript.empty()*/)
    {
        signingKeys.push_back(this->modules->btcJson->GetPrivateKey(signingAddress));
    }

    // Note: signingPrequisites can be empty, in that case bitcoin will sign with any key that fits.
    // this should only be done with locally generated transactions or transactions whose outputs were checked first
    // because otherwise someone could give you a transaction to send funds from your wallet to his and you'd blindly sign it.
    // if a signingAddress is passed, redeemScript is required aswell, at least for p2sh.

    // sign raw transaction
    // as we just created this tx ourselves, we can assume that it is safe to sign
    BtcSignedTransactionPtr signedTransact = this->modules->btcJson->SignRawTransaction(withdrawTransaction, signingPrequisites, signingKeys);

    return signedTransact;
}
