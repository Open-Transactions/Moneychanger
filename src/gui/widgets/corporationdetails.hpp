#ifndef CORPORATIONDETAILS_HPP
#define CORPORATIONDETAILS_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <gui/widgets/editdetails.hpp>

namespace Ui {
class MTCorporationDetails;
}

class MTCorporationDetails : public MTEditDetails
{
    Q_OBJECT

public:
    explicit MTCorporationDetails(QWidget *parent, MTDetailEdit & theOwner);
    ~MTCorporationDetails();

    virtual void refresh(QString strID, QString strName);
    virtual void AddButtonClicked();
    virtual void DeleteButtonClicked();

    virtual void ClearContents();

private:
    QWidget * m_pHeaderWidget;

    Ui::MTCorporationDetails *ui;
};

#endif // CORPORATIONDETAILS_HPP
