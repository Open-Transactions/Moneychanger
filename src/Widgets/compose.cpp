#include <QDebug>
#include <QMessageBox>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>
#include <opentxs/OTLog.h>

#include "compose.h"
#include "ui_compose.h"

#include "dlgchooser.h"

#include "Handlers/contacthandler.h"



void MTCompose::on_fromButton_clicked()
{
    // Select from Nyms in local wallet.

    // -----------------------------------------------
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;





    the_map.insert(QString("ID1"), QString("NAME1"));
    the_map.insert(QString("ID2"), QString("NAME2"));
    the_map.insert(QString("ID3"), QString("NAME3"));
    the_map.insert(QString("ID4"), QString("NAME4"));
    the_map.insert(QString("ID5"), QString("NAME5"));
    the_map.insert(QString("ID6"), QString("NAME6"));
    // -----------------------------------------------
    theChooser.SetPreSelected("ID4");
    // -----------------------------------------------
    theChooser.setWindowTitle("Choose the Sender Nym");
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted) {
        qDebug() << QString("SELECT was clicked for ID: %1").arg(theChooser.m_qstrCurrentID);
    } else {
      qDebug() << "CANCEL was clicked";
    }
    // -----------------------------------------------


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
    theChooser.setWindowTitle("Choose the Recipient");
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
                ui->toButton->setText("<Click to Choose Recipient>");
                return;
            }
            // else...
            //
            qstrContactName = MTContactHandler::getInstance()->GetContactName(nSelectedContactID);

            if (qstrContactName.isEmpty())
                ui->toButton->setText("(Contact has a blank name)");
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
                        ui->toButton->setText("<Click to Choose Recipient>");
                        // -------------------------------------
                        QMessageBox::warning(this, QString("Contact has no known identities"),
                                             QString("Sorry, Contact '%1' has no known NymIDs (to send messages to.)").arg(qstrContactName));
                        return;
                    }
                }
                else // There are multiple Nyms to choose from.
                {
                    DlgChooser theNymChooser(this);
                    theNymChooser.m_map = theNymMap;
                    theNymChooser.setWindowTitle("Recipient has multiple Nyms. (Please choose one.)");
                    // -----------------------------------------------
                    if (theNymChooser.exec() == QDialog::Accepted)
                        m_recipientNymId = theNymChooser.m_qstrCurrentID;
                    else // User must have cancelled.
                    {
                        m_recipientNymId = QString("");
                        ui->toButton->setText("<Click to Choose Recipient>");
                    }
                }
            }
            else // No nyms found for this ContactID.
            {
                m_recipientNymId = QString("");
                ui->toButton->setText("<Click to Choose Recipient>");
                // -------------------------------------
                QMessageBox::warning(this, QString("Contact has no known identities"),
                                     QString("Sorry, Contact '%1' has no known NymIDs (to send messages to.)").arg(qstrContactName));
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

void MTCompose::on_serverButton_clicked()
{
    // Select from servers in local wallet filtered by both Nyms.
}

void MTCompose::on_sendButton_clicked()
{
    // Send message and then close dialog. Use progress bar.

    if (m_recipientNymId.isEmpty())
    {
        QMessageBox::warning(this, QString("Message Has No Recipient"),
                             QString("Please choose a recipient for this message, before sending."));
        return;
    }
    // -----------------------------------------------------------------

    // TODO: Actually send the message here.

}

void MTCompose::dialog()
{
/** Compose Dialog **/

    if (!already_init)
    {
        this->setWindowTitle("Compose: (no subject)");

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
            ui->fromButton->setText(QString("<Click to Choose Sender>"));
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
            ui->toButton->setText("<Click to Choose Recipient>");
        }
        else
            ui->toButton->setText(QString::fromStdString(str_recipient_name));
        // -------------------------------------------


        // -------------------------------------------
        std::string str_server_name;
        // -------------------------------------------
        if (!m_serverId.isEmpty()) // serverID was provided.
        {
            str_server_name = OTAPI_Wrap::GetServer_Name(m_serverId.toStdString());

            if (str_server_name.empty())
                str_server_name = m_serverId.toStdString();
        }
        // -------------------------------------------
        if (str_server_name.empty())
        {
            m_serverId = QString("");
            ui->serverButton->setText(QString("<Click to Choose Server>"));
        }
        else
            ui->serverButton->setText(QString::fromStdString(str_server_name));
        // -------------------------------------------




        // -------------------------------------------
        if (!m_subject.isEmpty())
        {
            QString qstrRe = m_subject.left(4);

            if (qstrRe.toLower() != QString("re: "))
            {
                QString strTemp = QString("re: %1").arg(m_subject);
                m_subject = strTemp;
            }
            // -----------------------
            QString qstrTempSubject = m_subject;

            ui->subjectEdit->setText(qstrTempSubject);
        }
        // -------------------------------------------




        /** Flag Already Init **/
        already_init = true;
    }

}

void MTCompose::closeEvent(QCloseEvent *event)
{
    // Pop up a Yes/No dialog to confirm the cancellation of this message.
    // (ONLY if the subject or contents fields contain text.)
    //
    if (!ui->contentsEdit->toPlainText().isEmpty() || !ui->subjectEdit->text().isEmpty())
    {
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, "", "Are you sure you want to cancel this message?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply != QMessageBox::Yes) {
          event->ignore();
          return;
        }
    }
    // -------------------------------------------
    QWidget::closeEvent(event);
}

MTCompose::MTCompose(QWidget *parent) :
    QWidget(parent, Qt::Window),
    already_init(false),
    ui(new Ui::MTCompose)
{
    ui->setupUi(this);

    this->installEventFilter(this);
}

MTCompose::~MTCompose()
{
    delete ui;
}

bool MTCompose::eventFilter(QObject *obj, QEvent *event){

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape){
            close(); // This is caught by this same filter.
            return true;
        }
        return true;
    }
    else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

void MTCompose::on_subjectEdit_textChanged(const QString &arg1)
{
    if (arg1.isEmpty())
    {
        m_subject = QString("");
        this->setWindowTitle(QString("Compose: (no subject)"));
    }
    else
    {
        m_subject = arg1;
        this->setWindowTitle(QString("Compose: %1").arg(arg1));
    }
}
