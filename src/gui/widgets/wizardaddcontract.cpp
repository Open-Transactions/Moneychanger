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
    return ui->wizardPage2->getContents();
}

void MTWizardAddContract::setAssetMode()
{
    bIsAssetMode_=true;

    QWizardPage *pageType            = new PageAsset_Type;
    QWizardPage *pageCurrencyDetails = new PageCurrency_Details;

    if (nullptr != pageType)            this->addPage(pageType);
    if (nullptr != pageCurrencyDetails) this->addPage(pageCurrencyDetails);
}
