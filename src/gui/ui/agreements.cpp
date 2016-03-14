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
//#include <core/handlers/modelpayments.hpp>
#include <core/handlers/focuser.h>

#include <opentxs/client/OTAPI.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>
#include <opentxs/core/Log.hpp>
#include <opentxs/core/OTTransaction.hpp>
#include <opentxs/core/OTTransactionType.hpp>
#include <opentxs/client/OTRecordList.hpp>

#include <QLabel>
#include <QDebug>
#include <QToolButton>
#include <QKeyEvent>
#include <QApplication>
#include <QMessageBox>
#include <QMenu>
#include <QList>
#include <QSqlRecord>
#include <QTimer>
#include <QFrame>

#include <string>
#include <map>


Agreements::Agreements(QWidget *parent) :
    QWidget(parent, Qt::Window),
    ui(new Ui::Agreements)
{
    ui->setupUi(this);

    this->installEventFilter(this);

//    connect(ui->toolButtonPay,      SIGNAL(clicked()), Moneychanger::It(), SLOT(mc_sendfunds_slot()));
//    connect(ui->toolButtonContacts, SIGNAL(clicked()), Moneychanger::It(), SLOT(mc_addressbook_slot()));
    connect(this, SIGNAL(needToCheckNym(QString, QString, QString)), Moneychanger::It(), SLOT(onNeedToCheckNym(QString, QString, QString)));

}

Agreements::~Agreements()
{
    delete ui;
}



static void setup_agreement_tableview(QTableView * pView, QAbstractItemModel * pProxyModel)
{
    AgreementsProxyModel * pTheProxyModel = static_cast<AgreementsProxyModel *>(pProxyModel);
    pTheProxyModel->setTableView(pView);

    pView->setModel(pProxyModel);
    pView->setSortingEnabled(true);
    pView->resizeColumnsToContents();
    pView->resizeRowsToContents();
    pView->horizontalHeader()->setStretchLastSection(true);
    pView->setEditTriggers(QAbstractItemView::NoEditTriggers);

//    QPointer<ModelAgreements> pSourceModel = DBHandler::getInstance()->getAgreementModel();

//    if (pSourceModel)
    {
//        QModelIndex sourceIndex = pSourceModel->index(0, PMNT_SOURCE_COL_TIMESTAMP);
//        QModelIndex proxyIndex  = (static_cast<AgreementsProxyModel *>(pProxyModel)) -> mapFromSource(sourceIndex);
        // ----------------------------------
//        AgreementsProxyModel * pProxyModel = static_cast<AgreementsProxyModel *>(pProxyModel);


      pView->sortByColumn(7, Qt::DescendingOrder); // The timestamp ends up at index 7 in all the proxy views.

//        qDebug() << "SORT COLUMN: " << proxyIndex.column() << "\n";

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

    pView->setModel(pProxyModel);
    pView->setSortingEnabled(true);
    pView->resizeColumnsToContents();
    pView->resizeRowsToContents();
    pView->horizontalHeader()->setStretchLastSection(true);
    pView->setEditTriggers(QAbstractItemView::NoEditTriggers);

//    QPointer<ModelAgreementReceipt> pSourceModel = DBHandler::getInstance()->getAgreementReceiptModel();

//    if (pSourceModel)
    {
//        QModelIndex sourceIndex = pSourceModel->index(0, PMNT_SOURCE_COL_TIMESTAMP);
//        QModelIndex proxyIndex  = (static_cast<AgreementReceiptsProxyModel *>(pProxyModel)) -> mapFromSource(sourceIndex);
        // ----------------------------------
//        AgreementReceiptsProxyModel * pProxyModel = static_cast<AgreementReceiptsProxyModel *>(pProxyModel);


      pView->sortByColumn(7, Qt::DescendingOrder); // The timestamp ends up at index 7 in all the proxy views.

//        qDebug() << "SORT COLUMN: " << proxyIndex.column() << "\n";

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
        std::string str_acct_nym    = opentxs::OTAPI_Wrap::It()->GetAccountWallet_NymID(str_acct_id);
        std::string str_acct_server = opentxs::OTAPI_Wrap::It()->GetAccountWallet_NotaryID(str_acct_id);
        std::string str_acct_asset  = opentxs::OTAPI_Wrap::It()->GetAccountWallet_InstrumentDefinitionID(str_acct_id);
        // -----------------------------------
        qstr_acct_nym    = QString::fromStdString(str_acct_nym);
        qstr_acct_server = QString::fromStdString(str_acct_server);
        qstr_acct_asset  = QString::fromStdString(str_acct_asset);
        // -----------------------------------
        std::string str_tla = opentxs::OTAPI_Wrap::It()->GetCurrencyTLA(str_acct_asset);
        qstr_tla = QString("<font color=grey>%1</font>").arg(QString::fromStdString(str_tla));

        qstr_balance = MTHome::shortAcctBalance(qstr_acct_id, qstr_acct_asset, false);
        // -----------------------------------
        std::string str_acct_name  = opentxs::OTAPI_Wrap::It()->GetAccountWallet_Name(str_acct_id);
        // -----------------------------------
        if (!str_acct_asset.empty())
        {
            std::string str_asset_name = opentxs::OTAPI_Wrap::It()->GetAssetType_Name(str_acct_asset);
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
        payment_code = opentxs::OTAPI_Wrap::It()->GetNym_Description(qstr_acct_nym.toStdString());
        qstrPaymentCode = QString::fromStdString(payment_code);
        // ----------------------------
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
    if (ui->tableViewRecurring && ui->tableViewSmartContracts &&
        pProxyModelRecurring_  && pProxyModelSmartContracts_)
    {
        pCurrentAgreementTableView_  = (0 == ui->tabWidgetAgreements->currentIndex()) ? ui->tableViewRecurring    : ui->tableViewSmartContracts;
        pCurrentAgreementProxyModel_ = (0 == ui->tabWidgetAgreements->currentIndex()) ? &(*pProxyModelRecurring_) : &(*pProxyModelSmartContracts_);
        // -------------------------------------------------
        QModelIndex the_index  = pCurrentAgreementTableView_->currentIndex();

        if (the_index.isValid())
            enableButtons();
        else
            disableButtons();
        // --------------------------------------
        RefreshAgreements();
    }
    else
    {
        pCurrentAgreementTableView_  = nullptr;
        pCurrentAgreementProxyModel_ = nullptr;

        disableButtons();
    }
}

void Agreements::on_tabWidgetReceipts_currentChanged(int index)
{
    if (ui->tableViewSent         && ui->tableViewReceived &&
        pReceiptProxyModelOutbox_ && pReceiptProxyModelInbox_)
    {
        pCurrentReceiptTableView_  = (0 == ui->tabWidgetReceipts->currentIndex()) ? ui->tableViewReceived    : ui->tableViewSent;
        pCurrentReceiptProxyModel_ = (0 == ui->tabWidgetReceipts->currentIndex()) ? &(*pReceiptProxyModelInbox_) : &(*pReceiptProxyModelOutbox_);
        // -------------------------------------------------
        QModelIndex the_index  = pCurrentReceiptTableView_->currentIndex();

        if (the_index.isValid())
            enableButtons();
        else
            disableButtons();
        // --------------------------------------
        RefreshReceipts();
    }
    else
    {
        pCurrentReceiptTableView_  = nullptr;
        pCurrentReceiptProxyModel_ = nullptr;

        disableButtons();
    }
}


void Agreements::enableButtons()
{
    ui->toolButtonDelete ->setEnabled(true);
}

void Agreements::disableButtons()
{
    ui->toolButtonDelete ->setEnabled(false);
}

//void Agreements::on_MarkAsRead_timer()
//{
//    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getAgreementModel();

//    if (!pModel)
//        return;
//    // ------------------------------
//    bool bEditing = false;

//    while (!listRecordsToMarkAsRead_.isEmpty())
//    {
//        QModelIndex index = listRecordsToMarkAsRead_.front();
//        listRecordsToMarkAsRead_.pop_front();
//        // ------------------------------------
//        if (!index.isValid())
//            continue;
//        // ------------------------------------
//        if (!bEditing)
//        {
//            bEditing = true;
//            pModel->database().transaction();
//        }
//        // ------------------------------------
//        pModel->setData(index, QVariant(1)); // 1 for "true" in sqlite. "Yes, we've now read this payment. Mark it as read."
//    } // while
//    // ------------------------------
//    if (bEditing)
//    {
//        if (pModel->submitAll())
//        {
//            pModel->database().commit();
//            // ------------------------------------
//            QTimer::singleShot(0, this, SLOT(RefreshPayments()));
//        }
//        else
//        {
//            pModel->database().rollback();
//            qDebug() << "Database Write Error" <<
//                       "The database reported an error: " <<
//                       pModel->lastError().text();
//        }
//    }
//}

//void Agreements::on_MarkAsUnread_timer()
//{
//    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getAgreementModel();

//    if (!pModel)
//        return;
//    // ------------------------------
//    bool bEditing = false;

//    while (!listRecordsToMarkAsUnread_.isEmpty())
//    {
//        QModelIndex index = listRecordsToMarkAsUnread_.front();
//        listRecordsToMarkAsUnread_.pop_front();
//        // ------------------------------------
//        if (!index.isValid())
//            continue;
//        // ------------------------------------
//        if (!bEditing)
//        {
//            bEditing = true;
//            pModel->database().transaction();
//        }
//        // ------------------------------------
//        pModel->setData(index, QVariant(0)); // 0 for "false" in sqlite. "This payment is now marked UNREAD."
//    } // while
//    // ------------------------------
//    if (bEditing)
//    {
//        if (pModel->submitAll())
//        {
//            pModel->database().commit();
//            // ------------------------------------
//            QTimer::singleShot(0, this, SLOT(RefreshPayments()));
//        }
//        else
//        {
//            pModel->database().rollback();
//            qDebug() << "Database Write Error" <<
//                       "The database reported an error: " <<
//                       pModel->lastError().text();
//        }
//    }
//}




void Agreements::on_tableViewRecurringSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous)
{
    // resume todo

    // Set a Filter based on which one is clicked.


}

void Agreements::on_tableViewSmartContractsSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous)
{

}

void Agreements::on_tableViewPartiesSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous)
{

}

void Agreements::on_tableViewSentSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous)
{
    if (!current.isValid())
    {
        disableButtons();
        // ----------------------------------------
    }
    else
    {
        enableButtons();
        // ----------------------------------------
        QPointer<ModelAgreementReceipts> pModel = DBHandler::getInstance()->getAgreementReceiptModel();

        if (pModel)
        {
            QModelIndex sourceIndex = pReceiptProxyModelOutbox_->mapToSource(current);

            QModelIndex haveReadSourceIndex  = pReceiptProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_HAVE_READ, sourceIndex);
            QModelIndex agreementIdSourceIndex    = pReceiptProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_AGRMT_ID, sourceIndex);
//            QModelIndex subjectSourceIndex   = pReceiptProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_MEMO,      sourceIndex);
//            QModelIndex senderSourceIndex    = pReceiptProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_MY_NYM,    sourceIndex);
//            QModelIndex recipientSourceIndex = pReceiptProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_RECIP_NYM, sourceIndex);
//            QModelIndex timestampSourceIndex = pReceiptProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_TIMESTAMP, sourceIndex);

//            QModelIndex subjectIndex      = pPmntProxyModelOutbox_->mapFromSource(subjectSourceIndex);
//            QModelIndex senderIndex       = pPmntProxyModelOutbox_->mapFromSource(senderSourceIndex);
//            QModelIndex recipientIndex    = pPmntProxyModelOutbox_->mapFromSource(recipientSourceIndex);
//            QModelIndex timestampIndex    = pPmntProxyModelOutbox_->mapFromSource(timestampSourceIndex);

            QVariant varagrmtid   = pModel->data(agreementIdSourceIndex);
            QVariant varHaveRead  = pModel->data(haveReadSourceIndex);
//            QVariant varSubject   = pPmntProxyModelOutbox_->data(subjectIndex);
//            QVariant varSender    = pPmntProxyModelOutbox_->data(senderIndex);
//            QVariant varRecipient = pPmntProxyModelOutbox_->data(recipientIndex);
//            QVariant varTimestamp = pPmntProxyModelOutbox_->data(timestampIndex);

//            QString qstrSubject   = varSubject.isValid()   ? varSubject.toString()   : "";
//            QString qstrSender    = varSender.isValid()    ? varSender.toString()    : "";
//            QString qstrRecipient = varRecipient.isValid() ? varRecipient.toString() : "";
//            QString qstrTimestamp = varTimestamp.isValid() ? varTimestamp.toString() : "";
            // ----------------------------------------------------------
            int agreement_id = varagrmtid.isValid() ? varagrmtid.toInt() : 0;
            const bool bHaveRead = varHaveRead.isValid() ? varHaveRead.toBool() : false;

            if (!bHaveRead && (agreement_id > 0)) // It's unread, so we need to set it as read.
            {
                listRecordsToMarkAsRead_.append(haveReadSourceIndex);
                QTimer::singleShot(1000, this, SLOT(on_MarkAsRead_timer()));
            }
        }
    }
}

void Agreements::on_tableViewReceivedSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous)
{
    if (!current.isValid())
    {
        disableButtons();
    }
    else
    {
        enableButtons();
        // ----------------------------------------
        QPointer<ModelAgreementReceipts> pModel = DBHandler::getInstance()->getAgreementReceiptModel();

        if (pModel)
        {
            QModelIndex sourceIndex = pReceiptProxyModelInbox_->mapToSource(current);

            QModelIndex haveReadSourceIndex  = pReceiptProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_HAVE_READ,  sourceIndex);
            QModelIndex agreementIdSourceIndex    = pReceiptProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_AGRMT_ID, sourceIndex);
//            QModelIndex subjectSourceIndex   = pReceiptProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_MEMO,       sourceIndex);
//            QModelIndex senderSourceIndex    = pReceiptProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_SENDER_NYM, sourceIndex);
//            QModelIndex recipientSourceIndex = pReceiptProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_MY_NYM,     sourceIndex);
//            QModelIndex timestampSourceIndex = pReceiptProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), AGRMT_RECEIPT_COL_TIMESTAMP,  sourceIndex);

//            QModelIndex subjectIndex      = pReceiptProxyModelInbox_->mapFromSource(subjectSourceIndex);
//            QModelIndex senderIndex       = pReceiptProxyModelInbox_->mapFromSource(senderSourceIndex);
//            QModelIndex recipientIndex    = pReceiptProxyModelInbox_->mapFromSource(recipientSourceIndex);
//            QModelIndex timestampIndex    = pReceiptProxyModelInbox_->mapFromSource(timestampSourceIndex);

            QVariant varagrmtid   = pModel->data(agreementIdSourceIndex);
            QVariant varHaveRead  = pModel->data(haveReadSourceIndex);
//            QVariant varSubject   = pReceiptProxyModelInbox_->data(subjectIndex);
//            QVariant varSender    = pReceiptProxyModelInbox_->data(senderIndex);
//            QVariant varRecipient = pReceiptProxyModelInbox_->data(recipientIndex);
//            QVariant varTimestamp = pReceiptProxyModelInbox_->data(timestampIndex);

//            QString qstrSubject   = varSubject.isValid()   ? varSubject  .toString() : "";
//            QString qstrSender    = varSender   .isValid() ? varSender   .toString() : "";
//            QString qstrRecipient = varRecipient.isValid() ? varRecipient.toString() : "";
//            QString qstrTimestamp = varTimestamp.isValid() ? varTimestamp.toString() : "";

//            ui->headerReceived->setSubject  (qstrSubject);
//            ui->headerReceived->setSender   (qstrSender);
//            ui->headerReceived->setRecipient(qstrRecipient);
//            ui->headerReceived->setTimestamp(qstrTimestamp);
//            ui->headerReceived->setFolder(tr("Received"));

            // ----------------------------------------------------------
            int agreement_id = varagrmtid.isValid() ? varagrmtid.toInt() : 0;
            const bool bHaveRead = varHaveRead.isValid() ? varHaveRead.toBool() : false;

            if (!bHaveRead && (agreement_id > 0)) // It's unread, so we need to set it as read.
            {
                listRecordsToMarkAsRead_.append(haveReadSourceIndex);
                QTimer::singleShot(1000, this, SLOT(on_MarkAsRead_timer()));
            }
        }
    }
}



void Agreements::dialog(int nSourceRow/*=-1*/, int nFolder/*=-1*/)
{
    if (!already_init)
    {
        ui->userBar->setStyleSheet("QWidget{background-color:#c0cad4;selection-background-color:#a0aac4;}");
        // ----------------------------------
        QPixmap pixmapSize(":/icons/icons/user.png"); // This is here only for size purposes.
        QIcon   sizeButtonIcon(pixmapSize);           // Other buttons use this to set their own size.
        // ----------------------------------------------------------------
        QPixmap pixmapRefresh (":/icons/icons/refresh.png");
        QPixmap pixmapDelete  (":/icons/icons/DeleteRed.png");
        // ----------------------------------------------------------------
        QIcon refreshButtonIcon (pixmapRefresh);
        QIcon deleteButtonIcon  (pixmapDelete);
        // ----------------------------------------------------------------
        ui->toolButtonRefresh->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->toolButtonRefresh->setAutoRaise(true);
        ui->toolButtonRefresh->setIcon(refreshButtonIcon);
//      ui->toolButtonRefresh->setIconSize(pixmapRefresh.rect().size());
        ui->toolButtonRefresh->setIconSize(pixmapSize.rect().size());
        ui->toolButtonRefresh->setText(tr("Refresh"));
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
        QWidget* pTab0 = ui->tabWidgetAgreements->widget(0);
        QWidget* pTab1 = ui->tabWidgetAgreements->widget(1);
        QWidget* pTab2 = ui->tabWidgetAgreements->widget(0);
        QWidget* pTab3 = ui->tabWidgetAgreements->widget(1);

        pTab0->setStyleSheet("QWidget { margin: 0 }");
        pTab1->setStyleSheet("QWidget { margin: 0 }");
        pTab2->setStyleSheet("QWidget { margin: 0 }");
        pTab3->setStyleSheet("QWidget { margin: 0 }");

//        ui->splitter->setStretchFactor(0, 2);
//        ui->splitter->setStretchFactor(1, 3);

//        ui->splitter->setStretchFactor(0, 1);
//        ui->splitter->setStretchFactor(1, 5);
        // ------------------------
        on_tabWidgetAgreements_currentChanged(0);

        /** Flag Already Init **/
        already_init = true;
    }
    // -------------------------------------------
    RefreshAll();

    Focuser f(this);
    f.show();
    f.focus();
    // -------------------------------------------
    setAsCurrentAgreement(nSourceRow, nFolder);
}

//resume
void Agreements::setAsCurrentAgreement(int nSourceRow, int nFolder) // The smart contract itself, or payment plan, that's selected.
{



}

void Agreements::setAsCurrentParty(int nSourceRow) // You may have multiple signer Nyms on the same agreement, in the same wallet
{

}

void Agreements::setAsCurrentPayment(int nSourceRow, int nFolder) // the recurring paymentReceipts for each smart contract or payment plan.
{

}

//void Agreements::setAsCurrentPayment(int nSourceRow, int nFolder)
//{
//    if (-1 == nSourceRow || -1 == nFolder)
//        return;

//    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getAgreementModel();

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
//}



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
//    bRefreshingAfterUpdatedClaims_ = false;
//    // -------------------------------------------

    QPointer<ModelAgreements> pModel = DBHandler::getInstance()->getAgreementModel();

    if (pModel)
    {
        pModel->select();
    }
    ui->tableViewRecurring->reset();
    ui->tableViewSmartContracts->reset();
    ui->tableViewSent->reset();
    ui->tableViewReceived->reset();
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

//        QPointer<ModelAgreements> pModel = DBHandler::getInstance()->getAgreementModel();

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

//    ui->tableViewSent->horizontalHeader()->setStretchLastSection(true);
//    ui->tableViewReceived->horizontalHeader()->setStretchLastSection(true);

    ui->tableViewRecurring->resizeColumnsToContents();
    ui->tableViewRecurring->resizeRowsToContents();

    ui->tableViewSmartContracts->resizeColumnsToContents();
    ui->tableViewSmartContracts->resizeRowsToContents();

    ui->tableViewSent->resizeColumnsToContents();
    ui->tableViewSent->resizeRowsToContents();

    ui->tableViewReceived->resizeColumnsToContents();
    ui->tableViewReceived->resizeRowsToContents();


    ui->tableViewRecurring->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewSmartContracts->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewSent->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewReceived->horizontalHeader()->setStretchLastSection(true);

}

void Agreements::RefreshReceipts()
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

//        QPointer<ModelPayments> pModel = DBHandler::getInstance()->getAgreementModel();

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

//    ui->tableViewSent->horizontalHeader()->setStretchLastSection(true);
//    ui->tableViewReceived->horizontalHeader()->setStretchLastSection(true);
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

}

void Agreements::tableViewReceiptsPopupMenu(const QPoint &pos, QTableView * pTableView, AgreementReceiptsProxyModel * pProxyModel)
{
//    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getAgreementModel();

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
//    QString qstrNotaryId;
//    QString qstrMethodType;
////  QString qstrSubject;

//    int nSenderContactByNym     = 0;
//    int nSenderContactByAddr    = 0;
//    int nRecipientContactByNym  = 0;
//    int nRecipientContactByAddr = 0;

//    ModelPayments::PaymentFlags flags = ModelAgreements::NoFlags;
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
//        QModelIndex indexNotaryId      = pModel->index(nRow, PMNT_SOURCE_COL_NOTARY_ID);
//        QModelIndex indexMethodType    = pModel->index(nRow, PMNT_SOURCE_COL_METHOD_TYPE);
//        QModelIndex indexFlags         = pModel->index(nRow, PMNT_SOURCE_COL_FLAGS);
////      QModelIndex indexSubject       = pModel->index(nRow, PMNT_SOURCE_COL_MEMO);

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

//        flags = ModelAgreements::PaymentFlag(static_cast<ModelAgreements::PaymentFlag>(lFlags));
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
//        if ( flags.testFlag(ModelAgreements::CanAcceptIncoming))
//        {
//            QString nameString;
//            QString actionString;

//            if ( flags.testFlag(ModelAgreements::IsTransfer) )
//            {
//                nameString = tr("Accept this Transfer");
//                actionString = tr("Accepting...");
//            }
//            else if ( flags.testFlag(ModelAgreements::IsReceipt) )
//            {
//                nameString = tr("Accept this Receipt");
//                actionString = tr("Accepting...");
//            }
//            else if ( flags.testFlag(ModelAgreements::IsInvoice) )
//            {
//                nameString = tr("Pay this Invoice");
//                actionString = tr("Paying...");
//            }
//            else if ( flags.testFlag(ModelAgreements::IsPaymentPlan) )
//            {
//                nameString = tr("Activate this Payment Plan");
//                actionString = tr("Activating...");
//            }
//            else if ( flags.testFlag(ModelAgreements::IsContract) )
//            {
//                nameString = tr("Sign this Smart Contract");
//                actionString = tr("Signing...");
//            }
//            else if ( flags.testFlag(ModelAgreements::IsCash) )
//            {
//                nameString = tr("Deposit this Cash");
//                actionString = tr("Depositing...");
//            }
//            else if ( flags.testFlag(ModelAgreements::IsCheque) )
//            {
//                nameString = tr("Deposit this Cheque");
//                actionString = tr("Depositing...");
//            }
//            else if ( flags.testFlag(ModelAgreements::IsVoucher) )
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

//        if (flags.testFlag(ModelAgreements::CanCancelOutgoing))
//        {
//            QString cancelString;
//            QString actionString = tr("Canceling...");
////          QString msg = tr("Cancellation Failed. Perhaps recipient had already accepted it?");

//            if (flags.testFlag(ModelAgreements::IsInvoice))
//                cancelString = tr("Cancel this Invoice");
//            else if (flags.testFlag(ModelAgreements::IsPaymentPlan))
//                cancelString = tr("Cancel this Payment Plan");
//            else if (flags.testFlag(ModelAgreements::IsContract))
//                cancelString = tr("Cancel this Smart Contract");
//            else if (flags.testFlag(ModelAgreements::IsCash))
//            {
//                cancelString = tr("Recover this Cash");
//                actionString = tr("Recovering...");
////              msg = tr("Recovery Failed. Perhaps recipient had already accepted it?");
//            }
//            else if (flags.testFlag(ModelAgreements::IsCheque))
//                cancelString = tr("Cancel this Cheque");
//            else if (flags.testFlag(ModelAgreements::IsVoucher))
//                cancelString = tr("Cancel this Payment");
//            else
//                cancelString = tr("Cancel this Payment");

//            pActionCancelOutgoing = popupMenu_->addAction(cancelString);
//        }

//        if (flags.testFlag(ModelAgreements::CanDiscardOutgoingCash))
//        {
//            QString discardString = tr("Discard this Sent Cash");

//            pActionDiscardOutgoingCash = popupMenu_->addAction(discardString);
//        }

//        if (flags.testFlag(ModelAgreements::CanDiscardIncoming))
//        {
//            QString discardString;

//            if (flags.testFlag(ModelAgreements::IsInvoice))
//                discardString = tr("Discard this Invoice");
//            else if (flags.testFlag(ModelAgreements::IsPaymentPlan))
//                discardString = tr("Discard this Payment Plan");
//            else if (flags.testFlag(ModelAgreements::IsContract))
//                discardString = tr("Discard this Smart Contract");
//            else if (flags.testFlag(ModelAgreements::IsCash))
//                discardString = tr("Discard this Cash");
//            else if (flags.testFlag(ModelAgreements::IsCheque))
//                discardString = tr("Discard this Cheque");
//            else if (flags.testFlag(ModelAgreements::IsVoucher))
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
//            QMessageBox::warning(this, tr("Moneychanger"), tr("Unable to find a NymId for this message. (Unable to download credentials without Id.)"));
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
//            QMessageBox::warning(this, tr("Moneychanger"),
//                                 tr("Strange: NymID %1 already belongs to an existing contact.").arg(qstrNymId));
//            return;
//        }
//        // ---------------------------------------------------
//        if (!qstrAddress.isEmpty() && MTContactHandler::getInstance()->GetContactByAddress(qstrAddress) > 0)
//        {
//            QMessageBox::warning(this, tr("Moneychanger"),
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
//                    QMessageBox::warning(this, tr("Moneychanger"), QString("Failed while trying to add NymID %1 to existing contact '%2' with contact ID: %3").
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
//                    QMessageBox::warning(this, tr("Moneychanger"), QString("Failed while trying to add Address %1 to existing contact '%2' with contact ID: %3").
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

}

void Agreements::tableViewReceiptsDoubleClicked(const QModelIndex &index, AgreementReceiptsProxyModel * pProxyModel)
{
//    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getAgreementModel();

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
//        qstrPayment = MTContactHandler::getInstance()->GetAgreementReceiptBody(nPaymentID);
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
//                                    tr("Deposit receipt:"), QString(" "),
//                                    qstrType,
//                                    qstrSubtitle, false);
//        dlgExported.setWindowTitle(QString("%1: %2").arg(tr("Memo")).arg(qstrSubject));
//        dlgExported.exec();
//    }
}




void Agreements::on_toolButtonDelete_clicked()
{
//    if ( (nullptr != pCurrentTabTableView_) &&
//         (nullptr != pCurrentTabProxyModel_) )
//    {
//        if (!pCurrentTabTableView_->selectionModel()->hasSelection())
//            return;
//        // ----------------------------------------------
//        QMessageBox::StandardButton reply;

//        reply = QMessageBox::question(this, tr("Moneychanger"), QString("%1<br/><br/>%2").arg(tr("Are you sure you want to delete these receipts?")).
//                                      arg(tr("WARNING: This is not reversible!")),
//                                      QMessageBox::Yes|QMessageBox::No);
//        if (reply != QMessageBox::Yes)
//            return;
//        // ----------------------------------------------
//        QPointer<ModelPayments> pModel = DBHandler::getInstance()->getAgreementModel();

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
//                            if (!MTContactHandler::getInstance()->DeleteAgreementReceiptBody(nPmntID))
//                                qDebug() << "Agreements::on_toolButtonDelete_clicked: Failed trying to delete payment body with payment_id: " << nPmntID << "\n";
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

//                        if (pCurrentTabTableView_ == ui->tableViewReceived)
//                            on_tableViewReceivedSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
//                        else
//                            on_tableViewSentSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
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
    RefreshAgreements();
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


