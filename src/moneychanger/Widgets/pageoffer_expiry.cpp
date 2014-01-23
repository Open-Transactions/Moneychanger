#include "pageoffer_expiry.h"
#include "ui_pageoffer_expiry.h"

PageOffer_Expiry::PageOffer_Expiry(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::PageOffer_Expiry)
{
    ui->setupUi(this);
}

PageOffer_Expiry::~PageOffer_Expiry()
{
    delete ui;
}
