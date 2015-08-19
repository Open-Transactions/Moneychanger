#include "dlgcreatepassphrase.hpp"
#include "ui_dlgcreatepassphrase.h"

DlgCreatePassphrase::DlgCreatePassphrase(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgCreatePassphrase)
{
    ui->setupUi(this);
}

DlgCreatePassphrase::~DlgCreatePassphrase()
{
    delete ui;
}
