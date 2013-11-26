#ifndef MARKETDETAILS_H
#define MARKETDETAILS_H

#include <string>

#include <QMap>
#include <QMultiMap>
#include <QWidget>

#include "editdetails.h"

namespace Ui {
class MTMarketDetails;
}

namespace OTDB {
class MarketData;
class OfferListMarket;
class TradeListMarket;
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

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void FavorLeftSideForIDs();
    // ----------------------------------
    QString CalculateTotalAssets    (QString & qstrID, QMultiMap<QString, QVariant> & multimap);
    QString CalculateNumberBids     (QString & qstrID, QMultiMap<QString, QVariant> & multimap);
    QString CalculateNumberAsks     (QString & qstrID, QMultiMap<QString, QVariant> & multimap);
    QString CalculateNumberOffers   (QString & qstrID, QMultiMap<QString, QVariant> & multimap, bool bIsBid);
    QString CalculateLastSalePrice  (QString & qstrID, QMultiMap<QString, QVariant> & multimap);
    QString CalculateCurrentBid     (QString & qstrID, QMultiMap<QString, QVariant> & multimap);
    QString CalculateCurrentAsk     (QString & qstrID, QMultiMap<QString, QVariant> & multimap);
    // ----------------------------------
    void ClearBidsGrid   ();
    void ClearAsksGrid   ();
    void ClearTradesGrid ();
    // ----------------------------------
    void RetrieveMarketOffers(QString & qstrID, QMultiMap<QString, QVariant> & multimap);
    void RetrieveMarketTrades(QString & qstrID, QMultiMap<QString, QVariant> & multimap);
    // ----------------------------------
    bool LowLevelRetrieveMarketOffers(OTDB::MarketData & marketData);
    bool LowLevelRetrieveMarketTrades(OTDB::MarketData & marketData);
    // ----------------------------------
    OTDB::OfferListMarket * LoadOfferListForMarket(OTDB::MarketData & marketData);
    OTDB::TradeListMarket * LoadTradeListForMarket(OTDB::MarketData & marketData);
    // ----------------------------------
    void PopulateMarketOffersGrids(QString & qstrID, QMultiMap<QString, QVariant> & multimap);
    void PopulateRecentTradesGrid (QString & qstrID, QMultiMap<QString, QVariant> & multimap);
    // ----------------------------------
private slots:
    void on_toolButtonAsset_clicked();

    void on_toolButtonCurrency_clicked();

private:
    QWidget * m_pHeaderWidget;

    Ui::MTMarketDetails *ui;
};

#endif // MARKETDETAILS_H
