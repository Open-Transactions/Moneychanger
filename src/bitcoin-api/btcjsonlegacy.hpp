#ifndef BTCJSONLEGACY_H
#define BTCJSONLEGACY_H

#include "btcjson.hpp"

class BtcJsonLegacy : public BtcJson
{
public:
    BtcJsonLegacy(BtcModules* modules);
    ~BtcJsonLegacy();

    virtual int64_t GetBalance(const char *account = NULL, const int32_t &minConfirmations = BtcHelper::MinConfirms, const bool &includeWatchonly = true);

    virtual btc::stringList ListAccounts(const int32_t &minConf = BtcHelper::MinConfirms, const bool &includeWatchonly = true);

    virtual BtcAddressBalances ListReceivedByAddress(const int32_t &minConf = BtcHelper::MinConfirms, const bool &includeEmpty = false, const bool &includeWatchonly = true);

    virtual BtcTransactions ListTransactions(const std::string &account = "*", const int32_t &count = 20, const int32_t &from = 0, const bool &includeWatchonly = true);

    // TODO: virtual BtcTransactions ListSinceBlock

    virtual BtcTransactionPtr GetTransaction(const std::string &txId, const bool& includeWatchonly = true);

protected:
    virtual BtcRpcPacketPtr CreateJsonQuery(const std::string &command, const Json::Value &params = Json::Value(), std::string id = std::string());
};

#endif // BTCJSONLEGACY_H
