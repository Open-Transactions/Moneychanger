#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/pageoffer_accounts.hpp>
#include <ui_pageoffer_accounts.h>

#include <QMessageBox>
#include <QTimer>

#include <core/moneychanger.hpp>

#include <gui/widgets/dlgchooser.hpp>
#include <gui/widgets/detailedit.hpp>
#include <gui/widgets/wizardnewoffer.hpp>

#include <opentxs/core/Version.hpp>
#include <opentxs/api/Api.hpp>
#include <opentxs/api/OT.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>


PageOffer_Accounts::PageOffer_Accounts(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::PageOffer_Accounts)
{
    ui->setupUi(this);

    QString style_sheet = "QPushButton{border: none; border-style: outset; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa);}"
            "QPushButton:pressed {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }"
            "QPushButton:hover {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }";

    ui->pushButtonSelectAssetAcct   ->setStyleSheet(style_sheet);
    ui->pushButtonSelectCurrencyAcct->setStyleSheet(style_sheet);

    ui->label_2->setStyleSheet("QLabel { color : gray; }");
    ui->label_4->setStyleSheet("QLabel { color : gray; }");

    ui->lineEditAssetAcctID        ->setStyleSheet("QLineEdit { border: none; background-color: lightgray }");
    ui->lineEditCurrencyAcctID     ->setStyleSheet("QLineEdit { border: none; background-color: lightgray }");
    ui->lineEditAssetAcctBalance   ->setStyleSheet("QLineEdit { border: none; background-color: lightgray }");
    ui->lineEditCurrencyAcctBalance->setStyleSheet("QLineEdit { border: none; background-color: lightgray }");

    ui->lineEditNotaryID           ->setStyleSheet("QLineEdit { border: none; background-color: lightgray }");
    ui->lineEditServerName         ->setStyleSheet("QLineEdit { border: none; background-color: lightgray }");
    ui->lineEditNymID              ->setStyleSheet("QLineEdit { border: none; background-color: lightgray }");
    ui->lineEditNymName            ->setStyleSheet("QLineEdit { border: none; background-color: lightgray }");

//    QString push_btn_style_sheet = "QPushButton {"
//                                        "border: none;"
//                                        "}";
//    ui->pushButtonManageNym         ->setStyleSheet(push_btn_style_sheet);
//    ui->pushButtonManageServer      ->setStyleSheet(push_btn_style_sheet);
//    ui->pushButtonManageAssetAcct   ->setStyleSheet(push_btn_style_sheet);
//    ui->pushButtonManageCurrencyAcct->setStyleSheet(push_btn_style_sheet);

    this->registerField("AssetAcctID*",        ui->lineEditAssetAcctID);
    this->registerField("CurrencyAcctID*",     ui->lineEditCurrencyAcctID);

    this->registerField("AssetAcctBalance",    ui->lineEditAssetAcctBalance);
    this->registerField("CurrencyAcctBalance", ui->lineEditCurrencyAcctBalance);

    this->registerField("AssetAcctName",       ui->pushButtonSelectAssetAcct,    "text");
    this->registerField("CurrencyAcctName",    ui->pushButtonSelectCurrencyAcct, "text");

    this->registerField("NotaryID",            ui->lineEditNotaryID);
    this->registerField("ServerName",          ui->lineEditServerName);
    this->registerField("NymID",               ui->lineEditNymID);
    this->registerField("NymName",             ui->lineEditNymName);
}



void PageOffer_Accounts::on_pushButtonManageServer_clicked()
{
    MTDetailEdit * pWindow = new MTDetailEdit(this);

    pWindow->setAttribute(Qt::WA_DeleteOnClose);
    // -------------------------------------
    mapIDName & the_map = pWindow->m_map;
    // -------------------------------------
    the_map.clear();
    // -------------------------------------
    QString qstrPreselected   = field("NotaryID").toString();
    bool    bFoundPreselected = false;
    // -------------------------------------
    int32_t the_count = opentxs::OT::App().API().Exec().GetServerCount();

    for (int32_t ii = 0; ii < the_count; ii++)
    {
        QString OT_id   = QString::fromStdString(opentxs::OT::App().API().Exec().GetServer_ID(ii));
        QString OT_name = QString::fromStdString(opentxs::OT::App().API().Exec().GetServer_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);

        if (!qstrPreselected.isEmpty() && (0 == qstrPreselected.compare(OT_id)))
            bFoundPreselected = true;
    } // for
    // -------------------------------------
    if (bFoundPreselected)
        pWindow->SetPreSelected(qstrPreselected);
    // -------------------------------------
    pWindow->setWindowTitle(tr("Manage Servers"));
    // -------------------------------------
    pWindow->dialog(MTDetailEdit::DetailEditTypeServer, true);
}

void PageOffer_Accounts::on_pushButtonManageNym_clicked()
{
    MTDetailEdit * pWindow = new MTDetailEdit(this);

    pWindow->setAttribute(Qt::WA_DeleteOnClose);
    // -------------------------------------
    mapIDName & the_map = pWindow->m_map;
    // -------------------------------------
    the_map.clear();
    // -------------------------------------
    QString qstrPreselected   = field("NymID").toString();
    bool    bFoundPreselected = false;
    // -------------------------------------
    int32_t the_count = opentxs::OT::App().API().Exec().GetNymCount();

    for (int32_t ii = 0; ii < the_count; ii++)
    {
        QString OT_id   = QString::fromStdString(opentxs::OT::App().API().Exec().GetNym_ID(ii));
        QString OT_name = QString::fromStdString(opentxs::OT::App().API().Exec().GetNym_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);

        if (!qstrPreselected.isEmpty() && (0 == qstrPreselected.compare(OT_id)))
            bFoundPreselected = true;
    } // for
    // -------------------------------------
    if (bFoundPreselected)
        pWindow->SetPreSelected(qstrPreselected);
    // -------------------------------------
    pWindow->setWindowTitle(tr("Manage Pseudonyms"));
    // -------------------------------------
    pWindow->dialog(MTDetailEdit::DetailEditTypeNym, true);
}

void PageOffer_Accounts::on_pushButtonManageAssetAcct_clicked()
{
    MTDetailEdit * pWindow = new MTDetailEdit(this);

    pWindow->setAttribute(Qt::WA_DeleteOnClose);
    // -------------------------------------
    mapIDName & the_map = pWindow->m_map;
    // -------------------------------------
    the_map.clear();
    // -------------------------------------
    QString qstrPreselected   = field("AssetAcctID").toString();
    bool    bFoundPreselected = false;
    // -------------------------------------
    int32_t the_count = opentxs::OT::App().API().Exec().GetAccountCount();

    for (int32_t ii = 0; ii < the_count; ii++)
    {
        QString OT_id   = QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_ID(ii));
        QString OT_name = QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);

        if (!qstrPreselected.isEmpty() && (0 == qstrPreselected.compare(OT_id)))
            bFoundPreselected = true;
    } // for
    // -------------------------------------
    if (bFoundPreselected)
        pWindow->SetPreSelected(qstrPreselected);
    // -------------------------------------
    pWindow->setWindowTitle(tr("Manage Asset Accounts"));
    // -------------------------------------
    pWindow->dialog(MTDetailEdit::DetailEditTypeAccount, true);
}

void PageOffer_Accounts::on_pushButtonManageCurrencyAcct_clicked()
{
    MTDetailEdit * pWindow = new MTDetailEdit(this);

    pWindow->setAttribute(Qt::WA_DeleteOnClose);
    // -------------------------------------
    mapIDName & the_map = pWindow->m_map;
    // -------------------------------------
    the_map.clear();
    // -------------------------------------
    QString qstrPreselected   = field("CurrencyAcctID").toString();
    bool    bFoundPreselected = false;
    // -------------------------------------
    int32_t the_count = opentxs::OT::App().API().Exec().GetAccountCount();

    for (int32_t ii = 0; ii < the_count; ii++)
    {
        QString OT_id   = QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_ID(ii));
        QString OT_name = QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);

        if (!qstrPreselected.isEmpty() && (0 == qstrPreselected.compare(OT_id)))
            bFoundPreselected = true;
    } // for
    // -------------------------------------
    if (bFoundPreselected)
        pWindow->SetPreSelected(qstrPreselected);
    // -------------------------------------
    pWindow->setWindowTitle(tr("Manage Currency Accounts"));
    // -------------------------------------
    pWindow->dialog(MTDetailEdit::DetailEditTypeAccount, true);
    // -------------------------------------
}


// returns true if the current default is available on the list.
// (Even when it returns false, that just means no default was found on the list,
//  so there may very well still BE a list for you to use -- just no default.)
//
bool PageOffer_Accounts::setupMapOfAccounts(mapIDName & accountMap, bool bIsAsset_or_currency)
{
    QString qstrNymID       = field("NymID")      .toString();
    QString qstrNotaryID    = field("NotaryID")   .toString();
    QString qstrAssetID;
    QString qstrCurrencyID;
    if (bIsAsset_or_currency)
        qstrAssetID = field("InstrumentDefinitionID") .toString();
    else
        qstrCurrencyID = field("CurrencyID") .toString();
    // -------------------------------------------
    QString qstrInstrumentDefinitionID  = bIsAsset_or_currency ? qstrAssetID : qstrCurrencyID;
    // -------------------------------------------
    QString qstr_current_id = bIsAsset_or_currency ? field("AssetAcctID").toString() : field("CurrencyAcctID").toString();
    // -------------------------------------------
    bool bFoundDefault = false;
    // -----------------------------------------------
    const int32_t the_count = opentxs::OT::App().API().Exec().GetAccountCount();
    // -----------------------------------------------
    for (int32_t ii = 0; ii < the_count; ++ii)
    {
        QString OT_id = QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_ID(ii));
        QString OT_name("");
        // -----------------------------------------------
        if (!OT_id.isEmpty())
        {
            // Filter the accounts shown based on asset type, server ID, and Nym ID.
            //
            QString qstrAcctNymID    = QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_NymID(OT_id.toStdString()));
            QString qstrAcctInstrumentDefinitionID  = QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID (OT_id.toStdString()));
            QString qstrAcctNotaryID = QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_NotaryID(OT_id.toStdString()));
            // -----------------------------------------------
            if ((qstrAcctNymID    != qstrNymID)   ||
                (qstrAcctInstrumentDefinitionID  != qstrInstrumentDefinitionID) ||
                (qstrAcctNotaryID != qstrNotaryID))
                continue;
            // -----------------------------------------------
            if (!qstr_current_id.isEmpty() && (0 == qstr_current_id.compare(OT_id)))
                bFoundDefault = true;
            // -----------------------------------------------
            OT_name = QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_Name(OT_id.toStdString()));
            // -----------------------------------------------
            accountMap.insert(OT_id, OT_name);
        }
    }
    // -----------------------------------------------
    return bFoundDefault;
}


void PageOffer_Accounts::on_pushButtonSelectAssetAcct_clicked()
{
    QString qstrNymID    = field("NymID")   .toString();
    QString qstrInstrumentDefinitionID  = field("InstrumentDefinitionID") .toString();
    QString qstrNotaryID = field("NotaryID").toString();
    // -------------------------------------------
    QString qstr_current_id = field("AssetAcctID").toString();
    // -------------------------------------------
    if (qstr_current_id.isEmpty() && (opentxs::OT::App().API().Exec().GetAccountCount() > 0))
        qstr_current_id = QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_ID(0));
    // -------------------------------------------
    // Select from asset accounts in local wallet.
    //
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;

    bool bFoundDefault = setupMapOfAccounts(the_map, true);

    if (bFoundDefault)
        theChooser.SetPreSelected(qstr_current_id);
    // -----------------------------------------------
    theChooser.setWindowTitle(tr("Select the Asset Account"));
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
        if (!theChooser.m_qstrCurrentID  .isEmpty() &&
            !theChooser.m_qstrCurrentName.isEmpty())
        {
            setField("AssetAcctName", theChooser.m_qstrCurrentName);
            setField("AssetAcctID",   theChooser.m_qstrCurrentID);
            // -----------------------------------------
            ui->lineEditAssetAcctID->home(false);
            // -----------------------------------------
            int64_t     lBalance      = opentxs::OT::App().API().Exec().GetAccountWallet_Balance(theChooser.m_qstrCurrentID.toStdString());
            std::string str_formatted = opentxs::OT::App().API().Exec().FormatAmount(qstrInstrumentDefinitionID.toStdString(), lBalance);
            QString     qstrBalance   = QString::fromStdString(str_formatted);

            setField("AssetAcctBalance", qstrBalance);
        }
    }
}

void PageOffer_Accounts::on_pushButtonSelectCurrencyAcct_clicked()
{
    QString qstrNymID                   = field("NymID")      .toString();
    QString qstrInstrumentDefinitionID  = field("CurrencyID") .toString();
    QString qstrNotaryID                = field("NotaryID")   .toString();
    // -------------------------------------------
    QString qstr_current_id = field("CurrencyAcctID").toString();
    // -------------------------------------------
    if (qstr_current_id.isEmpty() && (opentxs::OT::App().API().Exec().GetAccountCount() > 0))
        qstr_current_id = QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_ID(0));
    // -------------------------------------------
    // Select from currency accounts in local wallet.
    //
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;

    bool bFoundDefault = setupMapOfAccounts(the_map, false);

    if (bFoundDefault)
        theChooser.SetPreSelected(qstr_current_id);
    // -----------------------------------------------
    theChooser.setWindowTitle(tr("Select the Currency Account"));
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
        if (!theChooser.m_qstrCurrentID  .isEmpty() &&
            !theChooser.m_qstrCurrentName.isEmpty())
        {
            setField("CurrencyAcctName", theChooser.m_qstrCurrentName);
            setField("CurrencyAcctID",   theChooser.m_qstrCurrentID);
            // -----------------------------------------
            ui->lineEditCurrencyAcctID->home(false);
            // -----------------------------------------
            int64_t     lBalance      = opentxs::OT::App().API().Exec().GetAccountWallet_Balance(theChooser.m_qstrCurrentID.toStdString());
            std::string str_formatted = opentxs::OT::App().API().Exec().FormatAmount(qstrInstrumentDefinitionID.toStdString(), lBalance);
            QString     qstrBalance   = QString::fromStdString(str_formatted);

            setField("CurrencyAcctBalance", qstrBalance);
        }
    }
}


void PageOffer_Accounts::initializePage()
{
    // -------------------------------------------
    if (!Moneychanger::It()->expertMode())
    {
        ui->pushButtonManageAssetAcct->setVisible(false);
        ui->pushButtonManageCurrencyAcct->setVisible(false);
    }
    // -------------------------------------------
    QWizard        * pWizard         = wizard();
    WizardNewOffer * pWizardNewOffer = (WizardNewOffer *)pWizard;

    if (NULL != pWizardNewOffer) // Cast, so I can access these members:
    {
        setField("NymID",      pWizardNewOffer->GetNymID     ());
        setField("NymName",    pWizardNewOffer->GetNymName   ());
        setField("NotaryID",   pWizardNewOffer->GetNotaryID  ());
        setField("ServerName", pWizardNewOffer->GetServerName());

        ui->lineEditNotaryID->home(false);
        ui->lineEditNymID   ->home(false);
    }
    // -------------------------------------------
    QString qstrNymID      = field("NymID")      .toString();
    QString qstrAssetID    = field("InstrumentDefinitionID").toString();
    QString qstrCurrencyID = field("CurrencyID") .toString();
    QString qstrNotaryID   = field("NotaryID")   .toString();
    // -------------------------------------------
    QString qstrAssetAccountID    = field("AssetAcctID")   .toString();
    QString qstrCurrencyAccountID = field("CurrencyAcctID").toString();
    // -------------------------------------------
//  if (qstrAssetAccountID.isEmpty())
    {
        mapIDName accountMap;
        const bool bFoundDefault = setupMapOfAccounts(accountMap, true);
        const bool bGotAccounts  = accountMap.size() > 0;
        // const bool bGotAccounts  = MtContactHandler::getInstance()->GetAccounts(accountMap, qstrNymID, qstrNotaryID, qstrAssetID);

        if (bGotAccounts)
        {
            int nIndex = -1;
            mapIDName::iterator it_accounts = accountMap.begin();

            for (; it_accounts !=  accountMap.end(); ++it_accounts)
            {
                ++nIndex; // 0 on first iteration.
                // --------------------------------
                // If the ID was pre-set, we make sure to set the name based on it.
                //
                if (0 == it_accounts.key().compare(qstrAssetAccountID))
                {
                    setField("AssetAcctID", it_accounts.key());
                    setField("AssetAcctName", it_accounts.value());
                    break;
                }
                // --------------------------------
                // else if there is at least ONE account with the matching
                // IDs, we just take the first matching one.
                else if (0 == nIndex)
                {
                    setField("AssetAcctID", it_accounts.key());
                    setField("AssetAcctName", it_accounts.value());
                }
            }
            qstrAssetAccountID = field("AssetAcctID").toString();
        }
    }
    // -------------------------------------------
    // -------------------------------------------
//  if (qstrCurrencyAccountID.isEmpty())
    {
        mapIDName accountMap;
        const bool bFoundDefault = setupMapOfAccounts(accountMap, false);
        const bool bGotAccounts  = accountMap.size() > 0;
        // const bool bGotAccounts  = MtContactHandler::getInstance()->GetAccounts(accountMap, qstrNymID, qstrNotaryID, qstrAssetID);

        if (bGotAccounts)
        {
            int nIndex = -1;
            mapIDName::iterator it_accounts = accountMap.begin();

            for (; it_accounts !=  accountMap.end(); ++it_accounts)
            {
                ++nIndex; // 0 on first iteration.
                // --------------------------------
                // If the ID was pre-set, we make sure to set the name based on it.
                //
                if (0 == it_accounts.key().compare(qstrCurrencyAccountID))
                {
                    setField("CurrencyAcctID", it_accounts.key());
                    setField("CurrencyAcctName", it_accounts.value());
                    break;
                }
                // --------------------------------
                // else if there is at least ONE account with the matching
                // IDs, we just take the first matching one.
                else if (0 == nIndex)
                {
                    setField("CurrencyAcctID", it_accounts.key());
                    setField("CurrencyAcctName", it_accounts.value());
                }
            }
            qstrCurrencyAccountID = field("CurrencyAcctID").toString();
        }
    }
    // -------------------------------------------
    if (qstrCurrencyAccountID.isEmpty() || qstrAssetAccountID.isEmpty())
    {
        QMessageBox::warning(this, tr("Moneychanger"),
                             tr("Your nym doesn't have matching asset accounts for the selected asset (or currency) type. "
                                "Please create the appropriate asset accounts first, or select a different asset/currency pair to trade."));
        QTimer::singleShot(0, wizard(), SLOT(back()));
        return;
    }
    // -------------------------------------------
    const bool bIsBid        = field("bid").toBool();

    QString qstrAssetName    = field("AssetName").toString();
    QString qstrCurrencyName = field("CurrencyName").toString();

    if (bIsBid)
    {
        ui->labelAsset->        setText(QString("%1:").arg(tr("Buying Asset")));
        ui->labelAssetName->    setText(qstrAssetName);
        ui->labelCurrency->     setText(QString("%1:").arg(tr("With Currency")));
        ui->labelCurrencyName-> setText(qstrCurrencyName);
    }
    else
    {
        ui->labelAsset->        setText(QString("%1:").arg(tr("Selling Asset")));
        ui->labelAssetName->    setText(qstrAssetName);
        ui->labelCurrency->     setText(QString("%1:").arg(tr("For Currency")));
        ui->labelCurrencyName-> setText(qstrCurrencyName);
    }
    // ---------------------------------------------------------
    {
        ui->lineEditAssetAcctID->home(false);
        // -----------------------------------------
        int64_t     lBalance      = opentxs::OT::App().API().Exec().GetAccountWallet_Balance(qstrAssetAccountID.toStdString());
        std::string str_formatted = opentxs::OT::App().API().Exec().FormatAmount(qstrAssetID.toStdString(), lBalance);
        QString     qstrBalance   = QString::fromStdString(str_formatted);

        setField("AssetAcctBalance", qstrBalance);
    }
    // ---------------------------------------------------------
    {
        ui->lineEditCurrencyAcctID->home(false);
        // -----------------------------------------
        int64_t     lBalance      = opentxs::OT::App().API().Exec().GetAccountWallet_Balance(qstrCurrencyAccountID.toStdString());
        std::string str_formatted = opentxs::OT::App().API().Exec().FormatAmount(qstrCurrencyID.toStdString(), lBalance);
        QString     qstrBalance   = QString::fromStdString(str_formatted);

        setField("CurrencyAcctBalance", qstrBalance);
    }
    // ---------------------------------------------------------
}

PageOffer_Accounts::~PageOffer_Accounts()
{
    delete ui;
}



