#include <QDebug>
#include <QCheckBox>
#include <QDateTime>
#include <QMessageBox>

#include "cashpurse.h"
#include "ui_cashpurse.h"

#include "editdetails.h"

#include "home.h"


MTCashPurse::MTCashPurse(QWidget *parent) :
    QWidget(parent),
    m_qstrAcctId(""),
    m_qstrAssetId(""),
    m_pHeaderWidget(NULL),
    ui(new Ui::MTCashPurse)
{
    ui->setupUi(this);
    // ------------------------------------
    m_pHeaderWidget = new QWidget;
    ui->verticalLayoutPage->insertWidget(0, m_pHeaderWidget);
    // ------------------------------------
    ui->pushButtonDeposit ->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ui->pushButtonWithdraw->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->pushButtonDeposit ->setText(tr("Deposit Cash"));
    ui->pushButtonWithdraw->setText(tr("Withdraw Cash"));
    // ------------------------------------
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);

    ui->tableWidget->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignCenter);  // checkbox
    ui->tableWidget->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignCenter);  // value
    ui->tableWidget->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignCenter);  // expires
    ui->tableWidget->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignCenter);  // series


//    ui->tableWidget->horizontalHeader()->sectionResizeMode(QHeaderView::Stretch);

//    ui->tableWidget->horizontalHeaderItem(0)->setSizeHint(QSize(10, 0));  // checkbox
//    ui->tableWidget->horizontalHeaderItem(1)->setSizeHint(QSize(10, 0));  // value
//    ui->tableWidget->horizontalHeaderItem(2)->setSizeHint(QSize(30, 0));  // expires
//    ui->tableWidget->horizontalHeaderItem(3)->setSizeHint(QSize(10, 0));  // series

    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    // ------------------------------------
//    ui->tableWidget->setColumnWidth(0,60); // checkbox
//    ui->tableWidget->setColumnWidth(1,60); // amount
//    ui->tableWidget->setColumnWidth(2,140); // expires
//    ui->tableWidget->setColumnWidth(3,60); // series

//    QTableWidgetItem *newItem = new QTableWidgetItem(tr("%1").arg();
//    ui->tableWidget->setItem(row, column, newItem);
}


void MTCashPurse::refresh(QString strID, QString strName)
{
    if (NULL != ui)
    {
        m_qstrAcctId = strID;
        // ----------------------------------
        if (MTHome::rawAcctBalance(strID) > 0)
            ui->pushButtonWithdraw->setEnabled(true);
        else
            ui->pushButtonWithdraw->setEnabled(false);
        // ----------------------------------
        ui->pushButtonDeposit ->setText(tr("Deposit Cash"));
        // ----------------------------------
        QList<QString> selectedIDs;
        int64_t        lAmount=0;

        int nNumberChecked = this->TallySelections(selectedIDs, lAmount);
        // ----------------------------------
        QString   qstrAmount    = MTHome::shortAcctBalance(strID);
        QWidget * pHeaderWidget = MTEditDetails::CreateDetailHeaderWidget(strID, strName, qstrAmount, "", false);

        if (NULL != m_pHeaderWidget)
        {
            ui->verticalLayoutPage->removeWidget(m_pHeaderWidget);
            delete m_pHeaderWidget;
            m_pHeaderWidget = NULL;
        }
        ui->verticalLayoutPage->insertWidget(0, pHeaderWidget);
        m_pHeaderWidget = pHeaderWidget;
        // ----------------------------------
        std::string str_acct_id     = strID.toStdString();
        std::string str_acct_nym    = OTAPI_Wrap::It()->GetAccountWallet_NymID(str_acct_id);
        std::string str_acct_server = OTAPI_Wrap::It()->GetAccountWallet_ServerID(str_acct_id);
        std::string str_acct_asset  = OTAPI_Wrap::It()->GetAccountWallet_AssetTypeID(str_acct_id);
        // -----------------------------------
        QString qstr_acct_nym    = QString::fromStdString(str_acct_nym);
        QString qstr_acct_server = QString::fromStdString(str_acct_server);
        QString qstr_acct_asset  = QString::fromStdString(str_acct_asset);
        // -----------------------------------
        m_qstrAssetId = qstr_acct_asset;
        // -----------------------------------
        QString qstr_asset_name;

        if (!qstr_acct_asset.isEmpty())
            qstr_asset_name = QString("   (%1)").arg(
                    QString::fromStdString(OTAPI_Wrap::GetAssetType_Name(str_acct_asset)));
        // -----------------------------------
        int64_t  raw_cash_balance = MTHome::rawCashBalance(qstr_acct_server, qstr_acct_asset, qstr_acct_nym);

        ui->labelCashBalance->setText(QString("<font color=grey><big>%1</big></font>").
                                      arg(MTHome::cashBalance(qstr_acct_server, qstr_acct_asset, qstr_acct_nym)) );
        ui->labelAssetType->setText(QString("<font color=grey>%1</font>").arg(qstr_asset_name) );
        // -----------------------------------
        ui->tableWidget->clearContents();
        // -----------------------------------
        if (raw_cash_balance > 0)
        {
            // --------------------------------------
            std::string str_purse = OTAPI_Wrap::LoadPurse(str_acct_server, str_acct_asset, str_acct_nym);

            if (!str_purse.empty())
            {
                int32_t purse_count = OTAPI_Wrap::Purse_Count(str_acct_server, str_acct_asset, str_purse);
                // -------------------------------------------------------
                ui->tableWidget->setRowCount(static_cast<int>(purse_count));
                // -------------------------------------------------------
                for (int ii = 0; ii < purse_count; ii++)
                {
                    std::string cash_token = OTAPI_Wrap::Purse_Peek(str_acct_server, str_acct_asset, str_acct_nym, str_purse);

                    if (!cash_token.empty())
                    {
                        QDateTime qdate_expires     = QDateTime::fromTime_t(OTAPI_Wrap::Token_GetValidTo(str_acct_server, str_acct_asset, cash_token));
                        QString   qstr_token_id     = QString::fromStdString(OTAPI_Wrap::Token_GetID(str_acct_server, str_acct_asset, cash_token));
                        QString   qstr_denomination = QString("%1").arg(OTAPI_Wrap::Token_GetDenomination(str_acct_server, str_acct_asset, cash_token));
                        QString   qstr_series       = QString("%1").arg(OTAPI_Wrap::Token_GetSeries(str_acct_server, str_acct_asset, cash_token));
                        QString   qstr_expires      = qdate_expires.toString(QString("MMM d yyyy hh:mm:ss"));

                        QLabel * pLabelDenomination = new QLabel(qstr_denomination);
                        QLabel * pLabelExpires      = new QLabel(QString("<small>%1</small>").arg(qstr_expires));
                        QLabel * pLabelSeries       = new QLabel(qstr_series);
                        QLabel * pLabelTokenID      = new QLabel(QString("<small>%1</small>").arg(qstr_token_id));

                        pLabelDenomination  ->setAlignment(Qt::AlignCenter);
                        pLabelExpires       ->setAlignment(Qt::AlignCenter);
                        pLabelSeries        ->setAlignment(Qt::AlignCenter);
                        pLabelTokenID       ->setAlignment(Qt::AlignCenter);

//                        pLabelDenomination  ->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
//                        pLabelExpires       ->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
//                        pLabelSeries        ->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
//                        pLabelTokenID       ->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

                        // ------------------------------
                        QWidget     * wdg       = new QWidget;
                        QCheckBox   * pCheckbox = new QCheckBox(wdg);
                        QHBoxLayout * layout    = new QHBoxLayout(wdg);

                        layout->setContentsMargins(0,0,0,0);
                        layout->addWidget(pCheckbox);
                        layout->setAlignment( Qt::AlignCenter );
                        wdg->setLayout(layout);
                        ui->tableWidget->setCellWidget ( ii, 0, wdg);
//                      ui->tableWidget->setCellWidget ( ii, 0, pCheckbox );

                        ui->tableWidget->setCellWidget ( ii, 1, pLabelDenomination );
                        ui->tableWidget->setCellWidget ( ii, 2, pLabelExpires );
                        ui->tableWidget->setCellWidget ( ii, 3, pLabelSeries );
                        ui->tableWidget->setCellWidget ( ii, 4, pLabelTokenID );

                        connect(pCheckbox, SIGNAL(stateChanged(int) ), this, SLOT(checkboxClicked(int)));
                    }

                    str_purse = OTAPI_Wrap::Purse_Pop(str_acct_server, str_acct_asset, str_acct_nym, str_purse);
                } // for
                // -------------------------------------------------------
            } // if purse not empty.
        } // if cash balance > 0.
    }
}




//QTableWidgetItem *item = tableWidget->item(row,col);
//if (item->checkState() == ...)
//   ...

void MTCashPurse::checkboxClicked(int state)
{
    QList<QString> selectedIDs;
    int64_t        lAmount=0;

    int nNumberChecked = this->TallySelections(selectedIDs, lAmount);
}

void MTCashPurse::on_pushButtonWithdraw_clicked()
{
    // ----------------------------------------------------------------
    if (m_qstrAcctId.isEmpty() || m_qstrAssetId.isEmpty())
    {
        qDebug() << "MTCashPurse::on_pushButtonWithdraw_clicked: Strange: Acct ID or Asset ID was empty.";
        return;
    }

    std::string accountID = m_qstrAcctId.toStdString();
    std::string assetID   = m_qstrAssetId.toStdString();
    // ----------------------------------------------------------------
    QString qstrReason(tr("How much cash would you like to withdraw?"));
    // ----------------------------------------------------------------



    // ----------------------------------------------------------------
    OT_ME madeEasy;

    int32_t nWithdrawn = madeEasy.easy_withdraw_cash(accountID,
                                      const int64_t        AMOUNT)





    // ----------------------------------------
    std::string nymID     = OTAPI_Wrap::It()->GetAccountWallet_NymID(accountID);
    std::string serverID  = OTAPI_Wrap::It()->GetAccountWallet_ServerID(accountID);
    // ----------------------------------------------------------------
    if (nymID.empty() || serverID.empty())
    {
        qDebug() << "MTCashPurse::on_pushButtonWithdraw_clicked: Strange: Nym ID or Server ID was empty.";
        return;
    }
    // ----------------------------------------------------------------





    std::string serverResponseMessage;




    // ----------------------------------------------------------------
    // Make sure we actually have a copy of the asset contract.
    //
    std::string assetContract = OTAPI_Wrap::LoadAssetContract(assetID);
    // ----------------------------------------------------------------
    OT_ME madeEasy;

    std::string assetContract = madeEasy.load_or_retrieve_contract(serverID, nymID, assetID);
    // ----------------------------------------------------------------
    if (assetContract.empty())
    {
        QMessageBox::warning(this, tr("Failed loading asset contract."),
                             QString("%1: %2").arg(tr("Failed trying to load the asset contract")).arg(m_qstrAssetId));
        return;
    }
    // ---------------------------------------------------------
    // Download the public mintfile if it's not there (or if it's expired.)
    // Also, load it up into memory as a string (just to make sure it works.)
    // Then we can actually send the withdrawal transaction request. (Until
    // then, why bother?)
    //
    std::string mintFile;

    // expired or missing.
    if (false == OTAPI_Wrap::Mint_IsStillGood(serverID, assetID))
    {
        // ----------------------------------------
        OTAPI_Func theRequest = new OTAPI_Func(OTAPI_Func.FT.GET_MINT, serverID, nymID, assetID);
        String strResponse = OTAPI_Func.SendRequest(theRequest, "GET_MINT");

        if (!Utility.VerifyStringVal(strResponse)) {
            System.out.println("IN withdrawCash: OTAPI_Func.SendRequest(GET_MINT) failed. (I give up.) (Unable to get mint.)");
            return false;
        }
        // ----------------------------------------
        mintFile = OTAPI_Wrap::LoadMint(serverID, assetID);
        if (!Utility.VerifyStringVal(mintFile)) {
            System.out.println("OT_API_LoadMint returned null even after OT_API_getMint (I give up.) (Unable to find mint.)");
            return false;
        }
    }
    else // current mint IS available already on local storage (and not expired.)
    {
        mintFile = OTAPI_Wrap::LoadMint(serverID, assetID);
        if (!Utility.VerifyStringVal(mintFile)) {
            System.out.println("OT_API_LoadMint returned null even after successful OT_API_Mint_IsStillGood (I give up.) (Unable to find mint.)");
            return false;
        }
    }
    // ---------------------------------------------------
    // By this point, the mintfile is DEFINITELY good (available, not null,
    // not expired, and loaded up.) Now we know for a fact that when the API
    // call is made to withdraw cash, that it will find the mint properly.
    //
    OTAPI_Func theRequest = new OTAPI_Func(OTAPI_Func.FT.WITHDRAW_CASH, serverID, nymID, accountID, amount);
    String strResponse = OTAPI_Func.SendTransaction(theRequest, "WITHDRAW_CASH"); // <========================

    if (!Utility.VerifyStringVal(strResponse)) {
        System.out.println("OTAPI_Func.SendTransaction() failed, in withdrawCash.");
        return false;
    }
    // --------------------------------------------------------------------------------------

    return true;
}



void MTCashPurse::on_pushButtonDeposit_clicked()
{
    QList<QString> selectedIDs;
    int64_t        lAmount=0;

    int nNumberChecked = this->TallySelections(selectedIDs, lAmount);

    // TODO: perform the deposit!
}


// Returns the number that are selected. (If 5 boxes are checked, returns 5.)
// Also returns selected IDs and total sum amount of those tokens.
//
int MTCashPurse::TallySelections(QList<QString> & selectedIDs, int64_t & lAmount)
{
    selectedIDs.clear();
    // -------------------------
    int64_t lSelectedAmount=0;
    int     nNumberSelected=0;
    // -------------------------
    for (int ii = 0; ii < ui->tableWidget->rowCount(); ii++)
    {
        QWidget * pWidget = ui->tableWidget->cellWidget(ii, 0);

        if (NULL != pWidget)
        {
            QCheckBox * pCheckbox = pWidget->findChild<QCheckBox *>();

            if ( (NULL != pCheckbox) && (Qt::Checked == pCheckbox->checkState()))
            {
                // Add to tally.
                //
                nNumberSelected++;
                // -------------------------
                QWidget * pValueItem = ui->tableWidget->cellWidget(ii, 1); // the amount.
                QWidget * pIDItem    = ui->tableWidget->cellWidget(ii, 4); // the token ID.
                // -------------------------
                QLabel * pValueLabel = (QLabel *)pValueItem; // todo cast
                QLabel * pIDLabel    = (QLabel *)pIDItem; // todo cast
                // -------------------------
                if (NULL != pValueLabel)
                    lSelectedAmount += static_cast<int64_t>(pValueLabel->text().toLong());
                // -------------------------
                if (NULL != pIDLabel)
                    selectedIDs.append(pIDLabel->text());
                // -------------------------
            }
        }
    }
    // -------------------------
    lAmount = lSelectedAmount;
    // -------------------------
    if (nNumberSelected > 0)
    {
        ui->pushButtonDeposit->setEnabled(true);
        // ---------------------------------------
        QString qstr_amount("");

        if (m_qstrAssetId.isEmpty())
            qstr_amount = QString("%1").arg(lAmount);
        else
            qstr_amount = QString::fromStdString(OTAPI_Wrap::It()->FormatAmount(m_qstrAssetId.toStdString(), lAmount));
        // ---------------------------------------
        ui->pushButtonDeposit ->setText(QString("%1: %2").arg(tr("Deposit Cash")).arg(qstr_amount));
    }
    // -------------------------
    else
    {
        ui->pushButtonDeposit->setEnabled(false);

        ui->pushButtonDeposit ->setText(tr("Deposit Cash"));
    }
    // -------------------------
    return nNumberSelected;
}


void MTCashPurse::ClearContents()
{
    ui->tableWidget->clearContents();
    // ----------------------------------
    ui->labelCashBalance->setText("");
    // ----------------------------------
    if (NULL != m_pHeaderWidget)
    {
        ui->verticalLayoutPage->removeWidget(m_pHeaderWidget);
        delete m_pHeaderWidget;
        m_pHeaderWidget = NULL;
    }
    // ----------------------------------
    m_pHeaderWidget = new QWidget;
    ui->verticalLayoutPage->insertWidget(0, m_pHeaderWidget);
    // ----------------------------------
    ui->pushButtonDeposit ->setText(tr("Deposit Cash"));
    // ----------------------------------
    ui->pushButtonDeposit ->setEnabled(false);
    ui->pushButtonWithdraw->setEnabled(false);
    // ----------------------------------
    m_qstrAssetId = QString("");
    m_qstrAcctId  = QString("");
    // ----------------------------------
    ui->labelAssetType->setText(QString(""));
}


MTCashPurse::~MTCashPurse()
{
    delete ui;
}

