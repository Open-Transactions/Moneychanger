#ifndef PAGESMART_NYMACCT_HPP
#define PAGESMART_NYMACCT_HPP

#include <QWizardPage>

namespace Ui {
class PageSmart_NymAcct;
}

class PageSmart_NymAcct : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageSmart_NymAcct(QWidget *parent = 0);
    ~PageSmart_NymAcct();

    bool isComplete() const;

protected:
    void initializePage(); //virtual

    void SetFieldsBlank();

private slots:
    void on_pushButtonSelect_clicked();
    void on_pushButtonManageAcct_clicked();

private:
    QString m_qstrClickText;
    Ui::PageSmart_NymAcct *ui;
};

#endif // PAGESMART_NYMACCT_HPP
