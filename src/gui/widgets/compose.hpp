#ifndef COMPOSE_HPP
#define COMPOSE_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <QWidget>

namespace Ui {
class MTCompose;
}

class mapIDName;

class MTCompose : public QWidget
{
    Q_OBJECT
    
    QString m_subject;            // Message subject line.
    QString m_msgtype;            // Will be "otserver" or "bitmessage" etc.
    QString m_serverId;           // If msgtype is "otserver" then the OT Server ID must be set here.

    QString m_senderNymId;        // Sender NymID should always be available.
    int     m_senderMethodId;     // Method #5 might be the sender's Bitmessage connection. Mandatory if msgtype is "bitmessage" (or anything other than otserver.)
    QString m_senderAddress;      // If msgtype is "bitmessage" this will contain a Bitmessage address.

    QString m_recipientNymId;     // If not available, then m_recipientContactID must be available. (For Bitmessage, for example, Nym is optional.)
    int     m_recipientContactId; // Recipient Nym kjsdfds982345 might be Contact #2. (Or Nym itself might be blank, with ONLY Contact!)
    QString m_recipientAddress;   // If msgtype is "bitmessage" this will contain a Bitmessage address.

    bool    m_bSent;

public:
    explicit MTCompose(QWidget *parent = 0);
    ~MTCompose();
    
    bool hasSender();              // Determines if a proper sender is available based on msgtype.
    bool hasRecipient();           // Determines if a proper recipient is available based on msgtype.
    bool hasSenderAndRecipient();  // Determines if a proper sender and recipient are available based on msgtype.

    bool sendingThroughOTServer(); // Checks m_msgtype to see if it is set to "otserver".

    void setInitialSubject(QString subject);
    void setInitialMsgType(QString msgtype, QString server="");
    void setInitialServer(QString serverId);
    void setInitialSenderNym(QString nymId, QString address="");
    void setInitialSenderAddress(QString address);
    void setInitialRecipient(QString nymId, int contactid=0, QString address="");
    void setInitialRecipientNym(QString nymId, QString address="");
    void setInitialRecipientAddress(QString address);
    void setInitialRecipientContactID(int contactid, QString address="");

    void setSenderNameBasedOnAvailableData();
    void setRecipientNameBasedOnAvailableData();

    void dialog();

    bool sendMessage(QString body, QString fromNymId, QString toNymId, QString viaMethod, QString subject);

    // This insures that the sender and recipient nym are using the same Msg Method.
    // Which is to say, the same OT server, or both using Bitmessage, etc.
    //
    bool MakeSureCommonMsgMethod();
    bool CheckPotentialCommonMsgMethod(QString qstrMsgTypeAttempt, mapIDName * pmapSenderAddresses=NULL, mapIDName * pmapRecipientAddresses=NULL);

signals:
    void balancesChanged();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void closeEvent(QCloseEvent *event);

private slots:
    void onBalancesChanged();

    void on_toButton_clicked();
    void on_fromButton_clicked();
    void on_viaButton_clicked();
    void on_sendButton_clicked();

    void on_subjectEdit_textChanged(const QString &arg1);

private:
    bool already_init;

    Ui::MTCompose *ui;
};

#endif // COMPOSE_HPP
