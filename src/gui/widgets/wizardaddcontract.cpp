#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include "wizardaddcontract.h"
#include "ui_wizardaddcontract.h"

MTWizardAddContract::MTWizardAddContract(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::MTWizardAddContract)
{
    ui->setupUi(this);

    setOption(QWizard::NoCancelButton,     false);
    setOption(QWizard::CancelButtonOnLeft, true);
}

MTWizardAddContract::~MTWizardAddContract()
{
    delete ui;
}

QString MTWizardAddContract::getContents() const
{
    return ui->wizardPage2->getContents();
}
