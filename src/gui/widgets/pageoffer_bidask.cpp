#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/pageoffer_bidask.hpp>
#include <ui_pageoffer_bidask.h>


PageOffer_BidAsk::PageOffer_BidAsk(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::PageOffer_BidAsk)
{
    ui->setupUi(this);

    registerField("bid", ui->radioButtonBid);
}

PageOffer_BidAsk::~PageOffer_BidAsk()
{
    delete ui;
}
