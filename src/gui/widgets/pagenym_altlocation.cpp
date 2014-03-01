#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/pagenym_altlocation.hpp>
#include <ui_pagenym_altlocation.h>


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
