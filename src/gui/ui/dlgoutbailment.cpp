#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/ui/dlgoutbailment.hpp>
#include <ui_dlgoutbailment.h>

#include <opentxs/api/Activity.hpp>
#include <opentxs/api/Api.hpp>
#include <opentxs/api/ContactManager.hpp>
#include <opentxs/api/OT.hpp>
#include <opentxs/api/Wallet.hpp>
#include <opentxs/client/OT_API.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>
#include <opentxs/core/Identifier.hpp>

#include <QKeyEvent>
#include <QMessageBox>

DlgOutbailment::DlgOutbailment(QWidget *parent, std::int64_t & AMOUNT, std::string & blockchain_address,
                               QString asset_type, QString issuer_nym, QString notary) :
    QDialog(parent),
    ui(new Ui::DlgOutbailment),
    AMOUNT_(AMOUNT),
    blockchain_address_(blockchain_address),
    asset_type_(asset_type),
    issuer_nym_(issuer_nym),
    notary_(notary)
{
    ui->setupUi(this);

    this->installEventFilter(this);

    ui->lineEditAssetType   ->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditIssuerNym   ->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditNotary      ->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditAssetTypeId ->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditIssuerNymId ->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditNotaryId    ->setStyleSheet("QLineEdit { background-color: lightgray }");

//  ui->lineEditAssetType   ->setText(asset_type);
//  ui->lineEditIssuerNym   ->setText(issuer_nym);
//  ui->lineEditNotary      ->setText(notary);

    ui->lineEditAssetTypeId ->setText(asset_type);
    ui->lineEditIssuerNymId ->setText(issuer_nym);
    ui->lineEditNotaryId    ->setText(notary);

    const opentxs::Identifier asset_id(asset_type_.toStdString());
    opentxs::ConstUnitDefinition unit_definition = opentxs::OT::App().Wallet().UnitDefinition(asset_id);
    const QString qstrAssetAlias = QString::fromStdString(unit_definition->Alias());

    ui->labelAsset->setText(qstrAssetAlias);

    ui->lineEditAmount->setFocus();
}

DlgOutbailment::~DlgOutbailment()
{
    delete ui;
    ui=nullptr;
}

void DlgOutbailment::on_pushButtonWithdraw_clicked()
{
    // -----------------------------------------------------------------
    // Blockchain address:
    if (ui->lineEditBlockchain->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Destination empty"),
                             tr("Please provide a blockchain address for the withdrawal."));
        return;
    }
    // -----------------------------------------------------------------
    // Amount:
    if (ui->lineEditAmount->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Amount empty"),
                             tr("Please enter the amount you wish to withdraw."));
        return;
    }
    // -----------------------------------------------------------------

    on_lineEditAmount_editingFinished();


    // todo here: validate blockchain address.
    // Return here if invalid.

    // -----------------------------------------------------------------
    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(this, "", QString("%1 '%2'<br/>%3")
                                  .arg(tr("The amount is"))
                                  .arg(ui->lineEditAmount->text())
                                  .arg(tr("Withdraw to the blockchain?")),
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        QDialog::accept();
    }
}

void DlgOutbailment::on_lineEditBlockchain_editingFinished()
{
    // todo


}

void DlgOutbailment::on_lineEditAmount_editingFinished()
{
    OT_ASSERT(!asset_type_.isEmpty());

    if (!withdrawalPerformed_)
    {
        QString amt = ui->lineEditAmount->text();
        if (!amt.isEmpty())
        {
            std::string str_temp(amt.toStdString());
            if (std::string::npos == str_temp.find(".")) // not found
                str_temp += '.';
            AMOUNT_ = opentxs::OT::App().API().Exec().StringToAmount(asset_type_.toStdString(), str_temp);
            std::string  str_formatted_amount = opentxs::OT::App().API().Exec().FormatAmount(
                         asset_type_.toStdString(),
                         static_cast<int64_t>(AMOUNT_));
            QString      qstr_FinalAmount     = QString::fromStdString(str_formatted_amount);
            ui->lineEditAmount->setText(qstr_FinalAmount);
        }
    }
}

bool DlgOutbailment::eventFilter(QObject *obj, QEvent *event)
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
    return QDialog::eventFilter(obj, event);
}


