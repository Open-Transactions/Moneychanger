#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/pageacct_name.hpp>
#include <ui_pageacct_name.h>


MTPageAcct_Name::MTPageAcct_Name(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::MTPageAcct_Name)
{
    ui->setupUi(this);

    this->registerField("Name*", ui->lineEditName);
}

MTPageAcct_Name::~MTPageAcct_Name()
{
    delete ui;
}
