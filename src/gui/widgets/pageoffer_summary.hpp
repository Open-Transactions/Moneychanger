#ifndef PAGEOFFER_SUMMARY_HPP
#define PAGEOFFER_SUMMARY_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <QWizardPage>

namespace Ui {
class PageOffer_Summary;
}

class PageOffer_Summary : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageOffer_Summary(QWidget *parent = 0);
    ~PageOffer_Summary();

protected:
    void initializePage(); //virtual

private:
    Ui::PageOffer_Summary *ui;
};

#endif // PAGEOFFER_SUMMARY_HPP
