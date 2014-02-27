#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include "wizardaddnym.h"
#include "ui_wizardaddnym.h"

MTWizardAddNym::MTWizardAddNym(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::MTWizardAddNym)
{
    ui->setupUi(this);

    setOption(QWizard::NoCancelButton,     false);
    setOption(QWizard::CancelButtonOnLeft, true);
}

MTWizardAddNym::~MTWizardAddNym()
{
    delete ui;
}
