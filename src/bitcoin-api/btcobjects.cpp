#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin-api/btcobjects.hpp>

#include <bitcoin-api/btchelper.hpp>

#include <opentxs/OTAssert.hpp>

#include <cstdlib>
#include <cstdio>


BtcInfo::BtcInfo(Json::Value result)
{
    version = result["version"].asInt();
    protocolversion = result["protocolversion"].asInt();
    walletversion = result["walletversion"].asInt();
    balance = BtcHelper::CoinsToSatoshis(result["balance"].asDouble());
    blocks = result["blocks"].asInt();
    timeoffset = result["timeoffset"].asInt64();
    connections = result["connections"].asInt();
    proxy = result["proxy"].asString();
    difficulty = result["difficulty"].asInt64();
    testnet = result["testnet"].asBool();
    keypoololdest = result["kepyoololdest"].asInt64();
    keypoolsize = result["keypoolsize"].asInt();
    unlocked_until = result["unlocked_until"].asInt64();
    paytxfee = BtcHelper::CoinsToSatoshis(result["paytxfee"].asDouble());
    relayfee = BtcHelper::CoinsToSatoshis(result["relayfee"].asDouble());
    errors = result["errors"].asString();
}

BtcTxDetail::BtcTxDetail(const Json::Value &detail)
{
    if(!detail.isObject())
    {
        this->involvesWatchonly = false;
        this->account = std::string();
        this->address = std::string();
        this->category = std::string();
        this->amount = 0;
        this->fee = 0;
        return;
    }
    this->involvesWatchonly = detail["involvesWatchonly"].asBool();
    this->account = detail["account"].asString();
    this->address = detail["address"].asString();
    this->category = detail["category"].asString();
    this->amount = BtcHelper::CoinsToSatoshis(detail["amount"].asDouble());
    this->fee = BtcHelper::CoinsToSatoshis(detail["fee"].asDouble());
}

BtcTransaction::BtcTransaction(Json::Value reply)
{
    SetDefaults();

    if(!reply.isObject() || !reply["error"].isNull())
        return;

    this->Amount = BtcHelper::CoinsToSatoshis(reply["amount"].asDouble());
    this->Fee = BtcHelper::CoinsToSatoshis(reply["fee"].asDouble());
    this->Confirmations = reply["confirmations"].asInt();
    this->Blockhash = reply["blockhash"].asString();
    this->BlockIndex = reply["blockindex"].asInt();
    this->Blocktime = reply["blocktime"].asInt64();
    this->TxId = reply["txid"].asString();
    this->Time = reply["time"].asInt64();
    this->TimeReceived = reply["timereceived"].asInt64();
    this->Hex = reply["hex"].asString();

    if(reply["walletconflicts"].isArray())
    {
        for(Json::ArrayIndex i = 0; i < reply["walletconflicts"].size(); i++)
        {
            this->walletConflicts.push_back(reply["walletconflicts"][i].asString());
        }
    }

    // details array returned by 'gettransaction'
    if(reply["details"].isArray() && reply["details"].size() > 0)
    {
        Json::Value details = reply["details"];
        for (Json::Value::ArrayIndex i = 0; i < details.size(); i++)
        {
            if(!details[i].isObject())
                continue;
            this->Details.push_back(BtcTxDetailPtr(new BtcTxDetail(details[i])));
        }
    }
    else    // details directly contained in reply when calling 'listtransactions'
    {
        BtcTxDetailPtr detail = BtcTxDetailPtr(new BtcTxDetail(reply));
        this->Details.push_back(detail);
    }
}

void BtcTransaction::SetDefaults()
{
    this->Amount = 0;
    this->Fee = 0;
    this->Confirmations = 0;
    this->Blockhash = std::string();
    this->BlockIndex = 0;
    this->Blocktime = 0;
    this->TxId = std::string();
    this->Time = 0;
    this->TimeReceived = 0;
    this->Hex = std::string();
    this->Details = BtcTxDetails();
}

BtcRawTransaction::BtcRawTransaction(Json::Value rawTx)
{
    this->inputs = std::vector<VIN>();
    this->outputs = std::vector<VOUT>();

    this->txId = rawTx["txid"].asString();
    this->Version = rawTx["version"].asInt();
    this->LockTime = rawTx["locktime"].asInt();

    // inputs
    Json::Value vins = rawTx["vin"];
    for (Json::Value::ArrayIndex i = 0; i < vins.size(); i++)
    {
        Json::Value inputObj = vins[i];
        if(!inputObj["scriptSig"].isObject())
            continue;
        this->inputs.push_back(VIN(inputObj["txid"].asString(),
                                    inputObj["vout"].asUInt(),
                                    inputObj["scriptSig"]["hex"].asString(),
                                    inputObj["sequence"].asInt64()));
    }

    // outputs
    Json::Value vouts  = rawTx["vout"];
    for (Json::Value::ArrayIndex i = 0; i < vouts.size(); i++)
    {
        Json::Value outputObj = vouts[i];
        Json::Value scriptPubKey = outputObj["scriptPubKey"];

        Json::Value addresses = scriptPubKey["addresses"];
        btc::stringList addresslist;
        for (Json::Value::ArrayIndex i = 0; i < addresses.size(); i++)
        {
            addresslist.push_back(addresses[i].asString());
        }

        VOUT output = VOUT(BtcHelper::CoinsToSatoshis(outputObj["value"].asDouble()),
                            outputObj["n"].asInt64(),
                            scriptPubKey["hex"].asString(),
                            scriptPubKey["reqSigs"].asUInt(),
                            scriptPubKey["type"].asString(),
                            addresslist);

        this->outputs.push_back(output);
    }

}

BtcUnspentOutput::BtcUnspentOutput(Json::Value unspentOutput)
{
    Json::Value scriptPubKeyVal = unspentOutput["scriptPubKey"];
    if(scriptPubKeyVal.isObject())
    {
        this->scriptPubKey = scriptPubKeyVal["hex"].asString();

        // 'gettxout' returns addresses[] as array
        Json::Value addresses = scriptPubKeyVal["addresses"];
        if(!addresses.empty() && addresses.size() == 1)
        {
            Json::Value address = Json::Value(addresses[0]);
            if(address.isString())
            {
                this->address = address.asString();
            }
            else
            {
                this->address = std::string();
            }
        }
        else
        {
            this->address = std::string();
        }

        this->amount = BtcHelper::CoinsToSatoshis(unspentOutput["value"].asDouble());
    }
    else if(scriptPubKeyVal.isString())
    {
        this->address = unspentOutput["address"].asString();
        this->scriptPubKey = scriptPubKeyVal.asString();
        this->amount = BtcHelper::CoinsToSatoshis(unspentOutput["amount"].asDouble());
    }
    else
    {
        this->scriptPubKey = std::string();
    }

    this->txId = unspentOutput["txid"].asString();
    this->vout = unspentOutput["vout"].asInt64();
    this->account = unspentOutput["account"].asString();
    this->redeemScript = unspentOutput["redeemScript"].asString();
    this->confirmations = unspentOutput["confirmations"].asInt64();
    this->spendable = unspentOutput["spendable"].asBool();
}

BtcAddressBalance::BtcAddressBalance(Json::Value addressBalance)
{
    if(!addressBalance.isObject())
        addressBalance = Json::Value(Json::objectValue);
    this->involvesWatchonly = addressBalance["involvesWatchonly"].asBool();
    this->address = addressBalance["address"].asString();
    this->account = addressBalance["account"].asString();
    this->amount = BtcHelper::CoinsToSatoshis(addressBalance["amount"].asDouble());
    this->confirmations = addressBalance["confirmations"].asInt();
    this->txIds = btc::stringList();
    Json::Value txids = addressBalance["txids"];
    for(Json::ArrayIndex i = 0; i < txids.size(); i++)
    {
        this->txIds.push_back(txids[i].asString());
    }
}

BtcAddressInfo::BtcAddressInfo(Json::Value result)
{
    this->sigsRequired = 0;

    // it seems we don't need to do any error checking. thanks, .
    this->isvalid = result["isvalid"].asBool();
    this->address = result["address"].asString();
    this->ismine = result["ismine"].asBool();
    this->isWatchonly = result["iswatchonly"].asBool();
    this->isScript = result["isscript"].asBool();

    // regular addresses
    this->pubkey = result["pubkey"].asString();
    this->isCompressed = result["iscompressed"].asBool();

    // p2sh:
    this->script = result["script"].asString();
    this->redeemScript = result["hex"].asString();
    this->addresses = btc::stringList();
    for(Json::ArrayIndex i = 0; i < result["addresses"].size(); i++)
    {
        this->addresses.push_back(result["addresses"][i].asString());
    }
    this->sigsRequired = result["sigsrequired"].asUInt();

    this->account = result["account"].asString();
}

BtcMultiSigAddress::BtcMultiSigAddress(Json::Value result, const btc::stringList &publicKeys)
{
    this->address = result["address"].asString();
    this->redeemScript = result["redeemScript"].asString();

    this->publicKeys = publicKeys;
}

BtcBlock::BtcBlock()
{
    this->confirmations = 0;
    this->transactions = btc::stringList();
    this->height = 0;
    this->hash = std::string();
    this->previousHash = std::string();
}

BtcBlock::BtcBlock(Json::Value block)
{
    // latest block has 1 confirmations I think so tx->confirms == block->confirms
    this->confirmations = block["confirmations"].asInt64();

    // block number (count since genesis)
    this->height = block["height"].asInt64();

    this->hash = block["hash"].asString();
    this->previousHash = block["previousblockhash"].asString();

    // get list of transactions in the block
    Json::Value transacts = block["tx"];
    for (Json::Value::ArrayIndex i = 0; i < transacts.size(); i++)
    {
        this->transactions.push_back(transacts[i].asString());
    }
}

BtcTxIdVout::BtcTxIdVout(const std::string &txID, const int64_t &vout)
{
    (*this)["txid"] = txID;
    (*this)["vout"] = static_cast<Json::Int64>(vout);
}

BtcTxTargets::BtcTxTargets()
{

}

BtcTxTargets::BtcTxTargets(const std::string &toAddress, int64_t amount)
{
    (*this)[toAddress] = static_cast<Json::Int64>(amount);
}

void BtcTxTargets::SetTarget(const std::string &toAddress, int64_t amount)
{
    (*this)[toAddress] = static_cast<Json::Int64>(amount);
}

void BtcTxTargets::ConvertSatoshisToBitcoin()
{
    Members targetAddresses = this->getMemberNames();
    for (Json::Value::ArrayIndex i = 0; i < targetAddresses.size(); i++)
    {
        (*this)[targetAddresses[i]] = BtcHelper::SatoshisToCoins(
                    (*this)[targetAddresses[i]].asInt64());
    }
}

BtcSignedTransaction::BtcSignedTransaction(Json::Value signedTransactionObj)
{
    this->signedTransaction = signedTransactionObj["hex"].asString();
    this->complete = signedTransactionObj["complete"].asBool();
}

BtcSigningPrerequisite::BtcSigningPrerequisite()
{

}

BtcSigningPrerequisite::BtcSigningPrerequisite(const std::string &txId, const int64_t &vout, const std::string &scriptPubKey, const std::string &redeemScript)
{
    // all of these values must be set or else prerequisite is invalid
    // or maybe not. maybe there's a difference between regular and multisig. the more you know...

    (*this)["txid"] = txId;
    (*this)["vout"] = static_cast<Json::Int64>(vout);
    if(!scriptPubKey.empty())
        (*this)["scriptPubKey"] = scriptPubKey;
    if(!redeemScript.empty())
        (*this)["redeemScript"] = redeemScript;
}

BtcRpcPacket::BtcRpcPacket()
{
    SetDefaults();
    return;
}

BtcRpcPacket::BtcRpcPacket(const std::string &strData)
    :data(strData.begin(), strData.end()), pointerOffset(0)
{
    if(*(data.end()-1) != '\0')
        this->data.push_back('\0');
}

BtcRpcPacket::BtcRpcPacket(const BtcRpcPacketPtr packet)
    : data(packet->data.begin(), packet->data.end()), pointerOffset(0)
{
    if(*(data.end()-1) != '\0')
        this->data.push_back('\0');
}

BtcRpcPacket::~BtcRpcPacket()
{
    this->data.clear();
    this->pointerOffset = 0;
}

void BtcRpcPacket::SetDefaults()
{
    this->data.clear();
    this->data.push_back('\0');
    this->pointerOffset = 0;
}

void BtcRpcPacket::ResetOffset()
{
    this->pointerOffset = 0;
}

bool BtcRpcPacket::AddData(const std::string strData)
{
    // cut off the trailing '\0' as otherwise multipart messages won't work:
    std::string data(this->data.begin(), this->data.end() - 1);
    data.append(strData.begin(), strData.end());
    this->data.clear();
    this->data = std::vector<char>(data.begin(), data.end());
    if(*(data.end()-1) != '\0')
        this->data.push_back('\0');

    return true;
}

const char *BtcRpcPacket::ReadNextChar()
{
    if (this->pointerOffset < size())
        return &this->data.at(this->pointerOffset++);
    else return NULL;
}

size_t BtcRpcPacket::size()
{
    return this->data.size()-1;
}

const char* BtcRpcPacket::GetData()
{
    return &*this->data.begin();
}
















