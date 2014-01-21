#ifndef COMPOSE_H
#define COMPOSE_H

#include <QWidget>

namespace Ui {
class MTCompose;
}

class MTCompose : public QWidget
{
    Q_OBJECT
    
    QString m_senderNymId;
    QString m_recipientNymId;
    QString m_serverId;
    QString m_subject;

    bool m_bSent;

public:
    explicit MTCompose(QWidget *parent = 0);
    ~MTCompose();
    
    void setInitialSenderNym(QString nymId)    { m_senderNymId    = nymId;    }
    void setInitialRecipientNym(QString nymId) { m_recipientNymId = nymId;    }
    void setInitialServer(QString serverId)    { m_serverId       = serverId; }
    void setInitialSubject(QString subject)    { m_subject        = subject;  }

    void dialog();

    bool sendMessage(QString body, QString fromNymId, QString toNymId, QString atServerID, QString subject);

signals:
    void balancesChanged();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void closeEvent(QCloseEvent *event);

private slots:
    void onBalancesChanged();

    void on_toButton_clicked();
    void on_fromButton_clicked();
    void on_serverButton_clicked();
    void on_sendButton_clicked();

    void on_subjectEdit_textChanged(const QString &arg1);

private:
    bool already_init;

    Ui::MTCompose *ui;
};

#endif // COMPOSE_H
