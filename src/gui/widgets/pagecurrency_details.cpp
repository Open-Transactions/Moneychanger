
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
}

PageCurrency_Details::~PageCurrency_Details()
{
    delete ui;
}
