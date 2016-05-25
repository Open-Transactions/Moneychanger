#include "wizardconfirmsmartcontract.hpp"
#include "ui_wizardconfirmsmartcontract.h"

WizardConfirmSmartContract::WizardConfirmSmartContract(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::WizardConfirmSmartContract)
{
    ui->setupUi(this);

    setOption(QWizard::NoCancelButton,     false);
    setOption(QWizard::CancelButtonOnLeft, true);
}

WizardConfirmSmartContract::~WizardConfirmSmartContract()
{
    delete ui;
}
