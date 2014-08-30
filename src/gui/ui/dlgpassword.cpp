#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/ui/dlgpassword.hpp>
#include <ui_dlgpassword.h>

#include <gui/widgets/overridecursor.hpp>

#include <opentxs/core/OTPassword.hpp>



MTDlgPassword::MTDlgPassword(QWidget *parent, opentxs::OTPassword & thePassword) :
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
    extractPassword ();
}

void MTDlgPassword::extractPassword()
{
    if (!ui->lineEdit->text().isEmpty())
        m_pPassword->setPassword(ui->lineEdit->text().toStdString().c_str(),
                                 ui->lineEdit->text().toStdString().size());
}


