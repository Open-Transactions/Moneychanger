#ifndef PAGEACCT_ASSET_H
#define PAGEACCT_ASSET_H

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
    virtual void showEvent(QShowEvent * event);

private slots:
    void on_pushButtonSelect_clicked();

    void on_pushButtonManage_clicked();

private:
    bool m_bFirstRun;

    Ui::MTPageAcct_Asset *ui;
};

#endif // PAGEACCT_ASSET_H
