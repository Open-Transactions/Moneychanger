#include "payments.hpp"
#include "ui_payments.h"

Payments::Payments(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Payments)
{
    ui->setupUi(this);
}

Payments::~Payments()
{
    delete ui;
}
