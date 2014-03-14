#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/pagecreatecontract.hpp>
#include <ui_pagecreatecontract.h>


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

