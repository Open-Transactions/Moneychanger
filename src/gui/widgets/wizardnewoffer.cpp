#ifndef STABLE_H
#include <core/stable.hpp>
#endif

#include "wizardnewoffer.h"
#include "ui_wizardnewoffer.h"

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
