#ifndef DLGMENU_HPP
#define DLGMENU_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

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

private slots:
    void on_toolButton_main_clicked();
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

signals:
    void sig_on_toolButton_main_clicked();
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

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void closeEvent(QCloseEvent *event);

private:
    Ui::DlgMenu *ui;
};

#endif // DLGMENU_HPP
