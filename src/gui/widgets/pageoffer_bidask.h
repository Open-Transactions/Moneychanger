#ifndef PAGEOFFER_BIDASK_H
#define PAGEOFFER_BIDASK_H

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

#endif // PAGEOFFER_BIDASK_H
