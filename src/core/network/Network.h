#pragma once
//
//  Network.h
//  vp-auditservice
//

#include <string>
#include <vector>
#include <ctime>
#include <utility>
#include <iostream>

#include "TR1_Wrapper.hpp"

// A counter Template to count the number of modules loaded or alive

template <typename T>
struct NetCounter
{
    NetCounter()
    {
        loaded++;
        tested = false;
    }
    
    void setAlive()
    {
        if(!tested){
            alive++;
            tested = true;
        }
    }
    
    void dead()
    {
        alive--;
        tested = false;
    }
    
    virtual ~NetCounter()
    {
        --loaded;
        if(tested)
            alive--;
    }
    static int loaded;
    static int alive;
    bool tested;
};

template <typename T> int NetCounter<T>::loaded( 0 );
template <typename T> int NetCounter<T>::alive( 0 );


class NetworkMail {
    
public:
    
    NetworkMail(std::string from="", std::string to="", std::string subject="", std::string message="", bool isRead=false, std::string messageID="", std::time_t received=0, std::time_t sent=0) : m_from(from), m_to(to), m_subject(subject), m_mail(message), m_readStatus(isRead), m_messageID(messageID), m_received(received), m_sent(sent) {}
    
    std::string getFrom(){return m_from;}
    std::string getTo(){return m_to;}
    std::string getSubject(){return m_subject;}
    std::string getMessage(){return m_mail;}
    std::time_t getReceivedTime(){return m_received;}
    std::time_t getSentTime(){return m_sent;}
    void        setRead(bool status){m_readStatus = status;};
    bool        getRead(){ return m_readStatus;}
    std::string getMessageID(){return m_messageID;}
    
private:
    
    std::string m_from;
    std::string m_to;
    std::string m_subject;
    std::string m_mail;
    
    std::time_t m_received;
    std::time_t m_sent;
    
    bool m_readStatus;
    
    std::string m_messageID;
    
};


class NetworkModule : public NetCounter<NetworkModule> {
    
public:
    
    NetworkModule(std::string commstring) : m_commstring(commstring) {}
    
    std::string getCommstring() { return m_commstring; }
    
    virtual bool accessible(){return false;}
    
    virtual int  modulesLoaded(){return NetCounter::loaded;}
    virtual int  modulesAlive(){return NetCounter::alive;}
    
    virtual std::string moduleType(){return "";}
    
    virtual bool createAddress(std::string label=""){return false;}
    virtual bool createDeterministicAddress(std::string key, std::string label=""){return false;}
    virtual bool deleteLocalAddress(std::string address){return false;}
    
    virtual bool addressAccessible(std::string address){return false;}  // Checks to see if an address is useable for sending messages.
    
    virtual std::vector<std::pair<std::string, std::string> > getLocalAddresses(){return std::vector<std::pair<std::string, std::string> >();}
    virtual std::vector<std::pair<std::string, std::string> > getRemoteAddresses(){return std::vector<std::pair<std::string, std::string> >();}
    virtual bool checkLocalAddresses(){return false;} // Asks the network interface to manually check for new owned addresses.
    virtual bool checkRemoteAddresses(){return false;} // Asks the API to refresh its list of contacts.
    
    virtual bool checkMail(){return false;} // Asks the network interface to manually check for messages
    virtual bool newMailExists(std::string address=""){return false;} // checks for new mail, returns true if there is new mail in the queue.
    // In BitMessage this checks for unread mail in the queue, it will return true
    // If a read message has been marked unread manually.
    
    virtual std::vector<_SharedPtr<NetworkMail> > getInbox(std::string address=""){return std::vector<_SharedPtr<NetworkMail> >();}
    virtual std::vector<_SharedPtr<NetworkMail> > getAllInboxes(){return std::vector<_SharedPtr<NetworkMail> >();}
    virtual std::vector<_SharedPtr<NetworkMail> > getOutbox(std::string address=""){return std::vector<_SharedPtr<NetworkMail> >();}
    virtual std::vector<_SharedPtr<NetworkMail> > getAllOutboxes(){return std::vector<_SharedPtr<NetworkMail> >();}
    virtual std::vector<_SharedPtr<NetworkMail> > getUnreadMail(std::string address){return std::vector<_SharedPtr<NetworkMail> >();}
    virtual std::vector<_SharedPtr<NetworkMail> > getAllUnreadMail(){return std::vector<_SharedPtr<NetworkMail> >();}
    
    virtual bool deleteMessage(std::string messageID){return false;} // passed as a string, as different protocols handle message ID's differently (BitMessage for example)
    virtual bool markRead(std::string messageID, bool read=true){return false;} // By default this marks a given message as read or not, not all API's will support this and should thus return false.
    
    virtual bool sendMail(NetworkMail message){return false;} // Need To, From, Subject and Message in formatted NetworkMail object
    
    virtual bool publishSupport(){return false;}
    virtual std::vector<std::pair<std::string,std::string> > getSubscriptions(){return std::vector<std::pair<std::string, std::string> >();}
    virtual bool refreshSubscriptions(){return false;} // Need to run this to request a refresh of the subscriptions list without flooding network with data
    
    // Broadcasting Functions
    
    virtual bool createBroadcastAddress(){return false;}
    virtual bool broadcastOnAddress(std::string address){return false;}
    virtual bool subscribeToAddress(std::string address){return false;}
    
    // Functions for importing/exporting from BitMessage server addressbook
    
    
    virtual std::string getLabel(std::string address){return "";}
    virtual bool setLabel(std::string label, std::string address){return false;}
    virtual std::string getAddressFromLabel(std::string label){return "";}
    
    virtual bool addContact(std::string label, std::string address){return false;}
    
    // Return a vector of pairs, containing the Label and Addressess respectively
    virtual std::vector<std::pair<std::string, std::string> > getAllContacts(){return std::vector<std::pair<std::string, std::string> >();}
    
    // Queue Interaction Functions
    // Not all API's will have queuing.
    
    virtual bool startQueue(){return false;}
    virtual bool stopQueue(){return false;}
    virtual bool flushQueue(){return false;} // Forces a flush of the message queue.
    virtual int queueSize(){return 0;}
    
    
private:
    
    std::string m_commstring;
    
};