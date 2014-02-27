#ifndef AGREEMENTDETAILS_HPP
#define AGREEMENTDETAILS_HPP

#include <WinsockWrapper.h>
#include <ExportWrapper.h>

#include <gui/widgets/editdetails.hpp>


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

#endif // AGREEMENTDETAILS_HPP
