#ifndef PAGEACCT_NYM_HPP
#define PAGEACCT_NYM_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <QWizardPage>

namespace Ui {
class MTPageAcct_Nym;
}

class MTPageAcct_Nym : public QWizardPage
{
    Q_OBJECT

public:
    explicit MTPageAcct_Nym(QWidget *parent = 0);
    ~MTPageAcct_Nym();

signals:
    void SetDefaultNym(QString, QString);

protected:
    void initializePage(); //virtual

    void SetFieldsBlank();

private slots:
    void on_pushButtonSelect_clicked();

    void on_pushButtonManage_clicked();

private:
    bool m_bFirstRun;

    Ui::MTPageAcct_Nym *ui;
};

#endif // PAGEACCT_NYM_HPP
