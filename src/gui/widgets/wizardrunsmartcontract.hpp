#ifndef WIZARDRUNSMARTCONTRACT_HPP
#define WIZARDRUNSMARTCONTRACT_HPP

#include <core/handlers/contacthandler.hpp>

#include <QWizard>

namespace Ui {
class WizardRunSmartContract;
}

class WizardRunSmartContract : public QWizard
{
    Q_OBJECT

public:
    explicit WizardRunSmartContract(QWidget *parent = 0);
    ~WizardRunSmartContract();

private:
    Ui::WizardRunSmartContract *ui;
};

#endif // WIZARDRUNSMARTCONTRACT_HPP
