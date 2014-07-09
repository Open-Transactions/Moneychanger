#include "btcjsonlegacy.hpp"

#define METHOD_GETBALANCE            "getbalance"
#define METHOD_LISTACCOUNTS          "listaccounts"
#define METHOD_LISTRECEIVEDBYADDRESS "listreceivedbyaddress"
#define METHOD_LISTTRANSACTIONS      "listtransactions"
#define METHOD_GETTRANSACTION        "gettransaction"

BtcJsonLegacy::BtcJsonLegacy(BtcModules* modules)
    :BtcJson(modules)
{
}

BtcJsonLegacy::~BtcJsonLegacy()
{
}

BtcRpcPacketPtr BtcJsonLegacy::CreateJsonQuery(const std::string &command, const Json::Value &params, std::string id)
{
    if(id.empty())
        id = command + "legacy";

    return BtcJson::CreateJsonQuery(command, params, id);
}

int64_t BtcJsonLegacy::GetBalance(const char *account, const int32_t &minConfirmations, const bool &includeWatchonly)
{
    // note: json and bitcoind make a difference between NULL-strings and empty strings.

    Json::Value params = Json::Value();
    params.append(account != NULL ? account : Json::Value());      // account
    params.append(minConfirmations);

    BtcRpcPacketPtr reply = this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_GETBALANCE));

    Json::Value result;
    if(!ProcessRpcString(reply, result) || !result.isDouble())
    {
        return 0;
    }

    return this->modules->btcHelper->CoinsToSatoshis(result.asDouble());
}

btc::stringList BtcJsonLegacy::ListAccounts(const int32_t &minConf, const bool &includeWatchonly)
{
    Json::Value params = Json::Value();
    params.append(minConf);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_LISTACCOUNTS, params)), result))
        return btc::stringList();     // error

    if(!result.isObject())
        return btc::stringList();        // this shouldn't happen unless the protocol was changed

    return result.getMemberNames();      // each key is an account, each value is the account's balance
}

BtcAddressBalances BtcJsonLegacy::ListReceivedByAddress(const int32_t &minConf, const bool &includeEmpty, const bool &includeWatchonly)
{
    Json::Value params = Json::Value();
    params.append(minConf);
    params.append(includeEmpty);

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

BtcTransactions BtcJsonLegacy::ListTransactions(const std::string &account, const int32_t &count, const int32_t &from, const bool &includeWatchonly)
{
    Json::Value params = Json::Value();
    params.append(account);
    params.append(count);
    params.append(from);

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

// TODO: BtcTransactions ListSinceBlock()

BtcTransactionPtr BtcJsonLegacy::GetTransaction(const std::string &txId, const bool& includeWatchonly)
{
    Json::Value params = Json::Value();
    params.append(txId);

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
