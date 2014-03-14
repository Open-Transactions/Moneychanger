#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/wizardnewoffer.hpp>
#include <ui_wizardnewoffer.h>


WizardNewOffer::WizardNewOffer(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::WizardNewOffer)
{
    ui->setupUi(this);
}

WizardNewOffer::~WizardNewOffer()
{
    delete ui;
}
