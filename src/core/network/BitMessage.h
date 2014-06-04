#pragma once
//
//  BitMessage.h
//

#include <memory>
#include <string>
#include <ctime>
//#include <mutex>
#include "Network.h"
#include "XmlRPC.h"
#include "base64.h"
#include "WorkQueue.h"
#include "MsgQueue.h"
#include "BitMessageQueue.h"
#include "TR1_Wrapper.hpp"


typedef std::string BitMessageAddress;

class BitMessageIdentity {
    
public:
    
    BitMessageIdentity(base64 label, BitMessageAddress address, int stream=1, bool enabled=true, bool chan=false) : m_label(label), m_address(address), m_stream(stream), m_enabled(enabled), m_chan(chan) {}
    
    // Note "getLabel" returns a base64 formatted label, you will need to decode this object via base64::decoded
    base64 getLabel(){return m_label;}
    BitMessageAddress getAddress(){return m_address;}
    int getStream(){return m_stream;}
    bool getEnabled(){return m_enabled;}
    bool getChan(){return m_chan;}
    
private:
    
    base64 m_label;
    BitMessageAddress m_address;
    int m_stream;
    bool m_enabled;
    bool m_chan;
    
};

typedef std::vector<BitMessageIdentity> BitMessageIdentities;


class BitMessageAddressBookEntry {
    
public:
    
    BitMessageAddressBookEntry(BitMessageAddress address, base64 label) : m_address(address), m_label(label) {}
    
    BitMessageAddress getAddress(){return m_address;}
    base64 getLabel(){return m_label;}
    
private:
    
    BitMessageAddress m_address;
    base64 m_label;
    
};

typedef std::vector<BitMessageAddressBookEntry> BitMessageAddressBook;


class BitMessageSubscription {
    
public:
    
    BitMessageSubscription(std::string address, bool enabled, base64 label) : m_address(address), m_enabled(enabled), m_label(label) {}
    
    std::string getAddress(){return m_address;}
    bool getEnabled(){return m_enabled;}
    base64 getLabel(){return m_label;}
    
private:
    
    std::string m_address;
    bool m_enabled;
    base64 m_label;
    
};

typedef std::vector<BitMessageSubscription> BitMessageSubscriptionList;


class BitInboxMessage {
    
public:
    
    BitInboxMessage(std::string msgID, BitMessageAddress toAddress, BitMessageAddress fromAddress, base64 subject, base64 message, int encodingType, std::time_t m_receivedTime, bool m_read) : m_msgID(msgID), m_toAddress(toAddress), m_fromAddress(fromAddress), m_subject(subject), m_message(message), m_encodingType(encodingType), m_receivedTime(m_receivedTime), m_read(m_read) {}
    
    std::string getMessageID(){return m_msgID;}
    BitMessageAddress getToAddress(){return m_toAddress;}
    BitMessageAddress getFromAddress(){return m_fromAddress;}
    base64 getSubject(){return m_subject;}
    base64 getMessage(){return m_message;}
    int getEncodingType(){return m_encodingType;}
    std::time_t getReceivedTime(){return m_receivedTime;}
    bool getRead(){return m_read;}
    
    
private:
    
    std::string m_msgID;
    BitMessageAddress m_toAddress;
    BitMessageAddress m_fromAddress;
    base64 m_subject;
    base64 m_message;
    int m_encodingType;
    std::time_t m_receivedTime;
    bool m_read;
    
};

typedef std::vector<BitInboxMessage> BitMessageInbox;


class BitSentMessage {
    
public:
    
    BitSentMessage(std::string msgID, BitMessageAddress toAddress, BitMessageAddress fromAddress, base64 subject, base64 message, int encodingType, std::time_t lastActionTime, std::string status, std::string ackData) : m_msgID(msgID), m_toAddress(toAddress), m_fromAddress(fromAddress), m_subject(subject), m_message(message), m_encodingType(encodingType), m_lastActionTime(lastActionTime), m_status(status), m_ackData(ackData) {}
    
    std::string getMessageID(){return m_msgID;}
    BitMessageAddress getToAddress(){return m_toAddress;}
    BitMessageAddress getFromAddress(){return m_fromAddress;}
    base64 getSubject(){return m_subject;}
    base64 getMessage(){return m_message;}
    int getEncodingType(){return m_encodingType;}
    std::time_t getLastActionTime(){return m_lastActionTime;}
    std::string getStatus(){return m_status;}
    std::string getAckData(){return m_ackData;}
    
    
private:
    
    std::string m_msgID;
    BitMessageAddress m_toAddress;
    BitMessageAddress m_fromAddress;
    base64 m_subject;
    base64 m_message;
    int m_encodingType;
    std::time_t m_lastActionTime;
    std::string m_status;
    std::string m_ackData;
    
};

typedef std::vector<BitSentMessage> BitMessageOutbox;


class BitDecodedAddress {
    
public:
    
    BitDecodedAddress(std::string status, int addressVersion, std::string ripe, int streamNumber) : m_status(status), m_addressVersion(addressVersion), m_ripe(ripe), m_streamNumber(streamNumber) {}
    
    std::string getStatus(){return m_status;}
    int getAddressVersion(){return m_addressVersion;}
    std::string getRipe(){return m_ripe;}
    int getStreamNumber(){return m_streamNumber;}
    
    
private:
    
    std::string m_status;
    int m_addressVersion;
    std::string m_ripe;
    int m_streamNumber;
    
};


class BitMessageQueue; // Pre-defined here so we can hold one as an object in our BitMessage class.

class BitMessage : public NetworkModule {
    
public:
    
    BitMessage(std::string commstring);
    ~BitMessage();
    
    void forceKill(bool kill){m_forceKill = kill;};
    
    // Virtual Function Implementations
    bool accessible();
    
    std::string moduleType(){return "BitMessage";}
    
    bool createAddress(std::string label="");  // Queued
    bool createDeterministicAddress(std::string key, std::string label=""); // Queued
    bool deleteLocalAddress(std::string address); // Queued
    
    bool addressAccessible(std::string address);  // Queued
    
    std::vector<std::pair<std::string, std::string> > getRemoteAddresses();    // Queued
    std::vector<std::pair<std::string, std::string> > getLocalAddresses();    // Queued
    bool checkLocalAddresses(); // Queued
    bool checkRemoteAddresses(); // Queued
    
    bool checkMail(); // Asks the network interface to manually check for messages // Queued
    bool newMailExists(std::string address=""); // checks for new mail, returns true if there is new mail in the queue. // "Queued", will queue new mail request if inbox is empty.
    
    std::vector<_SharedPtr<NetworkMail> > getInbox(std::string address=""); // No queueing, if no inbox exists (which shouldn't happen), it will block until one is returned by the api server.
    std::vector<_SharedPtr<NetworkMail> > getAllInboxes();  // Implemented as passthrough function
    std::vector<_SharedPtr<NetworkMail> > getOutbox(std::string address="");
    std::vector<_SharedPtr<NetworkMail> > getAllOutboxes(); // Implemented as passthrough function
    std::vector<_SharedPtr<NetworkMail> > getUnreadMail(std::string address);
    std::vector<_SharedPtr<NetworkMail> > getAllUnreadMail();
    
    bool deleteMessage(std::string messageID); // Any part of the message should be able to be used to delete it from an inbox    // Queued
    bool markRead(std::string messageID, bool read=true); // By default this marks a given message as read or not, not all API's will support this and should thus return false.  // Queued
    
    bool sendMail(NetworkMail message); // Queued
    
    bool publishSupport(){return true;};
    std::vector<std::pair<std::string,std::string> > getSubscriptions();
    bool refreshSubscriptions();
    
    
    // Message Queue Interaction
    bool startQueue();
    bool stopQueue();
    bool flushQueue();
    int queueSize();
    
    
    //
    // Core API Functions
    // https://bitmessage.org/wiki/API_Reference
    //
    
    // Inbox Management
    
    void getAllInboxMessages();
    
    void getInboxMessageByID(std::string msgID, bool setRead=true);
    
    void getAllSentMessages();
    
    BitSentMessage getSentMessageByID(std::string msgID);
    
    BitSentMessage getSentMessageByAckData(std::string ackData);
    
    std::vector<BitSentMessage> getSentMessagesBySender(std::string address);
    
    void trashMessage(std::string msgID);
    
    bool trashSentMessageByAckData(std::string ackData);
    
    
    // Message Management
    void sendMessage(std::string fromAddress, std::string toAddress, base64 subject, base64 message, int encodingType=2);
    //std::string sendMessage(std::string fromAddress, std::string toAddress, std::string subject, std::string message, int encodingType=2){return sendMessage(fromAddress, toAddress, base64(subject), base64(message), encodingType);}
    
    std::string sendBroadcast(std::string fromAddress, base64 subject, base64 message, int encodingType=2);
    std::string sendBroadcast(std::string fromAddress, std::string subject, std::string message, int encodingType=2){return sendBroadcast(fromAddress, base64(subject), base64(message), encodingType);}
    
    
    // Subscription Management
    
    void listSubscriptions();
    
    bool addSubscription(std::string address, base64 label);
    bool addSubscription(std::string address, std::string label){return addSubscription(address, base64(label));}
    
    bool deleteSubscription(std::string address);
    
    
    // Channel Management
    
    BitMessageAddress createChan(base64 password);
    BitMessageAddress createChan(std::string password){return createChan(base64(password));}
    
    bool joinChan(base64 password, std::string address);
    bool joinChan(std::string password, std::string address){return joinChan(base64(password), address);}
    
    bool leaveChan(std::string address);
    
    
    // Address Management
    
    void listAddresses(); // This is technically "listAddresses2" in the API reference
    
    void createRandomAddress(base64 label=base64(""), bool eighteenByteRipe=false, int totalDifficulty=1, int smallMessageDifficulty=1);
    //void createRandomAddress(std::string label, bool eighteenByteRipe=false, int totalDifficulty=1, int smallMessageDifficulty=1){createRandomAddress(label, eighteenByteRipe, totalDifficulty, smallMessageDifficulty);}
    
    // Warning - This is not guaranteed to return a filled vector if the call does not return any new addresses.
    // You must check that you are accessing a legal position in the vector first.
    void createDeterministicAddresses(base64 password, int numberOfAddresses=1, int addressVersionNumber=0, int streamNumber=0, bool eighteenByteRipe=false, int totalDifficulty=1, int smallMessageDifficulty=1);
    //std::vector<BitMessageAddress> createDeterministicAddresses(std::string password, int numberOfAddresses=1, int addressVersionNumber=0, int streamNumber=0, bool eighteenByteRipe=false, int totalDifficulty=1, int smallMessageDifficulty=1){return createDeterministicAddresses(base64(password), numberOfAddresses, addressVersionNumber, streamNumber, eighteenByteRipe, totalDifficulty, smallMessageDifficulty);}
    
    BitMessageAddress getDeterministicAddress(base64 password, int addressVersionNumber=4, int streamNumber=1);
    BitMessageAddress getDeterministicAddress(std::string password, int addressVersionNumber=4, int streamNumber=1){return getDeterministicAddress(base64(password), addressVersionNumber, streamNumber);}
    
    void listAddressBookEntries();
    
    bool addAddressBookEntry(std::string address, base64 label);
    bool addAddressBookEntry(std::string address, std::string label){return addAddressBookEntry(address, base64(label));}
    
    bool deleteAddressBookEntry(std::string address);
    
    void deleteAddress(BitMessageAddress address);
    
    BitDecodedAddress decodeAddress(BitMessageAddress address);
    
    
    // Other API Commands
    
    int add(int x, int y);
    
    std::string getStatus(std::string ackData);
    std::string helloWorld(std::string first, std::string second);
    
    
    // Extra BitMessage Options (some of these are pass-through functions not related to the API)
    
    void setTimeout(int timeout);
    
    
private:
    
    typedef std::vector<xmlrpc_c::value> Parameters;
    typedef xmlrpc_c::value_int ValueInt;
    typedef xmlrpc_c::value_i8 ValueI8;
    typedef xmlrpc_c::value_boolean ValueBool;
    typedef xmlrpc_c::value_double ValueDouble;
    typedef xmlrpc_c::value_string ValueString;
    typedef xmlrpc_c::value_datetime ValueDateTime;
    typedef xmlrpc_c::value_bytestring ValueByteString;
    typedef xmlrpc_c::value_nil ValueNil;
    typedef xmlrpc_c::value_array ValueArray;
    typedef xmlrpc_c::value_struct ValueStruct;
    typedef std::time_t ValueTime;
    
    // Private member variables
    std::string m_host;
    int m_port;
    std::string m_pass;
    std::string m_username;
    
    bool m_serverAvailable;
    bool m_forceKill;  // If this is set, the class will ignore the status of the queue processing and force a shut down of the network.
    
    
    // Communication Library, XmlRPC in this case
    XmlRPC *m_xmllib;
    
    
    // Private Helper Functions
    
    void setServerAlive(bool alive);
    void parseCommstring(std::string commstring);
    void checkAlive(); // Forces a health check of the BitMessage API Server
    
    
    // Message Queing Plugs
    
    friend BitMessageQueue;
    
    BitMessageQueue *bm_queue; // Our Message Queue friend class.
    
    void initializeUserData(); // Manually pulls down startup data for BitMessage Class.
    
    // Local Objects and their corresponding Mutexes for thread safety.
    
    OT_MUTEX(m_newestCreatedAddressMutex);
    std::string newestCreatedAddress;
    
    OT_MUTEX(m_localIdentitiesMutex);
    BitMessageIdentities m_localIdentities;   // The addresses we have the ability to decrypt messages for.
    
    OT_MUTEX(m_localAddressBookMutex);
    BitMessageAddressBook m_localAddressBook;   // Remote user addresses.
    
    OT_MUTEX(m_localInboxMutex);
    std::vector<_SharedPtr<NetworkMail> > m_localInbox;
    BitMessageInbox m_localUnformattedInbox; // Necessary for doing operations on BitMessage-specific messages
    OT_ATOMIC(m_newMailExists);
    
    OT_MUTEX(m_localOutboxMutex);
    std::vector<_SharedPtr<NetworkMail> > m_localOutbox;
    BitMessageOutbox m_localUnformattedOutbox; // Necessary for doing operations on BitMessage-specific messages
    
    OT_MUTEX(m_localSubscriptionListMutex);
    BitMessageSubscriptionList m_localSubscriptionList;
    
};







