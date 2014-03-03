#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin-api/btcmodules.hpp>

#include <bitcoin-api/btcrpccurl.hpp>
#include <bitcoin-api/btcjson.hpp>
#include <bitcoin-api/MTBitcoin.hpp>
#include <bitcoin-api/bitcoinapi.hpp>

BtcModules* BtcModules::staticInstance;

BtcModules::BtcModules()
{
    this->btcRpc = BtcRpcCurlPtr(new BtcRpcCurl(this));
    this->btcJson = BtcJsonPtr(new BtcJson(this));
    this->btcHelper = BtcHelperPtr(new BtcHelper(this));

    this->mtBitcoin = MTBitcoinPtr(new MTBitcoin(this));

    // added this so the Sample classes in Moneychanger can access it
    // they could instead create their own instance though.
    if(this->staticInstance == NULL)
        this->staticInstance = this;
}

BtcModules::~BtcModules()
{
    this->mtBitcoin.reset();

    this->btcHelper.reset();
    this->btcJson.reset();
    this->btcRpc.reset();

    if(this->staticInstance == this)
        this->staticInstance = NULL;
}
