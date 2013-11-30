#include <QMessageBox>
#include <QDebug>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>
#include <opentxs/OTStorage.h>

#include "offerdetails.h"
#include "ui_offerdetails.h"

#include "detailedit.h"
#include "moneychanger.h"
#include "overridecursor.h"

MTOfferDetails::MTOfferDetails(QWidget *parent, MTDetailEdit & theOwner) :
    MTEditDetails(parent, theOwner),
    ui(new Ui::MTOfferDetails)
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
    ui->lineEditAssetAcct      ->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditCurrencyAcct   ->setStyleSheet("QLineEdit { background-color: lightgray }");
    // ----------------------------------
    ui->lineEditAssetAcctID    ->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditCurrencyAcctID ->setStyleSheet("QLineEdit { background-color: lightgray }");
    // ----------------------------------
    ui->lineEditAcctBalance    ->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditCurrencyBalance->setStyleSheet("QLineEdit { background-color: lightgray }");
    // ----------------------------------
    ui->tableWidgetTrades ->verticalHeader()->hide();
    // ----------------------------------
    ui->tableWidgetTrades->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetTrades->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->tableWidgetTrades->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->tableWidgetTrades->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetTrades->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetTrades->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetTrades->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetTrades->horizontalHeaderItem(4)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetTrades->horizontalHeaderItem(5)->setTextAlignment(Qt::AlignCenter);
    // ----------------------------------
}

void MTOfferDetails::ClearTradesGrid()
{
//    this->blockSignals(true);
    ui->tableWidgetTrades->blockSignals(true);
    // -------------------------------------------------------
    int nGridItemCount = ui->tableWidgetTrades->rowCount();
    // -------------------------------------------------------
    for (int ii = 0; ii < nGridItemCount; ii++)
    {
        QTableWidgetItem * item = ui->tableWidgetTrades->takeItem(0,1); // Row 0, Column 1
        ui->tableWidgetTrades->removeRow(0); // Row 0.

        if (NULL != item)
        {
            delete item;
            item = NULL;
        }
    }
    // -------------------------------------------------------
    ui->tableWidgetTrades->setRowCount(0);
    // -----------------------------------
//    this->blockSignals(false);
    ui->tableWidgetTrades->blockSignals(false);
}


void MTOfferDetails::on_toolButtonAssetAcct_clicked()
{
    if (!ui->lineEditAssetAcctID->text().isEmpty())
        emit ShowAccount(ui->lineEditAssetAcctID->text());
}

void MTOfferDetails::on_toolButtonCurrencyAcct_clicked()
{
    if (!ui->lineEditCurrencyAcctID->text().isEmpty())
        emit ShowAccount(ui->lineEditCurrencyAcctID->text());
}

void MTOfferDetails::refresh(QString strID, QString strName)
{
    // -----------------------------------
    ClearTradesGrid();
    // ----------------------------------------
    if (!strID.isEmpty() && (NULL != ui))
    {
        // FYI, contents of strID:
//      QString qstrCompositeID = QString("%1,%2").arg(qstrServerID).arg(qstrTransactionID);

        QString     qstrServerID, qstrTransactionID;
        QStringList theIDs = strID.split(","); // theIDs.at(0) ServerID, at(1) transaction ID

        if (2 == theIDs.size()) // Should always be 2...
        {
            qstrServerID      = theIDs.at(0);
            qstrTransactionID = theIDs.at(1);
        }
        // -------------------------------------
        if (m_pOwner && m_pOwner->m_pmapOffers)
        {
            QMap<QString, QVariant>::iterator it_offer = m_pOwner->m_pmapOffers->find(strID);

            if (m_pOwner->m_pmapOffers->end() != it_offer)
            {
                // ------------------------------------------------------
                OTDB::OfferDataNym * pOfferData = VPtr<OTDB::OfferDataNym>::asPtr(it_offer.value());

                if (NULL != pOfferData) // Should never be NULL.
                {
                    bool        bSelling          = pOfferData->selling;
                    // ------------------------------------------------------
                    int64_t     lTotalAssets      = OTAPI_Wrap::It()->StringToLong(pOfferData->total_assets);
                    int64_t     lFinished         = OTAPI_Wrap::It()->StringToLong(pOfferData->finished_so_far);
                    // ------------------------------------------------------
                    int64_t     lStillAvailable   = lTotalAssets - lFinished;
                    // ------------------------------------------------------
                    int64_t     lMinimumIncrement = OTAPI_Wrap::It()->StringToLong(pOfferData->minimum_increment);
                    // ------------------------------------------------------
                    int64_t     lScale            = OTAPI_Wrap::It()->StringToLong(pOfferData->scale);
                    std::string str_scale         = OTAPI_Wrap::FormatAmount(pOfferData->asset_type_id, lScale);
                    // ------------------------------------------------------
                    int64_t     lPrice            = OTAPI_Wrap::It()->StringToLong(pOfferData->price_per_scale);
                    std::string str_price         = OTAPI_Wrap::FormatAmount(pOfferData->currency_type_id, lPrice);
                    // ------------------------------------------------------
                    QString qstrPrice(tr("market order"));

                    if (lPrice > 0)
                        qstrPrice = QString("%1: %2").arg(tr("Price")).arg(QString::fromStdString(str_price));
                    // ------------------------------------------------------
                    QString qstrFormattedScale    = QString::fromStdString(str_scale);

                    if (lScale > 1)
                        qstrPrice += QString(" (%1 %2)").arg(tr("per")).arg(qstrFormattedScale);
                    // ------------------------------------------------------
                    QString qstrMinimumIncrement  = QString::fromStdString(OTAPI_Wrap::FormatAmount(pOfferData->asset_type_id, lMinimumIncrement));
                    QString qstrTotalAssets       = QString::fromStdString(OTAPI_Wrap::FormatAmount(pOfferData->asset_type_id, lTotalAssets));
                    QString qstrSoldOrPurchased   = QString::fromStdString(OTAPI_Wrap::FormatAmount(pOfferData->asset_type_id, lFinished));
                    QString qstrStillAvailable    = QString::fromStdString(OTAPI_Wrap::FormatAmount(pOfferData->asset_type_id, lStillAvailable));
                    // ------------------------------------------------------
                    std::string str_asset_name    = OTAPI_Wrap::GetAssetType_Name(pOfferData->asset_type_id);
                    // -----------------------------------------------------------------------
                    time_t tValidFrom      = static_cast<time_t>(OTAPI_Wrap::StringToLong(pOfferData->valid_from));
                    time_t tValidTo        = static_cast<time_t>(OTAPI_Wrap::StringToLong(pOfferData->valid_to));
                    // -----------------------------------------------------------------------
                    QDateTime qdate_from   = QDateTime::fromTime_t(tValidFrom);
                    QDateTime qdate_to     = QDateTime::fromTime_t(tValidTo);
                    // -----------------------------------------------------------------------
                    QString   qstrDateFrom = qdate_from.toString(QString("MMM d yyyy hh:mm:ss"));
                    QString   qstrDateTo   = qdate_to  .toString(QString("MMM d yyyy hh:mm:ss"));
                    // -----------------------------------------------------------------------
                    ui->labelValidFrom ->setText(qstrDateFrom);
                    ui->labelValidTo   ->setText(qstrDateTo);
                    // ------------------------------------------------------
                    ui->labelAvailable->setText(qstrStillAvailable);
                    ui->labelMinInc   ->setText(qstrMinimumIncrement);
                    // ------------------------------------------------------
                    QString qstrBuySell = bSelling ? tr("Sell") : tr("Buy");
                    QString qstrAmounts;

                    qstrAmounts = QString("%1").
                            arg(qstrTotalAssets);
                    // ------------------------------------------------------
                    QString qstrCompleted("");

                    if (lFinished > 0)
                        qstrCompleted = QString("%1: %2").
                                arg(tr("completed")).
                                arg(qstrSoldOrPurchased);
                    // --------------------------
        //          "Buy Silver Grams: 300g (40g finished so far)";
                    //
                    QString qstrOfferName = QString("%1 %2: %3").
                            arg(qstrBuySell).
                            arg(QString::fromStdString(str_asset_name)).
                            arg(qstrAmounts);
                    // -------------------------------------
                    QString qstrTrans = QString("%1# %2").arg(tr("Trans")).arg(qstrTransactionID);
//                    QString qstrTrans = QString("<font size=1 color=grey>%1#</font> %2").arg(tr("Trans")).arg(qstrTransactionID);
                    // -------------------------------------
                    QWidget * pHeaderWidget = MTEditDetails::CreateDetailHeaderWidget(m_Type, qstrPrice, qstrOfferName,
                                                                                      qstrCompleted, qstrTrans, ":/icons/icons/assets.png", false);

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
                    ui->lineEditAssetAcct      ->setText(QString::fromStdString(OTAPI_Wrap::GetAccountWallet_Name(pOfferData->asset_acct_id)));
                    ui->lineEditCurrencyAcct   ->setText(QString::fromStdString(OTAPI_Wrap::GetAccountWallet_Name(pOfferData->currency_acct_id)));
                    // ------------------------------------------------------
                    ui->lineEditAssetAcctID    ->setText(QString::fromStdString(pOfferData->asset_acct_id));
                    ui->lineEditCurrencyAcctID ->setText(QString::fromStdString(pOfferData->currency_acct_id));
                    // ------------------------------------------------------
                    const int64_t lAcctBalance     = OTAPI_Wrap::GetAccountWallet_Balance(pOfferData->asset_acct_id);
                    const int64_t lCurrencyBalance = OTAPI_Wrap::GetAccountWallet_Balance(pOfferData->currency_acct_id);

                    const std::string str_acct_balance     = OTAPI_Wrap::FormatAmount(pOfferData->asset_type_id, lAcctBalance);
                    const std::string str_currency_balance = OTAPI_Wrap::FormatAmount(pOfferData->asset_type_id, lCurrencyBalance);

                    ui->lineEditAcctBalance    ->setText(QString::fromStdString(str_acct_balance));
                    ui->lineEditCurrencyBalance->setText(QString::fromStdString(str_currency_balance));
                     // ------------------------------------------------------
                    PopulateNymTradesGrid (strID, m_pOwner->GetMarketNymID(), *(m_pOwner->m_pmapOffers));
                    // ------------------------------------------------------
                }
            }
        }
        // ----------------------------------
        FavorLeftSideForIDs();
        // ----------------------------------
    }
    else
        ClearContents();
}


// ----------------------------------------------------


/*
message TradeDataNym_InternalPB {
    optional string gui_label = 1;
    optional string completed_count = 2; // (How many trades have processed for the associated offer? We keep count for each trade.)
    optional string date = 3;			 // (The date of this trade's execution)
    optional string price = 4;			 // (The price this trade executed at.)
    optional string amount_sold = 5;	 // (Amount of asset sold for that price.)
    optional string transaction_id = 6;	 // (transaction number for original offer.)
    optional string updated_id = 7;      // NEW FIELD (transaction number for this trade receipt.)
    optional string offer_price = 8;     // NEW FIELD (price limit on the original offer.)
    optional string finished_so_far = 9; // NEW FIELD (total amount sold this offer across all trades.)
    optional string asset_id = 10;       // NEW FIELD asset id of trade
    optional string currency_id = 11;    // NEW FIELD currency id of trade
    optional string currency_paid = 12;  // NEW FIELD currency paid for this trade.
}

// ----------------------------------------------------

message TradeListNym_InternalPB {
    repeated TradeDataNym_InternalPB  trades = 1;
}

/*/



// Caller must delete.
OTDB::TradeListNym * MTOfferDetails::LoadTradeListForNym(OTDB::OfferDataNym & offerData, QString qstrServerID, QString qstrNymID)
{
    OTDB::TradeListNym * pTradeList = NULL;
    OTDB::Storable     * pStorable  = NULL;
    // ------------------------------------------
    QString qstrMarketID = m_pOwner->GetMarketID();
    // ------------------------------------------
    if (!m_pOwner || qstrServerID.isEmpty() || qstrNymID.isEmpty() || qstrMarketID.isEmpty())
        return NULL;
    // ------------------------------------------
    if (OTDB::Exists("nyms", "trades", offerData.server_id, qstrNymID.toStdString()))
    {
        pStorable = OTDB::QueryObject(OTDB::STORED_OBJ_TRADE_LIST_NYM, "nyms", "trades",
                                      offerData.server_id, qstrNymID.toStdString());
        if (NULL == pStorable)
            return NULL;
        // -------------------------------
        pTradeList = OTDB::TradeListNym::ot_dynamic_cast(pStorable);

        if (NULL == pTradeList)
            delete pStorable;
    }

    return pTradeList;
}


/*

message OfferDataNym_InternalPB {
    optional string gui_label = 1;

    optional string valid_from = 2;
    optional string valid_to = 3;

    optional string server_id = 4;
    optional string asset_type_id = 5;		// the asset type on offer.
    optional string asset_acct_id = 6;		// the account where asset is.
    optional string currency_type_id = 7;	// the currency being used to purchase the asset.
    optional string currency_acct_id = 8;	// the account where currency is.

    optional bool selling = 9;		// true for ask, false for bid.

    optional string scale = 10;	// 1oz market? 100oz market? 10,000oz market? This determines size and granularity.
    optional string price_per_scale = 11;

    optional string transaction_id = 12;

    optional string total_assets = 13;
    optional string finished_so_far = 14;


    // Each sale or purchase against (total_assets - finished_so_far) must be in minimum increments.
    // Minimum Increment must be evenly divisible by scale.
    // (This effectively becomes a "FILL OR KILL" order if set to the same value as total_assets. Also, MUST be 1
    // or greater. CANNOT be zero. Enforce this at class level. You cannot sell something in minimum increments of 0.)

    optional string minimum_increment = 15;

    optional string stop_sign = 16;		// If this is a stop order, this will contain '<' or '>'.
    optional string stop_price = 17;	// The price at which the stop order activates (less than X or greater than X, based on sign.)

    optional string date = 18; // (NEW FIELD) The date this offer was added to the market.
}

message TradeDataNym_InternalPB {
    optional string gui_label = 1;
    optional string completed_count = 2; // (How many trades have processed for the associated offer? We keep count for each trade.)
    optional string date = 3;			 // (The date of this trade's execution)
    optional string price = 4;			 // (The price this trade executed at.)
    optional string amount_sold = 5;	 // (Amount of asset sold for that price.)
    optional string transaction_id = 6;	 // (transaction number for original offer.)
    optional string updated_id = 7;      // NEW FIELD (transaction number for this trade receipt.)
    optional string offer_price = 8;     // NEW FIELD (price limit on the original offer.)
    optional string finished_so_far = 9; // NEW FIELD (total amount sold this offer across all trades.)
    optional string asset_id = 10;       // NEW FIELD asset id of trade
    optional string currency_id = 11;    // NEW FIELD currency id of trade
    optional string currency_paid = 12;  // NEW FIELD currency paid for this trade.
}

*/

void MTOfferDetails::PopulateNymTradesGrid(QString & qstrID, QString qstrNymID, QMap<QString, QVariant> & OFFER_MAP)
{
    // ------------------------------------------------------------------------
//    this->blockSignals(true);
    // -----------------------------------
    ui->tableWidgetTrades->blockSignals(true);
    // -----------------------------------
    int nTradesRowCount  = 0;
    // -----------------------------------
    int nTradesGridIndex = 0;
    // -----------------------------------
    QMap<QString, QVariant>::iterator it_offer = OFFER_MAP.find(qstrID);
    // -----------------------------
    if (OFFER_MAP.end() != it_offer)
    {
        OTDB::OfferDataNym * pOfferData = VPtr<OTDB::OfferDataNym>::asPtr(it_offer.value());

        if (NULL != pOfferData) // Should never be NULL.
        {
            std::string & str_server         = pOfferData->server_id;
            std::string   str_server_display = OTAPI_Wrap::GetServer_Name(str_server);
            QString       qstrServerName     = QString::fromStdString(str_server_display);
            // -----------------------------------------
            int64_t lScale = OTAPI_Wrap::StringToLong(pOfferData->scale);
            // -----------------------------------------
            QTableWidgetItem * pPriceHeader = ui->tableWidgetTrades->horizontalHeaderItem(0);

            if (NULL != pPriceHeader)
            {
                if (1 == lScale)
                    pPriceHeader->setText(tr("Actual Price"));
                else
                    pPriceHeader->setText(QString("%1 %2").arg(tr("Actual Price per")).arg(lScale));
            }
            // -----------------------------------------
            QTableWidgetItem * pAmountHeader = ui->tableWidgetTrades->horizontalHeaderItem(1);

            if (NULL != pAmountHeader)
            {
                if (pOfferData->selling)
                    pAmountHeader->setText(tr("Sold"));
                else
                    pAmountHeader->setText(tr("Purchased"));
            }
            // -----------------------------------------
            QTableWidgetItem * pCurrencyHeader = ui->tableWidgetTrades->horizontalHeaderItem(2);

            if (NULL != pCurrencyHeader)
            {
                if (pOfferData->selling)
                    pCurrencyHeader->setText(tr("Received"));
                else
                    pCurrencyHeader->setText(tr("Paid"));
            }
            // -----------------------------------------
            OTDB::TradeListNym * pTradeList = LoadTradeListForNym(*pOfferData,
                                                                  QString::fromStdString(str_server),
                                                                  qstrNymID);
            OTCleanup<OTDB::TradeListNym> theAngel(pTradeList);

            if (NULL != pTradeList)
            {
                size_t nTradeDataCount = pTradeList->GetTradeDataNymCount();
                // -------------------------------------
                nTradesRowCount += static_cast<int>(nTradeDataCount);
                // -------------------------------------
                ui->tableWidgetTrades->setRowCount(nTradesRowCount);
                // -------------------------------------
                for (size_t trade_index = 0; trade_index < nTradeDataCount; ++trade_index)
                {
                    OTDB::TradeDataNym * pTradeData = pTradeList->GetTradeDataNym(trade_index);

                    if (NULL == pTradeData) // Should never happen.
                        continue;
                    // -----------------------------------------------------------------------
                    int64_t lOfferID = OTAPI_Wrap::StringToLong(pOfferData->transaction_id);
                    int64_t lTradeID = OTAPI_Wrap::StringToLong(pTradeData->transaction_id);

                    if (lOfferID != lTradeID)
                    {
                        qDebug() << QString("Showing trades for Offer %1; skipping trade receipt with trans number %2.")
                                    .arg(lOfferID).arg(lTradeID);
                        continue;
                    }
                    // -----------------------------------------------------------------------
                    QString qstrUpdatedID     = QString::fromStdString(pTradeData->updated_id);
                    // -----------------------------------------------------------------------
                    time_t tDate = static_cast<time_t>(OTAPI_Wrap::StringToLong(pTradeData->date));

                    QDateTime qdate_added   = QDateTime::fromTime_t(tDate);
                    QString   qstrDateAdded = qdate_added.toString(QString("MMM d yyyy hh:mm:ss"));
                    // -----------------------------------------------------------------------
                    std::string & str_price         = pTradeData->price;
                    int64_t       lPrice            = OTAPI_Wrap::StringToLong(str_price); // this price is "per scale"

                    if (lPrice < 0)
                        lPrice *= (-1);

                    std::string   str_price_display = OTAPI_Wrap::FormatAmount(pOfferData->currency_type_id, lPrice);

                    QString qstrPrice = QString::fromStdString(str_price_display);
                    // -----------------------------------------------------------------------
                    std::string & str_amount_sold    = pTradeData->amount_sold;
                    int64_t       lQuantity          = OTAPI_Wrap::StringToLong(str_amount_sold); // Total amount of asset sold.

                    if (lQuantity < 0)
                        lQuantity *= (-1);

                    std::string   str_amount_display = OTAPI_Wrap::FormatAmount(pOfferData->asset_type_id, lQuantity);

                    QString qstrAmountSold = QString::fromStdString(str_amount_display);
                    // -----------------------------------------------------------------------
                    std::string & str_currency_paid   = pTradeData->currency_paid;
                    int64_t       lPayQuantity        = OTAPI_Wrap::StringToLong(str_currency_paid); // Total currency paid

                    if (lPayQuantity < 0)
                        lPayQuantity *= (-1);

                    std::string   str_paid_display    = OTAPI_Wrap::FormatAmount(pOfferData->currency_type_id, lPayQuantity);

                    QString qstrCurrencyPaid = QString::fromStdString(str_paid_display);
                    // -----------------------------------------------------------------------
                    QLabel * pLabelPrice        = new QLabel(qstrPrice);
                    QLabel * pLabelAmountSold   = new QLabel(qstrAmountSold);
                    QLabel * pLabelCurrencyPaid = new QLabel(qstrCurrencyPaid);
                    QLabel * pLabelDateAdded    = new QLabel(QString("<small>%1</small>").arg(qstrDateAdded));
                    QLabel * pLabelTransID      = new QLabel(qstrUpdatedID);
                    QLabel * pLabelServer       = new QLabel(qstrServerName);
                    // -----------------------------------------------------------------------
                    pLabelPrice     ->setAlignment(Qt::AlignCenter);
                    pLabelAmountSold->setAlignment(Qt::AlignCenter);
                    pLabelCurrencyPaid->setAlignment(Qt::AlignCenter);
                    pLabelDateAdded ->setAlignment(Qt::AlignCenter);
                    pLabelTransID   ->setAlignment(Qt::AlignCenter);
                    pLabelServer    ->setAlignment(Qt::AlignLeft);
                    // -----------------------------------------------------------------------
                    ui->tableWidgetTrades->setCellWidget ( nTradesGridIndex, 0, pLabelPrice     );
                    ui->tableWidgetTrades->setCellWidget ( nTradesGridIndex, 1, pLabelAmountSold);
                    ui->tableWidgetTrades->setCellWidget ( nTradesGridIndex, 2, pLabelCurrencyPaid);
                    ui->tableWidgetTrades->setCellWidget ( nTradesGridIndex, 3, pLabelDateAdded );
                    ui->tableWidgetTrades->setCellWidget ( nTradesGridIndex, 4, pLabelTransID   );
                    ui->tableWidgetTrades->setCellWidget ( nTradesGridIndex, 5, pLabelServer    );
                    // -----------------------------------------------------------------------
                    ++nTradesGridIndex;
                    // -----------------------------------------------------------------------
                } // for (trades)
                // -----------------------------------------------------------------------
            } // if (NULL != pTradeList)
        } // if (NULL != pTradeData)
        // -----------------
        ++it_offer;
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


void MTOfferDetails::AddButtonClicked()
{

}

void MTOfferDetails::DeleteButtonClicked()
{

}

void MTOfferDetails::FavorLeftSideForIDs()
{
    if (NULL != ui)
    {
        // -------------------------------------
        ui->lineEditAssetAcct      ->home(false);
        ui->lineEditCurrencyAcct   ->home(false);
        // -------------------------------------
        ui->lineEditAssetAcctID    ->home(false);
        ui->lineEditCurrencyAcctID ->home(false);
        // -------------------------------------
        ui->lineEditAcctBalance    ->home(false);
        ui->lineEditCurrencyBalance->home(false);
    }
}

void MTOfferDetails::ClearContents()
{
    // -------------------------------------
    ui->lineEditAssetAcct     ->setText("");
    ui->lineEditCurrencyAcct  ->setText("");
    // -------------------------------------
    ui->lineEditAssetAcctID   ->setText("");
    ui->lineEditCurrencyAcctID->setText("");
    // -------------------------------------
    ui->lineEditAcctBalance    ->setText("");
    ui->lineEditCurrencyBalance->setText("");
    // -------------------------------------
    ui->labelAvailable->setText("");
    ui->labelMinInc   ->setText("");
    ui->labelValidFrom->setText("");
    ui->labelValidTo  ->setText("");
    // -------------------------------------
    ClearTradesGrid();
}

// ------------------------------------------------------

bool MTOfferDetails::eventFilter(QObject *obj, QEvent *event)
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

MTOfferDetails::~MTOfferDetails()
{
    delete ui;
}
