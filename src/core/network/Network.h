#pragma once
//
//  Network.h
//  Auditor
//

#include <string>
#include <vector>
#include <ctime>
#include <utility>


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
    
    NetworkMail(std::string from="", std::string to="", std::string subject="", std::string message="", std::time_t received=0, std::time_t sent=0) : m_from(from), m_to(to), m_subject(subject), m_mail(message), m_received(received), m_sent(sent) {};
    
    std::string getFrom(){return m_from;};
    std::string getTo(){return m_to;};
    std::string getSubject(){return m_subject;};
    std::string getMessage(){return m_mail;};
    std::time_t getReceivedTime(){return m_received;};
    std::time_t getSentTime(){return m_sent;};
    
private:
  
    std::string m_from;
    std::string m_to;
    std::string m_subject;
    std::string m_mail;
    
    std::time_t m_received;
    std::time_t m_sent;
};


class NetworkModule : public NetCounter<NetworkModule> {
    
public:
    
    NetworkModule(std::string commstring) : m_commstring(commstring) {};

    
    virtual bool accessible(){return false;};
    
    virtual int  modulesLoaded(){return NetCounter::loaded;};
    virtual int  modulesAlive(){return NetCounter::alive;};
    
    virtual std::string moduleType(){return "";};
    
    virtual std::string createAddress(std::string options=""){return "";};
    virtual std::string createDeterministicAddress(std::string key){return "";};
    
    virtual bool addressAccessible(std::string address){return false;};
    virtual std::vector<std::string> getAddresses(){return std::vector<std::string>();};
    
    
    virtual std::vector<NetworkMail> getInbox(std::string address){return std::vector<NetworkMail>();};
    virtual std::vector<NetworkMail> getAllInboxes(){return std::vector<NetworkMail>();};
    virtual std::vector<NetworkMail> getAllUnread(){return std::vector<NetworkMail>();};
    
    virtual bool checkNewMail(std::string address){return false;}; // checks for new mail, returns true if there is new mail in the queue.
    virtual std::vector<NetworkMail> getUnreadMail(std::string address){return std::vector<NetworkMail>();}; // You don't want to have to do copies of your whole inbox for every download
    virtual bool deleteMessage(NetworkMail message){return false;}; // Any part of the message should be able to be used to delete it from an inbox
    virtual bool markRead(NetworkMail message, bool read=true){return false;}; // By default this marks a given message as read or not, not all API's will support this and should thus return false.
    
    virtual bool sendMail(NetworkMail message){return false;};
    
    virtual bool publishSupport(){return false;};
    virtual std::vector<std::string> getSubscriptions(){return std::vector<std::string>();};
    
    // Broadcasting Functions
    
    virtual bool createBroadcastAddress(){return false;};
    virtual bool broadcastOnAddress(std::string address){return false;};
    virtual bool subscribeToAddress(std::string address){return false;};
    
    // Functions for importing/exporting from BitMessage server addressbook
    
    virtual std::string getLabel(std::string address){return "";};
    virtual bool setLabel(std::string label, std::string address){return false;};
    
    virtual std::string getAddressFromLabel(std::string label){return "";};
    
    virtual bool addContact(std::string label, std::string address){return false;};
    
    // Return a vector of pairs, containing the Label and Addressess respectively
    virtual std::vector<std::pair<std::string, std::string> > getAllContacts(){return std::vector<std::pair<std::string, std::string> >();};

    // Queue Interaction Functions
    virtual bool flushQueue(){return false;}; // Forces a flush of the message queue.
    virtual int queueSize(){return 0;};
    
    
    std::string getCommstring() { return m_commstring; }

private:
    
    std::string m_commstring;
    
};
