#ifndef WIZARDADDACCOUNT_HPP
#define WIZARDADDACCOUNT_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

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

#endif // WIZARDADDACCOUNT_HPP
