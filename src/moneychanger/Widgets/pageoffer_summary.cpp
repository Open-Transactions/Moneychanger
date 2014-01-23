#include "pageoffer_summary.h"
#include "ui_pageoffer_summary.h"

PageOffer_Summary::PageOffer_Summary(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::PageOffer_Summary)
{
    ui->setupUi(this);
}

PageOffer_Summary::~PageOffer_Summary()
{
    delete ui;
}
