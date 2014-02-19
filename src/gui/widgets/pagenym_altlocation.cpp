#ifndef STABLE_H
#include <core/stable.h>
#endif

#include "pagenym_altlocation.h"
#include "ui_pagenym_altlocation.h"

MTPageNym_AltLocation::MTPageNym_AltLocation(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::MTPageNym_AltLocation)
{
    ui->setupUi(this);

    this->registerField("Location", ui->lineEditLocation);
}

MTPageNym_AltLocation::~MTPageNym_AltLocation()
{
    delete ui;
}
