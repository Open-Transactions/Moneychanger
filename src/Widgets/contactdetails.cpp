
#include <QDebug>

#include "contactdetails.h"
#include "ui_contactdetails.h"

MTContactDetails::MTContactDetails(QWidget *parent) :
    MTEditDetails(parent),
    ui(new Ui::MTContactDetails)
{
    ui->setupUi(this);
    this->setContentsMargins(0, 0, 0, 0);
}

MTContactDetails::~MTContactDetails()
{
    delete ui;
}

//virtual
void MTContactDetails::refresh(QString strID, QString strName)
{
    // todo

    ui->label->setText(strName);
    ui->label_2->setText(strID);

    qDebug() << "MTContactDetails::refresh";
}


