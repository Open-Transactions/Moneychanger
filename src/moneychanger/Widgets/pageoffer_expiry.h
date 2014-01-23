#ifndef PAGEOFFER_EXPIRY_H
#define PAGEOFFER_EXPIRY_H

#include <QWizardPage>

namespace Ui {
class PageOffer_Expiry;
}

class PageOffer_Expiry : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageOffer_Expiry(QWidget *parent = 0);
    ~PageOffer_Expiry();

private:
    Ui::PageOffer_Expiry *ui;
};

#endif // PAGEOFFER_EXPIRY_H
