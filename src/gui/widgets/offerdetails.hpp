#ifndef OFFERDETAILS_HPP
#define OFFERDETAILS_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <QWidget>

#include <gui/widgets/editdetails.hpp>

namespace Ui {
class MTOfferDetails;
}

namespace opentxs{
namespace OTDB {
class TradeListNym;
class OfferDataNym;
}
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

    void PopulateNymTradesGrid(QString & qstrID, QString qstrNymID, QMap<QString, QVariant> & OFFER_MAP);
    void ClearTradesGrid();

    opentxs::OTDB::TradeListNym * LoadTradeListForNym(opentxs::OTDB::OfferDataNym & offerData, QString qstrNotaryID, QString qstrNymID);

    bool ChooseServer(QString & qstrNotaryID, QString & qstrServerName);

private slots:
    void on_toolButtonAssetAcct_clicked();
    void on_toolButtonCurrencyAcct_clicked();

private:
    QPointer<QWidget> m_pHeaderWidget;

    Ui::MTOfferDetails *ui;
};

#endif // OFFERDETAILS_HPP
