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

    this->registerField("asset_type_currency", ui->radioButtonCurrency);
    this->registerField("asset_type_security", ui->radioButtonSecurity);
    this->registerField("asset_type_basket",   ui->radioButtonBasket);
}

PageAsset_Type::~PageAsset_Type()
{
    delete ui;
}


//virtual
int PageAsset_Type::nextId() const
{
    // -1 turns it into the last page.
    //
//    return -1;

    if (wizard()->field("asset_type_currency").toBool())
        return 3;
    if (wizard()->field("asset_type_security").toBool())
        return 4;
//    if (wizard()->field("asset_type_basket").toBool())
//        return 5;

    return QWizardPage::nextId();
 }

