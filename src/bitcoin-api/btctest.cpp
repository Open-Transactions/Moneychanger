#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin-api/btctest.hpp>

#include <bitcoin-api/btcmodules.hpp>

#include <cstdio>
#include <iostream>

BtcModulesPtr BtcTest::modules;
std::string BtcTest::multiSigAddress;
std::string BtcTest::depositTxId;

BtcTest::BtcTest()
{
    depositTxId = "";
    multiSigAddress = "";
}

bool BtcTest::TestBitcoinFunctions()
{
    modules = BtcModulesPtr(new BtcModules());

    if(modules == NULL ||
            modules->btcRpc == NULL ||
            modules->btcJson == NULL ||
            modules->btcHelper == NULL ||
            modules->mtBitcoin == NULL)
    {
        return false;
    }

    if(!TestBtcRpc())
        return false;

    if(!TestBtcJson())
        return false;

    if(!TestRawTransactions())
        return false;

    if(!TestMultiSig(false))    // false = don't wait for confirmations = quick test
        return false;

    if(!TestImportAddress(0))
        return false;

    //if(!TestImportAddress(1)) // waits for confirmations. i just tested it and it works.
    //    return false;

    if(!TestImportMultisig(0))
        return false;

    //if(!TestImportMultisig(1))
    //    return false;

    return true;
}

bool BtcTest::TestBtcRpc()
{
    // first testnet server:
    BitcoinServerPtr bitcoind1 = BitcoinServerPtr(new BitcoinServer("admin1", "123", "http://127.0.0.1", 19001));

    { BtcRpcPacketPtr(new BtcRpcPacket()); }
    { std::string str = "test"; BtcRpcPacketPtr(new BtcRpcPacket(str)); }
    { BtcRpcPacketPtr(new BtcRpcPacket("test")); }
    {
        BtcRpcPacketPtr ptr = BtcRpcPacketPtr(new BtcRpcPacket("test"));
        BtcRpcPacketPtr(new BtcRpcPacket(ptr));
    }
    {
        std::printf("\n");
        std::cout.flush();

        BtcRpcPacketPtr ptr = BtcRpcPacketPtr(new BtcRpcPacket());
        ptr->AddData("test");
        ptr->AddData("moreTEST");
        std::string data = "";
        for(size_t i = 0; i < ptr->size() + 10; i++)
        {
            const char* c = ptr->ReadNextChar();
            if(c)
            {
                data += *c;
                std::printf("%c\n", *c);
            }
            std::cout.flush();
        }
        std::printf("%s\n", data.c_str());
        std::cout.flush();
    }

    // connect to server (if this function succeeds, SendRpc() works too)
    if(!modules->btcRpc->ConnectToBitcoin(bitcoind1))
        return false;

    if(modules->btcRpc->ConnectToBitcoin(BitcoinServerPtr()))
        return false;

    if(modules->btcRpc->ConnectToBitcoin("wronguser", "wrongpw","127.0.0.1",19001))
        return false;

    if(modules->btcRpc->ConnectToBitcoin("","", "127.0.0.1", 123))
        return false;

    if(!modules->btcRpc->ConnectToBitcoin(bitcoind1))
        return false;

    // try sending some garbage and see if anything crashes
    modules->btcRpc->SendRpc(NULL);

    modules->btcRpc->SendRpc("garbage");

    modules->btcRpc->SendRpc("\0");

    modules->btcRpc->SendRpc(BtcRpcPacketPtr(new BtcRpcPacket()));

    return true;    // not crashing is enough to pass this test
}

bool BtcTest::TestBtcJson()
{
    BitcoinServerPtr bitcoind1 = BitcoinServerPtr(new BitcoinServer("admin1", "123", "http://127.0.0.1", 19001));

    // connect to server
    if(!modules->btcRpc->ConnectToBitcoin(bitcoind1))
        return false;

    // see if we can get some response:
    std::string address = modules->btcJson->GetNewAddress();
    if(address.empty())
        return false;

    // disconnect
    modules->btcRpc->ConnectToBitcoin(BitcoinServerPtr(new BitcoinServer("bla", "bla", "url", 123)));

    // try again
    address = modules->btcJson->GetNewAddress();
    if(!address.empty())
        return false;

    // reconnect
    if(!modules->btcRpc->ConnectToBitcoin(bitcoind1))
        return false;

    address = modules->btcJson->GetNewAddress();

    std::string txid;
    if((txid = modules->btcJson->SendToAddress(address, BtcHelper::CoinsToSatoshis(10))).empty())
        return false;

    // even when testing locally it takes a few ms
    if(modules->btcHelper->WaitGetTransaction(txid) == NULL)
        return false;

    // list unspent outputs
    BtcUnspentOutputs unspentOutputs = modules->btcJson->ListUnspent(/*optional*/);
    if(unspentOutputs.empty())
        return false;

    // now to spend an output
    // create a list of txid/vout pairs as input for our raw transaction:
    BtcTxIdVouts outputsToSpend;
    outputsToSpend.push_back(BtcTxIdVoutPtr(new BtcTxIdVout(unspentOutputs.front()->txId, unspentOutputs.front()->vout)));

    // create a list of address:vout mappings to which the inputs will be sent
    BtcTxTargets txTargets;
    txTargets[address] = (Json::Int64)(unspentOutputs.front()->amount - BtcHelper::FeeMultiSig);

    // create raw transaction
    std::string rawTx = modules->btcJson->CreateRawTransaction(outputsToSpend, txTargets);
    BtcSignedTransactionPtr signedTx = modules->btcJson->SignRawTransaction(rawTx);
    if(modules->btcJson->SendRawTransaction(signedTx->signedTransaction).empty())
        return false;

    // TODO: Test all the other functions. Nah, they should be safe to use.

    return true;
}

bool BtcTest::TestRawTransactions()
{
    BitcoinServerPtr bitcoind1 = BitcoinServerPtr(new BitcoinServer("admin1", "123", "http://127.0.0.1", 19001));

    // connect to server (if this function succeeds, SendRpc() works too)
    if(!modules->btcRpc->ConnectToBitcoin(bitcoind1))
        return false;

    // receive to address
    std::string myAddress1 = modules->mtBitcoin->GetNewAddress("test");
    if(myAddress1.empty())
        return false;

    // send to receiving address
    std::string txId = modules->mtBitcoin->SendToAddress(myAddress1, BtcHelper::CoinsToSatoshis(1));
    if(txId.empty())
        return false;

    // further send funds to other address:
    std::string myAddress2 = modules->mtBitcoin->GetNewAddress("test");
    if(myAddress2.empty())
        return false;

    BtcTxIdVouts unspentOutputList;
    BtcRawTransactionPtr decodedTx = modules->btcJson->GetDecodedRawTransaction(txId);
    if(decodedTx == NULL)
        return false;

    for (size_t i = 0; i < decodedTx->outputs.size(); i++)
    {
        if(decodedTx->outputs[i].addresses[0] == myAddress1)
            unspentOutputList.push_back(BtcTxIdVoutPtr(new BtcTxIdVout(txId, i)));
    }

    BtcTxTargets targetList;
    targetList[myAddress2] = (Json::Int64)BtcHelper::CoinsToSatoshis(1);
    // spend the received funds
    std::string rawTx = modules->btcJson->CreateRawTransaction(unspentOutputList, targetList);
    if(rawTx.empty())
        return false;
    BtcSignedTransactionPtr signedTx = modules->btcJson->SignRawTransaction(rawTx);
    if(signedTx == NULL)
        return false;
    if(!signedTx->complete)
        return false;

    std::string rawTxId = modules->btcJson->SendRawTransaction(signedTx->signedTransaction);
    if(rawTxId.empty())
        return false;

    return true;
}

bool BtcTest::TestMultiSig(bool waitForConfirmations = true)
{
    // first without confirmations
    // i thought zero-conf requires passing additional arguments to VoteMultiSigRelease,
    // but apparently this is not the case.
    if(!TestMultiSigDeposit(0))
        return false;

    if(!TestMultiSigWithdrawal(0))
        return false;

    if(!waitForConfirmations)
        return true;

    if(!TestMultiSigDeposit(1))
        return false;

    if(!TestMultiSigWithdrawal(1))
        return false;

    return true;
}

bool BtcTest::TestMultiSigDeposit(int minConfirms)
{
    // create two modules so we don't have to change connections all the time
    BtcModules bitcoin1;
    BtcModules bitcoin2;
    if(!bitcoin1.btcRpc->ConnectToBitcoin("admin1", "123", "http://127.0.0.1", 19001))
        return false;
    if(!bitcoin2.btcRpc->ConnectToBitcoin("admin2", "123", "http://127.0.0.1", 19011))
        return false;

    // public keys of the addresses used for multisig
    btc::stringList keys;

    // GetNewAddress on bitcoind #1
    std::string address1 = bitcoin1.mtBitcoin->GetNewAddress();
    if(address1.empty())
        return false;
    keys.push_back(bitcoin1.mtBitcoin->GetPublicKey(address1));

    // GetNewAddress on bitcoind #2
    std::string address2 = bitcoin2.mtBitcoin->GetNewAddress();
    if(address2.empty())
        return false;
    keys.push_back(bitcoin2.mtBitcoin->GetPublicKey(address2));

    // add address to bitcoind #1 wallet
    multiSigAddress = bitcoin1.mtBitcoin->GetMultiSigAddress(2, keys, true, "test");
    if(multiSigAddress.empty())
        return false;

    // add address to bitcoind #2 wallet
    // if we don't add it to the wallet, we'd have to pass additional arguments to CreateRawTransaction later
    bitcoin2.mtBitcoin->GetMultiSigAddress(2, keys, true, "test");



    // send from bitcoind #1 to multisig
    int64_t amountToSend = BtcHelper::CoinsToSatoshis(1.22);
    std::string txId = bitcoin1.mtBitcoin->SendToAddress(multiSigAddress, amountToSend);
    if(txId.empty())
        return false;
    depositTxId = txId;    // need to remember this for later;

    // get an object containing the decoded raw transaction data
    BtcRawTransactionPtr txToMultiSig = bitcoin1.mtBitcoin->WaitGetRawTransaction(txId);
    if(txToMultiSig == NULL)
        return false;

    // wait for confirmations and check for correct amount
    while(!bitcoin1.mtBitcoin->TransactionSuccessful(amountToSend, txToMultiSig, multiSigAddress, minConfirms))
    {
        btc::Sleep(500);
    }

    return true;
}

bool BtcTest::TestMultiSigWithdrawal(int minConfirms)
{
    BtcModules bitcoin1;
    BtcModules bitcoin2;
    if(!bitcoin1.btcRpc->ConnectToBitcoin("admin1", "123", "http://127.0.0.1", 19001))
        return false;
    if(!bitcoin2.btcRpc->ConnectToBitcoin("admin2", "123", "http://127.0.0.1", 19011))
        return false;

    // wait for transaction to propagate through the network
    if(bitcoin2.mtBitcoin->WaitGetRawTransaction(depositTxId) == NULL)
        return false;

    // now get ready to release to bitcoind #2.

    // create address to receive btc
    std::string targetAddress = bitcoin2.mtBitcoin->GetNewAddress();
    if(targetAddress.empty())
        return false;

    std::printf("releasing escrow from Tx #%s,\n address %s\nto address %s\n", depositTxId.c_str(), multiSigAddress.c_str(), targetAddress.c_str());
    std::cout.flush();

    // bitcoind #2 votes to release coins to his own address
    // the optional arguments redeemScript and signingAddress are only needed to
    // sign offline transactions or if depositTxId has no confirmations yet
    BtcSignedTransactionPtr partialTx2 = bitcoin2.mtBitcoin->VoteMultiSigRelease(depositTxId, multiSigAddress, targetAddress);
    if(partialTx2 == NULL)
        return false;

    // bitcoind #1 votes to release coins to #2's address
    BtcSignedTransactionPtr partialTx1 = bitcoin1.mtBitcoin->VoteMultiSigRelease(depositTxId, multiSigAddress, targetAddress);
    if(partialTx1 == NULL)
        return false;

    // combine both partially signed transactions into one
    std::string rawTxString = partialTx1->signedTransaction + partialTx2->signedTransaction;
    BtcSignedTransactionPtr completeTx = bitcoin1.mtBitcoin->CombineTransactions(rawTxString);
    if(completeTx == NULL || !completeTx->complete)
        return false;

    std::string releaseTxId = bitcoin1.mtBitcoin->SendRawTransaction(completeTx->signedTransaction);
    if(releaseTxId.empty())
        return false;

    BtcRawTransactionPtr releaseTx = bitcoin2.mtBitcoin->WaitGetRawTransaction(releaseTxId);
    if(releaseTx == NULL)
        return false;   

    // count how much btc we received in the deposit transaction and substract Fee
    int64_t amountToExpect = bitcoin1.btcHelper->GetTotalOutput(depositTxId, multiSigAddress) - BtcHelper::FeeMultiSig;

    // wait for confirmations and correct amount
    while(!bitcoin2.mtBitcoin->TransactionSuccessful(amountToExpect, releaseTx, targetAddress, minConfirms))
    {
        btc::Sleep(500);
    }

    return true;
}

bool BtcTest::TestImportAddress(int32_t confirmations)
{
    BtcModules bitcoin1;
    BtcModules bitcoin2;
    if(!bitcoin1.btcRpc->ConnectToBitcoin("admin1", "123", "http://127.0.0.1", 19001))
        return false;
    if(!bitcoin2.btcRpc->ConnectToBitcoin("admin2", "123", "http://127.0.0.1", 19011))
        return false;

    // generate new address
    std::string address = bitcoin1.mtBitcoin->GetNewAddress("watched");

    // import that address in other client without rescanning:
    bitcoin2.btcJson->ImportAddress(address, "watching", false);

    // send coins to that address
    int64_t amount = BtcHelper::CoinsToSatoshis(5.0);
    if(bitcoin1.mtBitcoin->SendToAddress(address, amount) == "")
        return false;

    // client 2 waits for a transaction without knowing the txid
    bool stop = false;
    while(!stop)
    {
        btc::stringList addressList;
        addressList.push_back(address);
        BtcUnspentOutputs outputs = bitcoin2.mtBitcoin->ListUnspentOutputs( addressList );
        for(BtcUnspentOutputs::iterator output = outputs.begin(); output != outputs.end(); output++)
        {
            BtcTransactionPtr transaction = bitcoin2.btcHelper->WaitGetTransaction((*output)->txId);
            if(bitcoin2.btcHelper->TransactionSuccessfull(amount, transaction, address, confirmations))
            {
                stop = true;
                break;
            }
        }
    }

    return true;
}

bool BtcTest::TestImportMultisig(int32_t confirmations)
{
    std::printf("test needed conifirmations: %d\n", confirmations); std::cout.flush();
    BtcModules bitcoin1;
    BtcModules bitcoin2;
    if(!bitcoin1.btcRpc->ConnectToBitcoin("admin1", "123", "http://127.0.0.1", 19001))
        return false;
    if(!bitcoin2.btcRpc->ConnectToBitcoin("admin2", "123", "http://127.0.0.1", 19011))
        return false;

    std::string addr1 = bitcoin1.mtBitcoin->GetNewAddress("multisig1");
    std::string pubkey1 = bitcoin1.mtBitcoin->GetPublicKey(addr1);
    std::string addr2 = bitcoin2.mtBitcoin->GetNewAddress("multisig2");
    std::string pubkey2 = bitcoin2.mtBitcoin->GetPublicKey(addr2);

    btc::stringList pubKeys; pubKeys.push_back(pubkey1); pubKeys.push_back(pubkey2);
    std::string multiSig = bitcoin1.mtBitcoin->GetMultiSigAddress(2, pubKeys, true, "multisigAddress");
    bitcoin2.mtBitcoin->GetMultiSigAddress(2, pubKeys, true, "multisigAddress");

    //if(!bitcoin1.mtBitcoin->ImportAddress(multiSig, "multiSigWatching", false))
    //    return false;   // command not supported yet
    if(!bitcoin2.mtBitcoin->ImportAddress(multiSig, "multiSigWatching", false))
        return false;   // command not supported yet

    // send to multisig
    int64_t amount = BtcHelper::CoinsToSatoshis(1.22222);
    if(bitcoin1.mtBitcoin->SendToAddress(multiSig, amount).empty())
        return false;

    // recipient waits for incoming transactions
    BtcUnspentOutputs newOutputs;
    while(true)
    {
        btc::stringList multiSigList;
        multiSigList.push_back(multiSig);
        newOutputs = bitcoin2.btcHelper->ListNewOutputs(newOutputs, multiSigList);
        btc::Sleep(500);
        if(newOutputs.size() > 0)
            break;
    }

    // find out which transaction sent enough funds
    BtcRawTransactionPtr txToMultisig;
    txToMultisig = bitcoin2.btcHelper->TransactionSuccessfull(amount, newOutputs, multiSig, 0);

    // wait for confirmations:
    if(!bitcoin2.btcHelper->WaitTransactionSuccessfull(amount, txToMultisig, multiSig, confirmations))
        return false;

    // now spend from the multisig
    std::string receivingAddress = bitcoin2.mtBitcoin->GetNewAddress("multisigrelease");
    BtcSignedTransactionPtr transaction1 = bitcoin1.mtBitcoin->VoteMultiSigRelease(txToMultisig->txId, multiSig, receivingAddress);
    BtcSignedTransactionPtr transaction2 = bitcoin2.mtBitcoin->VoteMultiSigRelease(txToMultisig->txId, multiSig, receivingAddress);
    BtcSignedTransactionPtr transaction = bitcoin2.mtBitcoin->CombineTransactions(transaction1->signedTransaction + transaction2->signedTransaction);
    std::string txReleaseId = bitcoin2.mtBitcoin->SendRawTransaction(transaction->signedTransaction);
    if(txReleaseId.empty())
        return false;

    if(bitcoin1.mtBitcoin->WaitGetRawTransaction(txReleaseId) == NULL)
        return false;

    return true;
}


















