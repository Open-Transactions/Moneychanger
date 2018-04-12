//
//  Network.cpp
//  Auditor
//


#include "Network.h"

#include <QtGlobal>

bool NetworkModule::addContact(std::string label, std::string address)
{
    Q_UNUSED(label);
    Q_UNUSED(address);

    return false;
}

std::string NetworkModule::getLabel(std::string address)
{
    Q_UNUSED(address);

    return "";
}

bool NetworkModule::setLabel(std::string label, std::string address)
{
    Q_UNUSED(label);
    Q_UNUSED(address);

    return false;
}

std::string NetworkModule::getAddressFromLabel(std::string label)
{
    Q_UNUSED(label);

    return "";
}

bool NetworkModule::createBroadcastAddress()
{
    return false;
}

bool NetworkModule::broadcastOnAddress(std::string address)
{
    Q_UNUSED(address);

    return false;
}

bool NetworkModule::subscribeToAddress(std::string address)
{
    Q_UNUSED(address);

    return false;
}





bool NetworkModule::createAddress(std::string label/*=""*/)
{
    Q_UNUSED(label);

    return false;
}

bool NetworkModule::createDeterministicAddress(std::string key, std::string label/*=""*/)
{
    Q_UNUSED(key);
    Q_UNUSED(label);

    return false;
}

bool NetworkModule::deleteLocalAddress(std::string address)
{
    Q_UNUSED(address);

    return false;
}

bool NetworkModule::addressAccessible(std::string address)
{
    Q_UNUSED(address);

    return false;
}  // Checks to see if an address is useable for sending messages.

bool NetworkModule::newMailExists(std::string address/*=""*/)
{
    Q_UNUSED(address);

    return false;
} // checks for new mail, returns true if there is new mail in the queue.
// In BitMessage this checks for unread mail in the queue, it will return true
// If a read message has been marked unread manually.

std::vector<_SharedPtr<NetworkMail> > NetworkModule::getInbox(std::string address/*=""*/)
{
    Q_UNUSED(address);

    return std::vector<_SharedPtr<NetworkMail> >();
}

std::vector<_SharedPtr<NetworkMail> > NetworkModule::getOutbox(std::string address/*=""*/)
{
    Q_UNUSED(address);

    return std::vector<_SharedPtr<NetworkMail> >();
}

std::vector<_SharedPtr<NetworkMail> > NetworkModule::getUnreadMail(std::string address)
{
    Q_UNUSED(address);

    return std::vector<_SharedPtr<NetworkMail> >();
}

bool NetworkModule::deleteMessage(std::string messageID)
{
    Q_UNUSED(messageID);

    return false;
} // passed as a string, as different protocols handle message ID's differently (BitMessage for example)

bool NetworkModule::deleteOutMessage(std::string messageID)
{
    Q_UNUSED(messageID);

    return false;
} // passed as a string, as different protocols handle message ID's differently (BitMessage for example)

bool NetworkModule::markRead(std::string messageID, bool read/*=true*/)
{
    Q_UNUSED(messageID);
    Q_UNUSED(read);

    return false;
} // By default this marks a given message as read or not, not all API's will support this and should thus return false.

bool NetworkModule::sendMail(NetworkMail message)
{
    Q_UNUSED(message);

    return false;
} // Need To, From, Subject and Message in formatted NetworkMail object


