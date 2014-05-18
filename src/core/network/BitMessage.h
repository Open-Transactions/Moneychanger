//
//  BitMessage.h
//  

#include <memory>
#include <string>
#include <ctime>
#include "Network.h"
#include "XmlRPC.h"
#include "base64.h"
#include "WorkQueue.h"
#include "MsgQueue.h"


typedef std::string BitMessageAddress;

class BitMessageIdentity {
    
public:
    
    BitMessageIdentity(base64 label, BitMessageAddress address, int stream=1, bool enabled=true, bool chan=false) : m_label(label), m_address(address), m_stream(stream), m_enabled(enabled), m_chan(chan) {};
    
    // Note "getLabel" returns a base64 formatted label, you will need to decode this object via base64::decoded
    base64 getLabel(){return m_label;};
    BitMessageAddress getAddress(){return m_address;};
    int getStream(){return m_stream;};
    bool getEnabled(){return m_enabled;};
    bool getChan(){return m_chan;};
    
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
    
    BitMessageAddressBookEntry(BitMessageAddress address, base64 label) : m_address(address), m_label(label) {};

    BitMessageAddress getAddress(){return m_address;};
    base64 getLabel(){return m_label;};
    
private:
    
    BitMessageAddress m_address;
    base64 m_label;
    
};

typedef std::vector<BitMessageAddressBookEntry> BitMessageAddressBook;


class BitMessageSubscription {
    
public:
    
    BitMessageSubscription(std::string address, bool enabled, base64 label) : m_address(address), m_enabled(enabled), m_label(label) {};
    
    std::string getAddress(){return m_address;};
    bool getEnabled(){return m_enabled;};
    base64 getLabel(){return m_label;};
    
private:
    
    std::string m_address;
    bool m_enabled;
    base64 m_label;
    
};

typedef std::vector<BitMessageSubscription> BitMessageSubscriptionList;


class BitInboxMessage {

public:
    
    BitInboxMessage(std::string msgID, BitMessageAddress toAddress, BitMessageAddress fromAddress, base64 subject, base64 message, int encodingType, std::time_t m_receivedTime, bool m_read) : m_msgID(msgID), m_toAddress(toAddress), m_fromAddress(fromAddress), m_subject(subject), m_message(message), m_encodingType(encodingType), m_receivedTime(m_receivedTime), m_read(m_read) {};
    
    std::string getMsgID(){return m_msgID;};
    BitMessageAddress getToAddress(){return m_toAddress;};
    BitMessageAddress getFromAddress(){return m_fromAddress;};
    base64 getSubject(){return m_subject;};
    base64 getMessage(){return m_message;};
    int getEncodingType(){return m_encodingType;};
    std::time_t getReceivedTime(){return m_receivedTime;};
    bool getRead(){return m_read;};


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
    
    BitSentMessage(std::string msgID, BitMessageAddress toAddress, BitMessageAddress fromAddress, base64 subject, base64 message, int encodingType, std::time_t lastActionTime, std::string status, std::string ackData) : m_msgID(msgID), m_toAddress(toAddress), m_fromAddress(fromAddress), m_subject(subject), m_message(message), m_encodingType(encodingType), m_lastActionTime(lastActionTime), m_status(status), m_ackData(ackData) {};
    
    std::string getMsgID(){return m_msgID;};
    BitMessageAddress getToAddress(){return m_toAddress;};
    BitMessageAddress getFromAddress(){return m_fromAddress;};
    base64 getSubject(){return m_subject;};
    base64 getMessage(){return m_message;};
    int getEncodingType(){return m_encodingType;};
    std::time_t getLastActionTime(){return m_lastActionTime;};
    std::string getStatus(){return m_status;};
    std::string getAckData(){return m_ackData;};

    
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
    
    BitDecodedAddress(std::string status, int addressVersion, std::string ripe, int streamNumber) : m_status(status), m_addressVersion(addressVersion), m_ripe(ripe), m_streamNumber(streamNumber) {};
    
    std::string getStatus(){return m_status;};
    int getAddressVersion(){return m_addressVersion;};
    std::string getRipe(){return m_ripe;};
    int getStreamNumber(){return m_streamNumber;};
    
    
private:
    
    std::string m_status;
    int m_addressVersion;
    std::string m_ripe;
    int m_streamNumber;
    
};



class BitMessage : public NetworkModule {

public:
    
    BitMessage(std::string commstring);
    ~BitMessage();
    
    
    // Virtual Function Implementations
    bool accessible();
    
    std::string moduleType(){return "bitmessage";};
    
    std::string createAddress(std::string options);
    std::string createDeterministicAddress(std::string key);
    
    // Not yet implemented
     
    bool addressAccessible(std::string address);
    std::vector<std::string> getAddresses();
    
    
    std::vector<NetworkMail> getInbox(std::string address);
    std::vector<NetworkMail> getAllInboxes();
    std::vector<NetworkMail> getAllUnread();
    
    bool checkNewMail(std::string address); // checks for new mail, returns true if there is new mail in the queue.
    std::vector<NetworkMail> getUnreadMail(std::string address); // You don't want to have to do copies of your whole inbox for every download
    bool deleteMessage(NetworkMail message); // Any part of the message should be able to be used to delete it from an inbox
    bool markRead(NetworkMail message, bool read=true); // By default this marks a given message as read or not, not all API's will support this and should thus return false.
    
    bool sendMail(NetworkMail message);
    
    bool publishSupport(){return true;};
    std::vector<std::string> getSubscriptions();
    
        
    //
    // Core API Functions
    // https://bitmessage.org/wiki/API_Reference
    //

    // Inbox Management
    
    BitMessageInbox getAllInboxMessages();
    
    BitInboxMessage getInboxMessageByID(std::string msgID, bool setRead=true);
    
    BitMessageOutbox getAllSentMessages();
    
    BitSentMessage getSentMessageByID(std::string msgID);

    BitSentMessage getSentMessageByAckData(std::string ackData);

    std::vector<BitSentMessage> getSentMessagesBySender(std::string address);
    
    bool trashMessage(std::string msgID);
    
    bool trashSentMessageByAckData(std::string ackData);

    
    // Message Management
    std::string sendMessage(std::string fromAddress, std::string toAddress, base64 subject, base64 message, int encodingType=2);
    std::string sendMessage(std::string fromAddress, std::string toAddress, std::string subject, std::string message, int encodingType=2){return sendMessage(fromAddress, toAddress, base64(subject), base64(message), encodingType);};

    std::string sendBroadcast(std::string fromAddress, base64 subject, base64 message, int encodingType=2);
    std::string sendBroadcast(std::string fromAddress, std::string subject, std::string message, int encodingType=2){return sendBroadcast(fromAddress, base64(subject), base64(message), encodingType);};

    
    // Subscription Management
    
    BitMessageSubscriptionList listSubscriptions();
    
    bool addSubscription(std::string address, base64 label);
    bool addSubscription(std::string address, std::string label){return addSubscription(address, base64(label));};

    bool deleteSubscription(std::string address);
    
    
    // Channel Management
    
    BitMessageAddress createChan(base64 password);
    BitMessageAddress createChan(std::string password){return createChan(base64(password));};
    
    bool joinChan(base64 password, std::string address);
    bool joinChan(std::string password, std::string address){return joinChan(base64(password), address);};
    
    bool leaveChan(std::string address);
    
    
    // Address Management
    
    BitMessageIdentities listAddresses(); // This is technically "listAddresses2" in the API reference
    
    BitMessageAddress createRandomAddress(base64 label=base64(""), bool eighteenByteRipe=false, int totalDifficulty=1, int smallMessageDifficulty=1);
    BitMessageAddress createRandomAddress(std::string label, bool eighteenByteRipe=false, int totalDifficulty=1, int smallMessageDifficulty=1){return createRandomAddress(label, eighteenByteRipe, totalDifficulty, smallMessageDifficulty);};

    // Warning - This is not guaranteed to return a filled vector if the call does not return any new addresses.
    // You must check that you are accessing a legal position in the vector first.
    std::vector<BitMessageAddress> createDeterministicAddresses(base64 password, int numberOfAddresses=1, int addressVersionNumber=0, int streamNumber=0, bool eighteenByteRipe=false, int totalDifficulty=1, int smallMessageDifficulty=1);
    std::vector<BitMessageAddress> createDeterministicAddresses(std::string password, int numberOfAddresses=1, int addressVersionNumber=0, int streamNumber=0, bool eighteenByteRipe=false, int totalDifficulty=1, int smallMessageDifficulty=1){return createDeterministicAddresses(base64(password), numberOfAddresses, addressVersionNumber, streamNumber, eighteenByteRipe, totalDifficulty, smallMessageDifficulty);};
    
    BitMessageAddress getDeterministicAddress(base64 password, int addressVersionNumber=4, int streamNumber=1);
    BitMessageAddress getDeterministicAddress(std::string password, int addressVersionNumber=4, int streamNumber=1){return getDeterministicAddress(base64(password), addressVersionNumber, streamNumber);};
    
    BitMessageAddressBook listAddressBookEntries();
    
    bool addAddressBookEntry(std::string address, base64 label);
    bool addAddressBookEntry(std::string address, std::string label){return addAddressBookEntry(address, base64(label));};

    bool deleteAddressBookEntry(std::string address);
    
    bool deleteAddress(BitMessageAddress address);
    
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
    
    // Communication Library, XmlRPC in this case
    XmlRPC *m_xmllib;
    
    
    // Private Helper Functions
    
    void setServerAlive(bool alive);
};
