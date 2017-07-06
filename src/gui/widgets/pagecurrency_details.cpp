
#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/pagecurrency_details.hpp>
#include <ui_pagecurrency_details.h>

PageCurrency_Details::PageCurrency_Details(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::PageCurrency_Details)
{
    ui->setupUi(this);

    this->registerField("currency_contract_name*",   ui->lineEditContractName);
    this->registerField("currency_primary_unit*",    ui->lineEditPrimaryUnit);
    this->registerField("currency_symbol*",          ui->lineEditSymbol);
    this->registerField("currency_tla*",             ui->lineEditTLA);
    this->registerField("currency_fractional_unit*", ui->lineEditFractionalUnit);
    this->registerField("currency_decimal_spaces*",  ui->lineEditDecimalSpaces);
}

PageCurrency_Details::~PageCurrency_Details()
{
    delete ui;
}

//virtual
int PageCurrency_Details::nextId() const
{
    return 5;
}

void PageCurrency_Details::on_pushButtonDefaults_clicked()
{
    this->setField("currency_contract_name",   QString("MtGox Dollar"));
    this->setField("currency_primary_unit",    QString("dollars"));
    this->setField("currency_symbol",          QString("$"));
    this->setField("currency_tla",             QString("USD"));
    this->setField("currency_fractional_unit", QString("cents"));
    this->setField("currency_decimal_spaces",  QString("2"));
}
