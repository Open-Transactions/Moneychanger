#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/wizardeditprofile.hpp>
#include <ui_wizardeditprofile.h>

WizardEditProfile::WizardEditProfile(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::WizardEditProfile),
    listContactDataTuples_(*(new listContactDataTuples))
{
    ui->setupUi(this);
}

WizardEditProfile::~WizardEditProfile()
{
    delete ui;
    delete (&listContactDataTuples_);
}
