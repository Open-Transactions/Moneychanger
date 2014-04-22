#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/marketdetails.hpp>
#include <ui_marketdetails.h>

#include <gui/widgets/overridecursor.hpp>

#include <core/moneychanger.hpp>

#include <opentxs/OTAPI.hpp>
#include <opentxs/OTAPI_Exec.hpp>
#include <opentxs/OT_ME.hpp>
#include <opentxs/OTStorage.hpp>
#include <opentxs/OTData.hpp>

#include <QDateTime>
#include <QDebug>



// How many market offers should be downloaded?
// Clearly we can't download "them all" so we pick a number.
// ...Also, no matter what we pick, an OT server may have a max limit as well.
//
const int64_t  MAX_DEPTH = 20;


/*
MyClass *p;
QVariant v = VPtr<MyClass>::asQVariant(p);
aka:
VPtr<OTDB::MarketData>::asQVariant(pMarketData->clone())


MyClass *p1 = VPtr<MyClass>::asPtr(v);
aka:
OTDB::MarketData * pMarketData = VPtr<OTDB::MarketData>::asPtr(it_map.value());
*/


MTMarketDetails::MTMarketDetails(QWidget *parent, MTDetailEdit & theOwner) :
    MTEditDetails(parent, theOwner),
    m_pHeaderWidget(NULL),
    m_bNeedToRetrieveMarketOffers(true),
    m_bNeedToRetrieveMarketTrades(true),
    ui(new Ui::MTMarketDetails)
{
    ui->setupUi(this);
    this->setContentsMargins(0, 0, 0, 0);
//  this->installEventFilter(this); // NOTE: Successfully tested theory that the base class has already installed this.
    // ----------------------------------
    // Note: This is a placekeeper, so later on I can just erase
    // the widget at 0 and replace it with the real header widget.
    //
    m_pHeaderWidget  = new QWidget;
    ui->verticalLayout->insertWidget(0, m_pHeaderWidget);
    // ----------------------------------
    ui->tableWidgetBids->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetBids->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->tableWidgetBids->setSelectionMode    (QAbstractItemView::SingleSelection);
    ui->tableWidgetBids->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->tableWidgetBids->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetBids->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetBids->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetBids->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetBids->horizontalHeaderItem(4)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetBids->horizontalHeaderItem(5)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetBids->horizontalHeaderItem(6)->setTextAlignment(Qt::AlignCenter);
    // ----------------------------------
    ui->tableWidgetAsks->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetAsks->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->tableWidgetAsks->setSelectionMode    (QAbstractItemView::SingleSelection);
    ui->tableWidgetAsks->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->tableWidgetAsks->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetAsks->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetAsks->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetAsks->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetAsks->horizontalHeaderItem(4)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetAsks->horizontalHeaderItem(5)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetAsks->horizontalHeaderItem(6)->setTextAlignment(Qt::AlignCenter);
    // ----------------------------------
    ui->tableWidgetTrades->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetTrades->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->tableWidgetTrades->setSelectionMode    (QAbstractItemView::SingleSelection);
    ui->tableWidgetTrades->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->tableWidgetTrades->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetTrades->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetTrades->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetTrades->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetTrades->horizontalHeaderItem(4)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetTrades->horizontalHeaderItem(5)->setTextAlignment(Qt::AlignCenter);
    // ----------------------------------
    ui->tableWidgetBids   ->verticalHeader()->hide();
    ui->tableWidgetAsks   ->verticalHeader()->hide();
    ui->tableWidgetTrades ->verticalHeader()->hide();
    // ----------------------------------
    ui->lineEditAsset     ->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditAssetID   ->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditCurrency  ->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditCurrencyID->setStyleSheet("QLineEdit { background-color: lightgray }");
    // ----------------------------------
}


void MTMarketDetails::on_toolButtonAsset_clicked()
{
    if (!ui->lineEditAssetID->text().isEmpty())
        emit ShowAsset(ui->lineEditAssetID->text());
}

void MTMarketDetails::on_toolButtonCurrency_clicked()
{
    if (!ui->lineEditCurrencyID->text().isEmpty())
        emit ShowAsset(ui->lineEditCurrencyID->text());
}

void MTMarketDetails::ClearBidsGrid()
{
//    this->blockSignals(true);
    ui->tableWidgetBids->blockSignals(true);
    // -----------------------------------
    ui->tableWidgetBids->clearContents();
    ui->tableWidgetBids->setRowCount (0);
    // -----------------------------------
//    this->blockSignals(false);
    ui->tableWidgetBids->blockSignals(false);
}

void MTMarketDetails::ClearAsksGrid()
{
//    this->blockSignals(true);
    ui->tableWidgetAsks->blockSignals(true);
    // -----------------------------------
    ui->tableWidgetAsks->clearContents();
    ui->tableWidgetAsks->setRowCount (0);
    // -----------------------------------
//    this->blockSignals(false);
    ui->tableWidgetAsks->blockSignals(false);
}

void MTMarketDetails::ClearTradesGrid()
{
//    this->blockSignals(true);
    ui->tableWidgetTrades->blockSignals(true);
    // -----------------------------------
    ui->tableWidgetTrades->clearContents();
    ui->tableWidgetTrades->setRowCount (0);
    // -----------------------------------
//    this->blockSignals(false);
    ui->tableWidgetTrades->blockSignals(false);
}

// ------------------------------------------------------------

void MTMarketDetails::RetrieveMarketOffers(QString & qstrID, QMultiMap<QString, QVariant> & multimap)
{
    QMap<QString, QVariant>::iterator it_market = multimap.find(qstrID);
    // -----------------------------
    while ((multimap.end() != it_market) && (it_market.key() == qstrID))
    {
        OTDB::MarketData * pMarketData = VPtr<OTDB::MarketData>::asPtr(it_market.value());

        if (NULL != pMarketData) // Should never be NULL.
        {
            LowLevelRetrieveMarketOffers(*pMarketData);
        }
        // --------------------
        ++it_market;
    }
}

// ----------------------------------------------------

bool MTMarketDetails::LowLevelRetrieveMarketOffers(OTDB::MarketData & marketData)
{
    if (!m_pOwner)
        return false;
    // ------------------------------
    QString     qstrNymID = m_pOwner->GetMarketNymID();
    std::string  strNymID = qstrNymID.toStdString();
    // ------------------------------
    if (strNymID.empty())
        return false;
    // ------------------------------
    OT_ME madeEasy;

    bool bSuccess = false;
    {
        MTSpinner         theSpinner;
        const std::string str_reply = madeEasy.get_market_offers(marketData.server_id, strNymID,
                                                                 marketData.market_id, MAX_DEPTH);
        const int32_t     nResult   = madeEasy.VerifyMessageSuccess(str_reply);

        bSuccess = (1 == nResult);
    }
    // -----------------------------------
    if (!bSuccess)
        Moneychanger::HasUsageCredits(this, marketData.server_id, strNymID);
    // -----------------------------------
    return bSuccess;
}

// Caller must delete.
OTDB::OfferListMarket * MTMarketDetails::LoadOfferListForMarket(OTDB::MarketData & marketData)
{
    OTDB::OfferListMarket * pOfferList = NULL;
    OTDB::Storable        * pStorable  = NULL;
    // ------------------------------------------
    QString qstrFilename = QString("%1.bin").arg(QString::fromStdString(marketData.market_id));

    if (OTDB::Exists("markets", marketData.server_id, "offers", qstrFilename.toStdString()))
    {
        pStorable = OTDB::QueryObject(OTDB::STORED_OBJ_OFFER_LIST_MARKET, "markets",
                                      marketData.server_id, "offers", qstrFilename.toStdString());
        if (NULL == pStorable)
            return NULL;
        // -------------------------------
        pOfferList = OTDB::OfferListMarket::ot_dynamic_cast(pStorable);

        if (NULL == pOfferList)
            delete pStorable;
    }

    return pOfferList;
}

// ----------------------------------

// public slot
void MTMarketDetails::onSetNeedToRetrieveOfferTradeFlags()
{
    m_bNeedToRetrieveMarketOffers = true;
    m_bNeedToRetrieveMarketTrades = true;
}

void MTMarketDetails::PopulateMarketOffersGrids(QString & qstrID, QMultiMap<QString, QVariant> & multimap)
{
    if (m_bNeedToRetrieveMarketOffers)
    {
        m_bNeedToRetrieveMarketOffers = false;
        // ------------------------------------
        RetrieveMarketOffers(qstrID, multimap);
    }
    // ------------------------------------------------------

//    this->blockSignals(true);
    // -----------------------------------
    ui->tableWidgetBids->blockSignals(true);
    // -----------------------------------
    ui->tableWidgetAsks->blockSignals(true);
    // -----------------------------------
    int nBidsRowCount = 0;
    int nAsksRowCount = 0;
    // -----------------------------------
    int nBidsGridIndex = 0;
    int nAsksGridIndex = 0;
    // -----------------------------------
    QMap<QString, QVariant>::iterator it_market = multimap.find(qstrID);
    // -----------------------------
    while ((multimap.end() != it_market) && (it_market.key() == qstrID))
    {
        OTDB::MarketData * pMarketData = VPtr<OTDB::MarketData>::asPtr(it_market.value());

        if (NULL != pMarketData) // Should never be NULL.
        {
            std::string & str_server         = pMarketData->server_id;
            std::string   str_server_display = OTAPI_Wrap::It()->GetServer_Name(str_server);
            QString       qstrServerName     = QString::fromStdString(str_server_display);
            // -----------------------------------------
            int64_t lScale = OTAPI_Wrap::It()->StringToLong(pMarketData->scale);

            const std::string str_price_per_scale(OTAPI_Wrap::It()->FormatAmount(pMarketData->asset_type_id,
                                                                           lScale));
            // -----------------------------------------
            {
                // BIDS
                QTableWidgetItem * pPriceHeader = ui->tableWidgetBids->horizontalHeaderItem(0);

                if (NULL != pPriceHeader)
                {
                    pPriceHeader->setText(QString("%1 %2").arg(tr("Price per")).
                                          arg(QString::fromStdString(str_price_per_scale)));
                }
            }
            // -----------------------------------------
            {
                // ASKS
                QTableWidgetItem * pPriceHeader = ui->tableWidgetAsks->horizontalHeaderItem(0);

                if (NULL != pPriceHeader)
                {
                    pPriceHeader->setText(QString("%1 %2").arg(tr("Price per")).
                                          arg(QString::fromStdString(str_price_per_scale)));
                }
            }
            // -----------------------------------------
            OTDB::OfferListMarket * pOfferList = LoadOfferListForMarket(*pMarketData);
            OTCleanup<OTDB::OfferListMarket> theAngel(pOfferList);

            if (NULL != pOfferList)
            {
                size_t nBidDataCount = pOfferList->GetBidDataCount();
                size_t nAskDataCount = pOfferList->GetAskDataCount();
                // -------------------------------------
                nBidsRowCount += static_cast<int>(nBidDataCount);
                nAsksRowCount += static_cast<int>(nAskDataCount);
                // -------------------------------------
                ui->tableWidgetBids->setRowCount(nBidsRowCount);
                ui->tableWidgetAsks->setRowCount(nAsksRowCount);
                // -------------------------------------
                for (size_t bid_index = 0; bid_index < nBidDataCount; ++bid_index)
                {
                    OTDB::BidData * pData = pOfferList->GetBidData(bid_index);

                    if (NULL == pData) // Should never happen.
                        continue;
                    // -----------------------------------------------------------------------
                    QString qstrTransactionID = QString::fromStdString(pData->transaction_id);
                    // -----------------------------------------------------------------------
                    std::string & str_price         = pData->price_per_scale;
                    int64_t       lPrice            = OTAPI_Wrap::It()->StringToLong(str_price); // this price is "per scale"
                    std::string   str_price_display = OTAPI_Wrap::It()->FormatAmount(pMarketData->currency_type_id, lPrice);

                    QString qstrPrice = QString::fromStdString(str_price_display);
                    // -----------------------------------------------------------------------
                    std::string & str_available         = pData->available_assets;
                    int64_t       lQuantity             = OTAPI_Wrap::It()->StringToLong(str_available); // Total overall quantity available
                    std::string   str_available_display = OTAPI_Wrap::It()->FormatAmount(pMarketData->asset_type_id, lQuantity);

                    QString qstrAvailable = QString::fromStdString(str_available_display);
                    // -----------------------------------------------------------------------
                    std::string & str_min_inc         = pData->minimum_increment;
                    int64_t       lMinInc             = OTAPI_Wrap::It()->StringToLong(str_min_inc);
                    std::string   str_min_inc_display = OTAPI_Wrap::It()->FormatAmount(pMarketData->asset_type_id, lMinInc);

                    QString qstrMinInc = QString::fromStdString(str_min_inc_display);
                    // -----------------------------------------------------------------------
                    time_t tDate = static_cast<time_t>(OTAPI_Wrap::It()->StringToLong(pData->date));

                    QDateTime qdate_added   = QDateTime::fromTime_t(tDate);
                    QString   qstrDateAdded = qdate_added.toString(QString("MMM d yyyy hh:mm:ss"));
                    // -----------------------------------------------------------------------
                    // Used as intermediary value for calculating lTotalCost.
                    //
                    int64_t lScaleUnits = 0; // Number of scale units available in total quantity. (120 total at scale of 10, is 12 units.)

                    if (lScale > 0)
                    {
                        double dScaleUnits = (static_cast<double>(lQuantity) / static_cast<double>(lScale));
                        lScaleUnits = static_cast<int64_t>(dScaleUnits);
                    }
                    // -----------------------------------------------------------------------
                    int64_t       lTotalCost     = (lPrice * lScaleUnits);
                    std::string   str_total_cost = OTAPI_Wrap::It()->FormatAmount(pMarketData->currency_type_id,
                                                                            lTotalCost);

                    QString qstrTotalCost = QString::fromStdString(str_total_cost);
                    // -----------------------------------------------------------------------
                    QLabel * pLabelPrice     = new QLabel(qstrPrice);
                    QLabel * pLabelAvailable = new QLabel(qstrAvailable);
                    QLabel * pLabelTotalCost = new QLabel(qstrTotalCost);
                    QLabel * pLabelDateAdded = new QLabel(QString("<small>%1</small>").arg(qstrDateAdded));
                    QLabel * pLabelTransID   = new QLabel(qstrTransactionID);
                    QLabel * pLabelMinInc    = new QLabel(qstrMinInc);
                    QLabel * pLabelServer    = new QLabel(qstrServerName);
                    // -----------------------------------------------------------------------
                    pLabelPrice    ->setAlignment(Qt::AlignCenter);
                    pLabelAvailable->setAlignment(Qt::AlignCenter);
                    pLabelTotalCost->setAlignment(Qt::AlignCenter);
                    pLabelDateAdded->setAlignment(Qt::AlignCenter);
                    pLabelTransID  ->setAlignment(Qt::AlignCenter);
                    pLabelMinInc   ->setAlignment(Qt::AlignCenter);
                    pLabelServer   ->setAlignment(Qt::AlignLeft);
                    // -----------------------------------------------------------------------
                    ui->tableWidgetBids->setCellWidget ( nBidsGridIndex, 0, pLabelPrice     );
                    ui->tableWidgetBids->setCellWidget ( nBidsGridIndex, 1, pLabelAvailable );
                    ui->tableWidgetBids->setCellWidget ( nBidsGridIndex, 2, pLabelTotalCost );
                    ui->tableWidgetBids->setCellWidget ( nBidsGridIndex, 3, pLabelDateAdded );
                    ui->tableWidgetBids->setCellWidget ( nBidsGridIndex, 4, pLabelTransID   );
                    ui->tableWidgetBids->setCellWidget ( nBidsGridIndex, 5, pLabelMinInc    );
                    ui->tableWidgetBids->setCellWidget ( nBidsGridIndex, 6, pLabelServer    );
                    // -----------------------------------------------------------------------
                    ++nBidsGridIndex;
                    // -----------------------------------------------------------------------
                } // for (bids)
                // -----------------------------------------------------------------------
                for (size_t ask_index = 0; ask_index < nAskDataCount; ++ask_index)
                {
                    OTDB::AskData * pData = pOfferList->GetAskData(ask_index);

                    if (NULL == pData) // Should never happen.
                        continue;
                    // -----------------------------------------------------------------------
                    QString qstrTransactionID = QString::fromStdString(pData->transaction_id);
                    // -----------------------------------------------------------------------
                    std::string & str_price         = pData->price_per_scale;
                    int64_t       lPrice            = OTAPI_Wrap::It()->StringToLong(str_price); // this price is "per scale"
                    std::string   str_price_display = OTAPI_Wrap::It()->FormatAmount(pMarketData->currency_type_id, lPrice);

                    QString qstrPrice = QString::fromStdString(str_price_display);
                    // -----------------------------------------------------------------------
                    std::string & str_available         = pData->available_assets;
                    int64_t       lQuantity             = OTAPI_Wrap::It()->StringToLong(str_available); // Total overall quantity available
                    std::string   str_available_display = OTAPI_Wrap::It()->FormatAmount(pMarketData->asset_type_id, lQuantity);

                    QString qstrAvailable = QString::fromStdString(str_available_display);
                    // -----------------------------------------------------------------------
                    std::string & str_min_inc         = pData->minimum_increment;
                    int64_t       lMinInc             = OTAPI_Wrap::It()->StringToLong(str_min_inc);
                    std::string   str_min_inc_display = OTAPI_Wrap::It()->FormatAmount(pMarketData->asset_type_id, lMinInc);

                    QString qstrMinInc = QString::fromStdString(str_min_inc_display);
                    // -----------------------------------------------------------------------
                    time_t tDate = static_cast<time_t>(OTAPI_Wrap::It()->StringToLong(pData->date));

                    QDateTime qdate_added   = QDateTime::fromTime_t(tDate);
                    QString   qstrDateAdded = qdate_added.toString(QString("MMM d yyyy hh:mm:ss"));
                    // -----------------------------------------------------------------------
                    // Used as intermediary value for calculating lTotalCost.
                    //
                    int64_t lScaleUnits = 0; // Number of scale units available in total quantity. (120 total at scale of 10, is 12 units.)

                    if (lScale > 0)
                    {
                        double dScaleUnits = (static_cast<double>(lQuantity) / static_cast<double>(lScale));
                        lScaleUnits = static_cast<int64_t>(dScaleUnits);
                    }
                    // -----------------------------------------------------------------------
                    int64_t       lTotalCost     = (lPrice * lScaleUnits);
                    std::string   str_total_cost = OTAPI_Wrap::It()->FormatAmount(pMarketData->currency_type_id, lTotalCost);

                    QString qstrTotalCost = QString::fromStdString(str_total_cost);
                    // -----------------------------------------------------------------------
                    QLabel * pLabelPrice     = new QLabel(qstrPrice);
                    QLabel * pLabelAvailable = new QLabel(qstrAvailable);
                    QLabel * pLabelTotalCost = new QLabel(qstrTotalCost);
                    QLabel * pLabelDateAdded = new QLabel(QString("<small>%1</small>").arg(qstrDateAdded));
                    QLabel * pLabelTransID   = new QLabel(qstrTransactionID);
                    QLabel * pLabelMinInc    = new QLabel(qstrMinInc);
                    QLabel * pLabelServer    = new QLabel(qstrServerName);
                    // -----------------------------------------------------------------------
                    pLabelPrice    ->setAlignment(Qt::AlignCenter);
                    pLabelAvailable->setAlignment(Qt::AlignCenter);
                    pLabelTotalCost->setAlignment(Qt::AlignCenter);
                    pLabelDateAdded->setAlignment(Qt::AlignCenter);
                    pLabelTransID  ->setAlignment(Qt::AlignCenter);
                    pLabelMinInc   ->setAlignment(Qt::AlignCenter);
                    pLabelServer   ->setAlignment(Qt::AlignLeft);
                    // -----------------------------------------------------------------------
                    ui->tableWidgetAsks->setCellWidget ( nAsksGridIndex, 0, pLabelPrice     );
                    ui->tableWidgetAsks->setCellWidget ( nAsksGridIndex, 1, pLabelAvailable );
                    ui->tableWidgetAsks->setCellWidget ( nAsksGridIndex, 2, pLabelTotalCost );
                    ui->tableWidgetAsks->setCellWidget ( nAsksGridIndex, 3, pLabelDateAdded );
                    ui->tableWidgetAsks->setCellWidget ( nAsksGridIndex, 4, pLabelTransID   );
                    ui->tableWidgetAsks->setCellWidget ( nAsksGridIndex, 5, pLabelMinInc    );
                    ui->tableWidgetAsks->setCellWidget ( nAsksGridIndex, 6, pLabelServer    );
                    // -----------------------------------------------------------------------
                    ++nAsksGridIndex;
                    // -----------------------------------------------------------------------
                } // for (asks)
                // -----------------------------------------------------------------------
            } // if (NULL != pOfferList)
        } // if (NULL != pMarketData)
        // -----------------
        ++it_market;
    } // while
    // -----------------------------------------------------
//    this->blockSignals(false);
    // -----------------------------------
    ui->tableWidgetBids->blockSignals(false);
    ui->tableWidgetAsks->blockSignals(false);
    // -----------------------------------------------------
    if (ui->tableWidgetBids->rowCount() > 0)
        ui->tableWidgetBids->setCurrentCell(0, 0);
    // -----------------------------------------------------
    if (ui->tableWidgetAsks->rowCount() > 0)
        ui->tableWidgetAsks->setCurrentCell(0, 0);
    // -----------------------------------------------------
}


void MTMarketDetails::RetrieveMarketTrades(QString & qstrID, QMultiMap<QString, QVariant> & multimap)
{
    QMap<QString, QVariant>::iterator it_market = multimap.find(qstrID);
    // -----------------------------
    while ((multimap.end() != it_market) && (it_market.key() == qstrID))
    {
        OTDB::MarketData * pMarketData = VPtr<OTDB::MarketData>::asPtr(it_market.value());

        if (NULL != pMarketData) // Should never be NULL.
        {
            LowLevelRetrieveMarketTrades(*pMarketData);
        }
        // --------------------
        ++it_market;
    }
}

// ----------------------------------------------------

bool MTMarketDetails::LowLevelRetrieveMarketTrades(OTDB::MarketData & marketData)
{
    if (!m_pOwner)
        return false;
    // ------------------------------
    QString     qstrNymID = m_pOwner->GetMarketNymID();
    std::string  strNymID = qstrNymID.toStdString();
    // ------------------------------
    if (strNymID.empty())
        return false;
    // ------------------------------
    OT_ME madeEasy;

    bool  bSuccess = false;
    {
        MTSpinner theSpinner;

        const std::string str_reply = madeEasy.get_market_recent_trades(marketData.server_id, strNymID,
                                                                        marketData.market_id);
        const int32_t     nResult   = madeEasy.VerifyMessageSuccess(str_reply);

        bSuccess = (1 == nResult);
    }
    // ---------------
    if (!bSuccess)
        Moneychanger::HasUsageCredits(this, marketData.server_id, strNymID);
    // -----------------------------------
    return bSuccess;
}

// Caller must delete.
OTDB::TradeListMarket * MTMarketDetails::LoadTradeListForMarket(OTDB::MarketData & marketData)
{
    OTDB::TradeListMarket * pTradeList = NULL;
    OTDB::Storable        * pStorable  = NULL;
    // ------------------------------------------
    QString qstrFilename = QString("%1.bin").arg(QString::fromStdString(marketData.market_id));

    if (OTDB::Exists("markets", marketData.server_id, "recent", qstrFilename.toStdString()))
    {
        pStorable = OTDB::QueryObject(OTDB::STORED_OBJ_TRADE_LIST_MARKET, "markets",
                                      marketData.server_id, "recent", qstrFilename.toStdString());
        if (NULL == pStorable)
            return NULL;
        // -------------------------------
        pTradeList = OTDB::TradeListMarket::ot_dynamic_cast(pStorable);

        if (NULL == pTradeList)
            delete pStorable;
    }

    return pTradeList;
}


void MTMarketDetails::PopulateRecentTradesGrid(QString & qstrID, QMultiMap<QString, QVariant> & multimap)
{
    // revisit.
//    if (m_bNeedToRetrieveMarketTrades)
//    {
//        m_bNeedToRetrieveMarketTrades = false;
        // --------------------------------------
        RetrieveMarketTrades(qstrID, multimap);
//    }
    // ------------------------------------------------------------------------
//    this->blockSignals(true);
    // -----------------------------------
    ui->tableWidgetTrades->blockSignals(true);
    // -----------------------------------
    int nTradesRowCount  = 0;
    // -----------------------------------
    int nTradesGridIndex = 0;
    // -----------------------------------
    QMap<QString, QVariant>::iterator it_market = multimap.find(qstrID);
    // -----------------------------
    while ((multimap.end() != it_market) && (it_market.key() == qstrID))
    {
        OTDB::MarketData * pMarketData = VPtr<OTDB::MarketData>::asPtr(it_market.value());

        if (NULL != pMarketData) // Should never be NULL.
        {
            std::string & str_server         = pMarketData->server_id;
            std::string   str_server_display = OTAPI_Wrap::It()->GetServer_Name(str_server);
            QString       qstrServerName     = QString::fromStdString(str_server_display);
            // -----------------------------------------
            int64_t lScale = OTAPI_Wrap::It()->StringToLong(pMarketData->scale);
            // -----------------------------------------
            QTableWidgetItem * pPriceHeader = ui->tableWidgetTrades->horizontalHeaderItem(0);

            if (NULL != pPriceHeader)
            {
                const std::string str_price_per_scale(OTAPI_Wrap::It()->FormatAmount(pMarketData->asset_type_id,
                                                                               lScale));
                pPriceHeader->setText(QString("%1 %2").arg(tr("Price per")).
                                      arg(QString::fromStdString(str_price_per_scale)));
            }
            // -----------------------------------------
            OTDB::TradeListMarket * pTradeList = LoadTradeListForMarket(*pMarketData);
            OTCleanup<OTDB::TradeListMarket> theAngel(pTradeList);

            if (NULL != pTradeList)
            {
                size_t nTradeDataCount = pTradeList->GetTradeDataMarketCount();
                // -------------------------------------
                nTradesRowCount += static_cast<int>(nTradeDataCount);
                // -------------------------------------
                ui->tableWidgetTrades->setRowCount(nTradesRowCount);
                // -------------------------------------
                for (size_t trade_index = 0; trade_index < nTradeDataCount; ++trade_index)
                {
                    OTDB::TradeDataMarket * pData = pTradeList->GetTradeDataMarket(trade_index);

                    if (NULL == pData) // Should never happen.
                        continue;
                    // -----------------------------------------------------------------------
                    QString qstrTransactionID = QString::fromStdString(pData->transaction_id);
                    // -----------------------------------------------------------------------
                    time_t tDate = static_cast<time_t>(OTAPI_Wrap::It()->StringToLong(pData->date));

                    QDateTime qdate_added   = QDateTime::fromTime_t(tDate);
                    QString   qstrDateAdded = qdate_added.toString(QString("MMM d yyyy hh:mm:ss"));
                    // -----------------------------------------------------------------------
                    std::string & str_price         = pData->price;
                    int64_t       lPrice            = OTAPI_Wrap::It()->StringToLong(str_price); // this price is "per scale"
                    std::string   str_price_display = OTAPI_Wrap::It()->FormatAmount(pMarketData->currency_type_id, lPrice);

                    QString qstrPrice = QString::fromStdString(str_price_display);
                    // -----------------------------------------------------------------------
                    std::string & str_amount_sold    = pData->amount_sold;
                    int64_t       lQuantity          = OTAPI_Wrap::It()->StringToLong(str_amount_sold); // Total amount of asset sold.
                    std::string   str_amount_display = OTAPI_Wrap::It()->FormatAmount(pMarketData->asset_type_id, lQuantity);

                    QString qstrAmountSold = QString::fromStdString(str_amount_display);
                    // -----------------------------------------------------------------------
                    // Used as intermediary value for calculating lTotalCost.
                    //
                    int64_t lScaleUnits = 0; // Number of scale units available in total quantity. (120 total at scale of 10, is 12 units.)

                    if (lScale > 0)
                    {
                        double dScaleUnits = (static_cast<double>(lQuantity) / static_cast<double>(lScale));
                        lScaleUnits = static_cast<int64_t>(dScaleUnits);
                    }
                    // -----------------------------------------------------------------------
                    int64_t       lTotalCost     = (lPrice * lScaleUnits);
                    std::string   str_total_cost = OTAPI_Wrap::It()->FormatAmount(pMarketData->currency_type_id, lTotalCost);

                    QString qstrTotalCost = QString::fromStdString(str_total_cost);
                    // -----------------------------------------------------------------------
                    QLabel * pLabelPrice      = new QLabel(qstrPrice);
                    QLabel * pLabelAmountSold = new QLabel(qstrAmountSold);
                    QLabel * pLabelTotalCost  = new QLabel(qstrTotalCost);
                    QLabel * pLabelDateAdded  = new QLabel(QString("<small>%1</small>").arg(qstrDateAdded));
                    QLabel * pLabelTransID    = new QLabel(qstrTransactionID);
                    QLabel * pLabelServer     = new QLabel(qstrServerName);
                    // -----------------------------------------------------------------------
                    pLabelPrice     ->setAlignment(Qt::AlignCenter);
                    pLabelAmountSold->setAlignment(Qt::AlignCenter);
                    pLabelTotalCost ->setAlignment(Qt::AlignCenter);
                    pLabelDateAdded ->setAlignment(Qt::AlignCenter);
                    pLabelTransID   ->setAlignment(Qt::AlignCenter);
                    pLabelServer    ->setAlignment(Qt::AlignLeft);
                    // -----------------------------------------------------------------------
                    ui->tableWidgetTrades->setCellWidget ( nTradesGridIndex, 0, pLabelPrice     );
                    ui->tableWidgetTrades->setCellWidget ( nTradesGridIndex, 1, pLabelAmountSold);
                    ui->tableWidgetTrades->setCellWidget ( nTradesGridIndex, 2, pLabelTotalCost );
                    ui->tableWidgetTrades->setCellWidget ( nTradesGridIndex, 3, pLabelDateAdded );
                    ui->tableWidgetTrades->setCellWidget ( nTradesGridIndex, 4, pLabelTransID   );
                    ui->tableWidgetTrades->setCellWidget ( nTradesGridIndex, 5, pLabelServer    );
                    // -----------------------------------------------------------------------
                    ++nTradesGridIndex;
                    // -----------------------------------------------------------------------
                } // for (trades)
                // -----------------------------------------------------------------------
            } // if (NULL != pTradeList)
        } // if (NULL != pMarketData)
        // -----------------
        ++it_market;
    } // while
    // -----------------------------------------------------
//    this->blockSignals(false);
    // -----------------------------------
    ui->tableWidgetTrades->blockSignals(false);
    // -----------------------------------------------------
    if (ui->tableWidgetTrades->rowCount() > 0)
        ui->tableWidgetTrades->setCurrentCell(0, 0);
    // -----------------------------------------------------
}

// ------------------------------------------------------------------------

void MTMarketDetails::refresh(QString strID, QString strName)
{
//    this->blockSignals(true);
    // -----------------------------------
    ui->tableWidgetBids  ->blockSignals(true);
    ui->tableWidgetAsks  ->blockSignals(true);
    ui->tableWidgetTrades->blockSignals(true);
    // -----------------------------------
    ui->tableWidgetBids->clearContents();
    ui->tableWidgetBids->setRowCount (0);
    // -----------------------------------
    ui->tableWidgetAsks->clearContents();
    ui->tableWidgetAsks->setRowCount (0);
    // -----------------------------------
    ui->tableWidgetTrades->clearContents();
    ui->tableWidgetTrades->setRowCount (0);
    // ----------------------------------------
    ui->tableWidgetBids  ->blockSignals(false);
    ui->tableWidgetAsks  ->blockSignals(false);
    ui->tableWidgetTrades->blockSignals(false);
    // ----------------------------------------
//  this->blockSignals(false);
    // ----------------------------------------
    if (!strID.isEmpty() && (NULL != ui))
    {
        // FYI, contents of strID:
//      QString qstrCompositeID = QString("%1,%2").arg(qstrMarketID).arg(qstrScale);

        QString     qstrMarketID, qstrMarketScale;
        QStringList theIDs = strID.split(","); // theIDs.at(0) MarketID, at(1) market Scale

        if (2 == theIDs.size()) // Should always be 2...
        {
            qstrMarketID    = theIDs.at(0);
            qstrMarketScale = theIDs.at(1);
        }
        // -------------------------------------
        if (m_pOwner && m_pOwner->m_pmapMarkets)
        {
            QMap<QString, QVariant>::iterator it_market = m_pOwner->m_pmapMarkets->find(strID);

            if (m_pOwner->m_pmapMarkets->end() != it_market)
            {
                // ------------------------------------------------------
                OTDB::MarketData * pMarketData = VPtr<OTDB::MarketData>::asPtr(it_market.value());

                if (NULL != pMarketData) // Should never be NULL.
                {
                    // ------------------------------------------------------
                    int64_t     lScale    = OTAPI_Wrap::It()->StringToLong(pMarketData->scale);
                    std::string str_scale = OTAPI_Wrap::It()->FormatAmount(pMarketData->asset_type_id, lScale);
                    // ------------------------------------------------------
                    QString qstrFormattedScale = QString::fromStdString(str_scale);
                    // ------------------------------------------------------
                    QString qstrTotalAssets   = CalculateTotalAssets(strID, *(m_pOwner->m_pmapMarkets));
                    QString qstrDisplayScale;

                    qstrDisplayScale = QString("%1: %2, %3 %4").
                            arg(tr("Total Assets")).arg(qstrTotalAssets).arg(tr("priced per")).
                            arg(qstrFormattedScale);
                    // ------------------------------------------------------
                    QString qstrCurrentBid    = CalculateCurrentBid    (strID, *(m_pOwner->m_pmapMarkets));
                    QString qstrCurrentAsk    = CalculateCurrentAsk    (strID, *(m_pOwner->m_pmapMarkets));

                    QString qstrDisplayOffers = QString("<font size=1 color=grey>%1:</font> %2&nbsp;&nbsp;&nbsp;"
                                                        "<font size=1 color=grey>%3:</font> %4").
                            arg(tr("Bid")).arg(qstrCurrentBid).arg(tr("Ask")).arg(qstrCurrentAsk);
                    // -------------------------------------
                    QWidget * pHeaderWidget = MTEditDetails::CreateDetailHeaderWidget(m_Type, qstrDisplayScale, strName, qstrDisplayOffers, "", ":/icons/markets", false);

                    pHeaderWidget->setObjectName(QString("DetailHeader")); // So the stylesheet doesn't get applied to all its sub-widgets.

                    if (m_pHeaderWidget)
                    {
                        ui->verticalLayout->removeWidget(m_pHeaderWidget);

                        m_pHeaderWidget->setParent(NULL);
                        m_pHeaderWidget->disconnect();
                        m_pHeaderWidget->deleteLater();

                        m_pHeaderWidget = NULL;
                    }
                    ui->verticalLayout->insertWidget(0, pHeaderWidget);
                    m_pHeaderWidget = pHeaderWidget;
                    // ------------------------------------------------------
                    // For these, we're doing it this way because we're calculating
                    // these values across multiple markets. (The same market type,
                    // but across multiple servers.)
                    //
                    // ------------------------------------------------------
                    ui->lineEditAsset       ->setText(QString::fromStdString(OTAPI_Wrap::It()->GetAssetType_Name(pMarketData->asset_type_id)));
                    ui->lineEditCurrency    ->setText(QString::fromStdString(OTAPI_Wrap::It()->GetAssetType_Name(pMarketData->currency_type_id)));
                    // ------------------------------------------------------
                    ui->lineEditAssetID     ->setText(QString::fromStdString(pMarketData->asset_type_id));
                    ui->lineEditCurrencyID  ->setText(QString::fromStdString(pMarketData->currency_type_id));
                    // ------------------------------------------------------
                    QString qstrNumberBids    = CalculateNumberBids    (strID, *(m_pOwner->m_pmapMarkets));
                    QString qstrNumberAsks    = CalculateNumberAsks    (strID, *(m_pOwner->m_pmapMarkets));
                    QString qstrLastSalePrice = CalculateLastSalePrice (strID, *(m_pOwner->m_pmapMarkets));
                    // ------------------------------------------------------
                    ui->labelNumberBidsValue->setText(qstrNumberBids);
                    ui->labelNumberAsksValue->setText(qstrNumberAsks);
                    ui->labelLastValue      ->setText(qstrLastSalePrice);
                    // ------------------------------------------------------
                    PopulateMarketOffersGrids(strID, *(m_pOwner->m_pmapMarkets));
                    PopulateRecentTradesGrid (strID, *(m_pOwner->m_pmapMarkets));
                    // ------------------------------------------------------
                }
            }
        }
        // ----------------------------------
        FavorLeftSideForIDs();
        // ----------------------------------
    }
}



// ----------------------------------
QString MTMarketDetails::CalculateTotalAssets(QString & qstrID, QMultiMap<QString, QVariant> & multimap)
{
    QString qstrReturnValue("");
    // -----------------------------
    bool    bFirstIteration = true;
    int64_t lTotal = 0;
    // -----------------------------
    QMap<QString, QVariant>::iterator it_market = multimap.find(qstrID);
    // -----------------------------
    while ((multimap.end() != it_market) && (it_market.key() == qstrID))
    {
        OTDB::MarketData * pMarketData = VPtr<OTDB::MarketData>::asPtr(it_market.value());
        // -----------------------------
        if (bFirstIteration)
            qstrReturnValue = QString::fromStdString(OTAPI_Wrap::It()->FormatAmount(pMarketData->asset_type_id, 0));

        bFirstIteration = false;
        // -----------------------------
        lTotal += OTAPI_Wrap::It()->StringToLong(pMarketData->total_assets);
        // --------------------
        ++it_market;
        // --------------------
        // We do this here where pMarketData is still a valid pointer.
        //
        if ((multimap.end() == it_market) || (it_market.key() != qstrID))
        {
            qstrReturnValue = QString::fromStdString(OTAPI_Wrap::It()->FormatAmount(pMarketData->asset_type_id, lTotal));
            break;
        }
    }
    // -----------------------------
    return qstrReturnValue;
}
// ----------------------------------
QString MTMarketDetails::CalculateNumberBids(QString & qstrID, QMultiMap<QString, QVariant> & multimap)
{
    return CalculateNumberOffers(qstrID, multimap, true); // bIsBid
}
// ----------------------------------
QString MTMarketDetails::CalculateNumberAsks(QString & qstrID, QMultiMap<QString, QVariant> & multimap)
{
    return CalculateNumberOffers(qstrID, multimap, false); // bIsBid
}
// ----------------------------------
QString MTMarketDetails::CalculateNumberOffers(QString & qstrID, QMultiMap<QString, QVariant> & multimap, bool bIsBid)
{
    QString qstrReturnValue("0");
    // -----------------------------
    int64_t lTotal = 0;
    // -----------------------------
    QMap<QString, QVariant>::iterator it_market = multimap.find(qstrID);
    // -----------------------------
    while ((multimap.end() != it_market) && (it_market.key() == qstrID))
    {
        OTDB::MarketData * pMarketData = VPtr<OTDB::MarketData>::asPtr(it_market.value());

        if (NULL != pMarketData) // Should never be NULL.
        {
            if (bIsBid)
                lTotal += OTAPI_Wrap::It()->StringToLong(pMarketData->number_bids);
            else
                lTotal += OTAPI_Wrap::It()->StringToLong(pMarketData->number_asks);
        }
        // --------------------
        ++it_market;
        // --------------------
        // We do this here where pMarketData is still a valid pointer.
        //
        if ((multimap.end() == it_market) || (it_market.key() != qstrID))
        {
            qstrReturnValue = QString("%1").arg(lTotal);
            break;
        }
    }
    // -----------------------------
    return qstrReturnValue;
}
// -------------------------------------------------------------------------
QString MTMarketDetails::CalculateLastSalePrice(QString & qstrID, QMultiMap<QString, QVariant> & multimap)
{
    QString qstrReturnValue("");
    // -----------------------------
    bool    bFirstIteration = true;
    int64_t lLastSaleDate   = 0;
    int64_t lLastSalePrice  = 0;
    // -----------------------------
    QMap<QString, QVariant>::iterator it_market = multimap.find(qstrID);
    // -----------------------------
    while ((multimap.end() != it_market) && (it_market.key() == qstrID))
    {
        OTDB::MarketData * pMarketData = VPtr<OTDB::MarketData>::asPtr(it_market.value());
        // -----------------------------
        if (bFirstIteration)
            qstrReturnValue = QString::fromStdString(OTAPI_Wrap::It()->FormatAmount(pMarketData->currency_type_id, 0));

        bFirstIteration = false;
        // -----------------------------
        int64_t lCurrentLastSaleDate  = OTAPI_Wrap::It()->StringToLong(pMarketData->last_sale_date);
        int64_t lCurrentLastSalePrice = OTAPI_Wrap::It()->StringToLong(pMarketData->last_sale_price);


        qDebug() << "lCurrentLastSaleDate: " << lCurrentLastSaleDate;
        qDebug() << "lCurrentLastSalePrice: " << lCurrentLastSalePrice;



        if (lCurrentLastSaleDate > lLastSaleDate)
        {
            lLastSaleDate  = lCurrentLastSaleDate;
            lLastSalePrice = lCurrentLastSalePrice;
        }
        // --------------------
        ++it_market;
        // --------------------
        // We do this here where pMarketData is still a valid pointer.
        //
        if ((multimap.end() == it_market) || (it_market.key() != qstrID))
        {
            qstrReturnValue = QString::fromStdString(OTAPI_Wrap::It()->FormatAmount(pMarketData->currency_type_id, lLastSalePrice));
            break;
        }
    }
    // -----------------------------
    return qstrReturnValue;
}
// ----------------------------------
QString MTMarketDetails::CalculateCurrentBid(QString & qstrID, QMultiMap<QString, QVariant> & multimap)
{
    QString qstrReturnValue("");
    // -----------------------------
    bool    bFirstIteration = true;
    int64_t lHighestBid = 0;
    // -----------------------------
    QMap<QString, QVariant>::iterator it_market = multimap.find(qstrID);
    // -----------------------------
    while ((multimap.end() != it_market) && (it_market.key() == qstrID))
    {
        OTDB::MarketData * pMarketData = VPtr<OTDB::MarketData>::asPtr(it_market.value());
        // -----------------------------
        if (bFirstIteration)
            qstrReturnValue = QString::fromStdString(OTAPI_Wrap::It()->FormatAmount(pMarketData->currency_type_id, 0));

        bFirstIteration = false;
        // -----------------------------
        int64_t lCurrentHighestBid = OTAPI_Wrap::It()->StringToLong(pMarketData->current_bid);

        if (lCurrentHighestBid > lHighestBid)
            lHighestBid = lCurrentHighestBid;
        // --------------------
        ++it_market;
        // --------------------
        // We do this here where pMarketData is still a valid pointer.
        //
        if ((multimap.end() == it_market) || (it_market.key() != qstrID))
        {
            qstrReturnValue = QString::fromStdString(OTAPI_Wrap::It()->FormatAmount(pMarketData->currency_type_id, lHighestBid));
            break;
        }
    }
    // -----------------------------
    return qstrReturnValue;
}
// ----------------------------------
QString MTMarketDetails::CalculateCurrentAsk(QString & qstrID, QMultiMap<QString, QVariant> & multimap)
{
    QString qstrReturnValue("");
    // -----------------------------
    bool    bFirstIteration = true;
    int64_t lLowestAsk = 0;
    // -----------------------------
    QMap<QString, QVariant>::iterator it_market = multimap.find(qstrID);
    // -----------------------------
    while ((multimap.end() != it_market) && (it_market.key() == qstrID))
    {
        OTDB::MarketData * pMarketData = VPtr<OTDB::MarketData>::asPtr(it_market.value());
        // -----------------------------
        if (bFirstIteration)
            qstrReturnValue = QString::fromStdString(OTAPI_Wrap::It()->FormatAmount(pMarketData->currency_type_id, 0));

        bFirstIteration = false;
        // -----------------------------
        int64_t lCurrentLowestAsk = OTAPI_Wrap::It()->StringToLong(pMarketData->current_ask);

        if ((0 == lLowestAsk) || ((0 != lCurrentLowestAsk) && (lCurrentLowestAsk < lLowestAsk)))
            lLowestAsk = lCurrentLowestAsk;
        // --------------------
        ++it_market;
        // --------------------
        // We do this here where pMarketData is still a valid pointer.
        //
        if ((multimap.end() == it_market) || (it_market.key() != qstrID))
        {
            qstrReturnValue = QString::fromStdString(OTAPI_Wrap::It()->FormatAmount(pMarketData->currency_type_id, lLowestAsk));
            break;
        }
    }
    // -----------------------------
    return qstrReturnValue;
}
// ----------------------------------



void MTMarketDetails::ClearContents()
{
    ui->lineEditAssetID     ->setText("");
    ui->lineEditCurrencyID  ->setText("");
    // -------------------------------------
    ui->lineEditAsset       ->setText("");
    ui->lineEditCurrency    ->setText("");
    // -------------------------------------
    ui->labelNumberBidsValue->setText("");
    ui->labelNumberAsksValue->setText("");
    ui->labelLastValue      ->setText("");
    // -------------------------------------
    ClearBidsGrid();
    ClearAsksGrid();
    ClearTradesGrid();
    // -------------------------------------
}

void MTMarketDetails::FavorLeftSideForIDs()
{
    if (NULL != ui)
    {
        ui->lineEditAsset     ->home(false);
        ui->lineEditCurrency  ->home(false);
        ui->lineEditAssetID   ->home(false);
        ui->lineEditCurrencyID->home(false);
    }
}

// ------------------------------------------------------

bool MTMarketDetails::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Resize)
    {
        // This insures that the left-most part of the IDs and Names
        // remains visible during all resize events.
        //
        FavorLeftSideForIDs();
    }
//    else
//    {
        // standard event processing
//        return QObject::eventFilter(obj, event);
        return MTEditDetails::eventFilter(obj, event);

        // NOTE: Since the base class has definitely already installed this
        // function as the event filter, I must assume that this version
        // is overriding the version in the base class.
        //
        // Therefore I call the base class version here, since as it's overridden,
        // I don't expect it will otherwise ever get called.
//    }
}

// ------------------------------------------------------

void MTMarketDetails::AddButtonClicked()
{
    // Empty by design.
}

void MTMarketDetails::DeleteButtonClicked()
{
    // Empty by design.
}

MTMarketDetails::~MTMarketDetails()
{
    delete ui;
}
