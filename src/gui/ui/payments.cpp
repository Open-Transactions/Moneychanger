#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/ui/payments.hpp>
#include <ui_payments.h>

#include <gui/widgets/compose.hpp>
#include <gui/ui/dlgexportedtopass.hpp>
#include <gui/ui/dlgexportedcash.hpp>

#include <gui/widgets/dlgchooser.hpp>
#include <gui/widgets/qrwidget.hpp>
#include <gui/ui/getstringdialog.hpp>

#include <core/moneychanger.hpp>
#include <core/handlers/DBHandler.hpp>
#include <core/handlers/contacthandler.hpp>
#include <core/handlers/modelpayments.hpp>
#include <core/handlers/focuser.h>

#include <opentxs/opentxs.hpp>
#include <opentxs/ui/IssuerItem.hpp>
#include <opentxs/ui/ActivitySummary.hpp>
#include <opentxs/ui/ActivityThread.hpp>
#include <opentxs/ui/ActivityThreadItem.hpp>
#include <opentxs/ui/ActivitySummaryItem.hpp>
#include <opentxs/SharedPimpl.hpp>

#include <QLabel>
#include <QDebug>
#include <QToolButton>
#include <QKeyEvent>
#include <QApplication>
#include <QMessageBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMenu>
#include <QList>
#include <QSqlRecord>
#include <QTimer>
#include <QFrame>

#include <string>
#include <map>

template class opentxs::SharedPimpl<opentxs::ui::IssuerItem>;
template class opentxs::SharedPimpl<opentxs::ui::AccountSummaryItem>;


PMNT_TREE_ITEM Payments::make_tree_item(int nCurrentContact, QString qstrMethodType, QString qstrViaTransport)
{
    return std::make_tuple(nCurrentContact, qstrMethodType.toStdString(), qstrViaTransport.toStdString());
}


void Payments::set_inbox_pmntid_for_tree_item(PMNT_TREE_ITEM & theItem, int nPmntID)
{
    mapOfPmntTreeItems::iterator it = mapCurrentRows_inbox.find(theItem);
    if (mapCurrentRows_inbox.end() != it) // found it.
    {
        mapCurrentRows_inbox.erase(it);
    }
    mapCurrentRows_inbox.insert( std::pair<PMNT_TREE_ITEM, int> (theItem, nPmntID) );
}

void Payments::set_outbox_pmntid_for_tree_item(PMNT_TREE_ITEM & theItem, int nPmntID)
{
    mapOfPmntTreeItems::iterator it = mapCurrentRows_outbox.find(theItem);
    if (mapCurrentRows_outbox.end() != it) // found it.
    {
        mapCurrentRows_outbox.erase(it);
    }
    mapCurrentRows_outbox.insert( std::pair<PMNT_TREE_ITEM, int> (theItem, nPmntID) );
}

int Payments::get_inbox_pmntid_for_tree_item(PMNT_TREE_ITEM & theItem)
{
    mapOfPmntTreeItems::iterator it = mapCurrentRows_inbox.find(theItem);
    if (mapCurrentRows_inbox.end() != it) // found it.
    {
        return it->second;
    }
    return 0;
}

int Payments::get_outbox_pmntid_for_tree_item(PMNT_TREE_ITEM & theItem)
{
    mapOfPmntTreeItems::iterator it = mapCurrentRows_outbox.find(theItem);
    if (mapCurrentRows_outbox.end() != it) // found it.
    {
        return it->second;
    }
    return 0;
}

Payments::Payments(QWidget *parent) :
    QWidget(parent, Qt::Window),
    ui(new Ui::Payments)
{
    ui->setupUi(this);

    this->installEventFilter(this);

    connect(ui->toolButtonPay,      SIGNAL(clicked()), Moneychanger::It(), SLOT(mc_sendfunds_slot()));
    connect(ui->toolButtonContacts, SIGNAL(clicked()), Moneychanger::It(), SLOT(mc_addressbook_slot()));
    connect(this, SIGNAL(needToCheckNym(QString, QString, QString)), Moneychanger::It(), SLOT(onNeedToCheckNym(QString, QString, QString)));

//  if (!Moneychanger::It()->expertMode())
    {
        // To size the splitter properly:
//        QList<int> list;
//        list.append(0);
//        list.append(100);
//        ui->splitter->setSizes(list);
    }
}

static void setup_tableview(QTableView * pView, QAbstractItemModel * pProxyModel)
{
    PaymentsProxyModel * pPmntProxyModel = static_cast<PaymentsProxyModel *>(pProxyModel);
    pPmntProxyModel->setTableView(pView);

    pView->setModel(pProxyModel);
    pView->setSortingEnabled(true);
    pView->resizeColumnsToContents();
    pView->resizeRowsToContents();
    pView->horizontalHeader()->setStretchLastSection(true);
    pView->setEditTriggers(QAbstractItemView::NoEditTriggers);

//    QPointer<ModelPayments> pSourceModel = DBHandler::getInstance()->getPaymentModel();

//    if (pSourceModel)
    {
//      QModelIndex sourceIndex = pSourceModel->index(0, PMNT_SOURCE_COL_TIMESTAMP);
//      QModelIndex proxyIndex  = (static_cast<PaymentsProxyModel *>(pProxyModel)) -> mapFromSource(sourceIndex);
        // ----------------------------------
//      PaymentsProxyModel * pPmntProxyModel = static_cast<PaymentsProxyModel *>(pProxyModel);

        pView->sortByColumn(7, Qt::DescendingOrder); // The timestamp ends up at index 7 in all the proxy views.

//      qDebug() << "SORT COLUMN: " << proxyIndex.column() << "\n";
    }
    pView->setContextMenuPolicy(Qt::CustomContextMenu);
    pView->verticalHeader()->hide();
    pView->setAlternatingRowColors(true);
    pView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    pView->setSelectionBehavior(QAbstractItemView::SelectRows);
}






void Payments::RefreshUserBar()
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


//QWidget * Payments::CreateUserBarWidget()
//{
//    QWidget     * pUserBar        = new QWidget;
//    QGridLayout * pUserBar_layout = new QGridLayout;

//    pUserBar_layout->setSpacing(12);
//    pUserBar_layout->setContentsMargins(0,0,0,0); // left top right bottom
////  pUserBar_layout->setContentsMargins(12, 3, 8, 10); // left top right bottom

//    pUserBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

//    pUserBar->setLayout(pUserBar_layout);
//    pUserBar->setStyleSheet("QWidget{background-color:#c0cad4;selection-background-color:#a0aac4;}");  // todo hardcoding.
//    // -------------------------------------------
//    QString qstr_acct_nym,
//            qstr_acct_server,
//            qstr_acct_asset, qstr_acct_asset_name("");
//    // -------------------------------------------
//    QString qstr_acct_name("");
//    QString qstr_balance(""), qstr_tla("");
//    QString qstr_acct_id = Moneychanger::It()->get_default_account_id();
//    // -------------------------------------------
//    if (qstr_acct_id.isEmpty())
//    {
//        qstr_balance     = tr("0.00");
//        qstr_acct_name = QString("");
//        // -----------------------------------
//        qstr_acct_nym    = Moneychanger::It()->get_default_nym_id();
//        qstr_acct_server = Moneychanger::It()->get_default_notary_id();
//        qstr_acct_asset  = Moneychanger::It()->get_default_asset_id();
//    }
//    else
//    {
//        // -----------------------------------
//        std::string str_acct_id     = qstr_acct_id.toStdString();
//        std::string str_acct_nym    = opentxs::OT::App().Client().Exec().GetAccountWallet_NymID(str_acct_id);
//        std::string str_acct_server = opentxs::OT::App().Client().Exec().GetAccountWallet_NotaryID(str_acct_id);
//        std::string str_acct_asset  = opentxs::OT::App().Client().Exec().GetAccountWallet_InstrumentDefinitionID(str_acct_id);
//        // -----------------------------------
//        qstr_acct_nym    = QString::fromStdString(str_acct_nym);
//        qstr_acct_server = QString::fromStdString(str_acct_server);
//        qstr_acct_asset  = QString::fromStdString(str_acct_asset);
//        // -----------------------------------
//        std::string str_tla = opentxs::OT::App().Client().Exec().GetCurrencyTLA(str_acct_asset);
//        qstr_tla = QString("<font color=grey>%1</font>").arg(QString::fromStdString(str_tla));

//        qstr_balance = MTHome::shortAcctBalance(qstr_acct_id, qstr_acct_asset, false);
//        // -----------------------------------
//        std::string str_acct_name  = opentxs::OT::App().Client().Exec().GetAccountWallet_Name(str_acct_id);
//        // -----------------------------------
//        if (!str_acct_asset.empty())
//        {
//            std::string str_asset_name = opentxs::OT::App().Client().Exec().GetAssetType_Name(str_acct_asset);
//            qstr_acct_asset_name = QString::fromStdString(str_asset_name);
//        }
//        // -----------------------------------
//        if (!str_acct_name.empty())
//        {
//            qstr_acct_name = QString("%1").arg(QString::fromStdString(str_acct_name));
//        }
//    }
//    // ---------------------------------------------
//    QToolButton * buttonAccount = new QToolButton;

//    buttonAccount->setAutoRaise(true);
//    buttonAccount->setStyleSheet("QToolButton { margin-left: 0; font-size:30pt;  font-weight:lighter; }");

//    QLabel * tla_label = new QLabel(qstr_tla);
//    tla_label->setAlignment(Qt::AlignRight|Qt::AlignBottom);
//    tla_label->setStyleSheet("QLabel { margin-right: 0; font-size:20pt;  font-weight:lighter; }");

//    buttonAccount->setText(qstr_balance);
//    // -------------------------------------------
//    connect(buttonAccount, SIGNAL(clicked()), Moneychanger::It(), SLOT(mc_show_account_manager_slot()));
//    // ----------------------------------------------------------------
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
//    // -------------------------------------------
//    QLabel * pCashLabel = new QLabel(qstrCash);
//    // ---------------------------------------------------------------
//    //pCashLabel->setText(qstrCash);
//    pCashLabel->setIndent(13);
//    // ---------------------------------------------------------------
//    QWidget * row_balance_container = new QWidget;
//    QHBoxLayout * row_balance_layout = new QHBoxLayout;

//    tla_label->setContentsMargins(12, 0, 0, 5);
//    tla_label->setAlignment(Qt::AlignRight|Qt::AlignBottom);
//    row_balance_layout->setSpacing(0);
//    row_balance_layout->addWidget(tla_label);
//    row_balance_layout->addWidget(buttonAccount);

//    row_balance_layout->setMargin(0);
//    row_balance_layout->setContentsMargins(0, 0, 0, 0);
////  row_balance_layout->setContentsMargins(0, 20, 0, 0);
//    row_balance_container->setContentsMargins(0, 0, 0, 0);

//    row_balance_container->setLayout(row_balance_layout);
//    // ----------------------------------------------------------------
//    QVBoxLayout * pAccountLayout = new QVBoxLayout;

//    pAccountLayout->setMargin(0);
//    pAccountLayout->setContentsMargins(0, 0, 0, 0);
//    pAccountLayout->setSpacing(3);

//    pAccountLayout->addWidget(row_balance_container);
//    pAccountLayout->addWidget(pCashLabel);
//    pAccountLayout->addStretch();
//    // ----------------------------------------------------------------
//    //Sub-info
//    QWidget * row_content_container = new QWidget;
//    QGridLayout * row_content_grid = new QGridLayout;

//    row_content_container->setContentsMargins(0, 0, 0, 0);

//    row_content_grid->setSpacing(4);
//    row_content_grid->setMargin(0); // new

//    row_content_grid->setContentsMargins(3, 0, 3, 0); // left top right bottom
////  row_content_grid->setContentsMargins(3, 4, 3, 4); // left top right bottom

//    row_content_container->setLayout(row_content_grid);
//    // -------------------------------------------
//    QString  identity_label_string = QString("<font color=grey>%1:</font> ").arg(tr("My Identity"));
//    QLabel * pIdentityLabel = new QLabel(identity_label_string);
//    pIdentityLabel->setIndent(2);
////  pIdentityLabel->setContentsMargins(12, 0, 0, 5);
//    pIdentityLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
////  pIdentityLabel->setStyleSheet("QLabel { margin-right: 0; font-size:20pt;  font-weight:lighter; }");
//    // --------------------------------------------
//    QString  nym_label_string = QString("");
//    // --------------------------------------------
//    QString qstrPaymentCode("");
//    std::string payment_code("");

//    if (!qstr_acct_nym.isEmpty())
//    {
//        payment_code = opentxs::OT::App().Client().Exec().GetNym_Description(qstr_acct_nym.toStdString());
//        qstrPaymentCode = QString::fromStdString(payment_code);
//        // ----------------------------
//        QString qstr_name = QString::fromStdString(opentxs::OT::App().Client().Exec().GetNym_Name(qstr_acct_nym.toStdString()));

//        if (!qstr_name.isEmpty())
//            nym_label_string = qstr_name;
//        else
//            nym_label_string = QString("(name is blank)");
//    }
//    else
//        nym_label_string += tr("(none selected)");
//    // ---------------------------------------------------------------
//    QToolButton * buttonNym = new QToolButton;

//    buttonNym->setText(nym_label_string);
//    buttonNym->setAutoRaise(true);
//    buttonNym->setStyleSheet("QToolButton { margin-left: 0; font-size:15pt;  font-weight:lighter; }");
////  buttonNym->setStyleSheet("QToolButton { margin-left: 0; font-size:20pt;  font-weight:lighter; }");
//    // -------------------------------------------
//    connect(buttonNym, SIGNAL(clicked()), Moneychanger::It(), SLOT(mc_defaultnym_slot()));
//    // ----------------------------------------------------------------
//    QHBoxLayout * pIdentityLayout = new QHBoxLayout;

//    pIdentityLayout->setMargin(0);
//    pIdentityLayout->setContentsMargins(0, 0, 0, 0); // new
//    pIdentityLayout->setSpacing(0);
//    pIdentityLayout->addSpacing(8);
//    pIdentityLayout->addWidget(pIdentityLabel);
//    pIdentityLayout->addWidget(buttonNym);
//    // ---------------------------------------------------------------
//    row_content_grid->addLayout(pIdentityLayout, 0,0, 1,1, Qt::AlignLeft);
//    // -------------------------------------------
//    int nColumn = 0;

//    const int nColumnWidthBalance = 2;

//    pUserBar_layout->addLayout(pAccountLayout,        0, nColumn, 1,nColumnWidthBalance, Qt::AlignLeft);
//    pUserBar_layout->addWidget(row_content_container, 1, nColumn, 1,nColumnWidthBalance, Qt::AlignLeft);

//    nColumn += nColumnWidthBalance;

//    if (!qstrPaymentCode.isEmpty())
//    {
//        QrWidget qrWidget;
//        qrWidget.setString(qstrPaymentCode);

//        QImage image;
//        qrWidget.asImage(image, 100);

//        QPixmap pixmapQR = QPixmap::fromImage(image);
//        // ----------------------------------------------------------------
//        QIcon qrButtonIcon  (pixmapQR);
//        // ----------------------------------------------------------------
//        QToolButton * buttonPaymentCode = new QToolButton;

//        buttonPaymentCode->setAutoRaise(true);
//        buttonPaymentCode->setStyleSheet("QToolButton { margin-left: 0; font-size:15pt;  font-weight:lighter; }");
//        buttonPaymentCode->setIcon(qrButtonIcon);
//        buttonPaymentCode->setIconSize(pixmapQR.rect().size());
////      buttonPaymentCode->setString(qstrPaymentCode);
//        // -------------------------------------------
//        connect(buttonPaymentCode, SIGNAL(clicked()), Moneychanger::It(), SLOT(mc_defaultnym_slot()));
//        // ----------------------------------------------------------------
//        const int nColumnWidthQrCode = 2;

//        pUserBar_layout->addWidget(buttonPaymentCode, 0, nColumn, 2, nColumnWidthQrCode, Qt::AlignLeft);

//        nColumn += nColumnWidthQrCode;
//    }
//    // -------------------------------------------
////    pUserBar_layout->addLayout(pAccountLayout,        0, nColumn, 1,2, Qt::AlignLeft);
////    pUserBar_layout->addWidget(row_content_container, 1, nColumn, 1,2, Qt::AlignLeft);
//    // -------------------------------------------
//    return pUserBar;
//}


QWidget * Payments::CreateUserBarWidget()
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
        std::string str_acct_nym    = opentxs::OT::App().Client().Exec().GetAccountWallet_NymID(str_acct_id);
        std::string str_acct_server = opentxs::OT::App().Client().Exec().GetAccountWallet_NotaryID(str_acct_id);
        std::string str_acct_asset  = opentxs::OT::App().Client().Exec().GetAccountWallet_InstrumentDefinitionID(str_acct_id);
        // -----------------------------------
        qstr_acct_nym    = QString::fromStdString(str_acct_nym);
        qstr_acct_server = QString::fromStdString(str_acct_server);
        qstr_acct_asset  = QString::fromStdString(str_acct_asset);
        // -----------------------------------
        std::string str_tla = opentxs::OT::App().Client().Exec().GetCurrencyTLA(str_acct_asset);
        qstr_tla = QString("<font color=grey>%1</font>").arg(QString::fromStdString(str_tla));

        qstr_balance = MTHome::shortAcctBalance(qstr_acct_id, qstr_acct_asset, false);
        // -----------------------------------
        std::string str_acct_name  = opentxs::OT::App().Client().Exec().GetAccountWallet_Name(str_acct_id);
        // -----------------------------------
        if (!str_acct_asset.empty())
        {
            std::string str_asset_name = opentxs::OT::App().Client().Exec().GetAssetType_Name(str_acct_asset);
            qstr_acct_asset_name = QString::fromStdString(str_asset_name);
        }
        // -----------------------------------
        if (!str_acct_name.empty())
        {
            qstr_acct_name = QString("%1").arg(QString::fromStdString(str_acct_name));
        }
    }
    // ---------------------------------------------
    QToolButton * buttonAccount = new QToolButton;

    buttonAccount->setAutoRaise(true);
    buttonAccount->setStyleSheet("QToolButton { margin-left: 0; font-size:30pt;  font-weight:lighter; }");

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
    QWidget * balanceWidget = new QWidget;
    QHBoxLayout * balanceLayout = new QHBoxLayout;

    tla_label->setContentsMargins(12, 0, 0, 5);
    tla_label->setAlignment(Qt::AlignRight|Qt::AlignBottom);
    balanceLayout->setSpacing(0);
    balanceLayout->addWidget(tla_label);
    balanceLayout->addWidget(buttonAccount);

    balanceLayout->setMargin(0);
    balanceLayout->setContentsMargins(0, 0, 0, 0);
//  balanceLayout->setContentsMargins(0, 20, 0, 0);
    balanceWidget->setContentsMargins(0, 0, 0, 0);

    balanceWidget->setLayout(balanceLayout);
    // ----------------------------------------------------------------
    QVBoxLayout * pAccountLayout = new QVBoxLayout;

    pAccountLayout->setMargin(0);
    pAccountLayout->setContentsMargins(0, 0, 0, 0);
    pAccountLayout->setSpacing(3);

    pAccountLayout->addWidget(balanceWidget);
    pAccountLayout->addWidget(pCashLabel);
    pAccountLayout->addStretch();
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
        payment_code = opentxs::OT::App().Client().Exec().GetNym_Description(qstr_acct_nym.toStdString());
        qstrPaymentCode = QString::fromStdString(payment_code);
        // ----------------------------
        QString qstr_name = QString::fromStdString(opentxs::OT::App().Client().Exec().GetNym_Name(qstr_acct_nym.toStdString()));

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
    pUserBar_layout->addLayout(pAccountLayout,  0, 2, 2,2, Qt::AlignLeft);
    pUserBar_layout->addLayout(pIdentityLayout, 0, 0, 2,2, Qt::AlignLeft);

    return pUserBar;
}


void Payments::on_tabWidget_currentChanged(int index)
{
//    if (ui->tableViewSent      && ui->tableViewReceived &&
//        pPmntProxyModelOutbox_ && pPmntProxyModelInbox_)
//    {
//        pCurrentTabTableView_  = (0 == ui->tabWidget->currentIndex()) ? ui->tableViewReceived    : ui->tableViewSent;
//        pCurrentTabProxyModel_ = (0 == ui->tabWidget->currentIndex()) ? &(*pPmntProxyModelInbox_) : &(*pPmntProxyModelOutbox_);
//        // -------------------------------------------------
//        QModelIndex the_index  = pCurrentTabTableView_->currentIndex();

//        if (the_index.isValid())
//            enableButtons();
//        else
//            disableButtons();
//        // --------------------------------------
//        RefreshPayments();
//    }
//    else
//    {
//        pCurrentTabTableView_  = nullptr;
//        pCurrentTabProxyModel_ = nullptr;

//        disableButtons();
//    }
}


void Payments::enableButtons()
{
    ui->toolButtonDelete ->setEnabled(true);
    ui->toolButtonReply  ->setEnabled(true);
    ui->toolButtonForward->setEnabled(true);
}

void Payments::disableButtons()
{
    ui->toolButtonDelete ->setEnabled(false);
    ui->toolButtonReply  ->setEnabled(false);
    ui->toolButtonForward->setEnabled(false);
}

void Payments::on_MarkAsRead_timer()
{
    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

    if (!pModel)
        return;
    // ------------------------------
    bool bEditing = false;

    while (!listRecordsToMarkAsRead_.isEmpty())
    {
        QModelIndex index = listRecordsToMarkAsRead_.front();
        listRecordsToMarkAsRead_.pop_front();
        // ------------------------------------
        if (!index.isValid())
            continue;
        // ------------------------------------
        if (!bEditing)
        {
            bEditing = true;
            pModel->database().transaction();
        }
        // ------------------------------------
        pModel->setData(index, QVariant(1)); // 1 for "true" in sqlite. "Yes, we've now read this payment. Mark it as read."
    } // while
    // ------------------------------
    if (bEditing)
    {
        if (pModel->submitAll())
        {
            pModel->database().commit();
            // ------------------------------------
            QTimer::singleShot(0, this, SLOT(RefreshPayments()));
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

void Payments::on_MarkAsUnread_timer()
{
    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

    if (!pModel)
        return;
    // ------------------------------
    bool bEditing = false;

    while (!listRecordsToMarkAsUnread_.isEmpty())
    {
        QModelIndex index = listRecordsToMarkAsUnread_.front();
        listRecordsToMarkAsUnread_.pop_front();
        // ------------------------------------
        if (!index.isValid())
            continue;
        // ------------------------------------
        if (!bEditing)
        {
            bEditing = true;
            pModel->database().transaction();
        }
        // ------------------------------------
        pModel->setData(index, QVariant(0)); // 0 for "false" in sqlite. "This payment is now marked UNREAD."
    } // while
    // ------------------------------
    if (bEditing)
    {
        if (pModel->submitAll())
        {
            pModel->database().commit();
            // ------------------------------------
            QTimer::singleShot(0, this, SLOT(RefreshPayments()));
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

void Payments::on_MarkAsReplied_timer()
{
    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

    if (!pModel)
        return;
    // ------------------------------
    bool bEditing = false;

    while (!listRecordsToMarkAsReplied_.isEmpty())
    {
        QModelIndex index = listRecordsToMarkAsReplied_.front();
        listRecordsToMarkAsReplied_.pop_front();
        // ------------------------------------
        if (!index.isValid())
            continue;
        // ------------------------------------
        if (!bEditing)
        {
            bEditing = true;
            pModel->database().transaction();
        }
        // ------------------------------------
        pModel->setData(index, QVariant(1)); // 1 for "true" in sqlite. "Yes, we've now replied to this payment. Mark it as replied."
    } // while
    // ------------------------------
    if (bEditing)
    {
        if (pModel->submitAll())
        {
            pModel->database().commit();
            // ------------------------------------
            QTimer::singleShot(0, this, SLOT(RefreshPayments()));
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

void Payments::on_MarkAsForwarded_timer()
{
    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

    if (!pModel)
        return;
    // ------------------------------
    bool bEditing = false;

    while (!listRecordsToMarkAsForwarded_.isEmpty())
    {
        QModelIndex index = listRecordsToMarkAsForwarded_.front();
        listRecordsToMarkAsForwarded_.pop_front();
        // ------------------------------------
        if (!index.isValid())
            continue;
        // ------------------------------------
        if (!bEditing)
        {
            bEditing = true;
            pModel->database().transaction();
        }
        // ------------------------------------
        pModel->setData(index, QVariant(1)); // 1 for "true" in sqlite. "Yes, we've now forwarded this payment. Mark it as forwarded."
    } // while
    // ------------------------------
    if (bEditing)
    {
        if (pModel->submitAll())
        {
            pModel->database().commit();
            // ------------------------------------
            QTimer::singleShot(0, this, SLOT(RefreshPayments()));
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

void Payments::on_tableViewSentSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous)
{
//    if (!current.isValid())
//    {
//        disableButtons();
//        // ----------------------------------------
//        PMNT_TREE_ITEM theItem = make_tree_item(nCurrentContact_, qstrMethodType_, qstrViaTransport_);
//        set_outbox_pmntid_for_tree_item(theItem, 0);
//    }
//    else
//    {
//        enableButtons();
//        // ----------------------------------------
//        QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

//        if (pModel)
//        {
//            QModelIndex sourceIndex = pPmntProxyModelOutbox_->mapToSource(current);

//            QModelIndex haveReadSourceIndex  = pPmntProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_HAVE_READ, sourceIndex);
//            QModelIndex pmntidSourceIndex    = pPmntProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_PMNT_ID,   sourceIndex);
////            QModelIndex subjectSourceIndex   = pPmntProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_MEMO,      sourceIndex);
////            QModelIndex senderSourceIndex    = pPmntProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_MY_NYM,    sourceIndex);
////            QModelIndex recipientSourceIndex = pPmntProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_RECIP_NYM, sourceIndex);
////            QModelIndex timestampSourceIndex = pPmntProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_TIMESTAMP, sourceIndex);

////            QModelIndex subjectIndex      = pPmntProxyModelOutbox_->mapFromSource(subjectSourceIndex);
////            QModelIndex senderIndex       = pPmntProxyModelOutbox_->mapFromSource(senderSourceIndex);
////            QModelIndex recipientIndex    = pPmntProxyModelOutbox_->mapFromSource(recipientSourceIndex);
////            QModelIndex timestampIndex    = pPmntProxyModelOutbox_->mapFromSource(timestampSourceIndex);

//            QVariant varpmntid    = pModel->data(pmntidSourceIndex);
//            QVariant varHaveRead  = pModel->data(haveReadSourceIndex);
////            QVariant varSubject   = pPmntProxyModelOutbox_->data(subjectIndex);
////            QVariant varSender    = pPmntProxyModelOutbox_->data(senderIndex);
////            QVariant varRecipient = pPmntProxyModelOutbox_->data(recipientIndex);
////            QVariant varTimestamp = pPmntProxyModelOutbox_->data(timestampIndex);

////            QString qstrSubject   = varSubject.isValid()   ? varSubject.toString()   : "";
////            QString qstrSender    = varSender.isValid()    ? varSender.toString()    : "";
////            QString qstrRecipient = varRecipient.isValid() ? varRecipient.toString() : "";
////            QString qstrTimestamp = varTimestamp.isValid() ? varTimestamp.toString() : "";
//            // ----------------------------------------------------------
//            int payment_id = varpmntid.isValid() ? varpmntid.toInt() : 0;
//            if (payment_id > 0)
//            {
//                PMNT_TREE_ITEM theItem = make_tree_item(nCurrentContact_, qstrMethodType_, qstrViaTransport_);
//                set_outbox_pmntid_for_tree_item(theItem, payment_id);
//            }
//            // ----------------------------------------------------------
//            const bool bHaveRead = varHaveRead.isValid() ? varHaveRead.toBool() : false;

//            if (!bHaveRead && (payment_id > 0)) // It's unread, so we need to set it as read.
//            {
//                listRecordsToMarkAsRead_.append(haveReadSourceIndex);
//                QTimer::singleShot(1000, this, SLOT(on_MarkAsRead_timer()));
//            }
//        }
//    }
}

void Payments::on_tableViewReceivedSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous)
{
//    if (!current.isValid())
//    {
//        disableButtons();
//        // ----------------------------------------
//        PMNT_TREE_ITEM theItem = make_tree_item(nCurrentContact_, qstrMethodType_, qstrViaTransport_);
//        set_inbox_pmntid_for_tree_item(theItem, 0);
//    }
//    else
//    {
//        enableButtons();
//        // ----------------------------------------
//        QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

//        if (pModel)
//        {
//            QModelIndex sourceIndex = pPmntProxyModelInbox_->mapToSource(current);

//            QModelIndex haveReadSourceIndex  = pPmntProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_HAVE_READ,  sourceIndex);
//            QModelIndex pmntidSourceIndex    = pPmntProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_PMNT_ID,    sourceIndex);
////            QModelIndex subjectSourceIndex   = pPmntProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_MEMO,       sourceIndex);
////            QModelIndex senderSourceIndex    = pPmntProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_SENDER_NYM, sourceIndex);
////            QModelIndex recipientSourceIndex = pPmntProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_MY_NYM,     sourceIndex);
////            QModelIndex timestampSourceIndex = pPmntProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_TIMESTAMP,  sourceIndex);

////            QModelIndex subjectIndex      = pPmntProxyModelInbox_->mapFromSource(subjectSourceIndex);
////            QModelIndex senderIndex       = pPmntProxyModelInbox_->mapFromSource(senderSourceIndex);
////            QModelIndex recipientIndex    = pPmntProxyModelInbox_->mapFromSource(recipientSourceIndex);
////            QModelIndex timestampIndex    = pPmntProxyModelInbox_->mapFromSource(timestampSourceIndex);

//            QVariant varpmntid    = pModel->data(pmntidSourceIndex);
//            QVariant varHaveRead  = pModel->data(haveReadSourceIndex);
////            QVariant varSubject   = pPmntProxyModelInbox_->data(subjectIndex);
////            QVariant varSender    = pPmntProxyModelInbox_->data(senderIndex);
////            QVariant varRecipient = pPmntProxyModelInbox_->data(recipientIndex);
////            QVariant varTimestamp = pPmntProxyModelInbox_->data(timestampIndex);

////            QString qstrSubject   = varSubject.isValid()   ? varSubject  .toString() : "";
////            QString qstrSender    = varSender   .isValid() ? varSender   .toString() : "";
////            QString qstrRecipient = varRecipient.isValid() ? varRecipient.toString() : "";
////            QString qstrTimestamp = varTimestamp.isValid() ? varTimestamp.toString() : "";

////            ui->headerReceived->setSubject  (qstrSubject);
////            ui->headerReceived->setSender   (qstrSender);
////            ui->headerReceived->setRecipient(qstrRecipient);
////            ui->headerReceived->setTimestamp(qstrTimestamp);
////            ui->headerReceived->setFolder(tr("Received"));

//            int payment_id = varpmntid.isValid() ? varpmntid.toInt() : 0;
//            if (payment_id > 0)
//            {
//                PMNT_TREE_ITEM theItem = make_tree_item(nCurrentContact_, qstrMethodType_, qstrViaTransport_);
//                set_inbox_pmntid_for_tree_item(theItem, payment_id);
//            }
//            // ----------------------------------------------------------
//            const bool bHaveRead = varHaveRead.isValid() ? varHaveRead.toBool() : false;

//            if (!bHaveRead && (payment_id > 0)) // It's unread, so we need to set it as read.
//            {
//                listRecordsToMarkAsRead_.append(haveReadSourceIndex);
//                QTimer::singleShot(1000, this, SLOT(on_MarkAsRead_timer()));
//            }
//        }
//    }
}



void Payments::dialog(int nSourceRow/*=-1*/, int nFolder/*=-1*/)
{
    if (!already_init)
    {
        ui->userBar->setStyleSheet("QWidget{background-color:#c0cad4;selection-background-color:#a0aac4;}");
        // ----------------------------------
        QPixmap pixmapSize(":/icons/icons/user.png"); // This is here only for size purposes.
        QIcon   sizeButtonIcon(pixmapSize);           // Other buttons use this to set their own size.
        // ----------------------------------------------------------------
        QPixmap pixmapSend    (":/icons/icons/fistful_of_cash_72.png");
        QPixmap pixmapContacts(":/icons/icons/rolodex_small");
//      QPixmap pixmapCompose (":/icons/icons/pencil.png");
        QPixmap pixmapRefresh (":/icons/icons/refresh.png");
        QPixmap pixmapReply   (":/icons/icons/reply.png");
        QPixmap pixmapForward (":/icons/sendfunds");
        QPixmap pixmapDelete  (":/icons/icons/DeleteRed.png");
        QPixmap pixmapRequest (":/icons/requestpayment");
        QPixmap pixmapPending (":/icons/icons/pending.png");
        QPixmap pixmapImport  (":/icons/icons/request.png");
        QPixmap pixmapMessages(":/icons/icons/pencil.png");
        QPixmap pixmapExchange  (":/icons/markets");
        QPixmap pixmapSecrets   (":/icons/icons/vault.png");
        // ----------------------------------------------------------------
        QIcon contactsButtonIcon(pixmapContacts);
        QIcon refreshButtonIcon (pixmapRefresh);
        QIcon sendButtonIcon    (pixmapSend);
        QIcon replyButtonIcon   (pixmapReply);
        QIcon forwardButtonIcon (pixmapForward);
        QIcon deleteButtonIcon  (pixmapDelete);
        QIcon requestButtonIcon  (pixmapRequest);
        QIcon pendingButtonIcon  (pixmapPending);
        QIcon importButtonIcon   (pixmapImport);
        QIcon messagesButtonIcon (pixmapMessages);
        QIcon exchangeButtonIcon (pixmapExchange);
        QIcon secretsButtonIcon  (pixmapSecrets);
        // ----------------------------------------------------------------
        ui->toolButtonRefresh->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButtonRefresh->setAutoRaise(true);
        ui->toolButtonRefresh->setIcon(refreshButtonIcon);
//      ui->toolButtonRefresh->setIconSize(pixmapRefresh.rect().size());
        ui->toolButtonRefresh->setIconSize(pixmapSize.rect().size());
        ui->toolButtonRefresh->setText(tr("Refresh"));
        // ----------------------------------------------------------------
        ui->toolButtonContacts->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButtonContacts->setAutoRaise(true);
        ui->toolButtonContacts->setIcon(contactsButtonIcon);
//      ui->toolButtonContacts->setIconSize(pixmapContacts.rect().size());
        ui->toolButtonContacts->setIconSize(pixmapSize.rect().size());
        ui->toolButtonContacts->setText(tr("Address Book"));
//      ui->toolButtonContacts->setHidden(true);
        // ----------------------------------------------------------------
        ui->toolButtonPay->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButtonPay->setAutoRaise(true);
        ui->toolButtonPay->setIcon(sendButtonIcon);
        ui->toolButtonPay->setIconSize(pixmapSend.rect().size());
        ui->toolButtonPay->setText(tr("Pay"));
        // ----------------------------------------------------------------
        ui->toolButtonDelete->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButtonDelete->setAutoRaise(true);
        ui->toolButtonDelete->setIcon(deleteButtonIcon);
//      ui->toolButtonDelete->setIconSize(pixmapDelete.rect().size());
        ui->toolButtonDelete->setIconSize(pixmapSize.rect().size());
        ui->toolButtonDelete->setText(tr("Delete"));
        // ----------------------------------------------------------------
        ui->toolButtonReply->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButtonReply->setAutoRaise(true);
        ui->toolButtonReply->setIcon(replyButtonIcon);
//      ui->toolButtonReply->setIconSize(pixmapReply.rect().size());
        ui->toolButtonReply->setIconSize(pixmapSize.rect().size());
        ui->toolButtonReply->setText(tr("Reply"));
        ui->toolButtonReply->setHidden(true);
        // ----------------------------------------------------------------
        ui->toolButtonForward->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButtonForward->setAutoRaise(true);
        ui->toolButtonForward->setIcon(forwardButtonIcon);
//      ui->toolButtonForward->setIconSize(pixmapForward.rect().size());
        ui->toolButtonForward->setIconSize(pixmapSize.rect().size());
        ui->toolButtonForward->setText(tr("Forward"));
        ui->toolButtonForward->setHidden(true);
        // ----------------------------------------------------------------
        ui->toolButtonRequest->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButtonRequest->setAutoRaise(true);
        ui->toolButtonRequest->setIcon(requestButtonIcon);
//      ui->toolButtonRequest->setIconSize(pixmapRequest.rect().size());
        ui->toolButtonRequest->setIconSize(pixmapSize.rect().size());
        ui->toolButtonRequest->setText(tr("Request Funds"));
        ui->toolButtonRequest->setHidden(true);
        // ----------------------------------------------------------------
        ui->toolButtonPending->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButtonPending->setAutoRaise(true);
        ui->toolButtonPending->setIcon(pendingButtonIcon);
//      ui->toolButtonPending->setIconSize(pixmapPending.rect().size());
        ui->toolButtonPending->setIconSize(pixmapSize.rect().size());
        ui->toolButtonPending->setText(tr("View Pending"));
        // ----------------------------------------------------------------
        ui->toolButtonImport->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButtonImport->setAutoRaise(true);
        ui->toolButtonImport->setIcon(importButtonIcon);
//      ui->toolButtonImport->setIconSize(pixmapImport.rect().size());
        ui->toolButtonImport->setIconSize(pixmapSize.rect().size());
        ui->toolButtonImport->setText(tr("Import Cash"));
        ui->toolButtonImport->setHidden(true);
        // ----------------------------------------------------------------
        ui->toolButtonMessages->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButtonMessages->setAutoRaise(true);
        ui->toolButtonMessages->setIcon(messagesButtonIcon);
//      ui->toolButtonMessages->setIconSize(pixmapMessages.rect().size());
        ui->toolButtonMessages->setIconSize(pixmapSize.rect().size());
        ui->toolButtonMessages->setText(tr("Messages"));
        // ----------------------------------------------------------------
        ui->toolButtonExchange->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButtonExchange->setAutoRaise(true);
        ui->toolButtonExchange->setIcon(exchangeButtonIcon);
//      ui->toolButtonExchange->setIconSize(pixmapExchange.rect().size());
        ui->toolButtonExchange->setIconSize(pixmapSize.rect().size());
        ui->toolButtonExchange->setText(tr("Exchange"));
        // ----------------------------------------------------------------
        ui->toolButtonSecrets->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButtonSecrets->setAutoRaise(true);
        ui->toolButtonSecrets->setIcon(secretsButtonIcon);
//      ui->toolButtonSecrets->setIconSize(pixmapSecrets.rect().size());
        ui->toolButtonSecrets->setIconSize(pixmapSize.rect().size());
        ui->toolButtonSecrets->setText(tr("Secrets"));
        ui->toolButtonSecrets->setHidden(true);
        // ----------------------------------------------------------------
        connect(ui->toolButtonRequest,   SIGNAL(clicked()),  Moneychanger::It(), SLOT(mc_requestfunds_slot()));
        connect(ui->toolButtonPending,   SIGNAL(clicked()),  Moneychanger::It(), SLOT(mc_overview_slot()));
        connect(ui->toolButtonImport,    SIGNAL(clicked()),  Moneychanger::It(), SLOT(mc_import_slot()));
        connect(ui->toolButtonMessages,  SIGNAL(clicked()),  Moneychanger::It(), SLOT(mc_messages_slot()));
        connect(ui->toolButtonExchange,  SIGNAL(clicked()),  Moneychanger::It(), SLOT(mc_market_slot()));
        connect(ui->toolButtonSecrets,   SIGNAL(clicked()),  Moneychanger::It(), SLOT(mc_passphrase_manager_slot()));
        connect(ui->toolButtonContacts,  SIGNAL(clicked()),  Moneychanger::It(), SLOT(mc_addressbook_slot()));
        // ----------------------------------
        // Note: This is a placekeeper, so later on I can just erase
        // the widget at 0 and replace it with the real header widget.
        //
        m_pHeaderFrame  = new QFrame;
        QHBoxLayout * pHBoxLayout = static_cast<QHBoxLayout*>(ui->userBar->layout());
        pHBoxLayout->insertWidget(0, m_pHeaderFrame);
        // ----------------------------------

        // ******************************************************
//        QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

//        if (pModel)
//        {
//            pPmntProxyModelInbox_  = new PaymentsProxyModel;
//            pPmntProxyModelOutbox_ = new PaymentsProxyModel;
//            pPmntProxyModelInbox_ ->setSourceModel(pModel);
//            pPmntProxyModelOutbox_->setSourceModel(pModel);
//            pPmntProxyModelOutbox_->setFilterFolder(0);
//            pPmntProxyModelInbox_ ->setFilterFolder(1);
            // ---------------------------------
//            pPmntProxyModelInbox_ ->setFilterKeyColumn(-1);
//            pPmntProxyModelOutbox_->setFilterKeyColumn(-1);
            // ---------------------------------
//            setup_tableview(ui->tableViewSent, pPmntProxyModelOutbox_);
//            setup_tableview(ui->tableViewReceived, pPmntProxyModelInbox_);
//            // ---------------------------------
//            QItemSelectionModel *sm1 = ui->tableViewSent->selectionModel();
//            connect(sm1, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
//                    this, SLOT(on_tableViewSentSelectionModel_currentRowChanged(QModelIndex,QModelIndex)));
//            QItemSelectionModel *sm2 = ui->tableViewReceived->selectionModel();
//            connect(sm2, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
//                    this, SLOT(on_tableViewReceivedSelectionModel_currentRowChanged(QModelIndex,QModelIndex)));
            // ---------------------------------
//            connect(pPmntProxyModelInbox_,  SIGNAL(modelReset()),
//                    this,                  SLOT(RefreshPayments()));
//            connect(pPmntProxyModelOutbox_, SIGNAL(modelReset()),
//                    this,                  SLOT(RefreshPayments()));
//        }
        // --------------------------------------------------------
        connect(this, SIGNAL(showContact(QString)),               Moneychanger::It(), SLOT(mc_showcontact_slot(QString)));
        // --------------------------------------------------------
        connect(this, SIGNAL(showContactAndRefreshHome(QString)), Moneychanger::It(), SLOT(onNeedToPopulateRecordlist()));
        connect(this, SIGNAL(showContactAndRefreshHome(QString)), Moneychanger::It(), SLOT(mc_showcontact_slot(QString)));
        // --------------------------------------------------------
//        QWidget* pTab0 = ui->tabWidget->widget(0);
//        QWidget* pTab1 = ui->tabWidget->widget(1);

//        pTab0->setStyleSheet("QWidget { margin: 0 }");
//        pTab1->setStyleSheet("QWidget { margin: 0 }");

//        ui->splitter->setStretchFactor(0, 1);
//        ui->splitter->setStretchFactor(1, 5);
        // ------------------------
        on_tabWidget_currentChanged(0);

        /** Flag Already Init **/
        already_init = true;
    }
    // -------------------------------------------
    RefreshAll();

    Focuser f(this);
    f.show();
    f.focus();
    // -------------------------------------------
    setAsCurrentPayment(nSourceRow, nFolder);
}


void Payments::setAsCurrentPayment(int nSourceRow, int nFolder)
{
//    if (-1 == nSourceRow || -1 == nFolder)
//        return;

//    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

//    if (!pModel)
//        return;
//    // -------------------
//    QTableView * pTableView = (0 == nFolder) ? ui->tableViewSent : ui->tableViewReceived;
//    if (nullptr == pTableView) return; // should never happen.
//    QPointer<PaymentsProxyModel> & pProxyModel = (0 == nFolder) ? pPmntProxyModelOutbox_ : pPmntProxyModelInbox_;
//    // ----------------------------
//    // If the table view we're switching to, is not the current one, then we
//    // need to make it the current one.
//    //
//    if (pTableView != pCurrentTabTableView_)
//    {
//        ui->tabWidget->setCurrentIndex((0 == nFolder) ? 1 : 0);
//    }
//    // ----------------------------
//    QModelIndex sourceIndex = pModel->index(nSourceRow, PMNT_SOURCE_COL_PMNT_ID);
//    QModelIndex proxyIndex;
//    if (sourceIndex.isValid())
//        proxyIndex = pProxyModel->mapFromSource(sourceIndex);
//    if (proxyIndex.isValid())
//        pTableView->setCurrentIndex(proxyIndex);
}


void Payments::ClearTree()
{
    ui->treeWidgetSummary->blockSignals(true);
    ui->treeWidgetSummary->clear();
    ui->treeWidgetSummary->blockSignals(false);
}


void Payments::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous);

    if (nullptr != current)
    {
        nCurrentContact_  = current->data(0, Qt::UserRole).toInt();
        qstrMethodType_   = current->data(1, Qt::UserRole).isValid() ? current->data(1, Qt::UserRole).toString() : QString("");
        qstrViaTransport_ = current->data(2, Qt::UserRole).isValid() ? current->data(2, Qt::UserRole).toString() : QString("");

        if ((0 == nCurrentContact_) && qstrMethodType_.isEmpty() && qstrViaTransport_.isEmpty())
        {
            nCurrentContact_  = 0;
            qstrMethodType_   = QString("");
            qstrViaTransport_ = QString("");

            pPmntProxyModelInbox_ ->setFilterNone();
            pPmntProxyModelOutbox_->setFilterNone();
        }
        else if (qstrMethodType_.isEmpty() && qstrViaTransport_.isEmpty())
        {
            pPmntProxyModelInbox_ ->setFilterTopLevel(nCurrentContact_);
            pPmntProxyModelOutbox_->setFilterTopLevel(nCurrentContact_);
        }
        else if (0 == qstrMethodType_.compare("otserver"))
        {
            pPmntProxyModelInbox_ ->setFilterNotary(qstrViaTransport_, nCurrentContact_);
            pPmntProxyModelOutbox_->setFilterNotary(qstrViaTransport_, nCurrentContact_);
        }
        else
        {
            pPmntProxyModelInbox_ ->setFilterMethodAddress(qstrMethodType_, qstrViaTransport_);
            pPmntProxyModelOutbox_->setFilterMethodAddress(qstrMethodType_, qstrViaTransport_);
        }
    }
    else
    {
        nCurrentContact_  = 0;
        qstrMethodType_   = QString("");
        qstrViaTransport_ = QString("");

        pPmntProxyModelInbox_ ->setFilterNone();
        pPmntProxyModelOutbox_->setFilterNone();
    }
    // --------------------------------------
    RefreshPayments();
}



void Payments::onClaimsUpdatedForNym(QString nymId)
{
    if (!bRefreshingAfterUpdatedClaims_)
    {
        bRefreshingAfterUpdatedClaims_ = true;
        QTimer::singleShot(500, this, SLOT(RefreshPayments()));
    }
}


void Payments::RefreshPayments()
{
//    bRefreshingAfterUpdatedClaims_ = false;
//    // -------------------------------------------
//    ui->tableViewSent->reset();
//    ui->tableViewReceived->reset();
//    // -------------------------------------------
//    PMNT_TREE_ITEM theItem = make_tree_item(nCurrentContact_, qstrMethodType_, qstrViaTransport_);

//    bool bIsInbox = (0 == ui->tabWidget->currentIndex());
//    int  nPmntID   = bIsInbox ? get_inbox_pmntid_for_tree_item(theItem) : get_outbox_pmntid_for_tree_item(theItem);

//    if (0 == nPmntID) // There's no "current selected payment ID" set for this tree item.
//    {
//        int nRowToSelect = -1;

//        if (pCurrentTabProxyModel_->rowCount() > 0) // But there ARE rows for this tree item...
//            nRowToSelect = 0;

//        // So let's select the first one in the list!
//        QModelIndex previous = pCurrentTabTableView_->currentIndex();
//        pCurrentTabTableView_->blockSignals(true);
//        pCurrentTabTableView_->selectRow(nRowToSelect);
//        pCurrentTabTableView_->blockSignals(false);

//        if (bIsInbox)
//            on_tableViewReceivedSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
//        else
//            on_tableViewSentSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
//    }
//    else // There IS a "current selected payment ID" for the current tree item.
//    {
//        // So let's try to select that in the tree again! (If it's still there. Otherwise set it to row 0.)

//        QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

//        if (pModel)
//        {
//            bool bFoundIt = false;

//            const int nRowCount = pCurrentTabProxyModel_->rowCount();

//            for (int ii = 0; ii < nRowCount; ++ii)
//            {
//                QModelIndex indexProxy  = pCurrentTabProxyModel_->index(ii, 0);
//                QModelIndex indexSource = pCurrentTabProxyModel_->mapToSource(indexProxy);

//                QSqlRecord record = pModel->record(indexSource.row());

//                if (!record.isEmpty())
//                {
//                    QVariant the_value = record.value(PMNT_SOURCE_COL_PMNT_ID);
//                    const int nRecordpmntid = the_value.isValid() ? the_value.toInt() : 0;

//                    if (nRecordpmntid == nPmntID)
//                    {
//                        bFoundIt = true;

//                        QModelIndex previous = pCurrentTabTableView_->currentIndex();
//                        pCurrentTabTableView_->blockSignals(true);
//                        pCurrentTabTableView_->selectRow(ii);
//                        pCurrentTabTableView_->blockSignals(false);

//                        if (bIsInbox)
//                            on_tableViewReceivedSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
//                        else
//                            on_tableViewSentSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
//                        break;
//                    }
//                }
//            }
//            // ------------------------------------
//            if (!bFoundIt)
//            {
//                int nRowToSelect = -1;

//                if (nRowCount > 0)
//                    nRowToSelect = 0;

//                QModelIndex previous = pCurrentTabTableView_->currentIndex();
//                pCurrentTabTableView_->blockSignals(true);
//                pCurrentTabTableView_->selectRow(nRowToSelect);
//                pCurrentTabTableView_->blockSignals(false);

//                if (bIsInbox)
//                    on_tableViewReceivedSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
//                else
//                    on_tableViewSentSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
//            }
//        }
//    }
//    // -------------------------------------------
//    ui->tableViewSent->resizeColumnsToContents();
//    ui->tableViewSent->resizeRowsToContents();
//    ui->tableViewReceived->resizeColumnsToContents();
//    ui->tableViewReceived->resizeRowsToContents();
//    {
//    int nWidthFirstColumn = ui->tableViewSent->columnWidth(0);
//    int nNewWidth = static_cast<int>( static_cast<float>(nWidthFirstColumn) * 1.2 );
//    ui->tableViewSent->setColumnWidth(0,nNewWidth);
//    }{
//    int nWidthFirstColumn = ui->tableViewReceived->columnWidth(0);
//    int nNewWidth = static_cast<int>( static_cast<float>(nWidthFirstColumn) * 1.2 );
//    ui->tableViewReceived->setColumnWidth(0,nNewWidth);
//    }
//    ui->tableViewSent->horizontalHeader()->setStretchLastSection(true);
//    ui->tableViewReceived->horizontalHeader()->setStretchLastSection(true);
}

void Payments::RefreshSummaryTree()
{

}

void Payments::RefreshTree()
{
//    ClearTree();
//    // ----------------------------------------
//    ui->treeWidget->blockSignals(true);
//    // ----------------------------------------
//    QList<QTreeWidgetItem *> items;
//    // ------------------------------------
//    QTreeWidgetItem * pUnfilteredItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(tr("All Payments (Unfiltered)")));
//    pUnfilteredItem->setData(0, Qt::UserRole, QVariant(0));
//    items.append(pUnfilteredItem);
//    // ----------------------------------------
//    mapIDName mapContacts;

//    if (MTContactHandler::getInstance()->GetContacts(mapContacts))
//    {
//        for (mapIDName::iterator ii = mapContacts.begin(); ii != mapContacts.end(); ii++)
//        {
//            QString qstrContactID   = ii.key();
//            QString qstrContactName = ii.value();
//            int     nContactID      = qstrContactID.toInt();
//            // ------------------------------------
//            QTreeWidgetItem * pTopItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(qstrContactName));
//            pTopItem->setData(0, Qt::UserRole, QVariant(nContactID));
//            items.append(pTopItem);
//            // ------------------------------------
//            mapIDName mapMethodTypes;
//            // So we can look up these names quickly without having to repeatedly hit the database
//            // for the same names over and over again.
//            MTContactHandler::getInstance()->GetMsgMethodTypes(mapMethodTypes);
//            // ------------------------------------
//            mapIDName mapTransport;

//            if (MTContactHandler::getInstance()->GetMsgMethodTypesByContact(mapTransport, nContactID, true)) // True means to add the OT servers as well.
//            {
//                for (mapIDName::iterator it_transport = mapTransport.begin(); it_transport != mapTransport.end(); it_transport++)
//                {
//                    QString qstrID   = it_transport.key();
////                  QString qstrName = it_transport.value();

//                    QStringList stringlist = qstrID.split("|");

//                    if (stringlist.size() >= 2) // Should always be 2...
//                    {
//                        QString qstrViaTransport = stringlist.at(1);
//                        QString qstrTransportName = qstrViaTransport;
//                        QString qstrMethodType = stringlist.at(0);
//                        QString qstrMethodName = qstrMethodType;

//                        mapIDName::iterator it_mapMethodTypes = mapMethodTypes.find(qstrMethodType);

//                        if (mapMethodTypes.end() != it_mapMethodTypes)
//                        {
//                            QString qstrTemp = it_mapMethodTypes.value();
//                            if (!qstrTemp.isEmpty())
//                                qstrMethodName = qstrTemp;
//                        }
//                        // ------------------------------------------------------
//                        else if (0 == QString("otserver").compare(qstrMethodType))
//                        {
//                            qstrMethodName = tr("Notary");
//                            // ------------------------------
//                            QString qstrTemp = QString::fromStdString(opentxs::OT::App().Client().Exec().GetServer_Name(qstrViaTransport.toStdString()));
//                            if (!qstrTemp.isEmpty())
//                                qstrTransportName = qstrTemp;
//                        }
//                        // ------------------------------------------------------
//                        QTreeWidgetItem * pAddressItem = new QTreeWidgetItem(pTopItem, QStringList(qstrContactName) << qstrMethodName << qstrTransportName);
//                        pAddressItem->setData(0, Qt::UserRole, QVariant(nContactID));
//                        pAddressItem->setData(1, Qt::UserRole, QVariant(qstrMethodType));
//                        pAddressItem->setData(2, Qt::UserRole, QVariant(qstrViaTransport));
//                        items.append(pAddressItem);
//                    }
//                }
//            }
//        }
//        if (items.count() > 0)
//        {
//            ui->treeWidget->insertTopLevelItems(0, items);
//            ui->treeWidget->resizeColumnToContents(0);
//        }
//    }
//    // ----------------------------------------
//    // Make sure the same item that was selected before, is selected again.
//    // (If it still exists, which it probably does.)

//    QTreeWidgetItem * previous = ui->treeWidget->currentItem();

//    // In this case, just select the first thing on the list.
//    if ( (0 == nCurrentContact_) && qstrMethodType_.isEmpty() && qstrViaTransport_.isEmpty() )
//    {
//        if (ui->treeWidget->topLevelItemCount() > 0)
//            ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(0));
//    }
//    else // Find the one that was selected before the refresh.
//    {
//        bool bFoundIt = false;

//        QTreeWidgetItemIterator it(ui->treeWidget);

//        while (*it)
//        {
//            QVariant qvarContactID    = (*it)->data(0, Qt::UserRole);
//            QVariant qvarMethodType   = (*it)->data(1, Qt::UserRole);
//            QVariant qvarViaTransport = (*it)->data(2, Qt::UserRole);

//            const int     nContactID       = qvarContactID   .isValid() ? qvarContactID   .toInt()    :  0;
//            const QString qstrMethodType   = qvarMethodType  .isValid() ? qvarMethodType  .toString() : "";
//            const QString qstrViaTransport = qvarViaTransport.isValid() ? qvarViaTransport.toString() : "";

//            if ( (nContactID == nCurrentContact_ ) &&
//                 (0 == qstrMethodType.compare(qstrMethodType_)) &&
//                 (0 == qstrViaTransport.compare(qstrViaTransport_)) )
//            {
//                bFoundIt = true;
//                ui->treeWidget->setCurrentItem(*it);
//                break;
//            }
//            // ------------------------
//            ++it;
//        } //while

//        if (!bFoundIt)
//        {
//            if (ui->treeWidget->topLevelItemCount() > 0)
//                ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(0));
//        }
//    }
//    // ----------------------------------------
//    ui->treeWidget->blockSignals(false);
//    // ----------------------------------------
//    on_treeWidget_currentItemChanged(ui->treeWidget->currentItem(), previous);
}

// --------------------------------------------------

void Payments::on_tableViewReceived_customContextMenuRequested(const QPoint &pos)
{
//    tableViewPopupMenu(pos, ui->tableViewReceived, &(*pPmntProxyModelInbox_));
}

void Payments::on_tableViewSent_customContextMenuRequested(const QPoint &pos)
{
//    tableViewPopupMenu(pos, ui->tableViewSent, &(*pPmntProxyModelOutbox_));
}

// --------------------------------------------------
// TODO resume: payments::AcceptIncoming, CancelOutgoing, DiscardOutgoingCash, and DiscardIncoming:

void Payments::AcceptIncoming(QPointer<ModelPayments> & pModel, PaymentsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView)
{
    emit showDashboard();
}

void Payments::CancelOutgoing(QPointer<ModelPayments> & pModel, PaymentsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView)
{
    emit showDashboard();
}

void Payments::DiscardOutgoingCash(QPointer<ModelPayments> & pModel, PaymentsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView)
{
    emit showDashboard();
}

void Payments::DiscardIncoming(QPointer<ModelPayments> & pModel, PaymentsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView)
{
    emit showDashboard();
}
// --------------------------------------------------

void Payments::tableViewPopupMenu(const QPoint &pos, QTableView * pTableView, PaymentsProxyModel * pProxyModel)
{
//    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

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
//    pActionOpenNewWindow = popupMenu_->addAction(tr("View instrument(s)"));
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
//    pActionDiscardOutgoingCash = nullptr;
//    pActionDiscardIncoming     = nullptr;
//    pActionDownloadCredentials = nullptr;

//    int nContactId = 0;

//    QString qstrSenderNymId;
//    QString qstrSenderAddr;
//    QString qstrRecipientNymId;
//    QString qstrRecipientAddr;
//    QString qstrMsgNotaryId;
//    QString qstrPmntNotaryId;
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
//        QModelIndex indexSenderNym     = pModel->index(nRow, PMNT_SOURCE_COL_SENDER_NYM);
//        QModelIndex indexSenderAddr    = pModel->index(nRow, PMNT_SOURCE_COL_SENDER_ADDR);
//        QModelIndex indexRecipientNym  = pModel->index(nRow, PMNT_SOURCE_COL_RECIP_NYM);
//        QModelIndex indexRecipientAddr = pModel->index(nRow, PMNT_SOURCE_COL_RECIP_ADDR);
//        QModelIndex indexMsgNotaryId   = pModel->index(nRow, PMNT_SOURCE_COL_MSG_NOTARY_ID);
//        QModelIndex indexPmntNotaryId  = pModel->index(nRow, PMNT_SOURCE_COL_PMNT_NOTARY_ID);
//        QModelIndex indexMethodType    = pModel->index(nRow, PMNT_SOURCE_COL_METHOD_TYPE);
//        QModelIndex indexFlags         = pModel->index(nRow, PMNT_SOURCE_COL_FLAGS);
////      QModelIndex indexSubject       = pModel->index(nRow, PMNT_SOURCE_COL_MEMO);

//        QVariant varSenderNym     = pModel->rawData(indexSenderNym);
//        QVariant varSenderAddr    = pModel->rawData(indexSenderAddr);
//        QVariant varRecipientNym  = pModel->rawData(indexRecipientNym);
//        QVariant varRecipientAddr = pModel->rawData(indexRecipientAddr);
//        QVariant varMsgNotaryId   = pModel->rawData(indexMsgNotaryId);
//        QVariant varPmntNotaryId  = pModel->rawData(indexPmntNotaryId);
//        QVariant varMethodType    = pModel->rawData(indexMethodType);
//        QVariant varFlags         = pModel->rawData(indexFlags);
////      QVariant varSubject       = pModel->rawData(indexSubject);

//        qint64 lFlags      = varFlags        .isValid() ? varFlags        .toLongLong() : 0;
//        qstrSenderNymId    = varSenderNym    .isValid() ? varSenderNym    .toString()   : QString("");
//        qstrSenderAddr     = varSenderAddr   .isValid() ? varSenderAddr   .toString()   : QString("");
//        qstrRecipientNymId = varRecipientNym .isValid() ? varRecipientNym .toString()   : QString("");
//        qstrRecipientAddr  = varRecipientAddr.isValid() ? varRecipientAddr.toString()   : QString("");
//        qstrMsgNotaryId    = varMsgNotaryId  .isValid() ? varMsgNotaryId  .toString()   : QString("");
//        qstrPmntNotaryId   = varPmntNotaryId .isValid() ? varPmntNotaryId .toString()   : QString("");
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
//            pActionViewContact = popupMenu_->addAction(tr("View contact in address book"));
//        else // There is no known contact for this payment.
//        {
//            pActionCreateContact = popupMenu_->addAction(tr("Create new contact in address book"));
//            pActionExistingContact = popupMenu_->addAction(tr("Add to existing contact in address book"));
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

//            if ( flags.testFlag(ModelPayments::IsTransfer) )
//            {
//                nameString = tr("Accept this Transfer");
//                actionString = tr("Accepting...");
//            }
//            else if ( flags.testFlag(ModelPayments::IsReceipt) )
//            {
//                nameString = tr("Accept this Receipt");
//                actionString = tr("Accepting...");
//            }
//            else if ( flags.testFlag(ModelPayments::IsInvoice) )
//            {
//                nameString = tr("Pay this Invoice");
//                actionString = tr("Paying...");
//            }
//            else if ( flags.testFlag(ModelPayments::IsPaymentPlan) )
//            {
//                nameString = tr("Activate this Payment Plan");
//                actionString = tr("Activating...");
//            }
//            else if ( flags.testFlag(ModelPayments::IsContract) )
//            {
//                nameString = tr("Sign this Smart Contract");
//                actionString = tr("Signing...");
//            }
//            else if ( flags.testFlag(ModelPayments::IsCash) )
//            {
//                nameString = tr("Deposit this Cash");
//                actionString = tr("Depositing...");
//            }
//            else if ( flags.testFlag(ModelPayments::IsCheque) )
//            {
//                nameString = tr("Deposit this Cheque");
//                actionString = tr("Depositing...");
//            }
//            else if ( flags.testFlag(ModelPayments::IsVoucher) )
//            {
//                nameString = tr("Accept this Payment");
//                actionString = tr("Accepting...");
//            }
//            else
//            {
//                nameString = tr("Deposit this Payment");
//                actionString = tr("Depositing...");
//            }

//            pActionAcceptIncoming = popupMenu_->addAction(nameString);
//        }

//        if (flags.testFlag(ModelPayments::CanCancelOutgoing))
//        {
//            QString cancelString;
//            QString actionString = tr("Canceling...");
////          QString msg = tr("Cancellation Failed. Perhaps recipient had already accepted it?");

//            if (flags.testFlag(ModelPayments::IsInvoice))
//                cancelString = tr("Cancel this Invoice");
//            else if (flags.testFlag(ModelPayments::IsPaymentPlan))
//                cancelString = tr("Cancel this Payment Plan");
//            else if (flags.testFlag(ModelPayments::IsContract))
//                cancelString = tr("Cancel this Smart Contract");
//            else if (flags.testFlag(ModelPayments::IsCash))
//            {
//                cancelString = tr("Recover this Cash");
//                actionString = tr("Recovering...");
////              msg = tr("Recovery Failed. Perhaps recipient had already accepted it?");
//            }
//            else if (flags.testFlag(ModelPayments::IsCheque))
//                cancelString = tr("Cancel this Cheque");
//            else if (flags.testFlag(ModelPayments::IsVoucher))
//                cancelString = tr("Cancel this Payment");
//            else
//                cancelString = tr("Cancel this Payment");

//            pActionCancelOutgoing = popupMenu_->addAction(cancelString);
//        }

//        if (flags.testFlag(ModelPayments::CanDiscardOutgoingCash))
//        {
//            QString discardString = tr("Discard this Sent Cash");

//            pActionDiscardOutgoingCash = popupMenu_->addAction(discardString);
//        }

//        if (flags.testFlag(ModelPayments::CanDiscardIncoming))
//        {
//            QString discardString;

//            if (flags.testFlag(ModelPayments::IsInvoice))
//                discardString = tr("Discard this Invoice");
//            else if (flags.testFlag(ModelPayments::IsPaymentPlan))
//                discardString = tr("Discard this Payment Plan");
//            else if (flags.testFlag(ModelPayments::IsContract))
//                discardString = tr("Discard this Smart Contract");
//            else if (flags.testFlag(ModelPayments::IsCash))
//                discardString = tr("Discard this Cash");
//            else if (flags.testFlag(ModelPayments::IsCheque))
//                discardString = tr("Discard this Cheque");
//            else if (flags.testFlag(ModelPayments::IsVoucher))
//                discardString = tr("Discard this Payment");
//            else
//                discardString = tr("Discard this Payment");

//            pActionDiscardIncoming = popupMenu_->addAction(discardString);
//        }
//    }
//    // --------------------------------------------------
//    QPoint globalPos = pTableView->mapToGlobal(pos);
//    const QAction* selectedAction = popupMenu_->exec(globalPos); // Here we popup the menu, and get the user's click.
//    if (nullptr == selectedAction)
//        return;
//    // ----------------------------------
//    if (selectedAction == pActionAcceptIncoming) // Only approves the current payment.
//    {
//        pTableView->setCurrentIndex(indexAtRightClick);
//        AcceptIncoming(pModel, pProxyModel, nRow, pTableView);
//        return;
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionCancelOutgoing) // Only cancels the current payment.
//    {
//        pTableView->setCurrentIndex(indexAtRightClick);
//        CancelOutgoing(pModel, pProxyModel, nRow, pTableView);
//        return;
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionDiscardOutgoingCash) // Only discards the current payment.
//    {
//        pTableView->setCurrentIndex(indexAtRightClick);
//        DiscardOutgoingCash(pModel, pProxyModel, nRow, pTableView);
//        return;
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionDiscardIncoming) // Only discards the current payment.
//    {
//        pTableView->setCurrentIndex(indexAtRightClick);
//        DiscardIncoming(pModel, pProxyModel, nRow, pTableView);
//        return;
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionReply) // Only replies to the current payment.
//    {
//        pTableView->setCurrentIndex(indexAtRightClick);
//        on_toolButtonReply_clicked();
//        return;
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionForward) // Only fowards the current payments.
//    {
//        pTableView->setCurrentIndex(indexAtRightClick);
//        on_toolButtonForward_clicked();
//        return;
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionDelete) // May delete many payments.
//    {
//        on_toolButtonDelete_clicked();
//        return;
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionOpenNewWindow) // May open many payments.
//    {
//        pTableView->setCurrentIndex(indexAtRightClick);

//        if (pTableView == ui->tableViewReceived)
//            on_tableViewReceived_doubleClicked(indexAtRightClick); // just one for now. baby steps!
//        else if (pTableView == ui->tableViewSent)
//            on_tableViewSent_doubleClicked(indexAtRightClick); // just one for now. baby steps!
//        return;
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionMarkRead) // May mark many payments.
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
//            QModelIndex sourceIndexHaveRead = pModel->sibling(sourceIndex.row(), PMNT_SOURCE_COL_HAVE_READ, sourceIndex);
//            // --------------------------------
//            if (sourceIndexHaveRead.isValid())
//                listRecordsToMarkAsRead_.append(sourceIndexHaveRead);
//        }
//        if (listRecordsToMarkAsRead_.count() > 0)
//            QTimer::singleShot(0, this, SLOT(on_MarkAsRead_timer()));
//        return;
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionMarkUnread) // May mark many payments.
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
//            QModelIndex sourceIndexHaveRead = pModel->sibling(sourceIndex.row(), PMNT_SOURCE_COL_HAVE_READ, sourceIndex);
//            // --------------------------------
//            if (sourceIndexHaveRead.isValid())
//                listRecordsToMarkAsUnread_.append(sourceIndexHaveRead);
//        }
//        if (listRecordsToMarkAsUnread_.count() > 0)
//            QTimer::singleShot(0, this, SLOT(on_MarkAsUnread_timer()));
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
//            nContactId = MTContactHandler::getInstance()->CreateContactBasedOnNym(qstrSenderNymId, qstrMsgNotaryId);
//        else if ((0 == nSenderContactByAddr) && !qstrSenderAddr.isEmpty())
//            nContactId = MTContactHandler::getInstance()->CreateContactBasedOnAddress(qstrSenderAddr, qstrMethodType);
//        else if ((0 == nRecipientContactByNym) && !qstrRecipientNymId.isEmpty())
//            nContactId = MTContactHandler::getInstance()->CreateContactBasedOnNym(qstrRecipientNymId, qstrMsgNotaryId);
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
//            emit needToCheckNym("", it_nyms.key(), qstrMsgNotaryId);
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
//                if (!qstrMsgNotaryId.isEmpty())
//                    MTContactHandler::getInstance()->NotifyOfNymServerPair(qstrNymId, qstrMsgNotaryId);
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


void Payments::on_tableViewReceived_doubleClicked(const QModelIndex &index)
{
    tableViewDoubleClicked(index, &(*pPmntProxyModelInbox_));
}

void Payments::on_tableViewSent_doubleClicked(const QModelIndex &index)
{
    tableViewDoubleClicked(index, &(*pPmntProxyModelOutbox_));
}

void Payments::tableViewDoubleClicked(const QModelIndex &index, PaymentsProxyModel * pProxyModel)
{
//    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

//    if (!pModel)
//        return;

//    if (!index.isValid())
//        return;

//    QModelIndex sourceIndex = pProxyModel->mapToSource(index);

//    if (!sourceIndex.isValid())
//        return;
//    // -------------------------------
//    QModelIndex pmntidIndex   = pModel->sibling(sourceIndex.row(), PMNT_SOURCE_COL_PMNT_ID, sourceIndex);
//    QModelIndex subjectIndex = pModel->sibling(sourceIndex.row(), PMNT_SOURCE_COL_MEMO, sourceIndex);

//    QVariant qvarpmntid   = pModel->data(pmntidIndex);
//    QVariant qvarSubject = pModel->data(subjectIndex);

//    int     nPaymentID  = qvarpmntid.isValid() ? qvarpmntid.toInt() : 0;
//    QString qstrSubject = qvarSubject.isValid() ? qvarSubject.toString() : "";
//    // -------------------------------
//    QString qstrPayment, qstrPending, qstrType, qstrSubtitle;
//    // --------------------------------------------------
//    if (nPaymentID > 0)
//    {
//        qstrPayment = MTContactHandler::getInstance()->GetPaymentBody(nPaymentID);
//        qstrPending = MTContactHandler::getInstance()->GetPaymentPendingBody(nPaymentID);
//    }
//    // --------------------------------------------------
//    QModelIndex myNymIndex        = pModel->sibling(sourceIndex.row(), PMNT_SOURCE_COL_MY_NYM, sourceIndex);
//    QModelIndex senderNymIndex    = pModel->sibling(sourceIndex.row(), PMNT_SOURCE_COL_SENDER_NYM, sourceIndex);
//    QModelIndex recipientNymIndex = pModel->sibling(sourceIndex.row(), PMNT_SOURCE_COL_RECIP_NYM, sourceIndex);

//    QModelIndex myNymProxyIndex        = pProxyModel->mapFromSource(myNymIndex);
//    QModelIndex senderNymProxyIndex    = pProxyModel->mapFromSource(senderNymIndex);
//    QModelIndex recipientNymProxyIndex = pProxyModel->mapFromSource(recipientNymIndex);

//    QVariant qvarMyNymName        = myNymProxyIndex.isValid()        ? pProxyModel->data(myNymProxyIndex) : QString("");
//    QVariant qvarSenderNymName    = senderNymProxyIndex.isValid()    ? pProxyModel->data(senderNymProxyIndex) : QString("");
//    QVariant qvarRecipientNymName = recipientNymProxyIndex.isValid() ? pProxyModel->data(recipientNymProxyIndex) : QString("");

//    QString qstrMyNymName        = qvarMyNymName.isValid() ? qvarMyNymName.toString() : "";
//    QString qstrSenderNymName    = qvarSenderNymName.isValid() ? qvarSenderNymName.toString() : "";
//    QString qstrRecipientNymName = qvarRecipientNymName.isValid() ? qvarRecipientNymName.toString() : "";

//    if (!qstrSenderNymName.isEmpty())
//    {
//        qstrType = QString("%1: %2").arg(tr("To")).arg(qstrMyNymName);
//        qstrSubtitle = QString("%1: %2").arg(tr("From")).arg(qstrSenderNymName);
//    }
//    else if (!qstrRecipientNymName.isEmpty())
//    {
//        qstrType = QString("%1: %2").arg(tr("To")).arg(qstrRecipientNymName);
//        qstrSubtitle = QString("%1: %2").arg(tr("From")).arg(qstrMyNymName);
//    }
//    else
//    {
//        qstrType = QString("Instrument:");
//        qstrSubtitle = QString(" ");
//    }
//    // -----------
//    // Pop up the result dialog.
//    //
//    if (qstrPayment.isEmpty() || qstrPending.isEmpty())
//    {
//        DlgExportedToPass dlgExported(this, qstrPayment.isEmpty() ? qstrPending : qstrPayment,
//                                      qstrType,
//                                      qstrSubtitle, false);
//        dlgExported.setWindowTitle(QString("%1: %2").arg(tr("Memo")).arg(qstrSubject));
//        dlgExported.exec();
//    }
//    else
//    {
//        DlgExportedCash dlgExported(this, qstrPending, qstrPayment,
//                                    tr("Receipt:"), QString(" "),
//                                    qstrType,
//                                    qstrSubtitle, false);
//        dlgExported.setWindowTitle(QString("%1: %2").arg(tr("Memo")).arg(qstrSubject));
//        dlgExported.exec();
//    }
}


void Payments::on_toolButtonReply_clicked()
{
//    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

//    if (!pModel)
//        return;
//    // ------------------------------------
//    QModelIndex proxyIndex = pCurrentTabTableView_->currentIndex();

//    if (!proxyIndex.isValid())
//        return;
//    // ------------------------------------
//    QModelIndex sourceIndex = pCurrentTabProxyModel_->mapToSource(proxyIndex);

//    if (!sourceIndex.isValid())
//        return;

//    QModelIndex haveRepliedIndex = pModel->sibling(sourceIndex.row(),
//                                                   PMNT_SOURCE_COL_HAVE_REPLIED,
//                                                   sourceIndex);
//    // ------------------------------------
//    QSqlRecord record = pModel->record(sourceIndex.row());

//    if (record.isEmpty())
//        return;
//    // ------------------------------------
//    const int nPaymentID = record.value(PMNT_SOURCE_COL_PMNT_ID).isValid() ? record.value(PMNT_SOURCE_COL_PMNT_ID).toInt() : 0;

//    const bool bOutgoing = (0 == record.value(PMNT_SOURCE_COL_FOLDER).toInt());

//    const QVariant qvar_method_type = record.value(PMNT_SOURCE_COL_METHOD_TYPE);
//    const QString  methodType = qvar_method_type.isValid() ? qvar_method_type.toString() : "";

//    const QVariant qvar_my_nym_id = record.value(PMNT_SOURCE_COL_MY_NYM);
//    const QString  myNymID = qvar_my_nym_id.isValid() ? qvar_my_nym_id.toString() : "";

//    const QVariant qvar_my_addr = record.value(PMNT_SOURCE_COL_MY_ADDR);
//    const QString  myAddress = qvar_my_addr.isValid() ? qvar_my_addr.toString() : "";

//    const QVariant qvar_sender_nym_id = record.value(PMNT_SOURCE_COL_SENDER_NYM);
//    const QString  senderNymID = qvar_sender_nym_id.isValid() ? qvar_sender_nym_id.toString() : "";

//    const QVariant qvar_recipient_nym_id = record.value(PMNT_SOURCE_COL_RECIP_NYM);
//    const QString  recipientNymID = qvar_recipient_nym_id.isValid() ? qvar_recipient_nym_id.toString() : "";

//    const QVariant qvar_msg_notary_id = record.value(PMNT_SOURCE_COL_MSG_NOTARY_ID);
//    const QVariant qvar_pmnt_notary_id = record.value(PMNT_SOURCE_COL_PMNT_NOTARY_ID);
//    const QString  MsgNotaryID = qvar_msg_notary_id.isValid() ? qvar_msg_notary_id.toString() : "";
//    const QString  PmntNotaryID = qvar_pmnt_notary_id.isValid() ? qvar_pmnt_notary_id.toString() : "";

//    const QVariant qvar_sender_addr = record.value(PMNT_SOURCE_COL_SENDER_ADDR);
//    const QString  senderAddr = qvar_sender_addr.isValid() ? qvar_sender_addr.toString() : "";

//    const QVariant qvar_recipient_addr = record.value(PMNT_SOURCE_COL_RECIP_ADDR);
//    const QString  recipientAddr = qvar_recipient_addr.isValid() ? qvar_recipient_addr.toString() : "";

//    const QVariant qvar_subject = record.value(PMNT_SOURCE_COL_MEMO);
//    const QString  subject = qvar_subject.isValid() ? MTContactHandler::getInstance()->Decode(qvar_subject.toString()) : "";
//    // --------------------------------------------------
//    const QString& otherNymID = bOutgoing ? recipientNymID : senderNymID;
//    const QString& otherAddress  = bOutgoing ? recipientAddr  : senderAddr;
//    // --------------------------------------------------
//    const bool bUsingNotary   = !MsgNotaryID.isEmpty();
//    const bool bIsSpecialMail = !bUsingNotary;
//    // --------------------------------------------------
//    MTCompose * compose_window = new MTCompose;
//    compose_window->setAttribute(Qt::WA_DeleteOnClose);
//    // --------------------------------------------------
//    if (!myNymID.isEmpty()) // If there's a nym ID.
//    {
//        if (!myAddress.isEmpty())
//            compose_window->setInitialSenderNym(myNymID, myAddress);
//        else
//            compose_window->setInitialSenderNym(myNymID);
//    }
//    else if (!myAddress.isEmpty())
//        compose_window->setInitialSenderAddress(myAddress);
//    // ---------------------------------------
//    if (!otherNymID.isEmpty()) // If there's an "other nym ID".
//    {
//        if (!otherAddress.isEmpty())
//            compose_window->setInitialRecipientNym(otherNymID, otherAddress);
//        else
//            compose_window->setInitialRecipientNym(otherNymID);
//    }
//    else if (!otherAddress.isEmpty())
//        compose_window->setInitialRecipientAddress(otherAddress);
//    // --------------------------------------------------
//    if (bUsingNotary)
//        compose_window->setInitialServer(MsgNotaryID);
//    // --------------------------------------------------
//    compose_window->setInitialSubject(subject);
//    // --------------------------------------------------
//    if (nPaymentID > 0)
//    {
//        QString body        = MTContactHandler::getInstance()->GetPaymentBody(nPaymentID);
//        QString pendingBody = MTContactHandler::getInstance()->GetPaymentPendingBody(nPaymentID);

//        if (!body.isEmpty())
//            compose_window->setInitialBody(body);
//        else if (!pendingBody.isEmpty())
//            compose_window->setInitialBody(pendingBody);
//    }
//    // --------------------------------------------------
//    compose_window->setVariousIds(
//                            bOutgoing ? myNymID : senderNymID,
//                            bOutgoing ? recipientNymID : myNymID,
//                            bOutgoing ? myAddress : senderAddr,
//                            bOutgoing ? recipientAddr : myAddress);
//    // --------------------------------------------------
//    compose_window->dialog();
//    Focuser f(compose_window);
//    f.show();
//    f.focus();
//    // -----------------------------
//    if (haveRepliedIndex.isValid())
//        listRecordsToMarkAsReplied_.append(haveRepliedIndex);
//    if (listRecordsToMarkAsReplied_.count() > 0)
//        QTimer::singleShot(0, this, SLOT(on_MarkAsReplied_timer()));
}

void Payments::on_toolButtonForward_clicked()
{
//    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

//    if (!pModel)
//        return;
//    // ------------------------------------
//    QModelIndex proxyIndex = pCurrentTabTableView_->currentIndex();

//    if (!proxyIndex.isValid())
//        return;
//    // ------------------------------------
//    QModelIndex sourceIndex = pCurrentTabProxyModel_->mapToSource(proxyIndex);

//    if (!sourceIndex.isValid())
//        return;

//    QModelIndex haveForwardedIndex = pModel->sibling(sourceIndex.row(),
//                                                     PMNT_SOURCE_COL_HAVE_FORWARDED,
//                                                     sourceIndex);
//    // ------------------------------------
//    QSqlRecord record = pModel->record(sourceIndex.row());

//    if (record.isEmpty())
//        return;
//    // ------------------------------------
//    const int nPaymentID = record.value(PMNT_SOURCE_COL_PMNT_ID).isValid() ? record.value(PMNT_SOURCE_COL_PMNT_ID).toInt() : 0;

//    const bool bOutgoing = (0 == record.value(PMNT_SOURCE_COL_FOLDER).toInt());

//    const QVariant qvar_method_type = record.value(PMNT_SOURCE_COL_METHOD_TYPE);
//    const QString  methodType = qvar_method_type.isValid() ? qvar_method_type.toString() : "";

//    const QVariant qvar_my_nym_id = record.value(PMNT_SOURCE_COL_MY_NYM);
//    const QString  myNymID = qvar_my_nym_id.isValid() ? qvar_my_nym_id.toString() : "";

//    const QVariant qvar_my_addr = record.value(PMNT_SOURCE_COL_MY_ADDR);
//    const QString  myAddress = qvar_my_addr.isValid() ? qvar_my_addr.toString() : "";

//    const QVariant qvar_sender_nym_id = record.value(PMNT_SOURCE_COL_SENDER_NYM);
//    const QString  senderNymID = qvar_sender_nym_id.isValid() ? qvar_sender_nym_id.toString() : "";

//    const QVariant qvar_recipient_nym_id = record.value(PMNT_SOURCE_COL_RECIP_NYM);
//    const QString  recipientNymID = qvar_recipient_nym_id.isValid() ? qvar_recipient_nym_id.toString() : "";

//    const QVariant qvar_msg_notary_id = record.value(PMNT_SOURCE_COL_MSG_NOTARY_ID);
//    const QVariant qvar_pmnt_notary_id = record.value(PMNT_SOURCE_COL_PMNT_NOTARY_ID);
//    const QString  MsgNotaryID = qvar_msg_notary_id.isValid() ? qvar_msg_notary_id.toString() : "";
//    const QString  PmntNotaryID = qvar_pmnt_notary_id.isValid() ? qvar_pmnt_notary_id.toString() : "";

//    const QVariant qvar_sender_addr = record.value(PMNT_SOURCE_COL_SENDER_ADDR);
//    const QString  senderAddr = qvar_sender_addr.isValid() ? qvar_sender_addr.toString() : "";

//    const QVariant qvar_recipient_addr = record.value(PMNT_SOURCE_COL_RECIP_ADDR);
//    const QString  recipientAddr = qvar_recipient_addr.isValid() ? qvar_recipient_addr.toString() : "";

//    const QVariant qvar_subject = record.value(PMNT_SOURCE_COL_MEMO);
//    const QString  subject = qvar_subject.isValid() ? MTContactHandler::getInstance()->Decode(qvar_subject.toString()) : "";
//    // --------------------------------------------------
//    const QString& otherNymID = bOutgoing ? recipientNymID : senderNymID;
//    const QString& otherAddress  = bOutgoing ? recipientAddr  : senderAddr;
//    // --------------------------------------------------
//    const bool bUsingNotary   = !MsgNotaryID.isEmpty();
//    const bool bIsSpecialMail = !bUsingNotary;
//    // --------------------------------------------------
//    MTCompose * compose_window = new MTCompose;
//    compose_window->setAttribute(Qt::WA_DeleteOnClose);
//    // --------------------------------------------------
//    if (!myNymID.isEmpty()) // If there's a nym ID.
//    {
//        if (!myAddress.isEmpty())
//            compose_window->setInitialSenderNym(myNymID, myAddress);
//        else
//            compose_window->setInitialSenderNym(myNymID);
//    }
//    else if (!myAddress.isEmpty())
//        compose_window->setInitialSenderAddress(myAddress);
//    // ---------------------------------------
////    if (!otherNymID.isEmpty()) // If there's an "other nym ID".
////    {
////        if (!otherAddress.isEmpty())
////            compose_window->setInitialRecipientNym(otherNymID, otherAddress);
////        else
////            compose_window->setInitialRecipientNym(otherNymID);
////    }
////    else if (!otherAddress.isEmpty())
////        compose_window->setInitialRecipientAddress(otherAddress);
//    // --------------------------------------------------
//    if (bUsingNotary)
//        compose_window->setInitialServer(MsgNotaryID);
//    // --------------------------------------------------
//    compose_window->setInitialSubject(subject);
//    // --------------------------------------------------
//    if (nPaymentID > 0)
//    {
//        QString body        = MTContactHandler::getInstance()->GetPaymentBody(nPaymentID);
//        QString pendingBody = MTContactHandler::getInstance()->GetPaymentPendingBody(nPaymentID);

//        if (!body.isEmpty())
//            compose_window->setInitialBody(body);
//        else if (!pendingBody.isEmpty())
//            compose_window->setInitialBody(pendingBody);
//    }
//    // --------------------------------------------------
//    compose_window->setForwarded();
//    // --------------------------------------------------
//    compose_window->setVariousIds(
//                            bOutgoing ? myNymID : senderNymID,
//                            bOutgoing ? recipientNymID : myNymID,
//                            bOutgoing ? myAddress : senderAddr,
//                            bOutgoing ? recipientAddr : myAddress);
//    // --------------------------------------------------
//    compose_window->dialog();
//    Focuser f(compose_window);
//    f.show();
//    f.focus();
//    // -----------------------------
//    if (haveForwardedIndex.isValid())
//        listRecordsToMarkAsForwarded_.append(haveForwardedIndex);
//    if (listRecordsToMarkAsForwarded_.count() > 0)
//        QTimer::singleShot(0, this, SLOT(on_MarkAsForwarded_timer()));
}



void Payments::on_toolButtonDelete_clicked()
{
//    if ( (nullptr != pCurrentTabTableView_) &&
//         (nullptr != pCurrentTabProxyModel_) )
//    {
//        if (!pCurrentTabTableView_->selectionModel()->hasSelection())
//            return;
//        // ----------------------------------------------
//        QMessageBox::StandardButton reply;

//        reply = QMessageBox::question(this, tr(MONEYCHANGER_APP_NAME), QString("%1<br/><br/>%2").arg(tr("Are you sure you want to delete these receipts?")).
//                                      arg(tr("WARNING: This is not reversible!")),
//                                      QMessageBox::Yes|QMessageBox::No);
//        if (reply != QMessageBox::Yes)
//            return;
//        // ----------------------------------------------
//        QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

//        if (pModel)
//        {
//            QItemSelection selection( pCurrentTabTableView_->selectionModel()->selection() );

//            int nFirstProxyRowRemoved = -1;
//            int nLastProxyRowRemoved  = -1;
//            int nCountRowsRemoved     = 0;

//            QList<int> rows, payment_ids;
//            foreach( const QModelIndex & index, selection.indexes() ) {
//                QModelIndex sourceIndex = pCurrentTabProxyModel_->mapToSource(index);
//                rows.append( sourceIndex.row() );
//                // --------------------------------
//                nLastProxyRowRemoved = index.row();
//                if ((-1) == nFirstProxyRowRemoved)
//                    nFirstProxyRowRemoved = index.row();
//            }

//            qSort( rows );

//            bool bRemoved = false;

//            int prev = -1;

//            for(int ii = rows.count() - 1; ii >= 0; ii -= 1 ) {
//               int current = rows[ii];
//               if( current != prev ) {
//                   bRemoved = true;
//                   QModelIndex sourceIndexpmntid = pModel->index(current, PMNT_SOURCE_COL_PMNT_ID);
//                   if (sourceIndexpmntid.isValid())
//                       payment_ids.append(pModel->data(sourceIndexpmntid).toInt());
//                   pModel->removeRows( current, 1 );
//                   prev = current;
//                   nCountRowsRemoved++;
//               }
//            }

//            if (bRemoved)
//            {
//                if (pModel->submitAll())
//                {
//                    pModel->database().commit();
//                    // ------------------------
//                    // Now we just deleted some receipts; let's delete also the corresponding
//                    // receipt contents. (We saved the deleted IDs for this purpose.)
//                    //
//                    for (int ii = 0; ii < payment_ids.count(); ++ii)
//                    {
//                        const int nPmntID = payment_ids[ii];

//                        if (nPmntID > 0)
//                            if (!MTContactHandler::getInstance()->DeletePaymentBody(nPmntID))
//                                qDebug() << "Payments::on_toolButtonDelete_clicked: Failed trying to delete payment body with payment_id: " << nPmntID << "\n";
//                    }
//                    // ------------------------
//                    // We just deleted the selected rows.
//                    // So now we need to choose another row to select.

//                    int nRowToSelect = -1;

//                    if ((nFirstProxyRowRemoved >= 0) && (nFirstProxyRowRemoved < pCurrentTabProxyModel_->rowCount()))
//                        nRowToSelect = nFirstProxyRowRemoved;
//                    else if (0 == nFirstProxyRowRemoved)
//                        nRowToSelect = 0;
//                    else if (nFirstProxyRowRemoved > 0)
//                        nRowToSelect = pCurrentTabProxyModel_->rowCount() - 1;
//                    else
//                        nRowToSelect = 0;

//                    if ((pCurrentTabProxyModel_->rowCount() > 0) && (nRowToSelect >= 0) &&
//                            (nRowToSelect < pCurrentTabProxyModel_->rowCount()))
//                    {
//                        QModelIndex previous = pCurrentTabTableView_->currentIndex();
//                        pCurrentTabTableView_->blockSignals(true);
//                        pCurrentTabTableView_->selectRow(nRowToSelect);
//                        pCurrentTabTableView_->blockSignals(false);

////                        if (pCurrentTabTableView_ == ui->tableViewReceived)
////                            on_tableViewReceivedSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
////                        else
////                            on_tableViewSentSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
//                    }
//                }
//                else
//                {
//                    pModel->database().rollback();
//                    qDebug() << "Database Write Error" <<
//                               "The database reported an error: " <<
//                               pModel->lastError().text();
//                }
//            }
//        }
//    }
}

void Payments::on_toolButtonRefresh_clicked()
{
    emit needToDownloadAccountData();
}

void Payments::onRecordlistPopulated()
{
    RefreshAll();
}

void Payments::RefreshAll()
{
    RefreshUserBar();
    RefreshTree();  // Deprecated in favor of the below.

    RefreshSummaryTree();
}

void Payments::onBalancesChanged()
{
    RefreshUserBar();
}

// The balances hasn't necessarily changed.
// (Perhaps the default account was changed.)
//
void Payments::onNeedToRefreshUserBar()
{
    RefreshUserBar();
}

void Payments::onNeedToRefreshRecords()
{
    RefreshAll();
}


void Payments::on_pushButtonSearch_clicked()
{
//    QString qstrSearchText = ui->lineEdit->text();
//    this->doSearch(qstrSearchText.simplified());
}

void Payments::doSearch(QString qstrInput)
{
    if (pPmntProxyModelInbox_)
    {
        pPmntProxyModelInbox_ ->setFilterString(qstrInput);
    }
    if (pPmntProxyModelOutbox_)
    {
        pPmntProxyModelOutbox_->setFilterString(qstrInput);
    }

    RefreshPayments();
}

void Payments::on_lineEdit_textChanged(const QString &arg1)
{
    // This means someone clicked the "clear" button on the search box.
    if (arg1.isEmpty())
        doSearch(arg1);
}

void Payments::on_lineEdit_returnPressed()
{
//    QString qstrSearchText = ui->lineEdit->text();
//    this->doSearch(qstrSearchText.simplified());
}

Payments::~Payments()
{
    delete ui;
}

bool Payments::eventFilter(QObject *obj, QEvent *event)
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




