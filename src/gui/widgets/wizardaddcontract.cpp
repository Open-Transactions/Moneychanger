#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/wizardaddcontract.hpp>
#include <ui_wizardaddcontract.h>

#include <gui/widgets/pageasset_type.hpp>
#include <gui/widgets/pagecurrency_details.hpp>

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
    return ui->wizardPage1->getContents();
}

void MTWizardAddContract::setServerMode()
{
    bServerMode_ = true;
}

void MTWizardAddContract::setAssetMode()
{
    bAssetMode_ = true;
}

