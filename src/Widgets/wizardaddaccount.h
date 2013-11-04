#ifndef WIZARDADDACCOUNT_H
#define WIZARDADDACCOUNT_H

#include <QWizard>

namespace Ui {
class MTWizardAddAccount;
}

class Moneychanger;

class MTWizardAddAccount : public QWizard
{
    Q_OBJECT

public:
    explicit MTWizardAddAccount(QWidget *parent, Moneychanger & theMC);
    ~MTWizardAddAccount();

    Moneychanger * GetMoneychanger();

private:
    Moneychanger * m_pMoneychanger;

    Ui::MTWizardAddAccount *ui;
};

#endif // WIZARDADDACCOUNT_H
