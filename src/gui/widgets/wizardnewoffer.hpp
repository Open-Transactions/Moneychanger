#ifndef WIZARDNEWOFFER_HPP
#define WIZARDNEWOFFER_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <QWizard>

namespace Ui {
class WizardNewOffer;
}

class WizardNewOffer : public QWizard
{
    Q_OBJECT

public:
    explicit WizardNewOffer(QWidget *parent = 0);
    ~WizardNewOffer();

    void SetNymID     (QString qstrID)   { m_nymID      = qstrID;   }
    void SetNymName   (QString qstrName) { m_nymName    = qstrName; }
    void SetServerID  (QString qstrID)   { m_serverID   = qstrID;   }
    void SetServerName(QString qstrName) { m_serverName = qstrName; }

    QString GetNymID()      const { return m_nymID;      }
    QString GetNymName()    const { return m_nymName;    }
    QString GetServerID()   const { return m_serverID;   }
    QString GetServerName() const { return m_serverName; }

private:
    QString m_nymID;
    QString m_nymName;
    QString m_serverID;
    QString m_serverName;

    Ui::WizardNewOffer *ui;
};

#endif // WIZARDNEWOFFER_HPP
