#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include "pagesmart_nymacct.hpp"
#include "ui_pagesmart_nymacct.h"

#include <gui/widgets/wizardpartyacct.hpp>
#include <gui/widgets/home.hpp>
#include <gui/widgets/dlgchooser.hpp>
#include <gui/widgets/detailedit.hpp>

#include <core/handlers/contacthandler.hpp>
#include <core/moneychanger.hpp>

#include <opentxs/opentxs.hpp>

#include <QMessageBox>



PageSmart_NymAcct::PageSmart_NymAcct(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::PageSmart_NymAcct)
{
    ui->setupUi(this);

    QString style_sheet = "QPushButton{border: none; border-style: outset; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa);}"
            "QPushButton:pressed {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }"
            "QPushButton:hover {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }";

    ui->pushButtonSelect->setStyleSheet(style_sheet);

    ui->lineEditAcctID->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditBalance->setStyleSheet("QLineEdit { background-color: lightgray }");

    m_qstrClickText = QString("<%1>").arg(tr("Click to choose Account"));

    this->registerField("AcctID*", ui->lineEditAcctID, "text");
}

PageSmart_NymAcct::~PageSmart_NymAcct()
{
    delete ui;
}

void PageSmart_NymAcct::on_pushButtonManageAcct_clicked()
{
    MTDetailEdit * pWindow = new MTDetailEdit(this);

    pWindow->setAttribute(Qt::WA_DeleteOnClose);
    // -------------------------------------
    mapIDName & the_map = pWindow->m_map;
    // -------------------------------------
    the_map.clear();
    // -------------------------------------
    QString qstrPreselected   = field("AcctID").toString();
    bool    bFoundPreselected = false;
    // -------------------------------------

    for (const auto& [accountID, alias] : opentxs::OT::App().Client().Storage().AccountList())
    {
        QString OT_id   = QString::fromStdString(accountID);
        QString OT_name = QString::fromStdString(alias);

        the_map.insert(OT_id, OT_name);

        if (!qstrPreselected.isEmpty() && (0 == qstrPreselected.compare(OT_id)))
            bFoundPreselected = true;
    } // for
    // -------------------------------------
    if (bFoundPreselected)
        pWindow->SetPreSelected(qstrPreselected);
    // -------------------------------------
    pWindow->setWindowTitle(tr("Manage Accounts"));
    // -------------------------------------
    pWindow->dialog(MTDetailEdit::DetailEditTypeAccount, true);
}


void PageSmart_NymAcct::SetFieldsBlank()
{
    setField("AcctID", QString(""));

    ui->pushButtonSelect->setText(m_qstrClickText);
    ui->lineEditBalance->setText(QString(""));

//    emit completeChanged();
}


void PageSmart_NymAcct::on_pushButtonSelect_clicked()
{
    QWizard * pWizard = wizard();

    if (NULL == pWizard)
        return;

    WizardPartyAcct * pWizardPartyAcct = dynamic_cast<WizardPartyAcct *>(pWizard);

    if (NULL == pWizardPartyAcct)
        return;

    mapIDName & mapConfirmed = pWizardPartyAcct->m_mapConfirmed;

    QString qstrNotaryID = pWizardPartyAcct->m_qstrNotaryID;
    QString qstrNymID = pWizardPartyAcct->m_qstrNymID;

    std::string str_notary_id = qstrNotaryID.toStdString();
    std::string str_nym_id = qstrNymID.toStdString();
    // -------------------------------------------
    QString qstrTemplate = field("SmartTemplate").toString();
    std::string str_template = qstrTemplate.toStdString();
    // -------------------------------------------
    QString qstrParty = field("PartyName").toString();
    std::string str_party    = qstrParty.toStdString();
    // -------------------------------------------
    QString qstrAcctName = field("AcctName").toString();
    std::string str_acct_name = qstrAcctName.toStdString();
    // -------------------------------------------
    std::string templateInstrumentDefinitionID =
        opentxs::OT::App().Client().Exec().Party_GetAcctInstrumentDefinitionID(str_template, str_party, str_acct_name);
    bool foundTemplateInstrumentDefinitionID = "" != templateInstrumentDefinitionID;
    // -------------------------------------------
    QString qstr_current_id = field("AcctID").toString();
    // -------------------------------------------
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;

    bool bFoundDefault = false;

    for (const auto& [acctID, alias] : opentxs::OT::App().Client().Storage().AccountList())
    {
        if ("" == acctID) {
            QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME),
                                     tr("Strange, there is an account in your wallet without an ID. (Failure.)"));
            wizard()->reject();
            return;
        }
        QString qstrAcctID = QString::fromStdString(acctID);

        bool alreadyConfirmed = false;
        for (auto x = mapConfirmed.begin(); x != mapConfirmed.end(); x++) {
            if (x.value() == qstrAcctID) {
                alreadyConfirmed = true;
                break;
            }
        }

        if (alreadyConfirmed)
            continue;
        // --------------------------------------
        std::string acctNotaryID = opentxs::OT::App().Client().Exec().GetAccountWallet_NotaryID(acctID);
        std::string acctNymID = opentxs::OT::App().Client().Exec().GetAccountWallet_NymID(acctID);
        std::string acctInstrumentDefinitionID =
            opentxs::OT::App().Client().Exec().GetAccountWallet_InstrumentDefinitionID(acctID);

        if (str_notary_id != acctNotaryID || str_nym_id != acctNymID)
            continue;
        // --------------------------------------
        // If the smart contract doesn't specify the instrument definition ID of
        // the account, or if it DOES specify, AND they match, then it's a viable
        // choice. Display it.
        //
        if (!foundTemplateInstrumentDefinitionID ||
            templateInstrumentDefinitionID == acctInstrumentDefinitionID)
        {
            QString OT_id = qstrAcctID;
            QString OT_name = OT_id;
            // -----------------------------------------------
            if (!OT_id.isEmpty())
            {
                if (!qstr_current_id.isEmpty() && (0 == qstr_current_id.compare(OT_id)))
                    bFoundDefault = true;
                // -----------------------------------------------
                QString qstrTemp = QString::fromStdString(opentxs::OT::App().Client().Exec().GetAccountWallet_Name(OT_id.toStdString()));

                if (!qstrTemp.isEmpty())
                    OT_name = qstrTemp;

                the_map.insert(OT_id, OT_name);
            }
        }
    }
    // -----------------------------------------------
    if (0 == the_map.size())
    {
        QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME),
                                 tr("Sorry, but for the Nym and Server selected (and possibly the asset type), "
                                    "there are no matching accounts available (or they're already used.) "
                                    "Failure activating smart contract."));
        wizard()->reject();
        return;
    }
    // -----------------------------------------------
    if (bFoundDefault)
        theChooser.SetPreSelected(qstr_current_id);
    // -----------------------------------------------
    theChooser.setWindowTitle(tr("Choose the next account"));
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
        if (!theChooser.m_qstrCurrentID.isEmpty())
        {
            setField("AcctID", theChooser.m_qstrCurrentID);
            ui->pushButtonSelect->setText(theChooser.m_qstrCurrentName);

            QString qstrBalance = MTHome::shortAcctBalance(theChooser.m_qstrCurrentID);

            ui->lineEditBalance->setText(qstrBalance);

            emit completeChanged();
            return;
        }
    }
}

bool PageSmart_NymAcct::isComplete() const
{
    QString qstr_current_id = field("AcctID").toString();

    if (qstr_current_id.isEmpty())
        return false;

    return true;
}

void PageSmart_NymAcct::initializePage() //virtual
{
    if (!Moneychanger::It()->expertMode())
    {
        ui->pushButtonManageAcct->setVisible(false);
    }
    // -------------------------------------------
    QString qstrAcctName = field("AcctName").toString();

    ui->label_2->setText(QString("%1 \"%2\":").arg(tr("Select an account to be")).arg(qstrAcctName));
    // -------------------------------------------
    QString qstr_id = field("AcctID").toString();
    // -------------------------------------------
    if (qstr_id.isEmpty())
        SetFieldsBlank();
    else
    {
        setField("AcctID", qstr_id);
//        emit completeChanged();
    }
}

