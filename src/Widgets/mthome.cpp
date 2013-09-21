#include "mthome.h"
#include "ui_mthome.h"

#include "mthomedetail.h"


MTHome::MTHome(QWidget *parent) :
    QWidget(parent, Qt::Window),
    ui(new Ui::MTHome)
{
    ui->setupUi(this);
}

MTHome::~MTHome()
{
    delete ui;
}

void MTHome::on_tableView_clicked(const QModelIndex &index)
{
    MTHomeDetail * pDetailPane = new MTHomeDetail;

    QVBoxLayout * pDetailLayout = new QVBoxLayout;

    pDetailLayout->addWidget(pDetailPane);

    ui->frame->setLayout(pDetailLayout);
    pDetailPane->show();
//    ui->frame->show();
}
