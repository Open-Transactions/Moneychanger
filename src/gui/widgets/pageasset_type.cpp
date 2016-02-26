#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/pageasset_type.hpp>
#include <ui_pageasset_type.h>

PageAsset_Type::PageAsset_Type(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::PageAsset_Type)
{
    ui->setupUi(this);

    this->registerField("asset_type.currency", ui->radioButtonCurrency);
    this->registerField("asset_type.security", ui->radioButtonSecurity);
    this->registerField("asset_type.basket",   ui->radioButtonBasket);
}

PageAsset_Type::~PageAsset_Type()
{
    delete ui;
}


