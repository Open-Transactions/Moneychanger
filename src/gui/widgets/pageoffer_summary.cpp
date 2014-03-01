#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/pageoffer_summary.hpp>

#include "pageoffer_summary.h"
#include "ui_pageoffer_summary.h"

PageOffer_Summary::PageOffer_Summary(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::PageOffer_Summary)
{
    ui->setupUi(this);
}

void PageOffer_Summary::initializePage()
{
    const bool bIsBid         = field("bid")          .toBool();
    const bool bIsMarketOrder = field("isMarketOrder").toBool();
    const bool bIsFillOrKill  = field("isFillOrKill") .toBool();

    QString qstrSummary(QString("%1:\n\n").arg(tr("Summary")));
    // --------------------------------------------
    const QString qstrAssetName   (field("AssetName")   .toString());
    const QString qstrCurrencyName(field("CurrencyName").toString());
    // --------------------------------------------
    const QString qstrOrder(QString("%1%2:\n%3 %4 %5 %6\n\n").arg(bIsMarketOrder ? tr("MARKET ORDER") : tr("LIMIT ORDER")).
                            arg(bIsFillOrKill ? tr(" (Fill-or-Kill)") : "").
                            arg(bIsBid ? tr("Buying") : tr("Selling")).arg(qstrAssetName).arg(bIsBid ? tr("with") : tr("for")).
                            arg(qstrCurrencyName));

    qstrSummary += qstrOrder;
    // --------------------------------------------
    const QString qstrTotalAsset = field("totalAsset").toString();
    const QString qstrScale      = field("scale")     .toString();
    // --------------------------------------------
    const QString qstrQuantity(QString("%1 %2, %3 %4\n\n").arg(bIsBid ? tr("Buying") : tr("Selling")).arg(qstrTotalAsset).
                               arg(tr("priced per")).arg(qstrScale));

    qstrSummary += qstrQuantity;
    // --------------------------------------------
    QString qstrPriceLabel;

    if (bIsMarketOrder)
    {
        // "AT ANY PRICE."
        //
        qstrPriceLabel  = QString("%1.\n\n").arg(tr("Warning: Market orders are fulfilled at best available price"));
        qstrPriceLabel += QString("%1.\n\n").arg(tr("Market orders process instantly, success or fail, and do not remain on the market"));
    }
    else // Limit order.
    {
        const QString qstrPrice (field("pricePerScale").toString());
        const QString qstrExpire(field("expirationStr").toString());

        qstrPriceLabel = QString("%1 %2 %3 %4 %5 %6, %7.\n\n").
                arg(tr("Limit order will be fulfilled at")).
                arg(bIsBid ? tr("maximum") : tr("minimum")).
                arg(tr("of")).
                arg(qstrPrice).
                arg(tr("per")).
                arg(qstrScale).
                arg(tr("or best available price"));

        qstrPriceLabel += QString("%1: %2.\n\n").arg("Order expires after").arg(qstrExpire);
    }
    // --------------------------------------------
    if (bIsFillOrKill)
    {
        qstrSummary += QString("%1.\n\n").
                arg(tr("FILL OR KILL: Order will be filled entirely in a single trade, or not at all"));
    }
    // --------------------------------------------
    qstrSummary += qstrPriceLabel;

    ui->plainTextEdit->setPlainText(qstrSummary);
}

PageOffer_Summary::~PageOffer_Summary()
{
    delete ui;
}
