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
}

PageSecurity_Details::~PageSecurity_Details()
{
    delete ui;
}

void PageSecurity_Details::on_pushButtonDefaults_clicked()
{
    this->setField("security_contract_name",   QString("Sample Co. Share"));
    this->setField("security_primary_unit",    QString("shares"));
    this->setField("security_symbol",          QString("SAMPL"));
}
