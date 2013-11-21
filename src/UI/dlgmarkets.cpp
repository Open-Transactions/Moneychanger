
#include <QKeyEvent>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QDebug>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>

#include "dlgmarkets.h"
#include "ui_dlgmarkets.h"

#include "moneychanger.h"

#include "detailedit.h"


DlgMarkets::DlgMarkets(QWidget *parent) :
    QDialog(parent),
    m_bFirstRun(true),
    ui(new Ui::DlgMarkets)
{
    ui->setupUi(this);

    this->installEventFilter(this);
}

void DlgMarkets::ClearMarketMap()
{
    QMap<QString, OTDB::MarketData *> temp_map = m_mapMarkets;

    m_mapMarkets.clear();
    // ------------------------------------
    for (QMap<QString, OTDB::MarketData *>::iterator it_map = temp_map.begin();
         it_map != temp_map.end(); ++it_map)
    {
        OTDB::MarketData * pMarketData = it_map.value();

        if (NULL != pMarketData) // should never be NULL.
            delete pMarketData;
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
        }
        // ******************************************************


        // ******************************************************
        {
        m_pOfferDetails = new MTDetailEdit(this);
        // -------------------------------------
        m_pOfferDetails->setWindowTitle(tr("Offers"));
        // -------------------------------------
        QVBoxLayout * pLayout = new QVBoxLayout;
        pLayout->addWidget(m_pOfferDetails);

        m_pOfferDetails->setContentsMargins(1,1,1,1);
        pLayout->setContentsMargins(1,1,1,1);

        ui->tabOffers->setContentsMargins(1,1,1,1);

        ui->tabOffers->setLayout(pLayout);
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

    }
}


void DlgMarkets::on_pushButtonRefresh_clicked()
{
    RefreshRecords();
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
        OTDB::MarketList * pMarketList = LoadMarketList(qstrServerID.toStdString());
        OTCleanup<OTDB::MarketList> theAngel(pMarketList);

        if (NULL != pMarketList)
        {
            size_t nMarketDataCount = pMarketList->GetMarketDataCount();

            for (size_t ii = 0; ii < nMarketDataCount; ++ii)
            {
                OTDB::MarketData * pMarketData = pMarketList->GetMarketData(ii);

                if (NULL == pMarketData) // Should never happen.
                    continue;
                // --------------------------
                const std::string str_asset_name    = OTAPI_Wrap::GetAssetType_Name(pMarketData->asset_type_id);
                const std::string str_currency_name = OTAPI_Wrap::GetAssetType_Name(pMarketData->currency_type_id);
                // --------------------------
                QString qstrMarketID   = QString::fromStdString(pMarketData->market_id);
                QString qstrMarketName = QString("%1-%2").
                        arg(QString::fromStdString(str_asset_name)).
                        arg(QString::fromStdString(str_currency_name));
                // ---------------------------
                QString qstrCompositeID = QString("%1,%2").arg(qstrServerID).arg(qstrMarketID);

                the_map.insert(qstrCompositeID, qstrMarketName);
                // ---------------------------
                m_mapMarkets.insertMulti(qstrServerID, pMarketData->clone());
            } // for
        }
    }

    return bSuccess;
}



// Caller responsible to delete!
//
OTDB::MarketList * DlgMarkets::LoadMarketList(const std::string & serverID)
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
            RetrieveMarketList(the_map); // Download the list of markets from the server(s).
            // -------------------------------------
            m_pMarketDetails->show_widget(MTDetailEdit::DetailEditTypeMarket);
        }
        // ***********************************************
        {
            mapIDName & the_map = m_pOfferDetails->m_map;
            // -------------------------------------
            the_map.clear();

            // TODO: populate the map here.

            // -------------------------------------------
            m_pOfferDetails->show_widget(MTDetailEdit::DetailEditTypeOffer);
        }
        // ***********************************************
    }
    else
    {
        m_pMarketDetails->setVisible(false);
        m_pOfferDetails ->setVisible(false);
    }
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
        m_nymId = QString("");
    // -----------------------------------------------
    if (m_mapServers.size() > 0)
    {
        SetCurrentServerIDBasedOnIndex(nDefaultServerIndex);
        ui->comboBoxServer->setCurrentIndex(nDefaultServerIndex);
    }
    else
        m_serverId = QString("");
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
}


