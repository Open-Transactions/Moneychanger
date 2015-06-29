#ifndef WIZARDPARTYACCT_HPP
#define WIZARDPARTYACCT_HPP

#include <core/handlers/contacthandler.hpp>

#include <QWizard>

namespace Ui {
class WizardPartyAcct;
}

class WizardPartyAcct : public QWizard
{
    Q_OBJECT

public:
    explicit WizardPartyAcct(QWidget *parent = 0);
    ~WizardPartyAcct();

    QString m_qstrNotaryID;
    QString m_qstrNymID;
    mapIDName m_mapConfirmed;

private:
    Ui::WizardPartyAcct *ui;
};

#endif // WIZARDPARTYACCT_HPP
