#ifndef PAGEACCT_NYM_H
#define PAGEACCT_NYM_H

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

protected:
    virtual void showEvent(QShowEvent * event);

private slots:
    void on_pushButtonSelect_clicked();

    void on_pushButtonManage_clicked();

private:
    bool m_bFirstRun;

    Ui::MTPageAcct_Nym *ui;
};

#endif // PAGEACCT_NYM_H
