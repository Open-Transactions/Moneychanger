#include "dlgpassword.h"
#include "ui_dlgpassword.h"

#include <opentxs/OTPassword.h>

MTDlgPassword::MTDlgPassword(QWidget *parent, OTPassword & thePassword) :
    QDialog(parent),
    m_pPassword(&thePassword),
    ui(new Ui::MTDlgPassword)
{
    ui->setupUi(this);
}

MTDlgPassword::~MTDlgPassword()
{
    delete ui;
}

void MTDlgPassword::setDisplay(QString qstrDisplay)
{
    ui->labelDisplay->setText(qstrDisplay);
}

void MTDlgPassword::on_MTDlgPassword_accepted()
{
    if (!ui->lineEdit->text().isEmpty())
        m_pPassword->setPassword(ui->lineEdit->text().toStdString().c_str(),
                                 ui->lineEdit->text().toStdString().size());
}
