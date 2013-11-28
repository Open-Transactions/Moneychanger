
#include <QKeyEvent>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QDebug>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>
#include <opentxs/OTStorage.h>

#include "dlgmarkets.h"
#include "ui_dlgmarkets.h"

#include "moneychanger.h"

#include "detailedit.h"

#include "editdetails.h"



DlgMarkets::DlgMarkets(QWidget *parent) :
    QDialog(parent),
    m_bFirstRun(true),
    m_bHaveRetrievedFirstTime(false),
    m_bHaveRetrievedOffersFirstTime(false),
    ui(new Ui::DlgMarkets)
{
    ui->setupUi(this);

    this->installEventFilter(this);
}

void DlgMarkets::ClearMarketMap()
{
    QMultiMap<QString, QVariant> temp_map = m_mapMarkets;

    m_mapMarkets.clear();
    // ------------------------------------
    for (QMultiMap<QString, QVariant>::iterator it_map = temp_map.begin();
         it_map != temp_map.end(); ++it_map)
    {
        OTDB::MarketData * pMarketData = VPtr<OTDB::MarketData>::asPtr(it_map.value());

        if (NULL != pMarketData) // should never be NULL.
            delete pMarketData;
    }
    // --------------------
    temp_map.clear();
}

void DlgMarkets::ClearOfferMap()
{
    QMap<QString, QVariant> temp_map = m_mapOffers;

    m_mapOffers.clear();
    // ------------------------------------
    for (QMap<QString, QVariant>::iterator it_map = temp_map.begin();
         it_map != temp_map.end(); ++it_map)
    {
        OTDB::OfferDataNym * pOfferData = VPtr<OTDB::OfferDataNym>::asPtr(it_map.value());

        if (NULL != pOfferData) // should never be NULL.
            delete pOfferData;
    }
    // --------------------
    temp_map.clear();
}

void DlgMarkets::dialog()
{
    FirstRun();
    // -------------------------------------------
    RefreshRecords();
    // -------------------------------------------
    show();
    setFocus();
}

void DlgMarkets::FirstRun()
{
    if (m_bFirstRun)
    {
        m_bFirstRun = false;
        // -----------------------
        // Initialization here...

        // ******************************************************
        {
            m_pMarketDetails = new MTDetailEdit(this);

            m_pMarketDetails->SetMarketMap(m_mapMarkets);
            // -------------------------------------
            m_pMarketDetails->setWindowTitle(tr("Markets"));
            // -------------------------------------
            QVBoxLayout * pLayout = new QVBoxLayout;
            pLayout->addWidget(m_pMarketDetails);

            m_pMarketDetails->setContentsMargins(1,1,1,1);
            pLayout->setContentsMargins(1,1,1,1);

            ui->tabMarkets->setContentsMargins(1,1,1,1);

            ui->tabMarkets->setLayout(pLayout);
            // -------------------------------------
            connect(m_pMarketDetails, SIGNAL(CurrentMarketChanged(QString)),
                    this,             SLOT(onCurrentMarketChanged_Markets(QString)));
        }
        // ******************************************************
//        void CurrentMarketChanged(QString qstrMarketID);


        // ******************************************************
        {
            m_pOfferDetails = new MTDetailEdit(this);

            m_pOfferDetails->SetMarketMap(m_mapMarkets);
            m_pOfferDetails->SetOfferMap (m_mapOffers);
            // -------------------------------------
            m_pOfferDetails->setWindowTitle(tr("Orders"));

            // NOTE: This gets set already whenever MTDetailEdit::FirstRun
            // is called. But we want it to be set before that, in this case,
            // since m_pMarketDetails and m_pOfferDetails are intertwined.
            //
            m_pOfferDetails->SetType(MTDetailEdit::DetailEditTypeOffer);
            // -------------------------------------
            QVBoxLayout * pLayout = new QVBoxLayout;
            pLayout->addWidget(m_pOfferDetails);

            m_pOfferDetails->setContentsMargins(1,1,1,1);
            pLayout->setContentsMargins(1,1,1,1);

            ui->tabOffers->setContentsMargins(1,1,1,1);

            ui->tabOffers->setLayout(pLayout);
            // -------------------------------------
            connect(m_pOfferDetails, SIGNAL(CurrentMarketChanged(QString)),
                    this,            SLOT(onCurrentMarketChanged_Offers(QString)));
            // -------------------------------------
            connect(m_pOfferDetails, SIGNAL(RefreshOffers(QString)),
                    this,            SLOT(onNeedToRefreshOffers(QString)));
            // -------------------------------------
        }
        // ******************************************************

        // Whenever the GUI refreshes (say, the list of servers is cleared
        // and re-populated) then we will set the CURRENT selection as whatever
        // is in m_serverId / m_nymId.
        //
        // The initial state for this comes from the defaults, but once the user
        // starts changing the selection of the combo box, the new current selection
        // will go into these variables.
        //
        m_nymId    = Moneychanger::It()->get_default_nym_id();
        m_serverId = Moneychanger::It()->get_default_server_id();

        m_pMarketDetails->SetMarketNymID(m_nymId);
        m_pOfferDetails ->SetMarketNymID(m_nymId);
    }
}

// The "current market" has changed on the offers tab.
// (Thus, we need to notify the markets tab.)
//
void DlgMarkets::onCurrentMarketChanged_Offers (QString qstrMarketID)
{
    m_pMarketDetails->onMarketIDChangedFromAbove(qstrMarketID);

    onNeedToRefreshOffers(qstrMarketID);
}


// The "current market" has changed on the markets tab.
// (Thus, we need to notify the offers tab.)
//
void DlgMarkets::onCurrentMarketChanged_Markets(QString qstrMarketID)
{
    m_pOfferDetails->onMarketIDChangedFromAbove(qstrMarketID);
}

void DlgMarkets::on_pushButtonRefresh_clicked()
{
    m_bHaveRetrievedFirstTime = false; // To force RefreshRecords to re-download.
    m_bHaveRetrievedOffersFirstTime = false;

    RefreshRecords();

    // TODO here: turn refresh button black.
}


void DlgMarkets::SetCurrentNymIDBasedOnIndex(int index)
{
    if ((m_mapNyms.size() > 0) && (index >= 0))
    {
        int nCurrentIndex = -1;

        for (mapIDName::iterator it_map = m_mapNyms.begin(); it_map != m_mapNyms.end(); ++it_map)
        {
            ++nCurrentIndex; // zero on first iteration.

            if (nCurrentIndex == index)
            {
                m_nymId = it_map.key();
                break;
            }
        }
    }
    else
        m_nymId = QString("");
    // ------------------------------------------
    if (m_pMarketDetails)
        m_pMarketDetails->SetMarketNymID(m_nymId);
    // ------------------------------------------
    if (m_pOfferDetails)
        m_pOfferDetails->SetMarketNymID(m_nymId);
    // ------------------------------------------
}

void DlgMarkets::SetCurrentServerIDBasedOnIndex(int index)
{
    if ((m_mapServers.size() > 0) && (index >= 0))
    {
        int nCurrentIndex = -1;

        for (mapIDName::iterator it_map = m_mapServers.begin(); it_map != m_mapServers.end(); ++it_map)
        {
            ++nCurrentIndex; // zero on first iteration.

            if (nCurrentIndex == index)
            {
                m_serverId = it_map.key();
                break;
            }
        }
    }
    else
        m_serverId = QString("");
    // ------------------------------------------
}

void DlgMarkets::on_comboBoxServer_currentIndexChanged(int index)
{
    SetCurrentServerIDBasedOnIndex(index);
    // -----------------------------
    RefreshMarkets();
}



void DlgMarkets::on_comboBoxNym_currentIndexChanged(int index)
{
    SetCurrentNymIDBasedOnIndex(index);
    // -----------------------------
    RefreshMarkets();
}

// -----------------------------------------------
bool DlgMarkets::RetrieveOfferList(mapIDName & the_map, QString qstrMarketID)
{
    if (m_serverId.isEmpty() || m_nymId.isEmpty())
        return false;
    // -----------------
    bool bSuccess = true;
    QString qstrAll("all");
    // -----------------
    if (m_serverId != qstrAll)
        return LowLevelRetrieveOfferList(m_serverId, m_nymId, the_map, qstrMarketID);
    else
    {
        int nCurrentIndex = -1;

        for (mapIDName::iterator it_map = m_mapServers.begin(); it_map != m_mapServers.end(); ++it_map)
        {
            ++nCurrentIndex; // zero on first iteration.
            // ---------------
            if (0 == nCurrentIndex)
                continue; // Skipping the "all" option. (Looping through all the ACTUAL servers.)
            // ---------------
            QString qstrServerID = it_map.key();
            // ---------------
            if (false == LowLevelRetrieveOfferList(qstrServerID, m_nymId, the_map, qstrMarketID))
                bSuccess = false; // Failure here just means ONE of the servers failed.
        }
    }
    return bSuccess;
}
// -----------------------------------------------
bool DlgMarkets::LowLevelRetrieveOfferList(QString qstrServerID, QString qstrNymID, mapIDName & the_map, QString qstrMarketID)
{
    if (qstrServerID.isEmpty() || qstrNymID.isEmpty())
        return false;
    // -----------------
    OT_ME madeEasy;

    const std::string str_reply = madeEasy.get_nym_market_offers(qstrServerID.toStdString(),
                                                                 qstrNymID   .toStdString());
    const int32_t     nResult   = madeEasy.VerifyMessageSuccess(str_reply);

    const bool bSuccess = (1 == nResult);
    // -----------------------------------
    if (bSuccess)
    {
        return LowLevelLoadOfferList(qstrServerID, m_nymId, the_map, qstrMarketID);
    }

    return bSuccess;

}
// -----------------------------------------------
bool DlgMarkets::LoadOfferList(mapIDName & the_map, QString qstrMarketID)
{
    if (m_serverId.isEmpty() || m_nymId.isEmpty())
        return false;
    // -----------------
    bool bSuccess = true;
    QString qstrAll("all");
    // -----------------
    if (m_serverId != qstrAll)
        return LowLevelLoadOfferList(m_serverId, m_nymId, the_map, qstrMarketID);
    else
    {
        int nCurrentIndex = -1;

        for (mapIDName::iterator it_map = m_mapServers.begin(); it_map != m_mapServers.end(); ++it_map)
        {
            ++nCurrentIndex; // zero on first iteration.
            // ---------------
            if (0 == nCurrentIndex)
                continue; // Skipping the "all" option. (Looping through all the ACTUAL servers.)
            // ---------------
            QString qstrServerID = it_map.key();
            // ---------------
            if (false == LowLevelLoadOfferList(qstrServerID, m_nymId, the_map, qstrMarketID))
                bSuccess = false; // Failure here just means ONE of the servers failed.
        }
    }
    return bSuccess;
}
// -----------------------------------------------
bool DlgMarkets::GetMarket_AssetCurrencyScale(QString qstrMarketID, QString & qstrAssetID, QString & qstrCurrencyID, QString & qstrScale)
{
    // -------------------------------------------------------------
    QMap<QString, QVariant>::iterator it_market = m_mapMarkets.find(qstrMarketID);

    if (m_mapMarkets.end() != it_market)
    {
        // ------------------------------------------------------
        OTDB::MarketData * pMarketData = VPtr<OTDB::MarketData>::asPtr(it_market.value());

        if (NULL != pMarketData) // Should never be NULL.
        {
            qstrAssetID    = QString::fromStdString(pMarketData->asset_type_id);
            qstrCurrencyID = QString::fromStdString(pMarketData->currency_type_id);
            qstrScale      = QString::fromStdString(pMarketData->scale);

            return true;
        }
    }
    // ------------------------------
    return false;
}
// -----------------------------------------------
bool DlgMarkets::LowLevelLoadOfferList(QString qstrServerID, QString qstrNymID, mapIDName & the_map, QString qstrMarketID)
{
    if (qstrServerID.isEmpty() || qstrNymID.isEmpty() || qstrMarketID.isEmpty())
        return false;
    // -----------------------------------
    QString qstrAssetID, qstrCurrencyID, qstrMarketScale;

    const bool bGotIDs = GetMarket_AssetCurrencyScale(qstrMarketID, qstrAssetID, qstrCurrencyID, qstrMarketScale);
    // -----------------------------------
    if (bGotIDs)
    {
        OTDB::OfferListNym * pOfferList = LoadOfferListForServer(qstrServerID.toStdString(), qstrNymID.toStdString());
        OTCleanup<OTDB::OfferListNym> theAngel(pOfferList);

        if (NULL != pOfferList)
        {
            size_t nOfferDataCount = pOfferList->GetOfferDataNymCount();

            for (size_t ii = 0; ii < nOfferDataCount; ++ii)
            {
                OTDB::OfferDataNym * pOfferData = pOfferList->GetOfferDataNym(ii);

                if (NULL == pOfferData) // Should never happen.
                    continue;
                // -----------------------------------------------------------------------
                QString qstrOfferAssetID    = QString::fromStdString(pOfferData->asset_type_id);
                QString qstrOfferCurrencyID = QString::fromStdString(pOfferData->currency_type_id);
                QString qstrOfferScale      = QString::fromStdString(pOfferData->scale);
                // -----------------------------------------------------------------------
                if ((qstrAssetID     != qstrOfferAssetID)    ||
                    (qstrCurrencyID  != qstrOfferCurrencyID) ||
                    (qstrMarketScale != qstrOfferScale))
                    continue;
                // -----------------------------------------------------------------------
                QString qstrTransactionID = QString::fromStdString(pOfferData->transaction_id);
                // -----------------------------------------------------------------------
                QString qstrCompositeID = QString("%1,%2").arg(qstrServerID).arg(qstrTransactionID);
                // -----------------------------------------------------------------------
                m_mapOffers.insert(qstrCompositeID, VPtr<OTDB::OfferDataNym>::asQVariant(pOfferData->clone()));
                // -----------------------------------------------------------------------
                QString qstrBuySell = pOfferData->selling ? tr("Sell") : tr("Buy");

                const std::string str_asset_name = OTAPI_Wrap::GetAssetType_Name(pOfferData->asset_type_id);
                // --------------------------
                int64_t lTotalAssets   = OTAPI_Wrap::StringToLong(pOfferData->total_assets);
                int64_t lFinishedSoFar = OTAPI_Wrap::StringToLong(pOfferData->finished_so_far);
                // --------------------------
                const std::string str_total_assets    = OTAPI_Wrap::FormatAmount(pOfferData->asset_type_id, lTotalAssets);
                const std::string str_finished_so_far = OTAPI_Wrap::FormatAmount(pOfferData->asset_type_id, lFinishedSoFar);
                // --------------------------
                QString qstrAmounts;

                if (lFinishedSoFar > 0) // "300g (40g finished so far)"
                    qstrAmounts = QString("%1 (%2 %3)").
                            arg(QString::fromStdString(str_total_assets)).
                            arg(QString::fromStdString(str_finished_so_far)).
                            arg(tr("finished so far"));
                else // "300g"
                    qstrAmounts = QString("%1").
                            arg(QString::fromStdString(str_total_assets));
                // --------------------------
    //          "Buy Silver Grams: 300g (40g finished so far)";
                //
                QString qstrOfferName = QString("%1 %2: %3").
                        arg(qstrBuySell).
                        arg(QString::fromStdString(str_asset_name)).
                        arg(qstrAmounts);
                // ---------------------------
                the_map.insert(qstrCompositeID, qstrOfferName);
                // ---------------------------
            } // for
        }
    }
    // -----------------------------------
    return true;
}
// -----------------------------------------------
OTDB::OfferListNym * DlgMarkets::LoadOfferListForServer(const std::string & serverID, const std::string & nymID)
{
    OTDB::OfferListNym * pOfferList = NULL;
    OTDB::Storable     * pStorable  = NULL;
    // ------------------------------------------
    QString qstrFilename = QString("%1.bin").arg(QString::fromStdString(nymID));

    if (OTDB::Exists("nyms", serverID, "offers", qstrFilename.toStdString()))
    {
        pStorable = OTDB::QueryObject(OTDB::STORED_OBJ_OFFER_LIST_NYM, "nyms", serverID, "offers", qstrFilename.toStdString());

        if (NULL == pStorable)
            return NULL;
        // -------------------------------
        pOfferList = OTDB::OfferListNym::ot_dynamic_cast(pStorable);

        if (NULL == pOfferList)
            delete pStorable;
    }

    return pOfferList;
}
// -----------------------------------------------



bool DlgMarkets::LoadMarketList(mapIDName & the_map)
{
    if (m_serverId.isEmpty() || m_nymId.isEmpty())
        return false;
    // -----------------
    bool bSuccess = true;
    QString qstrAll("all");
    // -----------------
    if (m_serverId != qstrAll)
        return LowLevelLoadMarketList(m_serverId, m_nymId, the_map);
    else
    {
        int nCurrentIndex = -1;

        for (mapIDName::iterator it_map = m_mapServers.begin(); it_map != m_mapServers.end(); ++it_map)
        {
            ++nCurrentIndex; // zero on first iteration.
            // ---------------
            if (0 == nCurrentIndex)
                continue; // Skipping the "all" option. (Looping through all the ACTUAL servers.)
            // ---------------
            QString qstrServerID = it_map.key();
            // ---------------
            if (false == LowLevelLoadMarketList(qstrServerID, m_nymId, the_map))
                bSuccess = false; // Failure here just means ONE of the servers failed.
        }
    }
    return bSuccess;
}


bool DlgMarkets::LowLevelLoadMarketList(QString qstrServerID, QString qstrNymID, mapIDName & the_map)
{
    if (qstrServerID.isEmpty() || qstrNymID.isEmpty())
        return false;
    // -----------------------------------
    OTDB::MarketList * pMarketList = LoadMarketListForServer(qstrServerID.toStdString());
    OTCleanup<OTDB::MarketList> theAngel(pMarketList);

    if (NULL != pMarketList)
    {
        size_t nMarketDataCount = pMarketList->GetMarketDataCount();

        for (size_t ii = 0; ii < nMarketDataCount; ++ii)
        {
            OTDB::MarketData * pMarketData = pMarketList->GetMarketData(ii);

            if (NULL == pMarketData) // Should never happen.
                continue;
            // -----------------------------------------------------------------------
            QString qstrMarketID    = QString::fromStdString(pMarketData->market_id);
            QString qstrScale       = QString::fromStdString(pMarketData->scale);
            // -----------------------------------------------------------------------
            QString qstrCompositeID = QString("%1,%2").arg(qstrMarketID).arg(qstrScale);
            // -----------------------------------------------------------------------
            // This multimap will have multiple markets of the same key (from
            // different servers.)
            //
            m_mapMarkets.insertMulti(qstrCompositeID, VPtr<OTDB::MarketData>::asQVariant(pMarketData->clone()));
            // -----------------------------------------------------------------------
            // Whereas this map will only have a single entry for each key. (Thus
            // we only add it here if it's not already present.)
            //
            mapIDName::iterator it_map = the_map.find(qstrCompositeID);

            if (the_map.end() == it_map)
            {
                const std::string str_asset_name    = OTAPI_Wrap::GetAssetType_Name(pMarketData->asset_type_id);
                const std::string str_currency_name = OTAPI_Wrap::GetAssetType_Name(pMarketData->currency_type_id);
                // --------------------------
                QString qstrMarketName = QString("%1 for %2").
                        arg(QString::fromStdString(str_asset_name)).
                        arg(QString::fromStdString(str_currency_name));
                // ---------------------------
                the_map.insert(qstrCompositeID, qstrMarketName);
            }
            // ---------------------------
        } // for
    }
    // -----------------------------------
    return true;
}

bool DlgMarkets::RetrieveMarketList(mapIDName & the_map)
{
    if (m_serverId.isEmpty() || m_nymId.isEmpty())
        return false;
    // -----------------
    bool bSuccess = true;
    QString qstrAll("all");
    // -----------------
    if (m_serverId != qstrAll)
        return LowLevelRetrieveMarketList(m_serverId, m_nymId, the_map);
    else
    {
        int nCurrentIndex = -1;

        for (mapIDName::iterator it_map = m_mapServers.begin(); it_map != m_mapServers.end(); ++it_map)
        {
            ++nCurrentIndex; // zero on first iteration.
            // ---------------
            if (0 == nCurrentIndex)
                continue; // Skipping the "all" option. (Looping through all the ACTUAL servers.)
            // ---------------
            QString qstrServerID = it_map.key();
            // ---------------
            if (false == LowLevelRetrieveMarketList(qstrServerID, m_nymId, the_map))
                bSuccess = false; // Failure here just means ONE of the servers failed.
        }
    }
    return bSuccess;
}


bool DlgMarkets::LowLevelRetrieveMarketList(QString qstrServerID, QString qstrNymID, mapIDName & the_map)
{
    if (qstrServerID.isEmpty() || qstrNymID.isEmpty())
        return false;
    // -----------------
    OT_ME madeEasy;

    const std::string str_reply = madeEasy.get_market_list(qstrServerID.toStdString(),
                                                           qstrNymID   .toStdString());
    const int32_t     nResult   = madeEasy.VerifyMessageSuccess(str_reply);

    const bool bSuccess = (1 == nResult);
    // -----------------------------------
    if (bSuccess)
    {
        return LowLevelLoadMarketList(qstrServerID, m_nymId, the_map);
    }

    return bSuccess;
}



// Caller responsible to delete!
//
OTDB::MarketList * DlgMarkets::LoadMarketListForServer(const std::string & serverID)
{
    OTDB::MarketList * pMarketList = NULL;
    OTDB::Storable   * pStorable   = NULL;
    // ------------------------------------------
    if (OTDB::Exists("markets", serverID, "market_data.bin"))
    {
        pStorable = OTDB::QueryObject(OTDB::STORED_OBJ_MARKET_LIST, "markets", serverID, "market_data.bin");

        if (NULL == pStorable)
            return NULL;
        // -------------------------------
        pMarketList = OTDB::MarketList::ot_dynamic_cast(pStorable);

        if (NULL == pMarketList)
            delete pStorable;
    }

    return pMarketList;
}


// Detail level...
// (For markets and offers.)
//
void DlgMarkets::RefreshMarkets()
{
    if ((ui->comboBoxNym   ->currentIndex() >=0) &&
        (ui->comboBoxServer->currentIndex() >=0))
    {
        m_pMarketDetails->setVisible(true);
        m_pOfferDetails ->setVisible(true);

        // ***********************************************
        {
            // -------------------------------------
            mapIDName & the_map = m_pMarketDetails->m_map;

            the_map.clear();
            // -------------------------------------
            ClearMarketMap();
            // -------------------------------------
            if (!m_bHaveRetrievedFirstTime)
            {
                m_bHaveRetrievedFirstTime = true;
                RetrieveMarketList(the_map); // Download the list of markets from the server(s).
            }
            else
            {
                // TODO here: turn the "refresh" button Red.
                //
                LoadMarketList(the_map); // Load from local storage. (Let the user hit "Refresh" if he wants to re-download.)
            }
            // -------------------------------------
            m_pMarketDetails->show_widget(MTDetailEdit::DetailEditTypeMarket);
        }
        // ***********************************************
        {
            m_pOfferDetails->m_mapMarkets.clear();
            // -------------------------------------
            for (mapIDName::iterator it_offer_markets = m_pMarketDetails->m_map.begin();
                 it_offer_markets != m_pMarketDetails->m_map.end(); ++it_offer_markets)
            {
                QString qstrID    = it_offer_markets.key();
                QString qstrValue = it_offer_markets.value();
                // -------------------------------------------------------------
                QMap<QString, QVariant>::iterator it_market = m_mapMarkets.find(qstrID);

                if (m_mapMarkets.end() != it_market)
                {
                    // ------------------------------------------------------
                    OTDB::MarketData * pMarketData = VPtr<OTDB::MarketData>::asPtr(it_market.value());

                    if (NULL != pMarketData) // Should never be NULL.
                    {
                        // ------------------------------------------------------
                        int64_t     lScale    = OTAPI_Wrap::It()->StringToLong(pMarketData->scale);
                        if (lScale > 1)
                        {
                            std::string str_scale = OTAPI_Wrap::FormatAmount(pMarketData->asset_type_id, lScale);
                            // ------------------------------------------------------
                            QString qstrFormattedScale = QString::fromStdString(str_scale);
                            // ------------------------------------------------------
                            qstrValue += QString(" (%1 %2)").arg(tr("priced per")).arg(qstrFormattedScale);
                        }
                    }
                }
                // -------------------------------------------------------------
                m_pOfferDetails->m_mapMarkets.insert(qstrID, qstrValue);
            }

            // -------------------------------------
//            mapIDName & the_map = m_pOfferDetails->m_map;

//            the_map.clear();
//            // -------------------------------------
//            if (!m_bHaveRetrievedOffersFirstTime)
//            {
//                m_bHaveRetrievedOffersFirstTime = true;
//                RetrieveOfferList(the_map); // Download the list of offers from the server(s).
//            }
//            else
//            {
//                LoadOfferList(the_map); // Load from local storage. (Let the user hit "Refresh" if he wants to re-download.)
//            }
//            // -------------------------------------
//            m_pOfferDetails->show_widget(MTDetailEdit::DetailEditTypeOffer);
        }
        // ***********************************************
    }
    else
    {
        m_pMarketDetails->setVisible(false);
        m_pOfferDetails ->setVisible(false);
    }
}


void DlgMarkets::onNeedToRefreshOffers(QString qstrMarketID)
{
    mapIDName & the_map = m_pOfferDetails->m_map;

    the_map.clear();
    // -------------------------------------
    ClearOfferMap();
    // -------------------------------------
    if (!m_bHaveRetrievedOffersFirstTime)
    {
        m_bHaveRetrievedOffersFirstTime = true;
        RetrieveOfferList(the_map, qstrMarketID); // Download the list of offers from the server(s).
    }
    else
    {
        LoadOfferList(the_map, qstrMarketID); // Load from local storage. (Let the user hit "Refresh" if he wants to re-download.)
    }
    // -------------------------------------
    m_pOfferDetails->show_widget(MTDetailEdit::DetailEditTypeOffer);
}


// Top level...
// (For servers and nyms.)
//
void DlgMarkets::RefreshRecords()
{
    ui->comboBoxServer->blockSignals(true);
    ui->comboBoxNym   ->blockSignals(true);
    // ----------------------------
    m_mapServers.clear();
    m_mapNyms   .clear();
    // ----------------------------
    ui->comboBoxServer->clear();
    ui->comboBoxNym   ->clear();
    // ----------------------------
    int nDefaultServerIndex = 0;
    int nDefaultNymIndex    = 0;
    // ----------------------------
    bool bFoundServerDefault = false;
    // ----------------------------
    QString qstrAllID   = "all";
    QString qstrAllName = tr("All Servers");

    m_mapServers.insert(qstrAllID, qstrAllName);
    ui->comboBoxServer->insertItem(0, qstrAllName);
    // ----------------------------
    if (!m_serverId.isEmpty() && (m_serverId == qstrAllID))
    {
        bFoundServerDefault = true;
        nDefaultServerIndex = 0;
    }
    // ----------------------------
    const int32_t server_count = OTAPI_Wrap::GetServerCount();
    // -----------------------------------------------
    for (int32_t ii = 0; ii < server_count; ++ii)
    {
        //Get OT Server ID
        //
        QString OT_server_id = QString::fromStdString(OTAPI_Wrap::GetServer_ID(ii));
        QString OT_server_name("");
        // -----------------------------------------------
        if (!OT_server_id.isEmpty())
        {
            if (!m_serverId.isEmpty() && (OT_server_id == m_serverId))
            {
                bFoundServerDefault = true;
                nDefaultServerIndex = ii+1; // the +1 is because of "all" in the 0 position. (Servers only.)
            }
            // -----------------------------------------------
            OT_server_name = QString::fromStdString(OTAPI_Wrap::GetServer_Name(OT_server_id.toStdString()));
            // -----------------------------------------------
            m_mapServers.insert(OT_server_id, OT_server_name);
            ui->comboBoxServer->insertItem(ii+1, OT_server_name);
        }
    }
    // -----------------------------------------------

    // -----------------------------------------------
    bool bFoundNymDefault = false;
    const int32_t nym_count = OTAPI_Wrap::GetNymCount();
    // -----------------------------------------------
    for (int32_t ii = 0; ii < nym_count; ++ii)
    {
        //Get OT Nym ID
        QString OT_nym_id = QString::fromStdString(OTAPI_Wrap::GetNym_ID(ii));
        QString OT_nym_name("");
        // -----------------------------------------------
        if (!OT_nym_id.isEmpty())
        {
            if (!m_nymId.isEmpty() && (OT_nym_id == m_nymId))
            {
                bFoundNymDefault = true;
                nDefaultNymIndex = ii;
            }
            // -----------------------------------------------
            MTNameLookupQT theLookup;

            OT_nym_name = QString::fromStdString(theLookup.GetNymName(OT_nym_id.toStdString()));
            // -----------------------------------------------
            m_mapNyms.insert(OT_nym_id, OT_nym_name);
            ui->comboBoxNym->insertItem(ii, OT_nym_name);
        }
     }
    // -----------------------------------------------

    // -----------------------------------------------
    if (m_mapNyms.size() > 0)
    {
        SetCurrentNymIDBasedOnIndex(nDefaultNymIndex);
        ui->comboBoxNym->setCurrentIndex(nDefaultNymIndex);
    }
    else
        SetCurrentNymIDBasedOnIndex(-1);
    // -----------------------------------------------
    if (m_mapServers.size() > 0)
    {
        SetCurrentServerIDBasedOnIndex(nDefaultServerIndex);
        ui->comboBoxServer->setCurrentIndex(nDefaultServerIndex);
    }
    else
        SetCurrentServerIDBasedOnIndex(-1);
    // -----------------------------------------------
    ui->comboBoxServer->blockSignals(false);
    ui->comboBoxNym   ->blockSignals(false);
    // -----------------------------------------------

    RefreshMarkets();

}


bool DlgMarkets::eventFilter(QObject *obj, QEvent *event)\
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Escape)
        {
            close(); // This is caught by this same filter.
            return true;
        }
        return true;
    }
    // -------------------------------------------
    // standard event processing
    return QObject::eventFilter(obj, event);
}

DlgMarkets::~DlgMarkets()
{
    delete ui;

    ClearMarketMap();
    ClearOfferMap();
}


