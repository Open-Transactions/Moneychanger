#ifndef PAGEOFFER_BIDASK_HPP
#define PAGEOFFER_BIDASK_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <QWizardPage>

namespace Ui {
class PageOffer_BidAsk;
}

class PageOffer_BidAsk : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageOffer_BidAsk(QWidget *parent = 0);
    ~PageOffer_BidAsk();

private:
    Ui::PageOffer_BidAsk *ui;
};

#endif // PAGEOFFER_BIDASK_HPP
