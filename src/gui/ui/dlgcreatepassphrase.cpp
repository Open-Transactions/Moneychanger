#include "dlgcreatepassphrase.hpp"
#include "ui_dlgcreatepassphrase.h"

#include <gui/ui/dlgpassphrasemanager.hpp>

#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <QMessageBox>
#include <QPushButton>
#include <QClipboard>

#include <opentxs/core/Version.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>
#include <opentxs/core/String.hpp>


#define PASSWORDS_MATCH_STR  tr("Passwords match!")
#define PASSWORDS_NOT_MATCH_STR  tr("Passwords NOT matching.")


DlgCreatePassphrase::DlgCreatePassphrase(QWidget *parent) :
    QDialog(parent),
    m_pPassphrase(new opentxs::OTPassword),
    ui(new Ui::DlgCreatePassphrase)
{
    InitializeDlg();
}

DlgCreatePassphrase::DlgCreatePassphrase(const QString & qstrTitle,
                                         const QString & qstrUsername,
                                         const QString & qstrURL,
                                         const QString & qstrNotes,
                                         const opentxs::OTPassword & thePassword,
                                         QWidget *parent) :
    QDialog(parent),
    m_qstrTitle(qstrTitle),
    m_qstrUsername(qstrUsername),
    m_pPassphrase(new opentxs::OTPassword(thePassword)),
    m_qstrURL(qstrURL),
    m_qstrNotes(qstrNotes),
    ui(new Ui::DlgCreatePassphrase)
{
    InitializeDlg();
}

void DlgCreatePassphrase::InitializeDlg()
{
    ui->setupUi(this);

    this->installEventFilter(this);
    // ------------------------------
    ui->lineEditTitle     ->setText(m_qstrTitle);
    ui->lineEditUsername  ->setText(m_qstrUsername);
    ui->lineEditURL       ->setText(m_qstrURL);
    ui->plainTextEditNotes->setPlainText(m_qstrNotes);
    ui->lineEditPassphrase->setText(QString::fromUtf8(m_pPassphrase->getPassword()));
    ui->lineEditRetype    ->setText(QString::fromUtf8(m_pPassphrase->getPassword()));
    // ------------------------------
    if (ui->lineEditPassphrase->text().length() > 0)
        ui->labelMatching->setText(PASSWORDS_MATCH_STR);
    // ------------------------------
    ui->lineEditTitle->setFocus();

    QObject::connect(ui->lineEditPassphrase, SIGNAL(textChanged(QString)), this, SLOT(passphraseChangedSlot(QString)));
    QObject::connect(ui->lineEditRetype, SIGNAL(textChanged(QString)), this, SLOT(retypeChangedSlot(QString)));
}

void DlgCreatePassphrase::on_checkBox_toggled(bool checked)
{
    if (checked)
    {
        ui->lineEditPassphrase->setEchoMode(QLineEdit::Normal);
        ui->lineEditRetype    ->setEchoMode(QLineEdit::Normal);
    }
    else
    {
        ui->lineEditPassphrase->setEchoMode(QLineEdit::Password);
        ui->lineEditRetype    ->setEchoMode(QLineEdit::Password);
    }
}

void DlgCreatePassphrase::passphraseChangedSlot(QString text)
{
    Q_UNUSED(text);
    ui->lineEditRetype->setText("");
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

void DlgCreatePassphrase::retypeChangedSlot(QString text)
{
    Q_UNUSED(text);

    if (ui->lineEditPassphrase->text() != ui->lineEditRetype->text())
    {
        ui->labelMatching->setText(PASSWORDS_NOT_MATCH_STR);
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
    else
    {
        ui->labelMatching->setText(PASSWORDS_MATCH_STR);
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}



void DlgCreatePassphrase::on_toolButtonTitle_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();

    if (NULL != clipboard)
    {
        clipboard->setText(ui->lineEditTitle->text());
    }
}

void DlgCreatePassphrase::on_toolButtonUsername_clicked()
{
    ClipboardWrapper::It()->set(ui->lineEditUsername->text());
}

void DlgCreatePassphrase::on_toolButtonURL_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();

    if (NULL != clipboard)
    {
        clipboard->setText(ui->lineEditURL->text());
    }
}

void DlgCreatePassphrase::on_toolButtonPassphrase_clicked()
{
    ClipboardWrapper::It()->set(ui->lineEditPassphrase->text());
}

void DlgCreatePassphrase::on_buttonBox_accepted()
{
    m_qstrTitle    = ui->lineEditTitle->text();
    m_qstrUsername = ui->lineEditUsername->text();
    m_qstrURL      = ui->lineEditURL->text();
    m_qstrNotes    = ui->plainTextEditNotes->toPlainText();

    auto strTemp = opentxs::String(ui->lineEditPassphrase->text().toStdString());
    m_pPassphrase->setPassword(strTemp.Get(), strTemp.GetLength());

    QDialog::accept();
}

void DlgCreatePassphrase::on_pushButtonGenerate_clicked()
{
    m_pPassphrase->randomizePassword(12); // size of 12.

    ui->lineEditPassphrase->setText(QString::fromUtf8(m_pPassphrase->getPassword()));
    ui->lineEditRetype    ->setText(QString::fromUtf8(m_pPassphrase->getPassword()));

    ui->labelMatching->setText(PASSWORDS_MATCH_STR);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void DlgCreatePassphrase::on_buttonBox_rejected()
{
    QDialog::reject();
}

DlgCreatePassphrase::~DlgCreatePassphrase()
{
    delete ui;
    delete m_pPassphrase;
}

bool DlgCreatePassphrase::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Escape)
        {
            close(); // This is caught by this same filter.
            return true;
        }
    }
    // standard event processing
    return QDialog::eventFilter(obj, event);
}

