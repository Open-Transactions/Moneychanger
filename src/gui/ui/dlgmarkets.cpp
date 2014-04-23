#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/ui/dlgmarkets.hpp>
#include <ui_dlgmarkets.h>

#include <gui/widgets/editdetails.hpp>
#include <gui/widgets/overridecursor.hpp>

#include <core/moneychanger.hpp>

#include <opentxs/OTAPI.hpp>
#include <opentxs/OTAPI_Exec.hpp>
#include <opentxs/OT_ME.hpp>
#include <opentxs/OTStorage.hpp>
#include <opentxs/OTData.hpp>

#include <QKeyEvent>


DlgMarkets::DlgMarkets(QWidget *parent) :
    QDialog(parent),
    m_bFirstRun(true),
    m_bHaveRetrievedFirstTime(false),
    m_bHaveShownOffersFirstTime(false),
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
        // ----------------------------------------------------------------
        QPixmap pixmapContacts(":/icons/icons/user.png");
        QPixmap pixmapRefresh (":/icons/icons/refresh.png");
        // ----------------------------------------------------------------
        QIcon contactsButtonIcon(pixmapContacts);
        QIcon refreshButtonIcon (pixmapRefresh);
        // ----------------------------------------------------------------
        ui->toolButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButton->setAutoRaise(true);
        ui->toolButton->setIcon(refreshButtonIcon);
//      ui->toolButton->setIconSize(pixmapRefresh.rect().size());
        ui->toolButton->setIconSize(pixmapContacts.rect().size());
        ui->toolButton->setText(tr("Refresh"));

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
            connect(this,             SIGNAL(needToLoadOrRetrieveMarkets()),
                    m_pMarketDetails, SLOT(onSetNeedToRetrieveOfferTradeFlags()));
            // -------------------------------------
            connect(this, SIGNAL(needToLoadOrRetrieveMarkets()),
                    this, SLOT(LoadOrRetrieveMarkets()));
            // -------------------------------------
            connect(m_pMarketDetails, SIGNAL(CurrentMarketChanged(QString)),
                    this,             SLOT(onCurrentMarketChanged_Markets(QString)));
            // -------------------------------------
            // Connect market panel "current market changed" to *this "onCurrentMarketChanged
            connect(m_pMarketDetails, SIGNAL(CurrentMarketChanged(QString)),
                    this,             SLOT(onNeedToLoadOrRetrieveOffers(QString)));
            // -------------------------------------

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
            connect(m_pOfferDetails, SIGNAL(CurrentMarketChanged(QString)),
                    this,            SLOT(onNeedToLoadOrRetrieveOffers(QString)));
            // -------------------------------------
            connect(m_pOfferDetails, SIGNAL(NeedToLoadOrRetrieveOffers(QString)),
                    this,            SLOT(onNeedToLoadOrRetrieveOffers(QString)));
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

        m_pMarketDetails->SetMarketNymID   (m_nymId);
        m_pOfferDetails ->SetMarketNymID   (m_nymId);
        m_pMarketDetails->SetMarketServerID(m_serverId);
        m_pOfferDetails ->SetMarketServerID(m_serverId);
    }
}

// The "current market" has changed on the offers tab.
// (Thus, we need to notify the markets tab.)
//
void DlgMarkets::onCurrentMarketChanged_Offers (QString qstrMarketID)
{
//    m_pOfferDetails->SetMarketID(qstrMarketID);

    m_pMarketDetails->onMarketIDChangedFromAbove(qstrMarketID);

    // Not needed, since this is now triggered by the same signal
    // that triggered this function (onCurrentMarketChanged_Offers)
    // in the first place.
    //
//    onNeedToLoadOrRetrieveOffers(qstrMarketID);
}


// The "current market" has changed on the markets tab.
// (Thus, we need to notify the offers tab.)
//
void DlgMarkets::onCurrentMarketChanged_Markets(QString qstrMarketID)
{
    m_pOfferDetails->onMarketIDChangedFromAbove(qstrMarketID);
}

void DlgMarkets::onBalancesChangedFromAbove()
{
    emit needToLoadOrRetrieveMarkets();
}

void DlgMarkets::on_toolButton_clicked()
{
    m_bHaveRetrievedFirstTime = false; // To force RefreshRecords to re-download.

    m_pMarketDetails->ClearRecords();
    m_pOfferDetails-> ClearRecords();

    ClearMarketMap();
    ClearOfferMap();

    m_pOfferDetails->SetMarketID("");

    emit needToLoadOrRetrieveMarkets();

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
    if (m_pMarketDetails)
        m_pMarketDetails->SetMarketServerID(m_serverId);
    // ------------------------------------------
    if (m_pOfferDetails)
        m_pOfferDetails->SetMarketServerID(m_serverId);
    // ------------------------------------------
}

void DlgMarkets::on_comboBoxServer_currentIndexChanged(int index)
{
    m_pMarketDetails->ClearRecords();
    m_pOfferDetails->ClearRecords();

    ClearMarketMap();
    ClearOfferMap();

    SetCurrentServerIDBasedOnIndex(index);

    m_pOfferDetails->SetMarketID("");

//    RefreshRecords();
    emit needToLoadOrRetrieveMarkets();
}



void DlgMarkets::on_comboBoxNym_currentIndexChanged(int index)
{
    m_pMarketDetails->ClearRecords();
    m_pOfferDetails->ClearRecords();

    ClearMarketMap();
    ClearOfferMap();

    SetCurrentNymIDBasedOnIndex(index);

    m_pOfferDetails->SetMarketID("");

//    RefreshRecords();
    emit needToLoadOrRetrieveMarkets();
}

// -----------------------------------------------
bool DlgMarkets::RetrieveOfferList(mapIDName & the_map, QString qstrMarketID)
{
    if (m_serverId.isEmpty() || m_nymId.isEmpty())
        return false;
    // -----------------
    bool bSuccess = true;
    QString qstrAll(tr("all"));
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

    bool bSuccess = false;
    {
        MTSpinner theSpinner;

        const std::string str_reply = madeEasy.get_nym_market_offers(qstrServerID.toStdString(),
                                                                     qstrNymID   .toStdString());
        const int32_t     nResult   = madeEasy.VerifyMessageSuccess(str_reply);

        bSuccess = (1 == nResult);
    }
    // -----------------------------------
    if (bSuccess)
    {
        return LowLevelLoadOfferList(qstrServerID, m_nymId, the_map, qstrMarketID);
    }
    else
        Moneychanger::HasUsageCredits(this, qstrServerID, qstrNymID);
    // -----------------------------------
    return bSuccess;
}
// -----------------------------------------------
bool DlgMarkets::LoadOfferList(mapIDName & the_map, QString qstrMarketID)
{
    if (m_serverId.isEmpty() || m_nymId.isEmpty())
        return false;
    // -----------------
    bool bSuccess = true;
    QString qstrAll(tr("all"));
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
                QString qstrBuySell = pOfferData->selling ? tr("Sell") : tr("Buy");

                const std::string str_asset_name = OTAPI_Wrap::It()->GetAssetType_Name(pOfferData->asset_type_id);
                // --------------------------
                int64_t lTotalAssets   = OTAPI_Wrap::It()->StringToLong(pOfferData->total_assets);
                int64_t lFinishedSoFar = OTAPI_Wrap::It()->StringToLong(pOfferData->finished_so_far);
                // --------------------------
                const std::string str_total_assets    = OTAPI_Wrap::It()->FormatAmount(pOfferData->asset_type_id, lTotalAssets);
                const std::string str_finished_so_far = OTAPI_Wrap::It()->FormatAmount(pOfferData->asset_type_id, lFinishedSoFar);
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
                // NOTE that m_mapMarkets is a multimap, since there can be multiple markets with
                // the exact same ID and scale, across multiple servers. The single entry from MTMarketDetails::m_map
                // is then mapped to a group of entries in m_mapMarkets, or to a single entry by cross-referencing
                // the server ID.
                // Whereas in m_mapOffers, each Offer can be uniquely identified (regardless of server) by its unique key:
                // serverID,transactionID. Therefore MTOfferDetails::m_map and m_mapOffers are both maps (neither is a
                // multimap) and each offer is uniquely identified by that same key on both maps.
                // (That's why you see an insert() here instead of insertMulti.)
                //
                m_mapOffers.insert(qstrCompositeID, VPtr<OTDB::OfferDataNym>::asQVariant(pOfferData->clone()));
                // -----------------------------------------------------------------------
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
    QString qstrAll(tr("all"));
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
                const std::string str_asset_name    = OTAPI_Wrap::It()->GetAssetType_Name(pMarketData->asset_type_id);
                const std::string str_currency_name = OTAPI_Wrap::It()->GetAssetType_Name(pMarketData->currency_type_id);
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
    QString qstrAll(tr("all"));
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

    bool bSuccess = false;
    {
        MTSpinner theSpinner;

        const std::string str_reply = madeEasy.get_market_list(qstrServerID.toStdString(),
                                                               qstrNymID   .toStdString());
        const int32_t     nResult   = madeEasy.VerifyMessageSuccess(str_reply);

        bSuccess = (1 == nResult);
    }
    // -----------------------------------
    if (bSuccess)
    {
        return LowLevelLoadMarketList(qstrServerID, m_nymId, the_map);
    }
    else
        Moneychanger::HasUsageCredits(this, qstrServerID, qstrNymID);
    // -----------------------------------
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
void DlgMarkets::LoadOrRetrieveMarkets()
{
    if (!m_pMarketDetails || !m_pOfferDetails)
        return;
    // -----------------------------------------------
//  m_pMarketDetails->ClearRecords();
//  m_pOfferDetails ->ClearRecords();
    // -----------------------------------------------
    if (ui &&
        (ui->comboBoxNym   ->currentIndex() >=0) &&
        (ui->comboBoxServer->currentIndex() >=0))
    {
        m_pMarketDetails->setVisible(true);
        m_pOfferDetails ->setVisible(true);

        // ***********************************************
        {   // MARKET WIDGET (vs. Offers Widget)
            // -------------------------------------
            mapIDName & the_map = m_pMarketDetails->m_map;

            // -------------------------------------
//            if (!m_bHaveRetrievedFirstTime)
//            {
//                m_bHaveRetrievedFirstTime = true;
                RetrieveMarketList(the_map); // Download the list of markets from the server(s).
//            }
//            else
//            {
//                // TODO here: turn the "refresh" button Red.
//                //
//                LoadMarketList(the_map); // Load from local storage. (Let the user hit "Refresh" if he wants to re-download.)
//            }
            // -------------------------------------
            // Moving this to BELOW the spot (just below) where we put the same list of markets
            // into the combo box for the Offers panel.
            //
//            m_pMarketDetails->show_widget(MTDetailEdit::DetailEditTypeMarket);
        }
        // ***********************************************
        {
            // Set up the combo box data for the list of Markets
            // that appears on the OFFERS PAGE. (In a combo box.)

            m_pOfferDetails->m_mapMarkets.clear();
            // -------------------------------------
            // Loop through the ID/Display_name map in m_pMarketDetails.
            // For each ID/Display_Name, look up the pointer on DlgMarkets::m_mapMarkets (which
            // is a multimap of MarketData pointers.) Notice that there may be multiple entries there,
            // but we don't care -- any of them will do. We use it to look up the scale and asset type
            // ID for that market, so that we can format a custom display name for the combo box (drop-down)
            // that displays a list of markets on the Offers page. We thus set m_pOfferDetails->m_mapMarkets
            // to contain the same ID/Display_Name pair as on the Markets page, except with a custom
            // Display Name.
            //
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
                            std::string str_scale = OTAPI_Wrap::It()->FormatAmount(pMarketData->asset_type_id, lScale);
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

            // -------------------------------------------------------------

            // We show the offer details first (empty) so that it exists and is
            // properly initialized and ready to go, when it starts receiving signals
            // from the market panel as it begins refreshing for the first time.

            if (!m_bHaveShownOffersFirstTime)
            {
                m_bHaveShownOffersFirstTime = true;
                m_pOfferDetails->show_widget(MTDetailEdit::DetailEditTypeOffer);
            }

            // HOWEVER, after the first time this happens, we don't need it to happen
            // AGAIN, since now the object (m_pOfferDetails) is already created, and
            // m_pMarketDetails will ALREADY send it messages to refresh its offers
            // thereafter. Even the first time, since offers starts out blank, it is
            // actually the signals from this second call that actually cause the offers
            // panel to populate.
            // The first time here in LoadOrRetrieveMarkets(), we need to make sure offers panel
            // is created so markets panel can signal it properly. But the second time and
            // every time after that, we don't have to force refresh the offers panel before
            // the markets panel, since the markets panel will already trigger it to refresh
            // (and at the right time.)
            // Therefore you see above the check to make sure this only happens the first time.

            m_pMarketDetails->show_widget(MTDetailEdit::DetailEditTypeMarket);

            // -------------------------------------------------------------

            // We do NOT retrieve the offers yet (and have NOT shown the widget yet!!)
            // since that should be triggered by the showing of the market widget, which
            // has to populate its list of markets, THEN SELECT ONE, and only THEN should
            // we show the offers panel, since we need to know which market is selected
            // before we can show its markets.
            //
            // UPDATE: We still need to show_widget for the offers. It will have an empty
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
}


void DlgMarkets::onNeedToLoadOrRetrieveOffers(QString qstrMarketID)
{
    if (!m_pOfferDetails)
        return;

    mapIDName & the_map = m_pOfferDetails->m_map;
    // -------------------------------------
    the_map.clear(); // Clears m_pOfferDetails' map of OfferID to Display name
    // -------------------------------------
    ClearOfferMap(); // Clears *this' (DlgMarkets) map of OfferDataNym pointers.
    // -------------------------------------
    m_pOfferDetails->SetMarketID(qstrMarketID);

    // Note: this would fail to retrieve for every market except the first one.
    // Thus, we would have to store this boolean for EACH market, and not just
    // for the "first time" for the entire dialog.
    // Therefore for now, I am calling retrieve only, until a solution is worked
    // out for making those bools available here.
    //
//    if (!m_bHaveRetrievedOffersFirstTime)
//    {
//        m_bHaveRetrievedOffersFirstTime = true;

    if (!qstrMarketID.isEmpty())
        RetrieveOfferList(the_map, qstrMarketID); // Download the list of offers from the server(s).
//    }
//    else
//    {
//        LoadOfferList(the_map, qstrMarketID); // Load from local storage. (Let the user hit "Refresh" if he wants to re-download.)
//    }

    // -------------------------------------
    // Now that we've repopulated m_pOfferDetails->m_map and m_pOfferDetails->m_mapOffers,
    // (which MTOfferDetails sees as m_pOwner->m_pmapOffers) we need to Refresh the tablewidget
    // on m_pOfferDetails:
    //
    // -------------------------------------------
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
    ClearOfferMap();
    ClearMarketMap();
    // ----------------------------
    ui->comboBoxServer->clear();
    ui->comboBoxNym   ->clear();
    // ----------------------------
    int nDefaultServerIndex = 0;
    int nDefaultNymIndex    = 0;
    // ----------------------------
    bool bFoundServerDefault = false;
    // ----------------------------
    QString qstrAllID   = tr("all");
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
    const int32_t server_count = OTAPI_Wrap::It()->GetServerCount();
    // -----------------------------------------------
    for (int32_t ii = 0; ii < server_count; ++ii)
    {
        //Get OT Server ID
        //
        QString OT_server_id = QString::fromStdString(OTAPI_Wrap::It()->GetServer_ID(ii));
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
            OT_server_name = QString::fromStdString(OTAPI_Wrap::It()->GetServer_Name(OT_server_id.toStdString()));
            // -----------------------------------------------
            m_mapServers.insert(OT_server_id, OT_server_name);
            ui->comboBoxServer->insertItem(ii+1, OT_server_name);
        }
    }
    // -----------------------------------------------

    // -----------------------------------------------
    bool bFoundNymDefault = false;
    const int32_t nym_count = OTAPI_Wrap::It()->GetNymCount();
    // -----------------------------------------------
    for (int32_t ii = 0; ii < nym_count; ++ii)
    {
        //Get OT Nym ID
        QString OT_nym_id = QString::fromStdString(OTAPI_Wrap::It()->GetNym_ID(ii));
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
    emit needToLoadOrRetrieveMarkets();
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


