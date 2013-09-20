#include "mthomedetail.h"
#include "ui_mthomedetail.h"

MTHomeDetail::MTHomeDetail(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MTHomeDetail)
{
    ui->setupUi(this);
}

MTHomeDetail::~MTHomeDetail()
{
    delete ui;
}
