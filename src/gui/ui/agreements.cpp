#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/ui/agreements.hpp>
#include <ui_agreements.h>

#include <gui/widgets/compose.hpp>
#include <gui/ui/dlgexportedtopass.hpp>
#include <gui/ui/dlgexportedcash.hpp>

#include <gui/widgets/home.hpp>
#include <gui/widgets/dlgchooser.hpp>
#include <gui/widgets/qrwidget.hpp>
#include <gui/ui/getstringdialog.hpp>

#include <core/moneychanger.hpp>
#include <core/handlers/DBHandler.hpp>
#include <core/handlers/contacthandler.hpp>
#include <core/handlers/focuser.h>

#include <opentxs/opentxs.hpp>

#include <QLabel>
#include <QToolButton>
#include <QKeyEvent>
#include <QApplication>
#include <QMessageBox>
#include <QMenu>
#include <QList>
#include <QFrame>

#include <string>
#include <map>
#include <tuple>



Agreements::Agreements(QWidget *parent) :
    QWidget(parent, Qt::Window),
    ui(new Ui::Agreements)
{
    ui->setupUi(this);

    this->installEventFilter(this);

//    connect(ui->toolButtonPay,      SIGNAL(clicked()), Moneychanger::It(), SLOT(mc_sendfunds_slot()));
//    connect(ui->toolButtonContacts, SIGNAL(clicked()), Moneychanger::It(), SLOT(mc_addressbook_slot()));
    connect(this, SIGNAL(needToCheckNym(QString, QString, QString)), Moneychanger::It(), SLOT(onNeedToCheckNym(QString, QString, QString)));
    // --------------------------------------------------------
    connect(this, SIGNAL(needToRefreshAgreements()), this, SLOT(RefreshAgreements()));
    connect(this, SIGNAL(needToRefreshReceipts()),   this, SLOT(RefreshReceipts()));
}

Agreements::~Agreements()
{
    delete ui;
}



static void setup_agreement_tableview(QTableView * pView, QAbstractItemModel * pProxyModel)
{
    AgreementsProxyModel * pTheProxyModel = static_cast<AgreementsProxyModel *>(pProxyModel);
    pTheProxyModel->setTableView(pView);

    pView->setModel(pTheProxyModel);
    pView->setSortingEnabled(true);
    pView->resizeColumnsToContents();
    pView->resizeRowsToContents();
    pView->horizontalHeader()->setStretchLastSection(true);
    pView->setEditTriggers(QAbstractItemView::NoEditTriggers);

//    QPointer<ModelAgreements> pSourceModel = DBHandler::getInstance()->getAgreementModel();

//    if (pSourceModel)
    {
//        QModelIndex sourceIndex = pSourceModel->index(0, AGRMT_SOURCE_COL_TIMESTAMP);
//        QModelIndex proxyIndex  = pTheProxyModel -> mapFromSource(sourceIndex);
        // ----------------------------------
        pView->sortByColumn(5, Qt::DescendingOrder); // The timestamp ends up at index 5 in all the proxy views.
//      qDebug() << "SORT COLUMN: " << proxyIndex.column() << "\n";
    }
    pView->setContextMenuPolicy(Qt::CustomContextMenu);
    pView->verticalHeader()->hide();
    pView->setAlternatingRowColors(true);
    pView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    pView->setSelectionBehavior(QAbstractItemView::SelectRows);
}


static void setup_receipt_tableview(QTableView * pView, QAbstractItemModel * pProxyModel)
{
    AgreementReceiptsProxyModel * pTheProxyModel = static_cast<AgreementReceiptsProxyModel *>(pProxyModel);
    pTheProxyModel->setTableView(pView);

    pView->setModel(pTheProxyModel);
    pView->setSortingEnabled(true);
    pView->resizeColumnsToContents();
    pView->resizeRowsToContents();
    pView->horizontalHeader()->setStretchLastSection(true);
    pView->setEditTriggers(QAbstractItemView::NoEditTriggers);

//    QPointer<ModelAgreementReceipts> pSourceModel = DBHandler::getInstance()->getAgreementReceiptModel();

//    if (pSourceModel)
    {
//        QModelIndex sourceIndex = pSourceModel->index(0, AGRMT_RECEIPT_COL_TIMESTAMP);
//        QModelIndex proxyIndex  = pTheProxyModel -> mapFromSource(sourceIndex);
        // ----------------------------------
        pView->sortByColumn(2, Qt::DescendingOrder); // The timestamp ends up at index 2 in all the proxy views.
//      qDebug() << "SORT COLUMN: " << proxyIndex.column() << "\n";
    }
    pView->setContextMenuPolicy(Qt::CustomContextMenu);
    pView->verticalHeader()->hide();
    pView->setAlternatingRowColors(true);
    pView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    pView->setSelectionBehavior(QAbstractItemView::SelectRows);
}



void Agreements::RefreshUserBar()
{
    if (m_pHeaderFrame)
    {
        ui->userBar->layout()->removeWidget(m_pHeaderFrame);

        m_pHeaderFrame->setParent(NULL);
        m_pHeaderFrame->disconnect();
        m_pHeaderFrame->deleteLater();

        m_pHeaderFrame = NULL;
    }
    // -------------------------------------------------
    QPointer<QWidget> pUserBar = this->CreateUserBarWidget();

    if (pUserBar)
    {
        // Note: Frame Shape should be Styled Panel, and Frame Shadow should be Raised.
        // Frame line width should be 1. Mid line width 0. LayoutDirection on frame should
        // be leftToRight. Font kerning should be on. Horizontal and Vertical policies
        // should be Preferred.
//        QPointer<QFrame> pHeaderFrame  = new QFrame;
//        // -----------------------------------------
//        pHeaderFrame->setFrameShape (QFrame::StyledPanel);
//        pHeaderFrame->setFrameShadow(QFrame::Raised);
//        pHeaderFrame->setLineWidth(1);
//        pHeaderFrame->setMidLineWidth(0);
//        // -----------------------------------------
//        QPointer<QGridLayout> pBalanceLayout = new QGridLayout;
//        pBalanceLayout->setAlignment(Qt::AlignTop);
//        // -----------------------------------------
//        pBalanceLayout->addWidget(pUserBar);
//        // -----------------------------------------
//        pHeaderFrame->setLayout(pBalanceLayout);
        // -----------------------------------------
        QHBoxLayout * pHBoxLayout = static_cast<QHBoxLayout*>(ui->userBar->layout());
        pHBoxLayout->insertWidget(0, pUserBar);
        // -----------------------------------------
        m_pHeaderFrame = pUserBar;
    }
    // --------------------------------------------------
}

QWidget * Agreements::CreateUserBarWidget()
{
    QWidget     * pUserBar        = new QWidget;
    QGridLayout * pUserBar_layout = new QGridLayout;

    pUserBar_layout->setSpacing(12);
    pUserBar_layout->setContentsMargins(0,0,0,0); // left top right bottom
//  pUserBar_layout->setContentsMargins(12, 3, 8, 10); // left top right bottom

    pUserBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    pUserBar->setLayout(pUserBar_layout);
    pUserBar->setStyleSheet("QWidget{background-color:#c0cad4;selection-background-color:#a0aac4;}");  // todo hardcoding.
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
        }
    }
    // ---------------------------------------------
//    QToolButton * buttonAccount = new QToolButton;

//    buttonAccount->setAutoRaise(true);
//    buttonAccount->setStyleSheet("QToolButton { margin-left: 0; font-size:30pt;  font-weight:lighter; }");

//    QLabel * tla_label = new QLabel(qstr_tla);
//    tla_label->setAlignment(Qt::AlignRight|Qt::AlignBottom);
//    tla_label->setStyleSheet("QLabel { margin-right: 0; font-size:20pt;  font-weight:lighter; }");

//    buttonAccount->setText(qstr_balance);
//    // -------------------------------------------
//    connect(buttonAccount, SIGNAL(clicked()), Moneychanger::It(), SLOT(mc_show_account_manager_slot()));
    // ----------------------------------------------------------------
//    QString  cash_label_string = QString("");
//    QString  qstrCash = qstr_acct_name;

//    if (!qstr_acct_nym.isEmpty() && !qstr_acct_server.isEmpty() && !qstr_acct_asset.isEmpty())
//    {
//        int64_t  raw_cash_balance = MTHome::rawCashBalance(qstr_acct_server, qstr_acct_asset, qstr_acct_nym);

//        if (raw_cash_balance > 0)
//        {
//            cash_label_string = MTHome::cashBalance(qstr_acct_server, qstr_acct_asset, qstr_acct_nym);
//            qstrCash += QString(" <small><font color=grey>(%2 %3 %4)</font></small>").arg(tr("plus")).arg(cash_label_string).arg(tr("in cash"));
//        }
//    }
//    else
//        qstrCash = tr("");
////      qstrCash = tr("(no account selected)");
    // -------------------------------------------
//    QLabel * pCashLabel = new QLabel(qstrCash);
//    // ---------------------------------------------------------------
//    //pCashLabel->setText(qstrCash);
//    pCashLabel->setIndent(13);
    // ---------------------------------------------------------------
//    QWidget * balanceWidget = new QWidget;
//    QHBoxLayout * balanceLayout = new QHBoxLayout;

//    tla_label->setContentsMargins(12, 0, 0, 5);
//    tla_label->setAlignment(Qt::AlignRight|Qt::AlignBottom);
//    balanceLayout->setSpacing(0);
//    balanceLayout->addWidget(tla_label);
//    balanceLayout->addWidget(buttonAccount);

//    balanceLayout->setMargin(0);
//    balanceLayout->setContentsMargins(0, 0, 0, 0);
////  balanceLayout->setContentsMargins(0, 20, 0, 0);
//    balanceWidget->setContentsMargins(0, 0, 0, 0);

//    balanceWidget->setLayout(balanceLayout);
    // ----------------------------------------------------------------
//    QVBoxLayout * pAccountLayout = new QVBoxLayout;

//    pAccountLayout->setMargin(0);
//    pAccountLayout->setContentsMargins(0, 0, 0, 0);
//    pAccountLayout->setSpacing(3);

//    pAccountLayout->addWidget(balanceWidget);
//    pAccountLayout->addWidget(pCashLabel);
//    pAccountLayout->addStretch();
    // ----------------------------------------------------------------
    QString  identity_label_string = QString("<font color=grey>%1:</font> ").arg(tr("My Identity"));
    QLabel * pIdentityLabel = new QLabel(identity_label_string);
    pIdentityLabel->setIndent(2);
//  pIdentityLabel->setContentsMargins(12, 0, 0, 5);
    pIdentityLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
//  pIdentityLabel->setStyleSheet("QLabel { margin-right: 0; font-size:20pt;  font-weight:lighter; }");
    // --------------------------------------------
    QString  nym_label_string = QString("");
    // --------------------------------------------
    QString qstrPaymentCode("");
    std::string payment_code("");

    if (!qstr_acct_nym.isEmpty())
    {
        payment_code = opentxs::OT::App().API().Exec().GetNym_Description(qstr_acct_nym.toStdString());
        qstrPaymentCode = QString::fromStdString(payment_code);
        // ----------------------------
        QString qstr_name = QString::fromStdString(opentxs::OT::App().API().Exec().GetNym_Name(qstr_acct_nym.toStdString()));

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
    QToolButton * buttonPaymentCode = nullptr;
    if (!qstrPaymentCode.isEmpty())
    {
        QrWidget qrWidget;
        qrWidget.setString(qstrPaymentCode);

        QImage image;
        qrWidget.asImage(image, 100);

        QPixmap pixmapQR = QPixmap::fromImage(image);
        // ----------------------------------------------------------------
        QIcon qrButtonIcon  (pixmapQR);
        // ----------------------------------------------------------------
        buttonPaymentCode = new QToolButton;

        buttonPaymentCode->setAutoRaise(true);
        buttonPaymentCode->setStyleSheet("QToolButton { margin-left: 0; font-size:15pt;  font-weight:lighter; }");
        buttonPaymentCode->setIcon(qrButtonIcon);
        buttonPaymentCode->setIconSize(pixmapQR.rect().size());
//      buttonPaymentCode->setString(qstrPaymentCode);
        // -------------------------------------------
        connect(buttonPaymentCode, SIGNAL(clicked()), Moneychanger::It(), SLOT(mc_defaultnym_slot()));
    }
    // -------------------------------------------
    QGridLayout * pIdentityLayout = new QGridLayout;

    pIdentityLayout->setMargin(0);
    pIdentityLayout->setContentsMargins(0, 0, 0, 0); // new
    pIdentityLayout->setSpacing(0);
    if (nullptr != buttonPaymentCode)
        pIdentityLayout->addWidget(buttonPaymentCode, 0,0,2,2, Qt::AlignCenter);
    pIdentityLayout->addWidget(pIdentityLabel, 2, 0, 1, 1);
    pIdentityLayout->addWidget(buttonNym, 2, 1, 1, 1);
    // ---------------------------------------------------------------
//    pUserBar_layout->addLayout(pAccountLayout,  0, 2, 2,2, Qt::AlignLeft);
    pUserBar_layout->addLayout(pIdentityLayout, 0, 0, 2,2, Qt::AlignLeft);

    return pUserBar;
}


void Agreements::on_tabWidgetAgreements_currentChanged(int index)
{
    qDebug() << "Top of on_tabWidgetAGREEMENTS_currentChanged";

    if (!setupCurrentPointers())
        return;
    // ------------------------------------
    int nRow{-1};
    int nAgreementId{0};

    // Let's say we just switched from the recurring payment plan tableView
    // to the smart contract tableView.
    // And the above call to setupCurrentPointers() has now set our pCurrent-style pointers
    // to point to the correct tableView, proxyModel, and map.

    const bool bGotRowAndAgreementId = rowAndIdForCurrentAgreement(nRow, nAgreementId);

    // No need to select any agreement since there is one already selected
    // for the current tableviews. (Or not.)
    if (!bGotRowAndAgreementId)
    {
        disableButtons();
    }
    else
    {
        enableButtons();
    }
    // -------------------------------------------------
    updateFilters(nAgreementId);
    emit needToRefreshAgreements();
    // -------------------------------------------------
}

void Agreements::on_tabWidgetReceipts_currentChanged(int index)
{
    qDebug() << "Top of on_tabWidgetRECEIPTS_currentChanged";

    if (!setupCurrentPointers())
        return;
    // ------------------------------------
    int nRow{-1};
    int nAgreementId{0};
    int nReceiptKey{0};

    // Let's say we just switched from the inbox to the outbox tableview.
    // And the above call to setupCurrentPointers() has now set our pCurrent-style pointers
    // to point to the correct tableView, proxyModel, and map.

    const bool bGotIds = agreementIdAndReceiptKeyForCurrentReceipt(nRow, nAgreementId, nReceiptKey); // output params
    // -------------------------------------------------
    if (bGotIds)
        updateFilters(nAgreementId);
    else
        emit needToRefreshReceipts();
    // -------------------------------------------------
}


void Agreements::enableButtons()
{
    ui->toolButtonDelete ->setEnabled(true);
}

void Agreements::disableButtons()
{
    ui->toolButtonDelete ->setEnabled(false);
}


void Agreements::on_MarkAgreementsAsRead_timer()
{
    QPointer<ModelAgreements> pModel = DBHandler::getInstance()->getAgreementModel();

    if (!pModel)
        return;
    // ------------------------------
    timer_MarkAsRead<>(pModel, listAgreementRecordsToMarkAsRead_);
}

void Agreements::on_MarkAgreementsAsUnread_timer()
{
    QPointer<ModelAgreements> pModel = DBHandler::getInstance()->getAgreementModel();

    if (!pModel)
        return;
    // ------------------------------
    timer_MarkAsUnread<>(pModel, listAgreementRecordsToMarkAsUnread_);
}

void Agreements::on_MarkReceiptsAsRead_timer()
{
    QPointer<ModelAgreementReceipts> pModel = DBHandler::getInstance()->getAgreementReceiptModel();

    if (!pModel)
        return;
    // ------------------------------
    timer_MarkAsRead<>(pModel, listReceiptRecordsToMarkAsRead_);
}

void Agreements::on_MarkReceiptsAsUnread_timer()
{
    QPointer<ModelAgreementReceipts> pModel = DBHandler::getInstance()->getAgreementReceiptModel();

    if (!pModel)
        return;
    // ------------------------------
    timer_MarkAsUnread<>(pModel, listReceiptRecordsToMarkAsUnread_);
}

// Used for an OUTSIDE WINDOW to send a signal for THIS window to
// display a specific agrement. That's why it sets the current payment
// to 0,0 (for that agreement.)
//
void Agreements::setAsCurrentAgreement(int nSourceRow, int nFolder) // The smart contract itself, or payment plan, that's selected.
{
    if (-1 == nSourceRow || -1 == nFolder)
        return;

    QPointer<ModelAgreements> pModel = DBHandler::getInstance()->getAgreementModel();

    if (!pModel)
        return;
    // -------------------
    const bool bSetToRecurringFolder     = (0 == nFolder);
//  const bool bSetToSmartContractFolder = (1 == nFolder);
    // -------------------
    QTableView * pTableView = bSetToRecurringFolder ? ui->tableViewRecurring
                                                    : ui->tableViewSmartContracts;
    QPointer<AgreementsProxyModel> & pProxyModel = bSetToRecurringFolder ? pProxyModelRecurring_
                                                                         : pProxyModelSmartContracts_;

    if (!pProxyModel || nullptr == pTableView) return; // should never happen.
    // ----------------------------
    // If the table view we're switching to, is not the current one, then we
    // need to make it the current one.
    //
    if (pTableView != pCurrentAgreementTableView_)
    {
        pCurrentAgreementProxyModel_ = pProxyModel;
        pCurrentAgreementTableView_  = pTableView;

        ui->tabWidgetAgreements->blockSignals(true);
        ui->tabWidgetAgreements->setCurrentIndex(bSetToRecurringFolder ? 0 : 1);
        ui->tabWidgetAgreements->blockSignals(false);
        on_tabWidgetAgreements_currentChanged(bSetToRecurringFolder ? 0 : 1); // Should happen already.
    }
    // ----------------------------
    // Should happen already.
    QModelIndex sourceIndex = pModel->index(nSourceRow, AGRMT_SOURCE_COL_AGRMT_ID);
    QModelIndex proxyIndex;
    if (sourceIndex.isValid())
        proxyIndex = pProxyModel->mapFromSource(sourceIndex);
    if (proxyIndex.isValid())
        pTableView->setCurrentIndex(proxyIndex);
    // ----------------------------
//    setAsCurrentPayment(0,0);
}

void Agreements::setAsCurrentParty(int nSourceRow) // You may have multiple signer Nyms on the same agreement, in the same wallet
{
    // todo
}
//resume now
void Agreements::setAsCurrentPayment(int nSourceRow, int nFolder) // the recurring paymentReceipts for each smart contract or payment plan.
{
    if (-1 == nSourceRow || -1 == nFolder)
        return;

    QPointer<ModelAgreementReceipts> pModel = DBHandler::getInstance()->getAgreementReceiptModel();

    if (!pModel)
        return;
    // -------------------
    const bool bSetToReceivedFolder = (0 == nFolder);
//  const bool bSetToSentFolder     = (1 == nFolder);
    // -------------------
    QTableView * pTableView =
            bSetToReceivedFolder ? ui->tableViewReceived : ui->tableViewSent;
    QPointer<AgreementReceiptsProxyModel> & pProxyModel =
            bSetToReceivedFolder ? pReceiptProxyModelInbox_ : pReceiptProxyModelOutbox_;

    if (!pProxyModel || nullptr == pTableView) return; // should never happen.
    // ----------------------------
    // If the table view we're switching to, is not the current one, then we
    // need to make it the current one.
    //
    if (pTableView != pCurrentReceiptTableView_)
    {
        pCurrentReceiptTableView_  = pTableView;
        pCurrentReceiptProxyModel_ = pProxyModel;
        ui->tabWidgetReceipts->blockSignals(true);
        ui->tabWidgetReceipts->setCurrentIndex(bSetToReceivedFolder ? 0 : 1);
        ui->tabWidgetReceipts->blockSignals(false);
        on_tabWidgetReceipts_currentChanged(bSetToReceivedFolder    ? 0 : 1); // Should happen already
    }
    // ----------------------------
    // Should happen already
    QModelIndex sourceIndex = pModel->index(nSourceRow, AGRMT_RECEIPT_COL_RECEIPT_ID);
    QModelIndex proxyIndex;
    if (sourceIndex.isValid())
        proxyIndex = pProxyModel->mapFromSource(sourceIndex);
    if (proxyIndex.isValid())
        pTableView->setCurrentIndex(proxyIndex);
}

// -------------------------------------------------------

void Agreements::updateFilters(int nAgreementId)
{
    qDebug() << "*** updateFilters, agreementId: " << QString("%1").arg(nAgreementId);

    if (nAgreementId > 0)
    {
        if (pReceiptProxyModelInbox_)  pReceiptProxyModelInbox_ ->setFilterAgreementId(nAgreementId);
        if (pReceiptProxyModelOutbox_) pReceiptProxyModelOutbox_->setFilterAgreementId(nAgreementId);
    }
    else
    {
        if (pReceiptProxyModelInbox_)  pReceiptProxyModelInbox_ ->setFilterAgreementId(0);
        if (pReceiptProxyModelOutbox_) pReceiptProxyModelOutbox_->setFilterAgreementId(0);
    }
}

void Agreements::on_tableViewRecurringSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous)
{
    qDebug() << QString("on_tableView RECURRING (top window) SelectionModel_currentRowChanged: %1").arg(current.row());

    int nRow{-1};
    int nAgreementId{0};

    const bool bGotRowAndAgreementId = agreementIdAndRow_Recurring(nRow, nAgreementId, &current);

    updateFilters(nAgreementId);
    emit needToRefreshReceipts();
}

void Agreements::on_tableViewSmartContractsSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous)
{
    qDebug() << QString("on_tableView SMART CONTRACTS (top window) SelectionModel_currentRowChanged: %1").arg(current.row());

    int nRow{-1};
    int nAgreementId{0};

    const bool bGotRowAndAgreementId = agreementIdAndRow_SmartContract(nRow, nAgreementId, &current);

    updateFilters(nAgreementId);
    emit needToRefreshReceipts();
}

// -------------------------------------------------------

void Agreements::on_tableViewPartiesSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous)
{
    // todo
}

// -------------------------------------------------------

//#define AGRMT_RECEIPT_COL_AGRMT_RECEIPT_KEY 0
//#define AGRMT_RECEIPT_COL_AGRMT_ID 1
//#define AGRMT_RECEIPT_COL_RECEIPT_ID 2
//#define AGRMT_RECEIPT_COL_TIMESTAMP 3
//#define AGRMT_RECEIPT_COL_HAVE_READ 4
//#define AGRMT_RECEIPT_COL_TXN_ID_DISPLAY 5
//#define AGRMT_RECEIPT_COL_EVENT_ID 6
//#define AGRMT_RECEIPT_COL_MEMO 7
//#define AGRMT_RECEIPT_COL_MY_ASSET_TYPE 8
//#define AGRMT_RECEIPT_COL_MY_NYM 9
//#define AGRMT_RECEIPT_COL_MY_ACCT 10
//#define AGRMT_RECEIPT_COL_MY_ADDR 11
//#define AGRMT_RECEIPT_COL_SENDER_NYM 12
//#define AGRMT_RECEIPT_COL_SENDER_ACCT 13
//#define AGRMT_RECEIPT_COL_SENDER_ADDR 14
//#define AGRMT_RECEIPT_COL_RECIP_NYM 15
//#define AGRMT_RECEIPT_COL_RECIP_ACCT 16
//#define AGRMT_RECEIPT_COL_RECIP_ADDR 17
//#define AGRMT_RECEIPT_COL_AMOUNT 18
//#define AGRMT_RECEIPT_COL_FOLDER 19
//#define AGRMT_RECEIPT_COL_METHOD_TYPE 20
//#define AGRMT_RECEIPT_COL_METHOD_TYPE_DISP 21
//#define AGRMT_RECEIPT_COL_NOTARY_ID 22
//#define AGRMT_RECEIPT_COL_DESCRIPTION 23
//#define AGRMT_RECEIPT_COL_RECORD_NAME 24
//#define AGRMT_RECEIPT_COL_INSTRUMENT_TYPE 25
//#define AGRMT_RECEIPT_COL_FLAGS 26


bool Agreements::setupCurrentPointers()
{
    qDebug() << "Top of setupCurrentPointers";

    if (   !pProxyModelRecurring_
        || !pProxyModelSmartContracts_
        || !pReceiptProxyModelInbox_
        || !pReceiptProxyModelOutbox_
        || (nullptr == ui)
        || (nullptr == ui->tabWidgetAgreements)
        || (nullptr == ui->tabWidgetReceipts)) {
        qDebug() << "setupCurrentPointers: Pointers were unexpectedly null.";
        return false;
    }
    // ---------------------------
    const bool bIsRecurring     = (0 == ui->tabWidgetAgreements->currentIndex());
    const bool bIsSmartContract = (1 == ui->tabWidgetAgreements->currentIndex());
    const bool bIsInbox         = (0 == ui->tabWidgetReceipts  ->currentIndex());
    const bool bIsOutbox        = (1 == ui->tabWidgetReceipts  ->currentIndex());
    // ---------------------------
    if (bIsRecurring) {
        pCurrentAgreementTableView_  = ui->tableViewRecurring;
        pCurrentAgreementProxyModel_ = &(*pProxyModelRecurring_);
    }
    else if (bIsSmartContract) {
        pCurrentAgreementTableView_  = ui->tableViewSmartContracts;
        pCurrentAgreementProxyModel_ = &(*pProxyModelSmartContracts_);
    }
    // ---------------------------
    if (bIsInbox) {
        pCurrentReceiptTableView_  = ui->tableViewReceived;
        pCurrentReceiptProxyModel_ = &(*pReceiptProxyModelInbox_);
    }
    else if (bIsOutbox) {
        pCurrentReceiptTableView_  = ui->tableViewSent;
        pCurrentReceiptProxyModel_ = &(*pReceiptProxyModelOutbox_);
    }
    // ---------------------------
    if (   (nullptr == pCurrentAgreementTableView_)
        || (nullptr == pCurrentAgreementProxyModel_)
        || (nullptr == pCurrentReceiptTableView_)
        || (nullptr == pCurrentReceiptProxyModel_) ) {
        qDebug() << "setupCurrentPointers: a table view or proxy model was somehow unexpectedly nullptr.";
        return false;
    }
    // ---------------------------
    std::map<int, int> & mapLastSelectedReceiptKey = bIsInbox
            ? mapLastSelectedInboxReceiptKey_
            : mapLastSelectedOutboxReceiptKey_;
    pMapLastSelectedReceiptKey_ = &mapLastSelectedReceiptKey;
    // ---------------------------
    return true;
}


bool Agreements::agreementIdAndReceiptKeyForInbox (int & nRow, int & nAgreementId, int & nReceiptKey,// output params
                                                   const QModelIndex * pModelIndex/*=nullptr*/)  // defaults to current index.
{
    qDebug() << "Top of agreementIdAndReceiptKeyForInbox (bottom window)";

    nRow = -1;
    nAgreementId = 0;
    nReceiptKey  = 0;
    // ---------------------------
    if ((nullptr == ui) || !pReceiptProxyModelInbox_)
        return false;
    // ---------------------------
    QTableView                  * pTableView  = ui->tableViewReceived;
    AgreementReceiptsProxyModel * pProxyModel = &(*pReceiptProxyModelInbox_);

    std::map<int, int> & mapReceiptKey = mapLastSelectedInboxReceiptKey_;
    // ---------------------------
    const QModelIndex & proxyIndexCurrentReceipt = (nullptr == pModelIndex)
            ? pTableView->currentIndex()
            : *pModelIndex;

    // If the current selected index on the inbox/outbox is invalid...
    // (This is normal, like row of -1 for an empty box.)
    //
//    if (!proxyIndexCurrentReceipt.isValid())
//        return false;
    // ---------------------------
    // Below this point, we are calling a low-level function, and it is guaranteed
    // that the output params are already initialized properly. It is also guaranteed
    // to have correct, valid, and matching tableView / proxyModel / proxyIndex / and map.
    // (So it doesn't check. It's a low level function.)
    //
    return agreementIdAndReceiptKey(nRow, nAgreementId, nReceiptKey, // output params on top
                                   mapReceiptKey,
                                   *pProxyModel,
                                   proxyIndexCurrentReceipt); // will be a valid index for the above tableview/proxymodel
}

bool Agreements::agreementIdAndReceiptKeyForOutbox(int & nRow, int & nAgreementId, int & nReceiptKey,// output params
                                                   const QModelIndex * pModelIndex/*=nullptr*/) // defaults to current index.
{
    qDebug() << "Top of agreementIdAndReceiptKeyForOutbox (bottom window)";

    nRow = -1;
    nAgreementId = 0;
    nReceiptKey  = 0;
    // ---------------------------
    if ((nullptr == ui) || !pReceiptProxyModelOutbox_)
        return false;
    // ---------------------------
    QTableView                  * pTableView  = ui->tableViewSent;
    AgreementReceiptsProxyModel * pProxyModel = &(*pReceiptProxyModelOutbox_);

    std::map<int, int> & mapReceiptKey = mapLastSelectedOutboxReceiptKey_;
    // ---------------------------
    const QModelIndex & proxyIndexCurrentReceipt = (nullptr == pModelIndex)
            ? pTableView->currentIndex()
            : *pModelIndex;

    // If the current selected index on the inbox/outbox is invalid...
    // (This is normal, like row of -1 for an empty box.)
    //
//    if (!proxyIndexCurrentReceipt.isValid())
//        return false;
    // ---------------------------
    // Below this point, we are calling a low-level function, and it is guaranteed
    // that the output params are already initialized properly. It is also guaranteed
    // to have correct, valid, and matching tableView / proxyModel / proxyIndex / and map.
    // (So it doesn't check. It's a low level function.)
    //
    return agreementIdAndReceiptKey(nRow, nAgreementId, nReceiptKey, // output params on top
                                   mapReceiptKey,
                                   *pProxyModel,
                                   proxyIndexCurrentReceipt); // will be a valid index for the above tableview/proxymodel
}

// This is just like currentAgreementIdAndReceiptKeyForInbox and
// currentAgreementIdAndReceiptKeyForOutbox, except it goes with
// whichever box is the one currently visible / selected on the screen.
//
bool Agreements::agreementIdAndReceiptKeyForCurrentReceipt(int & nRow, int & nAgreementId, int & nReceiptKey) // output params
{
    qDebug() << "--- Top of agreementIdAndReceiptKeyForCurrentReceipt (bottom window)";

    nRow = -1;
    nAgreementId = 0;
    nReceiptKey  = 0;
    // ---------------------------
    if (!setupCurrentPointers()) // Should never fail. Logs on failure.
        return false;
    // ---------------------------
    const QModelIndex & proxyIndexCurrentReceipt = pCurrentReceiptTableView_->currentIndex();

    // If the current selected index on the inbox/outbox is invalid...
    // (This is normal, like row of -1 for an empty box.)
    //
//    if (!proxyIndexCurrentReceipt.isValid())
//        return false;
    // ---------------------------
    // Below this point, we are calling a low-level function, and it is guaranteed
    // that the output params are already initialized properly. It is also guaranteed
    // to have correct, valid, and matching tableView / proxyModel / proxyIndex / and map.
    // (So it doesn't check. It's a low level function.)
    //
    return agreementIdAndReceiptKey(nRow, nAgreementId, nReceiptKey, // output params on top
                                   *pMapLastSelectedReceiptKey_,
                                   *pCurrentReceiptProxyModel_,
                                   proxyIndexCurrentReceipt); // will be a valid index for the above tableview/proxymodel
}



// This is a low-level function.
// Returns the rowId, agreementId, and receiptKey for a given index on a given proxy model.
// The row is the index of the proxy model. (And thus the index of the tablview on the user's screen.)
//
bool Agreements::agreementIdAndReceiptKey(int & nRow, int & nAgreementId, int & nReceiptKey, // output params on top
                                          std::map<int, int> & mapLastSelectedReceiptKey, // will be the right one for the inbox or outbox.
                                          AgreementReceiptsProxyModel & receiptProxyModel, // will be the right one for the above table view.
                                          const QModelIndex & proxyIndex) // will be a valid index for the above tableview/proxymodel
{
    // ---------------------------
    QPointer<ModelAgreementReceipts> pModel = DBHandler::getInstance()->getAgreementReceiptModel();

    if (!pModel) {
        qDebug() << "agreementIdAndReceiptKey: pModel was unexpectedly null.";
        return false;
    }
    // ---------------------------

    // TODO:

    // resume now

    int agreement_id{0};
    int receipt_key{0};

    // if proxyIndex is invalid, then try to find the last appropriate agreement ID
    // and use it to find the last receipt key according to the MAP.
    // Then use those, if you find anything, to get a proxy index we can actually use here
    // We'll see if that fixes it...

    if (!proxyIndex.isValid())
    {
        const bool bIsRecurring = (0 == ui->tabWidgetAgreements->currentIndex());

        const int & lastSelectedIndex = bIsRecurring
                ? nLastSelectedRecurringIndex_
                : nLastSelectedContractIndex_;
        const int & lastSelectedAgreementId = bIsRecurring
                ? nLastSelectedRecurringAgreementId_
                : nLastSelectedContractAgreementId_;
        // -------------------------------------------
        if (lastSelectedAgreementId > 0)
        {
            std::map<int,int>::iterator it = mapLastSelectedReceiptKey.find(lastSelectedAgreementId);

            if (mapLastSelectedReceiptKey.end() != it) // Found it.
            {
                agreement_id = lastSelectedAgreementId;
                receipt_key = it->second;
            }
        }
        // ---------------------------
        if (agreement_id > 0 && receipt_key > 0)
        {
            nRow         = lastSelectedIndex;
            nAgreementId = agreement_id;
            nReceiptKey  = receipt_key;
            return true;
        }
    }
    // ---------------------------
    // It would have returned if it had figured something out by now.
    // So if it's still invalid by this point, we have to try something else.
    //
    if (!proxyIndex.isValid())
    {
//        emit needToRefreshReceipts();
        return false;
    }
    // ---------------------------
    QModelIndex sourceIndex            = receiptProxyModel.mapToSource(proxyIndex);
    QModelIndex agreementIdSourceIndex = pModel->sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_AGRMT_ID, sourceIndex);
    QModelIndex receiptKeySourceIndex  = pModel->sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_AGRMT_RECEIPT_KEY, sourceIndex);
    QVariant    varAgrmtId = pModel->data(agreementIdSourceIndex);
    QVariant    varRcptKey = pModel->data(receiptKeySourceIndex);
    // ---------------------------
    agreement_id = varAgrmtId.isValid() ? varAgrmtId.toInt() : 0;
    receipt_key  = varRcptKey.isValid() ? varRcptKey.toInt() : 0;

    qDebug() << QString("       agreementIdAndReceiptKey (bottom window): %1, %2").arg(agreement_id).arg(receipt_key);

    if (agreement_id <= 0 || receipt_key <= 0)
        return false;
    // ---------------------------
    // At this point we know both Id and Key are valid.
    // Since they are supposed to be paired together in the map that
    // was passed in, let's set it to make sure.

    // It's not already there.
    std::map<int,int>::iterator it = mapLastSelectedReceiptKey.find(agreement_id);

    if (mapLastSelectedReceiptKey.end() != it) { // it's already there. (So we remove it.)
        mapLastSelectedReceiptKey.erase(it);
        qDebug() << QString("*=*=*=*=*=* Row: %3, ERASING receipt_key: %2 for agreement_id: %1").
                    arg(agreement_id).arg(receipt_key).arg(proxyIndex.row());
    }
    // By this point we know it's not on the map, so we insert the latest values.
    //
    qDebug() << QString("*=*=*=*=*=* Row: %3, INSERTING receipt_key: %2 for agreement_id: %1").
                arg(agreement_id).arg(receipt_key).arg(proxyIndex.row());
    mapLastSelectedReceiptKey.insert(std::pair<int,int>(agreement_id, receipt_key));
    // ---------------------------
    nRow         = proxyIndex.row();
    nAgreementId = agreement_id;
    nReceiptKey  = receipt_key;

    return true;
}


bool Agreements::rowAndIdForCurrentAgreement(int & nRow, int & nAgreementId)
{
    qDebug() << "Top of rowAndIdForCurrentAgreement";

    nRow = -1;
    nAgreementId = 0;
    // ---------------------------
    if ( nullptr == ui )
        return false;
    // ---------------------------
    if (!setupCurrentPointers()) // Should never fail. Logs on failure.
        return false;
    // ---------------------------------------------
    const bool bOnRecurringTab = (0 == ui->tabWidgetAgreements->currentIndex());

    const bool bGotIdAndRow = bOnRecurringTab
            ? agreementIdAndRow_Recurring    (nRow, nAgreementId)  // defaults to the current selected index
            : agreementIdAndRow_SmartContract(nRow, nAgreementId); // on the tableView.
    // ---------------------------------------------
    //
    return bGotIdAndRow;
}

//QPointer<AgreementsProxyModel> pProxyModelRecurring_;
//QPointer<AgreementsProxyModel> pProxyModelSmartContracts_;

bool Agreements::agreementIdAndRow_Recurring(int & nRow, int & nAgreementId, // output params
                                             const QModelIndex * pProxyIndex/*=nullptr*/) // defaults to current.
{
    qDebug() << "Top of agreementIdAndRow_Recurring";

    nRow = -1;
    nAgreementId = 0;
    // ---------------------------
    if ( nullptr == ui )
        return false;
    // ---------------------------------------------
    QTableView * pTableView = ui->tableViewRecurring;
    QPointer<AgreementsProxyModel> & pProxyModelQPtr = pProxyModelRecurring_;
    // ---------------------------------------------
    if ( nullptr == pTableView || !pProxyModelQPtr )
        return false;

    AgreementsProxyModel * pProxyModel = &(*pProxyModelQPtr);
    // ---------------------------------------
    QTableView & agreementTableView = *pTableView;
    AgreementsProxyModel & agreementProxyModel = *pProxyModel;

    const bool bGotIdAndRow = agreementIdAndRow(nRow, nAgreementId, // output params
                                agreementTableView,
                                agreementProxyModel,
                                pProxyIndex);

    if (pTableView->currentIndex().isValid())
    {
        if (bGotIdAndRow && pTableView->currentIndex().row() == nRow)
        {
            nLastSelectedRecurringIndex_ = nRow;
            nLastSelectedRecurringAgreementId_ = nAgreementId;
        }
    }
    else
    {
        nLastSelectedRecurringIndex_ = -1;
        nLastSelectedRecurringAgreementId_ = 0;
    }

    return bGotIdAndRow;
}

bool Agreements::agreementIdAndRow_SmartContract(int & nRow, int & nAgreementId, // output params
                                                 const QModelIndex * pProxyIndex/*=nullptr*/) // defaults to current.
{
    qDebug() << "Top of agreementIdAndRow_SmartContract";

    nRow = -1;
    nAgreementId = 0;
    // ---------------------------
    if ( nullptr == ui )
        return false;
    // ---------------------------------------------
    QTableView * pTableView = ui->tableViewSmartContracts;
    QPointer<AgreementsProxyModel> & pProxyModelQPtr = pProxyModelSmartContracts_;
    // ---------------------------------------------
    if ( nullptr == pTableView || !pProxyModelQPtr )
        return false;

    AgreementsProxyModel * pProxyModel = &(*pProxyModelQPtr);
    // ---------------------------------------
    QTableView & agreementTableView = *pTableView;
    AgreementsProxyModel & agreementProxyModel = *pProxyModel;

    const bool bGotIdAndRow = agreementIdAndRow(nRow, nAgreementId, // output params
                                agreementTableView,
                                agreementProxyModel,
                                pProxyIndex);

    if (pTableView->currentIndex().isValid())
    {
        if (bGotIdAndRow && pTableView->currentIndex().row() == nRow)
        {
            nLastSelectedContractIndex_ = nRow;
            nLastSelectedContractAgreementId_ = nAgreementId;
        }
    }
    else
    {
        nLastSelectedContractIndex_ = -1;
        nLastSelectedContractAgreementId_ = 0;
    }

    return bGotIdAndRow;
}

// This is for the Agreements tableView (top half of the window.)
// It does not care if there is any receipt (bottom half) selected,
// or even if there are any receipts at all.
//
bool Agreements::agreementIdAndRow(int & nRow, int & nAgreementId, // output params
                                   QTableView & agreementTableView,
                                   AgreementsProxyModel & agreementProxyModel,
                                   const QModelIndex * pProxyIndex/*=nullptr*/) // defaults to current.
{
    qDebug() << "--- Top of agreementIdAndRow (top window)";


    nRow = -1;
    nAgreementId = 0;
    // ---------------------------
    QPointer<ModelAgreements> pModel = DBHandler::getInstance()->getAgreementModel();

    if (!pModel) {
        qDebug() << "agreementIdAndRow: pModel was unexpectedly null.";
        return false;
    }
    // ---------------------------
    const QModelIndex & proxyIndex = (nullptr == pProxyIndex)
            ? agreementTableView.currentIndex()
            : *pProxyIndex;

    if (!proxyIndex.isValid())
        return false;
    // ---------------------------
    QModelIndex sourceIndex            = agreementProxyModel.mapToSource(proxyIndex);
    QModelIndex agreementIdSourceIndex = pModel->sibling(sourceIndex.row(), AGRMT_SOURCE_COL_AGRMT_ID, sourceIndex);
    QVariant    varAgrmtId             = pModel->data(agreementIdSourceIndex);
    // ---------------------------
    const int agreement_id = varAgrmtId.isValid() ? varAgrmtId.toInt() : 0;

    if (agreement_id <= 0)
        return false;
    // ---------------------------
    // At this point we know the ID is valid.
    nRow = proxyIndex.row();
    nAgreementId = agreement_id;

    return true;
}




void Agreements::on_tableViewSentSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous)
{
    qDebug() << QString("on_tableView SENT (bottom window) SelectionModel_currentRowChanged: %1").arg(current.row());

    QPointer<AgreementReceiptsProxyModel> & pReceiptProxyModel = pReceiptProxyModelOutbox_;
    QPointer<ModelAgreementReceipts> pModel = DBHandler::getInstance()->getAgreementReceiptModel();
    if (!pModel || !pReceiptProxyModel) // Should never happen.
        return;
    // ----------------------------------------
    int nRow{-1};
    int nAgreementId{0};
    int nReceiptKey{0};

    // If this returns false, that just means the current selected row
    // is -1 (whether rows exist or now) and that the two IDs are thus 0.
    //
    const bool bRowIsSelected = agreementIdAndReceiptKeyForOutbox(nRow, nAgreementId, nReceiptKey, &current);
    // ----------------------------------------
    if (!bRowIsSelected)
    {
        disableButtons();
        return;
    }// else:
    enableButtons();
    // ----------------------------------------
    QModelIndex sourceIndex         = pReceiptProxyModel->mapToSource(current);
    QModelIndex haveReadSourceIndex = pModel->sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_HAVE_READ,  sourceIndex);
    QVariant    varHaveRead         = pModel->data(haveReadSourceIndex);
    // ----------------------------------------------------------
    const bool bHaveRead = varHaveRead.isValid() ? varHaveRead.toBool() : false;

    if (!bHaveRead && (nReceiptKey > 0)) // It's unread, so we need to set it as read.
    {
        listReceiptRecordsToMarkAsRead_.append(haveReadSourceIndex);
        QTimer::singleShot(1000, this, SLOT(on_MarkReceiptsAsRead_timer()));
    }
}


void Agreements::on_tableViewReceivedSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous)
{
    qDebug() << QString("on_tableView RECEIVED (bottom window) SelectionModel_currentRowChanged: %1").arg(current.row());

    QPointer<AgreementReceiptsProxyModel> & pReceiptProxyModel = pReceiptProxyModelInbox_;
    QPointer<ModelAgreementReceipts> pModel = DBHandler::getInstance()->getAgreementReceiptModel();
    if (!pModel || !pReceiptProxyModel) // Should never happen.
        return;
    // ----------------------------------------
    int nRow{-1};
    int nAgreementId{0};
    int nReceiptKey{0};

    // If this returns false, that just means the current selected row
    // is -1 (whether rows exist or now) and that the two IDs are thus 0.
    //
    const bool bRowIsSelected = agreementIdAndReceiptKeyForInbox(nRow, nAgreementId, nReceiptKey, &current);
    // ----------------------------------------
    if (!bRowIsSelected)
    {
        disableButtons();
        return;
    }// else:
    enableButtons();
    // ----------------------------------------
    QModelIndex sourceIndex         = pReceiptProxyModel->mapToSource(current);
    QModelIndex haveReadSourceIndex = pModel->sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_HAVE_READ,  sourceIndex);
    QVariant    varHaveRead         = pModel->data(haveReadSourceIndex);
    // ----------------------------------------------------------
    const bool bHaveRead = varHaveRead.isValid() ? varHaveRead.toBool() : false;

    if (!bHaveRead && (nReceiptKey > 0)) // It's unread, so we need to set it as read.
    {
        listReceiptRecordsToMarkAsRead_.append(haveReadSourceIndex);
        QTimer::singleShot(1000, this, SLOT(on_MarkReceiptsAsRead_timer()));
    }
}


void Agreements::dialog(int nSourceRow/*=-1*/, int nFolder/*=-1*/)
{
    if (!already_init)
    {
        // Hide the signers stuff for now.
        //
        ui->checkBoxFilterByNym->setVisible(false);
        ui->labelSigners->setVisible(false);
        ui->tableViewMySigners->setVisible(false);
        ui->horizontalSpacerSigners->changeSize(1,1,QSizePolicy::Fixed);
        // ----------------------------------
        ui->userBar->setStyleSheet("QWidget{background-color:#c0cad4;selection-background-color:#a0aac4;}");
        // ----------------------------------
        QPixmap pixmapSize(":/icons/icons/user.png"); // This is here only for size purposes.
        QIcon   sizeButtonIcon(pixmapSize);           // Other buttons use this to set their own size.
        // ----------------------------------------------------------------
        QPixmap pixmapRefresh   (":/icons/icons/refresh.png");
        QPixmap pixmapDelete    (":/icons/icons/DeleteRed.png");
        QPixmap pixmapRecurring (":/icons/icons/timer.png");
        QPixmap pixmapPending   (":/icons/icons/pending.png");
        QPixmap pixmapSmart     (":/icons/icons/smart_contract_64.png");
        // ----------------------------------------------------------------
        QIcon refreshButtonIcon    (pixmapRefresh);
        QIcon deleteButtonIcon     (pixmapDelete);
        QIcon recurringButtonIcon  (pixmapRecurring);
        QIcon pendingButtonIcon    (pixmapPending);
        QIcon smartButtonIcon      (pixmapSmart);
        // ----------------------------------------------------------------
        ui->toolButtonRefresh->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButtonRefresh->setAutoRaise(true);
        ui->toolButtonRefresh->setIcon(refreshButtonIcon);
//      ui->toolButtonRefresh->setIconSize(pixmapRefresh.rect().size());
        ui->toolButtonRefresh->setIconSize(pixmapSize.rect().size());
        ui->toolButtonRefresh->setText(tr("Refresh"));
        // ----------------------------------------------------------------
        ui->toolButtonRecurring->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButtonRecurring->setAutoRaise(true);
        ui->toolButtonRecurring->setIcon(recurringButtonIcon);
//      ui->toolButtonRecurring->setIconSize(pixmapRefresh.rect().size());
        ui->toolButtonRecurring->setIconSize(pixmapSize.rect().size());
        ui->toolButtonRecurring->setText(tr("Request Recurring"));
        // ----------------------------------------------------------------
        ui->toolButtonPending->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButtonPending->setAutoRaise(true);
        ui->toolButtonPending->setIcon(pendingButtonIcon);
//      ui->toolButtonPending->setIconSize(pixmapRefresh.rect().size());
        ui->toolButtonPending->setIconSize(pixmapSize.rect().size());
        ui->toolButtonPending->setText(tr("View Pending"));
        // ----------------------------------------------------------------
        ui->toolButtonSmartContracts->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButtonSmartContracts->setAutoRaise(true);
        ui->toolButtonSmartContracts->setIcon(smartButtonIcon);
//      ui->toolButtonSmartContracts->setIconSize(pixmapRefresh.rect().size());
        ui->toolButtonSmartContracts->setIconSize(pixmapSize.rect().size());
        ui->toolButtonSmartContracts->setText(tr("Smart Contracts"));
        // ----------------------------------------------------------------
        ui->toolButtonDelete->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButtonDelete->setAutoRaise(true);
        ui->toolButtonDelete->setIcon(deleteButtonIcon);
//      ui->toolButtonDelete->setIconSize(pixmapDelete.rect().size());
        ui->toolButtonDelete->setIconSize(pixmapSize.rect().size());
        ui->toolButtonDelete->setText(tr("Delete"));
        // ----------------------------------------------------------------
        // Note: This is a placekeeper, so later on I can just erase
        // the widget at 0 and replace it with the real header widget.
        //
        m_pHeaderFrame  = new QFrame;
        QHBoxLayout * pHBoxLayout = static_cast<QHBoxLayout*>(ui->userBar->layout());
        pHBoxLayout->insertWidget(0, m_pHeaderFrame);
        // ----------------------------------

        // ******************************************************
        {
        QPointer<ModelAgreements> pModel = DBHandler::getInstance()->getAgreementModel();

        if (pModel)
        {
            pProxyModelRecurring_      = new AgreementsProxyModel;
            pProxyModelSmartContracts_ = new AgreementsProxyModel;

            pProxyModelRecurring_     ->setSourceModel(pModel);
            pProxyModelSmartContracts_->setSourceModel(pModel);

            pProxyModelRecurring_     ->setFilterFolder(0); // 0 Payment Plan, 1 Smart Contract, 2 Entity
            pProxyModelSmartContracts_->setFilterFolder(1);
//          pProxyModelEntities_      ->setFilterFolder(2);
            // ---------------------------------
            setup_agreement_tableview(ui->tableViewRecurring,      pProxyModelRecurring_     );
            setup_agreement_tableview(ui->tableViewSmartContracts, pProxyModelSmartContracts_);
            // ---------------------------------
            QItemSelectionModel *sm1 = ui->tableViewRecurring->selectionModel();
            connect(sm1, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                    this, SLOT(on_tableViewRecurringSelectionModel_currentRowChanged(QModelIndex,QModelIndex)));
            // ---------------------------------
            QItemSelectionModel *sm2 = ui->tableViewSmartContracts->selectionModel();
            connect(sm2, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                    this, SLOT(on_tableViewSmartContractsSelectionModel_currentRowChanged(QModelIndex,QModelIndex)));
        }
        }
        // --------------------------------------------------------
        {
        QPointer<ModelAgreementReceipts> pModel = DBHandler::getInstance()->getAgreementReceiptModel();

        if (pModel)
        {
            pReceiptProxyModelInbox_  = new AgreementReceiptsProxyModel;
            pReceiptProxyModelOutbox_ = new AgreementReceiptsProxyModel;

            pReceiptProxyModelInbox_ ->setSourceModel(pModel);
            pReceiptProxyModelOutbox_->setSourceModel(pModel);

            pReceiptProxyModelOutbox_->setFilterSent();
            pReceiptProxyModelInbox_ ->setFilterReceived();
            // ---------------------------------
            setup_receipt_tableview(ui->tableViewSent, pReceiptProxyModelOutbox_);
            setup_receipt_tableview(ui->tableViewReceived, pReceiptProxyModelInbox_);
            // ---------------------------------
            QItemSelectionModel *sm1 = ui->tableViewSent->selectionModel();
            connect(sm1, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                    this, SLOT(on_tableViewSentSelectionModel_currentRowChanged(QModelIndex,QModelIndex)));
            QItemSelectionModel *sm2 = ui->tableViewReceived->selectionModel();
            connect(sm2, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                    this, SLOT(on_tableViewReceivedSelectionModel_currentRowChanged(QModelIndex,QModelIndex)));
        }
        }
        // --------------------------------------------------------
        connect(this, SIGNAL(showContact(QString)),               Moneychanger::It(), SLOT(mc_showcontact_slot(QString)));
        // --------------------------------------------------------
        connect(this, SIGNAL(showContactAndRefreshHome(QString)), Moneychanger::It(), SLOT(onNeedToPopulateRecordlist()));
        connect(this, SIGNAL(showContactAndRefreshHome(QString)), Moneychanger::It(), SLOT(mc_showcontact_slot(QString)));
        // --------------------------------------------------------
        connect(ui->toolButtonRecurring,      SIGNAL(clicked()),  Moneychanger::It(), SLOT(mc_proposeplan_slot()));
        connect(ui->toolButtonPending,        SIGNAL(clicked()),  Moneychanger::It(), SLOT(mc_overview_slot()));
        connect(ui->toolButtonSmartContracts, SIGNAL(clicked()),  Moneychanger::It(), SLOT(mc_smartcontract_slot()));
        // --------------------------------------------------------
        QWidget* pTab0 = ui->tabWidgetAgreements->widget(0);
        QWidget* pTab1 = ui->tabWidgetAgreements->widget(1);
        QWidget* pTab2 = ui->tabWidgetReceipts->widget(0);
        QWidget* pTab3 = ui->tabWidgetReceipts->widget(1);

        pTab0->setStyleSheet("QWidget { margin: 0 }");
        pTab1->setStyleSheet("QWidget { margin: 0 }");
        pTab2->setStyleSheet("QWidget { margin: 0 }");
        pTab3->setStyleSheet("QWidget { margin: 0 }");

//        ui->splitter->setStretchFactor(0, 2);
//        ui->splitter->setStretchFactor(1, 3);

//        ui->splitter->setStretchFactor(0, 1);
//        ui->splitter->setStretchFactor(1, 5);
        // ------------------------

//        if (!Moneychanger::It()->expertMode())
        {
            QList<int> list;
            list.append(100);
            list.append(0);
            ui->splitter->setSizes(list);
        }


        setupCurrentPointers();

        // resume now
//        ui->tabWidgetAgreements->setCurrentIndex(0);

//        ui->tabWidgetReceipts->setCurrentIndex(0);

//        on_tabWidgetReceipts_currentChanged(0);
    }
    // -------------------------------------------
    Focuser f(this);
    f.show();
    f.focus();
    // -------------------------------------------
//    if (nSourceRow >=0 && nFolder >= 0)
//        setAsCurrentAgreement(nSourceRow, nFolder);
//    else if (!already_init)
//        setAsCurrentAgreement(0,0);
    // -------------------------------------------
    if (!already_init)
    {
//        on_tabWidgetAgreements_currentChanged(0);
//        on_tabWidgetReceipts_currentChanged(0);

        /** Flag Already Init **/
        already_init = true;
    }

    RefreshAll();
}



void Agreements::onClaimsUpdatedForNym(QString nymId)
{
    if (!bRefreshingAfterUpdatedClaims_)
    {
        bRefreshingAfterUpdatedClaims_ = true;
        QTimer::singleShot(500, this, SLOT(RefreshAgreements()));
    }
}


void Agreements::RefreshAgreements()
{
    qDebug() << "======------ RefreshAgreements ------======";

    if (nullptr == pCurrentAgreementTableView_ || nullptr == pCurrentAgreementProxyModel_)
        return;
    // -------------------------------------------
    const bool bOnRecurringTab     = (0 == ui->tabWidgetAgreements->currentIndex());
    const bool bOnSmartContractTab = (1 == ui->tabWidgetAgreements->currentIndex());
    const bool bOnInboxTab         = (0 == ui->tabWidgetReceipts  ->currentIndex());
    const bool bOnOutboxTab        = (1 == ui->tabWidgetReceipts  ->currentIndex());
    // -------------------------------------------
    QPointer<ModelAgreements> pModel = DBHandler::getInstance()->getAgreementModel();
    if (!pModel)
        return;
    // -------------------------------------------
//  std::pair of: bool bTab, int nLastIndex
    //static std::map<bool, int> mapIndices;
    // -------------------------------------------
    int nLastSelectedIndex = bOnSmartContractTab
            ? nLastSelectedContractIndex_ : nLastSelectedRecurringIndex_;
    // -------------------------------------------
    QModelIndex currentIndex = pCurrentAgreementTableView_->currentIndex();

    if (currentIndex.isValid())
        nLastSelectedIndex = currentIndex.row();
    else if (pCurrentAgreementProxyModel_->rowCount() <= 0)
        nLastSelectedIndex = -1;
    else if (nLastSelectedIndex >= pCurrentAgreementProxyModel_->rowCount())
        nLastSelectedIndex = 0;
    // ------------------------------------------------------

    pModel->select(); // <=== REFRESHES ALL THE AGREEMENTS FROM THE DATABASE RIGHT HERE!!

//    ui->tableViewRecurring->reset();        // Might be unnecessary. todo remove.
//    ui->tableViewSmartContracts->reset();   // That goes for both of these.

    ui->tableViewRecurring->resizeColumnsToContents();
    ui->tableViewRecurring->resizeRowsToContents();

    ui->tableViewSmartContracts->resizeColumnsToContents();
    ui->tableViewSmartContracts->resizeRowsToContents();

    ui->tableViewRecurring->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewSmartContracts->horizontalHeader()->setStretchLastSection(true);
    // -------------------------------------------

    // -------------------------------------------
    int nRowToSelect = -1;

    if (pCurrentAgreementProxyModel_->rowCount() > 0)
    {
        nRowToSelect = nLastSelectedIndex;

        if (nRowToSelect < 0)
            nRowToSelect = 0;
        else if (nRowToSelect >= pCurrentAgreementProxyModel_->rowCount())
            nRowToSelect = 0;
    }
    // ------------------------------------------------------
    QModelIndex previous = pCurrentAgreementTableView_->currentIndex();
    pCurrentAgreementTableView_->blockSignals(true);
    qDebug() << "===> AGREEMENT (top window) calling selectRow: " << QString("%1").arg(nRowToSelect);
    pCurrentAgreementTableView_->selectRow(nRowToSelect);
    pCurrentAgreementTableView_->blockSignals(false);

    if (bOnSmartContractTab)
        on_tableViewSmartContractsSelectionModel_currentRowChanged(pCurrentAgreementTableView_->currentIndex(), previous);
    else
        on_tableViewRecurringSelectionModel_currentRowChanged(pCurrentAgreementTableView_->currentIndex(), previous);
    // ------------------------------------------
//    ui->tableViewRecurring->resizeColumnsToContents();
//    ui->tableViewRecurring->resizeRowsToContents();

//    ui->tableViewSmartContracts->resizeColumnsToContents();
//    ui->tableViewSmartContracts->resizeRowsToContents();

//    ui->tableViewRecurring->horizontalHeader()->setStretchLastSection(true);
//    ui->tableViewSmartContracts->horizontalHeader()->setStretchLastSection(true);
//    // -------------------------------------------
}


void Agreements::RefreshReceipts()
{
    qDebug() << "---=== RefreshReceipts ===---";

    if (nullptr == pCurrentReceiptTableView_ || nullptr == pCurrentReceiptProxyModel_)
        return;

    QPointer<ModelAgreementReceipts> pModel = DBHandler::getInstance()->getAgreementReceiptModel();
    if (!pModel)
        return;
    // -------------------------------------------
    bRefreshingAfterUpdatedClaims_ = false;
    // ------------------------------------------------------
    pModel->select();
    // -------------------------------------------
//    ui->tableViewSent->reset();
//    ui->tableViewReceived->reset();
    // -------------------------------------------
    bool bIsRecurring = (0 == ui->tabWidgetAgreements->currentIndex());
    bool bIsInbox     = (0 == ui->tabWidgetReceipts->currentIndex());
    // -------------------------------------------
    // Let's see if we can find what this is supposed to be, based
    // on whatever the current agreement_id is. (If there is one.)
    //
    int nReceiptKey = 0;
    int nLastSelectedAgreementId = bIsRecurring ? nLastSelectedRecurringAgreementId_ : nLastSelectedContractAgreementId_;
    std::map<int, int> & mapLastSelectedReceiptKey = bIsInbox ? mapLastSelectedInboxReceiptKey_ : mapLastSelectedOutboxReceiptKey_;
    // -------------------------------------------
    if (nLastSelectedAgreementId > 0
//        && mapLastSelectedReceiptKey.end() != mapLastSelectedReceiptKey.find(nLastSelectedAgreementId)
       )
    {
        std::map<int,int>::iterator it = mapLastSelectedReceiptKey.find(nLastSelectedAgreementId);

        if (mapLastSelectedReceiptKey.end() != it) // it's there.
            nReceiptKey = it->second;
    }

    int nRowToSelect = -1;

    if (nReceiptKey <= 0) // There's no "current selected agreement receipt key" set for this agreement_id.
    {
        if (pCurrentReceiptProxyModel_->rowCount() > 0) // But there ARE receipt rows for this agreement_id...
            nRowToSelect = 0;

        // So let's select the first one in the list!
        QModelIndex previous = pCurrentReceiptTableView_->currentIndex();
        pCurrentReceiptTableView_->blockSignals(true);
        qDebug() << QString("===> RECEIPT (bottom window) with nLastSelectedAgreementId %1, calling selectRow: %2 for nReceiptKey: %3").
                    arg(nLastSelectedAgreementId).arg(nRowToSelect).arg(nReceiptKey);
        pCurrentReceiptTableView_->selectRow(nRowToSelect);
        pCurrentReceiptTableView_->blockSignals(false);

        if (bIsInbox)
            on_tableViewReceivedSelectionModel_currentRowChanged(pCurrentReceiptTableView_->currentIndex(), previous);
        else
            on_tableViewSentSelectionModel_currentRowChanged(pCurrentReceiptTableView_->currentIndex(), previous);
    }
    else // There IS a "current selected agreement receipt key" for the current agreement_id.
    {    // That is, nReceiptKey is larger than 0.
        // So let's try to select that again! (If it's still there. Otherwise set it to row 0.)

        bool bFoundIt = false;

        const int nRowCount = pCurrentReceiptProxyModel_->rowCount();

        for (int ii = 0; ii < nRowCount; ++ii)
        {
            QModelIndex indexProxy  = pCurrentReceiptProxyModel_->index(ii, 0);
            QModelIndex indexSource = pCurrentReceiptProxyModel_->mapToSource(indexProxy);

            QSqlRecord record = pModel->record(indexSource.row());

            if (!record.isEmpty())
            {
                QVariant the_value = record.value(AGRMT_RECEIPT_COL_AGRMT_RECEIPT_KEY);
                const int nAgreementReceiptKey = the_value.isValid() ? the_value.toInt() : 0;

                if (nAgreementReceiptKey == nReceiptKey)
                {
                    bFoundIt = true;
                    nRowToSelect = indexProxy.row();
                    break;
                }
            }
        }
        // ------------------------------------
        if (!bFoundIt)
        {
            if (nRowCount > 0)
                nRowToSelect = 0;
        }
        // ------------------------------------
        QModelIndex previous = pCurrentReceiptTableView_->currentIndex();
        pCurrentReceiptTableView_->blockSignals(true);
        qDebug() << QString("===> RECEIPT (bottom window) with nLastSelectedAgreementId %1, calling selectRow: %2 for nReceiptKey: %3").
                    arg(nLastSelectedAgreementId).arg(nRowToSelect).arg(nReceiptKey);
        pCurrentReceiptTableView_->selectRow(nRowToSelect);
        pCurrentReceiptTableView_->blockSignals(false);

        if (bIsInbox)
            on_tableViewReceivedSelectionModel_currentRowChanged(pCurrentReceiptTableView_->currentIndex(), previous);
        else
            on_tableViewSentSelectionModel_currentRowChanged(pCurrentReceiptTableView_->currentIndex(), previous);
    }
    // -------------------------------------------
    qDebug() << "RESIZING the bottom records on the screen.";

    ui->tableViewSent->resizeColumnsToContents();
    ui->tableViewReceived->resizeColumnsToContents();
    {
    int nWidthFirstColumn = ui->tableViewSent->columnWidth(0);
    int nNewWidth = static_cast<int>( static_cast<float>(nWidthFirstColumn) * 1.2 );
    ui->tableViewSent->setColumnWidth(0,nNewWidth);
    }{
    int nWidthFirstColumn = ui->tableViewReceived->columnWidth(0);
    int nNewWidth = static_cast<int>( static_cast<float>(nWidthFirstColumn) * 1.2 );
    ui->tableViewReceived->setColumnWidth(0,nNewWidth);
    }
    ui->tableViewSent->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewReceived->horizontalHeader()->setStretchLastSection(true);

    ui->tableViewSent->resizeRowsToContents();
    ui->tableViewReceived->resizeRowsToContents();
}

// --------------------------------------------------

void Agreements::on_tableViewReceived_customContextMenuRequested(const QPoint &pos)
{
    tableViewReceiptsPopupMenu(pos, ui->tableViewReceived, &(*pReceiptProxyModelInbox_));
}

void Agreements::on_tableViewSent_customContextMenuRequested(const QPoint &pos)
{
    tableViewReceiptsPopupMenu(pos, ui->tableViewSent, &(*pReceiptProxyModelOutbox_));
}

void Agreements::on_tableViewRecurring_customContextMenuRequested(const QPoint &pos)
{
    tableViewAgreementsPopupMenu(pos, ui->tableViewRecurring, &(*pProxyModelRecurring_));
}

void Agreements::on_tableViewSmartContracts_customContextMenuRequested(const QPoint &pos)
{
    tableViewAgreementsPopupMenu(pos, ui->tableViewSmartContracts, &(*pProxyModelSmartContracts_));
}

// --------------------------------------------------

void Agreements::tableViewAgreementsPopupMenu(const QPoint &pos, QTableView * pTableView, AgreementsProxyModel * pProxyModel)
{
    QPointer<ModelAgreements> pModel = DBHandler::getInstance()->getAgreementModel();

    if (!pModel)
        return;
    // ------------------------
    QModelIndex indexAtRightClick = pTableView->indexAt(pos);
    if (!indexAtRightClick.isValid())
        return;
    // I can't figure out how to ADD to the selection without UNSELECTING everything else.
    // The Qt docs indicate that the below options should do that -- but it doesn't work.
    // So this is commented out since it was deselecting everything.
    //pTableView->selectionModel()->select( indexAtRightClick, QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows );
    // ------------------------
    QModelIndex sourceIndexAtRightClick = pProxyModel->mapToSource(indexAtRightClick);
    const int nRow = sourceIndexAtRightClick.row();
    // ----------------------------------
    popupMenu_.reset(new QMenu(this));
    // ----------------------------------
    pActionDelete              = nullptr;
    pActionKill                = nullptr;
    pActionMarkRead            = nullptr;
    pActionMarkUnread          = nullptr;
    pActionOpenNewWindow       = nullptr;
    pActionViewContact         = nullptr;
    pActionCreateContact       = nullptr;
    pActionExistingContact     = nullptr;
    pActionAcceptIncoming      = nullptr;
    pActionCancelOutgoing      = nullptr;
    pActionDiscardIncoming     = nullptr;
    pActionDownloadCredentials = nullptr;
    pActionReply               = nullptr;
    pActionForward             = nullptr;
    // ----------------------------------
    int nAgreementId{};
    agreement_status nNewestState{};
    int64_t lTxnIdDisplay{};
    int64_t lNewestReceiptId{};
    time64_t timestamp{};

    QString qstrNotaryId;
    QString qstrSubject;
    QString qstrContractId;
    // ----------------------------------------------
    //    enum class agreement_status { ERROR = 0,
    //                                  OUTGOING = 1,
    //                                  INCOMING = 2,
    //                                  ACTIVATED = 3,
    //                                  PAID = 4,
    //                                  PAYMENT_FAILED = 5,    // Live agreement here and above this line.
    //                                  FAILED_ACTIVATING = 6, // Dead agreement here and below this line.
    //                                  CANCELED = 7,
    //                                  EXPIRED = 8,
    //                                  NO_LONGER_ACTIVE = 9,
    //                                  KILLED = 10
    //                                };

    if (nRow >= 0)
    {
        QModelIndex indexAgreementId     = pModel->index(nRow, AGRMT_SOURCE_COL_AGRMT_ID);
        QModelIndex indexTxnIdDisplay    = pModel->index(nRow, AGRMT_SOURCE_COL_TXN_ID_DISPLAY);
        QModelIndex indexNotaryId        = pModel->index(nRow, AGRMT_SOURCE_COL_NOTARY_ID);
        QModelIndex indexContractId      = pModel->index(nRow, AGRMT_SOURCE_COL_CONTRACT_ID);
        QModelIndex indexNewestReceiptId = pModel->index(nRow, AGRMT_SOURCE_COL_NEWEST_RECEIPT_ID);
        QModelIndex indexNewestState     = pModel->index(nRow, AGRMT_SOURCE_COL_NEWEST_KNOWN_STATE);
        QModelIndex indexTimestamp       = pModel->index(nRow, AGRMT_SOURCE_COL_TIMESTAMP);
        QModelIndex indexSubject         = pModel->index(nRow, AGRMT_SOURCE_COL_MEMO);

        QVariant varAgreementId     = pModel->rawData(indexAgreementId);
        QVariant varTxnIdDisplay    = pModel->rawData(indexTxnIdDisplay);
        QVariant varNotaryId        = pModel->rawData(indexNotaryId);
        QVariant varContractId      = pModel->rawData(indexContractId);
        QVariant varNewestReceiptId = pModel->rawData(indexNewestReceiptId);
        QVariant varNewestState     = pModel->rawData(indexNewestState);
        QVariant varTimestamp       = pModel->rawData(indexTimestamp);
        QVariant varSubject         = pModel->rawData(indexSubject);

        nAgreementId     = varAgreementId    .isValid() ? varAgreementId    .toInt()      : 0;
        lTxnIdDisplay    = varTxnIdDisplay   .isValid() ? varTxnIdDisplay   .toLongLong() : 0;
        qstrNotaryId     = varNotaryId       .isValid() ? varNotaryId       .toString()   : QString("");
        qstrContractId   = varContractId     .isValid() ? varContractId     .toString()   : QString("");
        lNewestReceiptId = varNewestReceiptId.isValid() ? varNewestReceiptId.toLongLong() : 0;
        nNewestState     = static_cast<agreement_status>(varNewestState.isValid() ? varNewestState.toInt() : 0);
        timestamp        = varTimestamp      .isValid() ? varTimestamp      .toLongLong() : 0;
        qstrSubject      = varSubject        .isValid() ? varSubject        .toString()   : QString("");
        // -------------------------------
        popupMenu_->addSeparator();
        // -------------------------------
        if ( nNewestState >= agreement_status::FAILED_ACTIVATING ) // It's dead, for one reason or another.
        {
            QString nameString = tr("Delete");
            QString actionString = tr("Deleting...");

            pActionDelete = popupMenu_->addAction(nameString);
            popupMenu_->addSeparator();
        }
        else if ( nNewestState >= agreement_status::ACTIVATED && nNewestState <= agreement_status::PAYMENT_FAILED )
        {
            QString nameString = tr("Kill");
            QString actionString = tr("Killing...");

            pActionKill = popupMenu_->addAction(nameString);
            popupMenu_->addSeparator();
        }
        pActionMarkRead = popupMenu_->addAction(tr("Mark as read"));
        pActionMarkUnread = popupMenu_->addAction(tr("Mark as unread"));
    }
    // --------------------------------------------------
    QPoint globalPos = pTableView->mapToGlobal(pos);
    const QAction* selectedAction = popupMenu_->exec(globalPos); // Here we popup the menu, and get the user's click.
    if (nullptr == selectedAction)
        return;
    // ----------------------------------
    else if (selectedAction == pActionDelete) // May delete many agreements.
    {
        on_toolButtonDelete_clicked();
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionKill) // May kill many agreements.
    {
        killSelectedAgreement();
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionMarkRead) // May mark many agreements.
    {
        if (!pTableView->selectionModel()->hasSelection())
            return;
        // ----------------------------------------------
        QItemSelection selection( pTableView->selectionModel()->selection() );
        QList<int> rows;
        foreach( const QModelIndex & index, selection.indexes() )
        {
            if (rows.indexOf(index.row()) != (-1)) // This row is already on the list, so skip it.
                continue;
            rows.append(index.row());
            // -----------------------
            QModelIndex sourceIndex = pProxyModel->mapToSource(index);
            QModelIndex sourceIndexHaveRead = pModel->sibling(sourceIndex.row(), AGRMT_SOURCE_COL_HAVE_READ, sourceIndex);
            // --------------------------------
            if (sourceIndexHaveRead.isValid())
                listAgreementRecordsToMarkAsRead_.append(sourceIndexHaveRead);
        }
        if (listAgreementRecordsToMarkAsRead_.count() > 0)
            QTimer::singleShot(0, this, SLOT(on_MarkAgreementsAsRead_timer()));
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionMarkUnread) // May mark many agreements.
    {
        if (!pTableView->selectionModel()->hasSelection())
            return;
        // ----------------------------------------------
        QItemSelection selection( pTableView->selectionModel()->selection() );
        QList<int> rows;
        foreach( const QModelIndex & index, selection.indexes() )
        {
            if (rows.indexOf(index.row()) != (-1)) // This row is already on the list, so skip it.
                continue;
            rows.append(index.row());
            // -----------------------
            QModelIndex sourceIndex = pProxyModel->mapToSource(index);
            QModelIndex sourceIndexHaveRead = pModel->sibling(sourceIndex.row(), AGRMT_SOURCE_COL_HAVE_READ, sourceIndex);
            // --------------------------------
            if (sourceIndexHaveRead.isValid())
                listAgreementRecordsToMarkAsUnread_.append(sourceIndexHaveRead);
        }
        if (listAgreementRecordsToMarkAsUnread_.count() > 0)
            QTimer::singleShot(0, this, SLOT(on_MarkAgreementsAsUnread_timer()));
        return;
    }
}

// --------------------------------------------------
//QAction * pActionReply               = nullptr;
//QAction * pActionForward             = nullptr;
//QAction * pActionAcceptIncoming      = nullptr;
//QAction * pActionCancelOutgoing      = nullptr;
//QAction * pActionDiscardOutgoingCash = nullptr;
//QAction * pActionDiscardIncoming     = nullptr;
//QAction * pActionDelete              = nullptr;
//QAction * pActionOpenNewWindow       = nullptr;
//QAction * pActionMarkRead            = nullptr;
//QAction * pActionMarkUnread          = nullptr;
//QAction * pActionViewContact         = nullptr;
//QAction * pActionCreateContact       = nullptr;
//QAction * pActionExistingContact     = nullptr;
//QAction * pActionDownloadCredentials = nullptr;

//QPointer<AgreementsProxyModel> pProxyModelRecurring_;
//QPointer<AgreementsProxyModel> pProxyModelSmartContracts_;

//QPointer<AgreementReceiptsProxyModel> pReceiptProxyModelInbox_;
//QPointer<AgreementReceiptsProxyModel> pReceiptProxyModelOutbox_;

//QTableView         * pCurrentAgreementTableView_ = nullptr; // Recurring payments or Smart contracts.
//QTableView         * pCurrentPartyTableView_     = nullptr; // Might remove this. (I don't see how it would ever change.)
//QTableView         * pCurrentReceiptTableView_   = nullptr; // Sent or Received.

//AgreementsProxyModel        * pCurrentAgreementProxyModel_ = nullptr;
//AgreementReceiptsProxyModel * pCurrentReceiptProxyModel_   = nullptr;

//QList<QModelIndex> listRecordsToMarkAsRead_;
//QList<QModelIndex> listRecordsToMarkAsUnread_;

//bool bRefreshingAfterUpdatedClaims_=false;

//int nLastSelectedRecurringIndex_ = -1;
//int nLastSelectedContractIndex_  = -1;


//#define AGRMT_RECEIPT_COL_AGRMT_RECEIPT_KEY 0
//#define AGRMT_RECEIPT_COL_AGRMT_ID 1
//#define AGRMT_RECEIPT_COL_RECEIPT_ID 2
//#define AGRMT_RECEIPT_COL_TIMESTAMP 3
//#define AGRMT_RECEIPT_COL_HAVE_READ 4
//#define AGRMT_RECEIPT_COL_TXN_ID_DISPLAY 5
//#define AGRMT_RECEIPT_COL_EVENT_ID 6
//#define AGRMT_RECEIPT_COL_MEMO 7
//#define AGRMT_RECEIPT_COL_MY_ASSET_TYPE 8
//#define AGRMT_RECEIPT_COL_MY_NYM 9
//#define AGRMT_RECEIPT_COL_MY_ACCT 10
//#define AGRMT_RECEIPT_COL_MY_ADDR 11
//#define AGRMT_RECEIPT_COL_SENDER_NYM 12
//#define AGRMT_RECEIPT_COL_SENDER_ACCT 13
//#define AGRMT_RECEIPT_COL_SENDER_ADDR 14
//#define AGRMT_RECEIPT_COL_RECIP_NYM 15
//#define AGRMT_RECEIPT_COL_RECIP_ACCT 16
//#define AGRMT_RECEIPT_COL_RECIP_ADDR 17
//#define AGRMT_RECEIPT_COL_AMOUNT 18
//#define AGRMT_RECEIPT_COL_FOLDER 19
//#define AGRMT_RECEIPT_COL_METHOD_TYPE 20
//#define AGRMT_RECEIPT_COL_METHOD_TYPE_DISP 21
//#define AGRMT_RECEIPT_COL_NOTARY_ID 22
//#define AGRMT_RECEIPT_COL_DESCRIPTION 23
//#define AGRMT_RECEIPT_COL_RECORD_NAME 24
//#define AGRMT_RECEIPT_COL_INSTRUMENT_TYPE 25
//#define AGRMT_RECEIPT_COL_FLAGS 26


void Agreements::AcceptIncomingReceipt (QPointer<ModelAgreementReceipts> & pModel, AgreementReceiptsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView)
{
    emit showDashboard();
}

void Agreements::CancelOutgoingReceipt (QPointer<ModelAgreementReceipts> & pModel, AgreementReceiptsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView)
{
    emit showDashboard();
}

void Agreements::DiscardIncomingReceipt(QPointer<ModelAgreementReceipts> & pModel, AgreementReceiptsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView)
{
    emit showDashboard();
}

void Agreements::tableViewReceiptsPopupMenu(const QPoint &pos, QTableView * pTableView, AgreementReceiptsProxyModel * pProxyModel)
{
//    QPointer<ModelAgreementReceipts> pModel = DBHandler::getInstance()->getAgreementReceiptModel();

//    if (!pModel)
//        return;
//    // ------------------------
//    QModelIndex indexAtRightClick = pTableView->indexAt(pos);
//    if (!indexAtRightClick.isValid())
//        return;
//    // I can't figure out how to ADD to the selection without UNSELECTING everything else.
//    // The Qt docs indicate that the below options should do that -- but it doesn't work.
//    // So this is commented out since it was deselecting everything.
//    //pTableView->selectionModel()->select( indexAtRightClick, QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows );
//    // ------------------------
//    QModelIndex sourceIndexAtRightClick = pProxyModel->mapToSource(indexAtRightClick);
//    const int nRow = sourceIndexAtRightClick.row();
//    // ----------------------------------
//    popupMenu_.reset(new QMenu(this));
//    pActionOpenNewWindow = popupMenu_->addAction(tr("View instrument"));
//    pActionReply = popupMenu_->addAction(tr("Reply"));
//    pActionForward = popupMenu_->addAction(tr("Forward"));
//    popupMenu_->addSeparator();
//    pActionDelete = popupMenu_->addAction(tr("Delete"));
//    popupMenu_->addSeparator();
//    pActionMarkRead = popupMenu_->addAction(tr("Mark as read"));
//    pActionMarkUnread = popupMenu_->addAction(tr("Mark as unread"));
//    // ----------------------------------
//    pActionViewContact         = nullptr;
//    pActionCreateContact       = nullptr;
//    pActionExistingContact     = nullptr;
//    pActionAcceptIncoming      = nullptr;
//    pActionCancelOutgoing      = nullptr;
//    pActionDiscardIncoming     = nullptr;
//    pActionDownloadCredentials = nullptr;
//    // ----------------------------------
//    int nContactId = 0;

//    QString qstrSenderNymId;
//    QString qstrSenderAddr;
//    QString qstrRecipientNymId;
//    QString qstrRecipientAddr;
//    QString qstrNotaryId;
//    QString qstrMethodType;
////  QString qstrSubject;

//    int nSenderContactByNym     = 0;
//    int nSenderContactByAddr    = 0;
//    int nRecipientContactByNym  = 0;
//    int nRecipientContactByAddr = 0;

//    ModelPayments::PaymentFlags flags = ModelPayments::NoFlags;
//    // ----------------------------------------------
//    // Look at the data for indexAtRightClick and see if I have a contact already in the
//    // address book. If so, add the "View Contact" option to the menu. But if not, add the
//    // "Create Contact" and "Add to Existing Contact" options to the menu instead.
//    //
//    // UPDATE: I've now also added similar functionality, for other actions specific
//    // to certain payment records, based on their flags. (Pay this invoice, deposit
//    // this cash, etc.)
//    //
//    if (nRow >= 0)
//    {
//        QModelIndex indexSenderNym     = pModel->index(nRow, AGRMT_RECEIPT_COL_SENDER_NYM);
//        QModelIndex indexSenderAddr    = pModel->index(nRow, AGRMT_RECEIPT_COL_SENDER_ADDR);
//        QModelIndex indexRecipientNym  = pModel->index(nRow, AGRMT_RECEIPT_COL_RECIP_NYM);
//        QModelIndex indexRecipientAddr = pModel->index(nRow, AGRMT_RECEIPT_COL_RECIP_ADDR);
//        QModelIndex indexNotaryId      = pModel->index(nRow, AGRMT_RECEIPT_COL_NOTARY_ID);
//        QModelIndex indexMethodType    = pModel->index(nRow, AGRMT_RECEIPT_COL_METHOD_TYPE);
//        QModelIndex indexFlags         = pModel->index(nRow, AGRMT_RECEIPT_COL_FLAGS);
////      QModelIndex indexSubject       = pModel->index(nRow, AGRMT_RECEIPT_COL_MEMO);

//        QVariant varSenderNym     = pModel->rawData(indexSenderNym);
//        QVariant varSenderAddr    = pModel->rawData(indexSenderAddr);
//        QVariant varRecipientNym  = pModel->rawData(indexRecipientNym);
//        QVariant varRecipientAddr = pModel->rawData(indexRecipientAddr);
//        QVariant varNotaryId      = pModel->rawData(indexNotaryId);
//        QVariant varMethodType    = pModel->rawData(indexMethodType);
//        QVariant varFlags         = pModel->rawData(indexFlags);
////      QVariant varSubject       = pModel->rawData(indexSubject);

//        qint64 lFlags      = varFlags        .isValid() ? varFlags        .toLongLong() : 0;
//        qstrSenderNymId    = varSenderNym    .isValid() ? varSenderNym    .toString()   : QString("");
//        qstrSenderAddr     = varSenderAddr   .isValid() ? varSenderAddr   .toString()   : QString("");
//        qstrRecipientNymId = varRecipientNym .isValid() ? varRecipientNym .toString()   : QString("");
//        qstrRecipientAddr  = varRecipientAddr.isValid() ? varRecipientAddr.toString()   : QString("");
//        qstrNotaryId       = varNotaryId     .isValid() ? varNotaryId     .toString()   : QString("");
//        qstrMethodType     = varMethodType   .isValid() ? varMethodType   .toString()   : QString("");
////      qstrSubject        = varSubject      .isValid() ? varSubject      .toString()   : QString("");

//        nSenderContactByNym     = qstrSenderNymId.isEmpty()    ? 0 : MTContactHandler::getInstance()->FindContactIDByNymID(qstrSenderNymId);
//        nSenderContactByAddr    = qstrSenderAddr.isEmpty()     ? 0 : MTContactHandler::getInstance()->GetContactByAddress(qstrSenderAddr);
//        nRecipientContactByNym  = qstrRecipientNymId.isEmpty() ? 0 : MTContactHandler::getInstance()->FindContactIDByNymID(qstrRecipientNymId);
//        nRecipientContactByAddr = qstrRecipientAddr.isEmpty()  ? 0 : MTContactHandler::getInstance()->GetContactByAddress(qstrRecipientAddr);

//        nContactId = (nSenderContactByNym > 0) ? nSenderContactByNym : nSenderContactByAddr;

//        if (nContactId <= 0)
//            nContactId = (nRecipientContactByNym > 0) ? nRecipientContactByNym : nRecipientContactByAddr;

//        flags = ModelPayments::PaymentFlag(static_cast<ModelPayments::PaymentFlag>(lFlags));
//        // -------------------------------
//        popupMenu_->addSeparator();
//        // -------------------------------
//        if (nContactId > 0) // There's a known contact for this payment.
//            pActionViewContact = popupMenu_->addAction(tr("View contact"));
//        else // There is no known contact for this payment.
//        {
//            pActionCreateContact = popupMenu_->addAction(tr("Create new contact"));
//            pActionExistingContact = popupMenu_->addAction(tr("Add to existing contact"));
//        }
//        // -------------------------------
//        popupMenu_->addSeparator();
//        // -------------------------------
//        pActionDownloadCredentials = popupMenu_->addAction(tr("Download credentials"));
//        // -------------------------------
//        popupMenu_->addSeparator();
//        // -------------------------------
//        if ( flags.testFlag(ModelPayments::CanAcceptIncoming))
//        {
//            QString nameString;
//            QString actionString;

//            if ( flags.testFlag(ModelPayments::IsReceipt) )
//            {
//                nameString = tr("Accept this Receipt");
//                actionString = tr("Accepting...");
//                pActionAcceptIncoming = popupMenu_->addAction(nameString);
//            }
//            else if ( flags.testFlag(ModelPayments::IsPaymentPlan) )
//            {
//                nameString = tr("Activate this Payment Plan");
//                actionString = tr("Activating...");
//                pActionAcceptIncoming = popupMenu_->addAction(nameString);
//            }
//            else if ( flags.testFlag(ModelPayments::IsContract) )
//            {
//                nameString = tr("Sign this Smart Contract");
//                actionString = tr("Signing...");
//                pActionAcceptIncoming = popupMenu_->addAction(nameString);
//            }
//        }

//        if (flags.testFlag(ModelPayments::CanCancelOutgoing))
//        {
//            QString cancelString;
//            QString actionString = tr("Canceling...");
////          QString msg = tr("Cancellation Failed. Perhaps recipient had already accepted it?");

//            if (flags.testFlag(ModelPayments::IsPaymentPlan)) {
//                cancelString = tr("Cancel this Payment Plan");
//                pActionCancelOutgoing = popupMenu_->addAction(cancelString);
//            }
//            else if (flags.testFlag(ModelPayments::IsContract)) {
//                cancelString = tr("Cancel this Smart Contract");
//                pActionCancelOutgoing = popupMenu_->addAction(cancelString);
//            }
//        }

//        if (flags.testFlag(ModelPayments::CanDiscardIncoming))
//        {
//            QString discardString;

//            if (flags.testFlag(ModelPayments::IsPaymentPlan))
//                discardString = tr("Discard this Payment Plan");
//            else if (flags.testFlag(ModelPayments::IsContract))
//                discardString = tr("Discard this Smart Contract");

//            pActionDiscardIncoming = popupMenu_->addAction(discardString);
//        }
//    }
//    // --------------------------------------------------
//    QPoint globalPos = pTableView->mapToGlobal(pos);
//    const QAction* selectedAction = popupMenu_->exec(globalPos); // Here we popup the menu, and get the user's click.
//    if (nullptr == selectedAction)
//        return;
//    // ----------------------------------
//    if (selectedAction == pActionAcceptIncoming) // Only approves the current agreement receipt.
//    {
//        pTableView->setCurrentIndex(indexAtRightClick);
//        AcceptIncomingReceipt(pModel, pProxyModel, nRow, pTableView);
//        return;
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionCancelOutgoing) // Only cancels the current agreement receipt.
//    {
//        pTableView->setCurrentIndex(indexAtRightClick);
//        CancelOutgoingReceipt(pModel, pProxyModel, nRow, pTableView);
//        return;
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionDiscardIncoming) // Only discards the current agreement receipt.
//    {
//        pTableView->setCurrentIndex(indexAtRightClick);
//        DiscardIncomingReceipt(pModel, pProxyModel, nRow, pTableView);
//        return;
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionReply) // Only replies to the current agreement receipt.
//    {
//        pTableView->setCurrentIndex(indexAtRightClick);
//        //on_toolButtonReply_clicked();
//        return;
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionForward) // Only fowards the current agreement receipt.
//    {
//        pTableView->setCurrentIndex(indexAtRightClick);
//        //on_toolButtonForward_clicked();
//        return;
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionDelete) // May delete many agreement receipts.
//    {
//        on_toolButtonDelete_clicked();
//        return;
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionOpenNewWindow) // May open many agreement receipts.
//    {
//        pTableView->setCurrentIndex(indexAtRightClick);

//        if (pTableView == ui->tableViewReceived)
//            on_tableViewReceived_doubleClicked(indexAtRightClick); // just one for now. baby steps!
//        else if (pTableView == ui->tableViewSent)
//            on_tableViewSent_doubleClicked(indexAtRightClick); // just one for now. baby steps!
//        return;
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionMarkRead) // May mark many agreement receipts.
//    {
//        if (!pTableView->selectionModel()->hasSelection())
//            return;
//        // ----------------------------------------------
//        QItemSelection selection( pTableView->selectionModel()->selection() );
//        QList<int> rows;
//        foreach( const QModelIndex & index, selection.indexes() )
//        {
//            if (rows.indexOf(index.row()) != (-1)) // This row is already on the list, so skip it.
//                continue;
//            rows.append(index.row());
//            // -----------------------
//            QModelIndex sourceIndex = pProxyModel->mapToSource(index);
//            QModelIndex sourceIndexHaveRead = pModel->sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_HAVE_READ, sourceIndex);
//            // --------------------------------
//            if (sourceIndexHaveRead.isValid())
//                listReceiptRecordsToMarkAsRead_.append(sourceIndexHaveRead);
//        }
//        if (listReceiptRecordsToMarkAsRead_.count() > 0)
//            QTimer::singleShot(0, this, SLOT(on_MarkReceiptsAsRead_timer()));
//        return;
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionMarkUnread) // May mark many agreement receipts.
//    {
//        if (!pTableView->selectionModel()->hasSelection())
//            return;
//        // ----------------------------------------------
//        QItemSelection selection( pTableView->selectionModel()->selection() );
//        QList<int> rows;
//        foreach( const QModelIndex & index, selection.indexes() )
//        {
//            if (rows.indexOf(index.row()) != (-1)) // This row is already on the list, so skip it.
//                continue;
//            rows.append(index.row());
//            // -----------------------
//            QModelIndex sourceIndex = pProxyModel->mapToSource(index);
//            QModelIndex sourceIndexHaveRead = pModel->sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_HAVE_READ, sourceIndex);
//            // --------------------------------
//            if (sourceIndexHaveRead.isValid())
//                listReceiptRecordsToMarkAsUnread_.append(sourceIndexHaveRead);
//        }
//        if (listReceiptRecordsToMarkAsUnread_.count() > 0)
//            QTimer::singleShot(0, this, SLOT(on_MarkReceiptsAsUnread_timer()));
//        return;
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionViewContact)
//    {
//        pTableView->setCurrentIndex(indexAtRightClick);

//        if (nContactId > 0)
//        {
//            QString qstrContactId = QString::number(nContactId);
//            emit showContact(qstrContactId);
//        }
//        return;
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionCreateContact)
//    {
//        pTableView->setCurrentIndex(indexAtRightClick);

//        MTGetStringDialog nameDlg(this, tr("Enter a name for the new contact"));

//        if (QDialog::Accepted != nameDlg.exec())
//            return;
//        // --------------------------------------
//        QString strNewContactName = nameDlg.GetOutputString();
//        // --------------------------------------------------
//        // NOTE:
//        // if nSenderContactByNym > 0, then the sender Nym already has a contact.
//        // else if nSenderContactByNym == 0 but qstrSenderNymId exists, that means it
//        // contains a NymID that could be added to an existing contact, or used to
//        // create a new contact. (And the same is true for the Sender Address.)
//        //
//        // (And the same is also true for the recipient nymID and address.)
//        //
//        if ((0 == nSenderContactByNym) && !qstrSenderNymId.isEmpty())
//            nContactId = MTContactHandler::getInstance()->CreateContactBasedOnNym(qstrSenderNymId, qstrNotaryId);
//        else if ((0 == nSenderContactByAddr) && !qstrSenderAddr.isEmpty())
//            nContactId = MTContactHandler::getInstance()->CreateContactBasedOnAddress(qstrSenderAddr, qstrMethodType);
//        else if ((0 == nRecipientContactByNym) && !qstrRecipientNymId.isEmpty())
//            nContactId = MTContactHandler::getInstance()->CreateContactBasedOnNym(qstrRecipientNymId, qstrNotaryId);
//        else if ((0 == nRecipientContactByAddr) && !qstrRecipientAddr.isEmpty())
//            nContactId = MTContactHandler::getInstance()->CreateContactBasedOnAddress(qstrRecipientAddr, qstrMethodType);
//        // -----------------------------------------------------
//        if (nContactId > 0)
//        {
//            MTContactHandler::getInstance()->SetContactName(nContactId, strNewContactName);
//            // ---------------------------------
//            QString qstrContactID = QString("%1").arg(nContactId);
//            emit showContactAndRefreshHome(qstrContactID);
//        }
//        return;
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionDownloadCredentials)
//    {
//        pTableView->setCurrentIndex(indexAtRightClick);

//        const bool bHaveContact = (nContactId > 0);
//        mapIDName mapNymIds;

//        if (bHaveContact)
//        {
//            MTContactHandler::getInstance()->GetNyms(mapNymIds, nContactId);

//            // Check to see if there is more than one Nym for this contact.
//            // TODO: If so, get the user to select one of the Nyms, or give him the
//            // option to do them all.
//            // (Until then, we're just going to do them all.)
//        }
//        // ---------------------------------------------------
//        QString qstrAddress, qstrNymId;

//        if      (!qstrSenderNymId.isEmpty())    qstrNymId   = qstrSenderNymId;
//        else if (!qstrRecipientNymId.isEmpty()) qstrNymId   = qstrRecipientNymId;
//        // ---------------------------------------------------
//        if      (!qstrSenderAddr.isEmpty())     qstrAddress = qstrSenderAddr;
//        else if (!qstrRecipientAddr.isEmpty())  qstrAddress = qstrRecipientAddr;
//        // ---------------------------------------------------
//        // Might not have a contact. Even if we did, he might not have any NymIds.
//        // Here, if there are no known NymIds, but there's one on the message,
//        // then we add it to the map.
//        if ( (0 == mapNymIds.size()) && (qstrNymId.size() > 0) )
//        {
//            mapNymIds.insert(qstrNymId, QString("Name not used here"));
//        }
//        // ---------------------------------------------------
//        // By this point if there's still no Nym, we need to take the address,
//        // and then loop through all the claims in the database to see if there's
//        // a Nym associated with that Bitmessage address via his claims.
//        //
//        if ( (0 == mapNymIds.size()) && (qstrAddress.size() > 0) )
//        {
//            qstrNymId = MTContactHandler::getInstance()->GetNymByAddress(qstrAddress);

//            if (qstrNymId.isEmpty())
//                qstrNymId = MTContactHandler::getInstance()->getNymIdFromClaimsByBtMsg(qstrAddress);

//            if (qstrNymId.size() > 0)
//            {
//                mapNymIds.insert(qstrNymId, QString("Name not used here"));
//            }
//        }
//        // ---------------------------------------------------
//        if (0 == mapNymIds.size())
//        {
//            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), tr("Unable to find a NymId for this message. (Unable to download credentials without Id.)"));
//            qDebug() << "Unable to find a NymId for this message. (Unable to download credentials without Id.)";
//            return;
//        }
//        // Below this point we're guaranteed that there's at least one NymID.
//        // ---------------------------------------------------
//        int nFound = 0;
//        for (mapIDName::iterator
//             it_nyms  = mapNymIds.begin();
//             it_nyms != mapNymIds.end();
//             ++it_nyms)
//        {
//            nFound++;
//            emit needToCheckNym("", it_nyms.key(), qstrNotaryId);
//        }
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionExistingContact)
//    {
//        pTableView->setCurrentIndex(indexAtRightClick);

//        // This should never happen since we wouldn't even have gotten this menu option
//        // in the first place, unless contact ID had been 0.
//        if (nContactId > 0)
//            return;

//        // (And that means no contact was found for ANY of the Nym IDs or Addresses on this payment.)
//        // That means we can add the first one we find (which will probably be the only one as well.)
//        // Because I'll EITHER have a SenderNymID OR SenderAddress,
//        // ...OR I'll have a RecipientNymID OR RecipientAddress.
//        // Thus, only one of the four IDs/Addresses will actually be found.
//        // Therefore I don't care which one I find first:
//        //
//        QString qstrAddress, qstrNymId;

//        if      (!qstrSenderNymId.isEmpty())    qstrNymId   = qstrSenderNymId;
//        else if (!qstrSenderAddr.isEmpty())     qstrAddress = qstrSenderAddr;
//        else if (!qstrRecipientNymId.isEmpty()) qstrNymId   = qstrRecipientNymId;
//        else if (!qstrRecipientAddr.isEmpty())  qstrAddress = qstrRecipientAddr;
//        // ---------------------------------------------------
//        if (qstrNymId.isEmpty() && qstrAddress.isEmpty()) // Should never happen.
//            return;
//        // Below this point we're guaranteed that there's either a NymID or an Address.
//        // ---------------------------------------------------
//        if (!qstrNymId.isEmpty() && (MTContactHandler::getInstance()->FindContactIDByNymID(qstrNymId) > 0))
//        {
//            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
//                                 tr("Strange: NymID %1 already belongs to an existing contact.").arg(qstrNymId));
//            return;
//        }
//        // ---------------------------------------------------
//        if (!qstrAddress.isEmpty() && MTContactHandler::getInstance()->GetContactByAddress(qstrAddress) > 0)
//        {
//            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
//                                 tr("Strange: Address %1 already belongs to an existing contact.").arg(qstrAddress));
//            return;
//        }
//        // --------------------------------------------------------------------
//        // Pop up a Contact selection box. The user chooses an existing contact.
//        // If OK (vs Cancel) then add the Nym / Acct to the existing contact selected.
//        //
//        DlgChooser theChooser(this);
//        // -----------------------------------------------
//        mapIDName & the_map = theChooser.m_map;
//        MTContactHandler::getInstance()->GetContacts(the_map);
//        // -----------------------------------------------
//        theChooser.setWindowTitle(tr("Choose an Existing Contact"));
//        if (theChooser.exec() != QDialog::Accepted)
//            return;
//        // -----------------------------------------------
//        QString strContactID = theChooser.GetCurrentID();
//        nContactId = strContactID.isEmpty() ? 0 : strContactID.toInt();

//        if (nContactId > 0)
//        {
//            if (!qstrNymId.isEmpty()) // We're adding this NymID to the contact.
//            {
//                if (!MTContactHandler::getInstance()->AddNymToExistingContact(nContactId, qstrNymId))
//                {
//                    QString strContactName(MTContactHandler::getInstance()->GetContactName(nContactId));
//                    QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), QString("Failed while trying to add NymID %1 to existing contact '%2' with contact ID: %3").
//                                         arg(qstrNymId).arg(strContactName).arg(nContactId));
//                    return;
//                }
//                if (!qstrNotaryId.isEmpty())
//                    MTContactHandler::getInstance()->NotifyOfNymServerPair(qstrNymId, qstrNotaryId);
//            }
//            else if (!qstrAddress.isEmpty()) // We're adding this Address to the contact.
//            {
//                if (!MTContactHandler::getInstance()->AddMsgAddressToContact(nContactId, qstrMethodType, qstrAddress))
//                {
//                    QString strContactName(MTContactHandler::getInstance()->GetContactName(nContactId));
//                    QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), QString("Failed while trying to add Address %1 to existing contact '%2' with contact ID: %3").
//                                         arg(qstrAddress).arg(strContactName).arg(nContactId));
//                    return;
//                }
//            }
//            // ---------------------------------
//            // Display the normal contacts dialog, with the new contact
//            // being the one selected.
//            //
//            QString qstrContactID = QString("%1").arg(nContactId);
//            emit showContactAndRefreshHome(qstrContactID);
//            // ---------------------------------
//        } // nContactID > 0
//    }
}



//#define AGRMT_SOURCE_COL_AGRMT_ID 0
//#define AGRMT_SOURCE_COL_HAVE_READ 1
//#define AGRMT_SOURCE_COL_TXN_ID_DISPLAY 2
//#define AGRMT_SOURCE_COL_NOTARY_ID 3
//#define AGRMT_SOURCE_COL_CONTRACT_ID 4
//#define AGRMT_SOURCE_COL_NEWEST_RECEIPT_ID 5
//#define AGRMT_SOURCE_COL_NEWEST_KNOWN_STATE 6
//#define AGRMT_SOURCE_COL_TIMESTAMP 7
//#define AGRMT_SOURCE_COL_MEMO 8
//#define AGRMT_SOURCE_COL_FOLDER 9
//
//#define AGRMT_RECEIPT_COL_AGRMT_RECEIPT_KEY 0
//#define AGRMT_RECEIPT_COL_AGRMT_ID 1
//#define AGRMT_RECEIPT_COL_RECEIPT_ID 2
//#define AGRMT_RECEIPT_COL_TIMESTAMP 3
//#define AGRMT_RECEIPT_COL_HAVE_READ 4
//#define AGRMT_RECEIPT_COL_TXN_ID_DISPLAY 5
//#define AGRMT_RECEIPT_COL_EVENT_ID 6
//#define AGRMT_RECEIPT_COL_MEMO 7
//#define AGRMT_RECEIPT_COL_MY_ASSET_TYPE 8
//#define AGRMT_RECEIPT_COL_MY_NYM 9
//#define AGRMT_RECEIPT_COL_MY_ACCT 10
//#define AGRMT_RECEIPT_COL_MY_ADDR 11
//#define AGRMT_RECEIPT_COL_SENDER_NYM 12
//#define AGRMT_RECEIPT_COL_SENDER_ACCT 13
//#define AGRMT_RECEIPT_COL_SENDER_ADDR 14
//#define AGRMT_RECEIPT_COL_RECIP_NYM 15
//#define AGRMT_RECEIPT_COL_RECIP_ACCT 16
//#define AGRMT_RECEIPT_COL_RECIP_ADDR 17
//#define AGRMT_RECEIPT_COL_AMOUNT 18
//#define AGRMT_RECEIPT_COL_FOLDER 19
//#define AGRMT_RECEIPT_COL_METHOD_TYPE 20
//#define AGRMT_RECEIPT_COL_METHOD_TYPE_DISP 21
//#define AGRMT_RECEIPT_COL_NOTARY_ID 22
//#define AGRMT_RECEIPT_COL_DESCRIPTION 23
//#define AGRMT_RECEIPT_COL_RECORD_NAME 24
//#define AGRMT_RECEIPT_COL_INSTRUMENT_TYPE 25
//#define AGRMT_RECEIPT_COL_FLAGS 26

// ----------------------------------------------------------
//int  GetOrCreateLiveAgreementId(const int64_t transNumDisplay, const QString & notaryID, const QString & qstrEncodedMemo, const int nFolder); // returns nAgreementId
//bool UpdateLiveAgreementRecord(const int nAgreementId, const int64_t nNewestReceiptNum, const int nNewestKnownState, const int64_t timestamp);
// ------------------------------
// Why do we apparently have 2 receipt IDs? (ReceiptNum and AgreementReceiptKey)
// The former comes from OT itself, and though it MAY be unique to OT (presuming the server is honest)
// it's not unique across notaries in any case. But it's the number we'll need when dealing with OT.
// Whereas the latter is an autonumber created here locally, in Moneychanger, which ensures that it's
// unique to the local Moneychanger DB. So they are just used in slightly different ways and we ended up
// needing both of them.
//int  DoesAgreementReceiptAlreadyExist(const int nAgreementId, const int64_t receiptNum, const QString & qstrNymId,
//                                      const int64_t transNumDisplay=0); // returns nAgreementReceiptKey
// ------------------------------
//bool CreateAgreementReceiptBody(const int nAgreementReceiptKey, QString & qstrReceiptBody); // When this is called, we already know the specific receipt is being added for the first time.
//bool DeleteAgreementReceiptBody(const int nID); // nID is nAgreementReceiptKey
//QString GetAgreementReceiptBody(const int nID); // nID is nAgreementReceiptKey


void Agreements::on_tableViewRecurring_doubleClicked(const QModelIndex &index)
{
    tableViewAgreementsDoubleClicked(index, &(*pProxyModelRecurring_));
}

void Agreements::on_tableViewSmartContracts_doubleClicked(const QModelIndex &index)
{
    tableViewAgreementsDoubleClicked(index, &(*pProxyModelSmartContracts_));
}

void Agreements::on_tableViewReceived_doubleClicked(const QModelIndex &index)
{
    tableViewReceiptsDoubleClicked(index, &(*pReceiptProxyModelInbox_));
}

void Agreements::on_tableViewSent_doubleClicked(const QModelIndex &index)
{
    tableViewReceiptsDoubleClicked(index, &(*pReceiptProxyModelOutbox_));
}

void Agreements::tableViewAgreementsDoubleClicked(const QModelIndex &index, AgreementsProxyModel * pProxyModel)
{
    // todo
}

void Agreements::tableViewReceiptsDoubleClicked(const QModelIndex &index, AgreementReceiptsProxyModel * pProxyModel)
{
    QPointer<ModelAgreementReceipts> pModel = DBHandler::getInstance()->getAgreementReceiptModel();

    if (!pModel)
        return;

    if (!index.isValid())
        return;

    QModelIndex sourceIndex = pProxyModel->mapToSource(index);

    if (!sourceIndex.isValid())
        return;
    // -------------------------------
    QModelIndex agreementReceiptKeyIndex = pModel->sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_AGRMT_RECEIPT_KEY, sourceIndex);
    QModelIndex subjectIndex = pModel->sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_MEMO, sourceIndex);

    QVariant qvarAgreementReceiptKey = pModel->data(agreementReceiptKeyIndex);
    QVariant qvarSubject = pModel->data(subjectIndex);

    int nAgreementReceiptKey = qvarAgreementReceiptKey.isValid() ? qvarAgreementReceiptKey.toInt() : 0;
    QString qstrSubject = qvarSubject.isValid() ? qvarSubject.toString() : "";
    // -------------------------------
    QString qstrReceipt, qstrType, qstrSubtitle;
    // --------------------------------------------------
    if (nAgreementReceiptKey > 0)
        qstrReceipt = MTContactHandler::getInstance()->GetAgreementReceiptBody(nAgreementReceiptKey);
    // --------------------------------------------------
    QModelIndex myNymIndex        = pModel->sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_MY_NYM, sourceIndex);
    QModelIndex senderNymIndex    = pModel->sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_SENDER_NYM, sourceIndex);
    QModelIndex recipientNymIndex = pModel->sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_RECIP_NYM, sourceIndex);

    QModelIndex myNymProxyIndex        = pProxyModel->mapFromSource(myNymIndex);
    QModelIndex senderNymProxyIndex    = pProxyModel->mapFromSource(senderNymIndex);
    QModelIndex recipientNymProxyIndex = pProxyModel->mapFromSource(recipientNymIndex);

    QVariant qvarMyNymName        = myNymProxyIndex.isValid()        ? pProxyModel->data(myNymProxyIndex) : QString("");
    QVariant qvarSenderNymName    = senderNymProxyIndex.isValid()    ? pProxyModel->data(senderNymProxyIndex) : QString("");
    QVariant qvarRecipientNymName = recipientNymProxyIndex.isValid() ? pProxyModel->data(recipientNymProxyIndex) : QString("");

    QString qstrMyNymName        = qvarMyNymName.isValid() ? qvarMyNymName.toString() : "";
    QString qstrSenderNymName    = qvarSenderNymName.isValid() ? qvarSenderNymName.toString() : "";
    QString qstrRecipientNymName = qvarRecipientNymName.isValid() ? qvarRecipientNymName.toString() : "";

    if (!qstrSenderNymName.isEmpty())
    {
        qstrType = QString("%1: %2").arg(tr("To")).arg(qstrMyNymName);
        qstrSubtitle = QString("%1: %2").arg(tr("From")).arg(qstrSenderNymName);
    }
    else if (!qstrRecipientNymName.isEmpty())
    {
        qstrType = QString("%1: %2").arg(tr("To")).arg(qstrRecipientNymName);
        qstrSubtitle = QString("%1: %2").arg(tr("From")).arg(qstrMyNymName);
    }
    else
    {
        qstrType = QString("Receipt:");
        qstrSubtitle = QString(" ");
    }
    // -----------
    // Pop up the result dialog.
    //
    DlgExportedToPass dlgExported(this, qstrReceipt, qstrType,
                                  qstrSubtitle, false);
    if (qstrSubject.isEmpty())
        dlgExported.setWindowTitle(QString("%1").arg(tr("Receipt")));
    else
        dlgExported.setWindowTitle(QString("%1: %2").arg(tr("Memo")).arg(qstrSubject));
    dlgExported.exec();
}



void Agreements::killSelectedAgreement()
{
    // Kill
    // todo

    // resume now

//    enum class agreement_status { ERROR = 0,
//                                  OUTGOING = 1,
//                                  INCOMING = 2,
//                                  ACTIVATED = 3,
//                                  PAID = 4,
//                                  PAYMENT_FAILED = 5,
//                                  FAILED_ACTIVATING = 6,
//                                  CANCELED = 7,
//                                  EXPIRED = 8,
//                                  NO_LONGER_ACTIVE = 9,
//                                  KILLED = 10
//                                };

    // -----------------------------------------------
    if (    (nullptr == pCurrentReceiptTableView_)
         || (nullptr == pCurrentReceiptProxyModel_)
         || (nullptr == pCurrentAgreementTableView_)
         || (nullptr == pCurrentAgreementProxyModel_)
         ) return;
    // -----------------------------------------------
    if (!pCurrentAgreementTableView_->selectionModel()->hasSelection())
        return;
    // ----------------------------------------------
    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(this, tr(MONEYCHANGER_APP_NAME), QString("%1<br/><br/>%2").arg(tr("Are you sure you want to kill all selected active agreement(s)?")).
                                  arg(tr("WARNING: This is not reversible!")),
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply != QMessageBox::Yes)
        return;
    // ----------------------------------------------
    QPointer<ModelAgreements> pAgreementModel = DBHandler::getInstance()->getAgreementModel();

    if (!pAgreementModel)
        return;
    // ----------------------------------------------
    QItemSelection selection( pCurrentAgreementTableView_->selectionModel()->selection() );

    int nFirstProxyRowKilled = -1;
    int nLastProxyRowKilled  = -1;
    int nCountRowsKilled     = 0;

    QList<LiveAgreement> rows;
    foreach( const QModelIndex & index, selection.indexes() )
    {
        QModelIndex sourceIndex = pCurrentAgreementProxyModel_->mapToSource(index);

        QModelIndex indexAgreementId     = pAgreementModel->sibling(sourceIndex.row(), AGRMT_SOURCE_COL_AGRMT_ID, sourceIndex);
        QModelIndex indexTxnIdDisplay    = pAgreementModel->sibling(sourceIndex.row(), AGRMT_SOURCE_COL_TXN_ID_DISPLAY, sourceIndex);
        QModelIndex indexNotaryId        = pAgreementModel->sibling(sourceIndex.row(), AGRMT_SOURCE_COL_NOTARY_ID, sourceIndex);
        QModelIndex indexNewestState     = pAgreementModel->sibling(sourceIndex.row(), AGRMT_SOURCE_COL_NEWEST_KNOWN_STATE, sourceIndex);

        QVariant varAgreementId     = pAgreementModel->rawData(indexAgreementId);
        QVariant varTxnIdDisplay    = pAgreementModel->rawData(indexTxnIdDisplay);
        QVariant varNotaryId        = pAgreementModel->rawData(indexNotaryId);
        QVariant varNewestState     = pAgreementModel->rawData(indexNewestState);

        int     nAgreementId     = varAgreementId    .isValid() ? varAgreementId    .toInt()      : 0;
        int64_t lTxnIdDisplay    = varTxnIdDisplay   .isValid() ? varTxnIdDisplay   .toLongLong() : 0;
        QString qstrNotaryId     = varNotaryId       .isValid() ? varNotaryId       .toString()   : QString("");
        int     nNewestState     = varNewestState    .isValid() ? varNewestState    .toInt()      : 0;

        // Only append if the agreement is believed to be alive, etc.
        //
        if (    (nNewestState >= static_cast<int>(agreement_status::ACTIVATED))
             && (nNewestState <  static_cast<int>(agreement_status::FAILED_ACTIVATING))
             && (lTxnIdDisplay > 0)
             && (!qstrNotaryId.isEmpty())
             && (nAgreementId > 0) )
        {
            const std::string str_notary_id = qstrNotaryId.toStdString();
            const int nSourceRow = sourceIndex.row();

            LiveAgreement the_data = //source model row, agreement_id, txnIdDisplay, notaryId, newestState
                std::make_tuple//<int, int, int64_t, std::string, int>
                    (nSourceRow, nAgreementId, lTxnIdDisplay, str_notary_id, nNewestState);

            rows.append( the_data );
            // --------------------------------
            nLastProxyRowKilled = index.row();
            if ((-1) == nFirstProxyRowKilled)
                nFirstProxyRowKilled = index.row();
        }
    }
    // --------------------------------------------------------
    // By this point, rows is a QList containing 0 or more tuples
    // of type LiveAgreement.
    // If it's empty, we can return now.


    for(int ii = 0; ii < rows.count(); ii++) {
       auto & current = rows[ii];

       // Todo: call the opentxs API function for killing a live agreement.
       // in OT_ME

       nCountRowsKilled++;
    }


/*

    // resume now


    // TODO: This is for killing, not deleting.
    // So we don't want to delete any receipts out of the database.
    // Rather, we just want to loop through the agreement IDs and for each
    // one, send a "kill cron item" message via opentxs.

    // So basically we just want to compile a list of the agreement IDs,
    // and pass that to a new "kill" function, which possibly has to look up
    // the appropriate transaction number and then send the kill message
    // to the appropriate notary.


    for(int ii = rows.count() - 1; ii >= 0; ii -= 1 ) {
       int current = rows[ii];
       if( current != prev ) {
           bRemoved = true;
           QModelIndex sourceIndexAgreementReceiptKey = pAgreementModel->index(current, AGRMT_RECEIPT_COL_AGRMT_RECEIPT_KEY);
           if (sourceIndexAgreementReceiptKey.isValid())
               receipt_keys.append(pAgreementModel->data(sourceIndexAgreementReceiptKey).toInt());
           pAgreementModel->removeRows( current, 1 );
           prev = current;
           nCountRowsKilled++;
       }
    }

    if (bRemoved)
    {
        if (pAgreementModel->submitAll())
        {
            pAgreementModel->database().commit();
            // ------------------------
            // Now we just deleted some receipts; let's delete also the corresponding
            // receipt contents. (We saved the deleted IDs for this purpose.)
            //
            for (int ii = 0; ii < receipt_keys.count(); ++ii)
            {
                const int nReceiptKey = receipt_keys[ii];

                if (nReceiptKey > 0)
                    if (!MTContactHandler::getInstance()->DeleteAgreementReceiptBody(nReceiptKey))
                        qDebug() << "Agreements::on_toolButtonDelete_clicked: "
                                    "Failed trying to delete receipt body with agreement_receipt_key: " << nReceiptKey << "\n";
            }
            // ------------------------
            // We just deleted the selected rows.
            // So now we need to choose another row to select.

            int nRowToSelect = -1;

            if ((nFirstProxyRowKilled >= 0) && (nFirstProxyRowKilled < pCurrentAgreementProxyModel_->rowCount()))
                nRowToSelect = nFirstProxyRowKilled;
            else if (0 == nFirstProxyRowKilled)
                nRowToSelect = 0;
            else if (nFirstProxyRowKilled > 0)
                nRowToSelect = pCurrentAgreementProxyModel_->rowCount() - 1;
            else
                nRowToSelect = 0;

            if ((pCurrentAgreementProxyModel_->rowCount() > 0) && (nRowToSelect >= 0) &&
                    (nRowToSelect < pCurrentAgreementProxyModel_->rowCount()))
            {
                QModelIndex previous = pCurrentAgreementTableView_->currentIndex();
                pCurrentAgreementTableView_->blockSignals(true);
                pCurrentAgreementTableView_->selectRow(nRowToSelect);
                pCurrentAgreementTableView_->blockSignals(false);

                if (pCurrentAgreementTableView_ == ui->tableViewRecurring)
                    on_tableViewRecurringSelectionModel_currentRowChanged(pCurrentAgreementTableView_->currentIndex(), previous);
                else
                    on_tableViewSmartContractSelectionModel_currentRowChanged(pCurrentAgreementTableView_->currentIndex(), previous);
            }
        }
        else
        {
            pAgreementModel->database().rollback();
            qDebug() << "Database Write Error" <<
                       "The database reported an error: " <<
                       pAgreementModel->lastError().text();
        }
    }

*/
}

void Agreements::on_toolButtonDelete_clicked()
{
    if ( (nullptr != pCurrentReceiptTableView_) &&
         (nullptr != pCurrentReceiptProxyModel_) )
    {
        if (!pCurrentReceiptTableView_->selectionModel()->hasSelection())
            return;

        // TODO: Add some code here to see if the user is trying to delete
        // the agreement itself, instead of just one or some of its receipts.

        // ----------------------------------------------
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, tr(MONEYCHANGER_APP_NAME), QString("%1<br/><br/>%2").arg(tr("Are you sure you want to delete these receipts?")).
                                      arg(tr("WARNING: This is not reversible!")),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply != QMessageBox::Yes)
            return;
        // ----------------------------------------------
        QPointer<ModelAgreementReceipts> pModel = DBHandler::getInstance()->getAgreementReceiptModel();

        if (pModel)
        {
            QItemSelection selection( pCurrentReceiptTableView_->selectionModel()->selection() );

            int nFirstProxyRowRemoved = -1;
            int nLastProxyRowRemoved  = -1;
            int nCountRowsRemoved     = 0;

            QList<int> rows, receipt_keys;
            foreach( const QModelIndex & index, selection.indexes() ) {
                QModelIndex sourceIndex = pCurrentReceiptProxyModel_->mapToSource(index);
                rows.append( sourceIndex.row() );
                // --------------------------------
                nLastProxyRowRemoved = index.row();
                if ((-1) == nFirstProxyRowRemoved)
                    nFirstProxyRowRemoved = index.row();
            }

            qSort( rows );

            bool bRemoved = false;

            int prev = -1;

            for(int ii = rows.count() - 1; ii >= 0; ii -= 1 ) {
               int current = rows[ii];
               if( current != prev ) {
                   bRemoved = true;
                   QModelIndex sourceIndexAgreementReceiptKey = pModel->index(current, AGRMT_RECEIPT_COL_AGRMT_RECEIPT_KEY);
                   if (sourceIndexAgreementReceiptKey.isValid())
                       receipt_keys.append(pModel->data(sourceIndexAgreementReceiptKey).toInt());
                   pModel->removeRows( current, 1 );
                   prev = current;
                   nCountRowsRemoved++;
               }
            }

            if (bRemoved)
            {
                if (pModel->submitAll())
                {
                    pModel->database().commit();
                    // ------------------------
                    // Now we just deleted some receipts; let's delete also the corresponding
                    // receipt contents. (We saved the deleted IDs for this purpose.)
                    //
                    for (int ii = 0; ii < receipt_keys.count(); ++ii)
                    {
                        const int nReceiptKey = receipt_keys[ii];

                        if (nReceiptKey > 0)
                            if (!MTContactHandler::getInstance()->DeleteAgreementReceiptBody(nReceiptKey))
                                qDebug() << "Agreements::on_toolButtonDelete_clicked: "
                                            "Failed trying to delete receipt body with agreement_receipt_key: " << nReceiptKey << "\n";
                    }
                    // ------------------------
                    // We just deleted the selected rows.
                    // So now we need to choose another row to select.

                    int nRowToSelect = -1;

                    if ((nFirstProxyRowRemoved >= 0) && (nFirstProxyRowRemoved < pCurrentReceiptProxyModel_->rowCount()))
                        nRowToSelect = nFirstProxyRowRemoved;
                    else if (0 == nFirstProxyRowRemoved)
                        nRowToSelect = 0;
                    else if (nFirstProxyRowRemoved > 0)
                        nRowToSelect = pCurrentReceiptProxyModel_->rowCount() - 1;
                    else
                        nRowToSelect = 0;

                    if ((pCurrentReceiptProxyModel_->rowCount() > 0) && (nRowToSelect >= 0) &&
                            (nRowToSelect < pCurrentReceiptProxyModel_->rowCount()))
                    {
                        QModelIndex previous = pCurrentReceiptTableView_->currentIndex();
                        pCurrentReceiptTableView_->blockSignals(true);
                        pCurrentReceiptTableView_->selectRow(nRowToSelect);
                        pCurrentReceiptTableView_->blockSignals(false);

                        if (pCurrentReceiptTableView_ == ui->tableViewReceived)
                            on_tableViewReceivedSelectionModel_currentRowChanged(pCurrentReceiptTableView_->currentIndex(), previous);
                        else
                            on_tableViewSentSelectionModel_currentRowChanged(pCurrentReceiptTableView_->currentIndex(), previous);
                    }
                }
                else
                {
                    pModel->database().rollback();
                    qDebug() << "Database Write Error" <<
                               "The database reported an error: " <<
                               pModel->lastError().text();
                }
            }
        }
    }
}


void Agreements::on_toolButtonRefresh_clicked()
{
    emit needToDownloadAccountData();
}

void Agreements::onRecordlistPopulated()
{
    RefreshAll();
}

void Agreements::RefreshAll()
{
    RefreshUserBar();
    emit needToRefreshAgreements();
}

void Agreements::onBalancesChanged()
{
    RefreshUserBar();
}

// The balances hasn't necessarily changed.
// (Perhaps the default account was changed.)
//
void Agreements::onNeedToRefreshUserBar()
{
    RefreshUserBar();
}

void Agreements::onNeedToRefreshRecords()
{
    RefreshAll();
}


bool Agreements::eventFilter(QObject *obj, QEvent *event)
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

