#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin-api/btcjson.hpp>

#include <json/json.h>

#include <iostream>
#include <map>
#include <cstdio>

// https://en.bitcoin.it/wiki/Proper_Money_Handling_%28JSON-RPC%29 on how to avoid rounding errors and such. might be worth a read someday.

// bitcoin rpc methods
#define METHOD_GETINFO              "getinfo"
#define METHOD_GETBALANCE           "getbalance"
#define METHOD_GETACCOUNTADDRESS    "getaccountaddress"
#define METHOD_GETNEWADDRESS        "getnewaddress"
#define METHOD_VALIDATEADDRESS      "validateaddress"
#define METHOD_DUMPPRIVKEY          "dumpprivkey"
#define METHOD_LISTACCOUNTS         "listaccounts"
#define METHOD_LISTUNSPENT          "listunspent"
#define METHOD_SENDTOADDRESS        "sendtoaddress"
#define METHOD_SENDMANY             "sendmany"
#define METHOD_SETTXFEE             "settxfee"
#define METHOD_ADDMULTISIGADDRESS   "addmultisigaddress"
#define METHOD_CREATEMULTISIG       "createmultisig"
#define METHOD_GETTRANSACTION       "gettransaction"
#define METHOD_GETRAWTRANSACTION    "getrawtransaction"
#define METHOD_DECODERAWTRANSACTION "decoderawtransaction"
#define METHOD_CREATERAWTRANSACTION "createrawtransaction"
#define METHOD_SIGNRAWTRANSACTION   "signrawtransaction"
#define METHOD_SENDRAWTRANSACTION   "sendrawtransaction"
#define METHOD_GETRAWMEMPOOL        "getrawmempool"
#define METHOD_GETBLOCKCOUNT        "getblockcount"
#define METHOD_GETBLOCKHASH         "getblockhash"
#define METHOD_GETBLOCK             "getblock"


BtcJson::BtcJson(BtcModules *modules)
{
    this->modules = modules;
}

BtcJson::~BtcJson()
{
    this->modules = NULL;
}

void BtcJson::Initialize()
{

}

BtcRpcPacketPtr BtcJson::CreateJsonQuery(const std::string &command, Json::Value params, std::string id)
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
        return;
    Json::Value replyObj;
    Json::Reader reader;
    if(!reader.parse(jsonString->GetData(), jsonString->GetData() + jsonString->size() - 1, replyObj))
        return;

    if(replyObj.isNull() || replyObj.empty())
        return;

    Json::StyledWriter writer;
    printf("Received JSON:\n%s\n", writer.write(replyObj).c_str());
    std::cout.flush();

    if(replyObj.isObject())
    {
        Json::Value idVal = replyObj["id"];    // this is the same ID we sent to bitcoin- earlier.
        id = idVal.isString() ? idVal.asString() : "";      // assign to reference parameter

        error = replyObj["error"];
        if(!error.isNull())
        {
            printf("Error in reply to \"%s\": %s\n\n", id.c_str(), error.isObject() ? (error["message"]).asString().c_str() : "");
            std::cout.flush();
        }

        result = replyObj["result"];
    }
}

void BtcJson::GetInfo()
{
    BtcRpcPacketPtr reply = this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_GETINFO));
    std::string id;
    Json::Value error;
    Json::Value result;
    ProcessRpcString(reply, id, error, result);

    if(!error.isNull())
        return;

    // TODO:: what is happening here!? (da2ce7)
    // Bitcoin is returning the balance as double, e.g. 5.123 bitcoins, whereas we want it as an integer denominated in satoshis to avoid rounding errors.
    // I'm not doing anything with the result because i haven't found any use for it yet. (JaSK)
    uint64_t balance = this->modules->btcHelper->CoinsToSatoshis(result["balance"].asDouble());
}

int64_t BtcJson::GetBalance(const char *account/*=NULL*/)
{
    // note: json and bitcoin- make a difference between NULL-strings and empty strings.

    Json::Value params = Json::Value();
    params.append(account != NULL ? account : Json::Value());      // account
    //params.append(1);       // min confirmations, 1 is default, we probably don't need this line.

    BtcRpcPacketPtr reply = this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_GETBALANCE));

    Json::Value result;
    if(!ProcessRpcString(reply, result) || !result.isDouble())
    {
        return 0;  // error, TODO: throw error or return NaN
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
        return "";     // error
    }

    if(!result.isString())
        return "";     // this should never happen unless the protocol was changed

    return result.asString();
}

std::stringList BtcJson::GetAddressesByAccount(const std::string &account)
{
    // not yet implemented
    return std::stringList();
}

std::string BtcJson::GetNewAddress(const std::string &account)
{
    Json::Value params = Json::Value();
    params.append(account);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_GETNEWADDRESS, params)), result))
    {
        return "";
    }

    if(!result.isString())
        return "";     // this should never happen unless the protocol was changed

    return result.asString();
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
        return "";

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
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_DUMPPRIVKEY, params)),
                result))
        return "";

    return result.asString();
}

BtcMultiSigAddressPtr BtcJson::AddMultiSigAddress(int nRequired, const std::list<std::string> &keys, const std::string &account)
{
    Json::Value params = Json::Value();
    params.append(nRequired);
    Json::Value keysValue;
    for(std::list<std::string>::const_iterator i = keys.begin(); i != keys.end(); i++)
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

BtcMultiSigAddressPtr BtcJson::CreateMultiSigAddress(int nRequired, const std::list<std::string> &keys)
{
    Json::Value params = Json::Value();
    params.append(nRequired);
    Json::Value keysValue;
    for(std::list<std::string>::const_iterator i = keys.begin(); i != keys.end(); i++)
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

std::string BtcJson::GetRedeemScript(int nRequired, std::list<std::string> keys)
{
    return CreateMultiSigAddress(nRequired, keys)->redeemScript;
}

std::vector<std::string> BtcJson::ListAccounts()
{
    Json::Value result = Json::Value();
    if(!ProcessRpcString(this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_LISTACCOUNTS)), result))
        return std::vector<std::string>();     // error

    if(!result.isObject())
        return std::vector<std::string>();        // this shouldn't happen unless the protocol was changed

    return result.getMemberNames();      // each key is an account, each value is the account's balance
}

BtcUnspentOutputs BtcJson::ListUnspent(int minConf, int maxConf, std::vector<std::string> addresses)
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
        return std::vector<BtcUnspentOutputPtr>();     // error

    if(!result.isArray())
        return std::vector<BtcUnspentOutputPtr>();


    std::vector<BtcUnspentOutputPtr> outputs = std::vector<BtcUnspentOutputPtr>();

    for (Json::Value::ArrayIndex i = 0; i < result.size(); i++)
    {
        outputs.push_back(BtcUnspentOutputPtr(new BtcUnspentOutput(result[i])));
    }

    return outputs;
}

std::string BtcJson::SendToAddress(const std::string &btcAddress, int64_t amount)
{
    // TODO: handle lack of funds, need of transaction fees and unlocking of the wallet

    Json::Value params = Json::Value();
    params.append(btcAddress);
    params.append(this->modules->btcHelper->SatoshisToCoins(amount));

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_SENDTOADDRESS, params)),result))
        return "";   // error

    if(!result.isString())
        return "";    // shouldn't happen unless protocol was changed

    return result.asString();
}

bool BtcJson::SetTxFee(int64_t fee)
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

std::string BtcJson::SendMany(BtcTxTarget txTargets, const std::string &fromAccount)
{
    Json::Value params = Json::Value();
    params.append(fromAccount);     // account to send coins from

    txTargets.ConvertSatoshisToBitcoin();
    params.append(txTargets);    // append map of addresses and btc amounts

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_SENDMANY, params)),
                result))
        return "";

    if(!result.isString())
        return "";

    return result.asString();
}

BtcTransactionPtr BtcJson::GetTransaction(std::string txID)
{
    // TODO: maybe we can automate the process of appending arguments
    //      and calling SendRPC(CreateJsonQuery..) as it's
    //      virtually the same code in every function.
    Json::Value params = Json::Value();
    params.append(txID);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETTRANSACTION, params)), result))
        return BtcTransactionPtr();     // error

    if(!result.isObject())
        return BtcTransactionPtr();     // error

    BtcTransactionPtr transaction = BtcTransactionPtr(new BtcTransaction(result));
    return transaction;

    // TODO:
    // for checking balance see "details"->received, confirmations, amount (and address(es)?)

    // also check what happens in multi-sig-transactions.
}

std::string BtcJson::GetRawTransaction(std::string txID)
{
    Json::Value params = Json::Value();
    params.append(txID);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETRAWTRANSACTION, params)), result))
        return "";    // error

    if(!result.isString())
        return "";    // error

    return result.asString();
}

BtcRawTransactionPtr BtcJson::GetDecodedRawTransaction(std::string txID)
{
    Json::Value params = Json::Value();
    params.append(txID);
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

BtcRawTransactionPtr BtcJson::DecodeRawTransaction(std::string rawTransaction)
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

std::string BtcJson::CreateRawTransaction(BtcTxIdVouts unspentOutputs, BtcTxTarget txTargets)
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
        return "";  // error

    return result.asString();
}

BtcSignedTransactionPtr BtcJson::SignRawTransaction(const std::string &rawTransaction, const std::list<BtcSigningPrerequisite> &previousTransactions, const std::stringList &privateKeys)
{
    Json::Value params = Json::Value();
    params.append(rawTransaction);

    // add array of unspent outputs
    if(previousTransactions.size() > 0)
    {
        Json::Value unspentArray = Json::Value();
        for(std::list<BtcSigningPrerequisite>::const_iterator i = previousTransactions.begin(); i != previousTransactions.end(); i++)
        {
            unspentArray.append(*i);
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
        Json::Value privKeysArray;               // TODO: figure out how to properly parse a string list
        for(std::stringList::const_iterator i = privateKeys.begin(); i != privateKeys.end(); i++)
        {
            privKeysArray.append(*i);
        }
        params.append(privKeysArray);
    }
    else
        params.append(Json::Value(Json::nullValue));            // if no private keys were given, append null


    // params.append(ALL|ANYONECANPAY, NONE|ANYONECANPAY, SINGLE|ANYONECANPAY)

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_SIGNRAWTRANSACTION, params)), result))
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

std::string BtcJson::SendRawTransaction(const std::string &rawTransaction)
{
    Json::Value params = Json::Value();
    params.append(rawTransaction);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_SENDRAWTRANSACTION, params)), result))
        return "";  // error

    return result.asString();
}

std::vector<std::string> BtcJson::GetRawMemPool()
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

std::string BtcJson::GetBlockHash(int blockNumber)
{
    Json::Value params = Json::Value();
    params.append(blockNumber);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETBLOCKHASH, params)), result))
        return "";

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
