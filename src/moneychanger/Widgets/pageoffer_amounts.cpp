
#include <cmath>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>

#include "pageoffer_amounts.h"
#include "ui_pageoffer_amounts.h"

PageOffer_Amounts::PageOffer_Amounts(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::PageOffer_Amounts)
{
    ui->setupUi(this);

    registerField("isMarketOrder", ui->radioButton);
    registerField("isLimitOrder",  ui->radioButton_2);
    registerField("isFillOrKill",  ui->checkBox);

    ui->lineEditTotal->setStyleSheet("QLineEdit { border: none; background-color: lightgray }");

//    ui->lineEditQuantity;
//    ui->lineEditPrice;
}


void PageOffer_Amounts::on_lineEditPrice_textChanged(const QString &arg1)
{
    // ----------------------------------------------
    QString qstrCurrencyID = field("CurrencyID").toString();
    const std::string str_currency(qstrCurrencyID.toStdString());
    // ----------------------------------------------
    QString qstrPrice = arg1;
    std::string str_price = qstrPrice.toStdString();
    // ----------------------------------------------
    int64_t lPrice = OTAPI_Wrap::StringToAmount(str_currency, str_price);

    std::string str_final(OTAPI_Wrap::FormatAmount(str_currency, lPrice));
    QString qstrFinal(QString::fromStdString(str_final));

    ui->labelCalculatedPrice->setText(qstrFinal);
}


void PageOffer_Amounts::on_lineEditQuantity_textChanged(const QString &arg1)
{
    QString qstrCombo    = ui->comboBox->currentText();
    QString qstrQuantity = arg1;

    int64_t lQuantity = static_cast<int64_t>(qstrQuantity.toLong());
    // ----------------------------------------------
    QString qstrAssetID = field("AssetID").toString();
    const std::string str_asset(qstrAssetID.toStdString());
    // ----------------------------------------------
    if (!qstrCombo.isEmpty())
    {
        int64_t lCombo  = OTAPI_Wrap::StringToAmount(str_asset, qstrCombo.toStdString());
        int64_t lResult = (lQuantity * lCombo);

        std::string str_result = OTAPI_Wrap::FormatAmount(str_asset, lResult);

        ui->lineEditTotal->setText(QString::fromStdString(str_result));
    }
}

void PageOffer_Amounts::on_comboBox_currentIndexChanged(const QString &arg1)
{
    QString qstrPer(QString("%1 %2").arg(tr("per")).arg(arg1));
    ui->labelPer->setText(qstrPer);
    // -------------------------------------------------
    QString qstrCombo    = arg1;
    QString qstrQuantity = ui->lineEditQuantity->text();

    int64_t lQuantity = static_cast<int64_t>(qstrQuantity.toLong());
    // ----------------------------------------------
    QString qstrAssetID = field("AssetID").toString();
    const std::string str_asset(qstrAssetID.toStdString());
    // ----------------------------------------------
    if (!qstrCombo.isEmpty())
    {
        int64_t lCombo  = OTAPI_Wrap::StringToAmount(str_asset, qstrCombo.toStdString());
        int64_t lResult = (lQuantity * lCombo);

        std::string str_result = OTAPI_Wrap::FormatAmount(str_asset, lResult);

        ui->lineEditTotal->setText(QString::fromStdString(str_result));
    }
}

void PageOffer_Amounts::on_radioButton_clicked()
{
    RadioChanged();
}

void PageOffer_Amounts::on_radioButton_2_clicked()
{
    RadioChanged();
}

void PageOffer_Amounts::RadioChanged()
{
    const bool bIsMarketOrder = field("isMarketOrder").toBool();

    if (bIsMarketOrder)
    {
        ui->labelPrice          ->setStyleSheet("QLabel { color : gray; }");
        ui->labelPer            ->setStyleSheet("QLabel { color : gray; }");
        ui->labelCalculatedPrice->setStyleSheet("QLabel { color : gray; }");

        ui->lineEditPrice->setReadOnly(true);
        ui->lineEditPrice->setEnabled(false);
    }
    else // Limit order
    {
        ui->labelPrice          ->setStyleSheet("QLabel { color : black; }");
        ui->labelPer            ->setStyleSheet("QLabel { color : black; }");
        ui->labelCalculatedPrice->setStyleSheet("QLabel { color : black; }");

        ui->lineEditPrice->setReadOnly(false);
        ui->lineEditPrice->setEnabled(true);
    }
}

void PageOffer_Amounts::initializePage()
{
    const bool bIsBid = field("bid").toBool();

    if (bIsBid)
    {
        ui->labelPrice   ->setText(QString("%1:").arg(tr("At Maximum Price")));
        ui->labelQuantity->setText(QString("%1:").arg(tr("Units Buying")));
        ui->labelTotal   ->setText(QString("%1:").arg(tr("Total Quantity for Purchase")));
    }
    else
    {
        ui->labelPrice   ->setText(QString("%1:").arg(tr("At Minimum Price")));
        ui->labelQuantity->setText(QString("%1:").arg(tr("Units Selling")));
        ui->labelTotal   ->setText(QString("%1:").arg(tr("Total Quantity for Sale")));
    }
    // ----------------------------------------------
//    QString qstrAssetName    = field("AssetName")   .toString();
//    QString qstrCurrencyName = field("CurrencyName").toString();

//    ui->labelAssetName   ->setText(qstrAssetName);
//    ui->labelCurrencyName->setText(qstrCurrencyName);
    // ----------------------------------------------
    QString qstrAssetID = field("AssetID").toString();
    const std::string str_asset(qstrAssetID.toStdString());
    // ----------------------------------------------
    // Populate the Combo Box
    //
    ui->comboBox->blockSignals(true);

    ui->comboBox->clear();

    for (int ii = 0; ii < 8; ++ii)
    {
        float   dValue = std::pow(10.0, ii);
        long    lScale = static_cast<long>(dValue);

        OTString strTemp;
        strTemp.Format("%ld\.", lScale); // So 1 becomes "1." which StringToAmount makes into 1.000 which is actually 1000

        const std::string str_input(strTemp.Get());

        int64_t lAmount = OTAPI_Wrap::StringToAmount(str_asset, str_input);

        std::string str_formatted = OTAPI_Wrap::FormatAmount(str_asset, lAmount);
        QString qstrFormatted(QString::fromStdString(str_formatted));

        QVariant qvarVal(lAmount);

        ui->comboBox->addItem(qstrFormatted, qvarVal);
    }
    // ----------------------------------------------
    ui->comboBox->blockSignals(false);

    ui->comboBox->setCurrentIndex(-1);
    ui->comboBox->setCurrentIndex(0);
    // ----------------------------------------------
    RadioChanged();
}


PageOffer_Amounts::~PageOffer_Amounts()
{
    delete ui;
}

