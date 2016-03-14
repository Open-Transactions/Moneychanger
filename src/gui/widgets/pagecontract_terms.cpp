#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/pagecontract_terms.hpp>
#include <ui_pagecontract_terms.h>

PageContract_Terms::PageContract_Terms(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::PageContract_Terms)
{
    ui->setupUi(this);

    this->registerField("terms", ui->plainTextEdit, "plainText", "textChanged");
}

PageContract_Terms::~PageContract_Terms()
{
    delete ui;
}

// Makes it the last page.
int PageContract_Terms::nextId() const { return -1; }
