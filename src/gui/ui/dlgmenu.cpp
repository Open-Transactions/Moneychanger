#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/ui/dlgmenu.hpp>
#include <ui_dlgmenu.h>

#include <core/moneychanger.hpp>
#include <core/handlers/focuser.h>

#include <QKeyEvent>
#include <QMessageBox>
#include <QDebug>


DlgMenu::DlgMenu(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgMenu)
{
    ui->setupUi(this);

    this->installEventFilter(this);

    connect(Moneychanger::It(), SIGNAL(expertModeUpdated(bool)), this, SLOT(onExpertModeUpdated()));
}

void DlgMenu::onExpertModeUpdated()
{
    refreshOptions();
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


void DlgMenu::onAboutToQuit()
{
    bQuitting_ = true;
}

void DlgMenu::closeEvent(QCloseEvent *event)
{
//    if (bQuitting_ || !this->isVisible())
        QDialog::closeEvent(event);
//    else
//    {
//        QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME),
//                                tr("To hide this window, there is an option in the Settings. To quit the application entirely, click 'Quit'."));
////      this->setVisible(false);
//        event->ignore();
//    }
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

    refreshOptions();
}


void DlgMenu::refreshOptions()
{
    ui->toolButton_payments->setVisible(Moneychanger::It()->hasAccounts());
    ui->toolButton_messages->setVisible(Moneychanger::It()->hasNyms());
    ui->toolButton_pending->setVisible(Moneychanger::It()->hasAccounts());
    ui->toolButton_markets->setVisible(Moneychanger::It()->hasAccounts());
    ui->toolButton_importCash->setVisible(Moneychanger::It()->hasAccounts() && Moneychanger::It()->expertMode());
    ui->toolButton_manageAccounts->setVisible(Moneychanger::It()->hasNyms() &&
                                              (Moneychanger::It()->get_server_list_id_size() > 0) &&
                                              (Moneychanger::It()->get_asset_list_id_size()  > 0) );
    ui->toolButton_smartContracts->setVisible(Moneychanger::It()->expertMode());
    ui->toolButton_Corporations->setVisible(Moneychanger::It()->expertMode());
    ui->toolButton_trade_archive->setVisible(Moneychanger::It()->hasAccounts());
    ui->toolButton_encrypt->setVisible(Moneychanger::It()->hasNyms());
    ui->toolButton_sign->setVisible(Moneychanger::It()->hasNyms());
    ui->toolButton_decrypt->setVisible(Moneychanger::It()->hasNyms());
    ui->toolButton_transport->setVisible(Moneychanger::It()->hasNyms());
    ui->toolButton_requestPayment->setVisible(Moneychanger::It()->hasAccounts());
    ui->toolButton_recurringPayment->setVisible(Moneychanger::It()->hasAccounts() && Moneychanger::It()->expertMode());
    ui->toolButton_liveAgreements->setVisible(Moneychanger::It()->hasAccounts() && Moneychanger::It()->expertMode());
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

void DlgMenu::on_toolButton_transport_clicked()
{
    emit sig_on_toolButton_Transport_clicked();
}

void DlgMenu::on_toolButton_requestPayment_clicked()
{
    emit sig_on_toolButton_requestPayment_clicked();
}

void DlgMenu::on_toolButton_recurringPayment_clicked()
{
    emit sig_on_toolButton_recurringPayment_clicked();
}

void DlgMenu::on_toolButton_liveAgreements_clicked()
{
    emit sig_on_toolButton_liveAgreements_clicked();
}

void DlgMenu::on_toolButton_activity_clicked()
{
    emit sig_on_toolButton_activity_clicked();
}
