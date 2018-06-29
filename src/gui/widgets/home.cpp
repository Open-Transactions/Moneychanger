#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/home.hpp>
#include <ui_home.h>

#include <gui/widgets/homedetail.hpp>
#include <gui/widgets/overridecursor.hpp>

#include <core/moneychanger.hpp>
#include <core/handlers/DBHandler.hpp>
#include <core/handlers/contacthandler.hpp>
#include <core/handlers/modelmessages.hpp>
#include <core/handlers/modelpayments.hpp>

#include <core/mtcomms.h>
#include <core/network/Network.h>
#include <core/handlers/focuser.h>

#include <opentxs/opentxs.hpp>

#include <QLabel>
#include <QDebug>
#include <QToolButton>
#include <QKeyEvent>
#include <QSqlTableModel>

#include <QMessageBox>



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
//        Moneychanger::It()->setupRecordList();
        // ------------------------
        /** Flag Already Init **/
        already_init = true;
    }
    // -------------------------------------------
    Focuser f(this);
    f.show();
    f.focus();
    // -------------------------------------------
    emit needToPopulateRecordlist();
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

        emit needToRefreshDetails(row, GetRecordlist());
    }
}


//opentxs::OTRecordList & MTHome::GetRecordlist()
//{
//    return Moneychanger::It()->GetRecordlist();
//}

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

void MTHome::onRecordlistPopulated()
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


void MTHome::onRecordDeleted()
{
    OnDeletedRecord();
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
    RefreshUserBar();
    // -------------------------------------------
    RefreshRecords(); // Refreshes the list of records on the left-hand side in the GUI, from the data.
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
//    MTSpinner theSpinner;  // I think the Moneychanger function already does this.
//    qDebug() << QString("Refreshing records from transaction servers.");
    emit needToDownloadAccountData();
}


// ----------------------------------------------------------------------

//static
QString MTHome::cashBalance(QString qstr_notary_id, QString qstr_asset_id, QString qstr_nym_id)
{
    int64_t     balance      = 0;
    QString     return_value = QString("");
    std::string str_output;

    balance    = MTHome::rawCashBalance(qstr_notary_id, qstr_asset_id, qstr_nym_id);
    str_output = opentxs::OT::App().API().Exec().FormatAmount(qstr_asset_id.toStdString(), balance);

    if (!str_output.empty())
        return_value = QString::fromStdString(str_output);

    return return_value;
}

// ----------------------------------------------------------------------

//static
int64_t MTHome::rawCashBalance(QString qstr_notary_id, QString qstr_asset_id, QString qstr_nym_id)
{
    int64_t balance = 0;

    std::string NotaryID(qstr_notary_id.toStdString());
    std::string InstrumentDefinitionID (qstr_asset_id.toStdString());
    std::string nymId   (qstr_nym_id.toStdString());

#if OT_CASH
    std::string str_purse = opentxs::OT::App().API().Exec().LoadPurse(NotaryID, InstrumentDefinitionID, nymId);

    if (!str_purse.empty())
    {
        int64_t temp_balance = opentxs::OT::App().API().Exec().Purse_GetTotalValue(NotaryID, InstrumentDefinitionID, str_purse);

        if (temp_balance >= 0)
            balance = temp_balance;
    }
#endif  // OT_CASH

    return balance;
}

// ----------------------------------------------------------------------

//static
QString MTHome::shortAcctBalance(QString qstr_acct_id, QString qstr_asset_id/*=QString("")*/, bool bWithSymbol/*=true*/)
{
    QString return_value("");
    // -------------------------------------------
    if (qstr_acct_id.isEmpty())
        return return_value; // Might want to assert here... (returns blank string.)
    // -------------------------------------------
    std::string  acctID     = qstr_acct_id.toStdString();
    int64_t      balance    = opentxs::OT::App().API().Exec().GetAccountWallet_Balance(acctID);
    std::string  InstrumentDefinitionID;
    // -------------------------------------------
    if (!qstr_asset_id.isEmpty())
        InstrumentDefinitionID = qstr_asset_id.toStdString();
    else
        InstrumentDefinitionID = opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID(acctID);
    // -------------------------------------------
    std::string  str_output;

    if (!InstrumentDefinitionID.empty())
    {
        str_output = bWithSymbol ?
                     opentxs::OT::App().API().Exec().FormatAmount(InstrumentDefinitionID, balance) :
                     opentxs::OT::App().API().Exec().FormatAmountWithoutSymbol(InstrumentDefinitionID, balance);

        if (!str_output.empty())
            return_value = QString::fromStdString(str_output);
        else
        {
            std::string  str_asset_name = opentxs::OT::App().API().Exec().GetAssetType_Name(InstrumentDefinitionID);
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
    int64_t ret = qstrAcctId.isEmpty() ? 0 : opentxs::OT::App().API().Exec().GetAccountWallet_Balance(qstrAcctId.toStdString());
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
    std::string str_acct_nym    = opentxs::OT::App().API().Exec().GetAccountWallet_NymID      (str_acct_id);
    std::string str_acct_server = opentxs::OT::App().API().Exec().GetAccountWallet_NotaryID   (str_acct_id);
    std::string str_acct_asset  = opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID(str_acct_id);
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

    pUserBarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    pUserBarWidget->setLayout(pUserBarWidget_layout);
    pUserBarWidget->setStyleSheet("QWidget{background-color:#c0cad4;selection-background-color:#a0aac4;}");
    // -------------------------------------------
    QString qstr_acct_nym,
            qstr_acct_server,
            qstr_acct_asset, qstr_acct_asset_name("");
    // -------------------------------------------
    QString qstr_acct_name("");
    QString qstr_balance(""), qstr_tla("");
    QString qstr_acct_id = Moneychanger::It()->get_default_account_id();
    // -------------------------------------------
    if (qstr_acct_id.isEmpty())
    {
        qstr_balance     = tr("0.00");
//      qstr_balance     = tr("(Click to Set Default Account)");
        qstr_acct_name = QString("");
        // -----------------------------------
        qstr_acct_nym    = Moneychanger::It()->get_default_nym_id();
        qstr_acct_server = Moneychanger::It()->get_default_notary_id();
        qstr_acct_asset  = Moneychanger::It()->get_default_asset_id();
    }
    else
    {
        // -----------------------------------
        std::string str_acct_id     = qstr_acct_id.toStdString();
        std::string str_acct_nym    = opentxs::OT::App().API().Exec().GetAccountWallet_NymID(str_acct_id);
        std::string str_acct_server = opentxs::OT::App().API().Exec().GetAccountWallet_NotaryID(str_acct_id);
        std::string str_acct_asset  = opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID(str_acct_id);
        // -----------------------------------
        qstr_acct_nym    = QString::fromStdString(str_acct_nym);
        qstr_acct_server = QString::fromStdString(str_acct_server);
        qstr_acct_asset  = QString::fromStdString(str_acct_asset);
        // -----------------------------------
        std::string str_tla = opentxs::OT::App().API().Exec().GetCurrencyTLA(str_acct_asset);
        qstr_tla = QString("<font color=grey>%1</font>").arg(QString::fromStdString(str_tla));

        qstr_balance = MTHome::shortAcctBalance(qstr_acct_id, qstr_acct_asset, false);
//      qstr_balance = QString("<font color=grey>%1</font> %2").arg(QString::fromStdString(str_tla)).arg(qstrTempBalance);
        // -----------------------------------
        std::string str_acct_name  = opentxs::OT::App().API().Exec().GetAccountWallet_Name(str_acct_id);
        // -----------------------------------
        if (!str_acct_asset.empty())
        {
            std::string str_asset_name = opentxs::OT::App().API().Exec().GetAssetType_Name(str_acct_asset);
            qstr_acct_asset_name = QString::fromStdString(str_asset_name);
        }
        // -----------------------------------
        if (!str_acct_name.empty())
        {
            qstr_acct_name = QString("%1").arg(QString::fromStdString(str_acct_name));
//          qstr_acct_name = QString("%1 <font color=grey>(%2)</font>").arg(QString::fromStdString(str_acct_name)).arg(qstr_acct_asset_name);
//          qstr_acct_name = QString("<small>%1 <font color=grey>(%2)</font></small>").arg(QString::fromStdString(str_acct_name)).arg(qstr_acct_asset_name);
        }
    }
    // ---------------------------------------------
    QToolButton * buttonAccount = new QToolButton;

    buttonAccount->setAutoRaise(true);
    buttonAccount->setStyleSheet("QToolButton { margin-left: 0; font-size:30pt;  font-weight:lighter; }");
//  buttonAccount->setStyleSheet("QToolButton { font-weight: bold; margin-left: 0; font-size:22pt; }");

    QLabel * tla_label = new QLabel(qstr_tla);
    tla_label->setAlignment(Qt::AlignRight|Qt::AlignBottom);
    tla_label->setStyleSheet("QLabel { margin-right: 0; font-size:20pt;  font-weight:lighter; }");

    buttonAccount->setText(qstr_balance);
    // -------------------------------------------
    connect(buttonAccount, SIGNAL(clicked()), Moneychanger::It(), SLOT(mc_show_account_manager_slot()));
    // ----------------------------------------------------------------
    QString  cash_label_string = QString("");
    QString  qstrCash = qstr_acct_name;

    if (!qstr_acct_nym.isEmpty() && !qstr_acct_server.isEmpty() && !qstr_acct_asset.isEmpty())
    {
        int64_t  raw_cash_balance = MTHome::rawCashBalance(qstr_acct_server, qstr_acct_asset, qstr_acct_nym);

        if (raw_cash_balance > 0)
        {
            cash_label_string = MTHome::cashBalance(qstr_acct_server, qstr_acct_asset, qstr_acct_nym);
            qstrCash += QString(" <small><font color=grey>(%2 %3 %4)</font></small>").arg(tr("plus")).arg(cash_label_string).arg(tr("in cash"));
        }
    }
    else
        qstrCash = tr("");
//      qstrCash = tr("(no account selected)");
    // -------------------------------------------
    QLabel * pCashLabel = new QLabel(qstrCash);
    // ---------------------------------------------------------------
    //pCashLabel->setText(qstrCash);
    pCashLabel->setIndent(13);
    // ---------------------------------------------------------------
    QWidget * row_balance_container = new QWidget;
    QHBoxLayout * row_balance_layout = new QHBoxLayout;

    tla_label->setContentsMargins(12, 0, 0, 5);
    tla_label->setAlignment(Qt::AlignRight|Qt::AlignBottom);
    row_balance_layout->setSpacing(0);
    row_balance_layout->addWidget(tla_label);
    row_balance_layout->addWidget(buttonAccount);

    row_balance_layout->setContentsMargins(0, 20, 0, 0);
    row_balance_container->setContentsMargins(0, 0, 0, 0);

    row_balance_container->setLayout(row_balance_layout);
    // ----------------------------------------------------------------
    QVBoxLayout * pAccountLayout = new QVBoxLayout;

    pAccountLayout->setMargin(0);
    //pAccountLayout->setContentsMargins(0, 0, 0, 0);
    pAccountLayout->setSpacing(3);

    pAccountLayout->addWidget(row_balance_container);
    pAccountLayout->addWidget(pCashLabel);
    pAccountLayout->addStretch();
    // ----------------------------------------------------------------
    pUserBarWidget_layout->addLayout(pAccountLayout, 0, 0, 1,1, Qt::AlignLeft);
    // ----------------------------------------------------------------



    QLabel * currency_amount_label = new QLabel;
    QString currency_amount = QString("");
    // ----------------------------------------------------------------
    currency_amount_label->setStyleSheet("QLabel { color : grey; }");
    currency_amount_label->setText(currency_amount);
    // ----------------------------------------------------------------
    QToolButton *buttonSend       = new QToolButton;
//  QToolButton *buttonRequest    = new QToolButton;
    QToolButton *buttonSecrets    = new QToolButton;
    QToolButton *buttonCompose    = new QToolButton;
    QToolButton *buttonExchange   = new QToolButton;
    QToolButton *buttonRefresh    = new QToolButton;
    // ----------------------------------------------------------------
    QPixmap pixmapSend      (":/icons/icons/fistful_of_cash_72.png");
//  QPixmap pixmapSend      (":/icons/icons/money_fist4_small.png");
//  QPixmap pixmapSend      (":/icons/icons/send.png");
//  QPixmap pixmapRequest   (":/icons/icons/request.png");
    QPixmap pixmapCompose   (":/icons/icons/pencil.png");
//  QPixmap pixmapCompose   (":/icons/icons/compose.png");
    QPixmap pixmapExchange  (":/icons/markets");
//  QPixmap pixmapIdentities(":/icons/icons/user.png");
//  QPixmap pixmapContacts  (":/icons/icons/rolodex_small");
    QPixmap pixmapSecrets   (":/icons/icons/vault.png");
//  QPixmap pixmapSecrets   (":/icons/icons/safe.png");

    QPixmap pixmapRefresh   (":/icons/icons/refresh.png");
    // ----------------------------------------------------------------
    QIcon sendButtonIcon      (pixmapSend);
//  QIcon requestButtonIcon   (pixmapRequest);
    QIcon secretsButtonIcon   (pixmapSecrets);
    QIcon composeButtonIcon   (pixmapCompose);
    QIcon exchangeButtonIcon  (pixmapExchange);
    QIcon refreshButtonIcon   (pixmapRefresh);
    // ----------------------------------------------------------------
    buttonSend->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    buttonSend->setAutoRaise(true);
    buttonSend->setIcon(sendButtonIcon);
    buttonSend->setIconSize(pixmapSend.rect().size());
    buttonSend->setText(tr("Payments"));
    // ----------------------------------------------------------------
//    buttonRequest->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
//    buttonRequest->setAutoRaise(true);
//    buttonRequest->setIcon(requestButtonIcon);
//    buttonRequest->setIconSize(pixmapRequest.rect().size());
//    buttonRequest->setText(tr("Request"));
    // ----------------------------------------------------------------
    buttonCompose->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    buttonCompose->setAutoRaise(true);
    buttonCompose->setIcon(composeButtonIcon);
    buttonCompose->setIconSize(pixmapCompose.rect().size());
    buttonCompose->setText(tr("Messages"));
    // ----------------------------------------------------------------
    buttonExchange->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    buttonExchange->setAutoRaise(true);
    buttonExchange->setIcon(exchangeButtonIcon);
    buttonExchange->setIconSize(pixmapExchange.rect().size());
    buttonExchange->setText(tr("Exchange"));
    // ----------------------------------------------------------------
    buttonSecrets->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    buttonSecrets->setAutoRaise(true);
    buttonSecrets->setIcon(secretsButtonIcon);
    buttonSecrets->setIconSize(pixmapRefresh.rect().size());
    buttonSecrets->setText(tr("Secrets"));
    // ----------------------------------------------------------------
    buttonRefresh->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    buttonRefresh->setAutoRaise(true);
    buttonRefresh->setIcon(refreshButtonIcon);
    buttonRefresh->setIconSize(pixmapExchange.rect().size());
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
//  pButtonLayout->addWidget(buttonRequest);
    pButtonLayout->addWidget(buttonCompose);
    pButtonLayout->addWidget(buttonExchange);
    pButtonLayout->addWidget(buttonSecrets);
    pButtonLayout->addWidget(buttonRefresh);
    // ----------------------------------------------------------------
    pUserBarWidget_layout->addLayout(pButtonLayout, 0, 1, 1,1, Qt::AlignRight);
//  pUserBarWidget_layout->addWidget(currency_amount_label, 0, 1, 1,1, Qt::AlignRight);
    // -------------------------------------------
//  connect(buttonRequest,    SIGNAL(clicked()),  Moneychanger::It(), SLOT(mc_requestfunds_slot()));
    connect(buttonSend,       SIGNAL(clicked()),  Moneychanger::It(), SLOT(mc_payments_slot()));
    connect(buttonCompose,    SIGNAL(clicked()),  Moneychanger::It(), SLOT(mc_messages_slot()));
    connect(buttonExchange,   SIGNAL(clicked()),  Moneychanger::It(), SLOT(mc_market_slot()));
    connect(buttonSecrets,    SIGNAL(clicked()),  Moneychanger::It(), SLOT(mc_passphrase_manager_slot()));
    connect(buttonRefresh,    SIGNAL(clicked()),  this,               SLOT(on_refreshButton_clicked()));
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
    QString  identity_label_string = QString("<font color=grey>%1:</font> ").arg(tr("My Identity"));
    QLabel * pIdentityLabel = new QLabel(identity_label_string);
    pIdentityLabel->setIndent(2);
//    pIdentityLabel->setContentsMargins(12, 0, 0, 5);
    pIdentityLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
//    pIdentityLabel->setStyleSheet("QLabel { margin-right: 0; font-size:20pt;  font-weight:lighter; }");
    // --------------------------------------------
    QString  nym_label_string = QString("");
    // --------------------------------------------
    if (!qstr_acct_nym.isEmpty())
    {
        MTNameLookupQT theLookup;
        QString qstr_name = QString::fromStdString(theLookup.GetNymName(qstr_acct_nym.toStdString(), ""));

        if (!qstr_name.isEmpty())
            nym_label_string = qstr_name;
        else
            nym_label_string = QString("(name is blank)");
    }
    else
        nym_label_string += tr("(none selected)");
    // ---------------------------------------------------------------
    QToolButton * buttonNym = new QToolButton;

    buttonNym->setText(nym_label_string);
    buttonNym->setAutoRaise(true);
    buttonNym->setStyleSheet("QToolButton { margin-left: 0; font-size:15pt;  font-weight:lighter; }");
//  buttonNym->setStyleSheet("QToolButton { margin-left: 0; font-size:20pt;  font-weight:lighter; }");
    // -------------------------------------------
    connect(buttonNym, SIGNAL(clicked()), Moneychanger::It(), SLOT(mc_defaultnym_slot()));
    // ----------------------------------------------------------------
    QHBoxLayout * pIdentityLayout = new QHBoxLayout;

    pIdentityLayout->setMargin(0);
    pIdentityLayout->setSpacing(0);
    pIdentityLayout->addSpacing(8);
    pIdentityLayout->addWidget(pIdentityLabel);
    pIdentityLayout->addWidget(buttonNym);
    // ---------------------------------------------------------------
    row_content_grid->addLayout(pIdentityLayout, 0,0, 1,1, Qt::AlignLeft);
    // -------------------------------------------
    return pUserBarWidget;
}

void MTHome::OnDeletedRecord()
{
    int nRowCount    = ui->tableWidget->rowCount();
    int nCurrentRow  = ui->tableWidget->currentRow();

    if ((nRowCount > 0) && (nCurrentRow >= 0) && (nCurrentRow < nRowCount))
    {
//      bool bRemoved = GetRecordlist().RemoveRecord(nCurrentRow);
        bool bRemoved = false;

        if (bRemoved)
        {
            qDebug() << QString("Removed record at index %1.").arg(nCurrentRow);
            // -----------------------------------------
            // We do this because the individual records keep track of their index inside their box.
            // Once a record is deleted, all the others now have bad indices, and must be reloaded.
            //
            emit needToPopulateRecordlist();
        }
        else
            qDebug() << QString("Failure removing opentxs::OTRecord at index %1.").arg(nCurrentRow);
    }
}

void MTHome::RefreshRecords()
{
    //(Lock the overview dialog refreshing mechinism until finished)
//    QMutexLocker overview_refresh_locker(&mc_overview_refreshing_visuals_mutex);
    // -------------------------------------------------------
    m_bTurnRefreshBtnRed = false;
    // -------------------------------------------------------
//  int listSize       = GetRecordlist().size();
    int listSize       = 0;  // Since I'm removing the recordlist.
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

        if (nullptr != item)
            delete item;
        item = nullptr;
    }
    // -------------------------------------------------------
    ui->tableWidget->setRowCount(nTotalRecords);
    // -------------------------------------------------------
//    for (int nIndex = 0; nIndex < listSize; ++nIndex)
//    {
//        opentxs::OTRecord record = GetRecordlist().GetRecord(nIndex);
//        {
//            opentxs::OTRecord& recordmt = record;
//            QWidget  * pWidget  = MTHomeDetail::CreateDetailHeaderWidget(recordmt);
//            // -------------------------------------------
//            if (NULL != pWidget)
//                ui->tableWidget->setCellWidget( nIndex, 1, pWidget );
//            else
//                qDebug() << "Failed creating detail header widget based on OTRecord.";
//        }
//    }
    // -------------------------------------------------------
    ui->tableWidget->blockSignals(false);
}




