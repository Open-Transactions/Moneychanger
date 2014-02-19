#ifndef AGREEMENTDETAILS_H
#define AGREEMENTDETAILS_H

#include <QWidget>

#include "editdetails.h"

namespace Ui {
class MTAgreementDetails;
}

class MTAgreementDetails : public MTEditDetails
{
    Q_OBJECT

public:
    explicit MTAgreementDetails(QWidget *parent, MTDetailEdit & theOwner);
    ~MTAgreementDetails();

    virtual void refresh(QString strID, QString strName);
    virtual void AddButtonClicked();
    virtual void DeleteButtonClicked();

    virtual void ClearContents();

private:
    QWidget * m_pHeaderWidget;
    Ui::MTAgreementDetails *ui;
};

#endif // AGREEMENTDETAILS_H
