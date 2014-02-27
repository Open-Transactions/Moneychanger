#ifndef STABLE_H
#include <core/stable.hpp>
#endif

#include "pagecreatecontract.h"
#include "ui_pagecreatecontract.h"

MTPageCreateContract::MTPageCreateContract(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::MTPageCreateContract)
{
    ui->setupUi(this);
}

MTPageCreateContract::~MTPageCreateContract()
{
    delete ui;
}

//virtual
int MTPageCreateContract::nextId() const
{
    // -1 turns it into the last page.
    //
    return -1;
 }

