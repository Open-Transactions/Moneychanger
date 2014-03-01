#ifndef PAGEACCT_SERVER_HPP
#define PAGEACCT_SERVER_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

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

signals:
    void SetDefaultServer(QString, QString);

protected:
    void initializePage(); //virtual
    void SetFieldsBlank();

private slots:
    void on_pushButtonSelect_clicked();

    void on_pushButtonManage_clicked();

private:
    bool m_bFirstRun;
    Ui::MTPageAcct_Server *ui;
};

#endif // PAGEACCT_SERVER_HPP
