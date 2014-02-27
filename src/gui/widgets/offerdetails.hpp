#ifndef OFFERDETAILS_HPP
#define OFFERDETAILS_HPP

#include <WinsockWrapper.h>
#include <ExportWrapper.h>

#include <QWidget>

#include "editdetails.h"

namespace Ui {
class MTOfferDetails;
}

namespace OTDB {
class TradeListNym;
class OfferDataNym;
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

    OTDB::TradeListNym * LoadTradeListForNym(OTDB::OfferDataNym & offerData, QString qstrServerID, QString qstrNymID);

    bool ChooseServer(QString & qstrServerID, QString & qstrServerName);

private slots:
    void on_toolButtonAssetAcct_clicked();
    void on_toolButtonCurrencyAcct_clicked();

private:
    QPointer<QWidget> m_pHeaderWidget;

    Ui::MTOfferDetails *ui;
};

#endif // OFFERDETAILS_HPP
