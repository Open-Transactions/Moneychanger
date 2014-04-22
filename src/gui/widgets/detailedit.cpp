#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/detailedit.hpp>
#include <ui_detailedit.h>

#include <gui/widgets/nymdetails.hpp>
#include <gui/widgets/contactdetails.hpp>
#include <gui/widgets/serverdetails.hpp>
#include <gui/widgets/assetdetails.hpp>
#include <gui/widgets/agreementdetails.hpp>
#include <gui/widgets/corporationdetails.hpp>
#include <gui/widgets/offerdetails.hpp>
#include <gui/widgets/marketdetails.hpp>
#include <gui/widgets/accountdetails.hpp>
#include <gui/widgets/home.hpp>

#include <core/moneychanger.hpp>

#include <opentxs/OTAPI.hpp>
#include <opentxs/OTAPI_Exec.hpp>
#include <opentxs/OTStorage.hpp>

#include <QDialog>
#include <QKeyEvent>
#include <QDebug>


MTDetailEdit::MTDetailEdit(QWidget *parent) :
    QWidget(parent, Qt::Window),
    m_bFirstRun(true),
    m_pmapMarkets(NULL),
    m_pmapOffers(NULL),
    m_nCurrentRow(-1),
    m_bEnableAdd(true),
    m_bEnableDelete(true),
    m_Type(MTDetailEdit::DetailEditTypeError),
    ui(new Ui::MTDetailEdit)
{
    ui->setupUi(this);
}

MTDetailEdit::~MTDetailEdit()
{
    delete ui;
}


void MTDetailEdit::SetMarketMap(QMultiMap<QString, QVariant> & theMap)
{
    m_pmapMarkets = &theMap;
}

void MTDetailEdit::SetOfferMap(QMap<QString, QVariant> & theMap)
{
    m_pmapOffers = &theMap;
}

void MTDetailEdit::onBalancesChangedFromAbove()
{
    this->RefreshRecords();
}


void MTDetailEdit::onBalancesChangedFromBelow(QString qstrAcctID)
{
    m_PreSelected   = qstrAcctID;
    m_qstrCurrentID = qstrAcctID;

    emit balancesChanged();
}



// Use for widget that appears on a parent dialog.
// (Use dialog() instead, to display this widget as a modeless or modal dialog.)
//
void MTDetailEdit::show_widget(MTDetailEdit::DetailEditType theType)
{
    FirstRun(theType); // This only does something the first time it's run. (Otherwise this does nothing.)
    // -------------------------------------------
    RefreshRecords();
    // -------------------------------------------
    // anything else? install event filter maybe?
}


// Use for modeless or modal dialogs.
// (Use show_widget instead, if displaying this widget NOT as a dialog.)
//
void MTDetailEdit::dialog(MTDetailEdit::DetailEditType theType, bool bIsModal/*=false*/)
{
    FirstRun(theType); // This only does something the first time it's run. (Otherwise this does nothing.)
    // -------------------------------------------
    RefreshRecords();
    // -------------------------------------------
    if (bIsModal)
    {
        QDialog theDlg;
        theDlg.setWindowTitle(this->windowTitle());
//      theDlg.installEventFilter(this);

        QVBoxLayout * pLayout = new QVBoxLayout;

        pLayout->addWidget(this);

        theDlg.setLayout(pLayout);
        theDlg.setWindowFlags(Qt::Tool); // A hack so it will show the close button.
        theDlg.exec();

        pLayout->removeWidget(this);
    }
    else
    {
        this->installEventFilter(this);

        show();
        setFocus();
    }
    // -------------------------------------------
}

// -------------------------------------------

// This only does something the first time you run it.
//
void MTDetailEdit::FirstRun(MTDetailEdit::DetailEditType theType)
{
    if (m_bFirstRun)
    {
        // -------------------------------------------
        ui->comboBox->setHidden(true);
        // -------------------------------------------
        ui->tableWidget->setColumnCount(2);
        ui->tableWidget->setSelectionMode    (QAbstractItemView::SingleSelection);
        ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        // -------------------------------------------
        ui->tableWidget->horizontalHeader()->resizeSection(0, 5);
        // -------------------------------------------
        ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
        ui->tableWidget->verticalHeader()->setDefaultSectionSize(60);
        ui->tableWidget->verticalHeader()->hide();
        ui->tableWidget->horizontalHeader()->hide();
        // -------------------------------------------
        ui->tableWidget->setContentsMargins(10,0,0,0);
        // -------------------------------------------
        ui->tableWidget->setSizePolicy(
                    QSizePolicy::Expanding,
                    QSizePolicy::Expanding);
        // ----------------------------------
        m_pTabWidget  = new QTabWidget;
        // ----------------------------------
        m_pTabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_pTabWidget->setContentsMargins(5, 5, 5, 5);
        // ----------------------------------
        QWidget * pTab1 = new QWidget;
        // ----------------------------------
        pTab1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        pTab1->setContentsMargins(5, 5, 5, 5);
        // ----------------------------------
        QString qstrTab1Title = tr("Details");

        m_pTabWidget->addTab(pTab1, qstrTab1Title);
        // -------------------------------------------
        // Instantiate m_pDetailPane to one of various types.
        //
        m_Type = theType;

        switch (m_Type)
        {
        case MTDetailEdit::DetailEditTypeNym:         m_pDetailPane = new MTNymDetails        (this, *this); break;
        case MTDetailEdit::DetailEditTypeContact:     m_pDetailPane = new MTContactDetails    (this, *this); break;
        case MTDetailEdit::DetailEditTypeServer:      m_pDetailPane = new MTServerDetails     (this, *this); break;
        case MTDetailEdit::DetailEditTypeAsset:       m_pDetailPane = new MTAssetDetails      (this, *this); break;
        case MTDetailEdit::DetailEditTypeAgreement:   m_pDetailPane = new MTAgreementDetails  (this, *this); break;
        case MTDetailEdit::DetailEditTypeCorporation: m_pDetailPane = new MTCorporationDetails(this, *this); break;

        case MTDetailEdit::DetailEditTypeOffer:
            ui->comboBox->setHidden(false);
            m_pDetailPane = new MTOfferDetails(this, *this);
            // -------------------------------------------
            connect(ui->comboBox, SIGNAL(currentIndexChanged(int)),
                    this,         SLOT  (on_comboBox_currentIndexChanged(int)));
            // -------------------------------------------
            break;
        case MTDetailEdit::DetailEditTypeMarket:
            EnableAdd   (false);
            EnableDelete(false);
            m_pDetailPane = new MTMarketDetails(this, *this);
            break;

        case MTDetailEdit::DetailEditTypeAccount:
            m_pDetailPane = new MTAccountDetails(this, *this);
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(DefaultAccountChanged(QString, QString)),
                    Moneychanger::It(), SLOT  (setDefaultAccount(QString, QString)));
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(SendFromAcct(QString)),
                    Moneychanger::It(), SLOT  (mc_send_from_acct(QString)));
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(RequestToAcct(QString)),
                    Moneychanger::It(), SLOT  (mc_request_to_acct(QString)));
            // -------------------------------------------
            break;

        default:
            qDebug() << "MTDetailEdit::dialog: MTDetailEdit::DetailEditTypeError";
            return;
        } //switch
        // -------------------------------------------
        connect(m_pDetailPane,      SIGNAL(NeedToUpdateMenu()),
                Moneychanger::It(), SLOT  (onNeedToUpdateMenu()));
        // -------------------------------------------
        connect(m_pDetailPane,      SIGNAL(RefreshRecordsAndUpdateMenu()),
                Moneychanger::It(), SLOT  (onNeedToUpdateMenu()));
        // -------------------------------------------
        connect(m_pDetailPane,      SIGNAL(RefreshRecordsAndUpdateMenu()),
                this,               SLOT  (onRefreshRecords()));
        // -------------------------------------------
        connect(m_pDetailPane,      SIGNAL(ShowAsset(QString)),
                Moneychanger::It(), SLOT  (mc_show_asset_slot(QString)));
        // -------------------------------------------
        connect(m_pDetailPane,      SIGNAL(ShowNym(QString)),
                Moneychanger::It(), SLOT  (mc_show_nym_slot(QString)));
        // -------------------------------------------
        connect(m_pDetailPane,      SIGNAL(ShowServer(QString)),
                Moneychanger::It(), SLOT  (mc_show_server_slot(QString)));
        // -------------------------------------------
        connect(m_pDetailPane,      SIGNAL(ShowAccount(QString)),
                Moneychanger::It(), SLOT  (mc_show_account_slot(QString)));
        // -------------------------------------------
        m_pDetailPane->SetOwnerPointer(*this);
        m_pDetailPane->SetEditType(theType);
        // -------------------------------------------
        m_pDetailLayout = new QVBoxLayout;
        m_pDetailLayout->addWidget(m_pDetailPane);

        m_pDetailPane  ->setContentsMargins(1,1,1,1);
        m_pDetailLayout->setContentsMargins(1,1,1,1);
        // ----------------------------------

        pTab1->setLayout(m_pDetailLayout);

        // ----------------------------------
        int nCustomTabCount = m_pDetailPane->GetCustomTabCount();

        if (nCustomTabCount > 0)
        {
            for (int ii = 0; ii < nCustomTabCount; ii++)
            {
                QWidget * pTab = m_pDetailPane->CreateCustomTab(ii);
                // ----------------------------------
                if (NULL != pTab)
                {
                    pTab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                    pTab->setContentsMargins(5, 5, 5, 5);

                    QString qstrTabName = m_pDetailPane->GetCustomTabName(ii);

                    m_pTabWidget->addTab(pTab, qstrTabName);
                }
                // ----------------------------------
            }
        }
        // -----------------------------------------------
        QGridLayout * pGridLayout = new QGridLayout;
        pGridLayout->addWidget(m_pTabWidget);

        pGridLayout->setContentsMargins(0,0,0,0);
        m_pTabWidget->setTabPosition(QTabWidget::South);
        // ----------------------------------
        ui->widget->setContentsMargins(1,1,1,1);
        // ----------------------------------
        ui->widget->setLayout(pGridLayout);
        // ----------------------------------
        if (!m_bEnableAdd)
            ui->addButton->setVisible(false);
        // ----------------------------------
        if (!m_bEnableDelete)
            ui->deleteButton->setVisible(false);
        // ----------------------------------
        m_bFirstRun = false;
    } // first run.
}


void MTDetailEdit::onRefreshRecords()
{
    RefreshRecords();
}


void MTDetailEdit::showEvent(QShowEvent * event)
{
    QWidget::showEvent(event);

//    if (m_map.size() < 1)
//        on_addButton_clicked();
}

//virtual
//void MTDetailEdit::showEvent(QShowEvent * event)
//{
//    dialog();

//    // ----------------------------------
//    // call inherited method
//    //
//    QDialog::showEvent(event);
//}



// -------------------------------------------

void MTDetailEdit::onMarketIDChangedFromAbove(QString qstrMarketID)
{
    if (MTDetailEdit::DetailEditTypeMarket == m_Type)
    {
        // The market ID has been changed on the Offers page.
        // (And *I* am the Markets page, responding to this event.)
        //
        // Note that we could use a simple find() here, but I wanted
        // to grab the index. Maybe there's a call on QStringMap that
        // does that for me... (checked -- nope.)
        //
        int nIndex = -1;
        mapIDName::iterator it_markets = m_map.begin();

        for (; it_markets !=  m_map.end(); ++it_markets)
        {
            ++nIndex; // 0 on first iteration.
            // --------------------------------
            if (it_markets.key() == qstrMarketID)
                break;
        }
        // ------------------------------------------------------------
//      mapIDName::iterator it_markets = m_map.find(qstrMarketID);

        if ((m_map.end() != it_markets) &&
            (nIndex < ui->tableWidget->rowCount())) // Should always be true since m_map is used to populate ui->tableWidget.
        {
            m_qstrCurrentID   = it_markets.key();
            m_qstrCurrentName = it_markets.value();
            // ----------------------------------------
            ui->tableWidget->blockSignals(true);
            // ----------------------------------------
            ui->tableWidget->setCurrentCell(nIndex, 1);
            // ----------------------------------------
            ui->tableWidget->blockSignals(false);
            // ----------------------------------------
            m_PreSelected = m_qstrCurrentID;

            // (We keep the signals blocked because we don't want the onCellChanged method
            // to trigger and bounce the football back to the offers panel for an infinite
            // loop of setting the market ID back and forth.)
            //
            // But of course, we still need to refresh the market details:
            // (e.g. the ui_labels on the market panel.)
            //
            if (m_pDetailPane)
                m_pDetailPane->refresh(m_qstrCurrentID, m_qstrCurrentName);
        }
    }
    else if (MTDetailEdit::DetailEditTypeOffer == m_Type)
    {
        // The market ID has been changed on the Markets page.
        // (And *I* am the Offers page, responding to this event.)
        //
        m_qstrMarketID = qstrMarketID;

        RefreshMarketCombo();

        // NOTE: If this DetailEdit is the Offers panel, and if we were notified from
        // above, then we must have notified by the Markets panel (which is why we are
        // setting the current m_qstrMarketID here, because we needed to know about it.)
        //
        // But if we are being notified by the Markets panel, then why we are bothering
        // to emit NeedToLoadOrRetrieveOffers(marketID) here? True, our market ID just
        // changed, so we DO need to load new offers to match that market. But we shouldn't
        // have to emit that here, since it should already be connected to the same signal
        // that triggered this function (that we're currently in) in the first place.
        // (And indeed it is, for both panels.)
        //
//        emit NeedToLoadOrRetrieveOffers(m_qstrMarketID);
    }
}

// -------------------------------------------

void MTDetailEdit::RefreshMarketCombo()
{
    if (MTDetailEdit::DetailEditTypeOffer != m_Type)
        return;
    // ----------------------------
    ui->comboBox->blockSignals(true);
    // ----------------------------
    ui->comboBox->clear();
    // ----------------------------
    int nCurrentMarketIndex  = 0;
    // ----------------------------
    bool bFoundCurrentMarket = false;
    // ----------------------------
    int nIndex = -1;
    for (mapIDName::iterator ii = m_mapMarkets.begin(); ii != m_mapMarkets.end(); ++ii)
    {
        ++nIndex; // 0 on first iteration.
        // ------------------------------
        QString OT_market_id   = ii.key();   // This is the marketID,scale
        QString OT_market_name = ii.value(); // This is the display name aka "Bitcoins for Silver Grams"
        // ------------------------------
        if (!m_qstrMarketID.isEmpty() && (OT_market_id == m_qstrMarketID))
        {
            bFoundCurrentMarket = true;
            nCurrentMarketIndex = nIndex;
        }
        // ------------------------------
        ui->comboBox->insertItem(nIndex, OT_market_name);
    }
    // -----------------------------------------------
    if (m_mapMarkets.size() > 0)
    {
        SetCurrentMarketIDBasedOnIndex(nCurrentMarketIndex);
        ui->comboBox->setCurrentIndex(nCurrentMarketIndex);
    }
    else
        SetCurrentMarketIDBasedOnIndex(-1);
    // -----------------------------------------------
    ui->comboBox->blockSignals(false);
    // -----------------------------------------------
//    emit NeedToLoadOrRetrieveOffers(m_qstrMarketID);

    // We are going to show_widget for the offers panel BEFORE the markets panel.
    // But it's only when the markets panel does its show_widget that the market
    // gets initially set. And it's only then that a signal is sent to the offers
    // panel (onMarketIDChangedFromAbove) instructing it to refresh the Markets
    // combo. (No point refreshing that before they've been downloaded, eh?)
    //
    // And at that time, there's no point emitting CurrentMarketChanged(m_qstrMarketID);
    // since we were just NOTIFIED of that -- that's why we're here -- so why pass the football
    // back and for for eternity? We will already emit CurrentMarketChanged if someone changes
    // the selection on the combo box.
    //
    // Not only do we cause an infinite refresh, but DlgMarkets::
    // onCurrentMarketChanged also retrieves the Offers by calling DlgMarkets::onNeedToLoadOrRetrieveOffers,
    // which downloads the offers and then calls m_pOfferDetails->show_widget(MTDetailEdit::DetailEditTypeOffer);
    //
    // Do we need that to happen? No, since onMarketIDChangedFromAbove is what calls the function
    // we're in, LoadOrRetrieveMarkets, and IMMEDIATELY after, it emits NeedToLoadOrRetrieveOffers(m_qstrMarketID) anyway.
    // Thus, DlgMarkets has already triggered onNeedToLoadOrRetrieveMarkets, even without the below emission.
    //
//    if (qstrOldMarketID != m_qstrMarketID)
//        emit CurrentMarketChanged(m_qstrMarketID);
    // -----------------------------------------------
}


void MTDetailEdit::SetCurrentMarketIDBasedOnIndex(int index)
{
    if ((m_mapMarkets.size() > 0) && (index >= 0) && (index < m_mapMarkets.size()))
    {
        int nCurrentIndex = -1;

        for (mapIDName::iterator it_map = m_mapMarkets.begin(); it_map != m_mapMarkets.end(); ++it_map)
        {
            ++nCurrentIndex; // zero on first iteration.

            if (nCurrentIndex == index)
            {
                m_qstrMarketID = it_map.key();
                break;
            }
        }
    }
    // ------------------------------------------
    else
        m_qstrMarketID = QString("");
}

// Market is selected from combo box, on Offers page.
void MTDetailEdit::on_comboBox_currentIndexChanged(int index)
{
    if (ui && (MTDetailEdit::DetailEditTypeOffer == m_Type))
    {
        // -----------------------------
        SetCurrentMarketIDBasedOnIndex(index);
        // -----------------------------

        // -----------------------------
//        emit NeedToLoadOrRetrieveOffers(m_qstrMarketID);
        emit CurrentMarketChanged(m_qstrMarketID);
        // ----------------------------
    }
}

void MTDetailEdit::ClearRecords()
{
    ui->tableWidget->blockSignals(true);
    // -------------------------------------------------------
    int nGridItemCount = ui->tableWidget->rowCount();
    // -------------------------------------------------------
    for (int ii = 0; ii < nGridItemCount; ii++)
    {
        QTableWidgetItem * item = ui->tableWidget->takeItem(0,1); // Row 0, Column 1
        ui->tableWidget->removeRow(0); // Row 0.

        if (NULL != item)
        {
            delete item;
            item = NULL;
        }
    }
    // -------------------------------------------------------
    ui->tableWidget->setRowCount(0);

    ui->tableWidget->blockSignals(false);

    m_map.clear();

    m_nCurrentRow     = -1;
    m_qstrCurrentID   = QString("");
    m_qstrCurrentName = QString("");

    ui->deleteButton->setEnabled(false);

    if (m_pDetailPane)
        m_pDetailPane->ClearContents();

    m_pTabWidget->setVisible(false);
}

void MTDetailEdit::ClearContents()
{
    if (m_pDetailPane && ui)
        m_pDetailPane->ClearContents();
}

void MTDetailEdit::RefreshRecords()
{
    ui->tableWidget->blockSignals(true);
    // ----------------------------------------------------------------------
    int mapSize = m_map.size();
    // -------------------------------------------------------
    int nTotalRecords = mapSize;
    // -------------------------------------------------------
    int nGridItemCount = ui->tableWidget->rowCount();
    // -------------------------------------------------------
    for (int ii = 0; ii < nGridItemCount; ii++)
    {
        QTableWidgetItem * item = ui->tableWidget->takeItem(0,1); // Row 0, Column 1
        ui->tableWidget->removeRow(0); // Row 0.

        if (NULL != item)
        {
            delete item;
            item = NULL;
        }
    }
    // -------------------------------------------------------
    ui->tableWidget->setRowCount(nTotalRecords);
    // -------------------------------------------------------
    int nPreselectedIndex = -1;
    // --------------------------------
    int nIndex = -1;
    for (mapIDName::iterator ii = m_map.begin(); ii != m_map.end(); ii++)
    {
        ++nIndex; // 0 on first iteration.

        qDebug() << "MTDetailEdit Iteration: " << nIndex;
        // -------------------------------------
        QString qstrID    = ii.key();
        QString qstrValue = ii.value();

        qDebug() << QString("MTDetailEdit::RefreshRecords: Name: %1, ID: %2").arg(qstrValue, qstrID);
        // -------------------------------------
        if (!m_PreSelected.isEmpty() && (m_PreSelected == qstrID))
            nPreselectedIndex = nIndex;
        // -------------------------------------
        QWidget * pWidget = NULL;

        // -------------------------------------------
        switch (m_Type)
        {
        case MTDetailEdit::DetailEditTypeContact:
            pWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, qstrID, qstrValue, "", "");
//          pWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, qstrID, qstrValue, "", "", ":/icons/icons/user.png");
            break;

        case MTDetailEdit::DetailEditTypeNym:
            pWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, qstrID, qstrValue, "", "");
//          pWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, qstrID, qstrValue, "", "", ":/icons/icons/identity_BW.png");
            break;

        case MTDetailEdit::DetailEditTypeServer:
            pWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, qstrID, qstrValue, "", "");
//          pWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, qstrID, qstrValue, "", "", ":/icons/server");
            break;

        case MTDetailEdit::DetailEditTypeAsset:
        {
            // Not exposed yet through API. Todo.
//            QString qstrCurrencySymbol =

            pWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, qstrID, qstrValue, "", "");
//          pWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, qstrID, qstrValue, "", "", ":/icons/icons/assets.png");
            break;
        }

        case MTDetailEdit::DetailEditTypeAccount:
        {
            QString qstrAmount = MTHome::shortAcctBalance(qstrID);

            pWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, qstrID, qstrValue, qstrAmount, "");
//          pWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, qstrID, qstrValue, qstrAmount, "", ":/icons/icons/vault.png");
            break;
        }

        case MTDetailEdit::DetailEditTypeMarket: // mc_systrayIcon_markets
        {
            // FYI, contents of qstrID:
//          QString qstrCompositeID = QString("%1,%2").arg(qstrMarketID).arg(qstrScale);

            QString     qstrMarketID, qstrMarketScale;
            QStringList theIDs = qstrID.split(","); // theIDs.at(0) MarketID, at(1) market Scale

            if (2 == theIDs.size()) // Should always be 2...
            {
                qstrMarketID    = theIDs.at(0);
                qstrMarketScale = theIDs.at(1);
            }
            // -------------------------------------
            QString qstrDisplayScale("");

            qstrDisplayScale = QString("<font size=1 color=grey>%1:</font> %2").
                    arg(tr("Priced per")).arg(qstrMarketScale);
            // -------------------------------------
            if (m_pmapMarkets)
            {
                QMap<QString, QVariant>::iterator it_market = m_pmapMarkets->find(qstrID);

                if (m_pmapMarkets->end() != it_market)
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
                        qstrDisplayScale = QString("<font size=1 color=grey>%1:</font> %2").
                                arg(tr("Priced per")).arg(qstrFormattedScale);
                    }
                }
            }
            // -------------------------------------
            pWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, qstrMarketID, qstrValue, qstrDisplayScale, "");
//          pWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, qstrMarketID, qstrValue, qstrDisplayScale, "", ":/icons/markets");
            break;
        }

        case MTDetailEdit::DetailEditTypeOffer:
        {
            if (m_pmapOffers)
            {
                // -------------------------------------
                QString     qstrServerID, qstrTransactionID;
                QStringList theIDs = qstrID.split(","); // theIDs.at(0) ServerID, at(1) transaction ID

                if (2 == theIDs.size()) // Should always be 2...
                {
                    qstrServerID      = theIDs.at(0);
                    qstrTransactionID = theIDs.at(1);
                }
                // -------------------------------------
                QMap<QString, QVariant>::iterator it_offer = m_pmapOffers->find(qstrID);

                if (m_pmapOffers->end() != it_offer)
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
                        int64_t     lScale            = OTAPI_Wrap::It()->StringToLong(pOfferData->scale);
                        std::string str_scale         = OTAPI_Wrap::It()->FormatAmount(pOfferData->asset_type_id, lScale);
                        // ------------------------------------------------------
                        int64_t     lPrice            = OTAPI_Wrap::It()->StringToLong(pOfferData->price_per_scale);
                        std::string str_price         = OTAPI_Wrap::It()->FormatAmount(pOfferData->currency_type_id, lPrice);
                        // ------------------------------------------------------
                        QString qstrPrice(tr("market order"));

                        if (lPrice > 0)
                            qstrPrice = QString("%1: %2").arg(tr("Price")).arg(QString::fromStdString(str_price));
                        // ------------------------------------------------------
                        QString qstrFormattedScale    = QString::fromStdString(str_scale);

                        qstrPrice += QString(" (%1 %2)").arg(tr("per")).arg(qstrFormattedScale);
                        // ------------------------------------------------------
                        QString qstrTotalAssets       = QString::fromStdString(OTAPI_Wrap::It()->FormatAmount(pOfferData->asset_type_id, lTotalAssets));
                        QString qstrSoldOrPurchased   = QString::fromStdString(OTAPI_Wrap::It()->FormatAmount(pOfferData->asset_type_id, lFinished));
                        // ------------------------------------------------------
                        std::string str_asset_name    = OTAPI_Wrap::It()->GetAssetType_Name(pOfferData->asset_type_id);
                        // -----------------------------------------------------------------------
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
                        QString qstrOfferName = QString("%1 %2: %3").
                                arg(qstrBuySell).
                                arg(QString::fromStdString(str_asset_name)).
                                arg(qstrAmounts);
                        // -------------------------------------
                        QString qstrTrans = QString("%1# %2").arg(tr("Trans")).arg(qstrTransactionID);
                        // -------------------------------------
                        pWidget = MTEditDetails::CreateDetailHeaderWidget(m_Type, qstrPrice, qstrOfferName,
                                                                          qstrCompleted, qstrTrans, "");
                    }
                }
            }

            break;
        }

        case MTDetailEdit::DetailEditTypeAgreement: //mc_systrayIcon_advanced_agreements
            pWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, qstrID, qstrValue, "", "");
            break;

        case MTDetailEdit::DetailEditTypeCorporation://mc_systrayIcon_advanced_corporations
            pWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, qstrID, qstrValue, "", "");
            break;

        default:
            qDebug() << "MTDetailEdit::RefreshRecords: MTDetailEdit::DetailEditTypeError";
            return;
        }
        // -------------------------------------------
        if (NULL != pWidget)
            ui->tableWidget->setCellWidget( nIndex, 1, pWidget );
        else
            qDebug() << "Failed creating detail header widget in MTDetailEdit::RefreshRecords()";
        // -------------------------------------------
    } // For loop
    // ------------------------
    // We are doing this here so onCurrentCellChanged can respond even if we set the current cell to -1.
    // TODO: If I end up having to remove the -1, then I will have to remove this here as well.
    //
    ui->tableWidget->blockSignals(false);

    if (ui->tableWidget->rowCount() > 0)
    {
//        ui->tableWidget->blockSignals(false);

        // Unnecessary, since the below call to setCurrentCell already triggers onCellChanged,
        // which already sets the tab widget visible or not, based on the new cell index.
        //
//        m_pTabWidget->setVisible(true);

        if ((nPreselectedIndex > (-1)) && (nPreselectedIndex < ui->tableWidget->rowCount()))
        {
            qDebug() << QString("SETTING current row to %1 on the tableWidget.").arg(nPreselectedIndex);
            ui->tableWidget->setCurrentCell(nPreselectedIndex, 1);
        }
        else
        {
            qDebug() << "SETTING current row to 0 on the tableWidget.";
            ui->tableWidget->setCurrentCell(0, 1);
        }
    }
    // ------------------------    
    // This is all handled in ui->tableWidget=>on_currentCell_changed.
    else
        ui->tableWidget->setCurrentCell(-1, -1); // NEW -- If this doesn't work, we'll maybe call this by hand:
//    void MTDetailEdit::on_tableWidget_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)

//    else
//    {
//        ui->deleteButton->setEnabled(false);

//        if (m_pDetailPane)
//            m_pDetailPane->ClearContents();

//        m_pTabWidget->setVisible(false);
//    }
    // --------------------------------------
//    if (!m_qstrCurrentID.isEmpty() && (MTDetailEdit::DetailEditTypeMarket == m_Type))
//        emit CurrentMarketChanged(m_qstrCurrentID);
}


void MTDetailEdit::on_addButton_clicked()
{
    if (m_pDetailPane)
        m_pDetailPane->AddButtonClicked();
}

void MTDetailEdit::on_deleteButton_clicked()
{
    if (!m_qstrCurrentID.isEmpty() && (m_pDetailPane))
        m_pDetailPane->DeleteButtonClicked();
}

void MTDetailEdit::on_tableWidget_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    Q_UNUSED(currentColumn);
    Q_UNUSED(previousRow);
    Q_UNUSED(previousColumn);

    m_nCurrentRow = currentRow;
    // -------------------------------------
    QString qstrOldID = m_qstrCurrentID;
    // -------------------------------------
    if ((currentRow >= 0) && (currentRow < m_map.size()))
    {
        m_pTabWidget->setVisible(true);

        int nIndex = -1;

        // Here we find m_qstrCurrentID and m_qstrCurrentName (display name)
        // based on its index in the listbox, by iterating through m_map.
        // Then

        for (mapIDName::iterator ii = m_map.begin(); ii != m_map.end(); ii++)
        {
            ++nIndex; // 0 on first iteration.
            // -------------------------------------
            if (nIndex == m_nCurrentRow) // <===== ONLY HAPPENS ONCE <=====
            {
                m_qstrCurrentID   = ii.key();
                m_qstrCurrentName = ii.value();

                if (m_bEnableDelete)
                    ui->deleteButton->setEnabled(true);

                qDebug() << "SETTING current row to " << nIndex << " on the tableWidget.";
                // ----------------------------------------
                m_PreSelected = m_qstrCurrentID;

                if (m_pDetailPane)
                    m_pDetailPane->refresh(m_qstrCurrentID, m_qstrCurrentName);
                // ----------------------------------------
                break; // <=== ONLY HAPPENS ONCE <=====
            }//resume
        }
    }
    // -------------------------------------
    else
    {
        m_nCurrentRow     = -1;
        m_qstrCurrentID   = QString("");
        m_qstrCurrentName = QString("");

        ui->deleteButton->setEnabled(false);

        if (m_pDetailPane)
            m_pDetailPane->ClearContents();

        m_pTabWidget->setVisible(false);
    }
    // -------------------------------------
    // If this is the markets page, and the current ID has changed, then we need
    // to notify the offers page.
    //
    if ((MTDetailEdit::DetailEditTypeMarket == m_Type) &&
//      (qstrOldID.isEmpty() || (!qstrOldID.isEmpty() && (qstrOldID != m_qstrCurrentID))))
        (qstrOldID.isEmpty() || (qstrOldID != m_qstrCurrentID)))
    {
        SetMarketID(m_qstrCurrentID);
        emit CurrentMarketChanged(m_qstrCurrentID);
    }
}

//public slot
void MTDetailEdit::onSetNeedToRetrieveOfferTradeFlags()
{
    if (m_pDetailPane)
    {
        MTEditDetails   * pEditDetails   = m_pDetailPane.data();
        MTMarketDetails * pMarketDetails = (MTMarketDetails * )pEditDetails;

        pMarketDetails->onSetNeedToRetrieveOfferTradeFlags();
    }
}


void MTDetailEdit::SetPreSelected(QString strSelected)
{
    m_PreSelected = strSelected;
}

bool MTDetailEdit::eventFilter(QObject *obj, QEvent *event)\
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
