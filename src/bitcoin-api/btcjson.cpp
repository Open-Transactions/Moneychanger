#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin-api/btcjson.hpp>

#include <json/json.h>

#include <iostream>
#include <map>
#include <cstdio>

#include "FastDelegate.hpp"
using namespace fastdelegate;

using namespace btc::json;

// https://en.bitcoin.it/wiki/Proper_Money_Handling_%28JSON-RPC%29

// bitcoin rpc methods
#define METHOD_GETINFO               "getinfo"
#define METHOD_GETBALANCE            "getbalance"
#define METHOD_GETACCOUNTADDRESS     "getaccountaddress"
#define METHOD_GETNEWADDRESS         "getnewaddress"
#define METHOD_IMPORTADDRESS         "importaddress"
#define METHOD_VALIDATEADDRESS       "validateaddress"
#define METHOD_DUMPPRIVKEY           "dumpprivkey"
#define METHOD_LISTACCOUNTS          "listaccounts"
#define METHOD_LISTRECEIVEDBYADDRESS "listreceivedbyaddress"
#define METHOD_LISTUNSPENT           "listunspent"
#define METHOD_LISTTRANSACTIONS      "listtransactions"
#define METHOD_SENDTOADDRESS         "sendtoaddress"
#define METHOD_SENDMANY              "sendmany"
#define METHOD_SETTXFEE              "settxfee"
#define METHOD_ADDMULTISIGADDRESS    "addmultisigaddress"
#define METHOD_CREATEMULTISIG        "createmultisig"
#define METHOD_GETTXOUT              "gettxout"
#define METHOD_GETTRANSACTION        "gettransaction"
#define METHOD_GETRAWTRANSACTION     "getrawtransaction"
#define METHOD_DECODERAWTRANSACTION  "decoderawtransaction"
#define METHOD_CREATERAWTRANSACTION  "createrawtransaction"
#define METHOD_SIGNRAWTRANSACTION    "signrawtransaction"
#define METHOD_SENDRAWTRANSACTION    "sendrawtransaction"
#define METHOD_GETRAWMEMPOOL         "getrawmempool"
#define METHOD_GETBLOCKCOUNT         "getblockcount"
#define METHOD_GETBLOCKHASH          "getblockhash"
#define METHOD_GETBLOCK              "getblock"
#define METHOD_SETGENERATE           "setgenerate"
#define METHOD_WALLETPASSPHRASE      "walletpassphrase"

/*
 * Methods requiring unlocked wallets:
 *
 *  dumpprivkey
 *  importprivkey (but not importaddress)
 *  keypoolrefill
 *  sendfrom
 *  sendmany
 *  sendtoaddress
 *  signmessage
 *  sometimes signrawtransaction
 *
 **/


int32_t BtcJson::walletUnlockTime = 2;

BtcJson::BtcJson(BtcModules *modules)
{
    this->modules = modules;

    this->passwordCallback = NULL;
}

BtcJson::~BtcJson()
{
    this->modules = NULL;
}

void BtcJson::Initialize()
{

}

void BtcJson::SetPasswordCallback(fastdelegate::FastDelegate0<std::string> callbackFunction)
{
    this->passwordCallback = callbackFunction;
}

bool BtcJson::SendJsonQuery(BtcRpcPacketPtr jsonString, Json::Value &result)
{
    Json::Value errorRef;
    std::string idRef;
    ProcessRpcString(this->modules->btcRpc->SendRpc(jsonString), idRef, errorRef, result);
    if(errorRef.isNull())
        return true;

    return ProcessError(errorRef, jsonString, result);
}

BtcRpcPacketPtr BtcJson::CreateJsonQuery(const std::string &command, const Json::Value &params, std::string id)
{
    if(id.empty())
        id = command;
    Json::Value jsonObj = Json::Value();
    jsonObj["jsonrpc"] = 1.0;
    jsonObj["id"] = id;
    jsonObj["method"] = command;
    jsonObj["params"] = params;
    //"{\"jsonrpc\": \"1.0\", \"id\":\"curltest\", \"method\": \"getinfo\", \"params\": [] }";

    Json::FastWriter writer;
    //Json::StyledWriter debugWriter;
    //OTLog::Output(0, debugWriter.write(jsonObj).c_str());
    return BtcRpcPacketPtr(new BtcRpcPacket(writer.write(jsonObj)));
}

bool BtcJson::ProcessRpcString(BtcRpcPacketPtr jsonString, Json::Value &result)
{
    std::string id;
    Json::Value error;
    ProcessRpcString(jsonString, id, error, result);

    return error.isNull();  // we should rather throw exceptions but I've never done that in c++ so this will have to do for now.
}

void BtcJson::ProcessRpcString(BtcRpcPacketPtr jsonString, std::string &id, Json::Value &error, Json::Value &result)
{
    if(jsonString == NULL || jsonString->GetData() == NULL || jsonString->size() <= 0)
    {
        error = Json::Value(true);
        return;
    }
    Json::Value replyObj;
    Json::Reader reader;
    if(!reader.parse(jsonString->GetData(), jsonString->GetData() + jsonString->size(), replyObj))
        return;

    if(replyObj.isNull() || replyObj.empty())
        return;

    // Outputs the entire JSON reply:
    Json::StyledWriter writer;
    std::printf("Received JSON:\n%s\n", writer.write(replyObj).c_str());
    std::cout.flush();

    if(replyObj.isObject())
    {
        Json::Value idVal = replyObj["id"];    // this is the same ID we sent to bitcoin- earlier.
        id = idVal.isString() ? idVal.asString() : "";      // assign to reference parameter

        error = replyObj["error"];
        if(!error.isNull())
        {
            std::printf("Error in reply to \"%s\": %s\n\n", id.c_str(), error.isObject() ? (error["message"]).asString().c_str() : "");
            std::cout.flush();
        }

        result = replyObj["result"];
    }
}

bool BtcJson::ProcessError(const Json::Value &error, BtcRpcPacketPtr jsonString, Json::Value &result)
{
    jsonString->ResetOffset();

    RPCErrorCode errorCode;
    if(error["code"].isInt())
        errorCode = static_cast<RPCErrorCode>(error["code"].asInt());
    else
        errorCode = static_cast<RPCErrorCode>(0);

    switch (errorCode)
    {
    case RPC_WALLET_UNLOCK_NEEDED:
    {
        if(this->passwordCallback == NULL)
            break;

        // see if we can get a password and retry
        std::string password = this->passwordCallback();
        if(password.empty())
            return false;

        if(WalletPassphrase(password, walletUnlockTime))
            return SendJsonQuery(jsonString, result);
        break;
    }
    case RPC_WALLET_ALREADY_UNLOCKED:
    {
        return true;    // everything went better than expected
        break;
    }
    default:
        break;
    }

    return false;
}

BtcInfoPtr BtcJson::GetInfo()
{
    BtcRpcPacketPtr reply = this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_GETINFO));
    std::string id;
    Json::Value error;
    Json::Value result;
    ProcessRpcString(reply, id, error, result);

    if(!error.isNull())
        return BtcInfoPtr();

    return BtcInfoPtr(new BtcInfo(result));
}

int64_t BtcJson::GetBalance(const std::string &account, const int32_t &minConfirmations, const bool &includeWatchonly)
{
    // note: json and bitcoind make a difference between NULL-strings and empty strings.

    Json::Value params = Json::Value();
    params.append(account);      // account
    params.append(minConfirmations);
    params.append(includeWatchonly);

    BtcRpcPacketPtr reply = this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_GETBALANCE, params));

    Json::Value result;
    if(!ProcessRpcString(reply, result) || !result.isDouble())
    {
        return 0;
    }

    return this->modules->btcHelper->CoinsToSatoshis(result.asDouble());
}

std::string BtcJson::GetAccountAddress(const std::string &account)
{
    Json::Value params = Json::Value();
    params.append(account);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_GETACCOUNTADDRESS, params)), result))
    {
        return std::string();     // error
    }

    if(!result.isString())
        return std::string();     // this should never happen unless the protocol was changed

    return result.asString();
}

btc::stringList BtcJson::GetAddressesByAccount(const std::string &account)
{
    // not yet implemented
    return btc::stringList();
}

std::string BtcJson::GetNewAddress(const std::string &account)
{
    Json::Value params = Json::Value();
    params.append(account);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_GETNEWADDRESS, params)), result))
    {
        return std::string();
    }

    if(!result.isString())
        return std::string();     // this should never happen unless the protocol was changed

    return result.asString();
}

bool BtcJson::ImportAddress(const std::string &address, const std::string &account, const bool &rescan)
{
    Json::Value params = Json::Value();
    params.append(address);
    params.append(account);
    params.append(rescan);

    Json::Value result = Json::Value();
    return ProcessRpcString(this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_IMPORTADDRESS, params)), result);
}

BtcAddressInfoPtr BtcJson::ValidateAddress(const std::string &address)
{
    Json::Value params = Json::Value();
    params.append(address);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_VALIDATEADDRESS, params)), result))
        return BtcAddressInfoPtr();

    if(!result.isObject())
        return BtcAddressInfoPtr();   // shouldn't happen unless protocol is changed

    BtcAddressInfoPtr addressInfo = BtcAddressInfoPtr(new BtcAddressInfo(result));
    return addressInfo;
}

std::string BtcJson::GetPublicKey(const std::string &address)
{
    BtcAddressInfoPtr addrInfo = ValidateAddress(address);
    if(addrInfo == NULL)
        return std::string();

    return addrInfo->pubkey;
}

std::string BtcJson::GetPrivateKey(const std::string &address)
{
    return DumpPrivKey(address);
}

std::string BtcJson::DumpPrivKey(const std::string &address)
{
    Json::Value params;
    params.append(address);

    Json::Value result = Json::Value();
    if(!SendJsonQuery(CreateJsonQuery(METHOD_DUMPPRIVKEY, params), result))
        return std::string();

    return result.asString();
}

BtcMultiSigAddressPtr BtcJson::AddMultiSigAddress(const uint32_t &nRequired, const btc::stringList &keys, const std::string &account)
{
    Json::Value params = Json::Value();
    params.append(nRequired);
    Json::Value keysValue;
    for(btc::stringList::const_iterator i = keys.begin(); i != keys.end(); i++)
    {
        keysValue.append(*i);
    }
    params.append(keysValue);
    params.append(account);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_ADDMULTISIGADDRESS, params)), result))
        return BtcMultiSigAddressPtr();      // error

    if(!result.isString())
        return BtcMultiSigAddressPtr();      // shouldn't happen unless protocol is changed

    if(result.asString().empty())
        return BtcMultiSigAddressPtr();     // error, shouldn't happen at all

    // re-create the address so we get more information and return that
    return CreateMultiSigAddress(nRequired, keys);
}

BtcMultiSigAddressPtr BtcJson::CreateMultiSigAddress(const uint32_t &nRequired, const btc::stringList &keys)
{
    Json::Value params = Json::Value();
    params.append(nRequired);
    Json::Value keysValue;
    for(btc::stringList::const_iterator i = keys.begin(); i != keys.end(); i++)
    {
        keysValue.append(*i);
    }
    params.append(keysValue);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_CREATEMULTISIG, params)),result))
        return BtcMultiSigAddressPtr();     // error

    if(!result.isObject())
        return BtcMultiSigAddressPtr();     // error

    BtcMultiSigAddressPtr multiSigAddr = BtcMultiSigAddressPtr(new BtcMultiSigAddress(result, keys));
    return multiSigAddr;
}

std::string BtcJson::GetRedeemScript(const uint32_t &nRequired, const btc::stringList &keys)
{
    // we can also get it from 'validateaddress' if we used 'addmultisigaddress'
    return CreateMultiSigAddress(nRequired, keys)->redeemScript;
}

btc::stringList BtcJson::ListAccounts(const int32_t &minConf, const bool &includeWatchonly)
{
    Json::Value params = Json::Value();
    params.append(minConf);
    params.append(includeWatchonly);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_LISTACCOUNTS, params)), result))
        return btc::stringList();     // error

    if(!result.isObject())
        return btc::stringList();        // this shouldn't happen unless the protocol was changed

    return result.getMemberNames();      // each key is an account, each value is the account's balance
}

BtcAddressBalances BtcJson::ListReceivedByAddress(const int32_t &minConf, const bool &includeEmpty, const bool &includeWatchonly)
{
    Json::Value params = Json::Value();
    params.append(minConf);
    params.append(includeEmpty);
    params.append(includeWatchonly);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_LISTRECEIVEDBYADDRESS, params)), result))
        return BtcAddressBalances();     // error

    if(!result.isArray())
        return BtcAddressBalances();

    BtcAddressBalances addressBalances;
    for (Json::Value::ArrayIndex i = 0; i < result.size(); i++)
    {
        addressBalances.push_back(BtcAddressBalancePtr(new BtcAddressBalance(result[i])));
    }

    return addressBalances;
}

BtcTransactions BtcJson::ListTransactions(const std::string &account, const int32_t &count, const int32_t &from, const bool &includeWatchonly)
{
    Json::Value params = Json::Value();
    params.append(account);
    params.append(count);
    params.append(from);
    params.append(includeWatchonly);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_LISTTRANSACTIONS, params)), result))
        return BtcTransactions();     // error

    if(!result.isArray())
        return BtcTransactions();

    BtcTransactions transactions;
    for (Json::Value::ArrayIndex i = 0; i < result.size(); i++)
    {
        transactions.push_back(BtcTransactionPtr(new BtcTransaction(result[i])));
    }

    return transactions;
}

BtcUnspentOutputs BtcJson::ListUnspent(const int32_t &minConf, const int32_t &maxConf, const btc::stringList &addresses)
{
    Json::Value params = Json::Value();
    params.append(minConf);
    params.append(maxConf);
    Json::Value addressesJson = Json::Value(Json::arrayValue);
    for (size_t i = 0; i < addresses.size(); i++)
    {
        addressesJson.append(addresses[i]);
    }
    params.append(addressesJson);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_LISTUNSPENT, params)),result))
        return BtcUnspentOutputs();     // error

    if(!result.isArray())
        return BtcUnspentOutputs();


    BtcUnspentOutputs outputs;

    for (Json::Value::ArrayIndex i = 0; i < result.size(); i++)
    {
        outputs.push_back(BtcUnspentOutputPtr(new BtcUnspentOutput(result[i])));
    }

    return outputs;
}



std::string BtcJson::SendToAddress(const std::string &btcAddress, const int64_t &amount)
{
    Json::Value params = Json::Value();
    params.append(btcAddress);
    params.append(this->modules->btcHelper->SatoshisToCoins(amount));

    Json::Value result = Json::Value();
    if(!SendJsonQuery(CreateJsonQuery(METHOD_SENDTOADDRESS, params), result))
        return std::string();   // error

    if(!result.isString())
        return std::string();    // shouldn't happen unless protocol was changed

    return result.asString();
}

bool BtcJson::SetTxFee(const int64_t &fee)
{
    Json::Value params = Json::Value();
    params.append(this->modules->btcHelper->SatoshisToCoins(fee));

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_SETTXFEE, params)),
                result))
        return false;

    return true;    // todo: check for more errors
}

std::string BtcJson::SendMany(BtcTxTargets txTargets, const std::string &fromAccount)
{


    Json::Value params = Json::Value();
    params.append(fromAccount);     // account to send coins from

    txTargets.ConvertSatoshisToBitcoin();
    params.append(txTargets);    // append map of addresses and btc amounts

    Json::Value result = Json::Value();
    if(!SendJsonQuery(CreateJsonQuery(METHOD_SENDMANY, params), result))
        return std::string();

    if(!result.isString())
        return std::string();

    return result.asString();
}

BtcUnspentOutputPtr BtcJson::GetTxOut(const std::string &txId, const int64_t &vout)
{
    Json::Value params = Json::Value();
    params.append(txId);
    params.append(static_cast<Json::Int64>(vout));

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETTXOUT, params)), result))
        return BtcUnspentOutputPtr();   // error

    if(!result.isObject())
        return BtcUnspentOutputPtr();   // error

    if(result.empty())
        return BtcUnspentOutputPtr();   // error

    BtcUnspentOutputPtr transaction = BtcUnspentOutputPtr(new BtcUnspentOutput(result));
    transaction->txId = txId;
    transaction->vout = vout;
    return transaction;
}

BtcTransactionPtr BtcJson::GetTransaction(const std::string &txId, const bool &includeWatchonly)
{
    Json::Value params = Json::Value();
    params.append(txId);
    params.append(includeWatchonly);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETTRANSACTION, params)), result))
        return BtcTransactionPtr();     // error

    if(!result.isObject())
        return BtcTransactionPtr();     // error

    BtcTransactionPtr transaction = BtcTransactionPtr(new BtcTransaction(result));
    return transaction;
}

std::string BtcJson::GetRawTransaction(const std::string &txId)
{
    Json::Value params = Json::Value();
    params.append(txId);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETRAWTRANSACTION, params)), result))
        return std::string();    // error

    if(!result.isString())
        return std::string();    // error

    return result.asString();
}

BtcRawTransactionPtr BtcJson::GetDecodedRawTransaction(const std::string &txId)
{
    Json::Value params = Json::Value();
    params.append(txId);
    params.append(1);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETRAWTRANSACTION, params)), result))
        return BtcRawTransactionPtr();  // return NULL

    if(!result.isObject())
        return BtcRawTransactionPtr();  // return NULL

    BtcRawTransactionPtr decodedRawTransaction = BtcRawTransactionPtr(new BtcRawTransaction(result));
    return decodedRawTransaction;


}

BtcRawTransactionPtr BtcJson::DecodeRawTransaction(const std::string &rawTransaction)
{
    Json::Value params = Json::Value();
    params.append(rawTransaction);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_DECODERAWTRANSACTION, params)), result))
        return BtcRawTransactionPtr();  // return NULL

    if(!result.isObject())
        return BtcRawTransactionPtr();  // return NULL

    BtcRawTransactionPtr decodedRawTransaction = BtcRawTransactionPtr(new BtcRawTransaction(result));
    return decodedRawTransaction;
}

std::string BtcJson::CreateRawTransaction(BtcTxIdVouts unspentOutputs, BtcTxTargets txTargets)
{
    Json::Value params = Json::Value();
    Json::Value outputsArray = Json::Value();
    for (size_t i = 0; i < unspentOutputs.size(); i++)
    {
        outputsArray.append(*unspentOutputs[i].get());
    }
    params.append(outputsArray);
    txTargets.ConvertSatoshisToBitcoin();   // divides amounts by 100.000.000 so don't forget this
    params.append(txTargets);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_CREATERAWTRANSACTION, params)), result))
        return std::string();  // error

    return result.asString();
}

BtcSignedTransactionPtr BtcJson::SignRawTransaction(const std::string &rawTransaction, const BtcSigningPrerequisites &previousTransactions, const btc::stringList &privateKeys)
{
    Json::Value params = Json::Value();
    params.append(rawTransaction);

    // add array of unspent outputs, scriptPubKeys and redeemScripts
    if(previousTransactions.size() > 0)
    {
        Json::Value unspentArray = Json::Value();
        for(BtcSigningPrerequisites::const_iterator i = previousTransactions.begin(); i != previousTransactions.end(); i++)
        {
            unspentArray.append((*(*i)));
        }
        params.append((unspentArray));
    }
    else
    {
        params.append(Json::Value(Json::nullValue));            // append null or else it won't work
    }

    // add array of private keys used to sign the transaction
    if(privateKeys.size() > 0)
    {
        Json::Value privKeysArray;
        for(btc::stringList::const_iterator i = privateKeys.begin(); i != privateKeys.end(); i++)
        {
            privKeysArray.append(*i);
        }
        params.append(privKeysArray);
    }
    else
        params.append(Json::Value(Json::nullValue));            // if no private keys were given, append null


    // there is an optional argument i didn't implement:
    // params.append(ALL|ANYONECANPAY, NONE|ANYONECANPAY, SINGLE|ANYONECANPAY)

    Json::Value result = Json::Value();
    if(!SendJsonQuery(CreateJsonQuery(METHOD_SIGNRAWTRANSACTION, params), result))
        return BtcSignedTransactionPtr();   // error

    if(!result.isObject())
        return BtcSignedTransactionPtr();   // error

    BtcSignedTransactionPtr signedTransaction = BtcSignedTransactionPtr(new BtcSignedTransaction(result));
    return signedTransaction;
}

BtcSignedTransactionPtr BtcJson::CombineSignedTransactions(const std::string &rawTransaction)
{
    Json::Value params = Json::Value();
    params.append(rawTransaction);  // a concatenation of partially signed tx's
    params.append(Json::Value(Json::arrayValue));    // dummy inputs and redeemscripts (must not be 'null')
    params.append(Json::Value(Json::arrayValue));    // dummy private keys (must not be 'null')
    // if dummy inputs are 'null', bitcoin will not just combine the raw transaction but do additional signing(!!!)

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_SIGNRAWTRANSACTION, params)), result))
        return BtcSignedTransactionPtr();   // error

    BtcSignedTransactionPtr signedTransaction = BtcSignedTransactionPtr(new BtcSignedTransaction(result));
    return signedTransaction;
}

std::string BtcJson::SendRawTransaction(const std::string &rawTransaction, const bool &allowHighFees)
{
    Json::Value params = Json::Value();
    params.append(rawTransaction);
    if(allowHighFees)
        params.append(allowHighFees);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_SENDRAWTRANSACTION, params)), result))
        return std::string();  // error

    return result.asString();
}

btc::stringList BtcJson::GetRawMemPool()
{
    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETRAWMEMPOOL, Json::Value())), result))
        return std::vector<std::string>();

    if(!result.isArray())
        return std::vector<std::string>();

    std::vector<std::string> rawMemPool = std::vector<std::string>();
    for (Json::Value::ArrayIndex i = 0; i < result.size(); i++)
    {
        rawMemPool.push_back(result[i].asString());
    }
    return rawMemPool;
}

int BtcJson::GetBlockCount()
{
    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETBLOCKCOUNT, Json::Value())), result))
        return -1;   // we should throw errors instead.

    return (int)result.asInt();
}

std::string BtcJson::GetBlockHash(const int32_t &blockNumber)
{
    Json::Value params = Json::Value();
    params.append(blockNumber);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETBLOCKHASH, params)), result))
        return std::string();

    return result.asString();
}

BtcBlockPtr BtcJson::GetBlock(const std::string &blockHash)
{
    Json::Value params = Json::Value();
    params.append(blockHash);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETBLOCK, params)), result))
        return BtcBlockPtr();

    if(!result.isObject())
        return BtcBlockPtr();

    BtcBlockPtr block = BtcBlockPtr(new BtcBlock(result));
    return block;
}

bool BtcJson::SetGenerate(const bool &generate)
{
    Json::Value params = Json::Value();
    params.append(generate);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_SETGENERATE, params)), result))
        return false;

    return true;
}

bool BtcJson::WalletPassphrase(const std::string &password, const time_t &unlockTime)
{
    Json::Value params = Json::Value();
    params.append(password);
    params.append(static_cast<Json::Int64>(unlockTime));

    Json::Value result = Json::Value();
    if(!SendJsonQuery(CreateJsonQuery(METHOD_WALLETPASSPHRASE, params), result))
        return false;

    return true;
}

void BtcJson::UnlockWallet()
{
    if(this->passwordCallback.empty())
        return;

    WalletPassphrase(this->passwordCallback(), walletUnlockTime);
}
