#ifndef WIZARDCONFIRMSMARTCONTRACT_HPP
#define WIZARDCONFIRMSMARTCONTRACT_HPP

#include <core/handlers/contacthandler.hpp>

#include <QWizard>

namespace Ui {
class WizardConfirmSmartContract;
}

class WizardConfirmSmartContract : public QWizard
{
    Q_OBJECT

public:
    explicit WizardConfirmSmartContract(QWidget *parent = 0);
    ~WizardConfirmSmartContract();

private:
    Ui::WizardConfirmSmartContract *ui;
};

#endif // WIZARDCONFIRMSMARTCONTRACT_HPP
