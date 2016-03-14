#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/pagesecurity_details.hpp>
#include <ui_pagesecurity_details.h>


PageSecurity_Details::PageSecurity_Details(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::PageSecurity_Details)
{
    ui->setupUi(this);

    this->registerField("security_contract_name*",   ui->lineEditContractName);
    this->registerField("security_primary_unit*",    ui->lineEditPrimaryUnit);
    this->registerField("security_symbol*",          ui->lineEditSymbol);
    this->registerField("security_tla*",             ui->lineEditTLA);
}

PageSecurity_Details::~PageSecurity_Details()
{
    delete ui;
}
