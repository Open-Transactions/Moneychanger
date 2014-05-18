//
//  BitMessage.cpp
//

#include "BitMessage.h"
#include "json/json.h"
#include "base64.h"

#include <string>
#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
//#include <thread>

#include<boost/tokenizer.hpp>


BitMessage::BitMessage(std::string commstring) : NetworkModule(commstring) {

    std::vector<std::string> parsedList;
    boost::tokenizer<boost::escaped_list_separator<char> > tokens(commstring);
    
    for(boost::tokenizer<boost::escaped_list_separator<char> >::iterator it=tokens.begin(); it!=tokens.end();++it){
        parsedList.push_back(*it);
    }
    
    // Needs to be parsed properly, this will undoubtedly lead to errors when initializing BitMessage comms
    // As it is prone to letting typo errors cause crashes.
    
    m_host = parsedList.at(0);
    m_port = std::atoi(parsedList.at(1).c_str());
    m_username = parsedList.at(2);
    m_pass = parsedList.at(3);
    
    m_xmllib = new XmlRPC(m_host, m_port, true, 10000);
    m_xmllib->setAuth(m_username, m_pass);
    
    
    // Runs to setup our counter
    accessible();
}


BitMessage::~BitMessage(){
    delete m_xmllib;
}




/*
 * Virtual Functions
 */


bool BitMessage::accessible(){
    
    if(helloWorld("Hello","World") != "Hello-World"){
        setServerAlive(false);
        return false;
    }
    else{
        setServerAlive(true);
        return true;
    }
}


std::string BitMessage::createAddress(std::string options=""){
    
    if(options == ""){
        return createRandomAddress();
    }
    else
        return "";  // To Be Implemented
    
}


std::string BitMessage::createDeterministicAddress(std::string key){
    
    // We will Implement more advanced parsing of the key soon
    return getDeterministicAddress(base64(key));
    
}


bool BitMessage::addressAccessible(std::string address){
    
    BitMessageIdentities identities(listAddresses());
    
    for(int x = 0; x < identities.size(); x++){
            if(identities.at(x).getAddress() == address){
            return true;
        }
    }
    return false;
}

// Defined in Header, BitMessage has publish support.
//bool BitMessage::publishSupport(){return true;};


std::vector<std::string> BitMessage::getAddresses(){return std::vector<std::string>();};


std::vector<NetworkMail> BitMessage::getInbox(std::string address){return std::vector<NetworkMail>();};
std::vector<NetworkMail> BitMessage::getAllInboxes(){return std::vector<NetworkMail>();};
std::vector<NetworkMail> BitMessage::getAllUnread(){return std::vector<NetworkMail>();};

bool BitMessage::checkNewMail(std::string address){return false;}; // checks for new mail, returns true if there is new mail in the queue.
std::vector<NetworkMail> BitMessage::getUnreadMail(std::string address){return std::vector<NetworkMail>();}; // You don't want to have to do copies of your whole inbox for every download
bool BitMessage::deleteMessage(NetworkMail message){return false;}; // Any part of the message should be able to be used to delete it from an inbox
bool BitMessage::markRead(NetworkMail message, bool read){return false;}; // By default this marks a given message as read or not, not all API's will support this and should thus return false.

bool BitMessage::sendMail(NetworkMail message){return false;};


std::vector<std::string> BitMessage::getSubscriptions(){return std::vector<std::string>();};


/*
 * Message Queueing
 */










/*
 * Direct API Functions
 */

// Inbox Management


std::vector<BitInboxMessage> BitMessage::getAllInboxMessages(){

    Parameters params;
    std::vector<BitInboxMessage> inbox;

    XmlResponse result = m_xmllib->run("getAllInboxMessages", params);
    
    if(result.first == false){
        std::cout << "Error: getAllInboxMessages failed" << std::endl;
        return inbox;
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return inbox;
        }
    }
    
    Json::Value root;
    Json::Reader reader;
    
    bool parsesuccess = reader.parse( ValueString(result.second), root );
    if ( !parsesuccess )
    {
        std::cout  << "Failed to parse inbox\n" << reader.getFormattedErrorMessages();
        return inbox;
    }
    
    const Json::Value inboxMessages = root["inboxMessages"];
    for ( int index = 0; index < inboxMessages.size(); ++index ){  // Iterates over the sequence elements.
        
        // We need to sanitize our string, or else it will get cut off because of the newlines.
        std::string dirtyMessage = inboxMessages[index].get("message", "").asString();
        dirtyMessage.erase(std::remove(dirtyMessage.begin(), dirtyMessage.end(), '\n'), dirtyMessage.end());
        base64 cleanMessage(dirtyMessage, true);
        
        BitInboxMessage message(inboxMessages[index].get("msgid", "").asString(), inboxMessages[index].get("toAddress", "").asString(), inboxMessages[index].get("fromAddress", "").asString(), base64(inboxMessages[index].get("subject", "").asString(), true), cleanMessage, inboxMessages[index].get("encodingType", 0).asInt(), std::atoi(inboxMessages[index].get("receivedTime", 0).asString().c_str()), inboxMessages[index].get("read", false).asBool());
        
        inbox.push_back(message);
        
    }
    
    return inbox;

};


BitInboxMessage BitMessage::getInboxMessageByID(std::string msgID, bool setRead){

    Parameters params;
    
    params.push_back(ValueString(msgID));
    params.push_back(ValueBool(setRead));

    XmlResponse result = m_xmllib->run("getInboxMessageByID", params);
    
    if(result.first == false){
        std::cout << "Error: getInboxMessageByID failed" << std::endl;
        BitInboxMessage message("", "", "", base64(""), base64(""), 0, 0, false);
        return message;
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            BitInboxMessage message("", "", "", base64(""), base64(""), 0, 0, false);
            return message;
        }
    }
    
    
    Json::Value root;
    Json::Reader reader;
    
    bool parsesuccess = reader.parse( ValueString(result.second), root );
    if ( !parsesuccess )
    {
        std::cout  << "Failed to parse inbox\n" << reader.getFormattedErrorMessages();
        BitInboxMessage message("", "", "", base64(""), base64(""), 0, 0, false);
        return message;
    }
    
    const Json::Value inboxMessage = root["inboxMessage"];
    
    std::string dirtyMessage = inboxMessage[0].get("message", "").asString();
    dirtyMessage.erase(std::remove(dirtyMessage.begin(), dirtyMessage.end(), '\n'), dirtyMessage.end());
    base64 cleanMessage(dirtyMessage, true);
    
    
    BitInboxMessage message(inboxMessage[0].get("msgid", "").asString(), inboxMessage[0].get("toAddress", "").asString(), inboxMessage[0].get("fromAddress", "").asString(), base64(inboxMessage[0].get("subject", "").asString(), true), cleanMessage, inboxMessage[0].get("encodingType", 0).asInt(), std::atoi(inboxMessage[0].get("receivedTime", 0).asString().c_str()), inboxMessage[0].get("read", false).asBool());
     
    return message;

};


BitMessageOutbox BitMessage::getAllSentMessages(){

    Parameters params;
    BitMessageOutbox outbox;
    
    XmlResponse result = m_xmllib->run("getAllSentMessages", params);
    
    if(result.first == false){
        std::cout << "Error: getAllSentMessages failed" << std::endl;;
        return outbox;
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            BitInboxMessage message("", "", "", base64(""), base64(""), 0, 0, false);
            return outbox;
        }
    }
    
    Json::Value root;
    Json::Reader reader;
    
    const Json::Value sentMessages = root["sentMessages"];
    for ( int index = 0; index < sentMessages.size(); ++index ){  // Iterates over the sequence elements.
        
        // We need to sanitize our string, or else it will get cut off because of the newlines.
        std::string dirtyMessage = sentMessages[index].get("message", "").asString();
        dirtyMessage.erase(std::remove(dirtyMessage.begin(), dirtyMessage.end(), '\n'), dirtyMessage.end());
        base64 cleanMessage(dirtyMessage, true);
        
        BitSentMessage message(sentMessages[index].get("msgid", "").asString(), sentMessages[index].get("toAddress", "").asString(), sentMessages[index].get("fromAddress", "").asString(), base64(sentMessages[index].get("subject", "").asString(), true), cleanMessage, sentMessages[index].get("encodingType", 0).asInt(), std::atoi(sentMessages[index].get("lastActionTime", 0).asString().c_str()), sentMessages[index].get("status", false).asString(), sentMessages[index].get("ackData", false).asString());
        
        outbox.push_back(message);
        
    }

   return outbox;
    
};


BitSentMessage BitMessage::getSentMessageByID(std::string msgID){
    
    Parameters params;
    
    params.push_back(ValueString(msgID));
    
    XmlResponse result = m_xmllib->run("getSentMessageByID", params);
    
    if(result.first == false){
        std::cout << "Error: getSentMessageByID failed" << std::endl;;
        return BitSentMessage("", "", "", base64(""), base64(""), 0, 0, "", "");
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return BitSentMessage("", "", "", base64(""), base64(""), 0, 0, "", "");
        }
    }
    
    
    Json::Value root;
    Json::Reader reader;
    
    bool parsesuccess = reader.parse( ValueString(result.second), root );
    if ( !parsesuccess )
    {
        std::cout  << "Failed to parse outbox\n" << reader.getFormattedErrorMessages();
        return BitSentMessage("", "", "", base64(""), base64(""), 0, 0, "", "");
    }
    
    const Json::Value sentMessage = root["sentMessage"];
    
    std::string dirtyMessage = sentMessage[0].get("message", "").asString();
    dirtyMessage.erase(std::remove(dirtyMessage.begin(), dirtyMessage.end(), '\n'), dirtyMessage.end());
    base64 cleanMessage(dirtyMessage, true);
    
    
    BitSentMessage message(sentMessage[0].get("msgid", "").asString(), sentMessage[0].get("toAddress", "").asString(), sentMessage[0].get("fromAddress", "").asString(), base64(sentMessage[0].get("subject", "").asString(), true), cleanMessage, sentMessage[0].get("encodingType", 0).asInt(), sentMessage[0].get("lastActionTime", 0).asInt(), sentMessage[0].get("status", false).asString(), sentMessage[0].get("ackData", false).asString());
    
    return message;
    
};


BitSentMessage BitMessage::getSentMessageByAckData(std::string ackData){

    Parameters params;
    
    params.push_back(ValueString(ackData));
    
    XmlResponse result = m_xmllib->run("getSentMessageByAckData", params);
    
    if(result.first == false){
        std::cout << "Error: getSentMessageByAckData failed" << std::endl;;
        return BitSentMessage("", "", "", base64(""), base64(""), 0, 0, "", "");
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return BitSentMessage("", "", "", base64(""), base64(""), 0, 0, "", "");
        }
    }
    
    
    Json::Value root;
    Json::Reader reader;
    
    bool parsesuccess = reader.parse( ValueString(result.second), root );
    if ( !parsesuccess )
    {
        std::cout  << "Failed to parse outbox\n" << reader.getFormattedErrorMessages();
        return BitSentMessage("", "", "", base64(""), base64(""), 0, 0, "", "");
    }
    
    const Json::Value sentMessage = root["sentMessage"];
    
    std::string dirtyMessage = sentMessage[0].get("message", "").asString();
    dirtyMessage.erase(std::remove(dirtyMessage.begin(), dirtyMessage.end(), '\n'), dirtyMessage.end());
    base64 cleanMessage(dirtyMessage, true);
    
    
    BitSentMessage message(sentMessage[0].get("msgid", "").asString(), sentMessage[0].get("toAddress", "").asString(), sentMessage[0].get("fromAddress", "").asString(), base64(sentMessage[0].get("subject", "").asString(), true), cleanMessage, sentMessage[0].get("encodingType", 0).asInt(), sentMessage[0].get("lastActionTime", 0).asInt(), sentMessage[0].get("status", false).asString(), sentMessage[0].get("ackData", false).asString());
    
    return message;
    
};


std::vector<BitSentMessage> BitMessage::getSentMessagesBySender(std::string address){

    
    Parameters params;
    BitMessageOutbox outbox;
    
    params.push_back(ValueString(address));
    
    XmlResponse result = m_xmllib->run("getSentMessagesBySender", params);
    
    if(result.first == false){
        std::cout << "Error: getAllSentMessages failed" << std::endl;;
        return outbox;
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            BitInboxMessage message("", "", "", base64(""), base64(""), 0, 0, false);
            return outbox;
        }
    }
    
    Json::Value root;
    Json::Reader reader;
    
    const Json::Value sentMessages = root["sentMessages"];
    for ( int index = 0; index < sentMessages.size(); ++index ){  // Iterates over the sequence elements.
        
        // We need to sanitize our string, or else it will get cut off because of the newlines.
        std::string dirtyMessage = sentMessages[index].get("message", "").asString();
        dirtyMessage.erase(std::remove(dirtyMessage.begin(), dirtyMessage.end(), '\n'), dirtyMessage.end());
        base64 cleanMessage(dirtyMessage, true);
        
        BitSentMessage message(sentMessages[index].get("msgid", "").asString(), sentMessages[index].get("toAddress", "").asString(), sentMessages[index].get("fromAddress", "").asString(), base64(sentMessages[index].get("subject", "").asString(), true), cleanMessage, sentMessages[index].get("encodingType", 0).asInt(), std::atoi(sentMessages[index].get("lastActionTime", 0).asString().c_str()), sentMessages[index].get("status", false).asString(), sentMessages[index].get("ackData", false).asString());
        
        outbox.push_back(message);
        
    }
    
    return outbox;

};


bool BitMessage::trashMessage(std::string msgID){

    Parameters params;
    params.push_back(ValueString(msgID));
    
    XmlResponse result = m_xmllib->run("trashMessage", params);
    
    if(result.first == false){
        std::cout << "Error: trashMessage failed" << std::endl;
        return false;
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return false;
        }
    }
    
    return true;
    
};


bool BitMessage::trashSentMessageByAckData(std::string ackData){

    Parameters params;
    params.push_back(ValueString(ackData));
    
    XmlResponse result = m_xmllib->run("trashSentMessageByAckData", params);
    
    if(result.first == false){
        std::cout << "Error: trashSentMessageByAckData failed" << std::endl;
        return false;
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return false;
        }
    }
    
    return true;

};



// Message Management


std::string BitMessage::sendMessage(std::string fromAddress, std::string toAddress, base64 subject, base64 message, int encodingType){

    Parameters params;
    params.push_back(ValueString(fromAddress));
    params.push_back(ValueString(toAddress));
    params.push_back(ValueString(subject.encoded()));
    params.push_back(ValueString(message.encoded()));
    params.push_back(ValueInt(encodingType));

    
    XmlResponse result = m_xmllib->run("sendMessage", params);
    
    if(result.first == false){
        std::cout << "Error: sendMessage failed" << std::endl;
        return "";
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return "";
        }
    }
    
    return std::string(ValueString(result.second));

};


std::string BitMessage::sendBroadcast(std::string fromAddress, base64 subject, base64 message, int encodingType){

    Parameters params;
    params.push_back(ValueString(fromAddress));
    params.push_back(ValueString(subject.encoded()));
    params.push_back(ValueString(message.encoded()));
    params.push_back(ValueInt(encodingType));
    
    
    XmlResponse result = m_xmllib->run("sendBroadcast", params);
    
    if(result.first == false){
        std::cout << "Error: sendBroadcast failed" << std::endl;
        return "";
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return "";
        }
    }
    
    return std::string(ValueString(result.second));

};



// Subscription Management


BitMessageSubscriptionList BitMessage::listSubscriptions(){

    Parameters params;
    BitMessageSubscriptionList subscriptionList;
    
    XmlResponse result = m_xmllib->run("listSubscriptions", params);
    
    if(result.first == false){
        std::cout << "Error: listSubscriptions failed" << std::endl;
        return subscriptionList;
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return subscriptionList;
        }
    }
    
    Json::Value root;
    Json::Reader reader;
    
    bool parsesuccess = reader.parse( ValueString(result.second), root );
    if ( !parsesuccess )
    {
        std::cout  << "Failed to parse subscription list\n" << reader.getFormattedErrorMessages();
        return subscriptionList;
    }
    
    const Json::Value subscriptions = root["subscriptions"];
    for ( int index = 0; index < subscriptions.size(); ++index ){  // Iterates over the sequence elements.
        
        std::string dirtyLabel = subscriptions[index].get("label", "").asString();
        dirtyLabel.erase(std::remove(dirtyLabel.begin(), dirtyLabel.end(), '\n'), dirtyLabel.end());
        std::string cleanRipe(dirtyLabel);
        
        BitMessageSubscription subscription(subscriptions[index].get("address", "").asString(), subscriptions[index].get("enabled", "").asBool(), base64(cleanRipe, true));
        
        subscriptionList.push_back(subscription);
        
    }
    
    return subscriptionList;

};


bool BitMessage::addSubscription(std::string address, base64 label){

    Parameters params;
    params.push_back(ValueString(address));
    params.push_back(ValueString(label.encoded()));

    
    XmlResponse result = m_xmllib->run("addSubscription", params);
    
    if(result.first == false){
        std::cout << "Error: createChan failed" << std::endl;
        return false;
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return false;
        }
    }
    
    return true;

};


bool BitMessage::deleteSubscription(std::string address){

    Parameters params;
    params.push_back(ValueString(address));
    
    XmlResponse result = m_xmllib->run("deleteSubscription", params);
    
    if(result.first == false){
        std::cout << "Error: createChan failed" << std::endl;
        return false;
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return false;
        }
    }
    
    return true;

};



// Channel Management


std::string BitMessage::createChan(base64 password){

    Parameters params;
    params.push_back(ValueString(password.encoded()));
    
    XmlResponse result = m_xmllib->run("createChan", params);
    
    if(result.first == false){
        std::cout << "Error: createChan failed" << std::endl;
        return "";
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return "";
        }
    }
    
    return std::string(ValueString(result.second));

};


bool BitMessage::joinChan(base64 password, std::string address){

    Parameters params;
    params.push_back(ValueString(password.encoded()));
    params.push_back(ValueString(address));
    
    XmlResponse result = m_xmllib->run("joinChan", params);
    
    if(result.first == false){
        std::cout << "Error: joinChan failed" << std::endl;
        return false;
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return false;
        }
    }
    
    return true;

};


bool BitMessage::leaveChan(std::string address){

    Parameters params;
    params.push_back(ValueString(address));
    
    XmlResponse result = m_xmllib->run("leaveChan", params);
    
    if(result.first == false){
        std::cout << "Error: leaveChan failed" << std::endl;
        return false;
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return false;
        }
    }
    
    return true;
};




// Address Management


BitMessageIdentities BitMessage::listAddresses(){
    
    std::vector<xmlrpc_c::value> params;
    BitMessageIdentities responses;

    XmlResponse result = m_xmllib->run("listAddresses2", params);
    
    if(result.first == false){
        std::cout << "Error: listAddresses2 failed" << std::endl;
        return responses;
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return responses;
        }
    }
    
    Json::Value root;
    Json::Reader reader;
    
    bool parsesuccess = reader.parse( ValueString(result.second), root );
    if ( !parsesuccess )
    {
        std::cout  << "Failed to parse configuration\n" << reader.getFormattedErrorMessages();
        return responses;
    }
    
    const Json::Value addresses = root["addresses"];
    for ( int index = 0; index < addresses.size(); ++index ){  // Iterates over the sequence elements.
        BitMessageIdentity entry(base64(addresses[index].get("label", "").asString()), addresses[index].get("address", "").asString(), addresses[index].get("stream", 0).asInt(), addresses[index].get("enabled", false).asBool(), addresses[index].get("chan", false).asBool());
        
        responses.push_back(entry);
        
    }
    
    return responses;
    
}


BitMessageAddress BitMessage::createRandomAddress(base64 label, bool eighteenByteRipe, int totalDifficulty, int smallMessageDifficulty){

    Parameters params;
    params.push_back(ValueString(label.encoded()));
    params.push_back(ValueBool(eighteenByteRipe));
    params.push_back(ValueInt(totalDifficulty));
    params.push_back(ValueInt(smallMessageDifficulty));

    
    XmlResponse result = m_xmllib->run("createRandomAddress", params);
    
    if(result.first == false){
        std::cout << "Error: createRandomAddress failed" << std::endl;
        return "";
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return "";
        }
    }
    
    return std::string(ValueString(result.second));

};


std::vector<BitMessageAddress> BitMessage::createDeterministicAddresses(base64 password, int numberOfAddresses, int addressVersionNumber, int streamNumber, bool eighteenByteRipe, int totalDifficulty, int smallMessageDifficulty){

    Parameters params;
    std::vector<BitMessageAddress> addressList;
    
    params.push_back(ValueString(password.encoded()));
    params.push_back(ValueInt(numberOfAddresses));
    params.push_back(ValueInt(addressVersionNumber));
    params.push_back(ValueInt(streamNumber));
    params.push_back(ValueBool(eighteenByteRipe));
    params.push_back(ValueInt(totalDifficulty));
    params.push_back(ValueInt(smallMessageDifficulty));

    
    XmlResponse result = m_xmllib->run("createDeterministicAddresses", params);
    
    if(result.first == false){
        std::cout << "Error: createDeterministicAddresses failed" << std::endl;
        return addressList;
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return addressList;
        }
    }
    
    Json::Value root;
    Json::Reader reader;
    
    bool parsesuccess = reader.parse( ValueString(result.second), root );
    if ( !parsesuccess )
    {
        std::cout  << "Failed to parse address list\n" << reader.getFormattedErrorMessages();
        return addressList;
    }
    
    const Json::Value addresses = root["addresses"];
    for ( int index = 0; index < addresses.size(); ++index ){  // Iterates over the sequence elements.
        BitMessageAddress generatedAddress = addresses[index].asString();
        
        addressList.push_back(generatedAddress);
        
    }
    
    return addressList;

};


BitMessageAddress BitMessage::getDeterministicAddress(base64 password, int addressVersionNumber, int streamNumber){
    
    Parameters params;
    params.push_back(ValueString(password.encoded()));
    params.push_back(ValueInt(addressVersionNumber));
    params.push_back(ValueInt(streamNumber));
    
    
    XmlResponse result = m_xmllib->run("getDeterministicAddress", params);
    
    if(result.first == false){
        std::cout << "Error: getDeterministicAddress failed" << std::endl;
        return "";
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return "";
        }
    }
    
    return std::string(ValueString(result.second));

};


BitMessageAddressBook BitMessage::listAddressBookEntries(){

    Parameters params;
    
    BitMessageAddressBook addressBook;
    
    XmlResponse result = m_xmllib->run("listAddressBookEntries", params);
    
    if(result.first == false){
        std::cout << "Error: listAddressBookEntries failed" << std::endl;
        return addressBook;
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return addressBook;
        }
    }
    
    Json::Value root;
    Json::Reader reader;
    
    bool parsesuccess = reader.parse( ValueString(result.second), root );
    if ( !parsesuccess )
    {
        std::cout  << "Failed to parse address list\n" << reader.getFormattedErrorMessages();
        return addressBook;
    }
    
    const Json::Value addresses = root["addresses"];
    for ( int index = 0; index < addresses.size(); ++index ){  // Iterates over the sequence elements.
        
        std::string dirtyLabel = addresses[index].get("label", "").asString();
        dirtyLabel.erase(std::remove(dirtyLabel.begin(), dirtyLabel.end(), '\n'), dirtyLabel.end());
        std::string cleanRipe(dirtyLabel);
        
        BitMessageAddressBookEntry address(addresses[index].get("address", "").asString(), base64(cleanRipe, true));
        
        addressBook.push_back(address);
        
    }
    
    return addressBook;
    
};


bool BitMessage::addAddressBookEntry(std::string address, base64 label){

    Parameters params;
    params.push_back(ValueString(address));
    params.push_back(ValueString(label.encoded()));
    
    XmlResponse result = m_xmllib->run("addAddressBookEntry", params);
    
    if(result.first == false){
        std::cout << "Error: addAddressBookEntry failed" << std::endl;
        return false;
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return false;
        }
    }
    
    std::cout << "BitMessage API Response: " << std::string(ValueString(result.second)) << std::endl;
    
    return true;

};


bool BitMessage::deleteAddressBookEntry(std::string address){

    Parameters params;
    params.push_back(ValueString(address));
    
    XmlResponse result = m_xmllib->run("deleteAddressBookEntry", params);
    
    if(result.first == false){
        std::cout << "Error: deleteAddressBookEntry failed" << std::endl;
        return false;
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return false;
        }
    }
    
    std::cout << "BitMessage API Response: " << std::string(ValueString(result.second)) << std::endl;
    
    return true;

};


bool BitMessage::deleteAddress(std::string address){

    Parameters params;
    params.push_back(ValueString(address));
    
    XmlResponse result = m_xmllib->run("deleteAddress", params);
    
    if(result.first == false){
        std::cout << "Error: deleteAddress failed" << std::endl;
        return false;
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return false;
        }
    }
    
    return true;

};


BitDecodedAddress BitMessage::decodeAddress(std::string address){

    Parameters params;
    
    params.push_back(ValueString(address));
    
    XmlResponse result = m_xmllib->run("decodeAddress", params);
    
    if(result.first == false){
        std::cout << "Error Accessing BitMessage API" << std::endl;
        return BitDecodedAddress("", 0, "", 0);
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return BitDecodedAddress("", 0, "", 0);
        }
    }
    
    
    Json::Value root;
    Json::Reader reader;
    
    bool parsesuccess = reader.parse( ValueString(result.second), root );
    if ( !parsesuccess )
    {
        std::cout  << "Failed to parse configuration\n" << reader.getFormattedErrorMessages();
        return BitDecodedAddress("", 0, "", 0);
    }
    
    const Json::Value decodedAddress = root;
    
    std::string dirtyRipe = decodedAddress.get("ripe", "").asString();
    dirtyRipe.erase(std::remove(dirtyRipe.begin(), dirtyRipe.end(), '\n'), dirtyRipe.end());
    std::string cleanRipe(dirtyRipe);
    
    return BitDecodedAddress(decodedAddress.get("status", "").asString(), decodedAddress.get("addressVersion", "").asInt(), cleanRipe, decodedAddress.get("streamNumber", "").asInt());

};





// Other API Commands


std::string BitMessage::helloWorld(std::string first, std::string second){
    
    Parameters params;
    params.push_back(ValueString(first));
    params.push_back(ValueString(second));
    
    XmlResponse result = m_xmllib->run("helloWorld", params);
    
    if(result.first == false){
        std::cout << "Error Accessing BitMessage API" << std::endl;
        return "";
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return "";
        }
    }
    
    return std::string(ValueString(result.second));
    
}


int BitMessage::add(int x, int y){
    
    Parameters params;
    params.push_back(ValueInt(x));
    params.push_back(ValueInt(y));

    XmlResponse result = m_xmllib->run("add", params);
    
    if(result.first == false){
        std::cout << "Error: add failed" << std::endl;
        return -1;
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return -1;
        }
        std::cout << "Unexpected Response to API Command: add" << std::endl;
        std::cout << std::string(ValueString(result.second)) << std::endl;
        return -1;
    }
    else{
        return ValueInt(result.second);
    }
    
};


std::string BitMessage::getStatus(std::string ackData){

    Parameters params;
    
    params.push_back(ValueString(ackData));

    XmlResponse result = m_xmllib->run("getStatus", params);
    
    if(result.first == false){
        std::cout << "Error Accessing BitMessage API" << std::endl;
        return "";
    }
    else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
        std::size_t found;
        found=std::string(ValueString(result.second)).find("API Error");
        if(found!=std::string::npos){
            std::cout << std::string(ValueString(result.second)) << std::endl;
            return "";
        }
    }
    
    return std::string(ValueString(result.second));

};


void BitMessage::setServerAlive(bool alive){
    
    if(alive){
        NetCounter::setAlive();
        m_serverAvailable = true;
    }
    else{
        NetCounter::dead();
        m_serverAvailable = false;
    }
    
}

