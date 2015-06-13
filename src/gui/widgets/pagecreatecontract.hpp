#ifndef PAGECREATECONTRACT_HPP
#define PAGECREATECONTRACT_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <QWizardPage>

namespace Ui {
class MTPageCreateContract;
}

class MTPageCreateContract : public QWizardPage
{
    Q_OBJECT

public:
    explicit MTPageCreateContract(QWidget *parent = 0);
    ~MTPageCreateContract();

    //virtual
    int nextId() const;

signals:
    void SetDefaultNym(QString, QString);

protected:
    void initializePage(); //virtual

    void SetFieldsBlank();

private slots:
    void on_pushButton_clicked();

    void on_pushButtonManage_clicked();

    void on_pushButtonSelect_clicked();

private:
    Ui::MTPageCreateContract *ui;

    bool m_bFirstRun;
};

#endif // PAGECREATECONTRACT_HPP
