#include "createinsurancecompany.h"
#include "ui_createinsurancecompany.h"

createinsurancecompany::createinsurancecompany(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::createinsurancecompany)
{
    ui->setupUi(this);
}

createinsurancecompany::~createinsurancecompany()
{
    delete ui;
}
