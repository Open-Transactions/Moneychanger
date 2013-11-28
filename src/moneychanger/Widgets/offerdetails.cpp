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
    ui->tableWidget ->verticalHeader()->hide();
    // ----------------------------------
    ui->lineEditAssetAcct     ->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditCurrencyAcct  ->setStyleSheet("QLineEdit { background-color: lightgray }");
    // ----------------------------------
    ui->lineEditAssetAcctID   ->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditCurrencyAcctID->setStyleSheet("QLineEdit { background-color: lightgray }");
}

// TODO: NYM TRADES
//
//if (otapi.Exists("nyms", "trades", serverID, nymID)) {
//    storable = otapi.QueryObject(StoredObjectType.STORED_OBJ_TRADE_LIST_NYM, "nyms", "trades", serverID, nymID);


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
    ui->tableWidget->blockSignals(true);
    // -----------------------------------
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount (0);
    // ----------------------------------------
    ui->tableWidget->blockSignals(false);
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
                    ui->lineEditAssetAcct     ->setText(QString::fromStdString(OTAPI_Wrap::GetAccountWallet_Name(pOfferData->asset_acct_id)));
                    ui->lineEditCurrencyAcct  ->setText(QString::fromStdString(OTAPI_Wrap::GetAccountWallet_Name(pOfferData->currency_acct_id)));
                    // ------------------------------------------------------
                    ui->lineEditAssetAcctID   ->setText(QString::fromStdString(pOfferData->asset_acct_id));
                    ui->lineEditCurrencyAcctID->setText(QString::fromStdString(pOfferData->currency_acct_id));
                     // ------------------------------------------------------
//                    QString qstrNumberBids    = CalculateNumberBids    (strID, *(m_pOwner->m_pmapOffers));
//                    QString qstrNumberAsks    = CalculateNumberAsks    (strID, *(m_pOwner->m_pmapOffers));
//                    QString qstrLastSalePrice = CalculateLastSalePrice (strID, *(m_pOwner->m_pmapOffers));
//                    // ------------------------------------------------------
//                    ui->labelNumberBidsValue->setText(qstrNumberBids);
//                    ui->labelNumberAsksValue->setText(qstrNumberAsks);
//                    ui->labelLastValue      ->setText(qstrLastSalePrice);
//                    // ------------------------------------------------------
//                    RetrieveMarketOffers     (strID, *(m_pOwner->m_pmapOffers));
//                    RetrieveMarketTrades     (strID, *(m_pOwner->m_pmapOffers));
//                    // ------------------------------------------------------
//                    PopulateMarketOffersGrids(strID, *(m_pOwner->m_pmapOffers));
//                    PopulateRecentTradesGrid (strID, *(m_pOwner->m_pmapOffers));
                    // ------------------------------------------------------
                }
            }
        }
        // ----------------------------------
        FavorLeftSideForIDs();
        // ----------------------------------
    }
}

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
        ui->lineEditAssetAcct     ->home(false);
        ui->lineEditCurrencyAcct  ->home(false);
        // -------------------------------------
        ui->lineEditAssetAcctID   ->home(false);
        ui->lineEditCurrencyAcctID->home(false);
        // -------------------------------------
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
    ui->labelAvailable->setText("");
    ui->labelMinInc   ->setText("");
    ui->labelValidFrom->setText("");
    ui->labelValidTo  ->setText("");
    // -------------------------------------
//    ClearGrid();
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
