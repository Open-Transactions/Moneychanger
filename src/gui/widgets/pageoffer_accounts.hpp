#ifndef PAGEOFFER_ACCOUNTS_HPP
#define PAGEOFFER_ACCOUNTS_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <QWizardPage>

namespace Ui {
class PageOffer_Accounts;
}

class PageOffer_Accounts : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageOffer_Accounts(QWidget *parent = 0);
    ~PageOffer_Accounts();

protected:
    void initializePage(); //virtual

private slots:
    void on_pushButtonManageAssetAcct_clicked();
    void on_pushButtonManageCurrencyAcct_clicked();
    void on_pushButtonSelectAssetAcct_clicked();
    void on_pushButtonSelectCurrencyAcct_clicked();
    void on_pushButtonManageServer_clicked();
    void on_pushButtonManageNym_clicked();

private:
    Ui::PageOffer_Accounts *ui;
};

#endif // PAGEOFFER_ACCOUNTS_HPP
