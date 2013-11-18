#ifndef MARKETDETAILS_H
#define MARKETDETAILS_H

#include <QWidget>

#include "editdetails.h"

namespace Ui {
class MTMarketDetails;
}

class MTMarketDetails : public MTEditDetails
{
    Q_OBJECT

public:
    explicit MTMarketDetails(QWidget *parent, MTDetailEdit & theOwner);
    ~MTMarketDetails();

    virtual void refresh(QString strID, QString strName);
    virtual void AddButtonClicked();
    virtual void DeleteButtonClicked();

    virtual void ClearContents();

private:
    QWidget * m_pHeaderWidget;

    Ui::MTMarketDetails *ui;
};

#endif // MARKETDETAILS_H
