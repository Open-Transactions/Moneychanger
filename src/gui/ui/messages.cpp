#include "messages.hpp"
#include "ui_messages.h"

Messages::Messages(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Messages)
{
    ui->setupUi(this);
}

Messages::~Messages()
{
    delete ui;
}
