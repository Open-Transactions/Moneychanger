#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/compose.hpp>
#include <ui_compose.h>

#include <gui/widgets/overridecursor.hpp>
#include <gui/widgets/dlgchooser.hpp>

#include <core/moneychanger.hpp>
#include <core/handlers/DBHandler.hpp>

#include <opentxs/OTAPI.hpp>
#include <opentxs/OTAPI_Exec.hpp>
#include <opentxs/OT_ME.hpp>

#include <QMessageBox>
#include <QDebug>


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
                                arg(address).arg(methodType).arg(m_msgtype).arg(senderMethodType);

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



void MTCompose::setSenderNameBasedOnAvailableData()
{
    if (NULL != ui)
    {
        QString qstrNymName("");

        if (!m_senderNymId.isEmpty())
            qstrNymName = QString::fromStdString(OTAPI_Wrap::It()->GetNym_Name(m_senderNymId.toStdString()));
        // ---------------------------
        QString qstrAddressPortion("");

        if (!this->m_senderAddress.isEmpty())
        {
            qstrAddressPortion = QString(" (%1)").arg(this->m_senderAddress);
        }
        // ---------------------------
        if (qstrNymName.isEmpty())
        {
            if (m_senderNymId.isEmpty())
                qstrNymName = tr("<Click to choose sender>");
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

        if (!m_recipientNymId.isEmpty())
            qstrNymName = QString::fromStdString(OTAPI_Wrap::It()->GetNym_Name(m_recipientNymId.toStdString()));
        // ---------------------------
        QString qstrContactName;

        if (m_recipientContactId > 0)
            qstrContactName = MTContactHandler::getInstance()->GetContactName(m_recipientContactId);
        // ---------------------------
        if (qstrContactName.isEmpty())
            qstrContactName = qstrNymName;
        // ---------------------------
        QString qstrAddressPortion("");

        if (!this->m_recipientAddress.isEmpty())
        {
            qstrAddressPortion = QString(" (%1)").arg(this->m_recipientAddress);
        }
        // ---------------------------
        if (qstrContactName.isEmpty())
        {
            if (m_recipientNymId.isEmpty() && (m_recipientContactId <= 0) && this->m_recipientAddress.isEmpty())
                qstrContactName = tr("<Click to choose recipient>");
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

void MTCompose::setInitialRecipientContactID(int contactid, QString address/*=""*/)
{
    m_recipientContactId = contactid;    // Recipient Nym kjsdfds982345 might be Contact #2. (Or Nym itself might be blank, with ONLY Contact!)
    m_recipientNymId     = QString("");  // If not available, then m_recipientContactID must be available. (For Bitmessage, for example, Nym is optional.)

    setInitialRecipientAddress(address); // If msgtype is "bitmessage" this will contain a Bitmessage address. For msgtype "otserver", address is blank.
    // NOTE: the display name is also set in this call.
}

void MTCompose::setInitialRecipient(QString nymId, int contactid/*=0*/, QString address/*=""*/)
{
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


void MTCompose::setInitialMsgType(QString msgtype, QString server/*=""*/)
{
    m_msgtype = server.isEmpty() ? msgtype : QString("otserver");

    if (sendingThroughOTServer())
        setInitialServer(server);
    else // m_msgtype is NOT otserver. Must be bitmessage or something...
        m_serverId = QString("");
}

void MTCompose::setInitialServer(QString serverId)
{
    m_msgtype  = QString("otserver");

    // if serverId is empty, but m_serverId was already set, we don't
    // want to overwrite it with a blank. So we leave m_serverId alone.
    // UNLESS serverId was actually NOT empty, in which case we set
    // m_serverId based on it.
    //
    if (!serverId.isEmpty())
        m_serverId = serverId;

    // These aren't used in the case of OT server, so we blank them out.
    // (Only Server ID and NymIDs are necessary in this case.)
    //
    m_senderAddress    = QString("");
    m_recipientAddress = QString("");
    m_senderMethodId   = 0; // Method #5 might be the sender's Bitmessage connection. Mandatory if msgtype is "bitmessage" (or anything other than otserver.)
}

// ----------------------------------------------------------------------
bool MTCompose::sendMessage(QString body, QString fromNymId, QString toNymId, QString atServerID, QString subject)
{
    if (fromNymId.isEmpty())
    {
        qDebug() << "Cannot send a message from an empty nym id, aborting.";
        return false;
    }
    // ----------------------------------------------------
    if (toNymId.isEmpty())
    {
        qDebug() << "Cannot send a message to an empty nym id, aborting.";
        return false;
    }
    // ----------------------------------------------------
    if (subject.isEmpty())
        subject = tr("From the desktop client. (Empty subject.)");
    // ----------------------------------------------------
    if (body.isEmpty())
        body = tr("From the desktop client. (Empty message body.)");
    // ----------------------------------------------------
    std::string str_serverId  (atServerID.toStdString());
    std::string str_fromNymId (fromNymId.toStdString());
    std::string str_toNymId   (toNymId.toStdString());
    // ----------------------------------------------------
    qDebug() << QString("Initiating sendMessage:\n Server:'%1'\n FromNym:'%2'\n ToNym:'%3'\n Subject:'%4'\n Body:'%5'").
                arg(atServerID).arg(fromNymId).arg(toNymId).arg(subject).arg(body);
    // ----------------------------------------------------
    QString contents = tr("%1: %2\n\n%3").arg(tr("Subject")).arg(subject).arg(body);
    // ----------------------------------------------------
    OT_ME madeEasy;

    std::string strResponse;
    {
        MTSpinner theSpinner;

        strResponse = madeEasy.send_user_msg(str_serverId, str_fromNymId, str_toNymId, contents.toStdString());
    }

    int32_t nReturnVal = madeEasy.VerifyMessageSuccess(strResponse);

    if (1 != nReturnVal)
    {
        qDebug() << "send_message: Failed.";

        Moneychanger::HasUsageCredits(this, str_serverId, str_fromNymId);

        return false;
    }

    qDebug() << "Success in send_message!";
    m_bSent = true;
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
            // OR Address is set. (Either.) Sender NymID is also mandatory.
            //
            if (m_senderNymId.isEmpty())
                return false;
            // --------------------------------
            // Notice we allow the sender address to be empty IF the method ID
            // is available. The presumption is that we can find the sender address
            // later, at send time, based on the method ID. If that fails, it will
            // fail then (not now.)
            //
            if ((m_senderAddress.isEmpty()) && (m_senderMethodId <= 0))
                return false;
            // --------------------------------
            // By this point we know we have one-or-the-other, and possibly both.
            //

            // NOTE: commenting this out here, since the msgtype should be coordinated
            // later between sender AND recipient -- at send time.

//            if (m_senderMethodId > 0)
//            {
//                QString qstrMsgType = MTContactHandler::getInstance()->GetMethodType(m_senderMethodId);

//                if (0 != qstrMsgType.compare(m_msgtype)) // qstrMsgType expected based on senderMethodId doesn't match m_msgtype.
//                {
//                    qDebug() << QString("MTCompose::hasSender: qstrMsgType (%1) expected based on senderMethodId doesn't match m_msgtype (%2.)").
//                                    arg(qstrMsgType).arg(m_msgtype);
//                    return false;
//                }
//                // By this point we know the senderMethodID we have, has the same message type,
//                // such as "bitmessage", that we expected based on m_msgtype.
//            }
//            // ---------------------------------
//            // If there's a sender address, we should make sure it corresponds
//            // to the msg type.
//            //
//            if (!m_senderAddress.isEmpty())
//            {
//                QString qstrAddressMsgType = MTContactHandler::getInstance()->GetMethodType(m_senderAddress);

//                if (0 != qstrAddressMsgType.compare(m_msgtype)) // qstrAddressMsgType expected based on m_senderAddress doesn't match m_msgtype.
//                {
//                    qDebug() << QString("MTCompose::hasSender: qstrAddressMsgType (%1) expected based on m_senderAddress doesn't match m_msgtype (%2.)").
//                                    arg(qstrAddressMsgType).arg(m_msgtype);
//                    return false;
//                }
//            }
            // ---------------------------------
            return true;
        }
    }
    // ------------------------------
    return false;
}

bool MTCompose::hasRecipient()
{
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

                if ((m_recipientContactId > 0) && MTContactHandler::getInstance()->GetAddressesByContact(theMap, m_recipientContactId, m_msgtype))
                    return true;
                else if (!m_recipientNymId.isEmpty() && MTContactHandler::getInstance()->GetAddressesByNym(theMap, m_recipientNymId, m_msgtype))
                    return true;

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
    mapIDName map_recipientMsgMethods;

    // If method is already explicitly set, then we may know about a method that the contact
    // record DOESN'T know about. No need to change anything if it's already set - leave it
    // alone. Therefore, we only do this block if the method ISN'T set. So if it's empty, but
    // Recipient Nym ID is known, try to find a server based on recipient Nym. Pre-select
    // the default server, if it's on that list of known servers for the recipient Nym.
    // But even before all that, see if the Nyms have any other transport methods in common.
    // For example, if Bitmessage is available, then add "Bitmessage" to the list along with
    // the OT servers. If the user chooses Bitmessage, and the recipient exists and has a
    // Bitmessage address, then change recipient to: "recipient, via Bitmessage".
    // If Bitmessage is the selected type, yet the sender nym and/or recipient contact don't
    // have a Bitmessage address, then ask the user to paste or select a Bitmessage address.
    // For sender, from the getAddresses. And for recipient, from bitmessage's getContacts.
    // Then associate those addresses for the future.
    //
    if (m_serverId.isEmpty() && !m_recipientNymId.isEmpty())
    {
        // There are many methods for sending a message. Perhaps it's going
        // through OT. Or perhaps it's going through Bitmessage. Etc.
        //

        int nContactID = MTContactHandler::getInstance()->FindContactIDByNymID (m_recipientNymId);

        //resume

//        bool MTContactHandler::getInstance()->GetMsgMethodsByContact(map_recipientMsgMethods,
//                                                                     nContactID, true);//bAddServers=false by default


//        bool MTContactHandler::getInstance()->GetMsgMethodsByNym(map_recipientMsgMethods,
//                                                                 m_recipientNymId,
//                                                                 true);//bool bAddServers=false by default

        // Get a list of servers, filtered based on recipient Nym ID.
        // If default server is on that list, set it as the default here.
        //
        if (MTContactHandler::getInstance()->GetServers(map_recipientMsgMethods, m_recipientNymId, true))
        {
            the_map = map_recipientMsgMethods;
            // ------------------------------
            // Look up the default.
            if (DBHandler::getInstance()->runQuery("SELECT `server` FROM `default_server` WHERE `default_id`='1' LIMIT 0,1"))
            {
                QString default_server_id = DBHandler::getInstance()->queryString("SELECT `server` FROM `default_server` WHERE `default_id`='1' LIMIT 0,1", 0, 0);

                if (!default_server_id.isEmpty())
                {
                    mapIDName::iterator it_server = the_map.find(default_server_id);

                    // Set the default server from moneychanger as default on this Chooser Dlg.
                    if (it_server != the_map.end())
                        theChooser.SetPreSelected(default_server_id);
                }
            }
        }
    }
    // -----------------------------------------------
    // If we found no servers for that Nym. (Or other transport methods.)
    //
    if (the_map.size() < 1)
        MTContactHandler::getInstance()->GetMsgMethodTypes(the_map, true); //bAddServers=false by default
    // -----------------------------------------------
    theChooser.setWindowTitle(tr("Select Messaging Type"));
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
        qDebug() << QString("SELECT was clicked for messaging via: %1").arg(theChooser.m_qstrCurrentID);

        if (!theChooser.m_qstrCurrentID.isEmpty())
        {
            m_serverId = theChooser.m_qstrCurrentID;
            // -----------------------------------------
            if (theChooser.m_qstrCurrentName.isEmpty())
                ui->viaButton->setText(tr("(blank name)"));
            else
                ui->viaButton->setText(theChooser.m_qstrCurrentName);
            // -----------------------------------------
            return;
        }
    }
    else
    {
      qDebug() << "CANCEL was clicked";
    }
    // -----------------------------------------------
    m_serverId = QString("");
    ui->viaButton->setText(tr("<Click to choose Transport>"));
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
    return (0 == m_msgtype.compare(QString("otserver")));
}

void MTCompose::on_sendButton_clicked()
{
    // Send message and then close dialog. Use progress bar.
    // -----------------------------------------------------------------
    if (!hasRecipient())
    {
        QMessageBox::warning(this, tr("Message Has No Recipient"),
                             tr("Please choose a recipient for this message, before sending."));
        return;
    }
    // -----------------------------------------------------------------
    if (!hasSender())
    {
        QMessageBox::warning(this, tr("Message Has No Sender"),
                             tr("Please choose a sender for this message, before sending."));
        return;
    }
    // -----------------------------------------------------------------
    if (m_msgtype.isEmpty())
    {
        QMessageBox::warning(this, tr("Message Has No Transport"),
                             tr("Before sending, please click 'Via' to choose how it will be sent."));
        return;
    }
    else // msgtype exists... but what type?
    {
        if (sendingThroughOTServer())
        {
            if (m_serverId.isEmpty())
            {
                QMessageBox::warning(this, tr("Message Has No Server ID"),
                                     tr("ERROR: This should never happen. We know the message type is 'otserver', so therefore we should also already know the Server ID by this point."));
                return;
            }
            // todo: else if sender Nym isn't registered on that server -- would you like to?
            // todo: else if recipient isn't known to be on that server -- send it anyway?
            // UPDATE: These todos need to be coded in MakeSureCommonMsgMethod.
        }
        // By this point we know, if the msgtype is 'otserver', that a server ID is available for us
        // to use.
        // What we don't know, yet, is if the type is "bitmessage" (or anything other than "otserver")
        // whether there are addresses available for sender and recipient, and whether they are appropriate
        // based on the msgtype.
        // We also don't know for sure, even if it is "otserver", whether the recipient Nym is actually set.
        // up until this point, a Contact ID is enough to get us here even without the NymID, since the NymID
        // can be resolved based on the Contact ID. But beyond this point, we ACTUALLY need the NymID. So
        // MakeSureCommonMsgMethod() will look that up, too, if we don't already have it.



        // TODO: This is where, if the sender or recipient ADDRESS isn't set,
        // or if the sender METHOD isn't set, that we derive (or ask) to get
        // those things set before the actual send.


        //resume

        // The below call is what insures all the addresses are in order.
        // When necessary, it will pop up a chooser dialog, or an "Are you sure?"
        // dialog, in order to gather the appropriate information.
        // If successful setting everything such that we could actually
        // send, it returns true. But if this function finishes in a state
        // where we do not have the necessary info to send, it will return false.
        //
        if (!MakeSureCommonMsgMethod())
            return false;








    }
    // -----------------------------------------------------------------
    if (ui->subjectEdit->text().isEmpty())
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
    QString viaServer    = m_serverId;
    QString viaTransport = m_msgtype;

    bool bSent = this->sendMessage(body, fromNymId, toNymId, viaServer, subject);
    // -----------------------------------------------------------------
    if (!bSent)
        QMessageBox::warning(this, tr("Failed Sending Message"),
                             tr("Failed trying to send the message."));
    else
    {
        QMessageBox::StandardButton info_btn =
                QMessageBox::information(this, tr("Success"), tr("Success sending message."));
        // -----------------------------------------------------------------
        emit balancesChanged(); // So we'll see the sent message in the transaction history
        // -----------------------------------------------------------------
    }
    // -----------------------------------------------------------------
}

// This insures that the sender and recipient nym are using the same Msg Method.
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
    // If we have both Nym IDs... (sender / recipient.)
    // That's the presupposition of this entire function.
    //
    // Either we have both IDs, and no m_method, or we have
    // both IDs, and we DO have m_method. Either way, we have
    // both IDs.
    //
    if (!hasSenderAndRecipient())
    {
        qDebug() << "MakeSureCommonMsgMethod was just called even though sender (or recipient) isn't set.";
        return false;
    }
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
        bool      bNeedToChooseNym = false;
        mapIDName theMap;

        if (sendingThroughOTServer())
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
                                         tr("Sorry, recipient '%1' has no known NymIDs (to send a message to.)").arg(qstrContactName));
                    return false;
                }
                else if (1 == theMap.size())
                {
                    mapIDName::iterator it = theMap.begin();

                    QString qstrNymID       = it.key();
//                  QString qstrNymName     = it.value();

                    this->setInitialRecipient(qstrNymID, m_recipientContactId, m_recipientAddress);
                }
                else // There are multiple Nyms available to choose from, for m_recipientContactId.
                    bNeedToChooseNym = true;
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
                else if (1 == theMap.size())
                {
                    mapIDName::iterator it = theMap.begin();

                    QString qstrNymID       = it.key();
//                  QString qstrNymName     = it.value();

                    this->setInitialRecipient(qstrNymID, m_recipientContactId, m_recipientAddress);
                }
                else // There are multiple Nyms available to choose from, for m_recipientContactId.
                    bNeedToChooseNym = true;
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
        // ----------------------------------------------
        // Select from Nyms in local wallet.
        //
        if (bNeedToChooseNym)
        {
            DlgChooser theChooser(this);
            // -----------------------------------------------
            theChooser.m_map = theMap;
            // -----------------------------------------------
            theChooser.setWindowTitle(tr("Choose Recipient Nym"));
            // -----------------------------------------------
            if (theChooser.exec() == QDialog::Accepted)
            {
                qDebug() << QString("SELECT was clicked for NymID: %1").arg(theChooser.m_qstrCurrentID);

                if (!theChooser.m_qstrCurrentID.isEmpty())
                    this->setInitialRecipient(theChooser.m_qstrCurrentID, m_recipientContactId, m_recipientAddress);
                else
                    return false;
            }
        }
    } // if recipientNymID.isEmpty()
    // -------------------------------------------
    // Below this block, we will know for a fact that either a Nym is available
    // if needed or wanted, or that if a Nym is not available, that he's not needed
    // and not wanted.
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
            m_msgtype = qstrMethodTypeSender;
        // -------------------------------------------------------------------------
        else if (!qstrMethodTypeSender.isEmpty() && !qstrMethodTypeRecipient.isEmpty())
        {
         //resume
        }
    }
    // -------------------------------------------
    // If we have no method, then we just pick one that the
    // sender/recipient have in common.
    //
    // Whereas if we DO have a method, then we make sure the sender
    // and recipient are compatible with it. This is where we know
    // we have chosen the right methodID for sender and address
    // for recipient. If method type is "otserver|SERVER_ID" then
    // we see if both are known to be on that server. Whereas if
    // the type is "bitmessage" then we look up the methodID and
    // address appropriately.
    //
    if (m_msgtype.isEmpty())
    {
//        QString m_msgtype;            // Will be "otserver" or "bitmessage" etc.
//        QString m_serverId;           // If msgtype is "otserver" then the OT Server ID must be set here. Otherwise should be blank.

//        QString m_senderNymId;        // Sender NymID should always be available.
//        int     m_senderMethodId;     // Method #5 might be the sender's Bitmessage connection. Mandatory if msgtype is "bitmessage" (or anything other than otserver.)
//        QString m_senderAddress;      // If msgtype is "bitmessage" this will contain a Bitmessage address.

//        QString m_recipientNymId;     // If not available, then m_recipientContactID must be available. (For Bitmessage, for example, Nym is optional.)
//        int     m_recipientContactId; // Recipient Nym kjsdfds982345 might be Contact #2. (Or Nym itself might be blank, with ONLY Contact!)
//        QString m_recipientAddress;   // If msgtype is "bitmessage" this will contain a Bitmessage address.



        // LOGIC:
        //
        // if m_serverId is set, and senderNymId is set (which it definitely is, by this point), and recipientNymId is set,
        // then set m_msgtype to "otserver."
        //
        // Else if all that is true but we are missing recipientNymID, and since we already know there is not going to BE a
        // RecipientNymID (period -- see blocks higher up), then we know for a FACT the msgtype can NOT be "otserver".
        // THEREFORE we loop through the types available in MTComms and find one that works for both parties.
        //
        // HOWEVER, what if both already have an ADDRESS / METHOD ID set? In that case we don't have to loop, since it's already
        // set. In that case though, we should have checked that before we even entered this block, and set the msgtype by now
        // already. Therefore I will add that now, above this block.
        //


        mapOfCommTypes mapTypes;
        bool bGotCommTypes = MTComms::types(mapOfCommTypes & mapTypes);


        QString qstrMsgTypeAttempt = QString("bitmessage");


        bool MTCompose::CheckPotentialCommonMsgMethod(QString qstrMsgTypeAttempt, mapIDName * pmapSenderAddresses/*=NULL*/, mapIDName * pmapRecipientAddresses/*=NULL*/)








//      bool GetMsgMethodsByNym        (mapIDName & theMap, QString filterByNym,  bool bAddServers=false); // Methods.
//      bool GetMsgMethodTypesByContact(mapIDName & theMap, int nFilterByContact, bool bAddServers=false); // Method Types.


        int nContactID = MTContactHandler::getInstance()->FindContactIDByNymID (m_recipientNymId);

        if (bRecipientTypeFoundOnNym && (nContactID > 0))
        {
            // If we have no method, then we just pick one that the
            // sender/recipient have in common.
            //
            // First we see if they both have Bitmessage, and if not,
            // see if they have any non-OT-server options in common.
            // If not, see if they have the default server in common.
            // If not, see if they have any server in common.
            // If not, return false.
            //
            // (Otherwise below this block we know we have a method chosen.)
            //
            if ( MTContactHandler::getInstance()->MethodTypeFoundOnNym    (QString("bitmessage"), m_senderNymId) &&
                 MTContactHandler::getInstance()->MethodTypeFoundOnContact(QString("bitmessage"), nContactID))
            {

            }
        } // if nContactID > 0


//      bool MTContactHandler::getInstance()->MethodTypeFoundOnNym    (QString method_type, QString filterByNym);
//      bool MTContactHandler::getInstance()->MethodTypeFoundOnContact(QString method_type, int nFilterByContact);

    }
    // -----------------------------------------
    // Whereas if we DO have a method, then we make sure the sender
    // and recipient are compatible with it. This is where we know
    // we have chosen the right methodID for sender and address
    // for recipient. If method type is "otserver|SERVER_ID" then
    // we see if both are known to be on that server. Whereas if
    // the type is "bitmessage" then we look up the methodID and
    // address appropriately.
    //
    if (!m_msgtype.isEmpty())
    {
        // Whereas if we DO have a method, then we make sure the
        // sender and recipient are compatible. This is where we know
        // we have chosen the right methodID for sender and address
        // for recipient. If method type is "otserver|SERVER_ID" then
        // we see if both are known to be on that server. Whereas if
        // the type is "bitmessage" then we look up the methodID and
        // address appropriately.
        //



//       bool MTContactHandler::getInstance()->MethodTypeFoundOnNym    (QString method_type, QString filterByNym);
//       bool MTContactHandler::getInstance()->MethodTypeFoundOnContact(QString method_type, int nFilterByContact);


    }
    // -----------------------------------------

    int nContactID = MTContactHandler::getInstance()->FindContactIDByNymID (m_recipientNymId);

    if (nContactID <= 0)
    {
        qDebug() << "MTCompose::MakeSureCommonMsgMethod: Unable to find contact ID based on recipient nym ID.";
        // Perhaps TODO, msgbox: "No contact found for recipient nym. Would you like to add one?"
    }
    else //nContactID > 0
    {
        mapIDName map_senderMsgMethods;
        mapIDName map_recipientMsgMethods;

//        bool bGotNymMethods     = MTContactHandler::getInstance()->GetMsgMethodsByNym(map_senderMsgMethods,
//                                                                                      m_senderNymId,
//                                                                                      true);//bool bAddServers=false by default

//        bool bGotContactMethods = MTContactHandler::getInstance()->GetMsgMethodsByContact(map_recipientMsgMethods,
//                                                                                          nContactID,
//                                                                                          true);//bAddServers=false by default
//        if (!bGotNymMethods)
//        {
//            qDebug() << "MTCompose::MakeSureCommonMsgMethod: Can find ANY messaging methods for sender Nym.";
//            // Perhaps todo: msgbox.
//            return false;
//        }
//        else if (!bGotContactMethods)
//        {
//            qDebug() << "MTCompose::MakeSureCommonMsgMethod: Can find ANY messaging methods for recipient Nym.";
//            // Perhaps todo: msgbox.
//            return false;
//        }
        // By this point we know there's a contact assigned to the recipient.
        // We also know that we were able to find messaging methods for the sender
        // AND for the recipient.
        // But what we don't know yet is, if they have any in COMMON.
        // -------------------------------------------
        // If a method is already set, then make sure sender and recipient both
        // support it.
        //
        if (!m_msgtype.isEmpty())
        {

            //resume




        }
        // Else if a method is NOT already set, then find one in common between sender
        // and recipient.
        //
        else // m_method is empty.
        {

        }
    } // if (nContactID > 0)









    // BELOW SAVED FROM PREVIOUS FUNCTION JUST IN CASE

    m_msgtype = qstrMsgTypeAttempt;

    // By this point, we KNOW both sender and recipient have ability to transport
    // via qstrMsgTypeAttempt ("bitmessage" or whatever.) Let's see if they already
    // have addresses set, in which case we'll stick with the ones already set.
    //
    if (!m_recipientAddress.isEmpty())
    {
        QString qstrMethodType = MTContactHandler::getInstance()->GetMethodType(m_recipientAddress);

        if (!qstrMethodType.isEmpty())
        {
            // Let's make sure whether the types match.
            //
            if (0 != qstrMethodType.compare(qstrMsgTypeAttempt))
            {
                // This means a type was found for the address, but it's different than
                // the type we just set as m_msgtype.
            }
        }
        else
        {
            // NOTE: Just because we didn't find the method type for a given address,
            // doesn't mean the address isn't of that type! We may have received a message
            // from someone who we haven't yet added to our own address book.
            //
            //
        }
    }

    if (!m_senderAddress.isEmpty())
    {

    }

}

// TODO: when sender is selected, and IF recipient is already selected,
// try to find a common communication method between them. If currently
// selected is already in common -- fine. Otherwise choose one if in
// common.
void MTCompose::on_fromButton_clicked()
{
    // Select from Nyms in local wallet.
    //
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;

    bool bFoundDefault = false;
    // -----------------------------------------------
    const int32_t nym_count = OTAPI_Wrap::It()->GetNymCount();
    // -----------------------------------------------
    for (int32_t ii = 0; ii < nym_count; ++ii)
    {
        //Get OT Nym ID
        QString OT_nym_id = QString::fromStdString(OTAPI_Wrap::It()->GetNym_ID(ii));
        QString OT_nym_name("");
        // -----------------------------------------------
        if (!OT_nym_id.isEmpty())
        {
            if (!m_senderNymId.isEmpty() && (OT_nym_id == m_senderNymId))
                bFoundDefault = true;
            // -----------------------------------------------
            MTNameLookupQT theLookup;

            OT_nym_name = QString::fromStdString(theLookup.GetNymName(OT_nym_id.toStdString()));
            // -----------------------------------------------
            the_map.insert(OT_nym_id, OT_nym_name);
        }
     }
    // -----------------------------------------------
    if (bFoundDefault && !m_senderNymId.isEmpty())
        theChooser.SetPreSelected(m_senderNymId);
    // -----------------------------------------------
    theChooser.setWindowTitle(tr("Choose your Sender identity"));
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
        qDebug() << QString("SELECT was clicked for NymID: %1").arg(theChooser.m_qstrCurrentID);

        if (!theChooser.m_qstrCurrentID.isEmpty())
        {
            m_senderNymId = theChooser.m_qstrCurrentID;
            // -----------------------------------------
            if (theChooser.m_qstrCurrentName.isEmpty())
                ui->fromButton->setText(QString(""));
            else
                ui->fromButton->setText(theChooser.m_qstrCurrentName);
            // -----------------------------------------






            return;
        }
    }
    else
    {
      qDebug() << "CANCEL was clicked";
    }
    // -----------------------------------------------
    m_senderNymId = QString("");
    ui->fromButton->setText(tr("<Click to choose Sender>"));
}


// TODO: when recipient is selected and IF sender is already selected,
// try to find a communication method in common between them, and if
// current method is in common, keep it selected. Perhaps if the initial
// method is "otserver|SERVER_ID" we set it that way to start, but if
// instead the method was initially blank, and if both sender and recip-
// ient have bitmessage, then perhaps prefer that even over an OT server
// they have in common such as the default OT server. Basically if the
// initial method is blank, prefer bitmessage, and if that's not found,
// then prefer the default server if it exists on a server list in common
// between them. And if it doesn't then find SOME server in common
// between them.
void MTCompose::on_toButton_clicked()
{
    // Select recipient from the address book and convert to Nym ID.
    // -----------------------------------------------
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;

    MTContactHandler::getInstance()->GetContacts(the_map);

    // Set Chooser's default to recipient nym's Contact, if nym is
    // there and Contact is found based on it.
    if (!m_recipientNymId.isEmpty())
    {
        int nContactID = MTContactHandler::getInstance()->FindContactIDByNymID(m_recipientNymId);

        if (nContactID > 0)
        {
            QString strTempID = QString("%1").arg(nContactID);
            theChooser.SetPreSelected(strTempID);
        }
    }
    // -----------------------------------------------
    theChooser.setWindowTitle(tr("Choose the Recipient"));
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
        qDebug() << QString("SELECT was clicked for ID: %1").arg(theChooser.m_qstrCurrentID);

        // If not the same as before, then we have to choose a NymID based on the selected Contact.
        //
        int nSelectedContactID = theChooser.m_qstrCurrentID.toInt();
        int nOldNymContactID   = MTContactHandler::getInstance()->FindContactIDByNymID(m_recipientNymId);

        // If they had matched, then we could have kept m_recipientNymId as it was.
        // But since they are different, we have to figure out a NymID to use, based
        // on nSelectedContactID.
        //
        if (nSelectedContactID != nOldNymContactID)
        {
            QString qstrContactName;

            if (nSelectedContactID <= 0) // Should never happen.
            {
                qstrContactName  = QString("");
                m_recipientNymId = QString("");
                ui->toButton->setText(tr("<Click to choose Recipient>"));
                return;
            }
            // else...
            //
            qstrContactName = MTContactHandler::getInstance()->GetContactName(nSelectedContactID);

            if (qstrContactName.isEmpty())
                ui->toButton->setText(tr("(Contact has a blank name)"));
            else
                ui->toButton->setText(qstrContactName);
            // ---------------------------------------------
            // Next we need to find a Nym based on this Contact...
            //
            mapIDName theNymMap;

            if (MTContactHandler::getInstance()->GetNyms(theNymMap, nSelectedContactID))
            {
                if (theNymMap.size() == 0)
                {
                    // TODO!

                    // Perhaps there are no Nyms for the recipient.
                    // (Perhaps there is a Bitmessage address, however.)
                    // In that case we'd end up in this block.
                }
                else if (theNymMap.size() == 1)
                {
                    mapIDName::iterator theNymIt = theNymMap.begin();

                    if (theNymIt != theNymMap.end())
                    {
                        QString qstrNymID   = theNymIt.key();
                        QString qstrNymName = theNymIt.value();

                        m_recipientNymId = qstrNymID;
                    }
                    else // Should never happen, since map size is 1.
                    {
                        m_recipientNymId = QString("");
                        ui->toButton->setText(tr("<Click to choose Recipient>"));
                        // -------------------------------------
                        QMessageBox::warning(this, tr("Contact has no known identities"),
                                             tr("Sorry, Contact '%1' has no known NymIDs (to send messages to.)").arg(qstrContactName));
                        return;
                    }
                }
                else // There are multiple Nyms to choose from.
                {
                    DlgChooser theNymChooser(this);
                    theNymChooser.m_map = theNymMap;
                    theNymChooser.setWindowTitle(tr("Recipient has multiple Nyms. (Please choose one.)"));
                    // -----------------------------------------------
                    if (theNymChooser.exec() == QDialog::Accepted)
                        m_recipientNymId = theNymChooser.m_qstrCurrentID;
                    else // User must have cancelled.
                    {
                        m_recipientNymId = QString("");
                        ui->toButton->setText(tr("<Click to choose Recipient>"));
                    }
                }
            }
            else // No nyms found for this ContactID.
            {
                m_recipientNymId = QString("");
                ui->toButton->setText(tr("<Click to choose Recipient>"));
                // -------------------------------------
                QMessageBox::warning(this, tr("Contact has no known identities"),
                                     tr("Sorry, Contact '%1' has no known NymIDs (to send messages to.)").arg(qstrContactName));
                return;
            }
            // --------------------------------
        }
    }
    else
    {
      qDebug() << "CANCEL was clicked";
    }
    // -----------------------------------------------
}




void MTCompose::dialog()
{
/** Compose Dialog **/

    if (!already_init)
    {
        connect(this,               SIGNAL(balancesChanged()),
                Moneychanger::It(), SLOT  (onBalancesChanged()));
        // ---------------------------------------
        this->setWindowTitle(tr("Compose: (no subject)"));

        QString style_sheet = "QPushButton{border: none; border-style: outset; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa);}"
                "QPushButton:pressed {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }"
                "QPushButton:hover {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }";

        ui->fromButton->setStyleSheet(style_sheet);
        ui->toButton->setStyleSheet(style_sheet);
        ui->viaButton->setStyleSheet(style_sheet);

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



        // Todo.

        // -------------------------------------------
        std::string str_sender_name;
        // -------------------------------------------
        if (!m_senderNymId.isEmpty()) // senderNym was provided.
        {
            MTNameLookupQT theLookup;

            str_sender_name = theLookup.GetNymName(m_senderNymId.toStdString());

            if (str_sender_name.empty())
                str_sender_name = m_senderNymId.toStdString();
        }
        // -------------------------------------------
        if (str_sender_name.empty())
        {
            m_senderNymId = QString("");
            ui->fromButton->setText(tr("<Click to choose Sender>"));
        }
        else
            ui->fromButton->setText(QString::fromStdString(str_sender_name));
        // -------------------------------------------


        // -------------------------------------------
        std::string str_recipient_name;
        // -------------------------------------------
        if (!m_recipientNymId.isEmpty()) // recipientNym was provided.
        {
            MTNameLookupQT theLookup;

            str_recipient_name = theLookup.GetNymName(m_recipientNymId.toStdString());

            if (str_recipient_name.empty())
                str_recipient_name = m_recipientNymId.toStdString();
        }
        // -------------------------------------------
        if (str_recipient_name.empty())
        {
            m_recipientNymId = QString("");
            ui->toButton->setText(tr("<Click to choose Recipient>"));
        }
        else
            ui->toButton->setText(QString::fromStdString(str_recipient_name));
        // -------------------------------------------


        // -------------------------------------------
        std::string str_server_name;
        // -------------------------------------------
        if (!m_serverId.isEmpty()) // serverID was provided.
        {
            str_server_name = OTAPI_Wrap::It()->GetServer_Name(m_serverId.toStdString());

            if (str_server_name.empty())
                str_server_name = m_serverId.toStdString();
        }
        // -------------------------------------------
        if (str_server_name.empty())
        {
            m_serverId = QString("");
            ui->viaButton->setText(tr("<Click to choose Method>"));
        }
        else
            ui->viaButton->setText(QString::fromStdString(str_server_name));
        // -------------------------------------------




        // -------------------------------------------
        if (!m_subject.isEmpty())
        {
            QString qstrRe = tr("re:");

            QString qstrSubjectLeft = m_subject.left(qstrRe.size());

            if (qstrSubjectLeft.toLower() != qstrRe)
            {
                QString strTemp = QString("%1 %2").arg(qstrRe).arg(m_subject);
                m_subject = strTemp;
            }
            // -----------------------
            QString qstrTempSubject = m_subject;

            ui->subjectEdit->setText(qstrTempSubject);
            // -----------------------
            this->setWindowTitle(QString("%1: %2").arg(tr("Compose")).arg(qstrTempSubject));
        }
        // -------------------------------------------


        ui->contentsEdit->setFocus();


        /** Flag Already Init **/
        already_init = true;
    }

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
}

MTCompose::~MTCompose()
{
    delete ui;
}

void MTCompose::on_subjectEdit_textChanged(const QString &arg1)
{
    if (arg1.isEmpty())
    {
        m_subject = QString("");
        this->setWindowTitle(tr("Compose: (no subject)"));
    }
    else
    {
        m_subject = arg1;
        this->setWindowTitle(QString("%1: %2").arg("Compose").arg(arg1));
    }
}
