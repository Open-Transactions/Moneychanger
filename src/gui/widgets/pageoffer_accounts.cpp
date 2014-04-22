#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/pageoffer_accounts.hpp>
#include <ui_pageoffer_accounts.h>

#include <gui/widgets/dlgchooser.hpp>
#include <gui/widgets/detailedit.hpp>
#include <gui/widgets/wizardnewoffer.hpp>

#include <opentxs/OTAPI.hpp>
#include <opentxs/OTAPI_Exec.hpp>


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

    ui->lineEditServerID           ->setStyleSheet("QLineEdit { border: none; background-color: lightgray }");
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

    this->registerField("ServerID",            ui->lineEditServerID);
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
    QString qstrPreselected   = field("ServerID").toString();
    bool    bFoundPreselected = false;
    // -------------------------------------
    int32_t the_count = OTAPI_Wrap::It()->GetServerCount();

    for (int32_t ii = 0; ii < the_count; ii++)
    {
        QString OT_id   = QString::fromStdString(OTAPI_Wrap::It()->GetServer_ID(ii));
        QString OT_name = QString::fromStdString(OTAPI_Wrap::It()->GetServer_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);

        if (!qstrPreselected.isEmpty() && (qstrPreselected == OT_id))
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
    int32_t the_count = OTAPI_Wrap::It()->GetNymCount();

    for (int32_t ii = 0; ii < the_count; ii++)
    {
        QString OT_id   = QString::fromStdString(OTAPI_Wrap::It()->GetNym_ID(ii));
        QString OT_name = QString::fromStdString(OTAPI_Wrap::It()->GetNym_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);

        if (!qstrPreselected.isEmpty() && (qstrPreselected == OT_id))
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
    int32_t the_count = OTAPI_Wrap::It()->GetAccountCount();

    for (int32_t ii = 0; ii < the_count; ii++)
    {
        QString OT_id   = QString::fromStdString(OTAPI_Wrap::It()->GetAccountWallet_ID(ii));
        QString OT_name = QString::fromStdString(OTAPI_Wrap::It()->GetAccountWallet_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);

        if (!qstrPreselected.isEmpty() && (qstrPreselected == OT_id))
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
    int32_t the_count = OTAPI_Wrap::It()->GetAccountCount();

    for (int32_t ii = 0; ii < the_count; ii++)
    {
        QString OT_id   = QString::fromStdString(OTAPI_Wrap::It()->GetAccountWallet_ID(ii));
        QString OT_name = QString::fromStdString(OTAPI_Wrap::It()->GetAccountWallet_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);

        if (!qstrPreselected.isEmpty() && (qstrPreselected == OT_id))
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



void PageOffer_Accounts::on_pushButtonSelectAssetAcct_clicked()
{
    QString qstrNymID    = field("NymID")   .toString();
    QString qstrAssetID  = field("AssetID") .toString();
    QString qstrServerID = field("ServerID").toString();
    // -------------------------------------------
    QString qstr_current_id = field("AssetAcctID").toString();
    // -------------------------------------------
    if (qstr_current_id.isEmpty() && (OTAPI_Wrap::It()->GetAccountCount() > 0))
        qstr_current_id = QString::fromStdString(OTAPI_Wrap::It()->GetAccountWallet_ID(0));
    // -------------------------------------------
    // Select from asset accounts in local wallet.
    //
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;

    bool bFoundDefault = false;
    // -----------------------------------------------
    const int32_t the_count = OTAPI_Wrap::It()->GetAccountCount();
    // -----------------------------------------------
    for (int32_t ii = 0; ii < the_count; ++ii)
    {
        QString OT_id = QString::fromStdString(OTAPI_Wrap::It()->GetAccountWallet_ID(ii));
        QString OT_name("");
        // -----------------------------------------------
        if (!OT_id.isEmpty())
        {
            // Filter the accounts shown based on asset type, server ID, and Nym ID.
            //
            QString qstrAcctNymID    = QString::fromStdString(OTAPI_Wrap::It()->GetAccountWallet_NymID       (OT_id.toStdString()));
            QString qstrAcctAssetID  = QString::fromStdString(OTAPI_Wrap::It()->GetAccountWallet_AssetTypeID (OT_id.toStdString()));
            QString qstrAcctServerID = QString::fromStdString(OTAPI_Wrap::It()->GetAccountWallet_ServerID    (OT_id.toStdString()));
            // -----------------------------------------------
            if ((qstrAcctNymID    != qstrNymID)   ||
                (qstrAcctAssetID  != qstrAssetID) ||
                (qstrAcctServerID != qstrServerID))
                continue;
            // -----------------------------------------------
            if (!qstr_current_id.isEmpty() && (OT_id == qstr_current_id))
                bFoundDefault = true;
            // -----------------------------------------------
            OT_name = QString::fromStdString(OTAPI_Wrap::It()->GetAccountWallet_Name(OT_id.toStdString()));
            // -----------------------------------------------
            the_map.insert(OT_id, OT_name);
        }
    }
    // -----------------------------------------------
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
            int64_t     lBalance      = OTAPI_Wrap::It()->GetAccountWallet_Balance(theChooser.m_qstrCurrentID.toStdString());
            std::string str_formatted = OTAPI_Wrap::It()->FormatAmount(qstrAssetID.toStdString(), lBalance);
            QString     qstrBalance   = QString::fromStdString(str_formatted);

            setField("AssetAcctBalance", qstrBalance);
        }
    }
}

void PageOffer_Accounts::on_pushButtonSelectCurrencyAcct_clicked()
{
    QString qstrNymID    = field("NymID")      .toString();
    QString qstrAssetID  = field("CurrencyID") .toString();
    QString qstrServerID = field("ServerID")   .toString();
    // -------------------------------------------
    QString qstr_current_id = field("CurrencyAcctID").toString();
    // -------------------------------------------
    if (qstr_current_id.isEmpty() && (OTAPI_Wrap::It()->GetAccountCount() > 0))
        qstr_current_id = QString::fromStdString(OTAPI_Wrap::It()->GetAccountWallet_ID(0));
    // -------------------------------------------
    // Select from currency accounts in local wallet.
    //
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;

    bool bFoundDefault = false;
    // -----------------------------------------------
    const int32_t the_count = OTAPI_Wrap::It()->GetAccountCount();
    // -----------------------------------------------
    for (int32_t ii = 0; ii < the_count; ++ii)
    {
        QString OT_id = QString::fromStdString(OTAPI_Wrap::It()->GetAccountWallet_ID(ii));
        QString OT_name("");
        // -----------------------------------------------
        if (!OT_id.isEmpty())
        {
            // Filter the accounts shown based on asset type, server ID, and Nym ID.
            //
            QString qstrAcctNymID    = QString::fromStdString(OTAPI_Wrap::It()->GetAccountWallet_NymID       (OT_id.toStdString()));
            QString qstrAcctAssetID  = QString::fromStdString(OTAPI_Wrap::It()->GetAccountWallet_AssetTypeID (OT_id.toStdString()));
            QString qstrAcctServerID = QString::fromStdString(OTAPI_Wrap::It()->GetAccountWallet_ServerID    (OT_id.toStdString()));
            // -----------------------------------------------
            if ((qstrAcctNymID    != qstrNymID)   ||
                (qstrAcctAssetID  != qstrAssetID) ||
                (qstrAcctServerID != qstrServerID))
                continue;
            // -----------------------------------------------
            if (!qstr_current_id.isEmpty() && (OT_id == qstr_current_id))
                bFoundDefault = true;
            // -----------------------------------------------
            OT_name = QString::fromStdString(OTAPI_Wrap::It()->GetAccountWallet_Name(OT_id.toStdString()));
            // -----------------------------------------------
            the_map.insert(OT_id, OT_name);
        }
    }
    // -----------------------------------------------
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
            int64_t     lBalance      = OTAPI_Wrap::It()->GetAccountWallet_Balance(theChooser.m_qstrCurrentID.toStdString());
            std::string str_formatted = OTAPI_Wrap::It()->FormatAmount(qstrAssetID.toStdString(), lBalance);
            QString     qstrBalance   = QString::fromStdString(str_formatted);

            setField("CurrencyAcctBalance", qstrBalance);
        }
    }
}


void PageOffer_Accounts::initializePage()
{
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
    QWizard        * pWizard         = wizard();
    WizardNewOffer * pWizardNewOffer = (WizardNewOffer *)pWizard;

    if (NULL != pWizardNewOffer)
    {
        setField("NymID",      pWizardNewOffer->GetNymID     ());
        setField("NymName",    pWizardNewOffer->GetNymName   ());
        setField("ServerID",   pWizardNewOffer->GetServerID  ());
        setField("ServerName", pWizardNewOffer->GetServerName());

        ui->lineEditServerID->home(false);
        ui->lineEditNymID   ->home(false);
    }
}

PageOffer_Accounts::~PageOffer_Accounts()
{
    delete ui;
}



