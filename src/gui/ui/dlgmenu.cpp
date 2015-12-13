#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/ui/dlgmenu.hpp>
#include <ui_dlgmenu.h>

#include <core/handlers/focuser.h>

#include <QKeyEvent>
#include <QMessageBox>


DlgMenu::DlgMenu(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgMenu)
{
    ui->setupUi(this);

    this->installEventFilter(this);
}

bool DlgMenu::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Escape)
        {
//          close(); // This is caught by this same filter.
            return true;
        }
    }

    // standard event processing
    return QDialog::eventFilter(obj, event);
}


void DlgMenu::closeEvent(QCloseEvent *event)
{
    QMessageBox::information(this, tr("Quit?"),
                            tr("If you want to close the application, click the 'Quit' button"));
    event->ignore();
//  QDialog::closeEvent(event);
}


DlgMenu::~DlgMenu()
{
    delete ui;
}


void DlgMenu::dialog()
{
    Focuser f(this);
    f.show();
    f.focus();
}


void DlgMenu::on_toolButton_payments_clicked()
{
    emit sig_on_toolButton_payments_clicked();
}

void DlgMenu::on_toolButton_messages_clicked()
{
    emit sig_on_toolButton_messages_clicked();
}

void DlgMenu::on_toolButton_pending_clicked()
{
    emit sig_on_toolButton_pending_clicked();
}

void DlgMenu::on_toolButton_markets_clicked()
{
    emit sig_on_toolButton_markets_clicked();
}

void DlgMenu::on_toolButton_importCash_clicked()
{
    emit sig_on_toolButton_importCash_clicked();
}

void DlgMenu::on_toolButton_contacts_clicked()
{
    emit sig_on_toolButton_contacts_clicked();
}

void DlgMenu::on_toolButton_manageAccounts_clicked()
{
    emit sig_on_toolButton_manageAccounts_clicked();
}

void DlgMenu::on_toolButton_manageAssets_clicked()
{
    emit sig_on_toolButton_manageAssets_clicked();
}

void DlgMenu::on_toolButton_manageNyms_clicked()
{
    emit sig_on_toolButton_manageNyms_clicked();
}

void DlgMenu::on_toolButton_manageServers_clicked()
{
    emit sig_on_toolButton_manageServers_clicked();
}

void DlgMenu::on_toolButton_smartContracts_clicked()
{
    emit sig_on_toolButton_smartContracts_clicked();
}

void DlgMenu::on_toolButton_Corporations_clicked()
{
    emit sig_on_toolButton_Corporations_clicked();
}

void DlgMenu::on_toolButton_settings_clicked()
{
    emit sig_on_toolButton_settings_clicked();
}

void DlgMenu::on_toolButton_quit_clicked()
{
    emit sig_on_toolButton_quit_clicked();
}

void DlgMenu::on_toolButton_trade_archive_clicked()
{
    emit sig_on_toolButton_trade_archive_clicked();
}

void DlgMenu::on_toolButton_secrets_clicked()
{
    emit sig_on_toolButton_secrets_clicked();
}

void DlgMenu::on_toolButton_encrypt_clicked()
{
    emit sig_on_toolButton_encrypt_clicked();
}

void DlgMenu::on_toolButton_sign_clicked()
{
    emit sig_on_toolButton_sign_clicked();
}

void DlgMenu::on_toolButton_decrypt_clicked()
{
    emit sig_on_toolButton_decrypt_clicked();
}
