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


void MTCompose::on_sendButton_clicked()
{
    // Send message and then close dialog. Use progress bar.
    // -----------------------------------------------------------------
    if (m_recipientNymId.isEmpty())
    {
        QMessageBox::warning(this, tr("Message Has No Recipient"),
                             tr("Please choose a recipient for this message, before sending."));
        return;
    }
    // -----------------------------------------------------------------
    if (m_senderNymId.isEmpty())
    {
        QMessageBox::warning(this, tr("Message Has No Sender"),
                             tr("Please choose a sender for this message, before sending."));
        return;
    }
    // -----------------------------------------------------------------
    if (m_serverId.isEmpty())
    {
        QMessageBox::warning(this, tr("Message Has No Server"),
                             tr("Before sending, please choose a server for this message to be sent through."));
        return;
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
    QString subject     = ui->subjectEdit->text();
    QString body        = ui->contentsEdit->toPlainText();
    QString fromNymId   = m_senderNymId;
    QString toNymId     = m_recipientNymId;
    QString atServerID  = m_serverId;

    bool bSent = this->sendMessage(body, fromNymId, toNymId, atServerID, subject);
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


void MTCompose::onBalancesChanged()
{
    this->close();
}



void MTCompose::on_serverButton_clicked()
{
    // Select from servers in local wallet filtered by both Nyms.
    //
    DlgChooser theChooser(this);
    mapIDName & the_map = theChooser.m_map;
    // -----------------------------------------------
    mapIDName map_recipientServers;

    // If server ID is explicitly set, then we may know about a server that the contact
    // record DOESN'T know about. Therefore we only do this block if the server ID ISN'T
    // set (where we filter it based on the known servers for a given Nym.)
    //
    if (m_serverId.isEmpty() && !m_recipientNymId.isEmpty())
    {
        if (MTContactHandler::getInstance()->GetServers(map_recipientServers, m_recipientNymId))
        {
            the_map = map_recipientServers;
            // ------------------------------
            if (DBHandler::getInstance()->runQuery("SELECT `server` FROM `default_server` WHERE `default_id`='1' LIMIT 0,1"))
            {
                QString default_server_id = DBHandler::getInstance()->queryString("SELECT `server` FROM `default_server` WHERE `default_id`='1' LIMIT 0,1", 0, 0);

                if (!default_server_id.isEmpty())
                {
                    mapIDName::iterator it_server = the_map.find(default_server_id);

                    if (it_server != the_map.end())
                        theChooser.SetPreSelected(default_server_id);
                }
            }
        }
    }
    // -----------------------------------------------
    if (the_map.size() < 1)
    {
        const int32_t server_count = OTAPI_Wrap::It()->GetServerCount();
        // -----------------------------------------------
        for (int32_t ii = 0; ii < server_count; ++ii)
        {
            //Get OT Server ID
            //
            QString OT_server_id = QString::fromStdString(OTAPI_Wrap::It()->GetServer_ID(ii));
            QString OT_server_name("");
            // -----------------------------------------------
            if (!OT_server_id.isEmpty())
            {
                if (!m_serverId.isEmpty() && (OT_server_id == m_serverId))
                    theChooser.SetPreSelected(m_serverId);
                // -----------------------------------------------
                OT_server_name = QString::fromStdString(OTAPI_Wrap::It()->GetServer_Name(OT_server_id.toStdString()));
                // -----------------------------------------------
                the_map.insert(OT_server_id, OT_server_name);
            }
        }
    }
    // -----------------------------------------------
    theChooser.setWindowTitle("Select an OT Server");
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
        qDebug() << QString("SELECT was clicked for ServerID: %1").arg(theChooser.m_qstrCurrentID);

        if (!theChooser.m_qstrCurrentID.isEmpty())
        {
            m_serverId = theChooser.m_qstrCurrentID;
            // -----------------------------------------
            if (theChooser.m_qstrCurrentName.isEmpty())
                ui->serverButton->setText(tr("(This server has a blank name)"));
            else
                ui->serverButton->setText(theChooser.m_qstrCurrentName);
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
    ui->serverButton->setText(tr("<Click to choose an OT Server>"));
}

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



void MTCompose::on_toButton_clicked()
{
    // Select recipient from the address book and convert to Nym ID.
    // -----------------------------------------------
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;

    MTContactHandler::getInstance()->GetContacts(the_map);

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
                if (theNymMap.size() == 1)
                {
                    mapIDName::iterator theNymIt = theNymMap.begin();

                    if (theNymIt != theNymMap.end())
                    {
                        QString qstrNymID   = theNymIt.key();
                        QString qstrNymName = theNymIt.value();

                        m_recipientNymId = qstrNymID;
                    }
                    else
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
        ui->serverButton->setStyleSheet(style_sheet);

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
            ui->serverButton->setText(tr("<Click to choose Server>"));
        }
        else
            ui->serverButton->setText(QString::fromStdString(str_server_name));
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
