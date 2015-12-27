#include "proposeplandlg.hpp"
#include "ui_proposeplandlg.h"

ProposePlanDlg::ProposePlanDlg(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProposePlanDlg)
{
    ui->setupUi(this);
}

ProposePlanDlg::~ProposePlanDlg()
{
    delete ui;
}
