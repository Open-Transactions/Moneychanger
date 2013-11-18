#include <QDebug>

#include "detailedit.h"
#include "ui_detailedit.h"

#include "editdetails.h"

#include "home.h"

#include "contactdetails.h"
#include "nymdetails.h"
#include "serverdetails.h"
#include "assetdetails.h"
#include "accountdetails.h"
#include "agreementdetails.h"
#include "corporationdetails.h"
#include "marketdetails.h"
#include "offerdetails.h"

#include "moneychanger.h"

#include <OTStorage.h>
#include <OTLog.h>

MTDetailEdit::MTDetailEdit(QWidget *parent) :
    QWidget(parent, Qt::Window),
    m_bFirstRun(true),
    m_nCurrentRow(-1),
    m_pmapMarkets(NULL),
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


// This only does something the first time you run it.
//
void MTDetailEdit::FirstRun(MTDetailEdit::DetailEditType theType)
{
    if (m_bFirstRun)
    {
        m_bFirstRun = false;
        // -------------------------------------------
        ui->tableWidget->setColumnCount(2);
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
        m_pTabWidget->addTab(pTab1, tr("Details"));
        // -------------------------------------------
        // Instantiate m_pDetailPane to one of various types.
        //
        m_Type = theType;

        switch (m_Type)
        {
        case MTDetailEdit::DetailEditTypeNym:         m_pDetailPane = new MTNymDetails(this, *this);         break;
        case MTDetailEdit::DetailEditTypeContact:     m_pDetailPane = new MTContactDetails(this, *this);     break;
        case MTDetailEdit::DetailEditTypeServer:      m_pDetailPane = new MTServerDetails(this, *this);      break;
        case MTDetailEdit::DetailEditTypeAsset:       m_pDetailPane = new MTAssetDetails(this, *this);       break;
        case MTDetailEdit::DetailEditTypeOffer:       m_pDetailPane = new MTOfferDetails(this, *this);       break;
        case MTDetailEdit::DetailEditTypeAgreement:   m_pDetailPane = new MTAgreementDetails(this, *this);   break;
        case MTDetailEdit::DetailEditTypeCorporation: m_pDetailPane = new MTCorporationDetails(this, *this); break;

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
            break;
        default:
            qDebug() << "MTDetailEdit::dialog: MTDetailEdit::DetailEditTypeError";
            return;
        }
        // -------------------------------------------
        m_pDetailPane->SetOwnerPointer(*this);
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
    } // first run.
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



void MTDetailEdit::RefreshRecords()
{
    disconnect(ui->tableWidget, SIGNAL(currentCellChanged(int,int,int,int)),
               this, SLOT(on_tableWidget_currentCellChanged(int,int,int,int)));
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
    QMap<QString, OTDB::MarketData *>::iterator it_markets;


//    QMessageBox::information(this, "", QCoreApplication::applicationDirPath());

    if (NULL != m_pmapMarkets)
    {
        it_markets = m_pmapMarkets->begin();




        // TODO: Remove this (where I loop through m_pmapMarkets simultaneous to looping through m_map.)
        //
        // ALSO: Change m_pmapMarkets to use the same composite server/market ID as m_map uses (for markets anyway.)
        //
        // That way, whenever market details or offer details need to query the market data by a unique ID, they will
        // be able to.
        //
        // The offer m_map also needs to use a composite ID: server/market/offer (comma separated.)
        //
        // Finish the markets / offers screens!


    }
    // ------------------------------------
    int nIndex = -1;
    for (mapIDName::iterator ii = m_map.begin(); ii != m_map.end(); ii++)
    {
        ++nIndex; // 0 on first iteration.

        qDebug() << "MTDetailEdit Iteration: " << nIndex;
        // -------------------------------------
        QString qstrMarketServerID(""), qstrMarketServerName("");
        OTDB::MarketData * pMarketData = NULL;

        if (NULL != m_pmapMarkets)
        {
            pMarketData          = it_markets.value();
            qstrMarketServerID   = it_markets.key();
            qstrMarketServerName = QString::fromStdString( OTAPI_Wrap::GetServer_Name(qstrMarketServerID.toStdString()) );
        }
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
            pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, "", "");
//          pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, "", "", ":/icons/icons/user.png");
            break;

        case MTDetailEdit::DetailEditTypeNym:
            pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, "", "");
//          pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, "", "", ":/icons/icons/identity_BW.png");
            break;

        case MTDetailEdit::DetailEditTypeServer:
            pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, "", "");
//          pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, "", "", ":/icons/server");
            break;

        case MTDetailEdit::DetailEditTypeAsset:
        {
            // Not exposed yet through API. Todo.
//            QString qstrCurrencySymbol =

            pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, "", "");
//          pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, "", "", ":/icons/icons/assets.png");
            break;
        }

        case MTDetailEdit::DetailEditTypeAccount:
        {
            QString qstrAmount = MTHome::shortAcctBalance(qstrID);

            pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, qstrAmount, "");
//          pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, qstrAmount, "", ":/icons/icons/vault.png");
            break;
        }

        case MTDetailEdit::DetailEditTypeMarket: // mc_systrayIcon_markets
            pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, qstrMarketServerName, "");
            break;

        case MTDetailEdit::DetailEditTypeOffer:
            pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, "", "");
            break;

        case MTDetailEdit::DetailEditTypeAgreement: //mc_systrayIcon_advanced_agreements
            pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, "", "");
            break;

        case MTDetailEdit::DetailEditTypeCorporation://mc_systrayIcon_advanced_corporations
            pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, "", "");
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
        if (NULL != m_pmapMarkets)
        {
            it_markets++;
        }
    } // For loop
    // ------------------------
    if (ui->tableWidget->rowCount() > 0)
    {
        connect(ui->tableWidget, SIGNAL(currentCellChanged(int,int,int,int)),
                this, SLOT(on_tableWidget_currentCellChanged(int,int,int,int)));

        m_pTabWidget->setVisible(true);

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
    else
    {
        ui->deleteButton->setEnabled(false);

        if (m_pDetailPane)
            m_pDetailPane->ClearContents();

        m_pTabWidget->setVisible(false);
    }
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
    m_nCurrentRow = currentRow;
    // -------------------------------------
    if (m_nCurrentRow >= 0)
    {
        m_pTabWidget->setVisible(true);

        int nIndex = -1;

        for (mapIDName::iterator ii = m_map.begin(); ii != m_map.end(); ii++)
        {
            ++nIndex; // 0 on first iteration.
            // -------------------------------------
            if (nIndex == m_nCurrentRow) // ONLY HAPPENS ONCE
            {
                m_qstrCurrentID   = ii.key();
                m_qstrCurrentName = ii.value();

                ui->deleteButton->setEnabled(true);

                qDebug() << "SETTING current row to " << nIndex << " on the tableWidget.";
                // ----------------------------------------
                m_PreSelected = m_qstrCurrentID;

                if (m_pDetailPane)
                    m_pDetailPane->refresh(m_qstrCurrentID, m_qstrCurrentName);
                // ----------------------------------------
                return;
            }
        }
    }
    // -------------------------------------
    m_nCurrentRow     = -1;
    m_qstrCurrentID   = QString("");
    m_qstrCurrentName = QString("");

    ui->deleteButton->setEnabled(false);

    m_pTabWidget->setVisible(false);
    // -------------------------------------
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
