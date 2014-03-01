#ifndef PAGEACCT_ASSET_HPP
#define PAGEACCT_ASSET_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <QWizardPage>

namespace Ui {
class MTPageAcct_Asset;
}

class MTPageAcct_Asset : public QWizardPage
{
    Q_OBJECT

public:
    explicit MTPageAcct_Asset(QWidget *parent = 0);
    ~MTPageAcct_Asset();

signals:
    void SetDefaultAsset(QString, QString);

protected:
    void initializePage(); //virtual

    void SetFieldsBlank();

private slots:
    void on_pushButtonSelect_clicked();

    void on_pushButtonManage_clicked();

private:
    bool m_bFirstRun;

    Ui::MTPageAcct_Asset *ui;
};

#endif // PAGEACCT_ASSET_HPP
