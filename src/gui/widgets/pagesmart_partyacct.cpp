#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include "pagesmart_partyacct.hpp"
#include "ui_pagesmart_partyacct.h"

#include <gui/widgets/wizardpartyacct.hpp>

#include <gui/widgets/dlgchooser.hpp>

#include <core/moneychanger.hpp>

#include <opentxs/opentxs.hpp>

#include <QMessageBox>


PageSmart_PartyAcct::PageSmart_PartyAcct(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::PageSmart_PartyAcct)
{
    ui->setupUi(this);

    QString style_sheet = "QPushButton{border: none; border-style: outset; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa);}"
            "QPushButton:pressed {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }"
            "QPushButton:hover {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }";

    ui->pushButtonSelect->setStyleSheet(style_sheet);

    ui->plainTextEditTemplate->setVisible(false);
    ui->lineEditPartyName->setVisible(false);

    m_qstrClickText = QString("<%1>").arg(tr("Click to choose Account"));

    this->registerField("AcctName*", ui->pushButtonSelect, "text");
    this->registerField("PartyName", ui->lineEditPartyName);
    this->registerField("AccountState", ui->plainTextEdit, "plainText");
    this->registerField("SmartTemplate", ui->plainTextEditTemplate, "plainText");

    ui->plainTextEdit->setVisible(false);
}

PageSmart_PartyAcct::~PageSmart_PartyAcct()
{
    delete ui;
}

void PageSmart_PartyAcct::SetFieldsBlank()
{
    setField("AcctName", m_qstrClickText);

//    emit completeChanged();
}


void PageSmart_PartyAcct::on_pushButtonSelect_clicked()
{
    QWizard * pWizard = wizard();

    if (NULL == pWizard)
        return;

    WizardPartyAcct * pWizardPartyAcct = dynamic_cast<WizardPartyAcct*>(pWizard);

    if (NULL == pWizardPartyAcct)
        return;

    mapIDName & mapConfirmed = pWizardPartyAcct->m_mapConfirmed;
    // -------------------------------------------
    QString qstrTemplate = field("SmartTemplate").toString();
    std::string str_template = qstrTemplate.toStdString();
    // -------------------------------------------
    QString qstrParty = field("PartyName").toString();
    std::string str_party    = qstrParty.toStdString();
    // -------------------------------------------
    QString qstr_current_id = field("AcctName").toString();

    if (0 == qstr_current_id.compare(m_qstrClickText))
        qstr_current_id = QString("");
    // -------------------------------------------
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;

    bool bFoundDefault = false;
    // -----------------------------------------------
    int32_t acct_count = Moneychanger::It()->OT().Exec().Party_GetAcctCount(str_template, str_party);

    for (int32_t i = 0; i < acct_count; i++)
    {
        std::string acctName =
            Moneychanger::It()->OT().Exec().Party_GetAcctNameByIndex(str_template, str_party, i);

        if ("" == acctName) {
            QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("Strange, there is an account on this smart contract without a name. Failure."));
            wizard()->reject();
            return;
        }
        QString qstrAcctName = QString::fromStdString(acctName);
        mapIDName::iterator it = mapConfirmed.find(qstrAcctName);

        bool alreadyConfirmed = (mapConfirmed.end() != it);

        std::string partyAcctID = Moneychanger::It()->OT().Exec().Party_GetAcctID(str_template, str_party, acctName);

        if (alreadyConfirmed || "" != partyAcctID) {
            continue;
        }
        // ------------------------------
        // It's not confirmed yet, so we can add it to the list of accounts to choose from.
        //
        QString OT_id = qstrAcctName;
        QString OT_name = OT_id;
        // -----------------------------------------------
        if (!OT_id.isEmpty())
        {
            if (!qstr_current_id.isEmpty() && (0 == qstr_current_id.compare(OT_id)))
                bFoundDefault = true;
            // -----------------------------------------------
            the_map.insert(OT_id, OT_name);
        }
    }
    // -----------------------------------------------
    // This should never happen because as long as the account
    // count is not the same as the mapConfirmed size, we thus still
    // have more accounts that need to be confirmed. That's the only
    // reason we're in this function at all -- Moneychanger popped up
    // this wizard to collect the next account because it can see
    // more accounts still need to be confirmed and that's how it decided
    // to pop up this wizard in the first place.
    //
    if (0 == the_map.size())
    {
        QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME),
                                 tr("Strange: though I didn't think it was the case at first, It seems now that all accounts ARE already confirmed on this contract. (Failure.)"));
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
            setField("AcctName", theChooser.m_qstrCurrentID);
            emit completeChanged();
            return;
        }
    }
}

bool PageSmart_PartyAcct::isComplete() const
{
    QString qstr_current_id = field("AcctName").toString();

    if (0 == qstr_current_id.compare(m_qstrClickText))
        qstr_current_id = QString("");

    if (qstr_current_id.isEmpty())
        return false;

    return true;
}

void PageSmart_PartyAcct::initializePage() //virtual
{
    QString qstr_id = field("AcctName").toString();
    // -------------------------------------------
    if (qstr_id.isEmpty())
        SetFieldsBlank();
    else
    {
        setField("AcctName", qstr_id);
//        emit completeChanged();
    }
}


void PageSmart_PartyAcct::on_plainTextEdit_textChanged()
{
    ui->plainTextEdit->setVisible(true);
}

void PageSmart_PartyAcct::on_lineEditPartyName_textChanged(const QString &arg1)
{
    QString qstrParty = field("PartyName").toString();

    ui->label->setText(QString("%1 \"%2\":").arg(tr("Select an unconfirmed account owned by the party")).arg(qstrParty));
}
