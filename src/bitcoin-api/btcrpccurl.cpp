#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <bitcoin-api/btcrpccurl.hpp>

#include <curl/curl.h>

#include <iostream>
#include <string.h>
#include <cstdio>

BtcRpcPacketPtr BtcRpcCurl::connectString = BtcRpcPacketPtr(new BtcRpcPacket("{\"jsonrpc\": \"1.0\", \"id\":\"curltest\", \"method\": \"getinfo\", \"params\": [] }"));

BtcRpcCurl::BtcRpcCurl(BtcModules *modules)
{
    this->modules = modules;

    this->curl = NULL;

    /* In windows, this will init the winsock stuff */
    this->res = curl_global_init(CURL_GLOBAL_DEFAULT);

    this->currentServer = BitcoinServerPtr();

    this->mutex = false;
}

BtcRpcCurl::~BtcRpcCurl()
{
    this->modules = NULL;

    this->currentServer = BitcoinServerPtr();

    this->CleanUpCurl();
}

void BtcRpcCurl::WaitMutex()
{
    while (this->mutex)
    {
        btc::Sleep(5);
    }
    this->mutex = true;
}

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
    BtcRpcPacket *pooh = (BtcRpcPacket *)userp;

    if(size*nmemb < 1)
        return 0;

    const char* dataPtr = pooh->ReadNextChar();  

    if(dataPtr == NULL)
        return 0;

    *(char *)ptr = dataPtr[0];
    return 1;

    //if(pooh->size)
    //{
    //    *(char *)ptr = pooh->data[0];       /* copy one single byte */
    //    pooh->data++;                       /* advance pointer */
    //    pooh->pointerOffset++;              // offset pointer so correct memory is freed
    //    pooh->size--;                       /* less data left */

    //    return 1;                        /* we return 1 byte at a time! */
    //}

    //return 0;                          /* no more data left to deliver */
}

static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
    BtcRpcPacket *pooh = (BtcRpcPacket *)userp;

    int newSize = size * nmemb;

    if(size < 1)
        return 0;

    const char *charBuf = static_cast<const char*>(ptr);
    std::vector<char> data(charBuf, charBuf + newSize);
    std::string strData(data.begin(), data.end());

    if (pooh->AddData(strData))
    {
        return newSize;
    }
    else
    {
        return 0;
    }

    //int newLength = pooh->size + size * nmemb;

    //if(pooh->data == NULL)
    //    pooh->data = (char*)realloc(pooh->data, newLength + 1);

    //if(pooh->data == NULL)
    //{
    //    std::printf("realloc failed when reading data from bitcoin-qt\n");
    //    std::cout.flush();
    //    return 0;
    //}

    //memcpy(pooh->data + pooh->size, ptr, size * nmemb);
    //pooh->data[newLength] = '\0';
    //pooh->size = newLength;

    //return size * nmemb;
}

bool BtcRpcCurl::ConnectToBitcoin(BitcoinServerPtr server)
{
    WaitMutex();

    if(server == NULL)
    {
        this->currentServer = server;
        CleanUpCurl();
        mutex = false;
        return false;
    }

    if(this->currentServer != server || !this->curl || this->res != CURLE_OK)
    {
        this->currentServer = server;
        CleanUpCurl();

        /* In windows, this will init the winsock stuff */
        this->res = curl_global_init(CURL_GLOBAL_DEFAULT);

        /* Check for errors */
        if(res != CURLE_OK)
        {
          fprintf(stderr, "curl_global_init() failed: %s\n",
                  curl_easy_strerror(res));
          mutex = false;
          return false;
        }

        /* get a curl handle */
        curl = curl_easy_init();

        if(!curl)
        {
            mutex = false;
            return false;
        }
    }

    /* First set the URL that is about to receive our POST. */
    curl_easy_setopt(curl, CURLOPT_URL, server->url.c_str());

    curl_easy_setopt(curl, CURLOPT_PORT, server->port);

    curl_easy_setopt(curl, CURLOPT_PASSWORD, server->password.c_str());

    curl_easy_setopt(curl, CURLOPT_USERNAME, server->user.c_str());

    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);

    mutex = false;
    return SendRpc(BtcRpcPacketPtr(new BtcRpcPacket(connectString))) != NULL;
}

bool BtcRpcCurl::ConnectToBitcoin(const std::string &user, const std::string &password, const std::string &url, int32_t port)
{
    return ConnectToBitcoin(BitcoinServerPtr(new BitcoinServer(user, password, url, port)));
}

BtcRpcPacketPtr BtcRpcCurl::SendRpc(const std::string &jsonString)
{
    return SendRpc(BtcRpcPacketPtr(new BtcRpcPacket(jsonString)));
}

BtcRpcPacketPtr BtcRpcCurl::SendRpc(BtcRpcPacketPtr jsonString)
{
    if(!curl)
    {
        return BtcRpcPacketPtr();
    }

    WaitMutex();

    BtcRpcPacketPtr receivedData = BtcRpcPacketPtr(new BtcRpcPacket()); // used when receiving data

    /* Now specify we want to POST data */
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    /* we want to use our own read function (called when sending) */
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    /* pointer to pass to our read function (cointains data to send) */
    curl_easy_setopt(curl, CURLOPT_READDATA, jsonString.get());

    /* we want to use our own write function (called when receiving) */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    /* pointer to pass to our write function (we'll write received data into it) */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, receivedData.get());

    /* get verbose debug output please */
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

    /*
        If you use POST to a HTTP 1.1 server, you can send data without knowing
        the size before starting the POST if you use chunked encoding. You
        enable this by adding a header like "Transfer-Encoding: chunked" with
        CURLOPT_HTTPHEADER. With HTTP 1.0 or without chunked transfer, you must
        specify the size in the request.
    */
    #ifdef USE_CHUNKED
    {
        struct curl_slist *chunk = NULL;

        chunk = curl_slist_append(chunk, "Transfer-Encoding: chunked");
        res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        /* use curl_slist_free_all() after the *perform() call to free this
            list again */
    }
    #else
        /* Set the expected POST size. If you want to POST large amounts of data,
            consider CURLOPT_POSTFIELDSIZE_LARGE */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonString->size());
    #endif

    #ifdef DISABLE_EXPECT
    {
        /*
            Using POST with HTTP 1.1 implies the use of a "Expect: 100-continue"
            header.  You can disable this header with CURLOPT_HTTPHEADER as usual.
            NOTE: if you want chunked transfer too, you need to combine these two
            since you can only set one list of headers with CURLOPT_HTTPHEADER. */

        /* A less good option would be to enforce HTTP 1.0, but that might also
            have other implications. */
        struct curl_slist *chunk = NULL;

        chunk = curl_slist_append(chunk, "Expect:");
        res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        /* use curl_slist_free_all() after the *perform() call to free this
            list again */
    }
    #endif

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));

        mutex = false;
        return BtcRpcPacketPtr();
    }

    //if(receivedData->data != NULL)
    //    OTLog::Output(0, receivedData->data);

    // we have to copy the response because for some reason the next few lines set the smart pointer to NULL (?!?!??)
    BtcRpcPacketPtr packetCopy = BtcRpcPacketPtr(new BtcRpcPacket(receivedData));

    receivedData.reset();

    int httpcode = 0;
    curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &httpcode);
    if (httpcode == 401)
    {
        std::printf("Error connecting to bitcoind: Wrong username or password\n");
        std::cout.flush();
        mutex = false;
        return BtcRpcPacketPtr();
    }
    else if (httpcode == 500)
    {
        std::printf("Bitcoind internal server error\n");
        std::cout.flush();
        // don't return
    }
    else if (httpcode != 200)
    {
        std::printf("BtcRpc curl error:\nHTTP response code %d\n", httpcode);
        std::cout.flush();
        mutex = false;
        return BtcRpcPacketPtr();
    }

    mutex = false;
    return packetCopy;
}

BtcRpcPacketPtr BtcRpcCurl::SendRpc(const char *jsonString) // TODO: also receive int size then call overloaded
{
    return BtcRpcPacketPtr();
}

bool BtcRpcCurl::IsConnected()
{
    return false;
}

void BtcRpcCurl::CleanUpCurl()
{
    this->currentServer.reset();

    if(curl)
    {
        curl_easy_cleanup(curl);
        curl = NULL;
    }

    curl_global_cleanup();
}
