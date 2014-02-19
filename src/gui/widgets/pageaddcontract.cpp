#ifndef STABLE_H
#include <core/stable.h>
#endif

#include "pageaddcontract.h"
#include "ui_pageaddcontract.h"

MTPageAddContract::MTPageAddContract(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::MTPageAddContract)
{
    ui->setupUi(this);

    this->setCommitPage(true);

    this->registerField("isImporting", ui->radioButton_1);
    this->registerField("isCreating",  ui->radioButton_2);
}


MTPageAddContract::~MTPageAddContract()
{
    delete ui;
}


//virtual
int MTPageAddContract::nextId() const
{
    // -1 turns it into the last page.
    //
//    if (ui->radioButton_1->isChecked())
//        return -1;

    if (ui->radioButton_1->isChecked())
        return 1; // goes to import page.
    else if (ui->radioButton_2->isChecked())
        return 2; // goes to create page.


    return QWizardPage::nextId();
}

