#ifndef DLGMENU_HPP
#define DLGMENU_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <QDialog>

namespace Ui {
class DlgMenu;
}

class DlgMenu : public QDialog
{
    Q_OBJECT

public:
    explicit DlgMenu(QWidget *parent = 0);
    ~DlgMenu();

    void dialog();

    void refreshOptions();

public slots:
    void onAboutToQuit();

private slots:
    void on_toolButton_payments_clicked();
    void on_toolButton_messages_clicked();
    void on_toolButton_pending_clicked();
    void on_toolButton_markets_clicked();
    void on_toolButton_importCash_clicked();
    void on_toolButton_contacts_clicked();
    void on_toolButton_manageAccounts_clicked();
    void on_toolButton_manageAssets_clicked();
    void on_toolButton_manageNyms_clicked();
    void on_toolButton_manageServers_clicked();
    void on_toolButton_smartContracts_clicked();
    void on_toolButton_Corporations_clicked();
    void on_toolButton_settings_clicked();
    void on_toolButton_quit_clicked();
    void on_toolButton_trade_archive_clicked();
    void on_toolButton_secrets_clicked();
    void on_toolButton_encrypt_clicked();
    void on_toolButton_sign_clicked();
    void on_toolButton_decrypt_clicked();
    void on_toolButton_transport_clicked();
    void on_toolButton_requestPayment_clicked();
    void on_toolButton_recurringPayment_clicked();
    void on_toolButton_liveAgreements_clicked();

    void onExpertModeUpdated();

    void on_toolButton_activity_clicked();

signals:
    void sig_on_toolButton_payments_clicked();
    void sig_on_toolButton_messages_clicked();
    void sig_on_toolButton_pending_clicked();
    void sig_on_toolButton_markets_clicked();
    void sig_on_toolButton_importCash_clicked();
    void sig_on_toolButton_contacts_clicked();
    void sig_on_toolButton_manageAccounts_clicked();
    void sig_on_toolButton_manageAssets_clicked();
    void sig_on_toolButton_manageNyms_clicked();
    void sig_on_toolButton_manageServers_clicked();
    void sig_on_toolButton_smartContracts_clicked();
    void sig_on_toolButton_Corporations_clicked();
    void sig_on_toolButton_settings_clicked();
    void sig_on_toolButton_quit_clicked();
    void sig_on_toolButton_trade_archive_clicked();
    void sig_on_toolButton_secrets_clicked();
    void sig_on_toolButton_encrypt_clicked();
    void sig_on_toolButton_sign_clicked();
    void sig_on_toolButton_decrypt_clicked();
    void sig_on_toolButton_Transport_clicked();
    void sig_on_toolButton_requestPayment_clicked();
    void sig_on_toolButton_recurringPayment_clicked();
    void sig_on_toolButton_liveAgreements_clicked();
    void sig_on_toolButton_activity_clicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void closeEvent(QCloseEvent *event);

private:
    Ui::DlgMenu *ui;

    bool bQuitting_=false;
};

#endif // DLGMENU_HPP
