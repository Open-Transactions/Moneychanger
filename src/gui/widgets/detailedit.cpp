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
#include <gui/widgets/opentxscontactdetails.hpp>
#include <gui/widgets/marketdetails.hpp>
#include <gui/widgets/accountdetails.hpp>
#include <gui/widgets/transportdetails.hpp>
#include <gui/widgets/home.hpp>

#include <core/moneychanger.hpp>
#include <core/handlers/focuser.h>

#include <opentxs/api/Api.hpp>
#include <opentxs/api/Native.hpp>
#include <opentxs/OT.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>
#include <opentxs/core/OTStorage.hpp>

#include <QDialog>
#include <QKeyEvent>
#include <QDebug>
#include <QTimer>


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


bool MTDetailEdit::getAccountIDs(QString & qstrAssetAcctID, QString & qstrCurrencyAcctID)
{
    if (!m_pDetailPane)
        return false;

    MTEditDetails  & theDetailPane = *m_pDetailPane;
    MTOfferDetails * pOfferDetails = qobject_cast<MTOfferDetails *>(&theDetailPane);

    if (nullptr != pOfferDetails)
    {
        return pOfferDetails->getAccountIDs(qstrAssetAcctID, qstrCurrencyAcctID);
    }
    return false;
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

        Focuser f(this);
        f.show();
        f.focus();
    }
}

// -------------------------------------------

QWidget * MTDetailEdit::GetTab(int nTab)
{
    if (m_pTabWidget)
        return m_pTabWidget->widget(nTab);
    return nullptr;
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
        case MTDetailEdit::DetailEditTypeServer:

            if (Moneychanger::It()->expertMode())
            {
                EnableAdd   (true);
                EnableDelete(true);
            }
            else
            {
                EnableAdd   (false);
                EnableDelete(false);
            }

            m_pDetailPane = new MTServerDetails(this, *this);
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(newServerAdded(QString)),
                    Moneychanger::It(), SLOT  (onNewServerAdded(QString)));
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(newServerAdded(QString)),
                    m_pDetailPane,      SIGNAL(RefreshRecordsAndUpdateMenu()));
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(serversChanged()),
                    Moneychanger::It(), SLOT  (onServersChanged()));
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(serversChanged()),
                    m_pDetailPane,      SIGNAL(RefreshRecordsAndUpdateMenu()));
            // -------------------------------------------
            break;

        case MTDetailEdit::DetailEditTypeAsset:
            if (Moneychanger::It()->expertMode())
            {
                EnableAdd   (true);
                EnableDelete(true);
            }
            else
            {
                EnableAdd   (false);
                EnableDelete(false);
            }

            m_pDetailPane = new MTAssetDetails(this, *this);
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(newAssetAdded(QString)),
                    Moneychanger::It(), SLOT  (onNewAssetAdded(QString)));
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(newAssetAdded(QString)),
                    m_pDetailPane,      SIGNAL(RefreshRecordsAndUpdateMenu()));
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(assetsChanged()),
                    Moneychanger::It(), SLOT  (onAssetsChanged()));
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(assetsChanged()),
                    m_pDetailPane,      SIGNAL(RefreshRecordsAndUpdateMenu()));
            // -------------------------------------------

            // For new issuer accounts: (which are created on the assets page when a user registers the currency onto a server.)
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(newAccountAdded(QString)),
                    Moneychanger::It(), SLOT  (onNewAccountAdded(QString)));
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(newAccountAdded(QString)),
                    m_pDetailPane,      SIGNAL(RefreshRecordsAndUpdateMenu()));
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(newAccountAdded(QString)),
                    m_pDetailPane,      SIGNAL(ShowAccount(QString)));
            // -------------------------------------------
            break;

        case MTDetailEdit::DetailEditTypeNym:
            if (Moneychanger::It()->expertMode())
            {
                EnableAdd   (true);
                EnableDelete(true);
            }
            else
            {
                EnableAdd   (false);
                EnableDelete(false);
            }

            m_pDetailPane = new MTNymDetails(this, *this);
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(newNymAdded(QString)),    // This also adds the new Nym as a Contact in the address book.
                    Moneychanger::It(), SLOT  (onNewNymAdded(QString))); // (For convenience for the user.)
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(newNymAdded(QString)), // Why do we do this, since we haven't actually done a check_nym?
                    Moneychanger::It(), SLOT  (onCheckNym(QString))); // Because this upserts the new Nym's claims/verifications into the local DB.
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(newNymAdded(QString)),
                    m_pDetailPane,      SIGNAL(RefreshRecordsAndUpdateMenu()));
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(nymsChanged()),
                    Moneychanger::It(), SLOT  (onNymsChanged()));
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(nymsChanged()),
                    m_pDetailPane,      SIGNAL(RefreshRecordsAndUpdateMenu()));
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(nymWasJustChecked(QString)),
                    Moneychanger::It(), SLOT  (onCheckNym(QString)));
            // -------------------------------------------
            connect(Moneychanger::It(), SIGNAL(claimsUpdatedForNym(QString)),
                    m_pDetailPane,      SLOT  (onClaimsUpdatedForNym(QString)));
            // -------------------------------------------
            connect(m_pDetailPane, SIGNAL(appendToLog(QString)), Moneychanger::It(), SLOT(mc_showlog_slot(QString)));

            break;

        case MTDetailEdit::DetailEditTypeContact:
            // -------------------------------------------
            m_pDetailPane = new MTContactDetails(this, *this);
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(nymWasJustChecked(QString)),
                    Moneychanger::It(), SLOT  (onCheckNym(QString)));
            // -------------------------------------------
            connect(Moneychanger::It(), SIGNAL(claimsUpdatedForNym(QString)),
                    m_pDetailPane,      SLOT  (onClaimsUpdatedForNym(QString)));
            // -------------------------------------------
            break;

        case MTDetailEdit::DetailEditTypeOpentxsContact:
            // -------------------------------------------
            m_pDetailPane = new MTOpentxsContactDetails(this, *this);
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(nymWasJustChecked(QString)),
                    Moneychanger::It(), SLOT  (onCheckNym(QString)));
            // -------------------------------------------
            connect(Moneychanger::It(), SIGNAL(claimsUpdatedForNym(QString)),
                    m_pDetailPane,      SLOT  (onClaimsUpdatedForNym(QString)));
            // -------------------------------------------
            break;

        case MTDetailEdit::DetailEditTypeCorporation: m_pDetailPane = new MTCorporationDetails(this, *this); break;
        case MTDetailEdit::DetailEditTypeTransport:   m_pDetailPane = new TransportDetails    (this, *this); break;

        case MTDetailEdit::DetailEditTypeAgreement:
            ui->comboBox->setHidden(false);
            m_pDetailPane = new MTAgreementDetails  (this, *this);
            // -------------------------------------------
            connect(ui->comboBox, SIGNAL(currentIndexChanged(int)),
                    this,         SLOT  (on_comboBox_currentIndexChanged(int)));
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(runSmartContract(QString, QString, int32_t)),
                    Moneychanger::It(), SLOT  (onRunSmartContract(QString, QString, int32_t)));
            // -------------------------------------------
            break;

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
            if (Moneychanger::It()->expertMode())
            {
                EnableAdd   (true);
                EnableDelete(true);
            }
            else
            {
                EnableAdd   (false);
                EnableDelete(false);
            }

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
            connect(m_pDetailPane,      SIGNAL(newAccountAdded(QString)),
                    Moneychanger::It(), SLOT  (onNewAccountAdded(QString)));
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(newAccountAdded(QString)),
                    m_pDetailPane,      SIGNAL(RefreshRecordsAndUpdateMenu()));
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(accountsChanged()),
                    Moneychanger::It(), SLOT  (onAccountsChanged()));
            // -------------------------------------------
            connect(m_pDetailPane,      SIGNAL(accountsChanged()),
                    m_pDetailPane,      SIGNAL(RefreshRecordsAndUpdateMenu()));
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

//      connect(Moneychanger::It(), SIGNAL(expertModeUpdated(bool)), this, SLOT(onExpertModeUpdated(bool)));

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
        m_pTabWidget->setTabBarAutoHide(true);

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
        else
            ui->addButton->setVisible(true);
        // ----------------------------------
        if (!m_bEnableDelete)
            ui->deleteButton->setVisible(false);
        else
            ui->deleteButton->setVisible(true);
        // ----------------------------------
        if (m_pDetailPane)
            m_pDetailPane->setVisible(false);

        m_pTabWidget->setVisible(false);
        // ----------------------------------
        m_bFirstRun = false;

    } // first run.
}


void MTDetailEdit::onRefreshRecords()
{
    RefreshRecords();
}

void MTDetailEdit::onExpertModeUpdated(bool bExpertMode)
{
    if (bExpertMode)
    {
        EnableAdd   (true);
        EnableDelete(true);
    }
    else
    {
        EnableAdd   (false);
        EnableDelete(false);
    }

    // ----------------------------------
    if (!m_bEnableAdd)
        ui->addButton->setVisible(false);
    else
        ui->addButton->setVisible(true);
    // ----------------------------------
    if (!m_bEnableDelete)
        ui->deleteButton->setVisible(false);
    else
        ui->deleteButton->setVisible(true);
    // ----------------------------------
}

void MTDetailEdit::showEvent(QShowEvent * event)
{
    QWidget::showEvent(event);

    if (m_map.size() < 1)
    {
        QTimer::singleShot(0, this, SLOT(on_addButton_clicked()));
    }
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
            if (0 == it_markets.key().compare(qstrMarketID))
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
        if (!m_qstrMarketID.isEmpty() && (0 == m_qstrMarketID.compare(OT_market_id)))
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

void MTDetailEdit::RefreshLawyerCombo()
{
    if (MTDetailEdit::DetailEditTypeAgreement != m_Type)
        return;
    // ----------------------------
    ui->comboBox->blockSignals(true);
    // ----------------------------
    ui->comboBox->clear();
    // -----------------------------------------------
    m_mapLawyers.clear();
    // -----------------------------------------------
    const int32_t the_count = opentxs::OT::App().API().Exec().GetNymCount();
    // -----------------------------------------------
    for (int32_t ii = 0; ii < the_count; ++ii)
    {
        QString OT_id = QString::fromStdString(opentxs::OT::App().API().Exec().GetNym_ID(ii));
        QString OT_name("");
        // -----------------------------------------------
        if (!OT_id.isEmpty())
        {
            OT_name = QString::fromStdString(opentxs::OT::App().API().Exec().GetNym_Name(OT_id.toStdString()));
            // -----------------------------------------------
            m_mapLawyers.insert(OT_id, OT_name);
        }
     }
    // -----------------------------------------------
    int32_t nCurrentLawyerIndex = 0;
    bool    bFoundCurrentLawyer = false;
    // ----------------------------
    int nIndex = -1;
    for (mapIDName::iterator ii = m_mapLawyers.begin(); ii != m_mapLawyers.end(); ++ii)
    {
        ++nIndex; // 0 on first iteration.
        // ------------------------------
        QString OT_lawyer_id   = ii.key();   // This is a nym ID.
        QString OT_lawyer_name = ii.value(); // This is the display name aka "Trader Bob"
        // ------------------------------
        if (!m_qstrLawyerID.isEmpty() && (0 == m_qstrLawyerID.compare(OT_lawyer_id)))
        {
            bFoundCurrentLawyer = true;
            nCurrentLawyerIndex = nIndex;
        }
        // ------------------------------
        ui->comboBox->insertItem(nIndex, OT_lawyer_name);
    }
    // -----------------------------------------------
    if (m_mapLawyers.size() > 0)
    {
        SetCurrentLawyerIDBasedOnIndex(nCurrentLawyerIndex);
        ui->comboBox->setCurrentIndex (nCurrentLawyerIndex);
    }
    else
        SetCurrentLawyerIDBasedOnIndex(-1);
    // -----------------------------------------------
    ui->comboBox->blockSignals(false);
    // -----------------------------------------------
}


void MTDetailEdit::SetCurrentLawyerIDBasedOnIndex(int index)
{
    if ((m_mapLawyers.size() > 0) && (index >= 0) && (index < m_mapLawyers.size()))
    {
        int nCurrentIndex = -1;

        for (mapIDName::iterator it_map = m_mapLawyers.begin(); it_map != m_mapLawyers.end(); ++it_map)
        {
            ++nCurrentIndex; // zero on first iteration.

            if (nCurrentIndex == index)
            {
                m_qstrLawyerID = it_map.key();
                break;
            }
        }
    }
    // ------------------------------------------
    else
        m_qstrLawyerID = QString("");
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
    if (ui && (MTDetailEdit::DetailEditTypeAgreement == m_Type))
    {
        SetCurrentLawyerIDBasedOnIndex(index);
    }
    // --------------------------------------------------------
    if (ui && (MTDetailEdit::DetailEditTypeOffer == m_Type))
    {
        // -----------------------------
        SetCurrentMarketIDBasedOnIndex(index);
        // -----------------------------

        // -----------------------------
//      emit NeedToLoadOrRetrieveOffers(m_qstrMarketID);
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

    if (m_pDetailPane) {
        m_pDetailPane->ClearContents();
        m_pDetailPane->setVisible(false);
    }

    m_pTabWidget->setVisible(false);
}

void MTDetailEdit::ClearContents()
{
    if (m_pDetailPane && ui)
        m_pDetailPane->ClearContents();
}

void MTDetailEdit::RefreshRecords()
{
    // -------------------------------------------
    // This does nothing unless it's a smart contract DetailEdit.
    RefreshLawyerCombo();
    // -------------------------------------------
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
    if (m_map.size() > 0)
    {
        if (m_pDetailPane)
            m_pDetailPane->setVisible(true);
        m_pTabWidget->setVisible(true);
    }
    else
    {
        if (m_pDetailPane)
        {
            m_pDetailPane->ClearContents();
            m_pDetailPane->setVisible(false);
        }
        m_pTabWidget->setVisible(false);
    }
    // --------------------------------
    int nIndex = -1;
    for (mapIDName::iterator ii = m_map.begin(); ii != m_map.end(); ii++)
    {
        ++nIndex; // 0 on first iteration.

//        qDebug() << "MTDetailEdit Iteration: " << nIndex;
        // -------------------------------------
        QString qstrID    = ii.key();
        QString qstrValue = ii.value();

//        qDebug() << QString("MTDetailEdit::RefreshRecords: Name: %1, ID: %2").arg(qstrValue, qstrID);
        // -------------------------------------
        if (!m_PreSelected.isEmpty() && (0 == m_PreSelected.compare(qstrID)))
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

        case MTDetailEdit::DetailEditTypeOpentxsContact:
            pWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, qstrID, qstrValue, "", "");
//          pWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, qstrID, qstrValue, "", "", ":/icons/icons/user.png");
            break;

        case MTDetailEdit::DetailEditTypeTransport:
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
                    opentxs::OTDB::MarketData * pMarketData = VPtr<opentxs::OTDB::MarketData>::asPtr(it_market.value());

                    if (NULL != pMarketData) // Should never be NULL.
                    {
                        // ------------------------------------------------------
                        int64_t     lScale    = opentxs::OT::App().API().Exec().StringToLong(pMarketData->scale);
                        std::string str_scale = opentxs::OT::App().API().Exec().FormatAmount(pMarketData->instrument_definition_id, lScale);
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
                QString     qstrNotaryID, qstrTransactionID;
                QStringList theIDs = qstrID.split(","); // theIDs.at(0) NotaryID, at(1) transaction ID

                if (2 == theIDs.size()) // Should always be 2...
                {
                    qstrNotaryID      = theIDs.at(0);
                    qstrTransactionID = theIDs.at(1);
                }
                // -------------------------------------
                QMap<QString, QVariant>::iterator it_offer = m_pmapOffers->find(qstrID);

                if (m_pmapOffers->end() != it_offer)
                {
                    // ------------------------------------------------------
                    opentxs::OTDB::OfferDataNym * pOfferData = VPtr<opentxs::OTDB::OfferDataNym>::asPtr(it_offer.value());

                    if (NULL != pOfferData) // Should never be NULL.
                    {
                        bool        bSelling          = pOfferData->selling;
                        // ------------------------------------------------------
                        int64_t     lTotalAssets      = opentxs::OT::App().API().Exec().StringToLong(pOfferData->total_assets);
                        int64_t     lFinished         = opentxs::OT::App().API().Exec().StringToLong(pOfferData->finished_so_far);
                        // ------------------------------------------------------
                        int64_t     lScale            = opentxs::OT::App().API().Exec().StringToLong(pOfferData->scale);
                        std::string str_scale         = opentxs::OT::App().API().Exec().FormatAmount(pOfferData->instrument_definition_id, lScale);
                        // ------------------------------------------------------
                        int64_t     lPrice            = opentxs::OT::App().API().Exec().StringToLong(pOfferData->price_per_scale);
                        std::string str_price         = opentxs::OT::App().API().Exec().FormatAmount(pOfferData->currency_type_id, lPrice);
                        // ------------------------------------------------------
                        QString qstrPrice(tr("market order"));

                        if (lPrice > 0)
                            qstrPrice = QString("%1: %2").arg(tr("Price")).arg(QString::fromStdString(str_price));
                        // ------------------------------------------------------
                        QString qstrFormattedScale    = QString::fromStdString(str_scale);

                        qstrPrice += QString(" (%1 %2)").arg(tr("per")).arg(qstrFormattedScale);
                        // ------------------------------------------------------
                        QString qstrTotalAssets       = QString::fromStdString(opentxs::OT::App().API().Exec().FormatAmount(pOfferData->instrument_definition_id, lTotalAssets));
                        QString qstrSoldOrPurchased   = QString::fromStdString(opentxs::OT::App().API().Exec().FormatAmount(pOfferData->instrument_definition_id, lFinished));
                        // ------------------------------------------------------
                        std::string str_asset_name    = opentxs::OT::App().API().Exec().GetAssetType_Name(pOfferData->instrument_definition_id);
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
            ui->tableWidget->blockSignals(true);
            return;
        }
        // -------------------------------------------
        if (NULL != pWidget)
            ui->tableWidget->setCellWidget( nIndex, 1, pWidget );
        else
            qDebug() << "Failed creating detail header widget in MTDetailEdit::RefreshRecords()";
        // -------------------------------------------
    } // For loop
    // -----------------------------------
    if (ui->tableWidget->rowCount() <= 0)
    {

        ui->tableWidget->setCurrentCell(-1, 1); // NEW -- If this doesn't work, we'll maybe call this by hand:
        ui->tableWidget->blockSignals(false);
        on_tableWidget_currentCellChanged(-1, 1, 0, 1);
    }
    else
    {
        if ((nPreselectedIndex > (-1)) && (nPreselectedIndex < ui->tableWidget->rowCount()))
        {
            ui->tableWidget->setCurrentCell(nPreselectedIndex, 1);
            ui->tableWidget->blockSignals(false);
            on_tableWidget_currentCellChanged(nPreselectedIndex, 1, 0, 1);
        }
        else
        {
            ui->tableWidget->setCurrentCell(0, 1);
            ui->tableWidget->blockSignals(false);
            on_tableWidget_currentCellChanged(0, 1, 0, 1);
        }
    }
    // ------------------------
    // NOTE: Why am I blocking the signal, then setting the current cell, and then unblocking
    // the signal, and then calling currentCellChanged by HAND?
    // The reason is that currentCellChanged doesn't always get triggered when I want it to.
    // (For various different reasons.) It usually gets triggered when a user has clicked on
    // the grid, but sometimes it doesn't when programmatically. Like when the grid is programmatically
    // cleared, and then setCurrentCell is called to -1, it was already -1, and so there was no "change"
    // and thus the currentCellChanged event doesn't get triggered. That's just one example. So what
    // I'm doing is, I'm letting the event handle the clicks, but when I set the current cell by HAND,
    // then I'm blocking signals to do it, and then I unblock and call currentCellChanged by HAND as well.
    //
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
        m_pDetailPane->setVisible(true);
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

//                qDebug() << "SETTING current row to " << nIndex << " on the tableWidget.";
                // ----------------------------------------
                m_PreSelected = m_qstrCurrentID;

                if (m_pDetailPane)
                    m_pDetailPane->refresh(m_qstrCurrentID, m_qstrCurrentName);
                // ----------------------------------------
                break; // <=== ONLY HAPPENS ONCE <=====
            }
        }
    }
    // -------------------------------------
    else
    {
        m_nCurrentRow     = -1;
        m_qstrCurrentID   = QString("");
        m_qstrCurrentName = QString("");

        ui->deleteButton->setEnabled(false);

        if (m_pDetailPane) {
            m_pDetailPane->ClearContents();
            m_pDetailPane->setVisible(false);
        }

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
    return QWidget::eventFilter(obj, event);
}
