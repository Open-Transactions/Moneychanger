
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

    this->registerField("currency.contract_name",   ui->lineEditContractName);
    this->registerField("currency.primary_unit",    ui->lineEditPrimaryUnit);
    this->registerField("currency.symbol",          ui->lineEditSymbol);
    this->registerField("currency.tla",             ui->lineEditTLA);
    this->registerField("currency.fractional_unit", ui->lineEditFractionalUnit);
}

PageCurrency_Details::~PageCurrency_Details()
{
    delete ui;
}
