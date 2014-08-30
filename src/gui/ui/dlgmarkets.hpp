#ifndef DLGMARKETS_HPP
#define DLGMARKETS_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <core/handlers/contacthandler.hpp>

#include <QPointer>
#include <QDialog>


namespace Ui {
class DlgMarkets;
}

namespace opentxs{

    namespace OTDB {
        class MarketList;
        class OfferListNym;
    }
}


class MTDetailEdit;

class Moneychanger;

class DlgMarkets : public QDialog
{
    Q_OBJECT

public:
    explicit DlgMarkets(QWidget *parent);
    ~DlgMarkets();

    void FirstRun();

    void dialog();

    void RefreshRecords(); // For servers and nyms.

    QString GetNymID() { return m_nymId; }

signals:
    void needToLoadOrRetrieveMarkets();

public slots:
    void onCurrentMarketChanged_Offers (QString qstrMarketID);
    void onCurrentMarketChanged_Markets(QString qstrMarketID);

    void LoadOrRetrieveMarkets(); // For markets and offers.

    void onNeedToLoadOrRetrieveOffers(QString qstrMarketID);

    void onBalancesChangedFromAbove();


protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void SetCurrentNymIDBasedOnIndex   (int index);
    void SetCurrentServerIDBasedOnIndex(int index);
    // -----------------------------------------------
    bool RetrieveMarketList(mapIDName & the_map);
    bool LowLevelRetrieveMarketList(QString qstrServerID, QString qstrNymID, mapIDName & the_map);
    // -----------------------------------------------
    bool LoadMarketList(mapIDName & the_map);
    bool LowLevelLoadMarketList(QString qstrServerID, QString qstrNymID, mapIDName & the_map);
    // -----------------------------------------------
    opentxs::OTDB::MarketList * LoadMarketListForServer(const std::string & serverID);
    // -----------------------------------------------
    bool RetrieveOfferList(mapIDName & the_map, QString qstrMarketID);
    bool LowLevelRetrieveOfferList(QString qstrServerID, QString qstrNymID, mapIDName & the_map, QString qstrMarketID);
    // -----------------------------------------------
    bool LoadOfferList(mapIDName & the_map, QString qstrMarketID);
    bool LowLevelLoadOfferList(QString qstrServerID, QString qstrNymID, mapIDName & the_map, QString qstrMarketID);
    // -----------------------------------------------
    opentxs::OTDB::OfferListNym * LoadOfferListForServer(const std::string & serverID, const std::string & nymID);
    // -----------------------------------------------
    bool GetMarket_AssetCurrencyScale(QString qstrMarketID, QString & qstrAssetID, QString & qstrCurrencyID, QString & qstrScale);

private slots:
    void on_comboBoxServer_currentIndexChanged(int index);

    void on_comboBoxNym_currentIndexChanged(int index);

    void on_toolButton_clicked();

private:
    void ClearMarketMap();
    void ClearOfferMap();

    QMultiMap<QString, QVariant> m_mapMarkets; // market/scale, marketdata

    QMap <QString, QVariant> m_mapOffers; // server/transID, offerdatanym

    mapIDName m_mapServers;
    mapIDName m_mapNyms;

    QString m_serverId;
    QString m_nymId;

    bool m_bFirstRun;
    bool m_bHaveRetrievedFirstTime;
    bool m_bHaveShownOffersFirstTime;

    QPointer<MTDetailEdit> m_pMarketDetails;
    QPointer<MTDetailEdit> m_pOfferDetails;

    Ui::DlgMarkets *ui;
};

#endif // DLGMARKETS_HPP
