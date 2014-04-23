#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/home.hpp>
#include <ui_home.h>

#include <gui/widgets/homedetail.hpp>
#include <gui/widgets/overridecursor.hpp>

#include <core/moneychanger.hpp>
#include <core/handlers/contacthandler.hpp>

#include <opentxs/OTAPI.hpp>
#include <opentxs/OTAPI_Exec.hpp>
#include <opentxs/OTLog.hpp>

#include <QLabel>
#include <QDebug>
#include <QToolButton>
#include <QKeyEvent>



MTHome::MTHome(QWidget *parent) :
    QWidget(parent, Qt::Window),
    already_init(false),
    m_list(*(new MTNameLookupQT)),
    m_bTurnRefreshBtnRed(false),
    ui(new Ui::MTHome)
{
    ui->setupUi(this);

    this->installEventFilter(this);
}

MTHome::~MTHome()
{
    delete ui;
}

bool MTHome::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Escape)
        {
            close(); // This is caught by this same filter.
            return true;
        }
    }

    // standard event processing
    return QWidget::eventFilter(obj, event);
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
        QPointer<MTHomeDetail> pDetailPane   = new MTHomeDetail;
        QPointer<QVBoxLayout>  pDetailLayout = new QVBoxLayout;
        // -------------------------------------------
        pDetailPane  ->setContentsMargins(1,1,1,1);
        pDetailLayout->setContentsMargins(1,1,1,1);
        // -------------------------------------------
        pDetailPane->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        // -------------------------------------------
        pDetailLayout->addWidget(pDetailPane); // MTHomeDetail
        // -------------------------------------------
        ui->widget->setLayout(pDetailLayout);
        // -------------------------------------------
        m_pDetailPane = pDetailPane;
        m_pDetailPane->SetHomePointer(*this);
        // ----------------------------------
        // Note: This is a placekeeper, so later on I can just erase
        // the widget at 0 and replace it with the real header widget.
        //
        m_pHeaderFrame  = new QFrame;
        ui->verticalLayout->insertWidget(0, m_pHeaderFrame);
        // ----------------------------------
        setupRecordList();
        // ------------------------
        /** Flag Already Init **/
        already_init = true;
    }
    // -------------------------------------------
    RefreshAll();

    show();
    setFocus();
}


void MTHome::on_tableWidget_currentCellChanged(int row, int column, int previousRow, int previousColumn)
{
    Q_UNUSED(column);
    Q_UNUSED(previousRow);
    Q_UNUSED(previousColumn);

    if (m_pDetailPane)
    {
        if ((-1) == row)
            m_pDetailPane->setVisible(false);
        else
            m_pDetailPane->setVisible(true);

        emit needToRefreshDetails(row, m_list);
    }
}

void MTHome::setupRecordList()
{
    int nServerCount  = OTAPI_Wrap::It()->GetServerCount();
    int nAssetCount   = OTAPI_Wrap::It()->GetAssetTypeCount();
    int nNymCount     = OTAPI_Wrap::It()->GetNymCount();
    int nAccountCount = OTAPI_Wrap::It()->GetAccountCount();
    // ----------------------------------------------------
    m_list.ClearServers();
    m_list.ClearAssets();
    m_list.ClearNyms();
    m_list.ClearAccounts();
    // ----------------------------------------------------
    for (int ii = 0; ii < nServerCount; ++ii)
    {
        std::string serverId = OTAPI_Wrap::It()->GetServer_ID(ii);
        m_list.AddServerID(serverId);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nAssetCount; ++ii)
    {
        std::string assetId = OTAPI_Wrap::It()->GetAssetType_ID(ii);
        m_list.AddAssetID(assetId);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nNymCount; ++ii)
    {
        std::string nymId = OTAPI_Wrap::It()->GetNym_ID(ii);
        m_list.AddNymID(nymId);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nAccountCount; ++ii)
    {
        std::string accountID = OTAPI_Wrap::It()->GetAccountWallet_ID(ii);
        m_list.AddAccountID(accountID);
    }
    // ----------------------------------------------------
    m_list.AcceptChequesAutomatically  (true);
    m_list.AcceptReceiptsAutomatically (true);
    m_list.AcceptTransfersAutomatically(false);
}





void MTHome::RefreshUserBar()
{   
    if (m_pHeaderFrame)
    {
        ui->verticalLayout->removeWidget(m_pHeaderFrame);

        m_pHeaderFrame->setParent(NULL);
        m_pHeaderFrame->disconnect();
        m_pHeaderFrame->deleteLater();

        m_pHeaderFrame = NULL;
    }
    // -------------------------------------------------
    QPointer<QWidget> pHeaderWidget = this->CreateUserBarWidget();

    if (pHeaderWidget)
    {
        // Note: Frame Shape should be Styled Panel, and Frame Shadow should be Raised.
        // Frame line width should be 1. Mid line width 0. LayoutDirection on frame should
        // be leftToRight. Font kerning should be on. Horizontal and Vertical policies
        // should be Preferred.
        QPointer<QFrame> pHeaderFrame  = new QFrame;
        // -----------------------------------------
        pHeaderFrame->setFrameShape (QFrame::StyledPanel);
        pHeaderFrame->setFrameShadow(QFrame::Raised);
        pHeaderFrame->setLineWidth(1);
        pHeaderFrame->setMidLineWidth(0);
        // -----------------------------------------
        QPointer<QGridLayout> pHeaderLayout = new QGridLayout;
        pHeaderLayout->setAlignment(Qt::AlignTop);
        // -----------------------------------------
        pHeaderLayout->addWidget(pHeaderWidget);
        // -----------------------------------------
        pHeaderFrame->setLayout(pHeaderLayout);
        // -----------------------------------------
        ui->verticalLayout->insertWidget(0, pHeaderFrame);
        // -----------------------------------------
        m_pHeaderFrame = pHeaderFrame;
    }
    // --------------------------------------------------
}

void MTHome::onNeedToRefreshRecords()
{
    RefreshAll();
}

void MTHome::onAccountDataDownloaded()
{
    RefreshAll();
}

void MTHome::onSetRefreshBtnRed()
{
    SetRefreshBtnRed();
}

// This means "refresh the user bar to have a RED Refresh button on it."
// m_bTurnRefreshBtnRed being set to true is what makes the button red once it's refreshed.
// (As as soon as the button is turned red, m_bTurnRefreshBtnRed is set back to false so it
// will be black again the next time around.)
//
void MTHome::SetRefreshBtnRed()
{
    m_bTurnRefreshBtnRed = true;

    RefreshUserBar();
}


void MTHome::onRecordDeleted(bool bNeedToRefreshUserBar)
{
    OnDeletedRecord(); // This does a "RefreshRecords" internally, but doesn't refresh the user bar.

    if (bNeedToRefreshUserBar)
        RefreshUserBar();
}

// The balances hasn't necessarily changed.
// (Perhaps the default account was changed.)
//
void MTHome::onNeedToRefreshUserBar()
{
    RefreshUserBar();
}

void MTHome::onBalancesChanged()
{
    RefreshUserBar();
}

void MTHome::RefreshAll()
{
//  int nRowCount    = ui->tableWidget->rowCount();
    int nCurrentRow  = ui->tableWidget->currentRow();
    // -----------------------------------------
    m_list.Populate(); // Refreshes the data from local storage.
    // -----------------------------------------
    RefreshUserBar();
    // -------------------------------------------
    RefreshRecords(); // Refreshes the list of records on the left-hand side, from the data.
    // -----------------------------------------
    if (nCurrentRow >= 0)
    {
        if (nCurrentRow < ui->tableWidget->rowCount())
        {
            ui->tableWidget->blockSignals(true);
            ui->tableWidget->setCurrentCell(nCurrentRow, 1);
            ui->tableWidget->blockSignals(false);
            on_tableWidget_currentCellChanged(nCurrentRow, 1, 0, 0);
        }
        // ------------------------------------------------
        else if (ui->tableWidget->rowCount() > 0)
        {
            ui->tableWidget->blockSignals(true);
            ui->tableWidget->setCurrentCell((ui->tableWidget->rowCount() - 1), 1);
            ui->tableWidget->blockSignals(false);
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
        ui->tableWidget->blockSignals(true);
        ui->tableWidget->setCurrentCell(0, 1);
        ui->tableWidget->blockSignals(false);
        on_tableWidget_currentCellChanged(0, 1, 0, 0);
    }
    // -----------------------------------------
    else
    {
        ui->tableWidget->blockSignals(true);
        ui->tableWidget->setCurrentCell(-1, 1);
        ui->tableWidget->blockSignals(false);
        on_tableWidget_currentCellChanged(-1, 1, 0, 0);
    }
}


void MTHome::on_refreshButton_clicked()
{
    MTSpinner theSpinner;
    // -----------------------------------------
    qDebug() << QString("Refreshing records from transaction servers.");
    // -----------------------------------------
    emit needToDownloadAccountData();
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

    std::string str_purse = OTAPI_Wrap::It()->LoadPurse(serverId, assetId, nymId);

    if (!str_purse.empty())
    {
        int64_t temp_balance = OTAPI_Wrap::It()->Purse_GetTotalValue(serverId, assetId, str_purse);

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
    int64_t      balance    = OTAPI_Wrap::It()->GetAccountWallet_Balance(acctID);
    std::string  assetId;
    // -------------------------------------------
    if (!qstr_asset_id.isEmpty())
        assetId = qstr_asset_id.toStdString();
    else
        assetId = OTAPI_Wrap::It()->GetAccountWallet_AssetTypeID(acctID);
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
    int64_t ret = qstrAcctId.isEmpty() ? 0 : OTAPI_Wrap::It()->GetAccountWallet_Balance(qstrAcctId.toStdString());
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


QWidget * MTHome::CreateUserBarWidget()
{
    QWidget     * pUserBarWidget        = new QWidget;
    QGridLayout * pUserBarWidget_layout = new QGridLayout;

    pUserBarWidget_layout->setSpacing(12);
    pUserBarWidget_layout->setContentsMargins(12, 3, 8, 10); // left top right bottom
//  pUserBarWidget_layout->setContentsMargins(12, 3, 8, 3); // left top right bottom

    pUserBarWidget->setLayout(pUserBarWidget_layout);
    pUserBarWidget->setStyleSheet("QWidget{background-color:#c0cad4;selection-background-color:#a0aac4;}");
    // -------------------------------------------
    QString qstr_acct_nym,
            qstr_acct_server,
            qstr_acct_asset, qstr_acct_asset_name("");
    // -------------------------------------------
    QString qstr_acct_name("");
    QString qstr_acct_id = Moneychanger::It()->get_default_account_id();
    // -------------------------------------------
    if (qstr_acct_id.isEmpty())
    {
        qstr_acct_name   = tr("(Default Account Isn't Set Yet)");
        // -----------------------------------
        qstr_acct_nym    = Moneychanger::It()->get_default_nym_id();
        qstr_acct_server = Moneychanger::It()->get_default_server_id();
        qstr_acct_asset  = Moneychanger::It()->get_default_asset_id();
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
    connect(buttonAccount, SIGNAL(clicked()), Moneychanger::It(), SLOT(mc_show_account_manager_slot()));
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
    pUserBarWidget_layout->addLayout(pAccountLayout, 0, 0, 1,1, Qt::AlignLeft);
    // ----------------------------------------------------------------
    QLabel * currency_amount_label = new QLabel;
    QString currency_amount = QString("");
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

    if (m_bTurnRefreshBtnRed)
    {
        buttonRefresh->setStyleSheet("color: red");
        m_bTurnRefreshBtnRed = false;
    }
    // ----------------------------------------------------------------
    QHBoxLayout * pButtonLayout = new QHBoxLayout;

    pButtonLayout->addWidget(currency_amount_label);
    pButtonLayout->addWidget(buttonSend);
    pButtonLayout->addWidget(buttonRequest);
    pButtonLayout->addWidget(buttonContacts);
    pButtonLayout->addWidget(buttonRefresh);
    // ----------------------------------------------------------------
    pUserBarWidget_layout->addLayout(pButtonLayout, 0, 1, 1,1, Qt::AlignRight);
//  pUserBarWidget_layout->addWidget(currency_amount_label, 0, 1, 1,1, Qt::AlignRight);
    // -------------------------------------------
    connect(buttonRefresh,  SIGNAL(clicked()),  this, SLOT(on_refreshButton_clicked()));
    connect(buttonContacts, SIGNAL(clicked()),  Moneychanger::It(), SLOT(mc_addressbook_slot()));
    connect(buttonSend,     SIGNAL(clicked()),  Moneychanger::It(), SLOT(mc_sendfunds_slot()));
    connect(buttonRequest,  SIGNAL(clicked()),  Moneychanger::It(), SLOT(mc_requestfunds_slot()));
    // -------------------------------------------
    //Sub-info
    QWidget * row_content_container = new QWidget;
    QGridLayout * row_content_grid = new QGridLayout;

    row_content_container->setContentsMargins(0, 0, 0, 0);

    row_content_grid->setSpacing(4);
    row_content_grid->setContentsMargins(3, 0, 3, 0); // left top right bottom
//  row_content_grid->setContentsMargins(3, 4, 3, 4); // left top right bottom

    row_content_container->setLayout(row_content_grid);

    pUserBarWidget_layout->addWidget(row_content_container, 1,0, 1,2);
    // -------------------------------------------
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
    // -------------------------------------------
    return pUserBarWidget;
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
            {
                m_pDetailPane->setVisible(false);
                ui->tableWidget->setCurrentCell(-1, 1);
            }
            // -----------------------------------------
        }
        else
            qDebug() << QString("Failure removing MTRecord at index %1.").arg(nCurrentRow);
    }
}

void MTHome::RefreshRecords()
{
    //(Lock the overview dialog refreshing mechinism until finished)
//    QMutexLocker overview_refresh_locker(&mc_overview_refreshing_visuals_mutex);
    // -------------------------------------------------------
    m_bTurnRefreshBtnRed = false;
    // -------------------------------------------------------
    int listSize       = m_list.size();
    // -------------------------------------------------------
    int nTotalRecords  = listSize;
    // -------------------------------------------------------
    ui->tableWidget->blockSignals(true);
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
    ui->tableWidget->blockSignals(false);
}




