#ifndef PAGEACCT_SERVER_H
#define PAGEACCT_SERVER_H

#include <QWizardPage>

namespace Ui {
class MTPageAcct_Server;
}

class MTPageAcct_Server : public QWizardPage
{
    Q_OBJECT

public:
    explicit MTPageAcct_Server(QWidget *parent = 0);
    ~MTPageAcct_Server();

protected:
    virtual void showEvent(QShowEvent * event);

private slots:
    void on_pushButtonSelect_clicked();

    void on_pushButtonManage_clicked();

private:
    bool m_bFirstRun;
    Ui::MTPageAcct_Server *ui;
};

#endif // PAGEACCT_SERVER_H
