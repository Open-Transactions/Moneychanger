#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include "wizardaddaccount.h"
#include "ui_wizardaddaccount.h"

#include "moneychanger.h"

MTWizardAddAccount::MTWizardAddAccount(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::MTWizardAddAccount)
{
    ui->setupUi(this);

    setOption(QWizard::NoCancelButton,     false);
    setOption(QWizard::CancelButtonOnLeft, true);
}


MTWizardAddAccount::~MTWizardAddAccount()
{
    delete ui;
}
