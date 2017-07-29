#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/ui/activity.hpp>
#include <ui_activity.h>

#include <core/moneychanger.hpp>
#include <core/handlers/DBHandler.hpp>
#include <core/handlers/contacthandler.hpp>
#include <core/handlers/focuser.h>

#include <opentxs/api/OT.hpp>
#include <opentxs/client/OTAPI_Wrap.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>
#include <opentxs/core/Log.hpp>
#include <opentxs/core/Types.hpp>
#include <opentxs/core/OTTransaction.hpp>
#include <opentxs/core/OTTransactionType.hpp>
#include <opentxs/client/OTRecordList.hpp>

#include <QLabel>
#include <QToolButton>
#include <QKeyEvent>
#include <QApplication>
#include <QMessageBox>
#include <QMenu>
#include <QList>
#include <QFrame>

#include <string>
#include <map>
#include <tuple>



Activity::Activity(QWidget *parent) :
    QWidget(parent, Qt::Window),
    ui(new Ui::Activity)
{
    ui->setupUi(this);

    this->installEventFilter(this);
}



void Activity::RefreshAll()
{
    RefreshAccountTab();
    RefreshConversationsTab();
}

void Activity::RefreshAccountTab()
{
    RefreshAccountTree();
}

void Activity::ClearAccountTree()
{
    ui->treeWidgetAccounts->clear();
}

//typedef QMap<QString, mapIDName> mapOfMapIDName; // TLA or share symbol, map of IDs/display names.

mapIDName & Activity::GetOrCreateAssetIdMapByCurrencyCode(QString qstrTLA, mapOfMapIDName & bigMap)
{
    mapOfMapIDName::iterator i = bigMap.find(qstrTLA);
    while (i != bigMap.end() && i.key() == qstrTLA) {
        return i.value();
    }
    // else create it.
    bigMap[qstrTLA] = mapIDName{};
    i = bigMap.find(qstrTLA);
    return i.value();
}

void Activity::GetAssetIdMapsByCurrencyCode(mapOfMapIDName & bigMap)
{
    int32_t  asset_count = opentxs::OTAPI_Wrap::Exec()->GetAssetTypeCount();
    for (int32_t ii = 0; ii < asset_count; ii++)
    {
        const QString OT_id   = QString::fromStdString(opentxs::OTAPI_Wrap::Exec()->GetAssetType_ID(ii));
        const QString OT_name = QString::fromStdString(opentxs::OTAPI_Wrap::Exec()->GetAssetType_Name(OT_id.toStdString()));
        const QString qstrTLA = QString::fromStdString(opentxs::OTAPI_Wrap::Exec()->GetCurrencyTLA(OT_id.toStdString()));
        mapIDName & mapTLA = GetOrCreateAssetIdMapByCurrencyCode(qstrTLA, bigMap);
        mapTLA.insert(OT_id, OT_name);
    }
}

int64_t Activity::GetAccountBalancesTotaledForUnitTypes(const mapIDName & mapUnitTypeIds)
{
    int64_t lTotal = 0;

    mapIDName::const_iterator ci = mapUnitTypeIds.begin();
    while (ci != mapUnitTypeIds.end())
    {
        QString qstrUnitTypeId = ci.key();
        // ----------------------------------------------------
        const int nAccountCount = opentxs::OTAPI_Wrap::Exec()->GetAccountCount();
        for (int ii = 0; ii < nAccountCount; ++ii)
        {
            std::string accountID   = opentxs::OTAPI_Wrap::Exec()->GetAccountWallet_ID(ii);
            std::string assetTypeID = opentxs::OTAPI_Wrap::Exec()->GetAccountWallet_InstrumentDefinitionID(accountID);

            QString qstrAssetTypeId = QString::fromStdString(assetTypeID);
            if (0 == qstrAssetTypeId.compare(qstrUnitTypeId))
            {
                const int64_t lSubtotal = opentxs::OTAPI_Wrap::Exec()->GetAccountWallet_Balance(accountID);
                lTotal += lSubtotal;
            }
        }
        // ----------------------------------------------------
        ++ci;
    }
    return lTotal;
}


void Activity::GetAccountIdMapsByServerId(mapOfMapIDName & bigMap, bool bPairedOrHosted) // true == paired, false == hosted.
{
    int32_t  account_count = opentxs::OTAPI_Wrap::Exec()->GetAccountCount();

    for (int32_t ii = 0; ii < account_count; ii++)
    {
        const QString OT_id   = QString::fromStdString(opentxs::OTAPI_Wrap::Exec()->GetAccountWallet_ID(ii));
        const QString OT_name = QString::fromStdString(opentxs::OTAPI_Wrap::Exec()->GetAccountWallet_Name(OT_id.toStdString()));
        const QString qstrServerId = QString::fromStdString(opentxs::OTAPI_Wrap::Exec()->GetAccountWallet_NotaryID(OT_id.toStdString()));
        const std::string str_server_id = qstrServerId.toStdString();

        // bPaireding means, paired or in the process of pairing.
        const bool bPaireding = opentxs::OTAPI_Wrap::Pair_Started(str_server_id);

        // Basically the caller either wants a list of accounts by server ID for PAIRED servers,
        // OR he wants a list of accounts by server ID for NON-Paired (hosted) servers.
        // So those are the two cases where we add the server to the list.
        if ( (bPairedOrHosted && bPaireding) || (!bPairedOrHosted && !bPaireding))
        {
            mapIDName & mapServerId = GetOrCreateAccountIdMapByServerId(qstrServerId, bigMap);
            mapServerId.insert(OT_id, OT_name);
        }
    }
}

mapIDName & Activity::GetOrCreateAccountIdMapByServerId(QString qstrServerId, mapOfMapIDName & bigMap)
{
    mapOfMapIDName::iterator i = bigMap.find(qstrServerId);
    while (i != bigMap.end() && i.key() == qstrServerId) {
        return i.value();
    }
    // else create it.
    bigMap[qstrServerId] = mapIDName{};
    i = bigMap.find(qstrServerId);
    return i.value();
}

void Activity::RefreshAccountTree()
{
    ClearAccountTree();
    // ----------------------------------------
    QTreeWidget * pTreeWidgetAccounts = ui->treeWidgetAccounts;
    if (nullptr == pTreeWidgetAccounts) {
        return;
    }
    pTreeWidgetAccounts->blockSignals(true);
    // ----------------------------------------
    QList<QTreeWidgetItem *> items;
    // ------------------------------------
    mapOfMapIDName bigMap;

    this->GetAssetIdMapsByCurrencyCode(bigMap);

    if (bigMap.size() > 0)
    {
        // NOTE: Someday this "Totals" row is where we will someday replace with the USD value of the total wallet.
        QTreeWidgetItem * pUnfilteredItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(tr("Totals")));
        items.append(pUnfilteredItem);
        pUnfilteredItem->setExpanded(true);
        pUnfilteredItem->setData(0, Qt::UserRole, QVariant(0));
        pUnfilteredItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
        pUnfilteredItem->setTextColor(0, Qt::white);
        pUnfilteredItem->setTextColor(1, Qt::white);
        pUnfilteredItem->setBackgroundColor(0, Qt::gray);
        pUnfilteredItem->setBackgroundColor(1, Qt::gray);
        // ----------------------------------------
        // Get the total balance for each list of unit types.
        // (That is, the total balance for each currency code).
        mapOfMapIDName::const_iterator ci = bigMap.begin();

        while (ci != bigMap.end()) {
            const QString qstrCurrencyCode = ci.key();
            const mapIDName & mapTLA = ci.value();
            const QString qstrFirstAssetTypeId = mapTLA.begin().key();
            const std::string first_unit_type  = qstrFirstAssetTypeId.toStdString();
            const int64_t nBalanceTotal = GetAccountBalancesTotaledForUnitTypes(mapTLA);
            const std::string str_formatted_amount = opentxs::OTAPI_Wrap::Exec()->FormatAmountWithoutSymbol(first_unit_type, nBalanceTotal);
            const QString qstrFormattedAmount = QString::fromStdString(str_formatted_amount);

            QTreeWidgetItem * pCurrencyCodeWidgetItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(qstrCurrencyCode) << qstrFormattedAmount);
            items.append(pCurrencyCodeWidgetItem);
            pCurrencyCodeWidgetItem->setExpanded(false);
            pCurrencyCodeWidgetItem->setData(0, Qt::UserRole, QVariant(0));
            pCurrencyCodeWidgetItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

            ++ci;
        }


//        QTreeWidgetItem * pDefaultCurrency = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList("USD") << "3,487.32");
//        items.append(pDefaultCurrency);
//        pDefaultCurrency->setExpanded(false);
//        pDefaultCurrency->setData(0, Qt::UserRole, QVariant(0));
//        pDefaultCurrency->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
    }

    // ----------------------------------------
    mapOfMapIDName bigMapPairedAccounts;

    const uint64_t paired_node_count = opentxs::OTAPI_Wrap::Paired_Node_Count();

    if (paired_node_count > 0)
    {
        this->GetAccountIdMapsByServerId(bigMapPairedAccounts, true); // True means, only give me paired servers.
        // ----------------------------------------
        QTreeWidgetItem * pYourNodeItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(tr("Your Stash Node")));
        items.append(pYourNodeItem);
        pYourNodeItem->setExpanded(true);
        pYourNodeItem->setData(0, Qt::UserRole, QVariant(0));
        pYourNodeItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
        pYourNodeItem->setTextColor(0, Qt::white);
        pYourNodeItem->setTextColor(1, Qt::white);
        pYourNodeItem->setBackgroundColor(0, Qt::gray);
        pYourNodeItem->setBackgroundColor(1, Qt::gray);
        pYourNodeItem->setFlags(pYourNodeItem->flags()&~Qt::ItemIsSelectable);
    }

    for (int index = 0; index < paired_node_count; index++) // FOR EACH PAIRED OR PAIRING NODE.
    {
        QString qstrSnpName = tr("Pairing Stash Node...");
        QString qstrServerId;
        const std::string str_index = QString::number(index).toStdString();

        // bPaireding means, paired or in the process of pairing.
        const bool bPaireding = opentxs::OTAPI_Wrap::Pair_Started(str_index);
        bool bConnected = false;
        // ------------------------------------------------------------------------
        if (bPaireding)  // If paired or pairing.
        {
            const std::string str_snp_server_id = opentxs::OTAPI_Wrap::Paired_Server(str_index);
            const bool bGotNotaryId = str_snp_server_id.size() > 0;

            if (bGotNotaryId)
            {
                qstrServerId = QString::fromStdString(str_snp_server_id);

                bConnected = opentxs::OTAPI_Wrap::Exec()->CheckConnection(str_snp_server_id);
                QString strConnected = QString(bConnected ? "Connected" : "Not connected");

                //String strLog = "SNP Notary ID: " + str_snp_server_id + " - " + strConnected;
                //Log.d("MService", strLog);

                QString strTemp = QString::fromStdString(opentxs::OTAPI_Wrap::Exec()->GetServer_Name(str_snp_server_id));
                if (strTemp.size() > 0)
                {
                    qstrSnpName = strTemp;
                }
                else
                {
                    qDebug() << "Failed to get notary name for this notary ID: " << QString::fromStdString(str_snp_server_id);
                }
            }
            else
            {
                qDebug() << "Failed to get notary ID for this index: " << QString::fromStdString(str_index);
            }
        }
        else
        {
            qDebug() << "This index is NOT paired or pairing yet: str_index: " << QString::fromStdString(str_index);
        }
        // ------------------------------------------------------------------------
        if (qstrServerId.isEmpty())
        {
            QTreeWidgetItem * pNodeItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(qstrSnpName));
            items.append(pNodeItem);
            pNodeItem->setExpanded(false);
            const QString qstrIconPath(bConnected ? ":/icons/icons/green_dot.png" : ":/icons/icons/red_dot.png");
            pNodeItem->setIcon(0, QIcon(qstrIconPath));
            pNodeItem->setData(0, Qt::UserRole, QVariant(0));
            pNodeItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
        }
        else // We know for this paired node, there's a server Id.
        {
            // So let's see if there are any accounts for that server Id.
            mapOfMapIDName::const_iterator ci_paired = bigMapPairedAccounts.find(qstrServerId);

            if (ci_paired != bigMapPairedAccounts.end() && (0 == qstrServerId.compare(ci_paired.key()))) // Found some.
            {
                const mapIDName & mapAccounts = ci_paired.value();
                const std::string str_server_id = qstrServerId.toStdString();
                qstrSnpName = QString::fromStdString(opentxs::OTAPI_Wrap::Exec()->GetServer_Name(str_server_id));

                if (1 == mapAccounts.size())
                {
                    const QString qstrAccountId = mapAccounts.begin().key();
                    const std::string str_acct_id = qstrAccountId.toStdString();
                    const std::string str_asset_id = opentxs::OTAPI_Wrap::Exec()->GetAccountWallet_InstrumentDefinitionID(str_acct_id);
                    const int64_t lBalance = opentxs::OTAPI_Wrap::Exec()->GetAccountWallet_Balance(str_acct_id);
                    const std::string str_formatted_amount = opentxs::OTAPI_Wrap::Exec()->FormatAmount(str_asset_id, lBalance);
                    const QString qstrFormattedAmount = QString::fromStdString(str_formatted_amount);
                    // ------------------
                    QTreeWidgetItem * pPairedItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(qstrSnpName) << qstrFormattedAmount);
                    items.append(pPairedItem);
                    pPairedItem->setExpanded(false);

                    const QString qstrIconPath = QString(bConnected ? ":/icons/icons/green_dot.png" : ":/icons/icons/red_dot.png");
                    pPairedItem->setIcon(0, QIcon(qstrIconPath));
                    if (bConnected)
                    {
    //                    pPairedItem->setTextColor(0, Qt::black);
    //                    pPairedItem->setTextColor(1, Qt::black);
                    }
                    else
                    {
                        pPairedItem->setTextColor(0, Qt::gray);
                        pPairedItem->setTextColor(1, Qt::gray);
                    }
                    pPairedItem->setData(0, Qt::UserRole, QVariant(0));
                    pPairedItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
                }
                else // Multiple accounts exist for this server ID.
                {
                    QTreeWidgetItem * pPairedItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(qstrSnpName));
                    items.append(pPairedItem);
                    pPairedItem->setExpanded(true);

                    const QString qstrIconPath = QString(bConnected ? ":/icons/icons/green_dot.png" : ":/icons/icons/red_dot.png");
                    pPairedItem->setIcon(0, QIcon(qstrIconPath));
                    if (bConnected)
                    {
    //                    pPairedItem->setTextColor(0, Qt::black);
    //                    pPairedItem->setTextColor(1, Qt::black);
                    }
                    else
                    {
                        pPairedItem->setTextColor(0, Qt::gray);
                        pPairedItem->setTextColor(1, Qt::gray);
                    }
                    pPairedItem->setData(0, Qt::UserRole, QVariant(0));
                    pPairedItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

                    // -------
                    mapIDName::const_iterator ci_accounts = mapAccounts.begin();

                    while (ci_accounts != mapAccounts.end())
                    {
                        const QString qstrAccountId = ci_accounts.key();
                        const std::string str_acct_id = qstrAccountId.toStdString();
                        const std::string str_asset_id = opentxs::OTAPI_Wrap::Exec()->GetAccountWallet_InstrumentDefinitionID(str_acct_id);
                        const int64_t lBalance = opentxs::OTAPI_Wrap::Exec()->GetAccountWallet_Balance(str_acct_id);
                        const std::string str_formatted_amount = opentxs::OTAPI_Wrap::Exec()->FormatAmount(str_asset_id, lBalance);
                        const QString qstrFormattedAmount = QString::fromStdString(str_formatted_amount);
                        const QString qstrAcctName = QString("     %1").arg(QString::fromStdString(opentxs::OTAPI_Wrap::Exec()->GetAccountWallet_Name(str_acct_id)));
                        // ------------------
                        QTreeWidgetItem * pSubItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(qstrAcctName) << qstrFormattedAmount);
                        items.append(pSubItem);
                        pSubItem->setExpanded(false);
                        //pSubItem->setIcon(0, QIcon(":/icons/icons/red_dot.png"));
                        if (bConnected)
                        {
        //                    pSubItem->setTextColor(0, Qt::black);
        //                    pSubItem->setTextColor(1, Qt::black);
                        }
                        else
                        {
                            pSubItem->setTextColor(0, Qt::gray);
                            pSubItem->setTextColor(1, Qt::gray);
                        }
                        pSubItem->setData(0, Qt::UserRole, QVariant(0));
                        pSubItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

                        ++ci_accounts;
                    }
                }
            }
            else // found none
            {
                QTreeWidgetItem * pNodeItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(qstrSnpName));
                items.append(pNodeItem);
                pNodeItem->setExpanded(false);
                const QString qstrIconPath(bConnected ? ":/icons/icons/green_dot.png" : ":/icons/icons/red_dot.png");
                pNodeItem->setIcon(0, QIcon(qstrIconPath));
                pNodeItem->setData(0, Qt::UserRole, QVariant(0));
                pNodeItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
            }
        }
    }
    // ------------------------------------------------------------------------

    mapOfMapIDName bigMapAccounts;

    this->GetAccountIdMapsByServerId(bigMapAccounts, false); // false means, only give me non-paired servers.

    if (bigMapAccounts.size() > 0)
    {
        // ----------------------------------------
        QTreeWidgetItem * pHostedAccountsItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(tr("Hosted Accounts")));
        items.append(pHostedAccountsItem);
        pHostedAccountsItem->setExpanded(true);
        pHostedAccountsItem->setData(0, Qt::UserRole, QVariant(0));
        pHostedAccountsItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
        pHostedAccountsItem->setTextColor(0, Qt::white);
        pHostedAccountsItem->setTextColor(1, Qt::white);
        pHostedAccountsItem->setBackgroundColor(0, Qt::gray);
        pHostedAccountsItem->setBackgroundColor(1, Qt::gray);
        pHostedAccountsItem->setFlags(pHostedAccountsItem->flags()&~Qt::ItemIsSelectable);
        // ----------------------------------------
        // From Vaage:
        // Retrieve a list of hosted accounts for each server.
        // For each server, if the list is empty, skip it.
        // If the list only contains one account, display
        // the balance directly on the same line as the server name.
        // Only give it a separate line if the user has gone
        // out of his way to name the account.
        // If the user has named it, or if there are multiple accounts
        // for that server, then display the balances below that server
        // on the tree.

        mapOfMapIDName::const_iterator ci_servers = bigMapAccounts.begin();

        while (ci_servers != bigMapAccounts.end()) {
            const QString qstrServerId = ci_servers.key();
            const mapIDName & mapAccounts = ci_servers.value();
            const std::string str_server_id = qstrServerId.toStdString();
            const bool bIsConnectionAlive = opentxs::OTAPI_Wrap::Exec()->CheckConnection(str_server_id);

            QString qstrServerName = QString::fromStdString(opentxs::OTAPI_Wrap::Exec()->GetServer_Name(str_server_id));

            if (1 == mapAccounts.size())
            {
                const QString qstrAccountId = mapAccounts.begin().key();
                const std::string str_acct_id = qstrAccountId.toStdString();
                const std::string str_asset_id = opentxs::OTAPI_Wrap::Exec()->GetAccountWallet_InstrumentDefinitionID(str_acct_id);
                const int64_t lBalance = opentxs::OTAPI_Wrap::Exec()->GetAccountWallet_Balance(str_acct_id);
                const std::string str_formatted_amount = opentxs::OTAPI_Wrap::Exec()->FormatAmount(str_asset_id, lBalance);
                const QString qstrFormattedAmount = QString::fromStdString(str_formatted_amount);
                // ------------------
                QTreeWidgetItem * pHostedItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(qstrServerName) << qstrFormattedAmount);
                items.append(pHostedItem);
                pHostedItem->setExpanded(false);

                const QString qstrIconPath = QString(bIsConnectionAlive ? ":/icons/icons/green_dot.png" : ":/icons/icons/red_dot.png");
                pHostedItem->setIcon(0, QIcon(qstrIconPath));
                if (bIsConnectionAlive)
                {
//                    pHostedItem->setTextColor(0, Qt::black);
//                    pHostedItem->setTextColor(1, Qt::black);
                }
                else
                {
                    pHostedItem->setTextColor(0, Qt::gray);
                    pHostedItem->setTextColor(1, Qt::gray);
                }
                pHostedItem->setData(0, Qt::UserRole, QVariant(0));
                pHostedItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
            }
            else // Multiple accounts exist for this server ID.
            {
                QTreeWidgetItem * pHostedItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(qstrServerName));
                items.append(pHostedItem);
                pHostedItem->setExpanded(true);

                const QString qstrIconPath = QString(bIsConnectionAlive ? ":/icons/icons/green_dot.png" : ":/icons/icons/red_dot.png");
                pHostedItem->setIcon(0, QIcon(qstrIconPath));
                if (bIsConnectionAlive)
                {
//                    pHostedItem->setTextColor(0, Qt::black);
//                    pHostedItem->setTextColor(1, Qt::black);
                }
                else
                {
                    pHostedItem->setTextColor(0, Qt::gray);
                    pHostedItem->setTextColor(1, Qt::gray);
                }
                pHostedItem->setData(0, Qt::UserRole, QVariant(0));
                pHostedItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

                // -------
                mapIDName::const_iterator ci_accounts = mapAccounts.begin();

                while (ci_accounts != mapAccounts.end())
                {
                    const QString qstrAccountId = ci_accounts.key();
                    const std::string str_acct_id = qstrAccountId.toStdString();
                    const std::string str_asset_id = opentxs::OTAPI_Wrap::Exec()->GetAccountWallet_InstrumentDefinitionID(str_acct_id);
                    const int64_t lBalance = opentxs::OTAPI_Wrap::Exec()->GetAccountWallet_Balance(str_acct_id);
                    const std::string str_formatted_amount = opentxs::OTAPI_Wrap::Exec()->FormatAmount(str_asset_id, lBalance);
                    const QString qstrFormattedAmount = QString::fromStdString(str_formatted_amount);
                    const QString qstrAcctName = QString("     %1").arg(QString::fromStdString(opentxs::OTAPI_Wrap::Exec()->GetAccountWallet_Name(str_acct_id)));
                    // ------------------

                    QTreeWidgetItem * pSubItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(qstrAcctName) << qstrFormattedAmount);
                    items.append(pSubItem);
                    pSubItem->setExpanded(false);
                    //pSubItem->setIcon(0, QIcon(":/icons/icons/red_dot.png"));
                    if (bIsConnectionAlive)
                    {
    //                    pSubItem->setTextColor(0, Qt::black);
    //                    pSubItem->setTextColor(1, Qt::black);
                    }
                    else
                    {
                        pSubItem->setTextColor(0, Qt::gray);
                        pSubItem->setTextColor(1, Qt::gray);
                    }
                    pSubItem->setData(0, Qt::UserRole, QVariant(0));
                    pSubItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

                    ++ci_accounts;
                }
            }

            ++ci_servers;
        }



    }
    // ----------------------------------------

//    {
//    QTreeWidgetItem * pHostedItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList("Ruritarian Reserve") << "BTC 1.686 46731");
//    items.append(pHostedItem);
//    pHostedItem->setExpanded(false);
//    pHostedItem->setIcon(0, QIcon(":/icons/icons/red_dot.png"));
//    pHostedItem->setTextColor(0, Qt::gray);
//    pHostedItem->setTextColor(1, Qt::gray);
//    pHostedItem->setData(0, Qt::UserRole, QVariant(0));
//    pHostedItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
//    }
//    {
//    QTreeWidgetItem * pHostedItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList("Nakamoto Market") << "BTC 0.000 00000");
//    items.append(pHostedItem);
//    pHostedItem->setExpanded(false);
//    pHostedItem->setIcon(0, QIcon(":/icons/icons/green_dot.png"));
//    pHostedItem->setData(0, Qt::UserRole, QVariant(0));
//    pHostedItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
//    }


    // ----------------------------------------
    QTreeWidgetItem * pContactsItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(tr("Contacts")));
    items.append(pContactsItem);
    pContactsItem->setExpanded(true);
    pContactsItem->setData(0, Qt::UserRole, QVariant(0));
    pContactsItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
    pContactsItem->setTextColor(0, Qt::white);
    pContactsItem->setTextColor(1, Qt::white);
    pContactsItem->setBackgroundColor(0, Qt::gray);
    pContactsItem->setBackgroundColor(1, Qt::gray);
    pContactsItem->setFlags(pContactsItem->flags()&~Qt::ItemIsSelectable);
    // ----------------------------------------





    mapIDName mapContacts;

    if (MTContactHandler::getInstance()->GetContacts(mapContacts))
    {
        for (mapIDName::iterator ii = mapContacts.begin(); ii != mapContacts.end(); ii++)
        {
            QString qstrContactID   = ii.key();
            QString qstrContactName = ii.value();
            int     nContactID      = qstrContactID.toInt();
            // ------------------------------------
            QTreeWidgetItem * pTopItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(qstrContactName));
            pTopItem->setData(0, Qt::UserRole, QVariant(nContactID));
            pTopItem->setExpanded(true);
            //pTopItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
            items.append(pTopItem);
            // ------------------------------------
            mapIDName mapMethodTypes;
            // So we can look up these names quickly without having to repeatedly hit the database
            // for the same names over and over again.
            MTContactHandler::getInstance()->GetMsgMethodTypes(mapMethodTypes);
            // ------------------------------------
            mapIDName mapTransport;

            if (MTContactHandler::getInstance()->GetMsgMethodTypesByContact(mapTransport, nContactID, true)) // True means to add the OT servers as well.
            {
                for (mapIDName::iterator it_transport = mapTransport.begin(); it_transport != mapTransport.end(); it_transport++)
                {
                    QString qstrID   = it_transport.key();
//                  QString qstrName = it_transport.value();

                    QStringList stringlist = qstrID.split("|");

                    if (stringlist.size() >= 2) // Should always be 2...
                    {
                        QString qstrViaTransport = stringlist.at(1);
                        QString qstrTransportName = qstrViaTransport;
                        QString qstrMethodType = stringlist.at(0);
                        QString qstrMethodName = qstrMethodType;

                        mapIDName::iterator it_mapMethodTypes = mapMethodTypes.find(qstrMethodType);

                        if (mapMethodTypes.end() != it_mapMethodTypes)
                        {
                            QString qstrTemp = it_mapMethodTypes.value();
                            if (!qstrTemp.isEmpty())
                                qstrMethodName = qstrTemp;
                        }
                        // ------------------------------------------------------
                        else if (0 == QString("otserver").compare(qstrMethodType))
                        {
                            qstrMethodName = tr("Notary");
                            // ------------------------------
                            QString qstrTemp = QString::fromStdString(opentxs::OTAPI_Wrap::Exec()->GetServer_Name(qstrViaTransport.toStdString()));
                            if (!qstrTemp.isEmpty())
                                qstrTransportName = qstrTemp;
                        }
                        // ------------------------------------------------------
                        QTreeWidgetItem * pAddressItem = new QTreeWidgetItem(pTopItem, QStringList(qstrContactName) << qstrMethodName << qstrTransportName);
                        pAddressItem->setData(0, Qt::UserRole, QVariant(nContactID));
                        pAddressItem->setData(1, Qt::UserRole, QVariant(qstrMethodType));
                        pAddressItem->setData(2, Qt::UserRole, QVariant(qstrViaTransport));
                        //pAddressItem->setExpanded(true);
                        //pAddressItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
                        items.append(pAddressItem);
                    }
                }
            }
        }
        if (items.count() > 0)
        {
            pTreeWidgetAccounts->insertTopLevelItems(0, items);
            pTreeWidgetAccounts->resizeColumnToContents(0);
        }
    }
    // ----------------------------------------
    // Make sure the same item that was selected before, is selected again.
    // (If it still exists, which it probably does.)

    QTreeWidgetItem * previous = pTreeWidgetAccounts->currentItem();

    // In this case, just select the first thing on the list.
    if ( (0 == nCurrentContact_) && qstrMethodType_.isEmpty() && qstrViaTransport_.isEmpty() )
    {
        if (pTreeWidgetAccounts->topLevelItemCount() > 0)
            pTreeWidgetAccounts->setCurrentItem(pTreeWidgetAccounts->topLevelItem(0));
    }
    else // Find the one that was selected before the refresh.
    {
        bool bFoundIt = false;

        QTreeWidgetItemIterator it(pTreeWidgetAccounts);

        while (*it)
        {
            QVariant qvarContactID    = (*it)->data(0, Qt::UserRole);
            QVariant qvarMethodType   = (*it)->data(1, Qt::UserRole);
            QVariant qvarViaTransport = (*it)->data(2, Qt::UserRole);

            const int     nContactID       = qvarContactID   .isValid() ? qvarContactID   .toInt()    :  0;
            const QString qstrMethodType   = qvarMethodType  .isValid() ? qvarMethodType  .toString() : "";
            const QString qstrViaTransport = qvarViaTransport.isValid() ? qvarViaTransport.toString() : "";

            if ( (nContactID == nCurrentContact_ ) &&
                 (0 == qstrMethodType.compare(qstrMethodType_)) &&
                 (0 == qstrViaTransport.compare(qstrViaTransport_)) )
            {
                bFoundIt = true;
                pTreeWidgetAccounts->setCurrentItem(*it);
                break;
            }
            // ------------------------
            ++it;
        } //while
        if (!bFoundIt)
        {
            if (pTreeWidgetAccounts->topLevelItemCount() > 0)
                pTreeWidgetAccounts->setCurrentItem(pTreeWidgetAccounts->topLevelItem(0));
        }
    }

//    pTreeWidgetAccounts->expandAll();
    // ----------------------------------------
    pTreeWidgetAccounts->blockSignals(false);
    // ----------------------------------------
    // I will uncomment this once I copy this function over from payments.cpp
//    on_treeWidget_currentItemChanged(pTreeWidgetAccounts->currentItem(), previous);
}

//void Activity::RefreshAccountList()
//{
//    QTreeWidget * pTreeWidgetAccounts = ui->treeWidgetAccounts;
//    if (nullptr == pTreeWidgetAccounts) {
//        return;
//    }
//    pTreeWidgetAccounts->blockSignals(true);
//    // ----------------------------------------------
//    // Clear it
//    pTreeWidgetAccounts->clear();
//    // ----------------------------------------------
//    // Re-populate it

//    int nIndex = 0;
//    QListWidgetItem * pItem = nullptr;

//    pTreeWidgetAccounts->addItem("Bitcoin");
//    pItem = pTreeWidgetAccounts->item(nIndex);
//    nIndex++;
//    pItem->setTextColor(Qt::white);
//    pItem->setBackgroundColor(Qt::gray);
//    // ----------------------------------------------
//    /*Loop through Blockchain Types for the totals*/ {
//        pTreeWidgetAccounts->addItem("BTC                        " + QString("5.038 47164"));
//        pItem = pTreeWidgetAccounts->item(nIndex);
//        nIndex++;
//        pItem->setTextColor(Qt::white);
//        pItem->setBackgroundColor(Qt::gray);
//        // ----------------------------------------------
//        pTreeWidgetAccounts->addItem("USD                        " + QString("$3,275.01"));
//        pItem = pTreeWidgetAccounts->item(nIndex);
//        nIndex++;
//        pItem->setTextColor(Qt::darkGray);
//        pItem->setBackgroundColor(Qt::gray);
//    }
//    // -----------------------------
//    pTreeWidgetAccounts->addItem("*GRN* Mac Desktop       " + QString("0.000 00000"));
//    pItem = pTreeWidgetAccounts->item(nIndex);
//    nIndex++;
//    pItem->setTextColor(Qt::white);
//    pItem->setBackgroundColor(Qt::darkGray);
//    // -----------------------------
//    pTreeWidgetAccounts->addItem("Your Stash Node");
//    pItem = pTreeWidgetAccounts->item(nIndex);
//    nIndex++;
//    pItem->setTextColor(Qt::lightGray);
//    pItem->setBackgroundColor(Qt::gray);
//    // -----------------------------
//    /*Loop through all the paired nodes*/ {
//        pTreeWidgetAccounts->addItem("*GRN* John Galt ");
//        pItem = pTreeWidgetAccounts->item(nIndex);
//        nIndex++;
//        pItem->setTextColor(Qt::white);
//        pItem->setBackgroundColor(Qt::darkGray);
//        // -----------------------------
//        pTreeWidgetAccounts->addItem("      Primary         " + QString("0.493 87640"));
//        pItem = pTreeWidgetAccounts->item(nIndex);
//        nIndex++;
//        pItem->setTextColor(Qt::lightGray);
//        pItem->setBackgroundColor(Qt::darkGray);
//        // -----------------------------
//        pTreeWidgetAccounts->addItem("      Savings         " + QString("2.730 67371"));
//        pItem = pTreeWidgetAccounts->item(nIndex);
//        nIndex++;
//        pItem->setTextColor(Qt::lightGray);
//        pItem->setBackgroundColor(Qt::darkGray);
//    }
//    // -----------------------------
//    pTreeWidgetAccounts->addItem("Hosted Accounts");
//    pItem = pTreeWidgetAccounts->item(nIndex);
//    nIndex++;
//    pItem->setTextColor(Qt::lightGray);
//    pItem->setBackgroundColor(Qt::gray);
//    // -----------------------------
//    /*Loop through all the hosted accounts*/ {
//        pTreeWidgetAccounts->addItem("*RED* Ruritarian Reserve   " + QString("1.686 46731"));
//        pItem = pTreeWidgetAccounts->item(nIndex);
//        nIndex++;
//        pItem->setTextColor(Qt::gray);
//        pItem->setBackgroundColor(Qt::darkGray);
//        // -----------------------------
//        pTreeWidgetAccounts->addItem("*GRN* John Galt                " + QString("0.000 00000"));
//        pItem = pTreeWidgetAccounts->item(nIndex);
//        nIndex++;
//        pItem->setTextColor(Qt::white);
//        pItem->setBackgroundColor(Qt::darkGray);
//    }
//    // -----------------------------
////    for (int i = 0; i < 10; i++)
////    {
////        if (0 == i) {
////            pTreeWidgetAccounts->addItem("MAIN BALANCE: " + QString::number(i));
////        }
////        else
////            pTreeWidgetAccounts->addItem("      Item " + QString::number(i));
////    }
//    // ----------------------------------------------
//    pTreeWidgetAccounts->blockSignals(false);
//}


void Activity::on_listWidgetConversations_currentRowChanged(int currentRow)
{
    if ((-1) == currentRow)
        ui->listWidgetConversation->setEnabled(false);
    else
        ui->listWidgetConversation->setEnabled(true);
    // -------------------
    RefreshConversationDetails(currentRow);
}
//        message StorageThread {
//            optional uint32 version = 1;
//            optional string id = 2;
//            repeated string participant = 3;
//            repeated StorageThreadItem item = 4;
//        }
//        enum class StorageBox : std::uint8_t {
//            SENTPEERREQUEST = 0,
//            INCOMINGPEERREQUEST = 1,
//            SENTPEERREPLY = 2,
//            INCOMINGPEERREPLY = 3,
//            FINISHEDPEERREQUEST = 4,
//            FINISHEDPEERREPLY = 5,
//            PROCESSEDPEERREQUEST = 6,
//            PROCESSEDPEERREPLY = 7,
//            MAILINBOX = 8,
//            MAILOUTBOX = 9
//        };
void Activity::RefreshConversationDetails(int nRow)
{
    if (ui->listWidgetConversations->currentRow() <  0)
    {
        ui->listWidgetConversation->clear();
    }
    else
    {
        ui->listWidgetConversation->clear();

        QListWidgetItem * conversation_widget = ui->listWidgetConversations->currentItem();

        if (nullptr == conversation_widget) {
            return;
        }

        const QVariant qvarMyNymId = conversation_widget->data(Qt::UserRole+1);
        const QVariant qvarThreadId = conversation_widget->data(Qt::UserRole+2);

        const QString  qstrMyNymId = qvarMyNymId.isValid() ? qvarMyNymId.toString() : QString("");
        const QString  qstrThreadId = qvarThreadId.isValid() ? qvarThreadId.toString() : QString("");

        if (!qstrMyNymId.isEmpty() && !qstrThreadId.isEmpty())
        {
            const std::string str_my_nym_id = qstrMyNymId.toStdString();
            const std::string str_thread_id = qstrThreadId.toStdString();
            // ----------------------------------------------
            std::shared_ptr<opentxs::proto::StorageThread> thread;
            opentxs::OT::App().DB().Load(str_my_nym_id, str_thread_id, thread);
            // ----------------------------------------------
            if (!thread) {
                return;
            }
            // ----------------------------------------------
            const int thread_size = thread->item_size();

            int nConversationIndex = 0;

            for(const auto & item : thread->item())
//          for (int nIndex = 0; nIndex < thread_size; nIndex++)
            {
//              const opentxs::proto::StorageThreadItem & item = thread->item(nIndex);
                bool bIncoming = false;

                if (opentxs::StorageBox::MAILINBOX == static_cast<opentxs::StorageBox>(item.box())) {
                    bIncoming = true;
                }

                //        message StorageThreadItem {
                //            optional uint32 version = 1;
                //            optional string id = 2;
                //            optional uint64 index = 3;
                //            optional uint64 time = 4;
                //            optional uint32 box = 5;
                //            optional string account = 6;
                //            optional bool unread = 7;
                //        }

                const std::string item_id = item.id();
//              const uint64_t item_index = item.index();
//              const uint64_t item_time = item.time();
//              const std::string str_item_account = item.account();
                const std::string str_item_contents = bIncoming ?
                            opentxs::OTAPI_Wrap::GetNym_MailContentsByIndex   (str_my_nym_id, item_id) :
                            opentxs::OTAPI_Wrap::GetNym_OutmailContentsByIndex(str_my_nym_id, item_id);
                // ----------------------------------------------
                QString qstrContents = QString::fromStdString(str_item_contents);

                // Maybe we already imported it?
                if (qstrContents.isEmpty())
                {
                    const QString qstrItemId = QString::fromStdString(item_id);
                    qstrContents = MTContactHandler::getInstance()->GetMessageBody(qstrItemId);
                }
                // ----------------------------------------------
                QString qstrSubject{"subject:"};
                const bool bHasContents = !qstrContents.isEmpty();
                const bool bHasSubject  = bHasContents && qstrContents.startsWith(qstrSubject, Qt::CaseInsensitive);

                if (bHasContents)// && !bHasSubject) // todo put this back.
                {
                    QListWidgetItem * pItem = nullptr;

                    ui->listWidgetConversation->addItem(qstrContents);
                    pItem = ui->listWidgetConversation->item(nConversationIndex);

                    if (bIncoming)
                    {
                        pItem->setTextColor(Qt::white);
                        pItem->setBackgroundColor(Qt::gray);
                    }
                    else
                    {
                        pItem->setTextColor(Qt::black);
                        pItem->setBackgroundColor(Qt::white);
                    }
                    nConversationIndex++;
                }
                // ----------------------------------------------
            }
            // ----------------------------------------------
        }
    }
}

void Activity::ClearListWidgetConversations()
{
    ui->listWidgetConversations->blockSignals(true);
    // -----------------------------------
    ui->listWidgetConversations->clear();
    // -----------------------------------
    ui->listWidgetConversations->setCurrentRow(-1, 0);
    ui->listWidgetConversations->blockSignals(false);

    on_listWidgetConversations_currentRowChanged(-1);
}

void Activity::RefreshConversationsTab()
{
    QListWidget * pListWidgetConversations = ui->listWidgetConversations;
    if (nullptr == pListWidgetConversations) {
        return;
    }
    pListWidgetConversations->blockSignals(true);
    // ----------------------------------------------
    // Clear it
    pListWidgetConversations->clear();
    // ----------------------------------------------
    // Re-populate it

    int nIndex = 0;
    QListWidgetItem * pItem = nullptr;

//    pListWidgetConversations->addItem("Cliff");
//    pItem = pListWidgetConversations->item(nIndex);
//    nIndex++;
//    pItem->setTextColor(Qt::white);
//    pItem->setBackgroundColor(Qt::gray);
//    // ----------------------------------------------
//    pListWidgetConversations->addItem("Justus");
//    pItem = pListWidgetConversations->item(nIndex);
//    nIndex++;
//    pItem->setTextColor(Qt::white);
//    pItem->setBackgroundColor(Qt::darkGray);
//    // ----------------------------------------------
//    pListWidgetConversations->addItem("Jimmy");
//    pItem = pListWidgetConversations->item(nIndex);
//    nIndex++;
//    pItem->setTextColor(Qt::white);
//    pItem->setBackgroundColor(Qt::gray);
//    // ----------------------------------------------
//    pListWidgetConversations->addItem("Hiro");
//    pItem = pListWidgetConversations->item(nIndex);
//    nIndex++;
//    pItem->setTextColor(Qt::white);
//    pItem->setBackgroundColor(Qt::darkGray);
//    // ----------------------------------------------
//    pListWidgetConversations->addItem("Daniel");
//    pItem = pListWidgetConversations->item(nIndex);
//    nIndex++;
//    pItem->setTextColor(Qt::white);
//    pItem->setBackgroundColor(Qt::gray);
//    // ----------------------------------------------
//    pListWidgetConversations->addItem("Hiro and Cliff");
//    pItem = pListWidgetConversations->item(nIndex);
//    nIndex++;
//    pItem->setTextColor(Qt::white);
//    pItem->setBackgroundColor(Qt::darkGray);
    // ----------------------------------------------
    // Conversational threads.
    //
    const std::string str_nym_id = Moneychanger::It()->get_default_nym_id().toStdString();

    if (str_nym_id.empty())
    {
        qDebug() << "Activity dialog, populate Conversations: Failure (default Nym was empty)." << endl;
        return;
    }
    // ----------------------------------------------
    opentxs::ObjectList threadList = opentxs::OT::App().Contract().Threads(opentxs::Identifier{str_nym_id});
    opentxs::ObjectList::const_iterator ci = threadList.begin();

    while (threadList.end() != ci)
    {
        const std::pair<std::string, std::string> & threadInfo = *ci;

        const std::string & str_thread_id = threadInfo.first;
        //const std::string & str_thread_name = threadInfo.second; // Justus: This field is probably blank.
        // ----------------------------------------------
        std::shared_ptr<opentxs::proto::StorageThread> thread;
        opentxs::OT::App().DB().Load(str_nym_id, str_thread_id, thread);
        // ----------------------------------------------
//        message StorageThread {
//            optional uint32 version = 1;
//            optional string id = 2;
//            repeated string participant = 3;
//            repeated StorageThreadItem item = 4;
//        }
//        message StorageThreadItem {
//            optional uint32 version = 1;
//            optional string id = 2;
//            optional uint64 index = 3;
//            optional uint64 time = 4;
//            optional uint32 box = 5;
//            optional string account = 6;
//            optional bool unread = 7;
//        }
//        enum class StorageBox : std::uint8_t {
//            SENTPEERREQUEST = 0,
//            INCOMINGPEERREQUEST = 1,
//            SENTPEERREPLY = 2,
//            INCOMINGPEERREPLY = 3,
//            FINISHEDPEERREQUEST = 4,
//            FINISHEDPEERREPLY = 5,
//            PROCESSEDPEERREQUEST = 6,
//            PROCESSEDPEERREPLY = 7,
//            MAILINBOX = 8,
//            MAILOUTBOX = 9
//        };

        if (!thread) {
            ++ci;
            continue;
        }
        // ----------------------------------------------
        if (thread->participant_size() <= 0) {
            ++ci;
            continue;
        }
        // ----------------------------------------------
        const std::string str_participant_contact_id = thread->participant(0); // Todo: currently hardcoding participant at index 0 in assumption there's only one other participant in the conversation.

        if (!str_participant_contact_id.empty())
        {
            QString qstrDisplayName = QString("Conv. w/contact id: %1").arg(QString::fromStdString(str_participant_contact_id)); // Todo lookup the actual contact display name.

            pListWidgetConversations->addItem(qstrDisplayName);
            pItem = pListWidgetConversations->item(nIndex);
            nIndex++;
            pItem->setTextColor(Qt::white);
            const QString qstrThreadId = QString::fromStdString(str_thread_id);
            const QString qstrMyNymId = QString::fromStdString(str_nym_id);
            pItem->setData(Qt::UserRole+1, QVariant(qstrMyNymId));
            pItem->setData(Qt::UserRole+2, QVariant(qstrThreadId));
            pItem->setBackgroundColor((nIndex%2 == 0) ? Qt::gray : Qt::darkGray);
        }
        // ----------------------------------------------
        ++ci;
    }
    // ----------------------------------------------



//resume


//    const std::string GetNym_MailContentsByIndex

    // ----------------------------------------------
    pListWidgetConversations->blockSignals(false);
}


void Activity::setupCurrentPointers()
{

}


void Activity::dialog()
{
    if (!already_init)
    {
        // ----------------------------------------------------------------

        // ******************************************************
        {
        //QPointer<ModelAgreements> pModel = DBHandler::getInstance()->getAgreementModel();

        //if (pModel)
        {
//            pProxyModelRecurring_      = new AgreementsProxyModel;
//            pProxyModelSmartContracts_ = new AgreementsProxyModel;

//            pProxyModelRecurring_     ->setSourceModel(pModel);
//            pProxyModelSmartContracts_->setSourceModel(pModel);

//            pProxyModelRecurring_     ->setFilterFolder(0); // 0 Payment Plan, 1 Smart Contract, 2 Entity
//            pProxyModelSmartContracts_->setFilterFolder(1);
//            // ---------------------------------
//            setup_agreement_tableview(ui->tableViewRecurring,      pProxyModelRecurring_     );
//            setup_agreement_tableview(ui->tableViewSmartContracts, pProxyModelSmartContracts_);
//            // ---------------------------------
//            QItemSelectionModel *sm1 = ui->tableViewRecurring->selectionModel();
//            connect(sm1, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
//                    this, SLOT(on_tableViewRecurringSelectionModel_currentRowChanged(QModelIndex,QModelIndex)));
//            // ---------------------------------
//            QItemSelectionModel *sm2 = ui->tableViewSmartContracts->selectionModel();
//            connect(sm2, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
//                    this, SLOT(on_tableViewSmartContractsSelectionModel_currentRowChanged(QModelIndex,QModelIndex)));
        }
        }
        // --------------------------------------------------------
        {
//        QPointer<ModelAgreementReceipts> pModel = DBHandler::getInstance()->getAgreementReceiptModel();

//        if (pModel)
        {
//            pReceiptProxyModelInbox_  = new AgreementReceiptsProxyModel;
//            pReceiptProxyModelOutbox_ = new AgreementReceiptsProxyModel;

//            pReceiptProxyModelInbox_ ->setSourceModel(pModel);
//            pReceiptProxyModelOutbox_->setSourceModel(pModel);

//            pReceiptProxyModelOutbox_->setFilterSent();
//            pReceiptProxyModelInbox_ ->setFilterReceived();
//            // ---------------------------------
//            setup_receipt_tableview(ui->tableViewSent, pReceiptProxyModelOutbox_);
//            setup_receipt_tableview(ui->tableViewReceived, pReceiptProxyModelInbox_);
//            // ---------------------------------
//            QItemSelectionModel *sm1 = ui->tableViewSent->selectionModel();
//            connect(sm1, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
//                    this, SLOT(on_tableViewSentSelectionModel_currentRowChanged(QModelIndex,QModelIndex)));
//            QItemSelectionModel *sm2 = ui->tableViewReceived->selectionModel();
//            connect(sm2, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
//                    this, SLOT(on_tableViewReceivedSelectionModel_currentRowChanged(QModelIndex,QModelIndex)));
        }
        }
        // --------------------------------------------------------
//        connect(this, SIGNAL(showContact(QString)),               Moneychanger::It(), SLOT(mc_showcontact_slot(QString)));
//        // --------------------------------------------------------
//        connect(this, SIGNAL(showContactAndRefreshHome(QString)), Moneychanger::It(), SLOT(onNeedToPopulateRecordlist()));
//        connect(this, SIGNAL(showContactAndRefreshHome(QString)), Moneychanger::It(), SLOT(mc_showcontact_slot(QString)));
//        // --------------------------------------------------------
//        connect(ui->toolButtonRecurring,      SIGNAL(clicked()),  Moneychanger::It(), SLOT(mc_proposeplan_slot()));
//        connect(ui->toolButtonPending,        SIGNAL(clicked()),  Moneychanger::It(), SLOT(mc_overview_slot()));
//        connect(ui->toolButtonSmartContracts, SIGNAL(clicked()),  Moneychanger::It(), SLOT(mc_smartcontract_slot()));
        // --------------------------------------------------------
//        QWidget* pTab0 = ui->tabWidgetAgreements->widget(0);
//        QWidget* pTab1 = ui->tabWidgetAgreements->widget(1);
//        QWidget* pTab2 = ui->tabWidgetReceipts->widget(0);
//        QWidget* pTab3 = ui->tabWidgetReceipts->widget(1);

//        pTab0->setStyleSheet("QWidget { margin: 0 }");
//        pTab1->setStyleSheet("QWidget { margin: 0 }");
//        pTab2->setStyleSheet("QWidget { margin: 0 }");
//        pTab3->setStyleSheet("QWidget { margin: 0 }");

//        ui->splitter->setStretchFactor(0, 2);
//        ui->splitter->setStretchFactor(1, 3);

//        ui->splitter->setStretchFactor(0, 1);
//        ui->splitter->setStretchFactor(1, 5);
        // ------------------------

        setupCurrentPointers();


        ui->treeWidgetAccounts->header()->close();
        ui->treeWidgetAccounts->setRootIsDecorated(false);

        // resume now
//        ui->tabWidgetAgreements->setCurrentIndex(0);

//        ui->tabWidgetReceipts->setCurrentIndex(0);

//        on_tabWidgetReceipts_currentChanged(0);
    }
    // -------------------------------------------
    RefreshAll();
    // -------------------------------------------
    Focuser f(this);
    f.show();
    f.focus();
    // -------------------------------------------
//    if (nSourceRow >=0 && nFolder >= 0)
//        setAsCurrentAgreement(nSourceRow, nFolder);
//    else if (!already_init)
//        setAsCurrentAgreement(0,0);
    // -------------------------------------------
    if (!already_init)
    {
//        on_tabWidgetAgreements_currentChanged(0);
//        on_tabWidgetReceipts_currentChanged(0);

        /** Flag Already Init **/
        already_init = true;
    }

}


void Activity::on_pushButtonSendMsg_clicked()
{
    const QString qstrPlainText = ui->plainTextEditMsg->toPlainText().simplified();

    if (!qstrPlainText.isEmpty())
    {
//resume
    }

    ui->plainTextEditMsg->setPlainText("");
}

bool Activity::eventFilter(QObject *obj, QEvent *event)
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
    return QWidget::eventFilter(obj, event);
}


Activity::~Activity()
{
    delete ui;
}


