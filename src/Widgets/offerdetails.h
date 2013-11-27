#ifndef OFFERDETAILS_H
#define OFFERDETAILS_H

#include <QWidget>

#include "editdetails.h"

namespace Ui {
class MTOfferDetails;
}

class MTOfferDetails : public MTEditDetails
{
    Q_OBJECT

public:
    explicit MTOfferDetails(QWidget *parent, MTDetailEdit & theOwner);
    ~MTOfferDetails();

    virtual void refresh(QString strID, QString strName);
    virtual void AddButtonClicked();
    virtual void DeleteButtonClicked();

    virtual void ClearContents();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void FavorLeftSideForIDs();

private:
    QPointer<QWidget> m_pHeaderWidget;

    Ui::MTOfferDetails *ui;
};

#endif // OFFERDETAILS_H
