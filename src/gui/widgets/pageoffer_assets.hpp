#ifndef PAGEOFFER_ASSETS_HPP
#define PAGEOFFER_ASSETS_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <QWizardPage>

namespace Ui {
class PageOffer_Assets;
}

class PageOffer_Assets : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageOffer_Assets(QWidget *parent = 0);
    ~PageOffer_Assets();

protected:
    void initializePage(); //virtual
    void SetAssetBlank();
    void SetCurrencyBlank();

private slots:
    void on_pushButtonManageAsset_clicked();
    void on_pushButtonManageCurrency_clicked();
    void on_pushButtonSelectAsset_clicked();
    void on_pushButtonSelectCurrency_clicked();

private:
    Ui::PageOffer_Assets *ui;
};

#endif // PAGEOFFER_ASSETS_HPP
