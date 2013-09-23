#include <QApplication>
#include <QMessageBox>
#include <QScrollArea>
#include <QDebug>


#include "moneychanger.h"

#include "home.h"
#include "ui_home.h"

#include "homedetail.h"

#include "ot_worker.h"


MTHome::MTHome(QWidget *parent) :
    QWidget(parent, Qt::Window),
    already_init(false),
    m_pDetailPane(NULL),
    m_pDetailLayout(NULL),
    m_list(*(new MTNameLookupQT)),
    ui(new Ui::MTHome)
{
    ui->setupUi(this);

    this->installEventFilter(this);
}

MTHome::~MTHome()
{
    delete m_pDetailPane;
    delete m_pDetailLayout;

    delete ui;
}

bool MTHome::eventFilter(QObject *obj, QEvent *event){

    if (event->type() == QEvent::Close) {
        ((Moneychanger *)parentWidget())->close_overview_dialog();
        return true;
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape){
            close(); // This is caught by this same filter.
            return true;
        }
        return true;
    }else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}


void MTHome::dialog()
{
/** Overview Dialog **/

    /** If the overview dialog has already been init
     *  just show it, Other wise, init and show if this is
     *  the first time.
     **/
    if (!already_init)
    {
        ui->tableWidget->setColumnCount(2);
        ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        // -------------------------------------------
        ui->tableWidget->horizontalHeader()->resizeSection(0, 3);
        // -------------------------------------------
        ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
        ui->tableWidget->verticalHeader()->setDefaultSectionSize(60);
        ui->tableWidget->verticalHeader()->hide();
        ui->tableWidget->horizontalHeader()->hide();
        // -------------------------------------------
        m_pDetailPane = new MTHomeDetail;
        m_pDetailLayout = new QVBoxLayout;
        m_pDetailLayout->addWidget(m_pDetailPane);

        ui->frame->setLayout(m_pDetailLayout);
        // -------------------------------------------

        setupRecordList();

        // -------------------------------------------

        /** Flag Already Init **/
        already_init = true;
    }

    //Refresh visual data
    //Tell OT to repopulate, and refresh backend.
//    ((Moneychanger*)parentWidget())->get_ot_worker_background()->mc_overview_ping();

    //Now refresh the repopulated data visually
    RefreshRecords();
}

void MTHome::setupRecordList()
{
    int nServerCount  = OTAPI_Wrap::GetServerCount();
    int nAssetCount   = OTAPI_Wrap::GetAssetTypeCount();
    int nNymCount     = OTAPI_Wrap::GetNymCount();
    int nAccountCount = OTAPI_Wrap::GetAccountCount();
    // ----------------------------------------------------
    m_list.ClearServers();
    m_list.ClearAssets();
    m_list.ClearNyms();
    m_list.ClearAccounts();
    // ----------------------------------------------------
    for (int ii = 0; ii < nServerCount; ++ii)
    {
        std::string serverId = OTAPI_Wrap::GetServer_ID(ii);
        m_list.AddServerID(serverId);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nAssetCount; ++ii)
    {
        std::string assetId = OTAPI_Wrap::GetAssetType_ID(ii);
        m_list.AddAssetID(assetId);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nNymCount; ++ii)
    {
        std::string nymId = OTAPI_Wrap::GetNym_ID(ii);
        m_list.AddNymID(nymId);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nAccountCount; ++ii)
    {
        std::string accountID = OTAPI_Wrap::GetAccountWallet_ID(ii);
        m_list.AddAccountID(accountID);
    }
    // ----------------------------------------------------
    m_list.AcceptChequesAutomatically  (true);
    m_list.AcceptReceiptsAutomatically (true);
    m_list.AcceptTransfersAutomatically(true);
}

void MTHome::on_tableWidget_cellClicked(int row, int column)
{
    m_pDetailPane->refresh(row, m_list);
}


void MTHome::RefreshUserBar()
{

}


void MTHome::RefreshRecords()
{
    //(Lock the overview dialog refreshing mechinism until finished)
    QMutexLocker overview_refresh_locker(&mc_overview_refreshing_visuals_mutex);
    // -------------------------------------------------------
    m_list.Populate();
    int listSize       = m_list.size();
    // -------------------------------------------------------
    int nTotalRecords  = listSize;
    // -------------------------------------------------------
    int nGridItemCount = ui->tableWidget->rowCount();
    // -------------------------------------------------------
    for (int ii = 0; ii < nGridItemCount; ii++)
    {
        QTableWidgetItem * item = ui->tableWidget->takeItem(0,1); // Row 0, Column 1
        ui->tableWidget->removeRow(0); // Row 0.

        if (NULL != item)
            delete item;
    }
    // -------------------------------------------------------
    ui->tableWidget->setRowCount(nTotalRecords);
    // -------------------------------------------------------
    for (int nIndex = 0; nIndex < listSize; ++nIndex)
    {
        weak_ptr_MTRecord   weakRecord = m_list.GetRecord(nIndex);
        shared_ptr_MTRecord record     = weakRecord.lock();

        if (weakRecord.expired())
        {
            OTLog::Output(2, "Reloading table due to expired pointer.\n");
            m_list.Populate();
            listSize = m_list.size();
            nIndex = 0;
        }
        else
        {
            MTRecord & recordmt = *record;
            QWidget  * pWidget  = MTHomeDetail::CreateDetailHeaderWidget(recordmt);
            // -------------------------------------------
            if (NULL != pWidget)
                ui->tableWidget->setCellWidget( nIndex, 1, pWidget );
            else
                qDebug() << "Failed creating detail header widget based on MTRecord.";
        }
    }
    // -------------------------------------------------------
}




