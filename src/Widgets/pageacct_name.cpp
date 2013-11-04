#include "pageacct_name.h"
#include "ui_pageacct_name.h"

MTPageAcct_Name::MTPageAcct_Name(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::MTPageAcct_Name)
{
    ui->setupUi(this);

    this->registerField("Name*", ui->lineEditName);
}

MTPageAcct_Name::~MTPageAcct_Name()
{
    delete ui;
}
