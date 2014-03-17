#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin-api/btcobjects.hpp>

#include <bitcoin-api/btchelper.hpp>

#include <opentxs/OTAssert.hpp>

#include <cstdlib>
#include <cstdio>

BtcTransaction::BtcTransaction(Json::Value reply)
{
    SetDefaults();

    if(!reply["error"].isNull())
        return;

    this->Confirmations = reply["confirmations"].asInt64();
    this->Amount = BtcHelper::CoinsToSatoshis(reply["amount"].asDouble());

    this->Fee = BtcHelper::CoinsToSatoshis(reply["fee"].asDouble());
    this->TxID = reply["txid"].asString();
    this->Time = reply["time"].asDouble();

    // details
    if(!reply["details"].isArray())
        return;
    Json::Value details = reply["details"];
    if(details.size() == 0)
        return;

    for (Json::Value::ArrayIndex i = 0; i < details.size(); i++)
    {
        Json::Value detail = details[i];
        std::string address = detail["address"].asString();
        std::string category = detail["category"].asString();
        int64_t amount = BtcHelper::CoinsToSatoshis(detail["amount"].asDouble());
        if(category == "send")
        {
            this->AddressesSent.push_back(address);
            this->AmountSent += amount; // will be 0 or less
        }
        else if(category == "receive")
        {
            this->AddressesRecv.push_back(address);
            this->AmountReceived += amount; // will be 0 or more
        }
        else if(category == "immature")
        {
            // that's block reward.
        }
    }
}

void BtcTransaction::SetDefaults()
{
    //TotalAmount = 0.0;
    this->Confirmations = 0;
    this->AmountReceived = 0.0;
    this->AmountSent = 0.0;
    this->Amount = 0.0;
    this->Fee = 0.0;
    this->TxID = std::string();
    this->Time = 0;
    this->AddressesRecv = std::list<std::string>();
    this->AddressesSent = std::list<std::string>();
}

BtcRawTransaction::BtcRawTransaction(Json::Value rawTx)
{
    this->inputs = std::vector<VIN>();
    this->outputs = std::vector<VOUT>();

    this->txID = rawTx["txid"].asString();

    Json::Value vin = rawTx["vin"];
    for (Json::Value::ArrayIndex i = 0; i < vin.size(); i++)
    {
        Json::Value inputObj = vin[i];
        this->inputs.push_back(VIN(inputObj["txid"].asString(), inputObj["vout"].asInt64()));
    }

    Json::Value vouts  = rawTx["vout"];
    for (Json::Value::ArrayIndex i = 0; i < vouts.size(); i++)
    {
        Json::Value outputObj = vouts[i];
        VOUT output;

        output.value = BtcHelper::CoinsToSatoshis(outputObj["value"].asDouble());
        output.n = outputObj["n"].asInt64();      // JSON doesn't know integers

        Json::Value scriptPubKey = outputObj["scriptPubKey"];
        output.reqSigs = scriptPubKey["reqSigs"].asInt64();
        Json::Value addresses = scriptPubKey["addresses"];
        for (Json::Value::ArrayIndex i = 0; i < addresses.size(); i++)
        {
            output.addresses.push_back(addresses[i].asString());
        }
        output.scriptPubKeyHex = scriptPubKey["hex"].asString();

        this->outputs.push_back(output);
    }

}

BtcUnspentOutput::BtcUnspentOutput(Json::Value unspentOutput)
{
    this->txId = unspentOutput["txid"].asString();
    this->vout = unspentOutput["vout"].asInt64();
    this->address = unspentOutput["address"].asString();
    this->account = unspentOutput["account"].asString();
    this->scriptPubKey = unspentOutput["scriptPubKey"].asString();
    this->amount = BtcHelper::CoinsToSatoshis(unspentOutput["amount"].asDouble());
    this->confirmations = unspentOutput["confirmations"].asInt64();
}

BtcAddressInfo::BtcAddressInfo(Json::Value result)
{
    this->sigsRequired = 0;

    // it seems we don't need to do any error checking. thanks, .
    this->address = result["address"].asString();   // if wrong type, default value will be returned
    this->pubkey = result["pubkey"].asString();
    this->account = result["account"].asString();
    this->ismine = result["ismine"].asBool();
    this->isvalid = result["isvalid"].asBool();

    // multi-sig properties:
    this->isScript = result["isscript"].asBool();
    this->addresses = result["addresses"];
}

BtcMultiSigAddress::BtcMultiSigAddress(Json::Value result, const std::stringList &publicKeys)
{
    this->address = result["address"].asString();
    this->redeemScript = result["redeemScript"].asString();

    this->publicKeys = publicKeys;
}

BtcBlock::BtcBlock()
{
    this->confirmations = 0;
    this->transactions = std::list<std::string>();
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

BtcTxIdVout::BtcTxIdVout(std::string txID, int64_t vout)
{
    (*this)["txid"] = txID;
    (*this)["vout"] = static_cast<Json::Int64>(vout);
}

BtcTxTarget::BtcTxTarget()
{

}

BtcTxTarget::BtcTxTarget(const std::string &toAddress, int64_t amount)
{
    (*this)[toAddress] = static_cast<Json::Int64>(amount);
}

void BtcTxTarget::ConvertSatoshisToBitcoin()
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

BtcSigningPrequisite::BtcSigningPrequisite()
{

}

BtcSigningPrequisite::BtcSigningPrequisite(std::string txId, int64_t vout, std::string scriptPubKey, std::string redeemScript)
{
    // all of these values must be set or else prequisite is invalid
    (*this)["txid"] = txId;
    (*this)["vout"] = static_cast<Json::Int64>(vout);
    (*this)["scriptPubKey"] = scriptPubKey;
    (*this)["redeemScript"] = redeemScript;
}

void BtcSigningPrequisite::SetTxId(std::string txId)
{
    // we can get this value from the transaction used to send funds to the p2sh
    (*this)["txid"] = txId;
}

void BtcSigningPrequisite::SetVout(Json::Int64 vout)
{
    // we can get this value from the transaction used to send funds to the p2sh
    (*this)["vout"] = vout;
}

void BtcSigningPrequisite::SetScriptPubKey(std::string scriptPubKey)
{
    // we can get this value from the transaction used to send funds to the p2sh
    (*this)["scriptPubKey"] = scriptPubKey;
}

void BtcSigningPrequisite::SetRedeemScript(std::string redeemScript)
{
    // we can get this from the createmultisig api function
    (*this)["redeemScript"] = redeemScript;
}

BtcRpcPacket::BtcRpcPacket()
{
    SetDefaults();
    return;
}

BtcRpcPacket::BtcRpcPacket(const std::string &strData)
: data(strData.begin(), strData.end()), pointerOffset(0)
{
    this->data.push_back('\0');
}

BtcRpcPacket::BtcRpcPacket(const BtcRpcPacketPtr packet)
: data(packet->data.begin(), packet->data.end()), pointerOffset(0)
{
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
    this->pointerOffset = 0;
}

bool BtcRpcPacket::AddData(const std::string &strData)
{
    std::string data(this->data.begin(), this->data.end());
    data.append(strData);

    this->data.clear();
    this->data = std::vector<char>(data.begin(), data.end());
    this->data.push_back('\0');

    return true;
}

const char *BtcRpcPacket::ReadNextChar()
{
    if (this->pointerOffset <= this->data.size())
        return &this->data.at(this->pointerOffset++);
    else return "";
}

size_t BtcRpcPacket::size()
{
    return this->data.size();
}

const char* BtcRpcPacket::GetData()
{
    return &*this->data.begin();
}
















