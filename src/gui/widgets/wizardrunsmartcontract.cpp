#include "wizardrunsmartcontract.hpp"
#include "ui_wizardrunsmartcontract.h"

WizardRunSmartContract::WizardRunSmartContract(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::WizardRunSmartContract)
{
    ui->setupUi(this);

    setOption(QWizard::NoCancelButton,     false);
    setOption(QWizard::CancelButtonOnLeft, true);
}

WizardRunSmartContract::~WizardRunSmartContract()
{
    delete ui;
}
