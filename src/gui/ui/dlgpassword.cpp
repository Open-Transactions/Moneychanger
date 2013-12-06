#ifndef STABLE_H
#include <core/stable.h>
#endif

#include "dlgpassword.h"
#include "ui_dlgpassword.h"

#include <opentxs/OTPassword.h>

#include "overridecursor.h"


MTDlgPassword::MTDlgPassword(QWidget *parent, OTPassword & thePassword) :
    QDialog(parent),
    m_pPassword(&thePassword),
    m_pCursor(MTOverrideCursor::Exists()),
    ui(new Ui::MTDlgPassword)
{
    ui->setupUi(this);

    this->installEventFilter(this);
    // --------------------------------
    // If the MTSpinner aka MTOverrideCursor was set already by the time this
    // password dialog activated, we want to TEMPORARILY deactivate it, until
    // this password dialog closes again.
    //
    if (NULL != m_pCursor)
        m_pCursor->Pause();
}

void MTDlgPassword::closeEvent(QCloseEvent * e)
{
    if (NULL != m_pCursor)
        m_pCursor->Unpause();
    // --------------------------------
    QDialog::closeEvent(e);
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


