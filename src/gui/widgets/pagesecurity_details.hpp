#ifndef PAGESECURITY_DETAILS_HPP
#define PAGESECURITY_DETAILS_HPP

#include <QWizardPage>

namespace Ui {
class PageSecurity_Details;
}

class PageSecurity_Details : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageSecurity_Details(QWidget *parent = 0);
    ~PageSecurity_Details();

private slots:
    void on_pushButtonDefaults_clicked();

private:
    Ui::PageSecurity_Details *ui;
};

#endif // PAGESECURITY_DETAILS_HPP


