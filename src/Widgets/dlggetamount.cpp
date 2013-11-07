#include "dlggetamount.h"
#include "ui_dlggetamount.h"

DlgGetAmount::DlgGetAmount(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgGetAmount)
{
    ui->setupUi(this);
}

DlgGetAmount::~DlgGetAmount()
{
    delete ui;
}

void DlgGetAmount::on_lineEdit_editingFinished()
{

}
