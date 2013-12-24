#include "dlgpasswordconfirm.h"
#include "ui_dlgpasswordconfirm.h"

#include <opentxs/OTPassword.h>

MTDlgPasswordConfirm::MTDlgPasswordConfirm(QWidget *parent, OTPassword & thePassword) :
    QDialog(parent),
    m_pPassword(&thePassword),
    ui(new Ui::MTDlgPasswordConfirm)
{
    ui->setupUi(this);

    this->installEventFilter(this);
}

MTDlgPasswordConfirm::~MTDlgPasswordConfirm()
{
    delete ui;
}

void MTDlgPasswordConfirm::setDisplay(QString qstrDisplay)
{
    ui->labelDisplay->setText(qstrDisplay);
}

void MTDlgPasswordConfirm::on_MTDlgPasswordConfirm_accepted()
{
    if (!ui->lineEditPassword->text().isEmpty() && // If a password was entered, and...
        (0 == (ui->lineEditPassword->text().compare(ui->lineEditConfirm->text())))) // ...If the two passwords match.
        // ---------------------------------------------------------------------------
        m_pPassword->setPassword(ui->lineEditPassword->text().toStdString().c_str(),
                                 ui->lineEditPassword->text().toStdString().size());
}
