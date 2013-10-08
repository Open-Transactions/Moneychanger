#include <QApplication>
#include <QMessageBox>
#include <QPushButton>
#include <QToolButton>
#include <QScrollArea>
#include <QDebug>


#include "moneychanger.h"

#include "home.h"
#include "ui_home.h"

#include "homedetail.h"

#include "Handlers/contacthandler.h"


MTHome::MTHome(QWidget *parent) :
    QWidget(parent, Qt::Window),
    already_init(false),
    m_pDetailPane(NULL),
    m_pDetailLayout(NULL),
    m_pHeaderLayout(NULL),
    m_list(*(new MTNameLookupQT)),
    m_bNeedRefresh(false),
    ui(new Ui::MTHome)
{
    ui->setupUi(this);

    this->installEventFilter(this);
}

MTHome::~MTHome()
{
    delete m_pDetailPane;
    delete m_pDetailLayout;
    delete m_pHeaderLayout;
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
        ui->tableWidget->horizontalHeader()->resizeSection(0, 5);
        // -------------------------------------------
        ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
        ui->tableWidget->verticalHeader()->setDefaultSectionSize(60);
        ui->tableWidget->verticalHeader()->hide();
        ui->tableWidget->horizontalHeader()->hide();
        // -------------------------------------------
        ui->tableWidget->setContentsMargins(10,0,0,0);
        // -------------------------------------------
        m_pDetailPane = new MTHomeDetail;
        m_pDetailPane->SetHomePointer(*this);
        // -------------------------------------------
        m_pDetailLayout = new QVBoxLayout;
        m_pDetailLayout->addWidget(m_pDetailPane);

        m_pDetailPane  ->setContentsMargins(1,1,1,1);
        m_pDetailLayout->setContentsMargins(1,1,1,1);

        ui->widget->setContentsMargins(1,1,1,1);

        ui->widget->setLayout(m_pDetailLayout);

        // -------------------------------------------

        setupRecordList();

        // -------------------------------------------

        /** Flag Already Init **/
        already_init = true;
    }

    //Refresh visual data
    //Tell OT to repopulate, and refresh backend.
//    ((Moneychanger*)parentWidget())->get_ot_worker_background()->mc_overview_ping();

    RefreshUserBar();

    //Now refresh the repopulated data visually
    RefreshRecords();

    if (ui->tableWidget->rowCount() > 0)
    {
        ui->tableWidget->setCurrentCell(0, 1);
    }
    // -------------------------------------------
    show();
    setFocus();
}


void MTHome::on_tableWidget_currentCellChanged(int row, int column, int previousRow, int previousColumn)
{
    m_pDetailPane->refresh(row, m_list);

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
    m_list.AcceptTransfersAutomatically(false);
}





void MTHome::RefreshUserBar()
{
    // --------------------------------------------------
    if (NULL != m_pHeaderLayout)
    {
        MTHomeDetail::clearLayout(m_pHeaderLayout);
        delete m_pHeaderLayout;
        m_pHeaderLayout = NULL;
    }
    // --------------------------------------------------
    m_pHeaderLayout = new QGridLayout;
    m_pHeaderLayout->setAlignment(Qt::AlignTop);
    // --------------------------------------------------
    QWidget * pUserBar = this->CreateUserBarWidget();

    if (NULL != pUserBar)
    {
        m_pHeaderLayout->addWidget(pUserBar);
    }
    // --------------------------------------------------
    ui->headerFrame->setLayout(m_pHeaderLayout);
}


void MTHome::SetNeedRefresh()
{
    m_bNeedRefresh = true;

    RefreshUserBar();
}

QWidget * MTHome::CreateUserBarWidget()
{
    //Append to transactions list in overview dialog.
    QWidget * row_widget = new QWidget;
    QGridLayout * row_widget_layout = new QGridLayout;

    row_widget_layout->setSpacing(12);
    row_widget_layout->setContentsMargins(12, 3, 8, 3);

    row_widget->setLayout(row_widget_layout);
    row_widget->setStyleSheet("QWidget{background-color:#c0cad4;selection-background-color:#a0aac4;}");
    // -------------------------------------------
    //Render row.
    //Header of row
    QString tx_name = QString("My Acct        <small><font color=grey>(US Dollars)</font></small>");

    if(tx_name.trimmed() == "")
    {
        //Tx has no name
        tx_name.clear();
        tx_name = "Transaction";
    }

    QLabel * header_of_row = new QLabel;
    QString header_of_row_string = QString("");
    header_of_row_string.append(tx_name);

    header_of_row->setStyleSheet("QLabel { font-weight: bold; font-size:18pt; }");
    header_of_row->setText(header_of_row_string);

    //Append header to layout
    row_widget_layout->addWidget(header_of_row, 0, 0, 1,1, Qt::AlignLeft);
    // -------------------------------------------


    // ----------------------------------------------------------------
//    QIcon(":/icons/request");
//    QIcon(":/icons/user");
//    QIcon(":/icons/refresh");
    // ----------------------------------------------------------------
//    // Amount (with currency tla)
    // ----------------------------------------------------------------
    QLabel * currency_amount_label = new QLabel;
    QString currency_amount = QString("");
    // ----------------------------------------------------------------
//    long lAmount = OTAPI_Wrap::StringToLong(recordmt.GetAmount());

//    if (recordmt.IsOutgoing() || (lAmount < 0))
//        currency_amount_label->setStyleSheet("QLabel { color : red; }");
//    else
//        currency_amount_label->setStyleSheet("QLabel { color : green; }");
    // ----------------------------------------------------------------
    currency_amount_label->setStyleSheet("QLabel { color : grey; }");
    currency_amount_label->setText(currency_amount);
    // ----------------------------------------------------------------
    QToolButton *buttonSend     = new QToolButton;
    QToolButton *buttonRequest  = new QToolButton;
    QToolButton *buttonContacts = new QToolButton;
    QToolButton *buttonRefresh  = new QToolButton;
    // ----------------------------------------------------------------
    QPixmap pixmapSend    (":/icons/icons/send.png");
    QPixmap pixmapRequest (":/icons/icons/request.png");
    QPixmap pixmapContacts(":/icons/icons/user.png");
    QPixmap pixmapRefresh (":/icons/icons/refresh.png");
    // ----------------------------------------------------------------
    QIcon sendButtonIcon    (pixmapSend);
    QIcon requestButtonIcon (pixmapRequest);
    QIcon contactsButtonIcon(pixmapContacts);
    QIcon refreshButtonIcon (pixmapRefresh);
    // ----------------------------------------------------------------
    buttonSend->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    buttonSend->setAutoRaise(true);
    buttonSend->setIcon(sendButtonIcon);
    buttonSend->setIconSize(pixmapSend.rect().size());
    buttonSend->setText("Send");
    // ----------------------------------------------------------------
    buttonRequest->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    buttonRequest->setAutoRaise(true);
    buttonRequest->setIcon(requestButtonIcon);
    buttonRequest->setIconSize(pixmapRequest.rect().size());
    buttonRequest->setText("Request");
    // ----------------------------------------------------------------
    buttonContacts->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    buttonContacts->setAutoRaise(true);
    buttonContacts->setIcon(contactsButtonIcon);
    buttonContacts->setIconSize(pixmapContacts.rect().size());
    buttonContacts->setText("Contacts");
    // ----------------------------------------------------------------
    buttonRefresh->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    buttonRefresh->setAutoRaise(true);
    buttonRefresh->setIcon(refreshButtonIcon);
//    buttonRefresh->setIconSize(pixmapRefresh.rect().size());
    buttonRefresh->setIconSize(pixmapContacts.rect().size());
    buttonRefresh->setText("Refresh");

    if (m_bNeedRefresh)
        buttonRefresh->setStyleSheet("color: red");
    // ----------------------------------------------------------------
    QHBoxLayout * pButtonLayout = new QHBoxLayout;

    pButtonLayout->addWidget(currency_amount_label);
    pButtonLayout->addWidget(buttonSend);
    pButtonLayout->addWidget(buttonRequest);
    pButtonLayout->addWidget(buttonContacts);
    pButtonLayout->addWidget(buttonRefresh);
    // ----------------------------------------------------------------
    row_widget_layout->addLayout(pButtonLayout, 0, 1, 1,1, Qt::AlignRight);
//    row_widget_layout->addWidget(currency_amount_label, 0, 1, 1,1, Qt::AlignRight);
    // -------------------------------------------




    // -------------------------------------------
    //Sub-info
    QWidget * row_content_container = new QWidget;
    QGridLayout * row_content_grid = new QGridLayout;

    // left top right bottom

    row_content_grid->setSpacing(4);
    row_content_grid->setContentsMargins(3, 4, 3, 4);

    row_content_container->setLayout(row_content_grid);

    row_widget_layout->addWidget(row_content_container, 1,0, 1,2);
    // -------------------------------------------
    // Column one
    //Date (sub-info)
    //Calc/convert date/times
    QLabel * row_content_date_label = new QLabel("Available: ");
    QString row_content_date_label_string("<font color=grey>Available:</font> $50.93 (+ $167.23 in cash)");
//    row_content_date_label_string.append(QString(timestamp.toString(Qt::SystemLocaleShortDate)));

//    row_content_date_label->setStyleSheet("QLabel { color : grey; }");
    row_content_date_label->setText(row_content_date_label_string);

    row_content_grid->addWidget(row_content_date_label, 0,0, 1,1, Qt::AlignLeft);
    // -------------------------------------------
    // Column two
    //Status
//    QLabel * row_content_status_label = new QLabel;
//    QString row_content_status_string;
//
//    std::string formatDescription_holder("DESCRIPTION");
////    recordmt.FormatDescription(formatDescription_holder);
//
//    row_content_status_string.append(QString::fromStdString(formatDescription_holder));
//    // -------------------------------------------
//    //add string to label
//    row_content_status_label->setStyleSheet("QLabel { color : grey; }");
//    row_content_status_label->setText(row_content_status_string);

//    //add to row_content grid
//    row_content_grid->addWidget(row_content_status_label, 0,1, 1,1, Qt::AlignRight);
    // -------------------------------------------
    return row_widget;
}


void MTHome::RefreshRecords()
{
    //(Lock the overview dialog refreshing mechinism until finished)
    QMutexLocker overview_refresh_locker(&mc_overview_refreshing_visuals_mutex);
    // -------------------------------------------------------
    m_bNeedRefresh = false;
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




