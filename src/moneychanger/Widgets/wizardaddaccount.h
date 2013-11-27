#ifndef WIZARDADDACCOUNT_H
#define WIZARDADDACCOUNT_H

#include <QPointer>
#include <QWizard>

namespace Ui {
class MTWizardAddAccount;
}


class MTWizardAddAccount : public QWizard
{
    Q_OBJECT

public:
    explicit MTWizardAddAccount(QWidget *parent);
    ~MTWizardAddAccount();

private:
    Ui::MTWizardAddAccount *ui;
};

#endif // WIZARDADDACCOUNT_H
