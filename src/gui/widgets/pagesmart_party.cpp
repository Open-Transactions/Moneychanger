#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include "pagesmart_party.hpp"
#include "ui_pagesmart_party.h"

#include <gui/widgets/dlgchooser.hpp>

#include <core/moneychanger.hpp>

#include <opentxs/opentxs.hpp>

#include <QMessageBox>

PageSmart_Party::PageSmart_Party(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::PageSmart_Party)
{
    ui->setupUi(this);

    QString style_sheet = "QPushButton{border: none; border-style: outset; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa);}"
            "QPushButton:pressed {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }"
            "QPushButton:hover {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }";

    ui->pushButtonSelect->setStyleSheet(style_sheet);

    m_qstrClickText = QString("<%1>").arg(tr("Click to choose Party"));

    ui->plainTextEdit->setVisible(false);

    this->registerField("PartyName*", ui->pushButtonSelect, "text");
    this->registerField("SmartTemplate", ui->plainTextEdit, "plainText");
}

PageSmart_Party::~PageSmart_Party()
{
    delete ui;
}

void PageSmart_Party::SetFieldsBlank()
{
    setField("PartyName", m_qstrClickText);
}

bool PageSmart_Party::isComplete() const
{
    QString qstr_current_id = field("PartyName").toString();

    if (0 == qstr_current_id.compare(m_qstrClickText))
        qstr_current_id = QString("");

    if (qstr_current_id.isEmpty())
        return false;

    return true;
}

void PageSmart_Party::on_pushButtonSelect_clicked()
{
    QString qstrTemplate = field("SmartTemplate").toString();
    std::string str_template = qstrTemplate.toStdString();

    if (str_template.empty()) // Should never happen.
    {
        QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), tr("Strange, somehow the SmartTemplate field is empty. Unable to proceed."));
        return;
    }
    // -------------------------------------------
    QString qstr_current_id = field("PartyName").toString();

    if (0 == qstr_current_id.compare(m_qstrClickText))
        qstr_current_id = QString("");
    // -------------------------------------------
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;

    bool bFoundDefault = false;
    // -----------------------------------------------
    // Iterate parties using Smart_GetPartyCount and Smart_GetPartyByIndex and get a list of unconfirmed parties using Smart_IsPartyConfirmed.

    int32_t party_count = opentxs::OT::App().API().Exec().Smart_GetPartyCount(str_template);

    for (int32_t i = 0; i < party_count; i++)
    {
        std::string name = opentxs::OT::App().API().Exec().Smart_GetPartyByIndex(str_template, i);

        if ("" == name) {
            QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("Strange, there is a party on this smart contract without a name. Failure."));
            wizard()->reject();
            return;
        }

        if (!opentxs::OT::App().API().Exec().Smart_IsPartyConfirmed(str_template, name))
        {
            QString OT_id = QString::fromStdString(name);
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
    }
    // -----------------------------------------------
    if (0 == the_map.size())
    {
        QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), tr("Strange: though I didn't think it was the case at first, I'm sure now that all parties ARE already confirmed on this contract. (Failure.)"));
        wizard()->reject();
        return;
    }
    // -----------------------------------------------
    if (bFoundDefault)
        theChooser.SetPreSelected(qstr_current_id);
    // -----------------------------------------------
    theChooser.setWindowTitle(tr("Choose the Party you will sign as:"));
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
        if (!theChooser.m_qstrCurrentID.isEmpty())
        {
            setField("PartyName", theChooser.m_qstrCurrentID);
            emit completeChanged();
            return;
        }
    }
}

void PageSmart_Party::initializePage() //virtual
{
    QString     qstr_id = field("PartyName").toString();
    // -------------------------------------------
    if (qstr_id.isEmpty())
        SetFieldsBlank();
    else
    {
        setField("PartyName", qstr_id);
    }
}



