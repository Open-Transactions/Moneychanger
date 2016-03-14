#ifndef PAGECURRENCY_DETAILS_HPP
#define PAGECURRENCY_DETAILS_HPP

#include <QWizardPage>

namespace Ui {
class PageCurrency_Details;
}

class PageCurrency_Details : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageCurrency_Details(QWidget *parent = 0);
    ~PageCurrency_Details();

    //virtual
    int nextId() const;

private:
    Ui::PageCurrency_Details *ui;
};

#endif // PAGECURRENCY_DETAILS_HPP
