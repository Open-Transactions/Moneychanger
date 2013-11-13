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

#include "overridecursor.h"

#include "Widgets/senddlg.h"
#include "Widgets/requestdlg.h"


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
        // ------------------------
        /** Flag Already Init **/
        already_init = true;
    }
    // -------------------------------------------
    //Refresh visual data
    //Tell OT to repopulate, and refresh backend.
//    ((Moneychanger*)parentWidget())->get_ot_worker_background()->mc_overview_ping();
    // -------------------------------------------
    RefreshAll();

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

void MTHome::RefreshAll()
{
    int nRowCount    = ui->tableWidget->rowCount();
    int nCurrentRow  = ui->tableWidget->currentRow();

//  bool bRefreshed = ;// PULL THE DATA FROM THE SERVER HERE.
    bool bRefreshed = true;

    if (bRefreshed)
    {
        // -----------------------------------------
        qDebug() << QString("Refreshing records from local storage.");
        // -----------------------------------------
        m_list.Populate();
        // -----------------------------------------
        RefreshUserBar();
        // -------------------------------------------
        RefreshRecords();
        // -----------------------------------------
        if (nCurrentRow >= 0)
        {
            if (nCurrentRow < ui->tableWidget->rowCount())
            {
                ui->tableWidget->setCurrentCell(nCurrentRow, 1);
                on_tableWidget_currentCellChanged(nCurrentRow, 1, 0, 0);
            }
            // ------------------------------------------------
            else if (ui->tableWidget->rowCount() > 0)
            {
                ui->tableWidget->setCurrentCell((ui->tableWidget->rowCount() - 1), 1);
                on_tableWidget_currentCellChanged((ui->tableWidget->rowCount() - 1), 1, 0, 0);
            }
            // ------------------------------------------------
            else
                qDebug() << QString("Apparently there are zero rows in the tableWidget.");
            // ------------------------------------------------
        }
        // ------------------------------------------------
        else if (ui->tableWidget->rowCount() > 0)
        {
            ui->tableWidget->setCurrentCell(0, 1);
            on_tableWidget_currentCellChanged(0, 1, 0, 0);
        }
        // -----------------------------------------
    }
    else
        qDebug() << QString("Failure removing MTRecord at index %1.").arg(nCurrentRow);
}


void MTHome::on_refreshButton_clicked()
{
    // ------------------------------------------------------
    {
        MTOverrideCursor theSpinner;
        // -----------------------------------------
        qDebug() << QString("Refreshing records from transaction servers.");
        // -----------------------------------------
//        ((Moneychanger *)(this->parentWidget()))->downloadAccountData();

        emit needToDownloadAccountData();
    }
    // ------------------------------------------------------
    RefreshAll();
}


void MTHome::on_contactsButton_clicked()
{
    Moneychanger * pMoneychanger = ((Moneychanger *)(this->parentWidget()));
    // --------------------------------------------------
    pMoneychanger->mc_addressbook_show(QString(""));
}


void MTHome::on_sendButton_clicked()
{
    Moneychanger * pMoneychanger = ((Moneychanger *)(this->parentWidget()));
    // --------------------------------------------------
    MTSendDlg * send_window = new MTSendDlg(NULL, *pMoneychanger);
    send_window->setAttribute(Qt::WA_DeleteOnClose);
    // --------------------------------------------------
    QString qstr_acct_id = pMoneychanger->get_default_account_id();

    if (!qstr_acct_id.isEmpty())
        send_window->setInitialMyAcct(qstr_acct_id);
    // ---------------------------------------
    send_window->dialog();
    send_window->show();
    // --------------------------------------------------
}

void MTHome::on_requestButton_clicked()
{
    Moneychanger * pMoneychanger = ((Moneychanger *)(this->parentWidget()));
    // --------------------------------------------------
    MTRequestDlg * request_window = new MTRequestDlg(NULL, *pMoneychanger);
    request_window->setAttribute(Qt::WA_DeleteOnClose);
    // --------------------------------------------------
    QString qstr_acct_id = pMoneychanger->get_default_account_id();

    if (!qstr_acct_id.isEmpty())
        request_window->setInitialMyAcct(qstr_acct_id);
    // ---------------------------------------
    request_window->dialog();
    request_window->show();
    // --------------------------------------------------
}

// ----------------------------------------------------------------------

//static
QString MTHome::cashBalance(QString qstr_server_id, QString qstr_asset_id, QString qstr_nym_id)
{
    int64_t     balance      = 0;
    QString     return_value = QString("");
    std::string str_output;

    balance    = MTHome::rawCashBalance(qstr_server_id, qstr_asset_id, qstr_nym_id);
    str_output = OTAPI_Wrap::It()->FormatAmount(qstr_asset_id.toStdString(), balance);

    if (!str_output.empty())
        return_value = QString::fromStdString(str_output);

    return return_value;
}

// ----------------------------------------------------------------------

//static
int64_t MTHome::rawCashBalance(QString qstr_server_id, QString qstr_asset_id, QString qstr_nym_id)
{
    int64_t balance = 0;

    std::string serverId(qstr_server_id.toStdString());
    std::string assetId (qstr_asset_id.toStdString());
    std::string nymId   (qstr_nym_id.toStdString());

    std::string str_purse = OTAPI_Wrap::LoadPurse(serverId, assetId, nymId);

    if (!str_purse.empty())
    {
        int64_t temp_balance = OTAPI_Wrap::Purse_GetTotalValue(serverId, assetId, str_purse);

        if (temp_balance >= 0)
            balance = temp_balance;
    }

    return balance;
}

// ----------------------------------------------------------------------

//static
QString MTHome::shortAcctBalance(QString qstr_acct_id, QString qstr_asset_id/*=QString("")*/)
{
    QString return_value("");
    // -------------------------------------------
    if (qstr_acct_id.isEmpty())
        return return_value; // Might want to assert here... (returns blank string.)
    // -------------------------------------------
    std::string  acctID     = qstr_acct_id.toStdString();
    int64_t      balance    = OTAPI_Wrap::GetAccountWallet_Balance(acctID);
    std::string  assetId;
    // -------------------------------------------
    if (!qstr_asset_id.isEmpty())
        assetId = qstr_asset_id.toStdString();
    else
        assetId = OTAPI_Wrap::GetAccountWallet_AssetTypeID(acctID);
    // -------------------------------------------
    std::string  str_output;

    if (!assetId.empty())
    {
        str_output = OTAPI_Wrap::It()->FormatAmount(assetId, balance);

        if (!str_output.empty())
            return_value = QString::fromStdString(str_output);
        else
        {
            std::string  str_asset_name = OTAPI_Wrap::It()->GetAssetType_Name(assetId);
            return_value = QString("%1 %2").arg(balance).arg(QString::fromStdString(str_asset_name));
        }
    }
    // -------------------------------------------
    return return_value;
}

// ----------------------------------------------------------------------

//static
int64_t MTHome::rawAcctBalance(QString qstrAcctId)
{
    int64_t ret = qstrAcctId.isEmpty() ? 0 : OTAPI_Wrap::GetAccountWallet_Balance(qstrAcctId.toStdString());
    return ret;
}

// ----------------------------------------------------------------------


QString MTHome::FormDisplayLabelForAcctButton(QString qstr_acct_id, QString qstr_display_name)
{
    QString display_name("");
    QString button_text("");
    // -----------------------------------------
    if (qstr_display_name.isEmpty())
        display_name = QString("");
    else
        display_name = qstr_display_name;
    // -----------------------------------------
    std::string str_acct_id     = qstr_acct_id.toStdString();
    std::string str_acct_nym    = OTAPI_Wrap::It()->GetAccountWallet_NymID      (str_acct_id);
    std::string str_acct_server = OTAPI_Wrap::It()->GetAccountWallet_ServerID   (str_acct_id);
    std::string str_acct_asset  = OTAPI_Wrap::It()->GetAccountWallet_AssetTypeID(str_acct_id);
    // -----------------------------------------
    QString qstr_acct_nym    = QString::fromStdString(str_acct_nym);
    QString qstr_acct_server = QString::fromStdString(str_acct_server);
    QString qstr_acct_asset  = QString::fromStdString(str_acct_asset);
    // -----------------------------------
    button_text = QString("%1 (%2").
            arg(display_name).
            arg(MTHome::shortAcctBalance(qstr_acct_id, qstr_acct_asset));
    // --------------------------------------------
    if (!qstr_acct_nym.isEmpty() && !qstr_acct_server.isEmpty() && !qstr_acct_asset.isEmpty())
    {
        int64_t  raw_cash_balance = MTHome::rawCashBalance(qstr_acct_server, qstr_acct_asset, qstr_acct_nym);

        if (raw_cash_balance > 0)
            button_text += QString(" + %1 %2").arg(MTHome::cashBalance(qstr_acct_server, qstr_acct_asset, qstr_acct_nym)).arg(tr("in cash"));
    }
    // --------------------------------------------
    button_text += QString( ")" );
    // -----------------------------------------
    return button_text;
}


// ----------------------------------------------------------------------

void MTHome::on_account_clicked()
{
    Moneychanger * pMoneychanger = (Moneychanger *)(this->parentWidget());
    // ----------------------------------------------
    pMoneychanger->mc_accountmanager_dialog();
}


QWidget * MTHome::CreateUserBarWidget()
{
    Moneychanger * pMoneychanger = (Moneychanger *)(this->parentWidget());
    // ----------------------------------------------
    //Append to transactions list in overview dialog.
    QWidget     * row_widget        = new QWidget;
    QGridLayout * row_widget_layout = new QGridLayout;

    row_widget_layout->setSpacing(12);
    row_widget_layout->setContentsMargins(12, 3, 8, 10); // left top right bottom
//  row_widget_layout->setContentsMargins(12, 3, 8, 3); // left top right bottom

    row_widget->setLayout(row_widget_layout);
    row_widget->setStyleSheet("QWidget{background-color:#c0cad4;selection-background-color:#a0aac4;}");
    // -------------------------------------------
    //Render row.
    //Header of row
    //
    QString qstr_acct_nym,
            qstr_acct_server,
            qstr_acct_asset, qstr_acct_asset_name("");
    // -------------------------------------------
    QString qstr_acct_name("");
    QString qstr_acct_id = pMoneychanger->get_default_account_id();
    // -------------------------------------------
    if (qstr_acct_id.isEmpty())
    {
        qstr_acct_name   = tr("(Default Account Isn't Set Yet)");
        // -----------------------------------
        qstr_acct_nym    = pMoneychanger->get_default_nym_id();
        qstr_acct_server = pMoneychanger->get_default_server_id();
        qstr_acct_asset  = pMoneychanger->get_default_asset_id();
    }
    else
    {
        std::string str_acct_id     = qstr_acct_id.toStdString();
        std::string str_acct_nym    = OTAPI_Wrap::It()->GetAccountWallet_NymID(str_acct_id);
        std::string str_acct_server = OTAPI_Wrap::It()->GetAccountWallet_ServerID(str_acct_id);
        std::string str_acct_asset  = OTAPI_Wrap::It()->GetAccountWallet_AssetTypeID(str_acct_id);
        // -----------------------------------
        qstr_acct_nym    = QString::fromStdString(str_acct_nym);
        qstr_acct_server = QString::fromStdString(str_acct_server);
        qstr_acct_asset  = QString::fromStdString(str_acct_asset);
        // -----------------------------------
        std::string str_acct_name  = OTAPI_Wrap::It()->GetAccountWallet_Name(str_acct_id);
        // -----------------------------------
        if (!str_acct_name.empty())
        {
            qstr_acct_name = QString::fromStdString(str_acct_name);
        }
        // -----------------------------------
        if (!str_acct_asset.empty())
        {
            std::string str_asset_name = OTAPI_Wrap::It()->GetAssetType_Name(str_acct_asset);
            qstr_acct_asset_name = QString("<small><font color=grey>(%1)</font></small>").arg(QString::fromStdString(str_asset_name));
        }
    }
    // -------------------------------------------
    QString tx_name;

    if (qstr_acct_name.trimmed() == "")
    {
        tx_name = tr("(Account Name is Blank)");
    }
    else
        tx_name = qstr_acct_name;
    // -------------------------------------------
    QString header_of_row_string = QString("");
    header_of_row_string.append(tx_name);
    // -------------------------------------------
    QToolButton * buttonAccount = new QToolButton;

    buttonAccount->setAutoRaise(true);
    buttonAccount->setStyleSheet("QToolButton { font-weight: bold; margin-left: 0; font-size:18pt; }");
    buttonAccount->setText(header_of_row_string);
    // -------------------------------------------
    connect(buttonAccount, SIGNAL(clicked()), this, SLOT(on_account_clicked()));
    // ----------------------------------------------------------------
    QLabel * asset_type = new QLabel;

    asset_type->setText(qstr_acct_asset_name);
    asset_type->setIndent(0);
    // ----------------------------------------------------------------
    QHBoxLayout * pAccountLayout = new QHBoxLayout;

//  pAccountLayout->setContentsMargins(10, 0, 0, 0);
//  pAccountLayout->setSpacing(0);

    pAccountLayout->addWidget(buttonAccount);
    pAccountLayout->addWidget(asset_type);
    // ----------------------------------------------------------------
    row_widget_layout->addLayout(pAccountLayout, 0, 0, 1,1, Qt::AlignLeft);
    // ----------------------------------------------------------------
//  QIcon(":/icons/request");
//  QIcon(":/icons/user");
//  QIcon(":/icons/refresh");
    // ----------------------------------------------------------------
    // Amount (with currency tla)
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
    buttonSend->setText(tr("Send"));
    // ----------------------------------------------------------------
    buttonRequest->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    buttonRequest->setAutoRaise(true);
    buttonRequest->setIcon(requestButtonIcon);
    buttonRequest->setIconSize(pixmapRequest.rect().size());
    buttonRequest->setText(tr("Request"));
    // ----------------------------------------------------------------
    buttonContacts->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    buttonContacts->setAutoRaise(true);
    buttonContacts->setIcon(contactsButtonIcon);
    buttonContacts->setIconSize(pixmapContacts.rect().size());
    buttonContacts->setText(tr("Contacts"));
    // ----------------------------------------------------------------
    buttonRefresh->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    buttonRefresh->setAutoRaise(true);
    buttonRefresh->setIcon(refreshButtonIcon);
//    buttonRefresh->setIconSize(pixmapRefresh.rect().size());
    buttonRefresh->setIconSize(pixmapContacts.rect().size());
    buttonRefresh->setText(tr("Refresh"));

    if (m_bNeedRefresh)
    {
        buttonRefresh->setStyleSheet("color: red");
        m_bNeedRefresh = false;
    }
    // ----------------------------------------------------------------
    QHBoxLayout * pButtonLayout = new QHBoxLayout;

    pButtonLayout->addWidget(currency_amount_label);
    pButtonLayout->addWidget(buttonSend);
    pButtonLayout->addWidget(buttonRequest);
    pButtonLayout->addWidget(buttonContacts);
    pButtonLayout->addWidget(buttonRefresh);
    // ----------------------------------------------------------------
    row_widget_layout->addLayout(pButtonLayout, 0, 1, 1,1, Qt::AlignRight);
//  row_widget_layout->addWidget(currency_amount_label, 0, 1, 1,1, Qt::AlignRight);
    // -------------------------------------------

    connect(buttonRefresh,  SIGNAL(clicked()),  this, SLOT(on_refreshButton_clicked()));
    connect(buttonContacts, SIGNAL(clicked()),  this, SLOT(on_contactsButton_clicked()));
    connect(buttonSend,     SIGNAL(clicked()),  this, SLOT(on_sendButton_clicked()));
    connect(buttonRequest,  SIGNAL(clicked()),  this, SLOT(on_requestButton_clicked()));

    // -------------------------------------------
    //Sub-info
    QWidget * row_content_container = new QWidget;
    QGridLayout * row_content_grid = new QGridLayout;


    row_content_container->setContentsMargins(0, 0, 0, 0);

    row_content_grid->setSpacing(4);
    row_content_grid->setContentsMargins(3, 0, 3, 0); // left top right bottom
//  row_content_grid->setContentsMargins(3, 4, 3, 4); // left top right bottom

    row_content_container->setLayout(row_content_grid);

    row_widget_layout->addWidget(row_content_container, 1,0, 1,2);
    // -------------------------------------------
    // Column one
    //Date (sub-info)
    //Calc/convert date/times
    //
    QLabel * pBalanceLabel = new QLabel(QString("<font color=grey>%1</font> %2").arg(tr("Available:")).arg(tr("no account selected")));
//  QString  balance_label_string("<font color=grey>Available:</font> $50.93 (+ $167.23 in cash)");
    // ---------------------------------------------
    QString  balance_label_string = QString("");

    if (!qstr_acct_id.isEmpty())
    {
        balance_label_string = QString("<font color=grey>%1</font> %2").arg(tr("Available:")).arg(MTHome::shortAcctBalance(qstr_acct_id, qstr_acct_asset));
    }
    // --------------------------------------------
    if (!qstr_acct_nym.isEmpty() && !qstr_acct_server.isEmpty() && !qstr_acct_asset.isEmpty())
    {
        int64_t  raw_cash_balance = MTHome::rawCashBalance(qstr_acct_server, qstr_acct_asset, qstr_acct_nym);

        if (raw_cash_balance > 0)
            balance_label_string += QString( " (+ %1 %2)" ).arg(MTHome::cashBalance(qstr_acct_server, qstr_acct_asset, qstr_acct_nym)).arg(tr("in cash"));
    }
    // ---------------------------------------------------------------
    pBalanceLabel->setText(balance_label_string);
    // ---------------------------------------------------------------
    QHBoxLayout * pBalanceLayout = new QHBoxLayout;

    pBalanceLayout->addSpacing(8);
    pBalanceLayout->addWidget(pBalanceLabel);
    // ---------------------------------------------------------------
    row_content_grid->addLayout(pBalanceLayout, 0,0, 1,1, Qt::AlignLeft);
//  row_content_grid->addWidget(pBalanceLabel,  0,0, 1,1, Qt::AlignLeft);
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

void MTHome::OnDeletedRecord()
{
    int nRowCount    = ui->tableWidget->rowCount();
    int nCurrentRow  = ui->tableWidget->currentRow();

    if ((nRowCount > 0) && (nCurrentRow >= 0))
    {
        bool bRemoved = m_list.RemoveRecord(nCurrentRow);

        if (bRemoved)
        {
            qDebug() << QString("Removed record at index %1.").arg(nCurrentRow);
            // -----------------------------------------
            // We do this because the individual records keep track of their index inside their box.
            // Once a record is deleted, all the others now have bad indices, and must be reloaded.
            //
            m_list.Populate();

            RefreshRecords();
            // -----------------------------------------
            if ((nCurrentRow >= 0) && (nCurrentRow < ui->tableWidget->rowCount()))
            {
                ui->tableWidget->setCurrentCell(nCurrentRow, 1);
            }
            else if (ui->tableWidget->rowCount() > 0)
            {
                ui->tableWidget->setCurrentCell((ui->tableWidget->rowCount() - 1), 1);
            }
            else
                qDebug() << QString("Apparently there are zero rows in the tableWidget.");
            // -----------------------------------------
        }
        else
            qDebug() << QString("Failure removing MTRecord at index %1.").arg(nCurrentRow);
    }
}

void MTHome::RefreshRecords()
{
    //(Lock the overview dialog refreshing mechinism until finished)
    QMutexLocker overview_refresh_locker(&mc_overview_refreshing_visuals_mutex);
    // -------------------------------------------------------
    m_bNeedRefresh = false;
    // -------------------------------------------------------
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




