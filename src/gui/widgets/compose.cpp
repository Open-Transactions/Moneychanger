#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/compose.hpp>
#include <ui_compose.h>

#include <gui/widgets/overridecursor.hpp>
#include <gui/widgets/dlgchooser.hpp>

#include <core/moneychanger.hpp>
#include <core/handlers/DBHandler.hpp>
#include <core/handlers/contacthandler.hpp>
#include <core/mtcomms.h>

#include <opentxs/opentxs.hpp>

#include <QMessageBox>
#include <QDebug>
#include <QStringList>


void MTCompose::setInitialBody(QString body)  { m_body = body; }
void MTCompose::setInitialSubject(QString subject)  { m_subject = subject; }


void MTCompose::setInitialSenderAddress(QString address)  // Bitmessage address would go here.
{
    m_senderAddress = address;

    if (!address.isEmpty()) // if address is known...
    {
        // Use address to find method type
        //
        QString senderMethodType = MTContactHandler::getInstance()->GetMethodType(address);

        // The type may not be known from the address, and may already be set.
        // Therefore we don't replace it unless we actually found a type based
        // on the address. (Must be not-empty.) Otherwise we might overwrite
        // a perfectly valid type with a blank value.
        //
        if (!senderMethodType.isEmpty())
            setInitialMsgType(senderMethodType);

        // Note that if the recipient had already been set and that had already set the msg_type,
        // we might have just changed it. But in the case where the sender and recipient's msg type
        // do not match, it is not resolved here, but upon sending.
    }

    setSenderNameBasedOnAvailableData();
}

void MTCompose::setInitialRecipientAddress(QString address) // Bitmessage address would go here.
{
    m_recipientAddress = address;

    // Note: a sender has a method ID, whereas a recipient has a contact ID.
    // But how do we KNOW for a fact that the recipient isn't one of MY Nyms?
    // (who in fact DOES have a method ID...)
    //
    // Therefore, we could still look up the method type based on the address,
    // if it's available, and set the msgtype IF it wasn't already set.
    //
    if (!address.isEmpty()) // if address is known...
    {
        // Use address to find method type
        //
        QString recipientMethodType = MTContactHandler::getInstance()->GetMethodType(address);

        // If msgtype is empty, set it to the one we found based on the address.
        //
        if (m_msgtype.isEmpty())
            setInitialMsgType(recipientMethodType);
        //
        // else (if msgtype is NOT empty), but we found one based on the address,
        // are they the same? If not, say something about it.
        //
        else if (!recipientMethodType.isEmpty() && (0 != m_msgtype.compare(recipientMethodType)))
        {
            // Well the sender's method ID isn't set anyway, so let's go ahead
            // and set the msgtype based on the one we found.
            if (0 == m_senderMethodId)
                setInitialMsgType(recipientMethodType);
            else // Sender's method ID IS already set...
            {
                // Let's see what the method type is for the sender based on his already-set methodId...
                //
                QString senderMethodType = MTContactHandler::getInstance()->GetMethodType(m_senderMethodId);

                // senderMethodType and recipientMethodType do not match!
                // In this case it will have to be resolved on the actual send. (Not here.)
                //
                if (0 != senderMethodType.compare(recipientMethodType))
                    qDebug() << QString("MTCompose::setInitialRecipient: Warning: address %1 has msg type %2 but "
                                        "current type was already set to %3, and sender's msg type is set to %4.").
                                arg(address).arg(recipientMethodType).arg(m_msgtype).arg(senderMethodType);

                // In this case, senderMethodType and recipientMethodType DO MATCH,
                // but we already know they do NOT match m_msgtype, which does exist.
                // Therefore, change m_msgtype to match with senderMethodType/recipientMethodType
                // (Since they already match with each other.) And BTW, should never happen,
                // since m_senderMethodId is already set, so the msgtype should have been set
                // at the same time.
                else
                    setInitialMsgType(recipientMethodType);
            }
        }
    }

    setRecipientNameBasedOnAvailableData();
}


void MTCompose::setInitialSenderNym(QString nymId, QString address/*=""*/)
{
    m_senderNymId    = nymId;   // Sender NymID should always be available.
    m_senderMethodId = 0;       // Method #5 might be the sender's Bitmessage connection. Mandatory if msgtype is "bitmessage" (or anything other than otserver.)

    setInitialSenderAddress(address); // If msgtype is "bitmessage" this will contain a Bitmessage address. For msgtype "otserver", address is blank.
    // NOTE: the display name is also set in this call.
}



void MTCompose::setTransportDisplayBasedOnAvailableData()
{
    if (NULL != ui)
    {
        QString qstrMsgTypeDisplay("");

        if (sendingThroughOTServer() && !m_NotaryID.isEmpty())
        {
            qstrMsgTypeDisplay = QString::fromStdString(opentxs::OT::App().API().Exec().GetServer_Name(m_NotaryID.toStdString()));

            if (qstrMsgTypeDisplay.isEmpty())
                qstrMsgTypeDisplay = m_NotaryID;
        }
        // --------------------------------------------------
        else if (!m_msgtype.isEmpty() && !sendingThroughOTServer())
        {
            qstrMsgTypeDisplay = QString::fromStdString(MTComms::displayName(m_msgtype.toStdString()));

            if (qstrMsgTypeDisplay.isEmpty())
                qstrMsgTypeDisplay = m_msgtype;
        }
        // --------------------------------------------------
        ui->viaButton->setText(qstrMsgTypeDisplay.isEmpty() ? tr("<Choose Transport>") : qstrMsgTypeDisplay);
    }
}

void MTCompose::setSenderNameBasedOnAvailableData()
{
    if (NULL != ui)
    {
        QString qstrNymName("");

        if (!m_senderNymId.isEmpty())
        {
            MTNameLookupQT theLookup;

            qstrNymName = QString::fromStdString(theLookup.GetNymName(m_senderNymId.toStdString(), ""));
        }
        // ---------------------------
        QString qstrAddressPortion("");

        if (!this->m_senderAddress.isEmpty())
            qstrAddressPortion = QString(" (%1)").arg(this->m_senderAddress);
        else if (!bCanMessage_ && !sendingThroughOTServer())
        {
            QString qstrMsgTypeDisplay = m_msgtype.isEmpty() ? QString(tr("transport or ")) :
                                                               QString("%1 ").arg(QString::fromStdString(MTComms::displayName(m_msgtype.toStdString())));

            qstrAddressPortion = QString(" (%1 %2%3)").arg(tr("No")).arg(qstrMsgTypeDisplay).arg(tr("address available"));
        }
        // ---------------------------
        if (qstrNymName.isEmpty())
        {
            if (m_senderNymId.isEmpty())
                qstrNymName = tr("<Choose Sender>");
            else
                qstrNymName = tr("(Sender name is blank)");
        }
        // ---------------------------
        QString qstrFinalName = QString("%1%2").arg(qstrNymName).arg(qstrAddressPortion);

        ui->fromButton->setText(qstrFinalName);
    }
}

void MTCompose::setRecipientNameBasedOnAvailableData()
{
    // If there's no contact ID, but there IS a Nym ID, let's go ahead
    // and try to find the contact ID based on the Nym ID, just for shits
    // and giggles. (We may or may not find it.)
    //
    if ((m_recipientContactId <= 0) && (!m_recipientNymId.isEmpty()))
        m_recipientContactId = MTContactHandler::getInstance()->FindContactIDByNymID(m_recipientNymId);
    // ---------------------------------------------------------------
    if (NULL != ui)
    {
        QString qstrNymName("");

        // ---------------------------
        QString qstrContactName;

        if (bCanMessage_)
        {
            MTNameLookupQT theLookup;
            qstrContactName = QString::fromStdString(theLookup.GetContactName(qstrContactId_.toStdString()));
        }
        else if (!m_recipientNymId.isEmpty())
        {
            MTNameLookupQT theLookup;
            qstrNymName = QString::fromStdString(theLookup.GetNymName(m_recipientNymId.toStdString(), ""));
        }
        else if (qstrContactName.isEmpty() && m_recipientContactId > 0)
        {
            qstrContactName = MTContactHandler::getInstance()->GetContactName(m_recipientContactId);
        }
        // ---------------------------
        if (qstrContactName.isEmpty())
            qstrContactName = qstrNymName;
        // ---------------------------
        QString qstrAddressPortion("");

        if (!this->m_recipientAddress.isEmpty())
        {
            qstrAddressPortion = QString(" (%1)").arg(this->m_recipientAddress);
            // ---------------------------
            if (qstrContactName.isEmpty())
            {
                MTNameLookupQT theLookup;
                qstrContactName = QString::fromStdString(theLookup.GetAddressName(this->m_recipientAddress.toStdString()));
            }
        }
        else if (!bCanMessage_ && !sendingThroughOTServer())
        {
            QString qstrMsgTypeDisplay = m_msgtype.isEmpty() ? QString(tr("transport or ")) :
                                                               QString("%1 ").arg(QString::fromStdString(MTComms::displayName(m_msgtype.toStdString())));

            qstrAddressPortion = QString(" (%1 %2%3)").arg(tr("No")).arg(qstrMsgTypeDisplay).arg(tr("address available"));
        }
        // ---------------------------
        if (qstrContactName.isEmpty())
        {
            if (m_recipientNymId.isEmpty() && (m_recipientContactId <= 0) && this->m_recipientAddress.isEmpty())
                qstrContactName = tr("<Choose Recipient>");
            else
                qstrContactName = tr("(Contact name is blank)");
        }
        // ---------------------------
        QString qstrFinalName = QString("%1%2").arg(qstrContactName).arg(qstrAddressPortion);

        ui->toButton->setText(qstrFinalName);
    }
}

void MTCompose::setInitialRecipientNym(QString nymId, QString address/*=""*/) // Also sets the Contact ID, if one was found for this Nym.
{
    m_recipientNymId     = nymId;       // If not available, then m_recipientContactID must be available. (For Bitmessage, for example, Nym is optional.)
    m_recipientContactId = 0;           // Recipient Nym kjsdfds982345 might be Contact #2. (Or Nym itself might be blank, with ONLY Contact!)

    const int nContactID = MTContactHandler::getInstance()->FindContactIDByNymID(nymId);

    if (nContactID > 0)
        m_recipientContactId = nContactID;

    setInitialRecipientAddress(address); // If msgtype is "bitmessage" this will contain a Bitmessage address. For msgtype "otserver", address is blank.
    // NOTE: the display name is also set in this call.
}

bool MTCompose::setRecipientNymBasedOnContact()
{
    // If there's no Contact ID set, we return immediately.
    if (m_recipientContactId <= 0)
        return false;

    // OK, the contact ID is set...

    // If the NymID is already set, let's see if it already matches the Contact ID...
    if (!m_recipientNymId.isEmpty())
    {
        const int nContactID = MTContactHandler::getInstance()->FindContactIDByNymID(m_recipientNymId);

        if (nContactID == m_recipientContactId)
            return true;
    }

    // By this point it means either the recipient Nym is empty, or it was set but didn't
    // match the current Contact. Either way, let's try to find a better Nym...
    //
    mapIDName theMap;

    if (MTContactHandler::getInstance()->GetNyms(theMap, m_recipientContactId))
    {
        mapIDName::iterator it = theMap.begin();

        if (it != theMap.end())
        {
            m_recipientNymId = it.key();
            return true;
        }
    }

    return false;
}


void MTCompose::setInitialRecipientContactID(QString qstrContactid, QString address/*=""*/)
{
    QString qstrSenderNym = m_senderNymId;
    QString qstrDefaultNym = Moneychanger::It()->get_default_nym_id();
    if (qstrSenderNym.isEmpty() && !qstrDefaultNym.isEmpty()) {
        qstrSenderNym = qstrDefaultNym;
        m_senderNymId = qstrDefaultNym;
    }
    if (!qstrSenderNym.isEmpty() && !qstrContactid.isEmpty()) {
        bCanMessage_ = (opentxs::Messagability::READY == opentxs::OT::App().API().Sync().CanMessage(opentxs::Identifier(qstrSenderNym.toStdString()), opentxs::Identifier(qstrContactid.toStdString())));
    }
    // -------------------------------------------
    mapIDName theMap;
    const bool bGotNyms = MTContactHandler::getInstance()->GetNyms(theMap, qstrContactid.toStdString());

    if (bGotNyms) {
        //const bool bRecipientNymChosen =
                chooseRecipientNym(theMap);

//      if (bRecipientNymChosen)
//          return;
    }
    // -------------------------------------------
    if (bCanMessage_) {
        qstrContactId_ = qstrContactid;

        if (m_subject.isEmpty())
        {
            ui->subjectEdit->setVisible(false);
            ui->subjectLabel->setVisible(false);
        }

        ui->label_3->setVisible(false);
        ui->toolButton_3->setVisible(false);

        ui->toolButtonTo->setVisible(false);
        ui->toolButtonFrom->setVisible(false);

        ui->viaButton->setVisible(false);
        ui->toolButtonFrom->setEnabled(false);
        ui->toolButtonTo->setEnabled(false);

        ui->fromButton->setEnabled(false);
        ui->toButton->setEnabled(false);
        return;
    }
}


void MTCompose::setInitialRecipientContactID(int contactid, QString address/*=""*/)
{
    if (bCanMessage_)
        return;

    m_recipientContactId = contactid;    // Recipient Nym kjsdfds982345 might be Contact #2. (Or Nym itself might be blank, with ONLY Contact!)
    m_recipientNymId     = QString("");  // If not available, then m_recipientContactID must be available. (For Bitmessage, for example, Nym is optional.)

    setInitialRecipientAddress(address); // If msgtype is "bitmessage" this will contain a Bitmessage address. For msgtype "otserver", address is blank.
    // NOTE: the display name is also set in this call.
}

void MTCompose::setInitialRecipient(QString nymId, int contactid/*=0*/, QString address/*=""*/)
{
    if (bCanMessage_)
        return;

    m_recipientNymId     = nymId;       // If not available, then m_recipientContactID must be available. (For Bitmessage, for example, Nym is optional.)
    m_recipientContactId = contactid;   // Recipient Nym kjsdfds982345 might be Contact #2. (Or Nym itself might be blank, with ONLY Contact!)

    int nContactID = 0;

    if (!nymId.isEmpty())
        nContactID = MTContactHandler::getInstance()->FindContactIDByNymID(nymId);

    // If a contact ID was passed in, and one was found, and they weren't the same:
    //
    if ((contactid > 0) && (nContactID > 0) && (nContactID != contactid))
        qDebug() << QString("MTCompose::setInitialRecipient: Warning: contactID %1 supplied for nym ID %2 doesn't match the contactID expected (%3. Using it anyway.)").
                    arg(contactid).arg(nymId).arg(nContactID);

    // If a contact ID was NOT passed in, but one was found, set to the one found.
    //
    if ((0 == contactid) && (0 != nContactID))
        m_recipientContactId = nContactID;

    setInitialRecipientAddress(address); // If msgtype is "bitmessage" this will contain a Bitmessage address. For msgtype "otserver", address is blank.
    // NOTE: the display name is also set in this call.
}


// If you call as:   setInitialMsgType("", notary_id);
// it will work, and set "otserver" as msgtype.
//
void MTCompose::setInitialMsgType(QString msgtype, QString server/*=""*/)
{
    m_msgtype = server.isEmpty() ? msgtype : QString("otserver");

    setInitialServer(sendingThroughOTServer() ? server : QString(""));
}

void MTCompose::setInitialServer(QString NotaryID)
{
    if (bCanMessage_)
        return;

    // If someone passed in an empty server Id, but the msgType IS set,
    // then if msgType is "otserver", we keep the NotaryID we already had,
    // and we don't blank it out. But if msgtype is "bitmessage" or something,
    // then we blank out the OT Server ID.
    //
    if (NotaryID.isEmpty() && !m_msgtype.isEmpty())
    {
        if (0 != m_msgtype.compare("otserver"))
            m_NotaryID = QString("");
        // else if msgtype IS "otserver" (and NotaryID arg is empty) then we
        // preserve whatever value was already in m_NotaryID.
    }
    // -------------------
    if (!NotaryID.isEmpty())
    {
        m_msgtype  = QString("otserver");
        m_NotaryID = NotaryID;

        // These aren't used in the case of OT server, so we blank them out.
        // (Only Server ID and NymIDs are necessary in this case.)
        //
        m_senderAddress    = QString("");
        m_recipientAddress = QString("");
        m_senderMethodId   = 0; // Method #5 might be the sender's Bitmessage connection. Mandatory if msgtype is "bitmessage" (or anything other than otserver.)
    }
    // -------------------
    setTransportDisplayBasedOnAvailableData();
}

// ----------------------------------------------------------------------
bool MTCompose::sendMessage(QString subject,   QString body, QString fromNymId, QString toNymId, QString fromAddress, QString toAddress,
                            QString viaServer, QString viaTransport, int viaMethodID)
{
    NetworkModule * pModule = NULL;

    if (!bCanMessage_ && viaTransport.isEmpty())
    {
        qDebug() << "Cannot send a message via a blank transport type, aborting.";
        return false;
    }
    // ----------------------------------------------------
    if (fromNymId.isEmpty())
    {
        qDebug() << "Cannot send a message from an empty nym id, aborting.";
        return false;
    }
    // ----------------------------------------------------
    if (!bCanMessage_ && 0 == viaTransport.compare("otserver"))
    {
        if (viaServer.isEmpty())
        {
            qDebug() << "Cannot send a message via an empty server id, aborting.";
            return false;
        }

        if (toNymId.isEmpty())
        {
            qDebug() << "Cannot send a message to an empty nym id, aborting.";
            return false;
        }
    }
    else if (!bCanMessage_) // All other transport types.
    {
        if (fromAddress.isEmpty())
        {
            qDebug() << "Cannot send a message from an empty address, aborting.";
            return false;
        }

        if (toAddress.isEmpty())
        {
            qDebug() << "Cannot send a message to an empty address, aborting.";
            return false;
        }

        if (viaMethodID <= 0)
        {
            qDebug() << QString("Cannot send a message via a nonexistent %1 interface, aborting.").arg(viaTransport);
            return false;
        }
        // ------------------------------------------------------
        QString qstrCommString = MTContactHandler::getInstance()->GetMethodConnectStr(viaMethodID);

        if (qstrCommString.isEmpty())
        {
            // todo probably need a messagebox here.
            qDebug() << QString("Cannot send a message via a %1 interface that has an empty connection string, aborting.").arg(viaTransport);
            return false;
        }
        else
        {
            pModule = MTComms::find(qstrCommString.toStdString());

            if ((NULL == pModule) && MTComms::add(viaTransport.toStdString(), qstrCommString.toStdString()))
                pModule = MTComms::find(qstrCommString.toStdString());

            if (NULL == pModule)
            {
                // todo probably need a messagebox here.
                qDebug() << QString("Unable to add a %1 interface with connection string: %2").arg(viaTransport).arg(qstrCommString);
                return false;
            }
        }
    }
    // ----------------------------------------------------
//    if (subject.isEmpty())
//        subject = tr("From the desktop client. (Empty subject.)");
    // ----------------------------------------------------
    if (body.isEmpty())
        body = tr("From the desktop client. (Empty message body.)");
    // ----------------------------------------------------
    std::string str_NotaryID  (viaServer  .toStdString());
    std::string str_fromNymId (fromNymId  .toStdString());
    std::string str_fromAddr  (fromAddress.toStdString());
    std::string str_toNymId   (toNymId    .toStdString());
    std::string str_toAddr    (toAddress  .toStdString());
    // ----------------------------------------------------
    qDebug() << QString("Initiating sendMessage:\n Transport:'%1'\n Server:'%2'\n From Nym:'%3'\n From Address:'%4'\n To Nym:'%5'\n To Address:'%6'\n Subject:'%7'\n Body:'%8'").
                arg(viaTransport).arg(viaServer).arg(fromNymId).arg(fromAddress).arg(toNymId).arg(toAddress).arg(subject).arg(body);
    // ----------------------------------------------------
    QString contents;
    if (subject.isEmpty())
        contents = body;
    else
        contents = QString("%1: %2\n\n%3").arg(tr("Subject")).arg(subject).arg(body);
    // ----------------------------------------------------
    if (bCanMessage_)
    {
        const opentxs::Identifier bgthreadId
            {opentxs::OT::App().API().Sync().
                MessageContact(opentxs::Identifier(str_fromNymId), opentxs::Identifier(qstrContactId_.toStdString()), contents.toStdString())};

        const auto status = opentxs::OT::App().API().Sync().Status(bgthreadId);

        const bool bAddToGUI = (opentxs::ThreadStatus::FINISHED_SUCCESS == status) ||
                               (opentxs::ThreadStatus::RUNNING == status);
        if (bAddToGUI) {
            const bool bUseGrayText = (opentxs::ThreadStatus::FINISHED_SUCCESS != status);
            m_bSent = true; // This means it's queued, not actually sent to the notary yet.
        }
    }
    // ----------------------------------------------------
    else if (0 == viaTransport.compare("otserver"))
    {
        std::string strResponse; {
            MTSpinner theSpinner;
            auto action = opentxs::OT::App().API().ServerAction().SendMessage(
            		opentxs::Identifier(str_fromNymId), opentxs::Identifier(str_NotaryID), opentxs::Identifier(str_toNymId), contents.toStdString());
            strResponse = action->Run();
        }

        int32_t  nReturnVal = opentxs::VerifyMessageSuccess(strResponse);
        if (1 != nReturnVal) {
            qDebug() << "OT send_message: Failed.";
            Moneychanger::It()->HasUsageCredits(str_NotaryID, str_fromNymId);
            return false;
        }

        qDebug() << "Success in OT send_message!";
        m_bSent = true; // In this case it means it actually sent it through the notary.
    }
    // ---------------------------------------------------------
    else if (NULL != pModule) // Anything but otserver. (Bitmessage, probably.)
    {
        bool bSuccessSending = false;
        {
            MTSpinner   theSpinner;
            NetworkMail message(str_fromAddr, str_toAddr, subject.toStdString(), body.toStdString());

            bSuccessSending = pModule->sendMail(message);
        }
        if (!bSuccessSending)
        {
            qDebug() << "send_message: Failed.";
            return false;
        }
        qDebug() << "Success in send_message!";
        m_bSent = true; // Sent via whatever the msg method is for this address.
    }
    // ---------------------------------------------------------
    return m_bSent;
}

// Determines if a proper sender and recipient are available based on msgtype.
//
bool MTCompose::hasSenderAndRecipient()
{
    return hasSender() && hasRecipient();
}

bool MTCompose::hasSender()
{
    if (bCanMessage_) {
        return true;
    }
    // If there IS NO message type, then we determine whether or not a sender
    // exists by whether or not the Sender Nym ID is set.
    //
    if (m_msgtype.isEmpty())
    {
        if (!m_senderNymId.isEmpty())
            return true;
    }
    // ------------------------------
    else // msgtype exists...
    {
        // If Message Type is OTSERVER, then all we care about is whether or not
        // the senderNymID is set.
        //
        if (sendingThroughOTServer())
        {
            if (!m_senderNymId.isEmpty())
                return true;
        }
        else // NOT sending through OT server
        {
            // We know, by this point, that there IS a message type, and that
            // it's NOT an OT Server. (Therefore it must be bitmessage or whatever.)
            // Therefore we know there's a sender if: the Sender Method ID
            // OR Address is set. (Either.)
            //
            // (Sender NymID is also mandatory.)
            //
            if (m_senderNymId.isEmpty())
                return false;
            // --------------------------------
            // Notice we allow the sender address to be empty IF the method ID
            // is available. The presumption is that we can find the sender address
            // later, at send time, based on the method ID. If that fails, it will
            // fail then (not now.)
            //
            // NOTE: it's possible to lookup a sender address purely based on sender NymID
            // and message type, and since we're actually doing that now (below) I removed
            // this if block.
//            if ((m_senderAddress.isEmpty()) && (m_senderMethodId <= 0))
//                return false;
            // --------------------------------
            if (m_senderAddress.isEmpty())
            {
                mapIDName theMap;

                QString qstrMsgTypeDisplay = QString::fromStdString(MTComms::displayName(m_msgtype.toStdString()));

                if ((m_senderMethodId > 0) && MTContactHandler::getInstance()->GetAddressesByNym(theMap, m_senderNymId, m_senderMethodId))
                    return this->chooseSenderAddress(theMap, qstrMsgTypeDisplay);
                else if (!m_senderNymId.isEmpty() && MTContactHandler::getInstance()->GetAddressesByNym(theMap, m_senderNymId, m_msgtype))
                    return this->chooseSenderAddress(theMap, qstrMsgTypeDisplay);

                return false;
            }
            // --------------------------------
            // else sender address is NOT empty (so return true.)
            return true;
        }
    }
    // ------------------------------
    return false;
}

bool MTCompose::hasRecipient()
{
    if (bCanMessage_) {
        return true;
    }
    // If there's no contact ID, but there IS a Nym ID, let's go ahead
    // and try to find the contact ID based on the Nym ID, just for shits
    // and giggles. (We may or may not find it.)
    //
    // NOTE: This 'if' may now be redundant since we do this exact thing
    // already in setRecipientNameBasedOnAvailableData().
    //
    if ((m_recipientContactId <= 0) && (!m_recipientNymId.isEmpty()))
        m_recipientContactId = MTContactHandler::getInstance()->FindContactIDByNymID(m_recipientNymId);
    // ---------------------------------------------------------------
    if (m_recipientContactId > 0)
    {
        // Conversely, if there's no NymID but there IS a Contact ID, let's
        // use it to find a NymID. If there's only one, we can select it!
        // (If there is more than one, the user will have to choose, which
        // is beyond the scope of this function.)
        //
        if (m_recipientNymId.isEmpty())
        {
            mapIDName theMap;

            if (MTContactHandler::getInstance()->GetNyms(theMap, m_recipientContactId))
            {
                if (1 == theMap.size())
                {
                    mapIDName::iterator it = theMap.begin();

                    QString qstrNymID       = it.key();
//                  QString qstrNymName     = it.value();

                    this->setInitialRecipient(qstrNymID, m_recipientContactId, m_recipientAddress);
                }
                // else if theMap.size() > 1, the NymID might still go "not set" at this point,
                // since the user would have to choose WHICH one from a chooser dialog.
                // So below this point we can still assume it's possible there is a contact ID,
                // from which a valid NymID could be ascertained, but that the NymID is still not set.
            }
        } // if (m_recipientNymId.isEmpty())
    }
    // ---------------------------------------------------------------
    // If there IS NO message type, then we determine whether or not a recipient
    // exists by whether or not the recipient Nym or Contact ID is set. (Either.)
    //
    if (m_msgtype.isEmpty())
    {
        if ((!m_recipientNymId.isEmpty()) || (m_recipientContactId > 0))
            return true;
    }
    // ------------------------------
    else // There's a message type.
    {
        // If Message Type is OTSERVER, then all we care about is whether or not
        // the recipientNymID is set. HOWEVER, we will accept the contact ID
        // as well, since a Nym can be looked-up later based on the contact ID.
        // So for NOW we'll assume there's a recipient, and if we need to do that
        // lookup later (on send) then we'll do it then.
        //
        if (sendingThroughOTServer())
        {
            if (!m_recipientNymId.isEmpty() || (m_recipientContactId > 0))
                return true;
        }
        else // NOT sending through OT server
        {
            // We know, by this point, that there IS a message type, and that
            // it's NOT an OT Server. (Therefore it must be bitmessage or whatever.)
            // Therefore we know there's a recipient if: the Recipient Contact ID
            // OR Address is set. (Either.) The Recipient NymID is optional in this
            // case.
            //
            // NOTE that it's possible to derive the contact ID from the NymID, but
            // this should already have been done by now (at the top of this very
            // function.) Therefore if there was a contactID to be had from the NymID,
            // we already have it by now.
            //
            // NOTE also that it's possible to derive the address from the contact ID.
            // So IF a contact ID is available, we're willing to let it slide if the
            // address is missing, since we're presuming we'll look up the address at
            // send time.
            //
            if (m_recipientAddress.isEmpty())
            {
                mapIDName theMap;

                QString qstrMsgTypeDisplay = QString::fromStdString(MTComms::displayName(m_msgtype.toStdString()));

                if ((m_recipientContactId > 0) && MTContactHandler::getInstance()->GetAddressesByContact(theMap, m_recipientContactId, m_msgtype))
                    return this->chooseRecipientAddress(theMap, qstrMsgTypeDisplay);
                else if (!m_recipientNymId.isEmpty() && MTContactHandler::getInstance()->GetAddressesByNym(theMap, m_recipientNymId, m_msgtype))
                    return this->chooseRecipientAddress(theMap, qstrMsgTypeDisplay);

                // Why check for a recipient address based on NymID (when normally we use ContactID) ?
                // Because it's possible that the recipient is one of my OWN Nyms.
                // (Maybe I'm sending from one of my Nyms to another.) So it may have
                // a NymID even if there's no Contact ID. Therefore I should also check
                // to see if there are addresses available based on NymID... just in case.
                // Because then "hasRecipient()" would be TRUE!

                return false;
            }
            // --------------------------------
            // else recipient address is NOT empty (so return true.)
            return true;
        }
    }
    // ------------------------------
    return false;
}




void MTCompose::onBalancesChanged()
{
    this->close();
}

/*
 * This button used to be:
 *
 * Server:  Localhost.com
 * (with ID set as "lkjsdfoisd09834oij234jsdf")
 *
 *
 * But now it's like this:
 *
 * Via: OT Server: Localhost.com
 * (with ID set as "otserver|lkjsdfoisd09834oij234jsdf")
 *
 * This is also possible:
 *
 * Via: Bitmessage
 * (with ID set as "bitmessage")
 *
 * Notice if the type is OT server, the server ID is known, and thus if both
 * sender and recipient are known, then Send() will be able to work without
 * any further information.
 *
 * However, if the type is Bitmessage, then we still don't have enough information
 * to send, since we don't know any of the sender's methodIDs for Bitmessage, and
 * neither do we know the recipient's Bitmessage address.
 *
 * If the sender has ONE Bitmessage method, and the recipient has ONE Bitmessage address,
 * then we can easily match them up at Send() time. If either has multiple, then we can ask
 * the user to choose at that time. If either has none, then we can inform the sender and
 * fail, or allow the sender the opportunity to set those values before sending.
 *
 */
void MTCompose::on_viaButton_clicked()
{
    // Select from servers in local wallet filtered by both Nyms.
    //
    DlgChooser theChooser(this);
    mapIDName & the_map = theChooser.m_map;
    // -----------------------------------------------
    MTContactHandler::getInstance()->GetMsgMethodTypes(the_map, true); //bAddServers=false by default
    // -----------------------------------------------
    // Set the pre-selected ID on the chooser, if one is already set.
    //
    int32_t nIndex        = -1;
    int32_t nDefaultIndex =  0;
    bool    bFoundDefault = false;
    QString qstrDefaultType("");
    QString qstrDefaultID  ("");
    QString qstrUseDefault ("");

    for (mapIDName::iterator it = the_map.begin(); it != the_map.end(); ++it)
    {
        ++nIndex;
        // ----------------------------------
        QString qstrType(""); // msgtype
        QString qstrID  (""); // server ID.

        QString qstrKey = it.key();

        if ((-1) != qstrKey.indexOf("|", 0)) // Pipe was found.
        {
            QStringList stringlist = qstrKey.split("|");

            if (stringlist.size() >= 2) // Should always be 2...
            {
                qstrType = stringlist.at(0);
                qstrID   = stringlist.at(1);
            }
        }
        // ----------------------------
        else // Pipe wasn't found.
            qstrType = qstrKey;
        // ----------------------------
        if (!m_NotaryID.isEmpty() && !qstrID.isEmpty() && (0 == qstrID.compare(m_NotaryID)))
        {
            bFoundDefault   = true;
            nDefaultIndex   = nIndex;
            qstrDefaultType = QString("otserver");
            qstrDefaultID   = qstrID;
            qstrUseDefault  = QString("%1|%2").arg(qstrDefaultType).arg(qstrDefaultID);
            break;
        }
        else if (!m_msgtype.isEmpty() && !qstrType.isEmpty() && (0 != m_msgtype.compare("otserver")) && (0 == m_msgtype.compare(qstrType)))
        {
            bFoundDefault   = true;
            nDefaultIndex   = nIndex;
            qstrDefaultType = qstrType;
            qstrUseDefault  = qstrDefaultType;
            break;
        }
    } //  for (find preselected)
    // -----------------------------------------------
    if (bFoundDefault)
        theChooser.SetPreSelected(qstrUseDefault);
    // -----------------------------------------------
    theChooser.setWindowTitle(tr("Select Messaging Type"));
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
        qDebug() << QString("SELECT was clicked for messaging via: %1").arg(theChooser.m_qstrCurrentID);

        if (!theChooser.m_qstrCurrentID.isEmpty())
        {
            QString qstrType(""); // msgtype
            QString qstrID  (""); // server ID.

            if ((-1) != theChooser.m_qstrCurrentID.indexOf("|", 0)) // Pipe was found.
            {
                QStringList stringlist = theChooser.m_qstrCurrentID.split("|");

                if (stringlist.size() >= 2) // Should always be 2...
                {
                    qstrType = stringlist.at(0);
                    qstrID   = stringlist.at(1);
                }
            }
            // ----------------------------
            else // Pipe wasn't found.
                qstrType = theChooser.m_qstrCurrentID;
            // ----------------------------
            QString qstrOldRecipientAddress = m_recipientAddress;
            // ----------------------------
            this->setInitialMsgType(qstrType, qstrID);
            // ----------------------------
            // Since the message type just changed, whether to OT server
            // or to Bitmessage, either way any previous address that was
            // set needs to be cleared.
            //
            if (!qstrType.isEmpty()) // qstrType not empty (should always be true.)
            {
                if (!m_senderAddress.isEmpty())
                {
                    QString senderMethodType = MTContactHandler::getInstance()->GetMethodType(m_senderAddress);

                    if (senderMethodType.isEmpty() || (0 != senderMethodType.compare(qstrType)) || (0 == senderMethodType.compare("otserver")))
                        this->setInitialSenderAddress(QString(""));
                }

                qDebug() << QString(" m_recipientAddress: %1\n m_recipientNymId: %2\n m_recipientContactId: %3\n")
                            .arg(m_recipientAddress)
                            .arg(m_recipientNymId)
                            .arg(m_recipientContactId);

                if (!qstrOldRecipientAddress.isEmpty())
                {
                    QString recipientMethodType = MTContactHandler::getInstance()->GetMethodType(qstrOldRecipientAddress);

                    if (recipientMethodType.isEmpty() || (0 != recipientMethodType.compare(qstrType)) || (0 == recipientMethodType.compare("otserver")))
                    {
                        // Since we above set the address to blank, let's consider that there MIGHT
                        // be a blank recipient Nym ID (like if the recipient has only an address,
                        // like from a Bitmessage, but no Nym ID set, we might still be able to
                        // derive a NymID before we wipe the address clean. We might also instead
                        // be able to derive a contactID before we wipe the address clean.
                        //
                        bool bAlreadySet = false;

                        if (m_recipientNymId.isEmpty())
                        {
                            QString qstrNymByAddress = MTContactHandler::getInstance()->GetNymByAddress(qstrOldRecipientAddress);

                            if (!qstrNymByAddress.isEmpty())
                            {
                                bAlreadySet = true;
                                setInitialRecipientNym(qstrNymByAddress, QString(""));
                            }
                        }
                        // -------------------------------
                        if (m_recipientNymId.isEmpty())
                        {
                            int nContactIDByAddress = m_recipientContactId;

                            if (0 == nContactIDByAddress)
                            {
                                nContactIDByAddress = MTContactHandler::getInstance()->GetContactByAddress(qstrOldRecipientAddress);

                                if (0 != nContactIDByAddress)
                                {
                                    bAlreadySet = true;
                                    setInitialRecipientContactID(nContactIDByAddress, QString(""));
                                }
                            }
                        }
                        // --------------------------------------------
                        if (!bAlreadySet)
                            this->setInitialRecipientAddress(QString(""));
                    }
                }
            }
            // else {} (should never happen.)
            // --------------------------------------
            // These calls will select the right addresses, if appropriate.
            this->hasSender();
            this->hasRecipient();
            // --------------------------------------
            this->setSenderNameBasedOnAvailableData();
            this->setRecipientNameBasedOnAvailableData();
            this->setTransportDisplayBasedOnAvailableData();
        }
    }
    else
    {
//      qDebug() << "CANCEL was clicked";
    }
}

// --------------------------------------------

// Assumes method is NOT "otserver". Used for all other types.
// This method is called only when m_msgtype is EMPTY.
// (Meaning it's called in the event that msgtype is unknown and must be "figured out.")
//
bool MTCompose::CheckPotentialCommonMsgMethod(QString qstrMsgTypeAttempt, mapIDName * pmapSenderAddresses/*=NULL*/, mapIDName * pmapRecipientAddresses/*=NULL*/)
{
    // Let's see if they both have qstrMsgTypeAttempt (say, "bitmessage".)
    //
    bool bMsgTypeFoundOnSender        = MTContactHandler::getInstance()->MethodTypeFoundOnNym(qstrMsgTypeAttempt, m_senderNymId);
    bool bMsgTypeFoundOnRecipient     = false;

    bool bMsgTypeFoundOnRecipContact  = false; // In case the "recipient" is a contact.
    bool bMsgTypeFoundOnRecipNym      = m_recipientNymId.isEmpty() ? false : // "recipient" could be one of MY OWN Nyms.
                                        MTContactHandler::getInstance()->MethodTypeFoundOnNym(qstrMsgTypeAttempt, m_recipientNymId);

    if (!bMsgTypeFoundOnRecipNym)
        bMsgTypeFoundOnRecipContact   = (m_recipientContactId <= 0) ? false :
                                        MTContactHandler::getInstance()->MethodTypeFoundOnContact(qstrMsgTypeAttempt, m_recipientContactId);
    // ------------------------------------------------------
    bMsgTypeFoundOnRecipient = (bMsgTypeFoundOnRecipNym || bMsgTypeFoundOnRecipContact);
    // ------------------------------------------------------
    // NOTE: at this point they both have Bitmessage (say), but we still need to get the
    // Bitmessage addresses set for both of them (if not already set.)
    //
    if (bMsgTypeFoundOnSender && bMsgTypeFoundOnRecipient)
    {
        mapIDName   mapActualSenderAddresses, mapActualRecipientAddresses;
        mapIDName & mapSenderAddresses    = (NULL == pmapSenderAddresses   ) ? mapActualSenderAddresses    : *pmapSenderAddresses;
        mapIDName & mapRecipientAddresses = (NULL == pmapRecipientAddresses) ? mapActualRecipientAddresses : *pmapRecipientAddresses;

        bool bGotSenderAddresses             = MTContactHandler::getInstance()->GetAddressesByNym(mapSenderAddresses, m_senderNymId, qstrMsgTypeAttempt);
        bool bGotRecipientAddresses          = false;
        bool bGotRecipientAddressesByNym     = false;
        bool bGotRecipientAddressesByContact = false;

        if (!m_recipientNymId.isEmpty())
            bGotRecipientAddressesByNym      = MTContactHandler::getInstance()->GetAddressesByNym(mapRecipientAddresses, m_recipientNymId, qstrMsgTypeAttempt);

        if (!bGotRecipientAddressesByNym && (m_recipientContactId > 0))
            bGotRecipientAddressesByContact  = MTContactHandler::getInstance()->GetAddressesByContact(mapRecipientAddresses, m_recipientContactId, qstrMsgTypeAttempt);

        bGotRecipientAddresses = (bGotRecipientAddressesByNym || bGotRecipientAddressesByContact);

        // NOTE: Since we are filtering by type, the ID in the map will not contain "type|blah" but rather, only "blah".
        // And what is blah? It's the address, such as the Bitmessage address.
        // ------------------------------------------------------
        // Sender (or Recipient) has no transport methods of qstrMsgTypeAttempt? Failure.
        //
        if (!bGotSenderAddresses || !bGotRecipientAddresses)
            return false;

        // By this point we know that addresses of type qstrMsgTypeAttempt (such
        // as "bitmessage") are available both for sender AND recipient.
        // ------------------------------------------------------
        return true;
    }
    return false;
}

bool MTCompose::sendingThroughOTServer()
{
    return (!m_msgtype.isEmpty() && (0 == m_msgtype.compare(QString("otserver"))));
}

void MTCompose::on_sendButton_clicked()
{
    // Send message and then close dialog. Use progress bar.
    // -----------------------------------------------------------------
    if (!hasSender())
    {
        QMessageBox::warning(this, tr("Message Has No Sender"),
                             tr("Please choose a sender for this message, before sending."));
        return;
    }
    // -----------------------------------------------------------------
    if (!hasRecipient())
    {
        QMessageBox::warning(this, tr("Message Has No Recipient"),
                             tr("Please choose a recipient for this message, before sending."));
        return;
    }
    // -----------------------------------------------------------------
    // What we don't know, yet, is if the type is "bitmessage" (or anything else), or whether
    // there are addresses available for sender and recipient, and whether they are appropriate
    // based on the msgtype.
    // We also don't know for sure, even if it is "otserver", whether the recipient Nym is actually set.
    // up until this point, a Contact ID is enough to get us here even without the NymID, since the NymID
    // can be resolved based on the Contact ID. But beyond this point, we ACTUALLY need the NymID. So
    // MakeSureCommonMsgMethod() will look that up, if we don't already have it.
    //
    // The below call is what insures all the addresses are in order.
    // When necessary, it will pop up a chooser dialog, or an "Are you sure?"
    // dialog, in order to gather the appropriate information.
    // If successful setting everything such that we could actually
    // send, it returns true. But if this function finishes in a state
    // where we do not have the necessary info to send, it will return false.
    //
    if (!MakeSureCommonMsgMethod())
        return;
    // -----------------------------------------------------------------
    if (ui->subjectEdit->isVisible() && ui->subjectEdit->text().isEmpty())
    {
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, "", tr("This message has a blank subject. Are you sure you want to send?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No)
          return;
    }
    // -----------------------------------------------------------------
    if (ui->contentsEdit->toPlainText().isEmpty())
    {
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, "", tr("The message contents are blank. Are you sure you want to send?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No)
          return;
    }
    // -----------------------------------------------------------------
    // Actually send the message here.
    //
    QString subject      = ui->subjectEdit->text();
    QString body         = ui->contentsEdit->toPlainText();
    QString fromNymId    = m_senderNymId;
    QString toNymId      = m_recipientNymId;
    QString fromAddress  = m_senderAddress;
    QString toAddress    = m_recipientAddress;
    QString viaServer    = m_NotaryID;
    QString viaTransport = m_msgtype;
    int     viaMethodID  = m_senderMethodId;

    bool bSent = this->sendMessage(subject, body, fromNymId, toNymId, fromAddress, toAddress, viaServer, viaTransport, viaMethodID);
    // -----------------------------------------------------------------
    if (!bSent)
        QMessageBox::warning(this, tr("Failed Sending Message"),
                             tr("Failed trying to send the message."));
    else
    {
        QMessageBox::StandardButton info_btn =
                QMessageBox::information(this, tr("Success"), tr("Success sending message."));
        // -----------------------------------------------------------------
        emit balancesChanged(); // So we'll see the sent message in the payment history.
        // -----------------------------------------------------------------
    }
    // -----------------------------------------------------------------
}





bool MTCompose::chooseSenderMethodID(mapIDName & theMap, QString qstrMsgTypeDisplay)
{
    if (1 == theMap.size())
    {
        mapIDName::iterator it = theMap.begin();
        m_senderMethodId = it.key().toInt();
        return true;
    }
    else if (theMap.size() > 1)
    {
        // There are multiple matching method types for the sender.
        // (Have the user choose one of them.)
        //
        DlgChooser theChooser(this);
        // -----------------------------------------------
        theChooser.m_map = theMap;
        // -----------------------------------------------
        QString qstrWindowTitle = QString("%1 %2 %3.").arg(tr("Multiple")).arg(qstrMsgTypeDisplay).arg(tr("transports are available"));
        theChooser.setWindowTitle(qstrWindowTitle);
        // -----------------------------------------------
        if (theChooser.exec() == QDialog::Accepted)
        {
            if (!theChooser.m_qstrCurrentID.isEmpty())
            {
                m_senderMethodId = theChooser.m_qstrCurrentID.toInt();
                return true;
            }
        }
    }
    // -----------------
    return false;
}



bool MTCompose::chooseSenderAddress(mapIDName & mapSenderAddresses, QString qstrMsgTypeDisplay, bool bForce/*=false*/)
{
    if (bForce || (1 == mapSenderAddresses.size()))
    {
        if (mapSenderAddresses.size() >= 1)
        {
            mapIDName::iterator it = mapSenderAddresses.begin();
            this->setInitialSenderAddress(it.key());
            m_senderMethodId = MTContactHandler::getInstance()->GetMethodIDByNymAndAddress(m_senderNymId, m_senderAddress);
            return true;
        }
    }
    else if (mapSenderAddresses.size() > 1)
    {
        // There are multiple matching addresses for the sender.
        // (Have the user choose one of them.)
        //
        DlgChooser theChooser(this);
        // -----------------------------------------------
        theChooser.m_map = mapSenderAddresses;
        // -----------------------------------------------
        QString qstrWindowTitle = QString("%1 %2 %3.").arg(tr("Sender has multiple")).arg(qstrMsgTypeDisplay).arg(tr("addresses"));
        theChooser.setWindowTitle(qstrWindowTitle);
        // -----------------------------------------------
        if (theChooser.exec() == QDialog::Accepted)
        {
            if (!theChooser.m_qstrCurrentID.isEmpty())
            {
                this->setInitialSenderAddress(theChooser.m_qstrCurrentID);
                m_senderMethodId = MTContactHandler::getInstance()->GetMethodIDByNymAndAddress(m_senderNymId, m_senderAddress);
                return true;
            }
        }
    }
    // -----------------
    return false;
}

bool MTCompose::chooseRecipientAddress(mapIDName & mapRecipientAddresses, QString qstrMsgTypeDisplay)
{
    if (1 == mapRecipientAddresses.size())
    {
        mapIDName::iterator it = mapRecipientAddresses.begin();
        this->setInitialRecipientAddress(it.key());
        return true;
    }
    else if (mapRecipientAddresses.size() > 1)
    {
        // There are multiple matching addresses for the recipient.
        // (Have the user choose one of them.)
        //
        DlgChooser theChooser(this);
        // -----------------------------------------------
        theChooser.m_map = mapRecipientAddresses;
        // -----------------------------------------------
        QString qstrWindowTitle = QString("%1 %2 %3.").arg(tr("Recipient has multiple")).arg(qstrMsgTypeDisplay).arg(tr("addresses"));
        theChooser.setWindowTitle(qstrWindowTitle);
        // -----------------------------------------------
        if (theChooser.exec() == QDialog::Accepted)
        {
            if (!theChooser.m_qstrCurrentID.isEmpty())
            {
                this->setInitialRecipientAddress(theChooser.m_qstrCurrentID);
                return true;
            }
        }
    }
    // --------------------
    return false;
}


bool MTCompose::chooseServer(mapIDName & theMap)
{
    if (1 == theMap.size())
    {
        mapIDName::iterator it = theMap.begin();
        this->setInitialServer(it.key());
        return true;
    }
    else if (theMap.size() > 1)
    {
        // There are multiple matching servers.
        // (Have the user choose one of them.)
        //
        DlgChooser theChooser(this);
        // -----------------------------------------------
        theChooser.m_map = theMap;
        // -----------------------------------------------
        QString qstrWindowTitle = QString(tr("Choose Server"));
        theChooser.setWindowTitle(qstrWindowTitle);
        // -----------------------------------------------
        if (theChooser.exec() == QDialog::Accepted)
        {
            if (!theChooser.m_qstrCurrentID.isEmpty())
            {
                this->setInitialServer(theChooser.m_qstrCurrentID);
                return true;
            }
        }
    }
    // --------------------
    return false;
}




// Recipient has just changed. Does Sender exist? If so, make sure he is compatible with msgtype or find a new one that matches both.
void MTCompose::FindSenderMsgMethod()
{
    if (bCanMessage_) {
        return;
    }
    // If sender doesn't exist, we can just return now.
    // BUT the sender could be m_senderNymId, m_senderAddress,
    // or some combination. Let's find what we can...
    //
    if (m_senderNymId.isEmpty() && !m_senderAddress.isEmpty())
    {
        QString qstrSenderNym = MTContactHandler::getInstance()->GetNymByAddress(m_senderAddress);

        if (!qstrSenderNym.isEmpty())
            this->setInitialSenderNym(qstrSenderNym, m_senderAddress);
    }
    // -------------------------------------------------
    // We use the recipient's NymID in this function, so we might as well make sure we
    // have all that info ready.
    //
    bool bUpdatedRecipientDetails = false;

    if (m_recipientContactId <= 0)
    {
        if (!m_recipientNymId.isEmpty())
        {
            m_recipientContactId = MTContactHandler::getInstance()->FindContactIDByNymID(m_recipientNymId);
            bUpdatedRecipientDetails = true;
        }
        // -------------------------------------------------
        if ((m_recipientContactId <= 0) && !m_recipientAddress.isEmpty())
        {
            m_recipientContactId = MTContactHandler::getInstance()->GetContactByAddress(m_recipientAddress);
            bUpdatedRecipientDetails = true;
        }
    }
    // -------------------------------------------------
    if (m_recipientNymId.isEmpty())
    {
        if (!m_recipientAddress.isEmpty())
        {
            m_recipientNymId = MTContactHandler::getInstance()->GetNymByAddress(m_recipientAddress);
            bUpdatedRecipientDetails = true;
        }
        // -------------------------------------------------
        else
        {
            mapIDName mapNyms;

            if ((m_recipientContactId > 0) && MTContactHandler::getInstance()->GetNyms(mapNyms, m_recipientContactId))
            {
                if (mapNyms.size() > 0)
                {
                    mapIDName::iterator it = mapNyms.begin();
                    m_recipientNymId = it.key();
                    bUpdatedRecipientDetails = true;
                }
            }
        }
    }
    // -------------------------------------------------
    if (bUpdatedRecipientDetails)
        this->setRecipientNameBasedOnAvailableData();
    // -------------------------------------------------
    if (!m_NotaryID.isEmpty() && m_msgtype.isEmpty())
        setInitialMsgType("otserver");
    // -------------------------------------------------
    if (sendingThroughOTServer())
    {
        if (!m_senderNymId.isEmpty())
        {
            if (!m_recipientNymId.isEmpty()) // we already know this or we wouldn't have even called this function. (Still, makes me feel safe.)
            {
                // If we already have a server ID, let's see if they both support it.
                // If we don't already have a server ID, let's find one they both agree on.

                if (!m_NotaryID.isEmpty()) // server ID not empty. Let's see if they both support it.
                {
                    if (verifySenderAgainstServer() && verifyRecipientAgainstServer())
                        return;  // SUCCESS!
                    // else if they DON'T both support it, therefore we should find one they DO support.
                    // Note, this is definite, since it actually checks each nym's registration against
                    // the server itself. There's no chance it's wrong. Therefore we can definitely
                    // dump this server and try to find another one.
                    else
                    {
                        m_NotaryID = QString("");  // We force this here, since setInitialServer won't normally blank out a NotaryID even if you pass a blank one.
                        setInitialServer(QString(""));
                    }
                }
                //else Server ID empty. Let's find one they both agree on.

                // Basically we just grab the servers the recipient is known to frequent,
                // and then loop through them and see if the sender is registered on any of them.
                //
                bool      bGotServers = false;
                mapIDName mapServers;

                if (m_recipientContactId > 0)
                    bGotServers = MTContactHandler::getInstance()->GetServers(mapServers, m_recipientContactId);

                if (!bGotServers && !m_recipientNymId.isEmpty())
                    bGotServers = MTContactHandler::getInstance()->GetServers(mapServers, m_recipientNymId);

                if (bGotServers)
                {
                    for (mapIDName::iterator it = mapServers.begin(); it != mapServers.end(); ++it)
                    {
                        QString qstrNotaryID = it.key();

                        std::string notary_id    = qstrNotaryID.toStdString();
                        std::string sender_id    = m_senderNymId.toStdString();

                        if (opentxs::OT::App().API().Exec().IsNym_RegisteredAtServer(sender_id, notary_id))
                        {
                            setInitialServer(qstrNotaryID);
                            return; // SUCCESS!
                        }
                    }
                } // get servers
            } // recipient nym ID not empty.
        } // sender nym ID not empty.
        // -------------------------------
        else // sender nym ID is empty.
        {
            // If sending through OT, then we MUST have a Sender Nym by this point.
            // If we don't, then we already KNOW we couldn't find one based on the address.
            // (If it even existed.) But let's say it DOES exist. If the address exists, then it
            // might be possible to find a recipient address for the same msgtype, and then just change
            // the msgtype to match them both.
            //
            // NOTE: This means we've given up on finding a matching OT server, and are pursuing
            // other methods at this point.
            // -------------------------------------------------------
            // Fall through...
        }
    } // if sending through OT server
    // --------------------------------
    //else Sending through other methods. (Bitmessage probably.)

    QString qstrMethodTypeSender("");
    QString qstrMethodTypeRecipient("");

    // If the sender address exists, then it might be possible to find a recipient address
    // for the same msgtype, and then just change the msgtype to match them both.

    if (!m_senderAddress.isEmpty())
        qstrMethodTypeSender = MTContactHandler::getInstance()->GetMethodType(m_senderAddress);
    if (qstrMethodTypeSender.isEmpty() && (m_senderMethodId > 0))
        qstrMethodTypeSender = MTContactHandler::getInstance()->GetMethodType(m_senderMethodId);
    // ---------------------------------
    if (!m_recipientAddress.isEmpty())
        qstrMethodTypeRecipient = MTContactHandler::getInstance()->GetMethodType(m_recipientAddress);
    // ---------------------------------



//    qDebug() << QString("m_senderAddress: %1").arg(m_senderAddress);
//    qDebug() << QString("m_recipientAddress: %1").arg(m_recipientAddress);
//    qDebug() << QString("qstrMethodTypeSender: %1").arg(qstrMethodTypeSender);
//    qDebug() << QString("qstrMethodTypeRecipient: %1").arg(qstrMethodTypeRecipient);


    // If the recipient and sender both have addresses, see if they are of a matching type
    // and if so, just go with that.
    if (!qstrMethodTypeSender     .isEmpty() &&
        !qstrMethodTypeRecipient  .isEmpty() &&
        (0 == qstrMethodTypeSender.compare(qstrMethodTypeRecipient)))
    {
        setInitialMsgType(qstrMethodTypeSender);
        return; // SUCCESS!
    }
    // --------------------------------
    // If the recipient address exists, then we know the sender address either doesn't exist,
    // or doesn't match its type. Let's see if we can find a better sender address that
    // DOES match its type.
    //
    if (!qstrMethodTypeRecipient.isEmpty())
    {
        mapIDName mapSenderAddresses;
        bool bGotAddresses = false;

        if (!m_senderNymId.isEmpty() &&
            MTContactHandler::getInstance()->GetAddressesByNym(mapSenderAddresses, m_senderNymId, qstrMethodTypeRecipient))
            bGotAddresses = true;

        if (bGotAddresses)
        {
            mapIDName::iterator it = mapSenderAddresses.begin();

            // We just automatically choose the first one in the list.
            // In other words, when the user selects a recipient, this function gets called, in order
            // to adjust the sender (if there is one) so the message will go to it, by setting
            // the sender's address to one that matches the type of the recipient's address.
            // So if we are here, and we found a list of matching addresses, we just pick the first
            // one. We don't ask the user to choose from the list, because he just clicked on RECIPIENT,
            // and it would confuse him to make him choose from the SENDER list. So instead, we
            // just pick the very first sender address that will work with the recipient he chose.
            // (If the user wants to use a different address, he can just click SENDER and do so.)
            //
            if (mapSenderAddresses.end() != it)
            {
                setInitialMsgType(qstrMethodTypeRecipient);
                setInitialSenderNym(m_senderNymId, it.key());
                return; // SUCCESS!
            }
        }
    }

    // If the sender address exists, then we know the recipient address either doesn't exist,
    // or doesn't match it. However, the only reason we're here is because the user just CHOSE
    // the recipient (that's why we're now in a function to try to make the sender match to him.)
    // So if we changed the recipient, we can't just change him again, because maybe the user was
    // about to change the sender, too! We can't have the recipient keep switching back, right
    // when the user has clicked on it!
    //
    // So here's what we'll do. We'll try to find a common msg method between the sender and
    // recipient. And IF WE FIND ONE, we'll THEN ask the user if he wants us to automatically
    // coordinate the transport type, giving him a simple yes/no option.
    //
    mapOfCommTypes mapTypes;
    bool bFoundOneInCommon = false;
    bool bGotCommTypes     = MTComms::types(mapTypes);

    // Here we're looping through the list of transport types OTHER than "otserver",
    // such as "bitmessage". See if we can find one that both parties have in common.
    //
    if (bGotCommTypes)
    {
        QString qstrMsgTypeAttempt, qstrMsgTypeDisplay;
        mapIDName mapSenderAddresses, mapRecipientAddresses;

        for (mapOfCommTypes::iterator it = mapTypes.begin(); it != mapTypes.end(); ++it)
        {
            qstrMsgTypeAttempt = QString("");
            qstrMsgTypeDisplay = QString("");

            std::string strTypeName    = it->first;
            std::string strTypeDisplay = it->second;

            if (!strTypeName.empty() && !strTypeDisplay.empty())
            {
                qstrMsgTypeAttempt = QString::fromStdString(strTypeName);
                qstrMsgTypeDisplay = QString::fromStdString(strTypeDisplay);

                mapSenderAddresses   .clear();
                mapRecipientAddresses.clear();

                if (this->CheckPotentialCommonMsgMethod(qstrMsgTypeAttempt, &mapSenderAddresses, &mapRecipientAddresses))
                {
                    bFoundOneInCommon = true;
                    break;
                }
            }
        } // for
        // -------------------------------
        // Did we find one?
        //
        if (bFoundOneInCommon)
        {
            QMessageBox::StandardButton reply;

            reply = QMessageBox::question(this, "",
                                          tr("Recipient and Sender have different transport types. Shall I auto-match? (You probably want to choose Yes.)"),
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes)
            {
                // Okay then, let's set the msgtype to qstrMsgTypeAttempt, and we'll set the sender and
                // recipient addresses. (And sender method ID.) If there's only one address to choose from
                // for either, we can just go with it. But if there are more than one, we must ask the user
                // to choose which he prefers.
                //
                if (!this->chooseSenderAddress(mapSenderAddresses, qstrMsgTypeDisplay))
                    return; // failure

                if (!this->chooseRecipientAddress(mapRecipientAddresses, qstrMsgTypeDisplay))
                    return; // failure.
                // -------------------------------------------
                // If both addresses were selected (we got this far, didn't we?)
                // then we go ahead and set the msgtype.
                //
                this->setInitialMsgType(qstrMsgTypeAttempt); // (server="" by default.)
                return; // SUCCESS!
            }
        } // if (bFoundOneInCommon)
    } // if (bGotCommTypes)
    // ----------------------
    // By this point, oh well. We gave it the old college try.
}


// Sender has just changed. Does Recipient exist? If so, make sure he is compatible with msgtype or find a new one that matches both.
void MTCompose::FindRecipientMsgMethod()
{
    if (bCanMessage_) {
        return;
    }
    // We use the sender's ID in this function, so let's make sure it's ready...
    //
    if (m_senderNymId.isEmpty() && !m_senderAddress.isEmpty())
    {
        QString qstrSenderNym = MTContactHandler::getInstance()->GetNymByAddress(m_senderAddress);

        if (!qstrSenderNym.isEmpty())
            this->setInitialSenderNym(qstrSenderNym, m_senderAddress);
    }
    // -------------------------------------------------
    // If recipient doesn't exist, we can just return now.
    // BUT the recipient could be m_recipientContactId, m_recipientNymId,
    // m_recipientAddress, or some combination. Let's find what we can...
    //
    bool bUpdatedRecipientDetails = false;

    if (m_recipientContactId <= 0)
    {
        if (!m_recipientNymId.isEmpty())
        {
            m_recipientContactId = MTContactHandler::getInstance()->FindContactIDByNymID(m_recipientNymId);
            bUpdatedRecipientDetails = true;
        }
        // -------------------------------------------------
        if ((m_recipientContactId <= 0) && !m_recipientAddress.isEmpty())
        {
            m_recipientContactId = MTContactHandler::getInstance()->GetContactByAddress(m_recipientAddress);
            bUpdatedRecipientDetails = true;
        }
    }
    // -------------------------------------------------
    if (m_recipientNymId.isEmpty())
    {
        if (!m_recipientAddress.isEmpty())
        {
            m_recipientNymId = MTContactHandler::getInstance()->GetNymByAddress(m_recipientAddress);
            bUpdatedRecipientDetails = true;
        }
        // -------------------------------------------------
        else
        {
            mapIDName mapNyms;

            if ((m_recipientContactId > 0) && MTContactHandler::getInstance()->GetNyms(mapNyms, m_recipientContactId))
            {
                if (mapNyms.size() > 0)
                {
                    mapIDName::iterator it = mapNyms.begin();
                    m_recipientNymId = it.key();
                    bUpdatedRecipientDetails = true;
                }
            }
        }
    }
    // -------------------------------------------------
    if (bUpdatedRecipientDetails)
        this->setRecipientNameBasedOnAvailableData();
    // -------------------------------------------------
    if (!m_NotaryID.isEmpty() && m_msgtype.isEmpty())
        setInitialMsgType("otserver");
    // -------------------------------------------------
    if (sendingThroughOTServer())
    {
        if (!m_recipientNymId.isEmpty())
        {
            if (!m_senderNymId.isEmpty()) // we already know this or we wouldn't have even called this function. (Still, makes me feel safe.)
            {
                // If we already have a server ID, let's see if they both support it.
                // If we don't already have a server ID, let's find one they both agree on.

                if (!m_NotaryID.isEmpty()) // server ID not empty. Let's see if they both support it.
                {
                    if (verifySenderAgainstServer() && verifyRecipientAgainstServer())
                        return;  // SUCCESS!
                    // else if they DON'T both support it, therefore we should find one they DO support.
                    // Note, this is definite, since it actually checks each nym's registration against
                    // the server itself. There's no chance it's wrong. Therefore we can definitely
                    // dump this server and try to find another one.
                    else
                    {
                        m_NotaryID = QString("");  // We force this here, since setInitialServer won't normally blank out a NotaryID even if you pass a blank one.
                        setInitialServer(QString(""));
                    }
                }
                //else Server ID empty. Let's find one they both agree on.

                // Basically we just grab the servers the recipient is known to frequent,
                // and then loop through them and see if the sender is registered on any of them.
                //
                bool      bGotServers = false;
                mapIDName mapServers;

                if (m_recipientContactId > 0)
                    bGotServers = MTContactHandler::getInstance()->GetServers(mapServers, m_recipientContactId);

                if (!bGotServers && !m_recipientNymId.isEmpty())
                    bGotServers = MTContactHandler::getInstance()->GetServers(mapServers, m_recipientNymId);

                if (bGotServers)
                {
                    for (mapIDName::iterator it = mapServers.begin(); it != mapServers.end(); ++it)
                    {
                        QString qstrNotaryID = it.key();

                        std::string notary_id    = qstrNotaryID.toStdString();
                        std::string sender_id    = m_senderNymId.toStdString();

                        if (opentxs::OT::App().API().Exec().IsNym_RegisteredAtServer(sender_id, notary_id))
                        {
                            setInitialServer(qstrNotaryID);
                            return; // SUCCESS!
                        }
                    }
                } // get servers
            } // sender nym ID not empty.
        } // recipient nym ID not empty.
        // -------------------------------
        else // recipient nym ID is empty.
        {
            // If sending through OT, then we MUST have a Recipient Nym by this point.
            // If we don't, then we already KNOW we couldn't find one based on the contact or address.
            // (If those even existed.) But let's say they DO exist. If the address exists, then it
            // might be possible to find a sender address for the same msgtype, and then just change
            // the msgtype to match them both. Similarly, if the contact exists, then it might be
            // possible to find a msgtype in common between the sender nym and the contact.
            //
            // NOTE: This means we've given up on finding a matching OT server, and are pursuing
            // other methods at this point.
            // -------------------------------------------------------
            // Fall through...
        }
    } // if sending through OT server
    // --------------------------------
    //else Sending through other methods. (Bitmessage probably.)

    QString qstrMethodTypeSender("");
    QString qstrMethodTypeRecipient("");

    // If the recipient address exists, then it might be possible to find a sender address
    // for the same msgtype, and then just change the msgtype to match them both.

    if (!m_senderAddress.isEmpty())
        qstrMethodTypeSender = MTContactHandler::getInstance()->GetMethodType(m_senderAddress);
    if (qstrMethodTypeSender.isEmpty() && (m_senderMethodId > 0))
        qstrMethodTypeSender = MTContactHandler::getInstance()->GetMethodType(m_senderMethodId);
    // ---------------------------------
    if (!m_recipientAddress.isEmpty())
        qstrMethodTypeRecipient = MTContactHandler::getInstance()->GetMethodType(m_recipientAddress);
    // ---------------------------------
    // If the recipient and sender both have addresses, see if they are of a matching type
    // and if so, just go with that.
    if (!qstrMethodTypeSender     .isEmpty() &&
        !qstrMethodTypeRecipient  .isEmpty() &&
        (0 == qstrMethodTypeSender.compare(qstrMethodTypeRecipient)))
    {
        setInitialMsgType(qstrMethodTypeSender);
        return; // SUCCESS!
    }
    // --------------------------------
    // If the sender address exists, then we know the recipient address either doesn't exist,
    // or doesn't match its type. Let's see if we can find a better recipient address that
    // DOES match its type.
    //
    if (!qstrMethodTypeSender.isEmpty())
    {
        if (!m_senderNymId.isEmpty()) // I already know this is the case, or this function wouldn't have even been called in the first place.
            m_senderMethodId = MTContactHandler::getInstance()->GetMethodIDByNymAndAddress(m_senderNymId, m_senderAddress);
        // -----------------------------
        mapIDName mapRecipAddresses;
        bool bGotAddresses = false;

        if ((m_recipientContactId > 0) && MTContactHandler::getInstance()->GetAddressesByContact(mapRecipAddresses, m_recipientContactId, qstrMethodTypeSender))
            bGotAddresses = true;

        if (!bGotAddresses && !m_recipientNymId.isEmpty() &&
            MTContactHandler::getInstance()->GetAddressesByNym(mapRecipAddresses, m_recipientNymId, qstrMethodTypeSender))
            bGotAddresses = true;

        if (bGotAddresses)
        {
            mapIDName::iterator it = mapRecipAddresses.begin();

            // We just automatically choose the first one in the list.
            // In other words, when the user selects a sender, this function gets called, in order
            // to adjust the recipient (if there is one) so the message will go to it, by setting
            // the recipient's address to one that matches the type of the sender's address.
            // So if we are here, and we found a list of matching addresses, we just pick the first
            // one. We don't ask the user to choose from the list, because he just clicked on SENDER,
            // and it would confuse him to make him choose from the RECIPIENT list. So instead, we
            // just pick the very first recipient address that will work with the sender he chose.
            // (If the user wants to use a different address, he can just click RECIPIENT and do so.)
            //
            if (mapRecipAddresses.end() != it)
            {
                setInitialMsgType(qstrMethodTypeSender);
                setInitialRecipient(m_recipientNymId, m_recipientContactId, it.key());
                return; // SUCCESS!
            }
        }
    }

    // If the recipient address exists, then we know the sender address either doesn't exist,
    // or doesn't match it. However, the only reason we're here is because the user just CHOSE
    // the sender (that's why we're now in a function to try to make the recipient match to him.)
    // So if we changed the sender, we can't just change him again, because maybe the user was
    // about to change the recipient, too! We can't have the sender keep switching back, right
    // when the user has clicked on it!
    //
    // So here's what we'll do. We'll try to find a common msg method between the sender and
    // recipient. And IF WE FIND ONE, we'll THEN ask the user if he wants us to automatically
    // coordinate the transport type, giving him a simple yes/no option.
    //
    mapOfCommTypes mapTypes;
    bool bFoundOneInCommon = false;
    bool bGotCommTypes     = MTComms::types(mapTypes);

    // Here we're looping through the list of transport types OTHER than "otserver",
    // such as "bitmessage". See if we can find one that both parties have in common.
    //
    if (bGotCommTypes)
    {
        QString qstrMsgTypeAttempt, qstrMsgTypeDisplay;
        mapIDName mapSenderAddresses, mapRecipientAddresses;

        for (mapOfCommTypes::iterator it = mapTypes.begin(); it != mapTypes.end(); ++it)
        {
            qstrMsgTypeAttempt = QString("");
            qstrMsgTypeDisplay = QString("");

            std::string strTypeName    = it->first;
            std::string strTypeDisplay = it->second;

            if (!strTypeName.empty() && !strTypeDisplay.empty())
            {
                qstrMsgTypeAttempt = QString::fromStdString(strTypeName);
                qstrMsgTypeDisplay = QString::fromStdString(strTypeDisplay);

                mapSenderAddresses   .clear();
                mapRecipientAddresses.clear();

                if (this->CheckPotentialCommonMsgMethod(qstrMsgTypeAttempt, &mapSenderAddresses, &mapRecipientAddresses))
                {
                    bFoundOneInCommon = true;
                    break;
                }
            }
        } // for
        // -------------------------------
        // Did we find one?
        //
        if (bFoundOneInCommon)
        {
            QMessageBox::StandardButton reply;

            reply = QMessageBox::question(this, "",
                                          tr("Sender and Recipient have different transport types. Shall I auto-match? (You probably want to choose Yes.)"),
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes)
            {
                // Okay then, let's set the msgtype to qstrMsgTypeAttempt, and we'll set the sender and
                // recipient addresses. (And sender method ID.) If there's only one address to choose from
                // for either, we can just go with it. But if there are more than one, we must ask the user
                // to choose which he prefers.
                //
                if (!this->chooseSenderAddress(mapSenderAddresses, qstrMsgTypeDisplay))
                    return; // failure

                if (!this->chooseRecipientAddress(mapRecipientAddresses, qstrMsgTypeDisplay))
                    return; // failure.
                // -------------------------------------------
                // If both addresses were selected (we got this far, didn't we?)
                // then we go ahead and set the msgtype.
                //
                this->setInitialMsgType(qstrMsgTypeAttempt); // (server="" by default.)
                return; // SUCCESS!
            }

        } // if (bFoundOneInCommon)
    } // if (bGotCommTypes)
    // ----------------------
    // By this point, oh well. We gave it the old college try.
}




// Loops through map1 and returns first ID from it found on map2.
//
QString MTCompose::FindIDMatch(mapIDName map1, mapIDName map2)
{
    for (mapIDName::iterator it = map1.begin(); it != map1.end(); ++it)
    {
        mapIDName::iterator found = map2.find(it.key());

        if (map2.end() != found)
            return it.key();
    }
    return QString("");
}




// This insures that the sender and recipient nym are using the sam e Msg Method.
// Which is to say, the same OT server, or both using Bitmessage, etc.
//
// NOTE: You probably shouldn't call this if m_method is already set, since it
// was probably set based on outside knowledge. Even if we can't prove that a
// given recipient uses a specific OT Server, that doesn't mean he doesn't, and
// that we weren't able to set that info on the compose window in the first place
// from some receipt. For example, likely the user just clicked "Reply to Sender"
// on a specific receipt.
//
// In the event when the sender and recipient are both known, but the method is
// empty, MakeSureCommonMsgMethod is used to automatically choose a transport method
// between them.
//
// What if the method isn't empty, and sender and recipient are both known? In that
// case we should probably make sure that the method (for sender) and address (for
// recipient) are known. If the message type is an OT Server, we make sure both
// parties are known to be on that server. If the message type is anything else
// (like Bitmessage) then we make sure we have a Bitmessage method for the sender
// and a Bitmessage address for the recipient.
//
bool MTCompose::MakeSureCommonMsgMethod()
{
    if (bCanMessage_) {
        return true;
    }

    // If we have both Nym IDs... (sender / recipient.)
    // That's the presupposition of this entire function.
    //
    // Either we have both IDs, and no m_method, or we have
    // both IDs, and we DO have m_method. Either way, we have
    // both IDs.
    //

    // NOTE: These are already checked in the calling function, so I didn't
    // want to needlessly check twice, since this function itself is only
    // called on one place.
    //
//    if (!hasSenderAndRecipient())
//    {
//        qDebug() << "MakeSureCommonMsgMethod was just called even though sender (or recipient) isn't set.";
//        return false;
//    }
    // -------------------------------------------
    // Note: the same contact could have 3 different Nyms.
    //
    // Note: If we had a Recipient Nym ID, but no ContactID was set, the
    //       above call to HasRecipient() sets the ContactID based on the
    //       NymID (if one is available.) Therefore by this point we know
    //       if there was a Recipient Nym but no Contact set, that the
    //       contact is now set.
    //       But what we DON'T know, is if there was a contact set but no
    //       Recipient Nym ID, whether the recipient NymID can be derived
    //       from the Contact ID. This is the point where we actually have
    //       to set that, if it's not already set (and if it's needed.)
    // -------------------------------------------
    // Why do I say "if it's needed" ?
    //
    // Because we definitely need a recipient NymID if we are sending through
    // an OT Server. But if we are sending through Bitmessage, the Recipient Nym
    // is optional. Sure, we'd still like to encrypt the message to a recipient
    // Nym IF one is available, but if one is not available, we'd still like the
    // ability to send the Bitmessage, since otherwise Moneychanger will be useless
    // as a Bitmessage client!
    //
    // Therefore here we need to see if the Recipient Nym exists. If it doesn't,
    // we need to see if msgtype is "otserver", and if it is, we need to REQUIRE
    // a recipient Nym to be selected.
    // But if the msgtype isn't "otserver" and the Recipient Nym doesn't exist,
    // then we need to ask the user to choose one of the Contact's Nyms. But if
    // there is no Recipient Contact ID at all, or if there are no Nyms found for
    // that contact, then pop up a warning dialog saying, "Warning, the Recipient
    // Contact apparently has no Nyms associated with it, and so the outgoing
    // message will not be encrypted by Open-Transactions. Is this okay?"
    //
    // Thus, below this block, we will know for a fact that either a Nym is available
    // if needed or wanted, or that if a Nym is not available, that he's not needed
    // and not wanted.
    //
    if (m_recipientNymId.isEmpty())
    {
        if ((m_recipientContactId <= 0) && !m_recipientAddress.isEmpty())
            m_recipientContactId = MTContactHandler::getInstance()->GetContactByAddress(m_recipientAddress);

        if ((m_recipientContactId <= 0) && !m_recipientAddress.isEmpty())
            m_recipientNymId = MTContactHandler::getInstance()->GetNymByAddress(m_recipientAddress);

        if (m_recipientNymId.isEmpty())
        {
            mapIDName theMap;

            if (sendingThroughOTServer()) // SENDING THROUGH AN OT SERVER
            {
                // REQUIRE a recipient Nym to be selected.
                //
                if (m_recipientContactId <= 0)
                {
                    qDebug() << "MTCompose::MakeSureCommonMsgMethod: SHOULD NEVER HAPPEN: Recipient wasn't even set.";

                    QMessageBox::warning(this, tr("Contact not even set"),
                                         tr("SHOULD NEVER HAPPEN: Recipient wasn't even set"));
                    return false;
                }
                else
                {
                    MTContactHandler::getInstance()->GetNyms(theMap, m_recipientContactId);

                    if (0 == theMap.size())
                    {
                        QString qstrContactName = MTContactHandler::getInstance()->GetContactName(m_recipientContactId);

                        QMessageBox::warning(this, tr("Contact has no known identities"),
                                             tr("Sorry, recipient '%1' has no known Nyms (to send an OT message to.)").arg(qstrContactName));
                        return false;
                    }
                    else if (!chooseRecipientNym(theMap))
                        return false;
                }
            }
            else // NOT sendingThroughOTServer()
            {
                // Ask the user to choose one of the Recipient Contact's Nyms.
                // If there is no Recipient Contact ID at all, or if there are no Nyms found
                // for that contact, then pop up a warning dialog saying, "Warning, the
                // Recipient Contact apparently has no Nyms associated with it, and so the
                // outgoing message will not be encrypted by Open-Transactions. Is this okay?"
                //
                bool bNeedToWarn = false;

                if (m_recipientContactId <= 0)
                    bNeedToWarn = true;
                else // There's a Contact ID, so let's choose one of his Nyms (if there are any.)
                {
                    MTContactHandler::getInstance()->GetNyms(theMap, m_recipientContactId);

                    if (0 == theMap.size())
                        bNeedToWarn = true;
                    else if (!chooseRecipientNym(theMap))
                        return false;
                }
                // -----------------------------------------------
                if (bNeedToWarn)
                {
                    QMessageBox::StandardButton reply;

                    reply = QMessageBox::question(this, "",
                                                  tr("The recipient has no Nyms associated with it, and so the outgoing message will NOT be encrypted by Open-Transactions. Is this okay?"),
                                                  QMessageBox::Yes|QMessageBox::No);
                    if (reply == QMessageBox::No)
                      return false;
                }
            } // else (NOT sendingThroughOTServer())
        }  // if recipientNymID.isEmpty()
    } // if recipientNymID.isEmpty()
    // -------------------------------------------
    // Below this block, we will know for a fact that either a recipient Nym is available
    // if needed or wanted, or that if a Nym is not available, that he's not needed.
    // -------------------------------------------
    // Here we check to see, if the msgtype is empty, whether we can set it based
    // on sender address/method or recipient address.
    //
    if (m_msgtype.isEmpty())
    {
        QString qstrMethodTypeSender, qstrMethodTypeRecipient;

        if (!m_senderAddress.isEmpty())
            qstrMethodTypeSender    = MTContactHandler::getInstance()->GetMethodType(m_senderAddress);

        if (!m_recipientAddress.isEmpty())
            qstrMethodTypeRecipient = MTContactHandler::getInstance()->GetMethodType(m_recipientAddress);
        // -------------------------------------------------------------------------
        if (!qstrMethodTypeSender.isEmpty() && !qstrMethodTypeRecipient.isEmpty() &&
                (0 == qstrMethodTypeRecipient.compare(qstrMethodTypeSender)))
            this->setInitialMsgType(qstrMethodTypeSender); // (server="" by default.)
        else if (!qstrMethodTypeSender.isEmpty() && !qstrMethodTypeRecipient.isEmpty())
        {
            // This means they both exist, but they don't match (because otherwise the
            // previous block would have run instead of this one.)
            // So whose do we choose, the sender, or the recipient? For now, I say we
            // choose the recipient, since the user must have chosen it, and then we'll
            // just select a matching method for the sender further on down below.
            //
            this->setInitialMsgType(qstrMethodTypeRecipient); // (server="" by default.)
        }
        // -------------------------------------------------------------------------
        // In this case, the method type IS known for the sender, but NOT for the recipient.
        // Therefore we'll go with the sender.
        //
        else if (!qstrMethodTypeSender.isEmpty() && qstrMethodTypeRecipient.isEmpty())
            this->setInitialMsgType(qstrMethodTypeSender); // (server="" by default.)
        // -------------------------------------------------------------------------
        // In this case, the method type IS known for the recipient, but NOT for the sender.
        // Therefore we'll go with the recipient.
        //
        else if (qstrMethodTypeSender.isEmpty() && !qstrMethodTypeRecipient.isEmpty())
            this->setInitialMsgType(qstrMethodTypeRecipient); // (server="" by default.)
        // -------------------------------------------------------------------------
        // Whereas if BOTH are empty, there might still be addresses set -- we just don't know
        // what type they are. You might reply to someone's message, which puts his address
        // as the "recipient" -- even if that address was never previously in your address
        // book. Therefore, just because you aren't familiar with the address, and can't
        // find its type, doesn't mean it's an invalid address. We probably still want to
        // try to send to that address! So further on below, you will see that when the type
        // of an address is not ascertainable, the address is still considered "valid" as long
        // as we know the msgtype we're supposed to TRY to use that address as.
        // But by THIS point, in THIS block, if we still haven't figured out the msgtype, then
        // we have to error out, because even though we might have an address, we have no idea
        // how to try to send to it.
        // So at this point we have to tell the user to click the "Via" button and specify the
        // transport.
        // UPDATE: I moved this MessageBox lower, since it's still possible yet to figure out
        // the transport type and set it dynamically. Only once we've exhausted any hope of
        // doing that, do we pop up this box and then return false. (Below, lower down.)
        //
//      QMessageBox::warning(this, tr("Message Has No Transport"),
//                           tr("Before sending, please click 'Via' to choose HOW it will be sent."));
//      return false;
    }
    // -------------------------------------------
    // Above we tried to ascertain the method (if it was empty)
    // based on whether it could be derived from one or both of
    // the sender/recipient.
    //
    // If we STILL have no method, then we just find and choose
    // one that the sender/recipient have in common, which we can
    // potentially do by searching our local database. This is
    // what the below block does. It has to then set their addresses
    // and method ID, and ask the user to choose when necessary.
    //
    // After that:
    // Either way, if we DO have a method now, then we next make sure
    // the sender/recipient are compatible with it. This is where we
    // insure we have chosen the right methodID for sender and address
    // for recipient. If method type is "otserver|notary_id" then
    // we see if both are known to be on that server. Whereas if
    // the type is "bitmessage" then we look up the methodID and
    // address appropriately.
    //
    if (m_msgtype.isEmpty())
    {
        // LOGIC:
        //
        // if m_NotaryID is set, and senderNymId is set (which it definitely is, by this point), and recipientNymId is set,
        // then set m_msgtype to "otserver."
        //
        // Else if all that is true but we are missing recipientNymID, and since we already know there is not going to BE a
        // RecipientNymID (period -- see blocks higher up), then we know for a FACT the msgtype can NOT be "otserver".
        // THEREFORE we loop through the types available in MTComms and find one that works for both parties.
        //
        // HOWEVER, what if both already have an ADDRESS / METHOD ID set? In that case we don't have to loop, since it's already
        // set. In that case though, we should have checked that before we even entered this block, and set the msgtype by now
        // already. Therefore I will add that now, above this block. Update: Added. Therefore we now know for a fact that if we
        // were able to ascertain the msgtype based on any preset addresses, that we have already done so by this point.
        // Therefore the fact that the msgtype is still blank, means we were definitely unable to determine it based on the
        // addresses (if there were any.) Therefore our next step in here is to try to find a msgtype that the sender/recipient
        // DO have in common, and set their addresses accordingly. If we fail, then we will display the "Message Has No Transport"
        // error and ask the user to click the "Via" button.
        //
        // -------------------------------------------
        if (!m_NotaryID.isEmpty() && !m_recipientNymId.isEmpty()) // No need to check if senderNymID isn't empty, since we already know it isn't, by this point.
        {
            this->setInitialMsgType(QString("otserver")); // arg server="" by default.
            // BUT: If m_NotaryID is already set, and blank is passed, then m_NotaryID's
            // value is preserved (not overwritten by the blank "")
        }
        else
        {
            // Since we know the msgtype can't possible be "otserver" (since either the server ID is empty,
            // or the recipientNymId is still empty) we go ahead and blank out any server ID that might be
            // there, and proceed to find a msgtype that they DO have in common.
            //
//          m_NotaryID = QString(""); // Now done farther below.
            // -------------------------------------
            mapOfCommTypes mapTypes;
            bool bFoundOneInCommon = false;
            bool bGotCommTypes     = MTComms::types(mapTypes);

            // Here we're looping through the list of transport types OTHER than "otserver",
            // such as "bitmessage". See if we can find one that both parties have in common.
            //
            if (bGotCommTypes)
            {
                QString qstrMsgTypeAttempt, qstrMsgTypeDisplay;
                mapIDName mapSenderAddresses, mapRecipientAddresses;

                for (mapOfCommTypes::iterator it = mapTypes.begin(); it != mapTypes.end(); ++it)
                {
                    qstrMsgTypeAttempt = QString("");
                    qstrMsgTypeDisplay = QString("");

                    std::string strTypeName    = it->first;
                    std::string strTypeDisplay = it->second;

                    if (!strTypeName.empty() && !strTypeDisplay.empty())
                    {
                        qstrMsgTypeAttempt = QString::fromStdString(strTypeName);
                        qstrMsgTypeDisplay = QString::fromStdString(strTypeDisplay);

                        mapSenderAddresses   .clear();
                        mapRecipientAddresses.clear();

                        if (this->CheckPotentialCommonMsgMethod(qstrMsgTypeAttempt, &mapSenderAddresses, &mapRecipientAddresses))
                        {
                            bFoundOneInCommon = true;
                            break;
                        }
                    }
                } // for
                // -------------------------------
                // Did we find one?
                //
                if (bFoundOneInCommon)
                {
                    // Okay then, let's set the msgtype to qstrMsgTypeAttempt, and we'll set the sender and
                    // recipient addresses. (And sender method ID.) If there's only one address to choose from
                    // for either, we can just go with it. But if there are more than one, we must ask the user
                    // to choose which he prefers.
                    //
                    if (!this->chooseSenderAddress(mapSenderAddresses, qstrMsgTypeDisplay))
                        return false;

                    if (!this->chooseRecipientAddress(mapRecipientAddresses, qstrMsgTypeDisplay))
                        return false;
                    // -------------------------------------------
                    // If both addresses were selected (we got this far, didn't we?)
                    // then we go ahead and set the msgtype.
                    //
                    this->setInitialMsgType(qstrMsgTypeAttempt); // (server="" by default.)

                } // if (bFoundOneInCommon)
            } // if (bGotCommTypes)
        }
    } // if (m_msgtype.isEmpty())
    // ------------------------------------
    // By this point, if msgtype STILL wasn't set, we have no choice but to return empty-handed.
    //
    if (m_msgtype.isEmpty())
    {
        QMessageBox::warning(this, tr("Message Has No Transport"),
                             tr("Before sending, please click 'Via' to choose HOW it will be sent."));
        return false;
    }
    // -----------------------------------------
    // By this point we know for a fact:
    //
    // -- We know that m_msgtype is definitely set to a specific messagetype.
    // -- If that type is "otserver", we know m_NotaryID is set.
    // -- We know, no matter what, that the sender Nym ID is set by now.
    // -- If the type is "otserver", then we know the recipient Nym IDs is set.
    // -- If the type is NOT "otserver", then we know the recipient Nym ID MIGHT be set...
    //    ...but also might NOT be. And either way, we know that's all we'll know about it.
    // -- If the type is NOT "otserver", (e.g. it's "bitmessage" or anything else) AND if
    //    we had to figure that out above, then we know the sender and recipient addresses
    //    ARE set, and we also know that we tried to find the sender method ID based on the
    //    sender address. (Which may or may not have succeeded.)
    // ----------------------------------------------------------------------------------------
    //
    // We DO have a method, so next, we need to make sure the sender
    // and recipient are compatible with it. This is where we make sure
    // we have chosen the right methodID for sender, and the right address
    // for the recipient.
    //
    if (!sendingThroughOTServer())
    {
        QString qstrMethodTypeSender, qstrMethodTypeRecipient;

        if (!m_senderAddress.isEmpty())
            qstrMethodTypeSender    = MTContactHandler::getInstance()->GetMethodType(m_senderAddress);

        if (!m_recipientAddress.isEmpty())
            qstrMethodTypeRecipient = MTContactHandler::getInstance()->GetMethodType(m_recipientAddress);
        // -------------------------------------------------------------------------
        // If both addresses exist, and both have a message type, and their message types
        // match each other, but they do NOT match the actual msgtype, then set it to
        // match them.
        //
        if (!qstrMethodTypeSender.isEmpty() && !qstrMethodTypeRecipient.isEmpty() &&  // If sender AND recipient address exists,
                (0 == qstrMethodTypeSender.compare(qstrMethodTypeRecipient)) &&       // and if they are the same,
                (0 != qstrMethodTypeSender.compare(m_msgtype)))                       // and if they do NOT match with m_msgtype
            this->setInitialMsgType(qstrMethodTypeSender);                            // ...Then set the msgtype to match with them.
        // -------------------------------------------------------------------------
        // We DON'T KNOW if both sender and recipient addresses are even set yet.
        // Further: just because an address is set doesn't mean our local database
        // has any record of it. (We must give unknown addresses the benefit of
        // the doubt.)
        // But we DO know what the msgtype is. Therefore if either address is set,
        // its type had better either be unknown (because it wasn't in our address
        // book yet) or known and matching the msgtype.
        // If the sender or recipient address is known, and DOESN'T match the msgtype,
        // then we have a problem and have to return false.
        // Then if BOTH addresses exist (and knowing they are already validated against
        // the msgtype just above) then we are good to go!
        // But if one (or both) of the addresses doesn't exist, then we have to choose
        // it from among the available addresses for the msgtype.
        // ----------------------------------------------------------
        // If either address is set, its type had better either be unknown (because
        // it wasn't in our address book yet) or known and MATCHES the msgtype.
        //
        // If the sender or recipient address type is known from the DB, yet DOESN'T
        // match the msgtype, then we have a problem and have to return false.

        if (!qstrMethodTypeSender.isEmpty() &&
            (0 != m_msgtype.compare(qstrMethodTypeSender)))
        {
            qDebug() << "MTCompose::MakeSureCommonMsgMethod: Transport type of sender doesn't match expected transport type.";

            QMessageBox::warning(this, tr("Wrong Transport"),
                                 tr("Sender address produced unexpected transport type. Please click 'From' and update the sender."));
            return false;
        }

        if (!qstrMethodTypeRecipient.isEmpty() &&
            (0 != m_msgtype.compare(qstrMethodTypeRecipient)))
        {
            qDebug() << "MTCompose::MakeSureCommonMsgMethod: Transport type of recipient doesn't match expected transport type.";

            QMessageBox::warning(this, tr("Wrong Transport"),
                                 tr("Recipient address produced unexpected transport type. Please click 'To' and update the recipient."));
            return false;
        }
        // ----------------------------------------------------------
        // But if one (or both) of the addresses doesn't exist, then we have to choose
        // it from among the available addresses for the msgtype.
        //
        QString qstrMsgTypeDisplay = QString::fromStdString(MTComms::displayName(m_msgtype.toStdString()));

        if (m_senderAddress.isEmpty())
        {
            mapIDName mapSenderAddresses;
            bool      bGotAddresses = false;

            if (!m_senderNymId.isEmpty())
            {
                if (m_senderMethodId > 0)
                    bGotAddresses = MTContactHandler::getInstance()->GetAddressesByNym(mapSenderAddresses,
                                                                                       m_senderNymId, m_senderMethodId);
                if (!bGotAddresses)
                    bGotAddresses = MTContactHandler::getInstance()->GetAddressesByNym(mapSenderAddresses,
                                                                                       m_senderNymId, m_msgtype);
                if (bGotAddresses)
                    this->chooseSenderAddress(mapSenderAddresses, qstrMsgTypeDisplay);
            }
        }

        if (m_recipientAddress.isEmpty())
        {
            mapIDName mapRecipientAddresses;
            bool      bGotAddresses = false;

            if (!m_recipientNymId.isEmpty())
                bGotAddresses = MTContactHandler::getInstance()->GetAddressesByNym(mapRecipientAddresses,
                                                                                   m_recipientNymId, m_msgtype);

            if (!bGotAddresses && (m_recipientContactId > 0))
                bGotAddresses = MTContactHandler::getInstance()->GetAddressesByContact(mapRecipientAddresses,
                                                                                       m_recipientContactId, m_msgtype);

            if (bGotAddresses)
                this->chooseRecipientAddress(mapRecipientAddresses, qstrMsgTypeDisplay);
        }
        // ----------------------------------------------------------
        if (m_senderAddress.isEmpty())
        {
            QMessageBox::warning(this, tr("Missing Sender Address"),
                                 tr("Sender address missing. Please click 'From' and update the sender."));
            return false;
        }
        else if (m_senderMethodId <= 0)
        {
            mapIDName mapMethods;
            bool bGotMethods = MTContactHandler::getInstance()->GetMsgMethodsByNym(mapMethods, m_senderNymId, /*bAddServers=*/false, m_msgtype);

            if (!bGotMethods)
            {
                // Just because the Sender Nym doesn't have msg methods for a specific transport type,
                // doesn't mean that NO NYMS IN THE ENTIRE WALLET have msg methods for that transport
                // type.
                // So next step is to see if there are ANY in the whole wallet for the given
                // type, and then ask the user to choose one that the address will be associated
                // with.
                //
                bGotMethods = MTContactHandler::getInstance()->GetMsgMethods(mapMethods, /*bAddServers=*/false, m_msgtype);

                if (bGotMethods)
                {
                    QMessageBox::StandardButton reply;

                    QString qstrQuestion = QString("%1 %2 %3").arg(tr("Sender has no")).arg(qstrMsgTypeDisplay).arg(tr("interfaces. Choose one?"));

                    reply = QMessageBox::question(this, "", qstrQuestion, QMessageBox::Yes|QMessageBox::No);

                    if ((reply == QMessageBox::No) || !this->chooseSenderMethodID(mapMethods, qstrMsgTypeDisplay))
                        return false;
                    else
                        MTContactHandler::getInstance()->AddMsgAddressToNym(m_senderNymId, m_senderMethodId, m_senderAddress);
                }
                else
                {
                    QString qstrWarning = QString("%1 %2 %3").arg(tr("No")).arg(qstrMsgTypeDisplay).arg(tr("interfaces found. Edit the Sender to add one, and then try again."));
                    QMessageBox::warning(this, tr("No Transport Interfaces"), qstrWarning);
                    return false;
                }
            }
            else if (!this->chooseSenderMethodID(mapMethods, qstrMsgTypeDisplay))
                return false;
        }
        // -------------------------------------
        if (m_recipientAddress.isEmpty())
        {
            QMessageBox::warning(this, tr("Missing Recipient Address"),
                                 tr("Recipient address missing. Please click 'To' and update the recipient."));
            return false;
        }
        // ----------------------------------------------------------
        m_NotaryID = QString("");
    }
    else // Sending through an OT server
    {
        // server id is set?
        if (m_NotaryID.isEmpty())
        {
            QMessageBox::warning(this, tr("Missing OT Server"),
                                 tr("OT Server ID missing. Please click 'Via' and choose a server."));
            return false;
        }
        // ---------------------------------------------------
        // recipient nym ID is set? Otherwise instruct user to click 'to' and choose one.
        //
        if (m_recipientNymId.isEmpty())
        {
            QMessageBox::warning(this, tr("Missing Recipient"),
                                 tr("Recipient Nym ID missing. Please click 'To' and choose a recipient."));
            return false;
        }
        // ---------------------------------------------------
        // sender nym is registered there? Warn if not and give option to register there.
        //
        if (!verifySenderAgainstServer())
            return false;
        // ---------------------------------------------------
        // recipient nym is known to frequent that server? if not, warn the user and give him
        // the option to just look it up directly on the server.
        //
        if (!verifyRecipientAgainstServer())
            return false;
    }
    // -----------------------------------------
    return true;
}


bool MTCompose::verifySenderAgainstServer(bool bAsk/*=true*/, QString qstrNotaryID/*=QString("")*/)   // Assumes senderNymId and NotaryID are set.
{
    if (bCanMessage_)
        return true;

    if (qstrNotaryID.isEmpty())
        qstrNotaryID = m_NotaryID;

    // sender nym is registered there? Warn if not and give option to register there.
    //
    std::string notary_id    = qstrNotaryID .toStdString();
    std::string sender_id    = m_senderNymId.toStdString();

    if (!opentxs::OT::App().API().Exec().IsNym_RegisteredAtServer(sender_id, notary_id))
    {
        if (bAsk)
        {
            QMessageBox::StandardButton reply;

            reply = QMessageBox::question(this, "", tr("Sender Nym not registered on selected OT server. Register now?"),
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes)
            {

                std::string response;
                {
                    MTSpinner theSpinner;

                    response = opentxs::String(opentxs::OT::App().API().Sync().RegisterNym(opentxs::Identifier(sender_id), opentxs::Identifier(notary_id), true)).Get();
                }

                qDebug() << QString("Nym Registration Response: %1").arg(QString::fromStdString(response));

                int32_t nReturnVal = opentxs::VerifyMessageSuccess(response);

                if (1 != nReturnVal)
                {
                    Moneychanger::It()->HasUsageCredits(notary_id, sender_id);
                    return false;
                }
                else
                    MTContactHandler::getInstance()->NotifyOfNymServerPair(QString::fromStdString(sender_id),
                                                                           QString::fromStdString(notary_id));
            }
            else
                return false;
        }
        else
            return false;
    }
    return true;
}

bool MTCompose::verifyRecipientAgainstServer(bool bAsk/*=true*/, QString qstrNotaryID/*=QString("")*/) // Assumes m_senderNymId, m_recipientNymId and NotaryID are set.
{
    if (bCanMessage_)
        return true;

    if (qstrNotaryID.isEmpty())
        qstrNotaryID = m_NotaryID;

    // recipient nym is known to frequent that server? if not, warn the user and give him
    // the option to just look it up directly on the server.
    //
    std::string notary_id    = qstrNotaryID    .toStdString();
    std::string sender_id    = m_senderNymId   .toStdString();
    std::string recipient_id = m_recipientNymId.toStdString();

    mapIDName mapServers;
    bool      bGotServers = false;

    if (m_recipientContactId > 0)
        bGotServers = MTContactHandler::getInstance()->GetServers(mapServers, m_recipientContactId);

    if (!bGotServers && !m_recipientNymId.isEmpty())
        bGotServers = MTContactHandler::getInstance()->GetServers(mapServers, m_recipientNymId);

    if (bGotServers)
    {
        mapIDName::iterator it = mapServers.find(qstrNotaryID);

        if (mapServers.end() == it)
        {
            if (bAsk)
            {
                QMessageBox::StandardButton reply;

                reply = QMessageBox::question(this, "", tr("Recipient Nym not known to frequent the selected OT server. Shall I ask the server and find out?"),
                                              QMessageBox::Yes|QMessageBox::No);
                if (reply == QMessageBox::Yes)
                {

                    std::string response;
                    {
                        MTSpinner theSpinner;

                        auto action = opentxs::OT::App().API().ServerAction().DownloadNym(
                        		opentxs::Identifier(sender_id), opentxs::Identifier(notary_id), opentxs::Identifier(recipient_id));
                        response = action->Run();
                    }

                    int32_t nReturnVal = opentxs::VerifyMessageSuccess(response);

                    if (1 != nReturnVal)
                    {
                        QMessageBox::warning(this, tr("Recipient Not Found on Server"),
                                             tr("Recipient Nym not found on selected OT server. Please click 'Via' and choose a different server or a different transport method."));
                        Moneychanger::It()->HasUsageCredits(notary_id, sender_id);
                        return false;
                    }
                    else
                        emit nymWasJustChecked(m_recipientNymId);
                }
                else
                    return false;
            }
            else
                return false;
        }
    }
    return true;
}



void MTCompose::on_fromButton_clicked()
{
    if (bCanMessage_)
        return;

    if (!m_senderAddress.isEmpty())
    {
        QString qstrOldNymByAddress  = MTContactHandler::getInstance()->GetNymByAddress(m_senderAddress);

        if (m_senderNymId.isEmpty() && !qstrOldNymByAddress.isEmpty())
            m_senderNymId = qstrOldNymByAddress;

        if ((0 == m_senderMethodId) && !m_senderNymId.isEmpty())
            m_senderMethodId = MTContactHandler::getInstance()->GetMethodIDByNymAndAddress(m_senderNymId, m_senderAddress);
    }
    // ---------------------------
    // Select from Nyms in local wallet.
    //
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;
    bool bFoundDefault = false;
    // -----------------------------------------------
    const int32_t nym_count = opentxs::OT::App().API().Exec().GetNymCount();
    // -----------------------------------------------
    for (int32_t ii = 0; ii < nym_count; ++ii)
    {
        //Get OT Nym ID
        QString OT_nym_id = QString::fromStdString(opentxs::OT::App().API().Exec().GetNym_ID(ii));
        QString OT_nym_name("");
        // -----------------------------------------------
        if (!OT_nym_id.isEmpty())
        {
            if (!m_senderNymId.isEmpty() && (OT_nym_id == m_senderNymId))
                bFoundDefault = true;
            // -----------------------------------------------
            MTNameLookupQT theLookup;

            OT_nym_name = QString::fromStdString(theLookup.GetNymName(OT_nym_id.toStdString(), ""));
            // -----------------------------------------------
            the_map.insert(OT_nym_id, OT_nym_name);
        }
    }
    // -----------------------------------------------
    if (bFoundDefault && !m_senderNymId.isEmpty())
        theChooser.SetPreSelected(m_senderNymId);
    // -----------------------------------------------
    theChooser.setWindowTitle(tr("Choose Sender"));
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
        if (!theChooser.m_qstrCurrentID.isEmpty())
        {
            this->setInitialSenderNym(theChooser.m_qstrCurrentID);
            // ---------------------------------------------
            if (!m_msgtype.isEmpty() && !sendingThroughOTServer())
            {
                QString qstrMsgTypeDisplay = QString::fromStdString(MTComms::displayName(m_msgtype.toStdString()));

                if (qstrMsgTypeDisplay.isEmpty())
                    qstrMsgTypeDisplay = m_msgtype;
                // -------------------------------------------
                mapIDName mapSenderAddresses;
                bool      bGotAddresses = false;

                if (!m_senderNymId.isEmpty())
                    bGotAddresses = MTContactHandler::getInstance()->GetAddressesByNym(mapSenderAddresses,
                                                                                       m_senderNymId, m_msgtype);
                if (bGotAddresses)
                    this->chooseSenderAddress(mapSenderAddresses, qstrMsgTypeDisplay);
            }
            // -------------------------------------------
            // We've just chosen the sender.
            //
            // What if a recipient is already there?
            //
            // Well, if the msgtype is already set, we should see if the recipient is
            // compatible with it. If he isn't, then we should see if we can auto-select
            // a new msgtype that sender and recipient are both compatible with.
            //
            // If the msgtype ISN'T set, then we should choose one automatically, if
            // possible, that both sender and recipient are compatible with.
            //
            FindRecipientMsgMethod();
        }
    }
    else
    {
//      qDebug() << "CANCEL was clicked";
    }
}


// -------------------------------------------------------------



bool MTCompose::chooseRecipientNym(mapIDName & theMap)
{
    if (1 == theMap.size())
    {
        mapIDName::iterator it = theMap.begin();
        this->setInitialRecipient(it.key(), m_recipientContactId, m_recipientAddress);
        return true;
    }
    else if (theMap.size() > 1)
    {
        // There are multiple matching addresses for the recipient.
        // (Have the user choose one of them.)
        //
        DlgChooser theChooser(this);
        // -----------------------------------------------
        theChooser.m_map = theMap;
        // -----------------------------------------------
        QString qstrWindowTitle = QString(tr("Recipient has multiple Nyms"));
        theChooser.setWindowTitle(qstrWindowTitle);
        // -----------------------------------------------
        if (theChooser.exec() == QDialog::Accepted)
        {
            if (!theChooser.m_qstrCurrentID.isEmpty())
            {
                this->setInitialRecipient(theChooser.m_qstrCurrentID, m_recipientContactId, m_recipientAddress);
                return true;
            }
        }
    }
    // --------------------
    return false;
}


void MTCompose::on_toButton_clicked()
{
    if (bCanMessage_)
        return;

    // Select recipient from the address book and convert to Nym ID.
    // -----------------------------------------------
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;

    MTContactHandler::getInstance()->GetContacts(the_map);
    // ---------------------------
    if (!m_recipientAddress.isEmpty())
    {
        QString qstrOldNymByAddress  = MTContactHandler::getInstance()->GetNymByAddress(m_recipientAddress);
        int     nOldContactByAddress = MTContactHandler::getInstance()->GetContactByAddress(m_recipientAddress);

        if (m_recipientNymId.isEmpty() && !qstrOldNymByAddress.isEmpty())
            m_recipientNymId = qstrOldNymByAddress;

        if ((m_recipientContactId <= 0) && (nOldContactByAddress > 0))
            m_recipientContactId = nOldContactByAddress;
    }
    // ---------------------------
    // Set Chooser's default to recipient nym's Contact, if nym is
    // there and Contact is found based on it.
    if (!m_recipientNymId.isEmpty())
    {
        const int nContactID = MTContactHandler::getInstance()->FindContactIDByNymID(m_recipientNymId);
        if (nContactID > 0)
            m_recipientContactId = nContactID;
    }
    // ---------------------------
    if (m_recipientContactId > 0)
    {
        QString strTempID = QString("%1").arg(m_recipientContactId);
        theChooser.SetPreSelected(strTempID);
    }
    // -----------------------------------------------
    theChooser.setWindowTitle(tr("Choose Recipient"));
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
//        qDebug() << QString("SELECT was clicked for ID: %1").arg(theChooser.m_qstrCurrentID);

        // We try to choose a NymID based on the selected Contact.
        //
        int nSelectedContactID = theChooser.m_qstrCurrentID.toInt();

        this->setInitialRecipient(QString(""), 0, QString(""));
        this->setInitialRecipientContactID(nSelectedContactID);
        // ---------------------------------------------
        // Next we try to find a Nym based on this Contact...
        //
        mapIDName theNymMap;

        if (MTContactHandler::getInstance()->GetNyms(theNymMap, nSelectedContactID))
            this->chooseRecipientNym(theNymMap);
        // ---------------------------------------------
        if (!m_msgtype.isEmpty() && !sendingThroughOTServer())
        {
            QString qstrMsgTypeDisplay = QString::fromStdString(MTComms::displayName(m_msgtype.toStdString()));

            if (qstrMsgTypeDisplay.isEmpty())
                qstrMsgTypeDisplay = m_msgtype;
            // -------------------------------------------
            mapIDName mapRecipientAddresses;
            bool      bGotAddresses = false;

            if (!m_recipientNymId.isEmpty())
                bGotAddresses = MTContactHandler::getInstance()->GetAddressesByNym(mapRecipientAddresses,
                                                                                   m_recipientNymId, m_msgtype);

            if (!bGotAddresses && (m_recipientContactId > 0))
                bGotAddresses = MTContactHandler::getInstance()->GetAddressesByContact(mapRecipientAddresses,
                                                                                       m_recipientContactId, m_msgtype);

            if (bGotAddresses)
                this->chooseRecipientAddress(mapRecipientAddresses, qstrMsgTypeDisplay);
            // -------------------------------------------
            // We've just chosen the recipient.
            //
            // What if a sender is already there?
            //
            // Well, if the msgtype is already set, we should see if the sender is
            // compatible with it. If he isn't, then we should see if we can auto-select
            // a new msgtype that sender and recipient are both compatible with.
            //
            // If the msgtype ISN'T set, then we should choose one automatically, if
            // possible, that both sender and recipient are compatible with.
            //
            FindSenderMsgMethod();
        }
    }
    else
    {
//      qDebug() << "CANCEL was clicked";
    }
    // -----------------------------------------------
}




void MTCompose::dialog()
{
/** Compose Dialog **/

    if (!already_init)
    {
        if (!Moneychanger::It()->expertMode())
        {
            ui->toolButtonTo->setVisible(false);
            ui->toolButtonFrom->setVisible(false);
        }
        connect(this, SIGNAL(balancesChanged()), Moneychanger::It(), SLOT  (onBalancesChanged()));
        connect(this, SIGNAL(nymWasJustChecked(QString)), Moneychanger::It(), SLOT  (onCheckNym(QString)));
        // ---------------------------------------
        this->setWindowTitle(tr("Compose Message"));

        QString style_sheet = "QPushButton{border: none; border-style: outset; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa);}"
                "QPushButton:pressed {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }"
                "QPushButton:hover {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }";

        ui->fromButton->setStyleSheet(style_sheet);
        ui->toButton  ->setStyleSheet(style_sheet);
        ui->viaButton ->setStyleSheet(style_sheet);

        // Here if there is pre-set data for the subject, contents, to, from, server, etc
        // then we set it here.
        //

        // Also, need ability to pre-set server since we might be replying
        // to a mail, or messaging based on a transaction, in which case
        // the server is already known and should be used.
        //
        // Also, we need ability to juggle the server setting.
        // For example, if we have known servers for the recipient,
        // we will try to see if our default server is one of those.
        // If so, we use that. Otherwise we see if ALL servers for
        // OUR Nym contain at least one match with the recipient, and
        // use the first one found.
        // Either way, the list we use will be filtered by both Nyms,
        // the above is only for setting the initial state.
        //
        // Otherwise, if a match cannot be found, the list will contain
        // ALL known servers, with the default server selected.
        //
        // When sending to a Nym on a particular server he's NOT known to be
        // on, a warning message should be displayed first.
        // -------------------------------------------
        setSenderNameBasedOnAvailableData();
        setRecipientNameBasedOnAvailableData();
        setTransportDisplayBasedOnAvailableData();
        // -------------------------------------------
        if (!m_subject.isEmpty())
        {
            QString qstrRe = m_bForwarding ? tr("fw:") : tr("re:");

            QString qstrSubjectLeft = m_subject.left(qstrRe.size());

            if (qstrSubjectLeft.toLower() != qstrRe)
            {
                QString strTemp = QString("%1 %2").arg(qstrRe).arg(m_subject);
                m_subject = strTemp;
            }
            // -----------------------
            ui->subjectEdit->setText(m_subject);
            // -----------------------
            this->setWindowTitle(QString("%1: %2").arg(tr("Compose")).arg(m_subject));
        }
        // -------------------------------------------
        if (!m_body.isEmpty())
        {
            QString qstrRe = m_bForwarding ? tr("\n\n-----Forwarded:\n") : tr("\n\n-----You wrote:\n");
            QString qstrReplyBody(m_body);

            qstrRe += QString("%1: %2\n").arg(tr("From")).arg(m_forwardSenderName);
            qstrRe += QString("%1: %2\n\n").arg(tr("To")).arg(m_forwardRecipientName);

            qstrReplyBody = qstrRe + m_body;

            ui->contentsEdit->setPlainText(qstrReplyBody);
        }
        // -------------------------------------------
        ui->contentsEdit->setFocus();

        /** Flag Already Init **/
        already_init = true;
    }

}

void MTCompose::setVariousIds(QString senderNymId, QString recipientNymId, QString senderAddress, QString recipientAddress)
{
    m_forwardSenderNymId      = senderNymId;
    m_forwardRecipientNymId   = recipientNymId;
    m_forwardSenderAddress    = senderAddress;
    m_forwardRecipientAddress = recipientAddress;
    // ----------------------------------------------
    if (!m_forwardSenderNymId.isEmpty())
    {
        MTNameLookupQT theLookup;
        m_forwardSenderName = QString::fromStdString(theLookup.GetNymName(m_forwardSenderNymId.toStdString(), ""));
    }
    if (m_forwardSenderName.isEmpty() && !m_forwardSenderAddress.isEmpty())
    {
        MTNameLookupQT theLookup;
        m_forwardSenderName = QString::fromStdString(theLookup.GetAddressName(m_forwardSenderAddress.toStdString()));
    }
    if (m_forwardSenderName.isEmpty() && !m_forwardSenderNymId.isEmpty())
        m_forwardSenderName = m_forwardSenderNymId;
    else if (m_forwardSenderName.isEmpty() && !m_forwardSenderAddress.isEmpty())
        m_forwardSenderName = m_forwardSenderAddress;
    // ----------------------------------------------
    if (!m_forwardRecipientNymId.isEmpty())
    {
        MTNameLookupQT theLookup;
        m_forwardRecipientName = QString::fromStdString(theLookup.GetNymName(m_forwardRecipientNymId.toStdString(), ""));
    }
    if (m_forwardRecipientName.isEmpty() && !m_forwardRecipientAddress.isEmpty())
    {
        MTNameLookupQT theLookup;
        m_forwardRecipientName = QString::fromStdString(theLookup.GetAddressName(m_forwardRecipientAddress.toStdString()));
    }
    if (m_forwardRecipientName.isEmpty() && !m_forwardRecipientNymId.isEmpty())
        m_forwardRecipientName = m_forwardRecipientNymId;
    else if (m_forwardRecipientName.isEmpty() && !m_forwardRecipientAddress.isEmpty())
        m_forwardRecipientName = m_forwardRecipientAddress;
}


bool MTCompose::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Escape)
        {
            close(); // This is caught by this same filter.
            return true;
        }
        return true;
    }
    else
    {
        // standard event processing
        return QWidget::eventFilter(obj, event);
    }
}


void MTCompose::closeEvent(QCloseEvent *event)
{
    // Pop up a Yes/No dialog to confirm the cancellation of this message.
    // (ONLY if the subject or contents fields contain text.)
    //
    if (!m_bSent && !ui->contentsEdit->toPlainText().isEmpty())
    {
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, "", tr("Are you sure you want to cancel this message?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply != QMessageBox::Yes)
        {
          event->ignore();
          return;
        }
    }
    // -------------------------------------------
    QWidget::closeEvent(event);
}



MTCompose::MTCompose(QWidget *parent) :
    QWidget(parent, Qt::Window),
    m_senderMethodId(0),
    m_recipientContactId(0),
    m_bSent(false),
    already_init(false),
    ui(new Ui::MTCompose)
{
    ui->setupUi(this);

    this->installEventFilter(this);

    connect(this, SIGNAL(balancesChanged()), this, SLOT(onBalancesChanged()));

    connect(this, SIGNAL(ShowContact(QString)),   Moneychanger::It(), SLOT(mc_showcontact_slot(QString)));
    connect(this, SIGNAL(ShowNym(QString)),       Moneychanger::It(), SLOT(mc_show_nym_slot(QString)));
    connect(this, SIGNAL(ShowTransport(QString)), Moneychanger::It(), SLOT(mc_showtransport_slot(QString)));
    connect(this, SIGNAL(ShowServer(QString)),    Moneychanger::It(), SLOT(mc_show_server_slot(QString)));

    ui->toolButton  ->setStyleSheet("QToolButton { border: 0px solid #575757; }");
    ui->toolButton_2->setStyleSheet("QToolButton { border: 0px solid #575757; }");
    ui->toolButton_3->setStyleSheet("QToolButton { border: 0px solid #575757; }");
}

MTCompose::~MTCompose()
{
    delete ui;
}

void MTCompose::on_subjectEdit_textChanged(const QString &arg1)
{
    if (arg1.isEmpty()) // Subject is empty
    {
        m_subject = QString("");
        if (bCanMessage_)
            this->setWindowTitle(tr("Compose message"));
        else
            this->setWindowTitle(tr("Compose: (no subject)"));
    }
    else
    {
        m_subject = arg1;
        this->setWindowTitle(QString("%1: %2").arg("Compose").arg(arg1));
    }
}

void MTCompose::on_toolButtonTo_clicked()
{
    if (bCanMessage_)
        return;

    QString qstrContactID("");
    // ------------------------------------------------
    if (m_recipientContactId > 0)
    {
        qstrContactID = QString("%1").arg(m_recipientContactId);
    }
    else if (!m_recipientNymId.isEmpty())
    {
        int nContactID = MTContactHandler::getInstance()->FindContactIDByNymID(m_recipientNymId);

        if (nContactID > 0)
            qstrContactID = QString("%1").arg(nContactID);
    }
    // ------------------------------------------------
    emit ShowContact(qstrContactID);
}

void MTCompose::on_toolButtonFrom_clicked()
{
    if (bCanMessage_)
        return;

    QString qstrNymID("");
    // ------------------------------------------------
    if (!m_senderNymId.isEmpty())
        qstrNymID = m_senderNymId;
    // ------------------------------------------------
    emit ShowNym(qstrNymID);
}

void MTCompose::on_toolButton_3_clicked()
{
    if (bCanMessage_)
        return;

    if ((QString("otserver") == m_msgtype) && !m_NotaryID.isEmpty())
        emit ShowServer(m_NotaryID);
    else if (QString("otserver") != m_msgtype)
    {
        QString methodID = QString("%1").arg(m_senderMethodId);
        emit ShowTransport(methodID);
    }
}
