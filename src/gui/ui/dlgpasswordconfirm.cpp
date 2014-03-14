#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/ui/dlgpasswordconfirm.hpp>
#include <ui_dlgpasswordconfirm.h>

#include <gui/widgets/overridecursor.hpp>

#include <opentxs/OTPassword.hpp>


MTDlgPasswordConfirm::MTDlgPasswordConfirm(QWidget *parent, OTPassword & thePassword) :
    QDialog(parent),
    m_pPassword(&thePassword),
    m_pCursor(MTOverrideCursor::Exists()),
    ui(new Ui::MTDlgPasswordConfirm)
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

void MTDlgPasswordConfirm::closeEvent(QCloseEvent * e)
{
    if (NULL != m_pCursor)
        m_pCursor->Unpause();
    // --------------------------------
    QDialog::closeEvent(e);
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
