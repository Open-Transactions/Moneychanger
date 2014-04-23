#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/cashpurse.hpp>
#include <ui_cashpurse.h>

#include <gui/widgets/home.hpp>
#include <gui/widgets/editdetails.hpp>
#include <gui/widgets/dlggetamount.hpp>
#include <gui/widgets/overridecursor.hpp>

#include <gui/ui/dlgexportedtopass.hpp>
#include <gui/ui/dlgexportedcash.hpp>
#include <gui/ui/dlgexportcash.hpp>

#include <core/moneychanger.hpp>

#include <opentxs/OTAPI.hpp>
#include <opentxs/OTAPI_Exec.hpp>
#include <opentxs/OT_ME.hpp>

#include <QDateTime>
#include <QCheckBox>
#include <QMessageBox>
#include <QDebug>


MTCashPurse::MTCashPurse(QWidget *parent, MTDetailEdit & theOwner) :
    QWidget(parent),
    m_qstrAcctId(""),
    m_qstrAssetId(""),
    m_pOwner(&theOwner),
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
    ui->pushButtonWithdraw->setText(tr("Withdraw Cash..."));
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

    ui->tableWidget->setSelectionMode    (QAbstractItemView::SingleSelection);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    // ------------------------------------
//    ui->tableWidget->setColumnWidth(0,60); // checkbox
//    ui->tableWidget->setColumnWidth(1,60); // amount
//    ui->tableWidget->setColumnWidth(2,140); // expires
//    ui->tableWidget->setColumnWidth(3,60); // series

//    QTableWidgetItem *newItem = new QTableWidgetItem(tr("%1").arg();
//    ui->tableWidget->setItem(row, column, newItem);
    // ------------------------------------

}


void MTCashPurse::refresh(QString strID, QString strName)
{
//    this->blockSignals(true);
    ui->tableWidget->blockSignals(true);
    // -----------------------------------
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount (0);
    // -----------------------------------
    if ((NULL != ui) && !strID.isEmpty())
    {
        m_qstrAcctId   = strID;
        m_qstrAcctName = strName;
        // ----------------------------------
        if (MTHome::rawAcctBalance(strID) > 0)
            ui->pushButtonWithdraw->setEnabled(true);
        else
            ui->pushButtonWithdraw->setEnabled(false);
        // ----------------------------------
        ui->pushButtonDeposit ->setText(tr("Deposit Cash"));
        // ----------------------------------
        QStringList selectedIndices;
        int64_t     lAmount=0;

//      int nNumberChecked =
                this->TallySelections(selectedIndices, lAmount);
        // ----------------------------------
        QString   qstrAmount    = MTHome::shortAcctBalance(strID);
        QWidget * pHeaderWidget = MTEditDetails::CreateDetailHeaderWidget(MTDetailEdit::DetailEditTypeAccount, strID, strName, qstrAmount, "", ":/icons/icons/vault.png", false);

        pHeaderWidget->setObjectName(QString("DetailHeader")); // So the stylesheet doesn't get applied to all its sub-widgets.

        if (m_pHeaderWidget)
        {
            ui->verticalLayoutPage->removeWidget(m_pHeaderWidget);

            m_pHeaderWidget->setParent(NULL);
            m_pHeaderWidget->disconnect();
            m_pHeaderWidget->deleteLater();

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
                    QString::fromStdString(OTAPI_Wrap::It()->GetAssetType_Name(str_acct_asset)));
        // -----------------------------------
        int64_t  raw_cash_balance = MTHome::rawCashBalance(qstr_acct_server, qstr_acct_asset, qstr_acct_nym);

        ui->labelCashBalance->setText(QString("<font color=grey><big>%1</big></font>").
                                      arg(MTHome::cashBalance(qstr_acct_server, qstr_acct_asset, qstr_acct_nym)) );
        ui->labelAssetType->setText(QString("<font color=grey>%1</font>").arg(qstr_asset_name) );
        // -----------------------------------
        if (raw_cash_balance > 0)
        {
            // --------------------------------------
            std::string str_purse = OTAPI_Wrap::It()->LoadPurse(str_acct_server, str_acct_asset, str_acct_nym);

            if (!str_purse.empty())
            {
                int32_t purse_count = OTAPI_Wrap::It()->Purse_Count(str_acct_server, str_acct_asset, str_purse);
                // -------------------------------------------------------
                ui->tableWidget->setRowCount(static_cast<int>(purse_count));
                // -------------------------------------------------------
                for (int ii = 0; ii < purse_count; ii++)
                {
                    std::string cash_token = OTAPI_Wrap::It()->Purse_Peek(str_acct_server, str_acct_asset, str_acct_nym, str_purse);

                    if (!cash_token.empty())
                    {
                        std::string str_amount = OTAPI_Wrap::It()->FormatAmount(str_acct_asset,
                                                                          OTAPI_Wrap::It()->Token_GetDenomination(str_acct_server,
                                                                                                            str_acct_asset,
                                                                                                            cash_token));
                        // ------------------------------------------------------
                        QDateTime qdate_expires     = QDateTime::fromTime_t(OTAPI_Wrap::It()->Token_GetValidTo(str_acct_server, str_acct_asset, cash_token));
                        QString   qstr_token_id     = QString::fromStdString(OTAPI_Wrap::It()->Token_GetID(str_acct_server, str_acct_asset, cash_token));
                        QString   qstr_denomination = QString::fromStdString(str_amount);
                        QString   qstr_series       = QString("%1").arg(OTAPI_Wrap::It()->Token_GetSeries(str_acct_server, str_acct_asset, cash_token));
                        QString   qstr_expires      = qdate_expires.toString(QString("MMM d yyyy hh:mm:ss"));

                        QLabel * pLabelDenomination = new QLabel(qstr_denomination);
                        QLabel * pLabelExpires      = new QLabel(QString("<small>%1</small>").arg(qstr_expires));
                        QLabel * pLabelSeries       = new QLabel(qstr_series);
                        QLabel * pLabelTokenID      = new QLabel(QString("<small>%1</small>").arg(qstr_token_id));

                        pLabelDenomination  ->setAlignment(Qt::AlignCenter);
                        pLabelExpires       ->setAlignment(Qt::AlignCenter);
                        pLabelSeries        ->setAlignment(Qt::AlignCenter);
                        pLabelTokenID       ->setAlignment(Qt::AlignCenter);

//                      pLabelDenomination  ->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
//                      pLabelExpires       ->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
//                      pLabelSeries        ->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
//                      pLabelTokenID       ->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

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

                    str_purse = OTAPI_Wrap::It()->Purse_Pop(str_acct_server, str_acct_asset, str_acct_nym, str_purse);
                } // for
                // -------------------------------------------------------
            } // if purse not empty.
        } // if cash balance > 0.
    }
    // --------------------------------------------
//    this->blockSignals(false);
    ui->tableWidget->blockSignals(false);
}




//QTableWidgetItem *item = tableWidget->item(row,col);
//if (item->checkState() == ...)
//   ...

void MTCashPurse::checkboxClicked(int state)
{
    Q_UNUSED(state);

    QStringList selectedIndices;
    int64_t        lAmount=0;

    //int nNumberChecked =
        this->TallySelections(selectedIndices, lAmount);
}

void MTCashPurse::on_pushButtonWithdraw_clicked()
{
    // ----------------------------------------------------------------
    if (m_qstrAcctId.isEmpty() || m_qstrAssetId.isEmpty())
    {
        qDebug() << "MTCashPurse::on_pushButtonWithdraw_clicked: Strange: Acct ID or Asset ID was empty.";
        return;
    }
    // ----------------------------------------------------------------
    std::string accountID = m_qstrAcctId.toStdString();
    // ----------------------------------------------------------------
    QString qstrReason(tr("How much cash would you like to withdraw?"));
    // ----------------------------------------------------------------
    // Let's find out the withdrawal amount.
    //
    DlgGetAmount dlgAmount(this, m_qstrAcctId, m_qstrAssetId, qstrReason);

    dlgAmount.setWindowTitle("Withdrawal Amount");

    if (dlgAmount.exec() != QDialog::Accepted)
        return;
    // ----------------------------------------------------------------
    // If we're in here, that means the person entered a valid (larger-than-zero)
    // withdrawal amount, and then clicked "OK".
    //
    bool bSent = false;
    {
        OT_ME   madeEasy;
        int64_t lAmount = dlgAmount.GetAmount();

        MTSpinner theSpinner;

        bSent = (1 == madeEasy.easy_withdraw_cash(accountID, lAmount));
    }
    // -----------------------------------------------------------------
    if (!bSent)
    {
        const std::string str_server = OTAPI_Wrap::It()->GetAccountWallet_ServerID(accountID);
        const std::string str_nym    = OTAPI_Wrap::It()->GetAccountWallet_NymID   (accountID);

        const int64_t lUsageCredits  = Moneychanger::HasUsageCredits(this, str_server, str_nym);

        // In the cases of -2 and 0, HasUsageCredits already pops up its own message box.
        //
        if (((-2) != lUsageCredits) && (0 != lUsageCredits))
            QMessageBox::warning(this, tr("Failed Withdrawing Cash"),
                                 tr("Failed trying to withdraw cash."));
    }
    else
    {
        QMessageBox::information(this, tr("Success Withdrawing Cash"),
                                tr("Success withdrawing cash!"));
        // --------------------------------------------------------
        emit balancesChanged(m_qstrAcctId);
    }
    // -----------------------------------------------------------------
}


// -----------------------------------------------------------------

void MTCashPurse::on_pushButtonExport_clicked()
{
    bool bSuccess = false;
    QStringList selectedIndices;
    int64_t     lAmount=0;

    //int nNumberChecked =
            this->TallySelections(selectedIndices, lAmount);
    // ------------------------------------------------------------------
    std::string str_acct_id     = m_qstrAcctId.toStdString();
    std::string str_acct_nym    = OTAPI_Wrap::It()->GetAccountWallet_NymID(str_acct_id);
    std::string str_acct_server = OTAPI_Wrap::It()->GetAccountWallet_ServerID(str_acct_id);
    std::string str_acct_asset  = OTAPI_Wrap::It()->GetAccountWallet_AssetTypeID(str_acct_id);
    // ------------------------------------------------------------------
    QString qstrSelectedIndices = selectedIndices.join(","); // Create a comma-separated list of selected indices.

    // This "should never happen" since the export button is disabled
    // when none of the cash indices are selected in the GUI.
    //
    if (qstrSelectedIndices.isEmpty())
        return;
    // ------------------------------------
    std::string str_amount = OTAPI_Wrap::It()->FormatAmount(str_acct_asset, lAmount);
    // ------------------------------------
    // Find out if they want it to be password-protected, and if not,
    // find out who the recipient Nym is meant to be.
    //
    DlgExportCash dlgExport(this);

    dlgExport.setWindowTitle("Export Cash");

    if (dlgExport.exec() != QDialog::Accepted)
        return;
    // ----------------------------------------------------------------
    // Now dlgExport can tell us whether the cash should be exported to a
    // particular Nym, or whether it should be password-protected.
    //
    bool    bExportToPassphrase = dlgExport.IsExportToPassphrase();
    QString qstrRecipNymID(""),
            qstrRecipName(""),
            qstrRecipientWarning(""),
            qstrRemovedFromPurse = tr("WARNING: the cash will be removed from your purse!");
    // ----------------------------------------------------------------
    if (bExportToPassphrase)
        qstrRecipientWarning = QString("%1.<br/><br/>%2").arg(tr("The cash will be exported to a passphrase")).arg(qstrRemovedFromPurse);
    else
    {
        qstrRecipNymID       = dlgExport.GetHisNymID();
        qstrRecipName        = dlgExport.GetHisName ();
        // ---------------------------------------------
        qstrRecipientWarning = QString("%1: '%2'<br/>%3: %4<br/><br/>%5").arg(tr("The cash will be exported to")).
                arg(qstrRecipName).arg(tr("Using his NymID")).arg(qstrRecipNymID).arg(qstrRemovedFromPurse);
    }
    // ----------------------------------------------------------------
    QMessageBox::StandardButton reply;

    QString qstrQuestion = QString("%1 %2<br/>%3<br/><br/>%4").arg(tr("Are you sure you wish to export")).
                                                          arg(QString::fromStdString(str_amount)).
                                                          arg(tr("from your cash purse?")).
                                                          arg(qstrRecipientWarning); // Perhaps not all languages have the same question mark...

    reply = QMessageBox::question(this, "Confirm Export", qstrQuestion,
                                  QMessageBox::Yes|QMessageBox::No);
    // ------------------------------------
    if (reply == QMessageBox::Yes)
    {
        OT_ME       madeEasy;
        std::string str_selected_indices(qstrSelectedIndices.toStdString()); // (FYI, you can also use "all" for all indices.)

        std::string str_exported,  // The exported cash, encrypted to recipient (or passphrase.)
                    str_retained;  // The exported cash, encrypted to sender (just in case...)

        str_exported = madeEasy.export_cash(str_acct_server,
                                            str_acct_nym,
                                            str_acct_asset,
                                            qstrRecipNymID.toStdString(),
                                            str_selected_indices,
                                            bExportToPassphrase,
                                            str_retained); // output
        // ------------------------------------
        if (str_exported.empty())
            QMessageBox::warning(this, tr("Failed Exporting Cash"),
                                 tr("Failed trying to export cash."));
        else if (bExportToPassphrase)
        {
            DlgExportedToPass dlgExported(this, QString::fromStdString(str_exported));
            dlgExported.exec();
            // --------------------------------------------------------
            bSuccess = true;
        }
        else
        {
            DlgExportedCash dlgExported(this,
                                        QString::fromStdString(str_exported),
                                        QString::fromStdString(str_retained));
            dlgExported.exec();
            // --------------------------------------------------------
            bSuccess = true;
        }
    }
    // -----------------------------------
    if (bSuccess)
    {
        for (int ii = 0; ii < ui->tableWidget->rowCount(); ii++)
        {
            QWidget * pWidget = ui->tableWidget->cellWidget(ii, 0);

            if (NULL != pWidget)
            {
                QCheckBox * pCheckbox = pWidget->findChild<QCheckBox *>();

                if ( (NULL != pCheckbox) && (Qt::Checked == pCheckbox->checkState()))
                    pCheckbox->setChecked(false);
            }
        }
        // --------------------------------------------------------
        //int nNumberSelected =
                this->TallySelections(selectedIndices, lAmount);
        // --------------------------------------------------------
        emit balancesChanged(m_qstrAcctId);
    }
}

// -----------------------------------------------------------------

void MTCashPurse::on_pushButtonDeposit_clicked()
{
    QStringList selectedIndices;
    int64_t     lAmount=0;

    //int nNumberChecked =
            this->TallySelections(selectedIndices, lAmount);
    // ------------------------------------------------------------------
    std::string str_acct_id     = m_qstrAcctId.toStdString();
    std::string str_acct_nym    = OTAPI_Wrap::It()->GetAccountWallet_NymID(str_acct_id);
    std::string str_acct_server = OTAPI_Wrap::It()->GetAccountWallet_ServerID(str_acct_id);
    std::string str_acct_asset  = OTAPI_Wrap::It()->GetAccountWallet_AssetTypeID(str_acct_id);
    // ------------------------------------------------------------------
    QString qstrSelectedIndices = selectedIndices.join(","); // Create a comma-separated list of selected indices.

    // This "should never happen" since the deposit button is disabled
    // when none of the cash indices are selected in the GUI.
    //
    if (qstrSelectedIndices.isEmpty())
        return;
    // ------------------------------------
    std::string str_amount = OTAPI_Wrap::It()->FormatAmount(str_acct_asset, lAmount);
    // ------------------------------------
    QMessageBox::StandardButton reply;

    QString qstrQuestion = QString("%1 %2<br/>%3: %4 %5").arg(tr("Are you sure you wish to deposit")).
                                                          arg(QString::fromStdString(str_amount)).
                                                          arg(tr("into the account")).
                                                          arg(m_qstrAcctName).arg(tr("?")); // Perhaps not all languages have the same question mark...

    reply = QMessageBox::question(this, "Confirm Deposit", qstrQuestion,
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        bool bSent = false;
        {
            OT_ME        madeEasy;
            MTSpinner    theSpinner;
            std::string  str_selected_indices(qstrSelectedIndices.toStdString()); // (FYI, you can also use "all" for all indices.)

            bSent = (1 == madeEasy.deposit_local_purse(str_acct_server, // <=======
                                                       str_acct_nym,
                                                       str_acct_id,
                                                       str_selected_indices));
        }
        // -----------------------------------------------------------------
        if (!bSent)
        {
            const int64_t lUsageCredits = Moneychanger::HasUsageCredits(this, str_acct_server, str_acct_nym);

            // In the cases of -2 and 0, HasUsageCredits already pops up its own error box.
            //
            if (((-2) != lUsageCredits) && (0 != lUsageCredits))
                QMessageBox::warning(this, tr("Failed Depositing Cash"),
                                     tr("Failed trying to deposit cash."));
        }
        else
        {
            QMessageBox::information(this, tr("Success Depositing Cash"),
                                    tr("Success depositing cash!"));
            // --------------------------------------------------------
            for (int ii = 0; ii < ui->tableWidget->rowCount(); ii++)
            {
                QWidget * pWidget = ui->tableWidget->cellWidget(ii, 0);

                if (NULL != pWidget)
                {
                    QCheckBox * pCheckbox = pWidget->findChild<QCheckBox *>();

                    if ( (NULL != pCheckbox) && (Qt::Checked == pCheckbox->checkState()))
                        pCheckbox->setChecked(false);
                }
            }
            // --------------------------------------------------------
            //int nNumberSelected =
                    this->TallySelections(selectedIndices, lAmount);
            // --------------------------------------------------------
            emit balancesChanged(m_qstrAcctId);
        }
    }
    // -----------------------------------------------------------------
}


// Returns the number that are selected. (If 5 boxes are checked, returns 5.)
// Also returns selected IDs and total sum amount of those tokens.
//
int MTCashPurse::TallySelections(QStringList & selectedIndices, int64_t & lAmount)
{
    selectedIndices.clear();
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
                {
                    int64_t lTokenAmount = OTAPI_Wrap::It()->StringToAmount(m_qstrAssetId.toStdString(),
                                                                      pValueLabel->text().toStdString());
                    lSelectedAmount += lTokenAmount;
                }
                // -------------------------
                if (NULL != pIDLabel) // NOTE that we now attach the index here, instead of the ID.
                    selectedIndices.append(QString("%1").arg(ii));
//                  selectedIndices.append(pIDLabel->text());
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
        ui->pushButtonExport ->setEnabled(true);
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
        ui->pushButtonExport ->setEnabled(false);

        ui->pushButtonDeposit ->setText(tr("Deposit Cash"));
    }
    // -------------------------
    return nNumberSelected;
}


void MTCashPurse::ClearContents()
{    
//    this->blockSignals(true);
    ui->tableWidget->blockSignals(true);
    // ----------------------------------
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount (0);
    // ----------------------------------
    ui->labelCashBalance->setText("");
    // ----------------------------------
    if (m_pHeaderWidget)
    {
        ui->verticalLayoutPage->removeWidget(m_pHeaderWidget);

        m_pHeaderWidget->setParent(NULL);
        m_pHeaderWidget->disconnect();
        m_pHeaderWidget->deleteLater();

        m_pHeaderWidget = NULL;
    }
    // ----------------------------------
    m_pHeaderWidget = new QWidget;
    ui->verticalLayoutPage->insertWidget(0, m_pHeaderWidget);
    // ----------------------------------
    ui->pushButtonDeposit ->setText(tr("Deposit Cash"));
    // ----------------------------------
    ui->pushButtonDeposit ->setEnabled(false);
    ui->pushButtonExport  ->setEnabled(false);
    ui->pushButtonWithdraw->setEnabled(false);
    // ----------------------------------
    m_qstrAssetId  = QString("");
    m_qstrAcctId   = QString("");
    m_qstrAcctName = QString("");
    // ----------------------------------
    ui->labelAssetType->setText(QString(""));
    // ----------------------------------
//    this->blockSignals(false);
    ui->tableWidget->blockSignals(false);
}


MTCashPurse::~MTCashPurse()
{
    delete ui;
}

