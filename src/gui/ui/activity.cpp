#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/ui/activity.hpp>
#include <ui_activity.h>

#include <gui/widgets/compose.hpp>
#include <gui/ui/dlgexportedtopass.hpp>
#include <gui/ui/dlgexportedcash.hpp>
#include <gui/ui/dlginbailment.hpp>
#include <gui/ui/dlgnewcontact.hpp>
#include <gui/ui/dlgoutbailment.hpp>
#include <gui/ui/getstringdialog.hpp>

#include <gui/widgets/home.hpp>
#include <gui/widgets/dlgchooser.hpp>
#include <gui/widgets/qrwidget.hpp>

#include <core/moneychanger.hpp>
#include <core/handlers/DBHandler.hpp>
#include <core/handlers/contacthandler.hpp>
#include <core/handlers/modelpayments.hpp>
#include <core/handlers/focuser.h>

#include <opentxs/opentxs.hpp>

#include <QLabel>
#include <QDebug>
#include <QVariant>
#include <QToolButton>
#include <QKeyEvent>
#include <QApplication>
#include <QMessageBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMenu>
#include <QList>
#include <QSqlRecord>
#include <QTimer>
#include <QFrame>

#include <string>
#include <map>
#include <tuple>
#include <ctime>

template class opentxs::SharedPimpl<opentxs::ui::IssuerItem>;
template class opentxs::SharedPimpl<opentxs::ui::AccountSummaryItem>;

/*
 * The tree items will have certain values attached to them.
 * I will have to figure out what those are.
 * This is the place in the code where I will put those notes until
 * I have it square.
 * Right now I see:
 *
 * Totals
 *  USD         0.00
 * Hosted Accounts
 *  (green) localhost $0.00
 * Contacts
 *  au (desktop) (local wallet)
 *  Bob (local wallet)
 *  Hirosan3000
 *  Issuer (local wallet)
 *
 * I know there are also Stash Node Pros, if one is paired.
 * I also know that if there are multiple accounts on a single server,
 * then the picture might change. Sometimes it combines them, sometimes it
 * separates them.
 * Next thing I'll do is add a "Bitcoin" currency contract and see how the
 * picture changes...
 *
 * Totals
 *  BTC         0.00000000
 *  USD         0.00
 * Hosted Accounts
 *  (green) localhost
 *    Bobs bitcoin      BTC 0.00000000
 *    Bobs dollars      $0.00
 *    New Issuer Acct   -$10,000,000.00
 *    New Issuer Acct   BTC 0.00000000
 *    Dollars (au)      $10,000,000.00
 * Contacts
 *  au (desktop) (local wallet)
 *  Bob (local wallet)
 *  Hirosan3000
 *  Issuer (local wallet)
 *
 * Okay so I definitely have to be able to filter by ASSET TYPE (if click on one of
 * the totals at the top).
 *
 * I also have to be able to filter by NOTARY (if clicking on "localhost" for example)
 * and in fact clicking on localhost DOES work already! But if I click on the accounts
 * inside the localhost notary, it doesn't seem to filter them yet.
 *
 * So I need to be able to filter by ACCOUNT as well.
 *
 * And currently I can also filter by CONTACT, though I think it's still using the old
 * Moneychanger contacts, not the new opentxs contacts (yet... that's what I'm working
 * on now...)
 * ------------------
 *
 * Let's also contrast this with the left sidebar on the old PAYMENTS dialog. That is an
 * obvious tree (expandable, with triangles) with "All Payments (Unfiltered)" at the
 * top, followed by the (old-style) contacts.
 * Each contact expands to show his Nyms, with a line for each Notary the Nym is known
 * to be registered on. This is all queried from the old Moneychanger contact system.
 *
 * I should also note, there may be a line under a contact that's NOT related to a notary.
 * For example, there may be 2 lines for 2 different Bitmessage addresses associated with
 * that contact.
 * ------------------
 *
 * I say all this because on this new ACTIVITY dialog, we don't have that at all, so anywhere
 * that's referencing "method types" under Contacts, is not needed in this code. In fact, an
 * Opentxs Notary would just be another "method", but with method type "opentxs" instead of
 * "bitmsg". On the payments dialog, it's a branch under the Contact on the tree. Whereas in
 * this new Activity dialog, it's just coincidental that we also have a need to filter based
 * on the Notary.
 * And in this case, it really is PURELY about the Notary, regardless of the asset type or
 * the Nym, or the Contact. Just purely the Notary alone (if that's what the user clicked
 * on).
 * But the user can also click on an account under that Notary -- in which case it's PURELY about
 * the ACCOUNT! That is, the Notary is still nice and all, since it's the notary of the account
 * we're filtering for, but in that case it's just circumstantial.
 * ------------------
 *
 * Another thing we know is that, in addition to the "Hosted Accounts" and the "Stash Node Pro",
 * there is also the "Local Wallets", even though they do not currently exist in Moneychanger.
 * (But they do exist on Android). That's still important to mention since local wallets have
 * the property that their accounts are NOT Opentxs accounts. The account is directly a Bitcoin
 * account in a local Bitcoin wallet on the local device, for example, rather than a Bitcoin
 * wallet on a StashNodePro somewhere on the Internet, being represented by way of an Opentxs balance.
 *
 * In anticipation of this, we cannot assume there to be "opentxs" type for asset types, nor
 * for "Local Wallet" accounts (though we do for "Hosted Accounts"). Nor for Contacts, who
 * may only have a Bitmessage address or Bitcoin address, and not any Opentxs accounts at all!
 * This means, in my mind, an Opentxs Notary filter must be kept blank, EXCEPT when the user has
 * directly clicked on an Opentxs Notary!
 * ------------------
 * Filter types:
 *
 * So far here's what I'm thinking. There's a HEADER type which will just have no user data.
 * (Like the headers for Totals, Hosted Accounts, Local Wallets, Stash Node Pros, and Contacts).
 *
 * There's also an ASSET type which has to transcend Opentxs asset type. Because the BTC balance
 * in the local Bitcoin wallet doesn't have an Opentxs asset type ID. But the BTC balance in the
 * Stash Node Pro DOES have an Opentxs asset type ID. And in fact the BTC balance on another
 * Stash Node Pro wll hace ANOTHER Opentxs asset type ID. So I need to do a query that returns
 * ALL the BTC "balances" I have, even the ones with no Opentxs asset type or account ID, and
 * even multiple different Opentxs asset type IDs and account IDs for what is supposedly the
 * same asset type (Bitcoin, in this example).
 *
 * I believe I'm doing that query already to a CERTAIN degree since I am already displaying
 * the account tree on the screen in this dialog. However, I think I'm just adding up the balances,
 * I'm not necessarily actually compiling a list of all the asset types for each asset type
 * (so to speak). I'll check on that.
 *
 * On a top-level "asset type" filter, I don't have or need a Notary ID, or asset type ID, or
 * account ID, though I may have lists of those things.
 *
 * On a "notary" filter, I only need the notary ID and I only want to filter on that alone.
 *
 * On an "account" filter, I only NEED the account ID but I might as well include other information
 * on the account's tree node such as its notary ID or asset type or balance. Why? I dunno, in case
 * it's useful? But I don't think I really need it at all...
 * But if I do keep asset type, I will need the "'BTC' versus 'LTC'" aspect of it, as well as the
 * Opentxs "asset type ID" aspect of it (IF applicable).
 *
 * I'm going to test creating a second Currency with the same BTC currency symbol... yup it does it by symbol. (TLA).
 *
 * Another note: The Hosted accounts are a loop through the servers, and accounts for each server. (Opentxs).
 * But the Stash Node Pros are looping through OTME_too's paired SNPs, not all of which even HAVE a server
 * ID yet! So we can't ASSUME we have a server ID, even in cases where we know we eventually WILL have one!
 *
 *
#define ACTIVITY_TREE_HEADER_TOTALS   0
#define ACTIVITY_TREE_HEADER_LOCAL    1
#define ACTIVITY_TREE_HEADER_SNP      2
#define ACTIVITY_TREE_HEADER_HOSTED   3
#define ACTIVITY_TREE_HEADER_CONTACTS 4

#define AC_NODE_TYPE_HEADER          0
#define AC_NODE_TYPE_ASSET_TOTAL     1
#define AC_NODE_TYPE_CONTACT         2
#define AC_NODE_TYPE_LOCAL_WALLET    3
#define AC_NODE_TYPE_LOCAL_ACCOUNT   4
#define AC_NODE_TYPE_SNP_NOTARY      5
#define AC_NODE_TYPE_SNP_ACCOUNT     6
#define AC_NODE_TYPE_HOSTED_NOTARY   7
#define AC_NODE_TYPE_HOSTED_ACCOUNT  8
 *
 * So UserData(0) should be the NODE TYPE. For the first row, it will be a header (Totals)
 * and so the "node type" should be: AC_NODE_TYPE_HEADER.
 * For the second row, it will be an asset total (probably BTC or USD) so the "node type"
 * would be: AC_NODE_TYPE_ASSET_TOTAL. Again, set in UserData(0).
 *
 * EVERY row in the tree is also under a "header", and so should have the header set.
 * Continuing the above example, the first row has "header" of ACTIVITY_TREE_HEADER_TOTALS
 * and the second row also has "header" of ACTIVITY_TREE_HEADER_TOTALS. This should probably
 * be UserData(1).
 *
 * IF the node type is AC_NODE_TYPE_ASSET_TOTAL, as seen just above for the second row,
 * we probably also want to have the TLA available. This should probably be UserData(2).
 *
 * IF the node type is instead, say, AC_NODE_TYPE_SNP_ACCOUNT or AC_NODE_TYPE_HOSTED_ACCOUNT,
 * tracking account instead of asset ID, then in that case UserData(2) should contain the TLA
 * as before (for consistency's sake). Therefore the account ID should be in UserData(4).
 *
 * That way I can put the Opentxs asset type ID (if one exists) into UserData(3).
 *
 * Let's say instead the "node type" is AC_NODE_TYPE_HOSTED_NOTARY -- in that case, the "header"
 * is ACTIVITY_TREE_HEADER_HOSTED. So that's user data 0 and 1.
 * Then UserData(2) would contain the Opentxs Notary ID, if one was available. (Which is possible,
 * at least in the case of AC_NODE_TYPE_SNP_NOTARY).
 * NO -- this should also contain the TLA in 2, the asset type in 3, and the account ID in 4,
 * IF APPLICABLE, since the notary MAY display a "rolled up" balance total as well, and I prefer
 * to have those values available if they are relevant, in a way consistent with the above.
 * (However, once again, I cannot assume they are present. They may not be.)
 *
 * Therefore the Notary ID must be in UserData(5)! IF it's present at all.
 *
 * Therefore if it turns out to be a Contact, "node type" AC_NODE_TYPE_CONTACT, the Contact ID
 * itself, a new-style string-based Opentxs Contact ID, will preferably be in UserData(6).
 */

//QString qstrCurrentTLA_;  // If the user clicks on "BTC" near the top, the current TLA is "BTC".
//QString qstrCurrentNotary_; // If the user clicks on "localhost" (hosted notary) then that Notary ID is set here.
//QString qstrCurrentAccount_; // If the user clicks on one of his Opentxs accounts, or a notary with only 1 account under it, we put the acct ID here.
//QString qstrCurrentContact_; // If the user clicks on one of his contacts, we put the Opentxs Contact ID here.

ACTIVITY_TREE_ITEM Activity::make_tree_item( QString & qstrCurrentTLA, QString & qstrCurrentNotary, QString & qstrCurrentAccount, QString & qstrCurrentContact)
{
    return std::make_tuple(qstrCurrentTLA.toStdString(), qstrCurrentNotary.toStdString(), qstrCurrentAccount.toStdString(), qstrCurrentContact.toStdString());
}


void Activity::set_inbox_pmntid_for_tree_item(ACTIVITY_TREE_ITEM & theItem, int nPmntID)
{
    mapOfActivityTreeItems::iterator it = mapCurrentRows_inbox.find(theItem);
    if (mapCurrentRows_inbox.end() != it) // found it.
    {
        mapCurrentRows_inbox.erase(it);
    }
    mapCurrentRows_inbox.insert( std::pair<ACTIVITY_TREE_ITEM, int> (theItem, nPmntID) );
}

void Activity::set_outbox_pmntid_for_tree_item(ACTIVITY_TREE_ITEM & theItem, int nPmntID)
{
    mapOfActivityTreeItems::iterator it = mapCurrentRows_outbox.find(theItem);
    if (mapCurrentRows_outbox.end() != it) // found it.
    {
        mapCurrentRows_outbox.erase(it);
    }
    mapCurrentRows_outbox.insert( std::pair<ACTIVITY_TREE_ITEM, int> (theItem, nPmntID) );
}

int Activity::get_inbox_pmntid_for_tree_item(ACTIVITY_TREE_ITEM & theItem)
{
    mapOfActivityTreeItems::iterator it = mapCurrentRows_inbox.find(theItem);
    if (mapCurrentRows_inbox.end() != it) // found it.
    {
        return it->second;
    }
    return 0;
}

int Activity::get_outbox_pmntid_for_tree_item(ACTIVITY_TREE_ITEM & theItem)
{
    mapOfActivityTreeItems::iterator it = mapCurrentRows_outbox.find(theItem);
    if (mapCurrentRows_outbox.end() != it) // found it.
    {
        return it->second;
    }
    return 0;
}


void Activity::RefreshUserBar()
{
    const std::string str_my_nym_id = Moneychanger::It()->get_default_nym_id().toStdString();
    // ----------------------------------------
    MTNameLookupQT theLookup;
    const QString qstrNymName = QString::fromStdString(theLookup.GetNymName(str_my_nym_id, ""));
    ui->toolButtonMyIdentity2->setText(qstrNymName);
}


void Activity::RefreshAll()
{
    RefreshUserBar();
    RefreshAccountTab();
    RefreshConversationsTab();
}


void Activity::RefreshAccountTab()
{
    QTimer::singleShot(0, this, SLOT(RefreshAccountTree()));
}

void Activity::ClearAccountTree()
{
//    QTreeWidget * pTreeWidgetAccounts = ui->treeWidgetAccounts;
//    if (nullptr == pTreeWidgetAccounts) {
//        return;
//    }
//    // ----------------------------------------
//    QTreeWidgetItem * pItem = pTreeWidgetAccounts->currentItem();

//    if (nullptr != pItem)
//    {
//        const int nNodeType = pItem->data(0, Qt::UserRole+0).toInt();
//        const int nHeader   = pItem->data(0, Qt::UserRole+1).toInt();

//        switch (nNodeType) {
//            case AC_NODE_TYPE_HEADER: {
////                qstrCurrentTLA_      = QString("");
////                qstrCurrentNotary_   = QString("");
////                qstrCurrentAccount_  = QString("");
////                qstrCurrentContact_  = QString("");
//                ;
//            } break;

//            case AC_NODE_TYPE_ASSET_TOTAL: {
//                const QVariant qvarTLA = pItem->data(0, Qt::UserRole+2);

//                qstrCurrentTLA_      = qvarTLA.isValid() ? qvarTLA.toString() : QString("");
//                qstrCurrentNotary_   = QString("");
//                qstrCurrentAccount_  = QString("");
//                qstrCurrentContact_  = QString("");

////                qDebug() << "Setting qstrCurrentContact_ to empty string! 1\n";

//            } break;

//                // Todo someday. Currently Moneychanger doesn't have its own local Bitcoin wallet.
//            case AC_NODE_TYPE_LOCAL_WALLET:
//            case AC_NODE_TYPE_LOCAL_ACCOUNT: {
//                qstrCurrentTLA_      = QString("");
//                qstrCurrentNotary_   = QString("");
//                qstrCurrentAccount_  = QString("");
//                qstrCurrentContact_  = QString("");

////                qDebug() << "Setting qstrCurrentContact_ to empty string! 2\n";

//            } break;

//            case AC_NODE_TYPE_SNP_NOTARY:
//            case AC_NODE_TYPE_HOSTED_NOTARY: {
//                // If a Notary has a single account, it may display them on a single line.
//                // In which case, sometimes we'll want the Notary ID off that line, but
//                // sometimes we'll want the account ID. (But the account ID will sometimes
//                // not be there, if there are sub-accounts listed under the notary in the UI).
//                //
////              const QVariant qvarAccountId = pItem->data(0, Qt::UserRole+4);
//                const QVariant qvarNotaryId  = pItem->data(0, Qt::UserRole+5);

//                qstrCurrentTLA_      = QString("");
//                qstrCurrentAccount_  = QString("");
////              qstrCurrentAccount_  = qvarAccountId.isValid() ? qvarAccountId.toString() : QString("");
//                qstrCurrentNotary_   = qvarNotaryId.isValid()  ? qvarNotaryId.toString()  : QString("");
//                qstrCurrentContact_  = QString("");

////                qDebug() << "Setting qstrCurrentContact_ to empty string! 3\n";

//            } break;

//            case AC_NODE_TYPE_SNP_ACCOUNT:
//            case AC_NODE_TYPE_HOSTED_ACCOUNT: {
//                const QVariant qvarAccountId = pItem->data(0, Qt::UserRole+4);

//                qstrCurrentTLA_      = QString("");
//                qstrCurrentNotary_   = QString("");
//                qstrCurrentAccount_  = qvarAccountId.isValid() ? qvarAccountId.toString() : QString("");
//                qstrCurrentContact_  = QString("");

////                qDebug() << "Setting qstrCurrentContact_ to empty string! 4\n";

//            } break;

//            case AC_NODE_TYPE_CONTACT: {
//                const QVariant qvarContactId = pItem->data(0, Qt::UserRole+6);

//                qstrCurrentTLA_      = QString("");
//                qstrCurrentNotary_   = QString("");
//                qstrCurrentAccount_  = QString("");
//                qstrCurrentContact_  = qvarContactId.isValid() ? qvarContactId.toString() : QString("");
//            } break;

//            default: {
//                qstrCurrentTLA_      = QString("");
//                qstrCurrentNotary_   = QString("");
//                qstrCurrentAccount_  = QString("");
//                qstrCurrentContact_  = QString("");

////                qDebug() << "Setting qstrCurrentContact_ to empty string! 5 it's a default case: " << QString::number(nNodeType) << "\n";

//            } break;
//        }
//    }
//    else
//    {
//        // NOTE: This may have been the bug that caused the left-hand of
//        // activity page to lose selection after a few seconds delay.
//        // Commenting it out now to test that. May keep it that way, if it fixes it.
////        qstrCurrentTLA_      = QString("");
////        qstrCurrentNotary_   = QString("");
////        qstrCurrentAccount_  = QString("");
////        qstrCurrentContact_  = QString("");

////        qDebug() << "Setting qstrCurrentContact_ to empty string! 6 it's nullptr\n";

//    }
//    // --------------------------------------
//    ui->treeWidgetAccounts->clear();
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

void Activity::GetAssetContractIdsInWallet(std::map<std::string, std::string> & map_output)
{
    const int32_t  asset_count = opentxs::OT::App().API().Exec().GetAssetTypeCount();
    for (int32_t ii = 0; ii < asset_count; ii++)
    {
        const std::string OT_id   = opentxs::OT::App().API().Exec().GetAssetType_ID(ii);
        const std::string OT_name = opentxs::OT::App().API().Exec().GetAssetType_TLA(OT_id);
        map_output.insert(std::pair<std::string, std::string>(OT_id, OT_name));
    }
}

void Activity::GetCurrencyTypesByAssetContractsInWallet(std::set<StringIntPair> & set_output)
{
    std::map<std::string, std::string> map_units;
    GetAssetContractIdsInWallet(map_units);

    for (const auto & unit : map_units)
    {
        const auto & unit_type_id = unit.first;
        const auto & unit_TLA     = unit.second;
        auto unitTypeId = opentxs::Identifier::Factory(unit_type_id);
        const opentxs::proto::ContactItemType currency_type =
                opentxs::OT::App().Wallet().CurrencyTypeBasedOnUnitType(unitTypeId);
        set_output.insert(StringIntPair(unit_TLA, static_cast<int>(currency_type)));
    }
}

void Activity::GetAssetIdMapsByCurrencyCode(mapOfMapIDName & bigMap)
{
    int32_t  asset_count = opentxs::OT::App().API().Exec().GetAssetTypeCount();
    for (int32_t ii = 0; ii < asset_count; ii++)
    {
        const QString OT_id   = QString::fromStdString(opentxs::OT::App().API().Exec().GetAssetType_ID(ii));
        const QString OT_name = QString::fromStdString(opentxs::OT::App().API().Exec().GetAssetType_Name(OT_id.toStdString()));
        const QString qstrTLA = QString::fromStdString(opentxs::OT::App().API().Exec().GetCurrencyTLA(OT_id.toStdString()));
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

        for (const auto& [accountID, alias] : opentxs::OT::App().DB().AccountList())
        {
            std::string assetTypeID = opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID(accountID);

            QString qstrAssetTypeId = QString::fromStdString(assetTypeID);
            if (0 == qstrAssetTypeId.compare(qstrUnitTypeId))
            {
                const int64_t lSubtotal = opentxs::OT::App().API().Exec().GetAccountWallet_Balance(accountID);

                // We don't count the negative balances, since those are for issuer accounts,
                // and if we included them, the total would always balance out to zero anyway.
                if (lSubtotal > 0)
                {
                    lTotal += lSubtotal;
                }
            }
        }
        // ----------------------------------------------------
        ++ci;
    }
    return lTotal;
}


// NOTE: There may be multiple Nyms in the same wallet, who are all paired to
// an issuer on the same server.
// Therefore, if Alice and Bob are both only paired to Issuer A, this function
// will return a "1" instead of a "2". Put another way, it returns a count based
// on a DEDUPLICATED server list.
//
int Activity::PairedNodeCount(std::set<opentxs::Identifier> * pUniqueServers/*=nullptr*/)
{
    std::set<opentxs::Identifier> uniqueServers;

    if (nullptr == pUniqueServers) {
        pUniqueServers = &uniqueServers;
    }

    pUniqueServers->clear();

    //auto servers = opentxs::OT::App().Wallet().ServerList();

    /*
    Justus
    It depends
    15:31
    That returns all nyms known to the wallet, not just your nyms
    15:31
    C
    Chris
    what if I only want my own?
    15:31
    J
    Justus
    You have to use OTAPI_Exec still
    */
    opentxs::ObjectList myNyms;
    const auto nNymCount = opentxs::OT::App().API().Exec().GetNymCount();

    for (int nym_index = 0; nym_index < nNymCount; ++nym_index)
    {
        const std::string nym_id = opentxs::OT::App().API().Exec().GetNym_ID(nym_index);

        if (!nym_id.empty())
        {
            const std::string nym_name = opentxs::OT::App().API().Exec().GetNym_Name(nym_id);
            myNyms.push_back(std::pair<std::string, std::string>{nym_id, nym_name});
        }
    }
    if (0 == myNyms.size()) {
        return 0;
    }
    // ----------------------------------------------------
    const auto & nyms = myNyms;
//  const auto   nyms = opentxs::OT::App().Wallet().NymList();

    for (const auto & [ nym_id, nym_alias] : nyms)
    {
        const opentxs::Identifier nymId{nym_id};
        auto setIssuerIDs = opentxs::OT::App().Wallet().IssuerList(nymId);

        auto it_issuer = setIssuerIDs.begin();

        while (it_issuer != setIssuerIDs.end())
        {
            const opentxs::Identifier issuerId = *it_issuer;
            it_issuer++;

            auto pIssuer = opentxs::OT::App().Wallet().Issuer(nymId, issuerId);

            if (!pIssuer) {
                continue;
            }
            // -------------------------------------------------
            // This call to Paired will soon be renamed to "Trusted"
            // and it actually means "Paired or in the process of Pairing"
            // (Aka "paireding").
            //
            if (pIssuer->Paired()) {
                pUniqueServers->insert(pIssuer->PrimaryServer());
                // -------------------------------
                //
                // The purpose of the below section is simply to notify Moneychanger
                // about the various relationships, such as that account_id A is
                // on notary B, and is owned by Nym C, and consists of asset type D.
                // There are many places in the Moneychanger code that query for
                // information about these relationships. So until all those places
                // are ported to use the new API calls, I do the below code so that
                // Moneychanger's existing system will continue functioning properly
                // by, for example, being able to search for accounts of a given unit
                // type or on a given notary.
                //
                auto pIssuerNym = opentxs::OT::App().Wallet().Nym(issuerId);

                if (pIssuerNym)
                {
                    const opentxs::String strNotaryId{pIssuer->PrimaryServer()};
                    const std::string str_notary_id{strNotaryId.Get()};

                    auto units = pIssuerNym->Claims().Section(opentxs::proto::CONTACTSECTION_CONTRACT);

                    for (const auto& [type, group] : *units) {
                        // type is a proto::ContactItemType which represents the unit of account
                        // such as BTC or LTC etc. This is what the UI needs to be converted to
                        // use, instead of the TLA or "3 digit code" it uses now.

                        for (const auto& [id, claim] : *group) {
                            // claim.Value() is a unit definition id;

                            const std::string str_unit_type_id{claim->Value()};
                            const opentxs::Identifier unit_type_id{str_unit_type_id};

                            auto accountList = pIssuer->AccountList(type, unit_type_id);

                            for (auto & accountId : accountList) {
                                // Note: we don't actually need the below name.
                                // We're just calling this function because it notifies
                                // Moneychanger's internal DB about the relationships
                                // between these IDs. This ensures that various queries
                                // pre-existing all over Moneychanger's code will
                                // continue to work properly until everything is converted
                                // to the new API.
                                //
                                MTNameLookupQT theLookup;
                                const std::string str_acct_name =
                                    theLookup.GetAcctName(accountId->str(), nym_id, str_notary_id, str_unit_type_id);
                            }
                        }
                    }
                }
            }
        }
    }
    return pUniqueServers->size();
}

/*
To loop through an issuer's claims about the currencies he's issued:

auto issuerNym = opentxs::OT::App().Wallet().Nym(Issuer.IssuerID());

auto issued_unit_types = issuerNym->Claims().Section(proto::CONTACTSECTION_CONTRACT)

for (const auto& [type, group] : *issued_unit_types) {
    // type is a proto::ContactItemType which represents the unit of account

    for (const auto& [id, claim] : group) {
        // claim.Value() is a unit definition id;
    }
}
*/

bool Activity::PairingStarted(const opentxs::Identifier & nymId, const opentxs::Identifier & issuerNymId)
{
    auto pIssuer = opentxs::OT::App().Wallet().Issuer(nymId, issuerNymId);

    if (!pIssuer) {
        return false;
    }
    // -------------------------------------------------
    // This call to Paired will soon be renamed to "Trusted"
    // and it actually means "Paired or in the process of Pairing"
    // (Aka "paireding").
    //
    else if (pIssuer->Paired()) {
        return true;
    }
    return false;
}

void Activity::GetAccountIdMapsByServerId(mapOfMapIDName & bigMap, const bool bTrusted, // true == paired, false == hosted.
                                          mapIDName * pMapTLAbyAccountId/*=nullptr*/)
{
    const bool bCallerWantsToSeePairedNodes = bTrusted;
    const auto myAccounts = opentxs::OT::App().DB().AccountList();

    for (const auto & [ account_id, account_name] : myAccounts)
    {
        const QString OT_id   = QString::fromStdString(account_id);
        const QString OT_name = QString::fromStdString(account_name);
        const std::string str_nym_id = opentxs::OT::App().API().Exec().GetAccountWallet_NymID(account_id);
        const opentxs::Identifier nymId{str_nym_id};
        const std::string str_server_id = opentxs::OT::App().API().Exec().GetAccountWallet_NotaryID(account_id);
        const QString qstrServerId = QString::fromStdString(str_server_id);

        const std::string str_unit_type_id = opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID(account_id);
        const opentxs::Identifier unit_type_id{str_unit_type_id};
        const QString qstrUnitTypeId = QString::fromStdString(str_unit_type_id);

        const std::string OT_asset_TLA  = opentxs::OT::App().API().Exec().GetCurrencyTLA(str_unit_type_id);
        const QString qstrTLA = QString::fromStdString(OT_asset_TLA);

        auto unitTypeContract = opentxs::OT::App().Wallet().UnitDefinition(unit_type_id);

        if (!unitTypeContract) {
            qDebug() << "GetAccountIdMapsByServerId: I own an account yet somehow dont have the unit type contract in my wallet.";
            continue;
        }

        const opentxs::Identifier issuerNymId = unitTypeContract->Nym()->ID();
        //std::string GetSignerNymID(const std::string& str_Contract) const;

        // bPaireding means, paired or in the process of pairing.
        const bool bPaireding = PairingStarted(nymId, issuerNymId);
        //const bool bPaireding = opentxs::OT::App().API().OTME_TOO().PairingStarted(str_server_id);

        // Basically the caller either wants a list of accounts by server ID for PAIRED servers,
        // OR he wants a list of accounts by server ID for NON-Paired (hosted) servers.
        // So those are the two cases where we add the server to the list.
        if (    ( bPaireding &&  bCallerWantsToSeePairedNodes)
             || (!bPaireding && !bCallerWantsToSeePairedNodes))
        {
            mapIDName & mapServerId = GetOrCreateAccountIdMapByServerId(qstrServerId, bigMap);
            mapServerId.insert(OT_id, OT_name);

            if (nullptr != pMapTLAbyAccountId) {
                pMapTLAbyAccountId->insert(OT_id, qstrTLA);
            }
        }
    }
}

mapIDName & Activity::GetOrCreateAccountIdMapByServerId(QString qstrServerId, mapOfMapIDName & bigMap)
{
    mapOfMapIDName::iterator i = bigMap.find(qstrServerId);
    while ((i != bigMap.end()) && (0 == qstrServerId.compare(i.key()))) {
        return i.value();
    }
    // else create it.
    bigMap.insert(qstrServerId, mapIDName{});
    i = bigMap.find(qstrServerId);
    return i.value();
}



void Activity::GetAccountsByTLAFromMap(
    const mapIDName & mapAccounts, // input
    const mapIDName & mapTLAByAccountId, // input
    mapOfMapIDName  & bigMap) // ouput
{
    mapIDName::const_iterator it_accounts = mapAccounts.begin();

    while (it_accounts != mapAccounts.end()) {

        const QString & qstrKey   = it_accounts.key(); // account id
        const QString & qstrValue = it_accounts.value(); // account name

        mapIDName::const_iterator it_TLA = mapTLAByAccountId.find(qstrKey);

        if (it_TLA != mapTLAByAccountId.end()) {
            const QString qstrTLA = it_TLA.value();
            mapIDName & mapAccounts = GetOrCreateAccountIdMapByTLA(qstrTLA, bigMap);
            mapAccounts.insert(qstrKey, qstrValue);
        }
        it_accounts++;
    }
}

mapIDName & Activity::GetOrCreateAccountIdMapByTLA(QString qstrTLA, mapOfMapIDName & bigMap)
{
    mapOfMapIDName::iterator i = bigMap.find(qstrTLA);
    while ((i != bigMap.end()) && (0 == qstrTLA.compare(i.key()))) {
        return i.value();
    }
    // else create it.
    bigMap.insert(qstrTLA, mapIDName{});
    i = bigMap.find(qstrTLA);
    return i.value();
}



/*
#define ACTIVITY_TREE_HEADER_TOTALS   0
#define ACTIVITY_TREE_HEADER_LOCAL    1
#define ACTIVITY_TREE_HEADER_SNP      2
#define ACTIVITY_TREE_HEADER_HOSTED   3
#define ACTIVITY_TREE_HEADER_CONTACTS 4

#define AC_NODE_TYPE_HEADER          0
#define AC_NODE_TYPE_ASSET_TOTAL     1
#define AC_NODE_TYPE_CONTACT         2
#define AC_NODE_TYPE_LOCAL_WALLET    3
#define AC_NODE_TYPE_LOCAL_ACCOUNT   4
#define AC_NODE_TYPE_SNP_NOTARY      5
#define AC_NODE_TYPE_SNP_ACCOUNT     6
#define AC_NODE_TYPE_HOSTED_NOTARY   7
#define AC_NODE_TYPE_HOSTED_ACCOUNT  8
 *
 * So UserData(0) should be the NODE TYPE. For the first row, it will be a header (Totals)
 * and so the "node type" should be: AC_NODE_TYPE_HEADER and "header" will be ACTIVITY_TREE_HEADER_TOTALS.
 *
 * For the second row, it will be an asset total (probably BTC or USD) so the "node type"
 * would be: AC_NODE_TYPE_ASSET_TOTAL. Again, set in UserData(0).
 *
 * EVERY row in the tree is also under a "header", and so should have the header set.
 * Continuing the above example, the first row has "header" of ACTIVITY_TREE_HEADER_TOTALS
 * and the second row also has "header" of ACTIVITY_TREE_HEADER_TOTALS. This should probably
 * be UserData(1).
 *
 * IF the node type is AC_NODE_TYPE_ASSET_TOTAL, as seen just above for the second row,
 * we probably also want to have the TLA available. This should probably be UserData(2).
 *
 * IF the node type is instead, say, AC_NODE_TYPE_SNP_ACCOUNT or AC_NODE_TYPE_HOSTED_ACCOUNT,
 * tracking account instead of asset ID, then in that case UserData(2) should contain the TLA
 * as before (for consistency's sake). Therefore the account ID should be in UserData(4).
 *
 * That way I can put the Opentxs asset type ID (if one exists) into UserData(3).
 *
 * Let's say instead the "node type" is AC_NODE_TYPE_HOSTED_NOTARY -- in that case, the "header"
 * is ACTIVITY_TREE_HEADER_HOSTED. So that's user data 0 and 1.
 * Then UserData(2) would contain the Opentxs Notary ID, if one was available. (Which is possible,
 * at least in the case of AC_NODE_TYPE_SNP_NOTARY).
 * NO -- this should also contain the TLA in 2, the asset type in 3, and the account ID in 4,
 * IF APPLICABLE, since the notary MAY display a "rolled up" balance total as well, and I prefer
 * to have those values available if they are relevant, in a way consistent with the above.
 * (However, once again, I cannot assume they are present. They may not be.)
 *
 * Therefore the Notary ID must be in UserData(5)! IF it's present at all.
 *
 * Therefore if it turns out to be a Contact, "node type" AC_NODE_TYPE_CONTACT, the Contact ID
 * itself, a new-style string-based Opentxs Contact ID, will preferably be in UserData(6).
 */

//void blah(QString strID) // asset ID
//{
//    // ----------------------------------
//    const QString qstrContents = QString::fromStdString(opentxs::OT::App().API().Exec().GetAssetType_Contract(strID.toStdString()));
//    opentxs::proto::UnitDefinition contractProto = opentxs::proto::StringToProto<opentxs::proto::UnitDefinition>
//                                                    (opentxs::String(qstrContents.toStdString()));
//    // ----------------------------------
//    QString qstrDetails("");

//    qstrDetails += contractProto.has_name() ? QString("- %1: %2\n").arg(tr("Name")).arg(QString::fromStdString(contractProto.name())) : QString("");
//    qstrDetails += contractProto.has_version() ? QString("- %1: %2\n").arg(tr("Version")).arg(QString::number(contractProto.version())) : QString("");
//    qstrDetails += contractProto.has_shortname() ? QString("- %1: %2\n").arg(tr("Short Name")).arg(QString::fromStdString(contractProto.shortname())) : QString("");
//    qstrDetails += contractProto.has_symbol() ? QString("- %1: %2\n").arg(tr("Symbol")).arg(QString::fromStdString(contractProto.symbol())) : QString("");

//    if (contractProto.has_currency()) {
//        qstrDetails += QString("%1:\n").arg(tr("Is currency"));
//        auto & params = contractProto.currency();
//        qstrDetails += params.has_tla() ? QString("- %1: %2\n").arg(tr("TLA")).arg(QString::fromStdString(params.tla())) : QString("");
//        qstrDetails += params.has_version() ? QString("- %1: %2\n").arg(tr("Version")).arg(QString::number(params.version())) : QString("");
//        qstrDetails += params.has_fraction() ? QString("- %1: %2\n").arg(tr("Fraction")).arg(QString::fromStdString(params.fraction())) : QString("");
//        qstrDetails += params.has_power() ? QString("- %1: %2\n").arg(tr("Power")).arg(QString::number(params.power())) : QString("");
//    }
//    if (contractProto.has_security()) {
//        qstrDetails += QString("%1:\n").arg(tr("Is shares"));
//        auto & params = contractProto.security();
//        qstrDetails += params.has_version() ? QString("- %1: %2\n").arg(tr("Version")).arg(QString::number(params.version())) : QString("");
//    }
//    if (contractProto.has_basket()) {
//        qstrDetails += QString("%1:\n").arg(tr("Is basket"));
//        auto & params = contractProto.basket();
//        qstrDetails += params.has_version() ? QString("- %1: %2\n").arg(tr("Version")).arg(QString::number(params.version())) : QString("");
//    }

//    qstrDetails += contractProto.has_terms() ? QString("- %1:\n%2").arg(tr("Terms")).arg(QString::fromStdString(contractProto.terms())) : QString("");

//}




void print_accounts(const opentxs::ui::IssuerItem& issuer) ;
void print_line(const opentxs::ui::IssuerItem& line) ;
void print_line(
    const opentxs::ui::AccountSummaryItem& line) ;


void print_accounts(const opentxs::ui::IssuerItem& issuer)
{
//    opentxs::otOut << "Top of print_ccounts..." << std::endl;

    auto account = issuer.First();

    if (false == account->Valid()) { opentxs::otOut << "RETURNING SINCE NO VALID ACCOUNT 1"; return; }

    print_line(account.get());

    auto lastAccount = account->Last();
    while (false == lastAccount) {
        opentxs::otOut << "Iterating another account..." << std::endl;
        account = issuer.Next();
        if (false == account->Valid()) { return; }
        lastAccount = account->Last();
        print_line(account.get());
    }
}

void print_line(const opentxs::ui::IssuerItem& line)
{
    opentxs::otOut << "* " << line.Name() << " [";

    if (line.ConnectionState()) {
        opentxs::otOut << "*";
    } else {
        opentxs::otOut << " ";
    }

    opentxs::otOut << "]\n";
    print_accounts(line);
}

void print_line(
    const opentxs::ui::AccountSummaryItem& line)
{
    opentxs::otOut << " Account display label: '" << line.Name() << "' " << "Balance: " << line.DisplayBalance() << std::endl;

    // This is the spot, ultimately, where the item would actually be added to the tree widget.
    // TODO.
}




// Note: If this function was even called in the first place, that means
// one of the widget IDs definitely did match to the one in the published notice.
// Otherwise we wouldn't bother refreshing this tree at all.
//
void Activity::RefreshSummaryTree()
{
//    opentxs::otErr << "Activity::RefreshSummaryTree was called!! We're at the top now!!" << std::endl;


    QTreeWidget * pTreeWidget = ui->treeWidgetSummary;
    if (nullptr == pTreeWidget) {
        return;
    }
    // ----------------------------------------

    const QVariant currentDataMyNym    = ui->comboBoxMyNym->currentData();
    const QVariant currentDataCurrency = ui->comboBoxCurrency->currentData();

    // ----------------------------------------
    pTreeWidget->blockSignals(true);

    pTreeWidget->clear();
    // ----------------------------------------

    if (!currentDataMyNym.isValid() ||
        !currentDataCurrency.isValid())
    {
        pTreeWidget->blockSignals(false);
        return;
    }
    // ------------------------------------
    const int currency_type = currentDataCurrency.toInt();
    const std::string mynym = currentDataMyNym.toString().toStdString();

    const auto currencyType = opentxs::proto::ContactItemType(currency_type);
    const opentxs::Identifier nymID{mynym};
    // ------------------------------------
    //resume
    QList<QTreeWidgetItem *> items;

    QTreeWidgetItem * pHostedAccountsItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(tr("Hosted Accounts")));
    items.append(pHostedAccountsItem);
    pHostedAccountsItem->setExpanded(true);
    pHostedAccountsItem->setData(0, Qt::UserRole+0, QVariant(AC_NODE_TYPE_HEADER));
    pHostedAccountsItem->setData(0, Qt::UserRole+1, QVariant(ACTIVITY_TREE_HEADER_HOSTED));
    pHostedAccountsItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
    pHostedAccountsItem->setTextColor(0, Qt::white);
    pHostedAccountsItem->setTextColor(1, Qt::white);
    pHostedAccountsItem->setBackgroundColor(0, Qt::gray);
    pHostedAccountsItem->setBackgroundColor(1, Qt::gray);
    pHostedAccountsItem->setBackgroundColor(2, Qt::gray);
    pHostedAccountsItem->setBackgroundColor(3, Qt::gray);
    pHostedAccountsItem->setFlags(pHostedAccountsItem->flags()&~Qt::ItemIsSelectable);
    // ----------------------------------------

    auto& list = opentxs::OT::App().UI().AccountSummary(nymID, currencyType);
    auto issuer = list.First();

    if (false == issuer->Valid()) {

        pTreeWidget->blockSignals(false);
        return;
    }

    const QString qstrIssuerName = QString::fromStdString(issuer->Name());

//    EXPORT virtual bool ConnectionState() const = 0;
//    EXPORT virtual std::string Debug() const = 0;
//    EXPORT virtual opentxs::SharedPimpl<opentxs::ui::AccountSummaryItem> First()
//        const = 0;
//    EXPORT virtual std::string Name() const = 0;
//    EXPORT virtual opentxs::SharedPimpl<opentxs::ui::AccountSummaryItem> Next()
//        const = 0;
//    EXPORT virtual bool Trusted() const = 0;

    // --------------------------------------------
    QTreeWidgetItem * pWidgetIssuer = new QTreeWidgetItem((QTreeWidget *)nullptr,
        QStringList(qstrIssuerName) << QString(""));
    items.append(pWidgetIssuer);
    pWidgetIssuer->setExpanded(false);
//    pWidgetIssuer->setData(0, Qt::UserRole+0, QVariant(AC_NODE_TYPE_ASSET_TOTAL));
//    pWidgetIssuer->setData(0, Qt::UserRole+1, QVariant(ACTIVITY_TREE_HEADER_TOTALS));
//    pWidgetIssuer->setData(0, Qt::UserRole+2, QVariant(qstrCurrencyCode));

    pWidgetIssuer->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);


    const QString qstrIconPath = QString(issuer->ConnectionState()
                                         ? ":/icons/icons/green_dot.png"
                                         : ":/icons/icons/red_dot.png");
    pWidgetIssuer->setIcon(0, QIcon(qstrIconPath));

    if (issuer->ConnectionState())
    {
//      pWidgetIssuer->setTextColor(0, Qt::black);
//      pWidgetIssuer->setTextColor(1, Qt::black);
    }
    else
    {
        pWidgetIssuer->setTextColor(0, Qt::gray);
        pWidgetIssuer->setTextColor(1, Qt::gray);
    }
    pWidgetIssuer->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

    auto account = issuer->First();

    if (account->Valid()) {
        const std::string account_id(account->AccountID());
        const std::string account_widget_id(account->WidgetID()->str());
        const std::string account_name(account->Name());
        const std::string display_balance(account->DisplayBalance());
        const QString qstrName = QString::fromStdString(account_name);
        const QString qstrDisplayBalance = QString::fromStdString(display_balance);

        QTreeWidgetItem * pWidgetItem = new QTreeWidgetItem((QTreeWidget *)nullptr,
            QStringList(qstrName) << qstrDisplayBalance);
        items.append(pWidgetItem);
        pWidgetItem->setExpanded(false);
//            pWidgetItem->setData(0, Qt::UserRole+0, QVariant(AC_NODE_TYPE_ASSET_TOTAL));
//            pWidgetItem->setData(0, Qt::UserRole+1, QVariant(ACTIVITY_TREE_HEADER_TOTALS));
//            pWidgetItem->setData(0, Qt::UserRole+2, QVariant(qstrCurrencyCode));

        pWidgetItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

        auto lastAccount = account->Last();
        while (false == lastAccount) {
            account = issuer->Next();
            if (false == account->Valid()) { break; }
            lastAccount = account->Last();
            const std::string account_id(account->AccountID());
            const std::string account_widget_id(account->WidgetID()->str());
            const std::string account_name(account->Name());
            const std::string display_balance(account->DisplayBalance());
            const QString qstrName = QString::fromStdString(account_name);
            const QString qstrDisplayBalance = QString::fromStdString(display_balance);

//            EXPORT virtual OTIdentifier WidgetID() const = 0;
//            EXPORT virtual bool Last() const = 0;
//            EXPORT virtual bool Valid() const = 0;
//            EXPORT virtual std::string AccountID() const = 0;
//            EXPORT virtual Amount Balance() const = 0;
//            EXPORT virtual std::string DisplayBalance() const = 0;
//            EXPORT virtual std::string Name() const = 0;


            QTreeWidgetItem * pWidgetItem = new QTreeWidgetItem((QTreeWidget *)nullptr,
                QStringList(qstrName) << qstrDisplayBalance);
            items.append(pWidgetItem);
            pWidgetItem->setExpanded(false);
//            pWidgetItem->setData(0, Qt::UserRole+0, QVariant(AC_NODE_TYPE_ASSET_TOTAL));
//            pWidgetItem->setData(0, Qt::UserRole+1, QVariant(ACTIVITY_TREE_HEADER_TOTALS));
//            pWidgetItem->setData(0, Qt::UserRole+2, QVariant(qstrCurrencyCode));

            pWidgetItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);




        }
    }


    if (items.count() > 0)
    {
        pTreeWidget->insertTopLevelItems(0, items);
//      pTreeWidget->resizeColumnToContents(0);
    }

    pTreeWidget->resizeColumnToContents(0);
    pTreeWidget->resizeColumnToContents(1);

    pTreeWidget->blockSignals(false);

    // ----------------------------------------












//    std::set<int> currency_types = opentxs::SwigWrap::GetCurrencyTypesForLocalAccounts();

//    for (const auto currency_type : currency_types)
//    {
//        std::set<std::string> set_unit_types = opentxs::SwigWrap::GetContractIdsForCurrencyType(currency_type);

//        for (const auto & unit_type_id : set_unit_types)
//        {
//           std::set<std::string> accounts = opentxs::SwigWrap::GetAccountIdsForContractId(unit_type_id);

//           for (const auto & account_id : accounts)
//           {
//               const std::string account_label   = opentxs::SwigWrap::GetAccountWallet_Name(account_id);
//               const std::string unit_type_label = opentxs::SwigWrap::GetAssetType_Name(unit_type_id);



//               // Here we have:
//               //   account_id, account_label
//               //   unit_type_id, unit_type_label
//               //   currency_type,
//           }

//        }
//    }






    //resume





}




void Activity::RefreshAccountTree()
{
//    QTreeWidget * pTreeWidgetAccounts = ui->treeWidgetAccounts;
//    if (nullptr == pTreeWidgetAccounts) {
//        return;
//    }
//    // ----------------------------------------
//    ClearAccountTree();
//    // ----------------------------------------
//    pTreeWidgetAccounts->blockSignals(true);
//    // ----------------------------------------
//    QList<QTreeWidgetItem *> items;
//    // ------------------------------------
//    mapOfMapIDName bigMap;

//    this->GetAssetIdMapsByCurrencyCode(bigMap);

//    if (bigMap.size() > 0)
//    {
//        // NOTE: Someday this "Totals" row is where we will someday replace with the USD value of the total wallet.
//        QTreeWidgetItem * pUnfilteredItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(tr("Totals")));
//        items.append(pUnfilteredItem);
//        pUnfilteredItem->setExpanded(true);
//        pUnfilteredItem->setData(0, Qt::UserRole+0, QVariant(AC_NODE_TYPE_HEADER));
//        pUnfilteredItem->setData(0, Qt::UserRole+1, QVariant(ACTIVITY_TREE_HEADER_TOTALS));
//        pUnfilteredItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
//        pUnfilteredItem->setFlags(pUnfilteredItem->flags()&~Qt::ItemIsSelectable);
//        pUnfilteredItem->setTextColor(0, Qt::white);
//        pUnfilteredItem->setTextColor(1, Qt::white);
//        pUnfilteredItem->setBackgroundColor(0, Qt::gray);
//        pUnfilteredItem->setBackgroundColor(1, Qt::gray);
//        pUnfilteredItem->setBackgroundColor(2, Qt::gray);
//        pUnfilteredItem->setBackgroundColor(3, Qt::gray);
//        // ----------------------------------------
//        // Get the total balance for each list of unit types.
//        // (That is, the total balance for each currency code).
//        mapOfMapIDName::const_iterator ci = bigMap.begin();

//        while (ci != bigMap.end()) {
//            const QString qstrCurrencyCode = ci.key();
//            const mapIDName & mapTLA = ci.value();
//            const int64_t nBalanceTotal = GetAccountBalancesTotaledForUnitTypes(mapTLA);
//            const QString qstrFirstAssetTypeId = mapTLA.begin().key();
//            const std::string first_unit_type  = qstrFirstAssetTypeId.toStdString();
//            const std::string str_formatted_amount = opentxs::OT::App().API().Exec().FormatAmountWithoutSymbol(first_unit_type, nBalanceTotal);
//            const QString qstrFormattedAmount = QString::fromStdString(str_formatted_amount);

//            const std::string asset_contract = opentxs::OT::App().API().Exec().GetAssetType_Contract(first_unit_type);
//            opentxs::proto::UnitDefinition contractProto = opentxs::proto::StringToProto<opentxs::proto::UnitDefinition>
//                                                            (opentxs::String(asset_contract));
//            const std::string str_shortname = contractProto.has_shortname() ? contractProto.shortname() : "";
//            const std::string str_name = contractProto.has_name() ? contractProto.name() : "";
//            const QString qstrName(QString::fromStdString(str_name));
//            const std::string str_symbol = (contractProto.has_currency() && contractProto.has_symbol()) ? contractProto.symbol() : "";
//            const QString qstrSymbol = QString::fromUtf8(str_symbol.c_str());

//            QTreeWidgetItem * pCurrencyCodeWidgetItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(qstrName) << qstrSymbol<< qstrFormattedAmount << QStringList(qstrCurrencyCode));
//            items.append(pCurrencyCodeWidgetItem);
//            pCurrencyCodeWidgetItem->setExpanded(false);
//            pCurrencyCodeWidgetItem->setData(0, Qt::UserRole+0, QVariant(AC_NODE_TYPE_ASSET_TOTAL));
//            pCurrencyCodeWidgetItem->setData(0, Qt::UserRole+1, QVariant(ACTIVITY_TREE_HEADER_TOTALS));
//            pCurrencyCodeWidgetItem->setData(0, Qt::UserRole+2, QVariant(qstrCurrencyCode));

//            pCurrencyCodeWidgetItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

//            ++ci;
//        }

////        QTreeWidgetItem * pDefaultCurrency = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList("USD") << "3,487.32");
////        items.append(pDefaultCurrency);
////        pDefaultCurrency->setExpanded(false);
////        pDefaultCurrency->setData(0, Qt::UserRole, QVariant(0));
////        pDefaultCurrency->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
//    }

//    // ----------------------------------------
//    mapOfMapIDName bigMapPairedAccounts;
//    mapIDName mapTLAByAccountId;

//    // First we'll do paired notaries.

//    std::set<opentxs::Identifier> uniquePairedServers;

//    const uint64_t paired_node_count = PairedNodeCount(&uniquePairedServers);

//    if (paired_node_count > 0)
//    {
//        this->GetAccountIdMapsByServerId(bigMapPairedAccounts, true, // True means, only give me accounts on paired servers.
//                                         &mapTLAByAccountId);
//        // ----------------------------------------
//        QTreeWidgetItem * pYourNodeItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(tr("Your Stash Node")));
//        items.append(pYourNodeItem);
//        pYourNodeItem->setExpanded(true);
//        pYourNodeItem->setData(0, Qt::UserRole+0, QVariant(AC_NODE_TYPE_HEADER));
//        pYourNodeItem->setData(0, Qt::UserRole+1, QVariant(ACTIVITY_TREE_HEADER_SNP));
//        pYourNodeItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
//        pYourNodeItem->setTextColor(0, Qt::white);
//        pYourNodeItem->setTextColor(1, Qt::white);
//        pYourNodeItem->setBackgroundColor(0, Qt::gray);
//        pYourNodeItem->setBackgroundColor(1, Qt::gray);
//        pYourNodeItem->setBackgroundColor(2, Qt::gray);
//        pYourNodeItem->setBackgroundColor(3, Qt::gray);
//        pYourNodeItem->setFlags(pYourNodeItem->flags()&~Qt::ItemIsSelectable);
//    }

//    /*
//Justus
//OT::App().Wallet().IssuerList() gives you the list of issuers
//06:58
//In fact, this is the only list you should display.
//06:59
//Don't worry about displaying accounts that were not added through the new Issuer interface.
//06:59
//TUESDAY 10:37
//C
//Chris
//Is there any differentiator between "Hosted Accounts" and "Paired Nodes" ?
//10:37
//used to be 2 lists
//10:37
//J
//Justus
//An issuer can be trusted or not
//10:38
//C
//Chris
//ok
//10:38
//J
//*/

//    for (const opentxs::Identifier & serverId : uniquePairedServers)
////  for (int index = 0; index < paired_node_count; index++) // FOR EACH PAIRED OR PAIRING NODE.
//    {
//        QString qstrSnpName = tr("Pairing Stash Node...");
//        QString qstrServerId;
//        //const std::string str_index = QString::number(index).toStdString();

//        // bPaireding means, paired or in the process of pairing.
//        // UPDATE: the fact we're even in this loop at all means we're paireding.
//        //
//        const bool bPaireding = true; //opentxs::OT::App().API().OTME_TOO().PairingStarted(str_index);
//        bool bConnected = false;
//        // ------------------------------------------------------------------------
//        if (bPaireding)  // If paired or pairing.
//        {
//            opentxs::String strServerId{serverId};

//            const std::string str_snp_server_id{strServerId.Get()};
////          const std::string str_snp_server_id = opentxs::OT::App().API().OTME_TOO().GetPairedServer(str_index);
//            const bool bGotNotaryId = str_snp_server_id.size() > 0;

//            if (bGotNotaryId)
//            {
//                qstrServerId = QString::fromStdString(str_snp_server_id);

//                bConnected = opentxs::OT::App().API().Exec().CheckConnection(str_snp_server_id);
//                QString strConnected = QString(bConnected ? "Connected" : "Not connected");

//                //String strLog = "SNP Notary ID: " + str_snp_server_id + " - " + strConnected;
//                //Log.d("MService", strLog);

//                QString strTemp = QString::fromStdString(opentxs::OT::App().API().Exec().GetServer_Name(str_snp_server_id));
//                if (strTemp.size() > 0)
//                {
//                    qstrSnpName = strTemp;
//                }
//                else
//                {
//                    qDebug() << "Failed to get notary name for this notary ID: " << QString::fromStdString(str_snp_server_id);
//                }
//            }
//            else
//            {
////              qDebug() << "Failed to get notary ID for this index: " << QString::fromStdString(str_index);
//            }
//        }
//        else
//        {
////            qDebug() << "This index is NOT paired or pairing yet: str_index: " << QString::fromStdString(str_index);
//        }
//        // ------------------------------------------------------------------------
//        if (qstrServerId.isEmpty())
//        {
//            QTreeWidgetItem * pNodeItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(qstrSnpName) );
//            items.append(pNodeItem);
//            pNodeItem->setExpanded(false);
//            const QString qstrIconPath(bConnected ? ":/icons/icons/green_dot.png" : ":/icons/icons/red_dot.png");
//            pNodeItem->setIcon(0, QIcon(qstrIconPath));
//            pNodeItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

//            pNodeItem->setData(0, Qt::UserRole+0, QVariant(AC_NODE_TYPE_SNP_NOTARY));
//            pNodeItem->setData(0, Qt::UserRole+1, QVariant(ACTIVITY_TREE_HEADER_SNP));

//        }
//        else // We know for this paired node, there's a server Id.
//        {
//            // So let's see if there are any accounts for that server Id.
//            mapOfMapIDName::const_iterator ci_paired = bigMapPairedAccounts.find(qstrServerId);

//            if (ci_paired != bigMapPairedAccounts.end() && (0 == qstrServerId.compare(ci_paired.key()))) // Found some.
//            {
////                mapOfMapIDName bigMapAccountsByTLA;
////                const mapIDName & mapAccounts = ci_paired.value();
////                const std::string str_server_id = qstrServerId.toStdString();
////                qstrSnpName = QString::fromStdString(opentxs::OT::App().API().Exec().GetServer_Name(str_server_id));
////                // ------------------------------------------------------
////                this->GetAccountsByTLAFromMap(mapAccounts, // input
////                                              mapTLAByAccountId, // input
////                                              bigMapAccountsByTLA); // output

////                mapOfMapIDName::const_iterator ci_acct_TLA = bigMapAccountsByTLA.begin();

////                bool bServerHasBeenShownAlready = false;

////                while (ci_acct_TLA != bigMapAccountsByTLA.end())
////                {

////                    const QString qstrTLA              = ci_acct_TLA.key();
////                    const mapIDName & mapAccountsByTLA = ci_acct_TLA.value();
////                    // ------------------------------------------------------

////                    // FOR EACH TLA:

////                    if (!bServerHasBeenShownAlready &&
////                            bigMapAccountsByTLA.size() > 1)  // If there are multiple TLAs
////                    {
////                        // ------------------
////                        QString qstrTempSNPName;
////                        if (!bServerHasBeenShownAlready) {
////                            qstrTempSNPName = qstrSnpName;
////                            //bServerHasBeenShownAlready = true;
////                        }
////                        // ------------------
////                        QTreeWidgetItem * pPairedItem = new QTreeWidgetItem((QTreeWidget *)nullptr,
////                                                                            QStringList(qstrTempSNPName) << QString("") << QString(""));
////                        items.append(pPairedItem);
////                        pPairedItem->setExpanded(true);

//////                      if (!bServerHasBeenShownAlready)
////                        {
////                            bServerHasBeenShownAlready = true;
////                            const QString qstrIconPath = QString(bConnected ? ":/icons/icons/green_dot.png" : ":/icons/icons/red_dot.png");
////                            pPairedItem->setIcon(0, QIcon(qstrIconPath));
////                            if (bConnected)
////                            {
////                                //pPairedItem->setTextColor(0, Qt::black);
////                                //pPairedItem->setTextColor(1, Qt::black);
////                            }
////                            else
////                            {
////                                pPairedItem->setTextColor(0, Qt::gray);
////                                pPairedItem->setTextColor(1, Qt::gray);
////                            }
////                            pPairedItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
////                        }

////                        pPairedItem->setData(0, Qt::UserRole+0, QVariant(AC_NODE_TYPE_SNP_NOTARY));
////                        pPairedItem->setData(0, Qt::UserRole+1, QVariant(ACTIVITY_TREE_HEADER_SNP));
////                        if (!qstrServerId.isEmpty()) {
////                            pPairedItem->setData(0, Qt::UserRole+5, QVariant(qstrServerId));
////                        }
////                    }
////                    // -------
////                    //const mapIDName & mapAccountsByTLA = ci_acct_TLA.value();

////                    mapIDName::const_iterator ci_accounts = mapAccountsByTLA.begin();

////                    while (ci_accounts != mapAccountsByTLA.end())
////                    {
////                        const QString qstrAccountId = ci_accounts.key();

////                        qDebug() << "DEBUGGING   qstrAccountId: " << qstrAccountId;


////                        const std::string str_acct_id = qstrAccountId.toStdString();
////                        const std::string str_asset_id = opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID(str_acct_id);
////                        const int64_t lBalance = opentxs::OT::App().API().Exec().GetAccountWallet_Balance(str_acct_id);
////                        const std::string str_formatted_amount = opentxs::OT::App().API().Exec().FormatAmountWithoutSymbol(str_asset_id, lBalance);
////    //                  const std::string str_formatted_amount = opentxs::OT::App().API().Exec().FormatAmount(str_asset_id, lBalance);
////                        const QString qstrFormattedAmount = QString::fromStdString(str_formatted_amount);

////                        const std::string asset_contract = opentxs::OT::App().API().Exec().GetAssetType_Contract(str_asset_id);
////                        opentxs::proto::UnitDefinition contractProto = opentxs::proto::StringToProto<opentxs::proto::UnitDefinition>
////                                                                        (opentxs::String(asset_contract));

////                        const std::string str_symbol = (contractProto.has_currency() && contractProto.has_symbol()) ? contractProto.symbol() : "";
////                        const QString qstrSymbol(QString::fromStdString(str_symbol));
////                        const QString qstrAcctName = QString("     %1").arg(QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_Name(str_acct_id)));
////                        const QString qstrAssetTypeId = QString::fromStdString(str_asset_id);
////                        //const QString qstrTLA = QString::fromStdString(opentxs::OT::App().API().Exec().GetCurrencyTLA(str_asset_id));
////                        // ------------------
////                        QTreeWidgetItem * pSubItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(qstrAcctName) << qstrSymbol << qstrFormattedAmount << qstrTLA);
////                        items.append(pSubItem);
////                        pSubItem->setExpanded(false);
////                        //pSubItem->setIcon(0, QIcon(":/icons/icons/red_dot.png"));
////                        if (bConnected)
////                        {
////                            //pSubItem->setTextColor(0, Qt::black);
////                            //pSubItem->setTextColor(1, Qt::black);
////                        }
////                        else
////                        {
////                            pSubItem->setTextColor(0, Qt::gray);
////                            pSubItem->setTextColor(1, Qt::gray);
////                        }
////                        pSubItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

////                        //todo
////                        pSubItem->setData(0, Qt::UserRole+0, QVariant(AC_NODE_TYPE_SNP_ACCOUNT));
////                        pSubItem->setData(0, Qt::UserRole+1, QVariant(ACTIVITY_TREE_HEADER_SNP));

////                        if (!qstrTLA.isEmpty()) {
////                            pSubItem->setData(0, Qt::UserRole+2, QVariant(qstrTLA));
////                        }
////                        if (!qstrAssetTypeId.isEmpty()) {
////                            pSubItem->setData(0, Qt::UserRole+3, QVariant(qstrAssetTypeId));
////                        }
////                        if (!qstrAccountId.isEmpty()) {
////                            pSubItem->setData(0, Qt::UserRole+4, QVariant(qstrAccountId));
////                        }

////                        if (!qstrServerId.isEmpty()) {
////                            pSubItem->setData(0, Qt::UserRole+5, QVariant(qstrServerId));
////                        }



////                        ++ci_accounts;
////                    }
////                    // ------------------------------------------------------
////                    ci_acct_TLA++;
////                }


//                bool bServerHasBeenShownAlready = false;
//                mapOfMapIDName bigMapAccountsByTLA;
//                const QString qstrServerId = ci_paired.key();
//                const mapIDName & mapAccounts = ci_paired.value();
//                const std::string str_server_id = qstrServerId.toStdString();
//                const bool bConnected = opentxs::OT::App().API().Exec().CheckConnection(str_server_id);

//                QString qstrServerName = QString::fromStdString(opentxs::OT::App().API().Exec().GetServer_Name(str_server_id));


//                // ------------------------------------------------------
//                this->GetAccountsByTLAFromMap(mapAccounts, // input
//                                              mapTLAByAccountId, // input
//                                              bigMapAccountsByTLA); // output

//                if (!bServerHasBeenShownAlready &&
//                        bigMapAccountsByTLA.size() > 1)  // If there are multiple TLAs, then we HAVE to show the server above them on its own line.
//                {
//                    QString qstrTempSNPName;
//                    if (!bServerHasBeenShownAlready) {
//                        qstrTempSNPName = qstrServerName;
//                    }
//                    // ------------------
//                    QTreeWidgetItem * pPairedItem = new QTreeWidgetItem((QTreeWidget *)nullptr,
//                                                                        QStringList(qstrTempSNPName) << QString("") << QString(""));
//                    items.append(pPairedItem);
//                    pPairedItem->setExpanded(true);

//                    const QString qstrIconPath = QString(bConnected ? ":/icons/icons/green_dot.png" : ":/icons/icons/red_dot.png");
//                    pPairedItem->setIcon(0, QIcon(qstrIconPath));
//                    if (bConnected)
//                    {
//                        //pPairedItem->setTextColor(0, Qt::black);
//                        //pPairedItem->setTextColor(1, Qt::black);
//                    }
//                    else
//                    {
//                        pPairedItem->setTextColor(0, Qt::gray);
//                        pPairedItem->setTextColor(1, Qt::gray);
//                    }
//                    pPairedItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

//                    pPairedItem->setData(0, Qt::UserRole+0, QVariant(AC_NODE_TYPE_SNP_NOTARY));
//                    pPairedItem->setData(0, Qt::UserRole+1, QVariant(ACTIVITY_TREE_HEADER_SNP));
//                    if (!qstrServerId.isEmpty()) {
//                        pPairedItem->setData(0, Qt::UserRole+5, QVariant(qstrServerId));
//                    }

//                    bServerHasBeenShownAlready = true;
//                }


//                mapOfMapIDName::const_iterator ci_acct_TLA = bigMapAccountsByTLA.begin();

//                while (ci_acct_TLA != bigMapAccountsByTLA.end())
//                {

//                    // FOR EACH TLA:

//                    const QString qstrTLA              = ci_acct_TLA.key();
//                    const mapIDName & mapAccountsByTLA = ci_acct_TLA.value();
//                    // ------------------------------------------------------
//                    mapIDName::const_iterator ci_accounts = mapAccountsByTLA.begin();

//                    while (ci_accounts != mapAccountsByTLA.end())
//                    {
//                            QString qstrDisplayName;
//                            bool bRollingUp = true;
//                            bool bAccountNamesExist = false;
//                            int64_t lTotalBalance = 0;
//                            int64_t lDisplayBalance = 0;
//                            std::string str_acct_id;
//                            std::string str_asset_id;
//                            std::string str_asset_id_for_formatting;
//                            std::string str_formatted_amount;
//                            std::string str_symbol_for_display;
//                            QString qstrAssetTypeId; // Used when known.
//                            QString qstrAccountId;

//                            bool bAllAcctsForTLAHaveSameUnitTypeId = true;

//                            for (mapIDName::const_iterator ci_accounts2 = mapAccountsByTLA.begin();
//                                 ci_accounts2 != mapAccountsByTLA.end(); ci_accounts2++)
//                            {

//                                // TODO: set bAllAcctsForTLAHaveSameUnitTypeId to false in the case
//                                // where the current iteration's unit type doesn't match the first
//                                // iteration's unit type.
//                                // Until that is coded, for now, set it to false:
//                                bAllAcctsForTLAHaveSameUnitTypeId = false;

//                                const QString qstrCurrentAccountId = ci_accounts2.key();
//                                const std::string str_current_acct_id = qstrCurrentAccountId.toStdString();
//                                str_asset_id_for_formatting = opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID(str_current_acct_id);
//                                const QString qstrCurrentAcctName = QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_Name(str_current_acct_id));
//                                const int64_t lCurrentBalance = opentxs::OT::App().API().Exec().GetAccountWallet_Balance(str_current_acct_id);

//                                if (str_symbol_for_display.empty())
//                                {
//                                    const std::string asset_contract = opentxs::OT::App().API().Exec().GetAssetType_Contract(str_asset_id_for_formatting);
//                                    opentxs::proto::UnitDefinition contractProto = opentxs::proto::StringToProto<opentxs::proto::UnitDefinition>
//                                                                                    (opentxs::String(asset_contract));
//                                    const std::string str_symbol = (contractProto.has_currency() && contractProto.has_symbol()) ? contractProto.symbol() : "";
//                                    str_symbol_for_display = str_symbol;
//                                }

//                                if (lCurrentBalance < 0) { // We don't include negative accounts (issuer accounts) on this screen.
//                                    continue;
//                                }
//                                lTotalBalance += lCurrentBalance; // Total balance for all non-negative accounts
//                                if (!qstrCurrentAcctName.isEmpty()) {
//                                    bAccountNamesExist = true;
//                                    bRollingUp = false;
//                                }
//                            }

//                            // In this case display all the accounts (for this TLA) on a single line.
//                            // Else multiple lines.
//                            //
//                            if (bRollingUp) // Rollup!
//                            {
//                                lDisplayBalance = lTotalBalance;

//                                qstrDisplayName = bServerHasBeenShownAlready ? QString("") : qstrServerName;

//                                // If we're rolling up, we can still set CERTAIN IDs, IF they are known.
//                                // (From case to case). For example, the TLA is already known for all of them, and the server ID.
//                                // Also for example, if there's only one account, we have an account ID AND an asset type ID.
//                                //
//                                if (1 == mapAccountsByTLA.size()) {
//                                    bAllAcctsForTLAHaveSameUnitTypeId = true;
//                                    qstrAccountId = ci_accounts.key();
//                                }

//                                // Here if we knew that all the accounts for a given TLA were all also the
//                                // same unit type ID, we could set the unit type for any of them as the official
//                                // unit type for this line on the tree.

//                                if (bAllAcctsForTLAHaveSameUnitTypeId) {
//                                    qstrAssetTypeId = QString::fromStdString(str_asset_id_for_formatting);
//                                }
//                            }
//                            // Else multiple lines.
//                            //
//                            else
//                            {
//                                // The server has not yet been shown, and we're NOT rolling up.
//                                // Therefore we have to show the server on its own line before
//                                // showing the accounts.
//                                if (!bServerHasBeenShownAlready)
//                                {
//                                    QTreeWidgetItem * pPairedItem = new QTreeWidgetItem((QTreeWidget *)nullptr,
//                                                                                        QStringList(qstrServerName) << QString("") << QString(""));
//                                    items.append(pPairedItem);
//                                    pPairedItem->setExpanded(true);

//                                    const QString qstrIconPath = QString(bConnected ? ":/icons/icons/green_dot.png" : ":/icons/icons/red_dot.png");
//                                    pPairedItem->setIcon(0, QIcon(qstrIconPath));
//                                    if (bConnected)
//                                    {
//                                        //pPairedItem->setTextColor(0, Qt::black);
//                                        //pPairedItem->setTextColor(1, Qt::black);
//                                    }
//                                    else
//                                    {
//                                        pPairedItem->setTextColor(0, Qt::gray);
//                                        pPairedItem->setTextColor(1, Qt::gray);
//                                    }
//                                    pPairedItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

//                                    pPairedItem->setData(0, Qt::UserRole+0, QVariant(AC_NODE_TYPE_SNP_NOTARY));
//                                    pPairedItem->setData(0, Qt::UserRole+1, QVariant(ACTIVITY_TREE_HEADER_SNP));
//                                    if (!qstrServerId.isEmpty()) {
//                                        pPairedItem->setData(0, Qt::UserRole+5, QVariant(qstrServerId));
//                                    }

//                                    bServerHasBeenShownAlready = true;
//                                }
//                                // --------------------------------------------------
//                                qstrAccountId = ci_accounts.key(); // Because we're on the first one.
//                                str_acct_id = qstrAccountId.toStdString();
//                                str_asset_id = opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID(str_acct_id);
//                                str_asset_id_for_formatting = str_asset_id;
//                                qstrAssetTypeId = QString::fromStdString(str_asset_id);
//                                lDisplayBalance = opentxs::OT::App().API().Exec().GetAccountWallet_Balance(str_acct_id);

//                                const QString qstrCurrentAcctName = QString("     %1")
//                                        .arg(QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_Name(str_acct_id)));

//                                qstrDisplayName = qstrCurrentAcctName;
//                                // ------------------
//                                const std::string asset_contract = opentxs::OT::App().API().Exec().GetAssetType_Contract(str_asset_id);
//                                opentxs::proto::UnitDefinition contractProto = opentxs::proto::StringToProto<opentxs::proto::UnitDefinition>
//                                                                                (opentxs::String(asset_contract));
//                                const std::string str_symbol = (contractProto.has_currency() && contractProto.has_symbol()) ? contractProto.symbol() : "";
//                                str_symbol_for_display = str_symbol;
//                            }
//                            // ------------------
//                            if (lDisplayBalance < 0) {
//                                ++ci_accounts;
//                                continue;
//                            }
//                            // ------------------
//                            str_formatted_amount = opentxs::OT::App().API().Exec().FormatAmountWithoutSymbol(str_asset_id_for_formatting, lDisplayBalance);
//                            const QString qstrFormattedAmount = QString::fromStdString(str_formatted_amount);
//                            // ------------------
//                            const QString qstrSymbol(QString::fromStdString(str_symbol_for_display)); // May be empty
//                            // ------------------
//                            QTreeWidgetItem * pHostedItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(qstrDisplayName) << qstrSymbol << qstrFormattedAmount << qstrTLA);
//                            items.append(pHostedItem);
//                            pHostedItem->setExpanded(false);

//                            pHostedItem->setData(0, Qt::UserRole+0, QVariant(AC_NODE_TYPE_SNP_ACCOUNT));
//                            pHostedItem->setData(0, Qt::UserRole+1, QVariant(ACTIVITY_TREE_HEADER_SNP));
//                            if (!qstrTLA.isEmpty()) {
//                                pHostedItem->setData(0, Qt::UserRole+2, QVariant(qstrTLA));
//                            }
//                            if (!qstrAssetTypeId.isEmpty()) {
//                                pHostedItem->setData(0, Qt::UserRole+3, QVariant(qstrAssetTypeId));
//                            }
//                            if (!qstrAccountId.isEmpty()) {
//                                pHostedItem->setData(0, Qt::UserRole+4, QVariant(qstrAccountId));
//                            }
//                            if (!qstrServerId.isEmpty()) {
//                                pHostedItem->setData(0, Qt::UserRole+5, QVariant(qstrServerId));
//                            }
//                            // --------------------------------------------
//                            if (!bServerHasBeenShownAlready) {
//                                const QString qstrIconPath = QString(bConnected ? ":/icons/icons/green_dot.png" : ":/icons/icons/red_dot.png");
//                                pHostedItem->setIcon(0, QIcon(qstrIconPath));
//                            }

//                            if (bConnected)
//                            {
//            //                    pHostedItem->setTextColor(0, Qt::black);
//            //                    pHostedItem->setTextColor(1, Qt::black);
//                            }
//                            else
//                            {
//                                pHostedItem->setTextColor(0, Qt::gray);
//                                pHostedItem->setTextColor(1, Qt::gray);
//                            }
//                            pHostedItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

//                            bServerHasBeenShownAlready = true;

//                            if (bRollingUp) {
//                                break; // Done, for this TLA anyway.
//                            }

//                        ++ci_accounts;
//                    }
//                    // ------------------------------------------------------
//                    ci_acct_TLA++;
//                }

//            }
//            else // found no accounts for this server ID.
//            {
//                QTreeWidgetItem * pNodeItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(qstrSnpName) << QString("") << QString(""));
//                items.append(pNodeItem);
//                pNodeItem->setExpanded(false);
//                const QString qstrIconPath(bConnected ? ":/icons/icons/green_dot.png" : ":/icons/icons/red_dot.png");
//                pNodeItem->setIcon(0, QIcon(qstrIconPath));
//                pNodeItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

//                pNodeItem->setData(0, Qt::UserRole+0, QVariant(AC_NODE_TYPE_SNP_NOTARY));
//                pNodeItem->setData(0, Qt::UserRole+1, QVariant(ACTIVITY_TREE_HEADER_SNP));
//                if (!qstrServerId.isEmpty()) {
//                    pNodeItem->setData(0, Qt::UserRole+5, QVariant(qstrServerId));
//                }
//            }
//        }
//    } // For each paired or pairing node.
//    // ------------------------------------------------------------------------

//    mapOfMapIDName bigMapAccounts;
//    mapTLAByAccountId.clear();
////  mapIDName mapTLAByAccountId;

//    this->GetAccountIdMapsByServerId(bigMapAccounts, false, // false means, only give me non-paired servers.
//                                     &mapTLAByAccountId);

//    if (bigMapAccounts.size() > 0)
//    {
//        // ----------------------------------------
//        //QTreeWidgetItem * pHostedAccountsItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(tr("Your Stash Node")));
//        // TODO: put this back. I only changed it for the video.
//        QTreeWidgetItem * pHostedAccountsItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(tr("Hosted Accounts")));
//        items.append(pHostedAccountsItem);
//        pHostedAccountsItem->setExpanded(true);
//        pHostedAccountsItem->setData(0, Qt::UserRole+0, QVariant(AC_NODE_TYPE_HEADER));
//        pHostedAccountsItem->setData(0, Qt::UserRole+1, QVariant(ACTIVITY_TREE_HEADER_HOSTED));
//        pHostedAccountsItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
//        pHostedAccountsItem->setTextColor(0, Qt::white);
//        pHostedAccountsItem->setTextColor(1, Qt::white);
//        pHostedAccountsItem->setBackgroundColor(0, Qt::gray);
//        pHostedAccountsItem->setBackgroundColor(1, Qt::gray);
//        pHostedAccountsItem->setBackgroundColor(2, Qt::gray);
//        pHostedAccountsItem->setBackgroundColor(3, Qt::gray);
//        pHostedAccountsItem->setFlags(pHostedAccountsItem->flags()&~Qt::ItemIsSelectable);
//        // ----------------------------------------
//        // From Vaage:
//        // Retrieve a list of hosted accounts for each server.
//        // For each server, if the list is empty, skip it.
//        // If the list only contains one account, display
//        // the balance directly on the same line as the server name.
//        // Only give it a separate line if the user has gone
//        // out of his way to name the account.
//        // If the user has named it, or if there are multiple accounts
//        // for that server, then display the balances below that server
//        // on the tree.

//        mapOfMapIDName::const_iterator ci_servers = bigMapAccounts.begin();

//        while (ci_servers != bigMapAccounts.end()) {
//            bool bServerHasBeenShownAlready = false;
//            mapOfMapIDName bigMapAccountsByTLA;
//            const QString qstrServerId = ci_servers.key();
//            const mapIDName & mapAccounts = ci_servers.value();
//            const std::string str_server_id = qstrServerId.toStdString();
//            const bool bConnected = opentxs::OT::App().API().Exec().CheckConnection(str_server_id);

//            QString qstrServerName = QString::fromStdString(opentxs::OT::App().API().Exec().GetServer_Name(str_server_id));


//            // ------------------------------------------------------
//            this->GetAccountsByTLAFromMap(mapAccounts, // input
//                                          mapTLAByAccountId, // input
//                                          bigMapAccountsByTLA); // output




//            if (!bServerHasBeenShownAlready &&
//                    bigMapAccountsByTLA.size() > 1)  // If there are multiple TLAs, then we HAVE to show the server above them on its own line.
//            {
//                QString qstrTempSNPName;
//                if (!bServerHasBeenShownAlready) {
//                    qstrTempSNPName = qstrServerName;
//                }
//                // ------------------
//                QTreeWidgetItem * pPairedItem = new QTreeWidgetItem((QTreeWidget *)nullptr,
//                                                                    QStringList(qstrTempSNPName) << QString("") << QString(""));
//                items.append(pPairedItem);
//                pPairedItem->setExpanded(true);

//                const QString qstrIconPath = QString(bConnected ? ":/icons/icons/green_dot.png" : ":/icons/icons/red_dot.png");
//                pPairedItem->setIcon(0, QIcon(qstrIconPath));
//                if (bConnected)
//                {
//                    //pPairedItem->setTextColor(0, Qt::black);
//                    //pPairedItem->setTextColor(1, Qt::black);
//                }
//                else
//                {
//                    pPairedItem->setTextColor(0, Qt::gray);
//                    pPairedItem->setTextColor(1, Qt::gray);
//                }
//                pPairedItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

//                pPairedItem->setData(0, Qt::UserRole+0, QVariant(AC_NODE_TYPE_HOSTED_NOTARY));
//                pPairedItem->setData(0, Qt::UserRole+1, QVariant(ACTIVITY_TREE_HEADER_HOSTED));
//                if (!qstrServerId.isEmpty()) {
//                    pPairedItem->setData(0, Qt::UserRole+5, QVariant(qstrServerId));
//                }

//                bServerHasBeenShownAlready = true;
//            }


//            mapOfMapIDName::const_iterator ci_acct_TLA = bigMapAccountsByTLA.begin();

//            while (ci_acct_TLA != bigMapAccountsByTLA.end())
//            {

//                // FOR EACH TLA:

//                const QString qstrTLA              = ci_acct_TLA.key();
//                const mapIDName & mapAccountsByTLA = ci_acct_TLA.value();
//                // ------------------------------------------------------
//                mapIDName::const_iterator ci_accounts = mapAccountsByTLA.begin();

//                while (ci_accounts != mapAccountsByTLA.end())
//                {
//                        QString qstrDisplayName;
//                        bool bRollingUp = true;
//                        bool bAccountNamesExist = false;
//                        int64_t lTotalBalance = 0;
//                        int64_t lDisplayBalance = 0;
//                        std::string str_acct_id;
//                        std::string str_asset_id;
//                        std::string str_asset_id_for_formatting;
//                        std::string str_formatted_amount;
//                        std::string str_symbol_for_display;
//                        QString qstrAssetTypeId; // Used when known.
//                        QString qstrAccountId;

//                        bool bAllAcctsForTLAHaveSameUnitTypeId = true;

//                        for (mapIDName::const_iterator ci_accounts2 = mapAccountsByTLA.begin();
//                             ci_accounts2 != mapAccountsByTLA.end(); ci_accounts2++)
//                        {

//                            // TODO: set bAllAcctsForTLAHaveSameUnitTypeId to false in the case
//                            // where the current iteration's unit type doesn't match the first
//                            // iteration's unit type.
//                            // Until that is coded, for now, set it to false:
//                            bAllAcctsForTLAHaveSameUnitTypeId = false;

//                            const QString qstrCurrentAccountId = ci_accounts2.key();
//                            const std::string str_current_acct_id = qstrCurrentAccountId.toStdString();
//                            str_asset_id_for_formatting = opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID(str_current_acct_id);
//                            const QString qstrCurrentAcctName = QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_Name(str_current_acct_id));
//                            const int64_t lCurrentBalance = opentxs::OT::App().API().Exec().GetAccountWallet_Balance(str_current_acct_id);

//                            if (str_symbol_for_display.empty())
//                            {
//                                const std::string asset_contract = opentxs::OT::App().API().Exec().GetAssetType_Contract(str_asset_id_for_formatting);
//                                opentxs::proto::UnitDefinition contractProto = opentxs::proto::StringToProto<opentxs::proto::UnitDefinition>
//                                                                                (opentxs::String(asset_contract));
//                                const std::string str_symbol = (contractProto.has_currency() && contractProto.has_symbol()) ? contractProto.symbol() : "";
//                                str_symbol_for_display = str_symbol;
//                            }

//                            if (lCurrentBalance < 0) { // We don't include negative accounts (issuer accounts) on this screen.
//                                continue;
//                            }
//                            lTotalBalance += lCurrentBalance; // Total balance for all non-negative accounts
//                            if (!qstrCurrentAcctName.isEmpty()) {
//                                bAccountNamesExist = true;
//                                bRollingUp = false;
//                            }
//                        }

//                        // In this case display all the accounts (for this TLA) on a single line.
//                        // Else multiple lines.
//                        //
//                        if (bRollingUp) // Rollup!
//                        {
//                            lDisplayBalance = lTotalBalance;

//                            qstrDisplayName = bServerHasBeenShownAlready ? QString("") : qstrServerName;

//                            // If we're rolling up, we can still set CERTAIN IDs, IF they are known.
//                            // (From case to case). For example, the TLA is already known for all of them, and the server ID.
//                            // Also for example, if there's only one account, we have an account ID AND an asset type ID.
//                            //
//                            if (1 == mapAccountsByTLA.size()) {
//                                bAllAcctsForTLAHaveSameUnitTypeId = true;
//                                qstrAccountId = ci_accounts.key();
//                            }

//                            // Here if we knew that all the accounts for a given TLA were all also the
//                            // same unit type ID, we could set the unit type for any of them as the official
//                            // unit type for this line on the tree.

//                            if (bAllAcctsForTLAHaveSameUnitTypeId) {
//                                qstrAssetTypeId = QString::fromStdString(str_asset_id_for_formatting);
//                            }


//                        }
//                        // Else multiple lines.
//                        //
//                        else
//                        {
//                            // The server has not yet been shown, and we're NOT rolling up.
//                            // Therefore we have to show the server on its own line before
//                            // showing the accounts.
//                            if (!bServerHasBeenShownAlready)
//                            {
//                                QTreeWidgetItem * pPairedItem = new QTreeWidgetItem((QTreeWidget *)nullptr,
//                                                                                    QStringList(qstrServerName) << QString("") << QString(""));
//                                items.append(pPairedItem);
//                                pPairedItem->setExpanded(true);

//                                const QString qstrIconPath = QString(bConnected ? ":/icons/icons/green_dot.png" : ":/icons/icons/red_dot.png");
//                                pPairedItem->setIcon(0, QIcon(qstrIconPath));
//                                if (bConnected)
//                                {
//                                    //pPairedItem->setTextColor(0, Qt::black);
//                                    //pPairedItem->setTextColor(1, Qt::black);
//                                }
//                                else
//                                {
//                                    pPairedItem->setTextColor(0, Qt::gray);
//                                    pPairedItem->setTextColor(1, Qt::gray);
//                                }
//                                pPairedItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

//                                pPairedItem->setData(0, Qt::UserRole+0, QVariant(AC_NODE_TYPE_HOSTED_NOTARY));
//                                pPairedItem->setData(0, Qt::UserRole+1, QVariant(ACTIVITY_TREE_HEADER_HOSTED));
//                                if (!qstrServerId.isEmpty()) {
//                                    pPairedItem->setData(0, Qt::UserRole+5, QVariant(qstrServerId));
//                                }

//                                bServerHasBeenShownAlready = true;
//                            }
//                            // --------------------------------------------------
//                            qstrAccountId = ci_accounts.key(); // Because we're on the first one.
//                            str_acct_id = qstrAccountId.toStdString();
//                            str_asset_id = opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID(str_acct_id);
//                            str_asset_id_for_formatting = str_asset_id;
//                            qstrAssetTypeId = QString::fromStdString(str_asset_id);
//                            lDisplayBalance = opentxs::OT::App().API().Exec().GetAccountWallet_Balance(str_acct_id);

//                            const QString qstrCurrentAcctName = QString("     %1")
//                                    .arg(QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_Name(str_acct_id)));

//                            qstrDisplayName = qstrCurrentAcctName;
//                            // ------------------
//                            const std::string asset_contract = opentxs::OT::App().API().Exec().GetAssetType_Contract(str_asset_id);
//                            opentxs::proto::UnitDefinition contractProto = opentxs::proto::StringToProto<opentxs::proto::UnitDefinition>
//                                                                            (opentxs::String(asset_contract));
//                            const std::string str_symbol = (contractProto.has_currency() && contractProto.has_symbol()) ? contractProto.symbol() : "";
//                            str_symbol_for_display = str_symbol;
//                        }
//                        // ------------------
//                        if (lDisplayBalance < 0) {
//                            ++ci_accounts;
//                            continue;
//                        }
//                        // ------------------
//                        str_formatted_amount = opentxs::OT::App().API().Exec().FormatAmountWithoutSymbol(str_asset_id_for_formatting, lDisplayBalance);
//                        const QString qstrFormattedAmount = QString::fromStdString(str_formatted_amount);
//                        // ------------------
//                        const QString qstrSymbol(QString::fromStdString(str_symbol_for_display)); // May be empty
//                        // ------------------
//                        QTreeWidgetItem * pHostedItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(qstrDisplayName) << qstrSymbol << qstrFormattedAmount << qstrTLA);
//                        items.append(pHostedItem);
//                        pHostedItem->setExpanded(false);

//                        pHostedItem->setData(0, Qt::UserRole, QVariant(AC_NODE_TYPE_HOSTED_ACCOUNT));
//                        pHostedItem->setData(0, Qt::UserRole+1, QVariant(ACTIVITY_TREE_HEADER_HOSTED));
//                        if (!qstrTLA.isEmpty()) {
//                            pHostedItem->setData(0, Qt::UserRole+2, QVariant(qstrTLA));
//                        }
//                        if (!qstrAssetTypeId.isEmpty()) {
//                            pHostedItem->setData(0, Qt::UserRole+3, QVariant(qstrAssetTypeId));
//                        }
//                        if (!qstrAccountId.isEmpty()) {
//                            pHostedItem->setData(0, Qt::UserRole+4, QVariant(qstrAccountId));
//                        }
//                        if (!qstrServerId.isEmpty()) {
//                            pHostedItem->setData(0, Qt::UserRole+5, QVariant(qstrServerId));
//                        }
//                        // --------------------------------------------
//                        if (!bServerHasBeenShownAlready) {
//                            const QString qstrIconPath = QString(bConnected ? ":/icons/icons/green_dot.png" : ":/icons/icons/red_dot.png");
//                            pHostedItem->setIcon(0, QIcon(qstrIconPath));
//                        }

//                        if (bConnected)
//                        {
//        //                    pHostedItem->setTextColor(0, Qt::black);
//        //                    pHostedItem->setTextColor(1, Qt::black);
//                        }
//                        else
//                        {
//                            pHostedItem->setTextColor(0, Qt::gray);
//                            pHostedItem->setTextColor(1, Qt::gray);
//                        }
//                        pHostedItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

//                        bServerHasBeenShownAlready = true;

//                        if (bRollingUp) {
//                            break; // Done, for this TLA anyway.
//                        }

//                    ++ci_accounts;
//                }
//                // ------------------------------------------------------
//                ci_acct_TLA++;
//            }

//            ++ci_servers;
//        }



//    } // The hosted accounts
//    // ----------------------------------------

////    {
////    QTreeWidgetItem * pHostedItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList("Ruritarian Reserve") << "BTC 1.686 46731");
////    items.append(pHostedItem);
////    pHostedItem->setExpanded(false);
////    pHostedItem->setIcon(0, QIcon(":/icons/icons/red_dot.png"));
////    pHostedItem->setTextColor(0, Qt::gray);
////    pHostedItem->setTextColor(1, Qt::gray);
////    pHostedItem->setData(0, Qt::UserRole, QVariant(0));
////    pHostedItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
////    }
////    {
////    QTreeWidgetItem * pHostedItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList("Nakamoto Market") << "BTC 0.000 00000");
////    items.append(pHostedItem);
////    pHostedItem->setExpanded(false);
////    pHostedItem->setIcon(0, QIcon(":/icons/icons/green_dot.png"));
////    pHostedItem->setData(0, Qt::UserRole, QVariant(0));
////    pHostedItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
////    }

//    mapIDName mapContacts;

//    if (MTContactHandler::getInstance()->GetOpentxsContacts(mapContacts))
//    {
//        // ----------------------------------------
//        QTreeWidgetItem * pContactsItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(tr("Contacts")));
//        items.append(pContactsItem);
//        pContactsItem->setExpanded(true);
//        pContactsItem->setData(0, Qt::UserRole+0, QVariant(AC_NODE_TYPE_HEADER));
//        pContactsItem->setData(0, Qt::UserRole+1, QVariant(ACTIVITY_TREE_HEADER_CONTACTS));
//        pContactsItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
//        pContactsItem->setTextColor(0, Qt::white);
//        pContactsItem->setTextColor(1, Qt::white);
//        pContactsItem->setBackgroundColor(0, Qt::gray);
//        pContactsItem->setBackgroundColor(1, Qt::gray);
//        pContactsItem->setBackgroundColor(2, Qt::gray);
//        pContactsItem->setBackgroundColor(3, Qt::gray);
//        pContactsItem->setFlags(pContactsItem->flags()&~Qt::ItemIsSelectable);
//        // ----------------------------------------
//        for (mapIDName::iterator ii = mapContacts.begin(); ii != mapContacts.end(); ii++)
//        {
//            QString qstrContactID   = ii.key();
//            QString qstrTemp = ii.value();
//            QString qstrContactName = QString("CONTACT_NAME_EMPTY");
//            if (!qstrTemp.isEmpty())
//            {
//                qstrContactName = qstrTemp;
//            }
//            // ------------------------------------
//            QTreeWidgetItem * pTopItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(qstrContactName));

//            pTopItem->setData(0, Qt::UserRole+0, QVariant(AC_NODE_TYPE_CONTACT));
//            pTopItem->setData(0, Qt::UserRole+1, QVariant(ACTIVITY_TREE_HEADER_CONTACTS));

//            if (!qstrContactID.isEmpty()) {
//                pTopItem->setData(0, Qt::UserRole+6, QVariant(qstrContactID));
//            }

//            pTopItem->setExpanded(true);
//            //pTopItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
//            items.append(pTopItem);
//        }
//        // ----------------------------------------
//        if (items.count() > 0)
//        {
//            pTreeWidgetAccounts->insertTopLevelItems(0, items);
////            pTreeWidgetAccounts->resizeColumnToContents(0);
//        }
//    }

//    pTreeWidgetAccounts->resizeColumnToContents(0);
//    pTreeWidgetAccounts->resizeColumnToContents(1);
//    pTreeWidgetAccounts->resizeColumnToContents(2);
//    pTreeWidgetAccounts->resizeColumnToContents(3);
//    // ----------------------------------------
//    // Make sure the same item that was selected before, is selected again.
//    // (If it still exists, which it probably does.)

//    QTreeWidgetItem * previous = pTreeWidgetAccounts->currentItem();

//    QTreeWidgetItem * pItemToSelect {nullptr};

//    // In this case, just select the first thing on the list.
//    if (    qstrCurrentTLA_.isEmpty()
//         && qstrCurrentNotary_.isEmpty()
//         && qstrCurrentAccount_.isEmpty()
//         && qstrCurrentContact_.isEmpty() )
//    {
////        qDebug() << "RefreshAccountTree: the strings were all empty\n";

//        if (pTreeWidgetAccounts->topLevelItemCount() > 0)
//        {
//            //qDebug() << "RefreshAccountTree: setting to 0th top level item. 1\n";

////            pItemToSelect = pTreeWidgetAccounts->topLevelItem(0);
//        }
//    }
//    else // Find the one that was selected before the refresh.
//    {
//        bool bFoundIt = false;

//        QTreeWidgetItemIterator it(pTreeWidgetAccounts);

////        qDebug() << "RefreshAccountTree: entering while loop...\n";

//        while (*it)
//        {
//            const int      nNodeType       = (*it)->data(0, Qt::UserRole+0).toInt();
//            const int      nHeader         = (*it)->data(0, Qt::UserRole+1).toInt();
//            const QVariant qvarTLA         = (*it)->data(0, Qt::UserRole+2);
//            const QVariant qvarAssetTypeId = (*it)->data(0, Qt::UserRole+3);
//            const QVariant qvarAccountId   = (*it)->data(0, Qt::UserRole+4);
//            const QVariant qvarServerId    = (*it)->data(0, Qt::UserRole+5);
//            const QVariant qvarContactId   = (*it)->data(0, Qt::UserRole+6);
//            // ------------------------------------------------------------
//            const QString  qstrTLA         = qvarTLA.isValid()         ? qvarTLA.toString()         : "";
//            const QString  qstrAssetTypeId = qvarAssetTypeId.isValid() ? qvarAssetTypeId.toString() : "";
//            const QString  qstrServerId    = qvarServerId.isValid()    ? qvarServerId.toString()    : "";
//            const QString  qstrAccountId   = qvarAccountId.isValid()   ? qvarAccountId.toString()   : "";
//            const QString  qstrContactId   = qvarContactId.isValid()   ? qvarContactId.toString()   : "";
//            // ------------------------------------------------------------
//            const bool bContactsMatch = !qstrContactId.isEmpty() && (0 == qstrContactId.compare(qstrCurrentContact_));
//            const bool bAccountsMatch = !qstrAccountId.isEmpty() && (0 == qstrAccountId.compare(qstrCurrentAccount_));
//            const bool bOtherMatch    = (   (0 == qstrTLA.compare(qstrCurrentTLA_ ))
//                                         && (0 == qstrServerId.compare(qstrCurrentNotary_))
//                                         && (0 == qstrAccountId.compare(qstrCurrentAccount_))
//                                         && (0 == qstrContactId.compare(qstrCurrentContact_))
//                                         );
//            if (bContactsMatch || bOtherMatch || bAccountsMatch)
//            {
////                qDebug() << "RefreshAccountTree: MATCHED.\n";
////
////                if (bContactsMatch)
////                    qDebug() << "RefreshAccountTree: Contacts MATCHED.\n";
////                if (bOtherMatch)
////                    qDebug() << "RefreshAccountTree: Other MATCHED.\n";
////                if (bAccountsMatch)
////                    qDebug() << "RefreshAccountTree: Accounts MATCHED.\n";

//                bFoundIt = true;
//                pItemToSelect = *it;

//                break;
//            }
//            else
//            {
////                qDebug() << "RefreshAccountTree: DID NOT match:";
////                qDebug() << "qstrTLA: " << qstrTLA << "qstrCurrentTLA_: " << qstrCurrentTLA_;
////                qDebug() << "qstrServerId: " << qstrServerId << "qstrCurrentNotary_: " << qstrCurrentNotary_;
////                qDebug() << "qstrAccountId: " << qstrAccountId << "qstrCurrentAccount_: " << qstrCurrentAccount_;
////                qDebug() << "qstrContactId: " << qstrContactId << "qstrCurrentContact_: " << qstrCurrentContact_ << "\n";
//            }
//            // ------------------------
//            ++it;
//        } //while
//        if (!bFoundIt)
//        {
////            if (pTreeWidgetAccounts->topLevelItemCount() > 0)
////            {
////                qDebug() << "RefreshAccountTree: setting to 0th top level item. 2\n";
////
////                pItemToSelect = pTreeWidgetAccounts->topLevelItem(0);
////            }
//        }
//    }
////  pTreeWidgetAccounts->expandAll();
//    // ----------------------------------------
//    pTreeWidgetAccounts->blockSignals(false);
//    // ----------------------------------------
////    if (nullptr == pItemToSelect)
////    {
////        pItemToSelect = pTreeWidgetAccounts->topLevelItem(0);
////    }
//    // ----------------------------------------
//    if (nullptr != pItemToSelect)
//    {
//        pTreeWidgetAccounts->setCurrentItem(pItemToSelect);
////        on_treeWidgetAccounts_currentItemChanged(pItemToSelect, previous);
//    }
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





//resume
//void Activity::on_listWidgetConversations_currentRowChanged


void Activity::on_listWidgetConversations_currentRowChanged(int currentRow)
{
//    if ((-1) == currentRow)
//        ui->listWidgetConversation->setEnabled(false);
//    else
//        ui->listWidgetConversation->setEnabled(true);
    // -------------------
    RefreshConversationDetails(currentRow);
    // -------------------
//    ui->tableViewConversation->resizeColumnsToContents();
//    ui->tableViewConversation->resizeRowsToContents();
//    ui->tableViewConversation->horizontalHeader()->setStretchLastSection(true);


//    auto qs = ui->tableViewConversation->sizeHint();
//    ui->tableViewConversation->resize(qs);

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

void Activity::PopulateConversationsForNym(
   QListWidget * pListWidgetConversations,
   int & nIndex,
   const std::string & str_my_nym_id,
   const std::string & activity_summary_id,
   QListWidgetItem  *& pItemToSelect)
{
    pItemToSelect = nullptr;

    const int  nNymCount = opentxs::OT::App().API().Exec().GetNymCount();
    OT_ASSERT(nNymCount > 0);
    const bool bOnlyOneNymInWallet = (1 == nNymCount);
    // ------------------
    MTNameLookupQT theLookup;
    std::string str_my_nym_name ("");
    // ------------------
    if (bOnlyOneNymInWallet)
    {
        str_my_nym_name = "Me";
    }
    else
    {
        str_my_nym_name = theLookup.GetNymName(str_my_nym_id, "");

        if (str_my_nym_name.empty()) {
            str_my_nym_name = "Me";
        }
    }
//  const auto response = opentxs::OT::App().Contact().NewContact(label, opentxs::Identifier{hisnym}, opentxs::PaymentCode{paymentCode});
//  const auto pContact = opentxs::OT::App().Contact().Contact(opentxs::Identifier{contact});

    //NOTE: no point here, since the above lookup already uses the below code.
    //      It finds the contact based on NymID and then gets the Label() for that contact.
//    if (str_my_nym_name.empty()) { // still empty?
//        get contact ID from Nym ID here. Then get pointer to contact.
//        const auto pContact = opentxs::OT::App().Contact().Contact(opentxs::Identifier{contact});
//        pContact->
//    }
    // ------------------------
    const QString qstrMyNymName = QString::fromStdString(str_my_nym_name);
    // ------------------------
    opentxs::ObjectList threadList = opentxs::OT::App().Activity().Threads(opentxs::Identifier{str_my_nym_id});
    opentxs::ObjectList::const_iterator ci = threadList.begin();

    while (threadList.end() != ci)
    {
        const std::pair<std::string, std::string> & threadInfo = *ci;

        const std::string & str_thread_id = threadInfo.first;
        const std::string & str_thread_name = threadInfo.second;
        // ----------------------------------------------
        std::shared_ptr<opentxs::proto::StorageThread> thread;
        opentxs::OT::App().DB().Load(str_my_nym_id, str_thread_id, thread);
        // ----------------------------------------------
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
        QString qstrDisplayName = bOnlyOneNymInWallet
                ? QString("%1").arg(QString::fromStdString(str_thread_name))
                : QString("%1 in convo with: %2")
                  .arg(qstrMyNymName)
                  .arg(QString::fromStdString(str_thread_name));

        pListWidgetConversations->addItem(qstrDisplayName);
        QListWidgetItem * pItem = pListWidgetConversations->item(nIndex);
        nIndex++;
        pItem->setTextColor(Qt::white);
        const QString qstrThreadId = QString::fromStdString(str_thread_id);
        const QString qstrMyNymId = QString::fromStdString(str_my_nym_id);

        QString qstrActivitySummaryId;

        if (!qstrMyNymId.isEmpty())
        {
            auto& thread_summary = opentxs::OT::App().UI().ActivitySummary(
                opentxs::Identifier(str_my_nym_id));

            std::map<std::string,std::string>::iterator
                it_thread_summary =
                    thread_summary_.find(str_my_nym_id);
            if (thread_summary_.end() != it_thread_summary)
            {
                thread_summary_.erase(it_thread_summary);
            }
            thread_summary_[str_my_nym_id] = thread_summary.WidgetID()->str();
            qstrActivitySummaryId = QString::fromStdString(thread_summary_[str_my_nym_id]);
        }

        pItem->setData(Qt::UserRole+1, QVariant(qstrMyNymId));
        pItem->setData(Qt::UserRole+2, QVariant(qstrThreadId));
        pItem->setData(Qt::UserRole+3, QVariant(qstrActivitySummaryId));

        if (!qstrActivitySummaryId.isEmpty())
        {
            const std::string str_current_summary_id = qstrActivitySummaryId.toStdString();
            const std::string str_current_thread_id  = qstrThreadId.toStdString();

            if (!activity_summary_id.empty() && !active_thread_.empty() &&
                0 == str_current_summary_id.compare(activity_summary_id) &&
                0 == str_current_thread_id.compare(active_thread_))
            {
                pItemToSelect = pItem;
            }
        }
        // ----------------------------------------------
        // BACKGROUND COLOR

        pItem->setBackgroundColor((nIndex%2 == 0) ? Qt::gray : Qt::darkGray);
        // ----------------------------------------------
        ++ci;
    }
}

void Activity::RefreshConversationsTab()
{
    QListWidget * pListWidgetConversations = ui->listWidgetConversations;
    if (nullptr == pListWidgetConversations) {
        return;
    }
    // --------------------------------------
    // Make some temporary record of which item was selected,
    // before we refresh, so that we can re-select it afterwards.
    //
    int nCurrentRow = pListWidgetConversations->currentRow();

    std::string selected_my_nym_id, selected_thread_id, activity_summary_id;
    QListWidgetItem * pItem = pListWidgetConversations->currentItem();

    if (nullptr != pItem) {
        QVariant qvarSelectedMyNymId  = pItem->data(Qt::UserRole+1);
        QVariant qvarSelectedThreadId = pItem->data(Qt::UserRole+2);
        QVariant qvarActivitySummaryId = pItem->data(Qt::UserRole+3);

        QString  qstrSelectedMyNymId = qvarSelectedMyNymId.isValid() ?
            qvarSelectedMyNymId.toString() : QString("");
        QString  qstrSelectedThreadId = qvarSelectedThreadId.isValid() ?
            qvarSelectedThreadId.toString() : QString("");
        QString  qstrActivitySummaryId = qvarActivitySummaryId.isValid() ?
            qvarActivitySummaryId.toString() : QString("");

        selected_my_nym_id  = qstrSelectedMyNymId.toStdString();
        selected_thread_id  = qstrSelectedThreadId.toStdString();
        activity_summary_id = qstrActivitySummaryId.toStdString();
    }
    // ----------------------------------------------
    pListWidgetConversations->blockSignals(true);
    // ----------------------------------------------
    // Clear it
    pListWidgetConversations->clear();
    // ----------------------------------------------
    // Re-populate it

    int nIndex = 0;
//    QListWidgetItem * pItem = nullptr;

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
//    pListWidgetConversations->addItem("Hiro and Cliff");
//    pItem = pListWidgetConversations->item(nIndex);
//    nIndex++;
//    pItem->setTextColor(Qt::white);
//    pItem->setBackgroundColor(Qt::darkGray);
    // ----------------------------------------------
    // Conversational threads.
    //
//  const std::string str_nym_id = Moneychanger::It()->get_default_nym_id().toStdString();


    opentxs::ObjectList myNyms;
    const auto nNymCount = opentxs::OT::App().API().Exec().GetNymCount();

    for (int nym_index = 0; nym_index < nNymCount; ++nym_index)
    {
        const std::string nym_id = opentxs::OT::App().API().Exec().GetNym_ID(nym_index);

        if (!nym_id.empty())
        {
            const std::string nym_name = opentxs::OT::App().API().Exec().GetNym_Name(nym_id);
            myNyms.push_back(std::pair<std::string, std::string>{nym_id, nym_name});
        }
    }
    // ----------------------------------------------------
    QListWidgetItem * pItemToSelect     {nullptr};
    QListWidgetItem * pItemToSelectLoop {nullptr};

    const auto & nyms = myNyms;
//  const auto   nyms = opentxs::OT::App().Wallet().NymList();

    for (const auto & [ nym_id, nym_alias] : nyms)
    {
        const opentxs::Identifier nymId{nym_id};

        bool bMatchesActivitySummaryId {false};
        this->PopulateConversationsForNym(pListWidgetConversations,
                                          nIndex,
                                          nym_id,
                                          activity_summary_id,
                                          pItemToSelectLoop);

        if (nullptr != pItemToSelectLoop)
        {
            pItemToSelect = pItemToSelectLoop;
        }
    }
    // ----------------------------------------------------
    pListWidgetConversations->blockSignals(false);

    if (nullptr != pItemToSelect)
    {
        pListWidgetConversations->setCurrentItem(pItemToSelect);
    }
    else
    {
        pListWidgetConversations->setCurrentRow(nCurrentRow);
    }
}





static void setup_tableview_conversation(QTableView * pView)
{
    pView->setSortingEnabled(true);
    pView->resizeColumnsToContents();
//  pView->resizeRowsToContents();
    pView->horizontalHeader()->setStretchLastSection(true);
    pView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    pView->sortByColumn(0, Qt::DescendingOrder); // The timestamp ends up at index 0 in this proxy view.
    pView->setContextMenuPolicy(Qt::CustomContextMenu);
    pView->verticalHeader()->hide();
    pView->horizontalHeader()->hide();
    pView->setAlternatingRowColors(true);
    pView->setSelectionBehavior(QAbstractItemView::SelectRows);
}


static void setup_tableview(QTableView * pView, QAbstractItemModel * pProxyModel)
{
    ActivityPaymentsProxyModel * pPmntProxyModel = static_cast<ActivityPaymentsProxyModel *>(pProxyModel);
    pPmntProxyModel->setTableView(pView);
    pView->setModel(pProxyModel);

    pView->setSortingEnabled(true);
    pView->resizeColumnsToContents();
    pView->resizeRowsToContents();
    pView->horizontalHeader()->setStretchLastSection(true);
    pView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    pView->sortByColumn(7, Qt::DescendingOrder); // The timestamp ends up at index 7 in all the proxy views.

    pView->setContextMenuPolicy(Qt::CustomContextMenu);
    pView->verticalHeader()->hide();
    pView->setAlternatingRowColors(true);
    pView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    pView->setSelectionBehavior(QAbstractItemView::SelectRows);
}



//QSharedPointer<QStandardItemModel>  pStandardModelMessages_;

//QSharedPointer<QSqlQueryMessages>  pModelMessages_;
//QSharedPointer<ConvMsgsProxyModel> pThreadItemsProxyModel_;

void Activity::resetConversationItemsDataModel(const bool bProvidedIds/*=false*/,
                                               const QString * pstrMyNymID/*=nullptr*/,
                                               const QString * pstrThreadID/*=nullptr*/)
{
    if ((nullptr == ui) || (nullptr == ui->tableViewConversation))
    {
        return;
        // Must be early in the run, no UI elements instantiated yet.
    }
    // --------------------------------------------------------
    if (bProvidedIds && (nullptr == pstrMyNymID || nullptr == pstrThreadID))
    {
        qDebug() << "resetConversationItemsDataModel: Apparently Ids were expected but not actually provided. Returning. (Failed). Should never happen.";
        return;
    }
    // By this point we know if bProvidedIds is true,
    // that neither of the optional parameters are nullptr.
    // --------------------------------------------------------
//    QSharedPointer<QSqlQueryMessages> pNewSourceModel;
    QSharedPointer<QStandardItemModel> pNewSourceModel;

    if (!bProvidedIds)
    {
        pNewSourceModel.reset(new QStandardItemModel(0)); // An empty one, since there's no IDs for a real one.
    }
    else // bProvidedIds is definitely true...
    {
        pNewSourceModel = DBHandler::getInstance()->getNewConversationItemModel(*pstrMyNymID, *pstrThreadID);
    }
    // --------------------------------------------------------
    QSharedPointer<ConvMsgsProxyModel> pThreadItemsProxyModel{new ConvMsgsProxyModel}; // A new proxy
    pThreadItemsProxyModel->setSourceModel(pNewSourceModel.data());
    // ---------------------------------
    // Todo: any other setup / filters for the proxy model.
    // ---------------------------------
    pModelMessages_ = pNewSourceModel;
    pThreadItemsProxyModel_ = pThreadItemsProxyModel;
    // ---------------------------------
    pThreadItemsProxyModel->setTableView(ui->tableViewConversation);
    ui->tableViewConversation->setModel(pThreadItemsProxyModel.data());





//    {
//        const opentxs::Identifier nymID{mynym};
//        auto& activity = opentxs::OT::App().UI().ActivitySummary(nymID);
//        otOut << "Activity:\n";
//        dashLine();
//        auto& line = activity.First();
//
//        if (false == line.Valid()) {
//
//            return 1;
//        }
//
//        auto last = line.Last();
//        otOut << "* " << line.DisplayName() << " (" << line.ThreadID() << "): "
//        << time(line.Timestamp()) << "\n  " << line.Text() << "\n";
//
//        while (false == last) {
//            auto& line = activity.Next();
//            last = line.Last();
//            otOut << "* " << line.DisplayName() << " (" << line.ThreadID() << "): "
//            << time(line.Timestamp()) << "\n  " << line.Text() << "\n";
//        }
//    }

//
//
//    QString str_select = QString(
//     "SELECT "
//     "    msg.message_id AS message_id, "
//     "    msg.my_nym_id AS my_nym_id, "
//     "    conv_msg.conversation_id AS thread_id, "
//     "    msg.thread_item_id AS thread_item_id, "
//     "    msg.timestamp AS timestamp, "
//     "    msg.folder AS folder, "
//     "    msg_body.body AS body "
//     "  FROM "
//     "     `message` AS msg "
//     "     INNER JOIN `message_body` AS msg_body "
//     "         ON  msg.message_id = msg_body.message_id "
//     "         AND msg.thread_item_id = msg_body.thread_item_id " // This line is probably superfluous. Test removing it.
//     "     INNER JOIN `conversation_msg` AS conv_msg "
//     "         ON  conv_msg.thread_item_id = msg_body.thread_item_id "
//     "         AND conv_msg.my_nym_id = msg.my_nym_id "
//     "     WHERE "
//     "           msg.archived='%1' "
//     "       AND msg.has_subject='%2' "
//     "       AND msg.my_nym_id='%3' "
//     "       AND conv_msg.conversation_id='%4' "
//     ).arg(nArchived).arg(nHasNoSubject).arg(qstrMyNymId).arg(qstrThreadId);
//
//    if (bProvidedIds)
//    {
//    }
//

}





void Activity::onOpentxsWidgetUpdated(QString qstrWidgetID)
{
//    qDebug() << "Activity::onOpentxsWidgetUpdated: " << qstrWidgetID
//    << " Current thread ID: " << QString::fromStdString(active_thread_);

    const std::string widget_id = qstrWidgetID.toStdString();

    if (!widget_id.empty())
    {
        for (auto & [nym_id, summary_id] : thread_summary_)
        {
            if (0 == widget_id.compare(summary_id))
            {
                // The conversations tab contains a list of conversations
                // on the left-hand side.
                // These conversations may be threads owned by local Nym Alice,
                // or they might be threads owned by local nym Bob.
                // Therefore there are potentially multiple activity_summary_ids
                // since there may be multiple local Nyms, and the conversations
                // of ALL of these Nyms will appear listed on the left-hand side.
                // Therefore, if ANY of these Nyms have broadcasted a change to
                // their activity summary (perhaps including new conversations)
                // then we need to refresh the conversations tab.
                RefreshConversationsTab();
                return;
            }
        }
        // -------------------------------------------
        if (0 == widget_id.compare(active_thread_))
        {
            RefreshConversationDetails(ui->listWidgetConversations->currentRow());
        }
        // -------------------------------------------

        PopulateIssuerWidgetIds();

        //opentxs::otErr << "Activity::onOpentxsWidgetUpdated: RECEIVED WIDGET ID NEEDING UPDATE: " << widget_id << std::endl;

        bool bNeedToRefreshSummaryTree = false;

        // We save list->Widget().str() in a map owned by Activity, called issuers_.
        // based on myNymId and the currencyType pair as the key for each issuer.
        // That way later (here) we can look up to see if the widget ID for any of
        // those issuers matches the one that was passed into this function, so we
        // can see if we need to call RefreshSummaryTree.

        //    std::map<std::pair<nym_id_str, currency>, widget_id_str> MapActivityWidgetIds;
        //typedef std::map<std::pair<std::string, int>, std::string> MapActivityWidgetIds;

        for (auto & [key, issuer_widget_id] : issuers_)
        {
            //auto & [nym_id, currency_type] = key;
            if (0 == issuer_widget_id.compare(widget_id)) {
                opentxs::otErr << "Activity::onOpentxsWidgetUpdated: FOUND MATCHING ISSUER WIDGET ID !!" << std::endl;

                bNeedToRefreshSummaryTree = true;
                break;
            }
        }

        if (!bNeedToRefreshSummaryTree) { // Haven't found anything yet? Let's try the account Widget IDs then...
            for (auto & [account_id, account_widget_id] : accountWidgets_) {
                if (0 == account_widget_id.compare(widget_id)) {
                    opentxs::otErr << "Activity::onOpentxsWidgetUpdated: FOUND MATCHING ACCOUNT WIDGET ID !!" << std::endl;

                    bNeedToRefreshSummaryTree = true;
                    break;
                }
            }
        }

        if (bNeedToRefreshSummaryTree) {
            QTimer::singleShot(0, this, SLOT(RefreshSummaryTree()));
        }
        else
        {
            opentxs::otErr << "Activity::onOpentxsWidgetUpdated: Oh well, Widget ID being refreshed did NOT match my list of IDs (this time!)" << std::endl;
        }
    }
}


void Activity::RefreshConversationDetails(int nRow)
{
    if (nRow < 0)
    {
        qDebug() << "WARNING!!! I'm setting active_thread_ to an empty string in Activity::RefreshConversationDetails!!!!";
        active_thread_ = "";
        ui->plainTextEditMsg->setPlainText("");
        resetConversationItemsDataModel();
    }
    else // nRow >=0
    {
        QListWidgetItem * conversation_widget = ui->listWidgetConversations->currentItem();

        if (nullptr == conversation_widget) {
            qDebug() << "WARNING!!! I'm setting active_thread_ to an empty string in Activity::RefreshConversationDetails 2!!!!";
            active_thread_ = "";
            ui->plainTextEditMsg->setPlainText("");
            resetConversationItemsDataModel();
            return;
        }

        const QVariant qvarMyNymId  = conversation_widget->data(Qt::UserRole+1);
        const QVariant qvarThreadId = conversation_widget->data(Qt::UserRole+2);

        QString  qstrMyNymId  = qvarMyNymId.isValid()  ? qvarMyNymId.toString()  : QString("");
        const QString  qstrThreadId = qvarThreadId.isValid() ? qvarThreadId.toString() : QString("");

        std::string str_my_nym_id;

        if (qstrMyNymId.isEmpty())
        {
            qstrMyNymId = Moneychanger::It()->get_default_nym_id();
            str_my_nym_id = qstrMyNymId.toStdString();
        }
        else
        {
            str_my_nym_id = qstrMyNymId.toStdString();
        }
        // -------------------------------------------
        if (!qstrMyNymId.isEmpty())
        {
            auto& thread_summary = opentxs::OT::App().UI().ActivitySummary(
                opentxs::Identifier(str_my_nym_id));

            std::map<std::string,std::string>::iterator
                it_thread_summary =
                    thread_summary_.find(str_my_nym_id);
            if (thread_summary_.end() != it_thread_summary)
            {
                thread_summary_.erase(it_thread_summary);
            }

            thread_summary_[str_my_nym_id] = thread_summary.WidgetID()->str();
        }
        // -------------------------------------------
        if (qstrMyNymId.isEmpty() || qstrThreadId.isEmpty())
        {
            qDebug() << "WARNING!!! I'm setting active_thread_ to an empty string in Activity::RefreshConversationDetails 3!!!!";

            active_thread_ = "";
            ui->plainTextEditMsg->setPlainText("");
            resetConversationItemsDataModel();
            return; // Should never happen.
        }
        // ----------------------------------------------
//      const std::string str_my_nym_id = qstrMyNymId.toStdString();
        const std::string str_thread_id = qstrThreadId.toStdString();
        // ----------------------------------------------
        // NOTE: Sometimes the "conversational thread id" is just the contact ID,
        // like when it's just you and the recipient in a private thread.
        // But otherwise, it might be a group conversation, in which case the
        // thread ID is the group ID (and thus NOT a contact ID). But it's okay
        // to pass it either way, because Opentxs handles it properly either way.
        //
        const opentxs::Identifier nymID(str_my_nym_id);
        const opentxs::Identifier threadID(str_thread_id);
        auto& thread = opentxs::OT::App().UI().ActivityThread(nymID, threadID);

        active_thread_ = thread.WidgetID()->str();

//        qDebug() << "I'm setting active_thread_ to Widget ID: " <<
//            QString::fromStdString(active_thread_);

        QString qstrMsg = QString::fromStdString(thread.GetDraft()).simplified();

        ui->plainTextEditMsg->setPlainText(qstrMsg);
        // ----------------------------------------------
        resetConversationItemsDataModel(true, &qstrMyNymId, &qstrThreadId);


//        const std::string str_my_nym_id = qstrMyNymId.toStdString();
//        const std::string str_thread_id = qstrThreadId.toStdString();
//
//        const auto& thread = opentxs::OT::App().UI().ActivityThread(
//            opentxs::Identifier(str_my_nym_id),
//            opentxs::Identifier(str_thread_id));
//        const auto& first = thread.First();
//        qDebug() << QString::fromStdString(thread.DisplayName()) << "\n";
//
//        if (false == first.Valid()) {
//            return;
//        }
//
//        auto last = first.Last();
//        qDebug() << " * "
//        // << time(first.Timestamp())
//        << " " << QString::fromStdString(first.Text()) << "\n";
//
//        while (false == last) {
//            auto& line = thread.Next();
//            last = line.Last();
//            qDebug() << " * "
//            //<< time(line.Timestamp())
//            << " "
//            << QString::fromStdString(line.Text()) << "\n";
//        }


        return;
        // ----------------------------------------------
        // We don't filter using the proxy model anymore.
        // At least, we don't filter by My Nym and Thread Id.
        // That's now done in the source model, since we query
        // it that way from the database.
        //
//        std::shared_ptr<opentxs::proto::StorageThread> thread;
//        opentxs::OT::App().DB().Load(str_my_nym_id, str_thread_id, thread);
//        // ----------------------------------------------
//        if (!thread) {
//            return;
//        }
//        // ----------------------------------------------
//        if (pThreadItemsProxyModel_)
//        {
//            // Note: until we have group convos, the Thread Id IS the Contact ID.
//            //
//            pThreadItemsProxyModel_->setFilterTopLevel_Convo(str_my_nym_id, str_thread_id);
//        }
        // ----------------------------------------------
        // Below is how we populated the OLD control (removed from UI already):

//        int nConversationIndex = 0;
//        for(const auto & item : thread->item())
//        {
//            bool bIncoming = false;
//            if (opentxs::StorageBox::MAILINBOX == static_cast<opentxs::StorageBox>(item.box())) {
//                bIncoming = true;
//            }
////          message StorageThreadItem {
////            optional uint32 version = 1;
////            optional string id = 2;
////            optional uint64 index = 3;
////            optional uint64 time = 4;
////            optional uint32 box = 5;
////            optional string account = 6;
////            optional bool unread = 7;
////            }
//            const std::string item_id = item.id();
////          const uint64_t item_index = item.index();
////          const uint64_t item_time = item.time();
////          const std::string str_item_account = item.account();
//            // ----------------------------------------------
//            const QString qstrItemId = QString::fromStdString(item_id);
//            // First let's see if we already imported it yet:
//            QString qstrContents = MTContactHandler::getInstance()->GetMessageBody(qstrItemId);
//            // Okay, maybe we haven't imported it yet?
//            // Let's try and grab a copy from the actual opentxs box. (inmail or outmail).
//            //
//            if (qstrContents.isEmpty()) {
//                const std::string str_item_contents = bIncoming ?
//                            opentxs::SwigWrap::GetNym_MailContentsByIndex   (str_my_nym_id, item_id) :
//                            opentxs::SwigWrap::GetNym_OutmailContentsByIndex(str_my_nym_id, item_id);
//                qstrContents = str_item_contents.empty() ? QString("") : QString::fromStdString(str_item_contents);
//            }
//            // ----------------------------------------------
//            QString qstrSubject{"subject:"};
//            const bool bHasContents = !qstrContents.isEmpty();
//            const bool bHasSubject  = bHasContents && qstrContents.startsWith(qstrSubject, Qt::CaseInsensitive);
//            if (bHasContents && !bHasSubject) {
//                // Notice I'm passing 'false' for bAddedByHand. That's because the
//                // item is being added from looping through an existing conversation.
//                if (AddItemToConversation(nConversationIndex, qstrContents, bIncoming, false)) {
//                    nConversationIndex++;
//                }
//            }
//        } // for
        // ----------------------------------------------
    } // else nRow >=0
}



void Activity::dialog(int nSourceRow/*=-1*/, int nFolder/*=-1*/)
{
    if (!already_init)
    {
        this->setWindowTitle(tr(MONEYCHANGER_APP_NAME));

        ui->horizontalLayout_4->setAlignment(ui->checkBoxSearchConversations, Qt::AlignRight);
        ui->horizontalLayout_4->setAlignment(ui->toolButtonMyIdentity2,       Qt::AlignLeft);

        setup_tableview_conversation(ui->tableViewConversation);

//        ui->treeWidgetAccounts->header()->close();
//        ui->treeWidgetAccounts->setRootIsDecorated(false);

        ui->treeWidgetSummary->header()->close();
        ui->treeWidgetSummary->setRootIsDecorated(false);

//        ui->treeWidgetSummary->setContextMenuPolicy(Qt::CustomContextMenu);
//        connect(ui->treeWidgetSummary, SIGNAL(customContextMenuRequested(const QPoint &)),
//                this, SLOT(on_treeWidgetSummary_customContextMenuRequested(const QPoint &)));

        // ******************************************************

        QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

        if (pModel)
        {
            pPmntProxyModelOutbox_ = new ActivityPaymentsProxyModel;
            pPmntProxyModelInbox_  = new ActivityPaymentsProxyModel;
            pPmntProxyModelInbox_ ->setSourceModel(pModel);
            pPmntProxyModelOutbox_->setSourceModel(pModel);
            pPmntProxyModelOutbox_->setFilterFolder(0);
            pPmntProxyModelInbox_ ->setFilterFolder(1);
            // ---------------------------------
            setup_tableview(ui->tableViewSent, pPmntProxyModelOutbox_);
            setup_tableview(ui->tableViewReceived, pPmntProxyModelInbox_);
            // ---------------------------------
            QItemSelectionModel *sm1 = ui->tableViewSent->selectionModel();
            connect(sm1, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                    this, SLOT(on_tableViewSentSelectionModel_currentRowChanged(QModelIndex,QModelIndex)));
            QItemSelectionModel *sm2 = ui->tableViewReceived->selectionModel();
            connect(sm2, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                    this, SLOT(on_tableViewReceivedSelectionModel_currentRowChanged(QModelIndex,QModelIndex)));
        }
        // --------------------------------------------------------
        connect(ui->toolButtonMyIdentity2, SIGNAL(clicked()), Moneychanger::It(), SLOT(mc_defaultnym_slot()));
        // --------------------------------------------------------
        connect(this, SIGNAL(showContact (QString)),         Moneychanger::It(), SLOT(mc_show_opentxs_contact_slot(QString)));
        connect(this, SIGNAL(showContacts()),                Moneychanger::It(), SLOT(mc_opentxs_contacts_slot()));
        // --------------------------------------------------------
//      connect(this, SIGNAL(showContactAndRefreshHome(QString)), Moneychanger::It(), SLOT(onNeedToPopulateRecordlist()));
        connect(this, SIGNAL(showContactAndRefreshHome(QString)), Moneychanger::It(), SLOT(mc_show_opentxs_contact_slot(QString)));
        // --------------------------------------------------------


        QWidget* pTab0 = ui->tabWidgetTransactions->widget(0);
        QWidget* pTab1 = ui->tabWidgetTransactions->widget(1);

        pTab0->setStyleSheet("QWidget { margin: 0 }");
        pTab1->setStyleSheet("QWidget { margin: 0 }");

//        ui->splitter->setStretchFactor(0, 2);
//        ui->splitter->setStretchFactor(1, 3);

        ui->splitterAccounts->setStretchFactor(0, 1);
        ui->splitterAccounts->setStretchFactor(2, 5);
        // ------------------------
        on_tabWidgetTransactions_currentChanged(0);

        this->on_checkBoxSearchConversations_toggled(false);
        this->on_checkBoxSearchPayments_toggled(false);
        // ------------------------
        //ui->tabWidgetMain->setStyleSheet("QTabWidget::tab:disabled { width: 0; height: 0; margin: 0; padding: 0; border: none; }");

        pTab0_ = ui->tabWidgetMain->widget(0);
        pTab1_ = ui->tabWidgetMain->widget(1);
        pTab2_ = ui->tabWidgetMain->widget(2);
        pTab3_ = ui->tabWidgetMain->widget(3);
        pTab4_ = ui->tabWidgetMain->widget(4);
        pTab5_ = ui->tabWidgetMain->widget(5);
        pTab6_ = ui->tabWidgetMain->widget(6);

        // ------------------------


        // ------------------------
        /** Flag Already Init **/
        already_init = true;
    }
    // -------------------------------------------
    /*
     Currently the tabs are:

     0 - Accounts
     1 - Conversations
     2 - Messages
     3 - Agreements
     4 - Exchange
     5 - Tools
     6 - Settings

     For now, unless we're in expert mode, we'll turn them ALL OFF except
     for Accounts, Conversations, and Settings.
     */

    ui->tabWidgetMain->blockSignals(true);

    while (ui->tabWidgetMain->count() > 0) {
        ui->tabWidgetMain->removeTab(0);
    }

    ui->tabWidgetMain->addTab(pTab0_, tr("Accounts"));
    ui->tabWidgetMain->addTab(pTab1_, tr("Conversations"));

    if (Moneychanger::It()->expertMode()) ui->tabWidgetMain->addTab(pTab2_, tr("Messages")); // Messages
    if (Moneychanger::It()->expertMode()) ui->tabWidgetMain->addTab(pTab3_, tr("Agreements")); // Agreements
    if (Moneychanger::It()->expertMode()) ui->tabWidgetMain->addTab(pTab4_, tr("Exchange")); // Exchange
    if (Moneychanger::It()->expertMode()) ui->tabWidgetMain->addTab(pTab5_, tr("Tools")); // Tools

//  ui->tabWidgetMain->addTab(pTab6_, tr("Settings"));

    ui->tabWidgetMain->blockSignals(false);
    // -------------------------------------------
    RefreshAll();
    // -------------------------------------------
    Focuser f(this);
    f.show();
    f.focus();
    // -------------------------------------------
    setAsCurrentPayment(nSourceRow, nFolder);



    PopulateIssuerWidgetIds();

    Populate_comboBoxMyNym();
    Populate_comboBoxCurrency();
}

void Activity::Populate_comboBoxMyNym()
{
    QComboBox * pComboBox = ui->comboBoxMyNym;
    // ----------------------------------------
    pComboBox->blockSignals(true);
    // ----------------------------------------
    pComboBox->clear();

    const std::set<std::string> localNyms = opentxs::OT::App().DB().LocalNyms();

    for (const auto & nym_id_str : localNyms)
    {
        const std::string nym_name = opentxs::SwigWrap::GetNym_Name(nym_id_str);
        const QString qstrNymId    = QString::fromStdString(nym_id_str);
        const QString qstrNymName  = QString::fromStdString(nym_name);
        const QVariant qvarUserData(qstrNymId);
        pComboBox->addItem(icon_nym_, qstrNymName, qvarUserData);
    }
    // ----------------------------------------
    int nCurrentIndexToSet = -1;

    if (pComboBox->count() > 0) {
        nCurrentIndexToSet = 0;
    }
    pComboBox->setCurrentIndex(nCurrentIndexToSet);
    // ----------------------------------------
    pComboBox->blockSignals(false);
    // ----------------------------------------
    on_comboBoxMyNym_activated(nCurrentIndexToSet);
}

void Activity::Populate_comboBoxCurrency()
{
    QComboBox * pComboBox = ui->comboBoxCurrency;
    // ----------------------------------------
    pComboBox->blockSignals(true);
    pComboBox->clear();
    // ----------------------------------------
    std::set<StringIntPair> set_currency_types;
    GetCurrencyTypesByAssetContractsInWallet(set_currency_types);

    for (const auto & currency_pair : set_currency_types)
    {
        const auto & currency_label = currency_pair.first;
        const auto & currency_type  = currency_pair.second;
        const QString qstrCurrencyLabel = QString::fromStdString(currency_label);
        const QVariant qvarUserData(currency_type);
        pComboBox->addItem(qstrCurrencyLabel, qvarUserData);
    }
    // ----------------------------------------
    int nCurrentIndexToSet = -1;

    if (pComboBox->count() > 0) {
        nCurrentIndexToSet = 0;
    }
    pComboBox->setCurrentIndex(nCurrentIndexToSet);
    // ----------------------------------------
    pComboBox->blockSignals(false);
    // ----------------------------------------
    on_comboBoxCurrency_activated(nCurrentIndexToSet);
}


void Activity::on_comboBoxMyNym_activated(int index)
{
    QTimer::singleShot(0, this, SLOT(RefreshSummaryTree()));
}

void Activity::on_comboBoxCurrency_activated(int index)
{
    QTimer::singleShot(0, this, SLOT(RefreshSummaryTree()));
}

void Activity::PopulateIssuerWidgetIds()
{
    std::set<int> currency_types = opentxs::SwigWrap::GetCurrencyTypesForLocalAccounts();

    for (const auto currency_type : currency_types)
    {
        const auto currencyType = opentxs::proto::ContactItemType(currency_type);

        const int32_t nymCount = opentxs::OT::App().API().Exec().GetNymCount();
        for (int32_t nymIndex = 0; nymIndex < nymCount; ++nymIndex)
        {
            std::string mynym = opentxs::OT::App().API().Exec().GetNym_ID(nymIndex);

            const opentxs::Identifier nymID{mynym};
            auto& list = opentxs::OT::App().UI().AccountSummary(nymID, currencyType);
            auto issuer = list.First();

            if (false == issuer->Valid()) {

                continue;
            }
            // ---------------
            // There's a valid issuer for this MyNym/CurrencyType pair.
            // Add its widget ID to our issuers_ map so that later we can see if a widget
            // publication from opentxs (which we are subscribed to) matches that same
            // widget ID, which is our signal to call RefreshSummaryTree.
            //
            auto pairCurrencyKey = std::pair<std::string, int>(mynym, currency_type);

            auto it_existing = issuers_.find(pairCurrencyKey);
            if (it_existing != issuers_.end()) // Found it already there
            {
                issuers_.erase(it_existing); // Now we know when we re-insert it, it will succeed and contain the latest version.
            }

            auto pairToInsert = std::pair<std::pair<std::string, int>, std::string>(pairCurrencyKey, list.WidgetID()->str());

            issuers_.insert(pairToInsert);
            // ---------------

            accountWidgets_.clear();

            auto account = issuer->First();

            if (account->Valid()) {
                const std::string account_id(account->AccountID());
                const std::string account_widget_id(account->WidgetID()->str());
                std::pair<std::string, std::string> accountPairToInsert(account_id, account_widget_id);
                accountWidgets_.insert(accountPairToInsert);
                auto lastAccount = account->Last();
                while (false == lastAccount) {
                    account = issuer->Next();
                    if (false == account->Valid()) { break; }
                    lastAccount = account->Last();
                    const std::string account_id(account->AccountID());
                    const std::string account_widget_id(account->WidgetID()->str());
                    std::pair<std::string, std::string> accountPairToInsert(account_id, account_widget_id);
                    accountWidgets_.insert(accountPairToInsert);
                }
            }
        }
    }
}

void Activity::on_tableViewConversation_clicked(const QModelIndex &index)
{
    if (!pModelMessages_ || !pThreadItemsProxyModel_ || !index.isValid()) {
        return;
    }


    //resume

    QTableView * pTableView = ui->tableViewConversation;

//    QModelIndex indexAtClick = pTableView->indexAt(pos);
//    if (!indexAtClick.isValid())
//        return;
    // I can't figure out how to ADD to the selection without UNSELECTING everything else.
    // The Qt docs indicate that the below options should do that -- but it doesn't work.
    // So this is commented out since it was deselecting everything.
    //pTableView->selectionModel()->select( indexAtClick, QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows );
    // ------------------------
    QModelIndex sourceIndexAtClick = pThreadItemsProxyModel_->mapToSource(index);
    const int nSourceRow = sourceIndexAtClick.row();
//  QModelIndex sourceIndexAtBody  = pModelMessages_->sibling(nSourceRow, CONV_SOURCE_COL_BODY, sourceIndexAtClick);
//  QVariant qvarData = pModelMessages_->data(sourceIndexAtBody,Qt::DisplayRole);
//  QString  qstrData      = qvarData.isValid() ? qvarData.toString() : "";
//  QString  qstrDecrypted = qstrData.isEmpty() ? "" : MTContactHandler::Decrypt(qstrData);



    //resume

    QWidget * pOldWidget = pTableView->indexWidget(index);
    QWidget * pNewWidget = pThreadItemsProxyModel_->CreateDetailHeaderWidget(nSourceRow, true);


    pTableView->setIndexWidget(index, pNewWidget);

    pTableView->setRowHeight(index.row(), pNewWidget->height()*1.2);


//    qDebug() << "DECRYPTED THE MESSAGE YOU JUST CLICKED: \n\n" << qstrDecrypted;

}


void Activity::on_tableViewSentSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous)
{
    if (!current.isValid())
    {
        disableButtons();
        // ----------------------------------------
        ACTIVITY_TREE_ITEM theItem = make_tree_item(qstrCurrentTLA_, qstrCurrentNotary_, qstrCurrentAccount_, qstrCurrentContact_);
        set_outbox_pmntid_for_tree_item(theItem, 0);
    }
    else
    {
        enableButtons();
        // ----------------------------------------
        QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

        if (pModel)
        {
            QModelIndex sourceIndex = pPmntProxyModelOutbox_->mapToSource(current);

            QModelIndex haveReadSourceIndex  = pPmntProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_HAVE_READ, sourceIndex);
            QModelIndex pmntidSourceIndex    = pPmntProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_PMNT_ID,   sourceIndex);
//            QModelIndex subjectSourceIndex   = pPmntProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_MEMO,      sourceIndex);
//            QModelIndex senderSourceIndex    = pPmntProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_MY_NYM,    sourceIndex);
//            QModelIndex recipientSourceIndex = pPmntProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_RECIP_NYM, sourceIndex);
//            QModelIndex timestampSourceIndex = pPmntProxyModelOutbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_TIMESTAMP, sourceIndex);

//            QModelIndex subjectIndex      = pPmntProxyModelOutbox_->mapFromSource(subjectSourceIndex);
//            QModelIndex senderIndex       = pPmntProxyModelOutbox_->mapFromSource(senderSourceIndex);
//            QModelIndex recipientIndex    = pPmntProxyModelOutbox_->mapFromSource(recipientSourceIndex);
//            QModelIndex timestampIndex    = pPmntProxyModelOutbox_->mapFromSource(timestampSourceIndex);

            QVariant varpmntid    = pModel->data(pmntidSourceIndex);
            QVariant varHaveRead  = pModel->data(haveReadSourceIndex);
//            QVariant varSubject   = pPmntProxyModelOutbox_->data(subjectIndex);
//            QVariant varSender    = pPmntProxyModelOutbox_->data(senderIndex);
//            QVariant varRecipient = pPmntProxyModelOutbox_->data(recipientIndex);
//            QVariant varTimestamp = pPmntProxyModelOutbox_->data(timestampIndex);

//            QString qstrSubject   = varSubject.isValid()   ? varSubject.toString()   : "";
//            QString qstrSender    = varSender.isValid()    ? varSender.toString()    : "";
//            QString qstrRecipient = varRecipient.isValid() ? varRecipient.toString() : "";
//            QString qstrTimestamp = varTimestamp.isValid() ? varTimestamp.toString() : "";
            // ----------------------------------------------------------
            const int payment_id = varpmntid.isValid() ? varpmntid.toInt() : 0;
            if (payment_id > 0)
            {
                ACTIVITY_TREE_ITEM theItem = make_tree_item(qstrCurrentTLA_, qstrCurrentNotary_, qstrCurrentAccount_, qstrCurrentContact_);
                set_outbox_pmntid_for_tree_item(theItem, payment_id);
            }
            // ----------------------------------------------------------
            const bool bHaveRead = varHaveRead.isValid() ? varHaveRead.toBool() : false;

            if (!bHaveRead && (payment_id > 0)) // It's unread, so we need to set it as read.
            {
                listRecordsToMarkAsRead_.append(haveReadSourceIndex);
                QTimer::singleShot(1000, this, SLOT(on_MarkAsRead_timer()));
            }
        }
    }
}

void Activity::on_tableViewReceivedSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous)
{
    if (!current.isValid())
    {
        disableButtons();
        // ----------------------------------------
        ACTIVITY_TREE_ITEM theItem = make_tree_item(qstrCurrentTLA_, qstrCurrentNotary_, qstrCurrentAccount_, qstrCurrentContact_);
        set_inbox_pmntid_for_tree_item(theItem, 0);
    }
    else
    {
        enableButtons();
        // ----------------------------------------
        QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

        if (pModel)
        {
            QModelIndex sourceIndex = pPmntProxyModelInbox_->mapToSource(current);

            QModelIndex haveReadSourceIndex  = pPmntProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_HAVE_READ,  sourceIndex);
            QModelIndex pmntidSourceIndex    = pPmntProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_PMNT_ID,    sourceIndex);
//            QModelIndex subjectSourceIndex   = pPmntProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_MEMO,       sourceIndex);
//            QModelIndex senderSourceIndex    = pPmntProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_SENDER_NYM, sourceIndex);
//            QModelIndex recipientSourceIndex = pPmntProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_MY_NYM,     sourceIndex);
//            QModelIndex timestampSourceIndex = pPmntProxyModelInbox_->sourceModel()->sibling(sourceIndex.row(), PMNT_SOURCE_COL_TIMESTAMP,  sourceIndex);

//            QModelIndex subjectIndex      = pPmntProxyModelInbox_->mapFromSource(subjectSourceIndex);
//            QModelIndex senderIndex       = pPmntProxyModelInbox_->mapFromSource(senderSourceIndex);
//            QModelIndex recipientIndex    = pPmntProxyModelInbox_->mapFromSource(recipientSourceIndex);
//            QModelIndex timestampIndex    = pPmntProxyModelInbox_->mapFromSource(timestampSourceIndex);

            QVariant varpmntid    = pModel->data(pmntidSourceIndex);
            QVariant varHaveRead  = pModel->data(haveReadSourceIndex);
//            QVariant varSubject   = pPmntProxyModelInbox_->data(subjectIndex);
//            QVariant varSender    = pPmntProxyModelInbox_->data(senderIndex);
//            QVariant varRecipient = pPmntProxyModelInbox_->data(recipientIndex);
//            QVariant varTimestamp = pPmntProxyModelInbox_->data(timestampIndex);

//            QString qstrSubject   = varSubject.isValid()   ? varSubject  .toString() : "";
//            QString qstrSender    = varSender   .isValid() ? varSender   .toString() : "";
//            QString qstrRecipient = varRecipient.isValid() ? varRecipient.toString() : "";
//            QString qstrTimestamp = varTimestamp.isValid() ? varTimestamp.toString() : "";

//            ui->headerReceived->setSubject  (qstrSubject);
//            ui->headerReceived->setSender   (qstrSender);
//            ui->headerReceived->setRecipient(qstrRecipient);
//            ui->headerReceived->setTimestamp(qstrTimestamp);
//            ui->headerReceived->setFolder(tr("Received"));

            int payment_id = varpmntid.isValid() ? varpmntid.toInt() : 0;
            if (payment_id > 0)
            {
                ACTIVITY_TREE_ITEM theItem = make_tree_item(qstrCurrentTLA_, qstrCurrentNotary_, qstrCurrentAccount_, qstrCurrentContact_);
                set_inbox_pmntid_for_tree_item(theItem, payment_id);
            }
            // ----------------------------------------------------------
            const bool bHaveRead = varHaveRead.isValid() ? varHaveRead.toBool() : false;

            if (!bHaveRead && (payment_id > 0)) // It's unread, so we need to set it as read.
            {
                listRecordsToMarkAsRead_.append(haveReadSourceIndex);
                QTimer::singleShot(1000, this, SLOT(on_MarkAsRead_timer()));
            }
        }
    }
}

// This button is on the "Conversation" tab. (Not the main activity tab).
//
void Activity::on_pushButtonSendMsg_clicked()
{
    const QString qstrPlainText = ui->plainTextEditMsg->toPlainText().simplified();

    if (!qstrPlainText.isEmpty())
    {
        const std::string message {qstrPlainText.toStdString()};

        if (ui->listWidgetConversations->currentRow() <  0)
        {
//            ui->listWidgetConversation->clear();
        }
        else
        {
            QListWidgetItem * conversation_widget = ui->listWidgetConversations->currentItem();

            if (nullptr == conversation_widget) {
                return;
            }

            const QVariant qvarMyNymId  = conversation_widget->data(Qt::UserRole+1);
            const QVariant qvarThreadId = conversation_widget->data(Qt::UserRole+2);

            const QString  qstrMyNymId  = qvarMyNymId.isValid()  ? qvarMyNymId.toString()  : QString("");
            const QString  qstrThreadId = qvarThreadId.isValid() ? qvarThreadId.toString() : QString("");

            if (!qstrMyNymId.isEmpty() && !qstrThreadId.isEmpty())
            {
                const std::string str_my_nym_id = qstrMyNymId.toStdString();
                const std::string str_thread_id = qstrThreadId.toStdString();
                // ----------------------------------------------
                // NOTE: Sometimes the "conversational thread id" is just the contact ID,
                // like when it's just you and the recipient in a private thread.
                // But otherwise, it might be a group conversation, in which case the
                // thread ID is the group ID (and thus NOT a contact ID). But it's okay
                // to pass it either way, because Opentxs handles it properly either way.
                //
                const opentxs::Identifier nymID(str_my_nym_id);
                const opentxs::Identifier threadID(str_thread_id);
                auto& thread = opentxs::OT::App().UI().ActivityThread(nymID, threadID);

                // NOTE: Normally I'd assume that the draft is already set by
                // the time we get to this point. But since we're here, might
                // as well make sure we have the very latest version before we
                // send it out.
                //
                const auto loaded = thread.SetDraft(message);

                OT_ASSERT(loaded)

                const auto sent = thread.SendDraft();
                if (sent) {
                    ui->plainTextEditMsg->setPlainText("");
                }
            }
        }
    }
}

void Activity::on_plainTextEditMsg_textChanged()
{
    const QString qstrPlainText = ui->plainTextEditMsg->toPlainText().simplified();

    if (!qstrPlainText.isEmpty())
    {
        const std::string message{qstrPlainText.toStdString()};

        if (ui->listWidgetConversations->currentRow() <  0)
        {
//          ui->listWidgetConversation->clear();
        }
        else
        {
            QListWidgetItem * conversation_widget = ui->listWidgetConversations->currentItem();

            if (nullptr == conversation_widget) {
                return;
            }

            const QVariant qvarMyNymId  = conversation_widget->data(Qt::UserRole+1);
            const QVariant qvarThreadId = conversation_widget->data(Qt::UserRole+2);

            const QString  qstrMyNymId  = qvarMyNymId.isValid()  ? qvarMyNymId.toString()  : QString("");
            const QString  qstrThreadId = qvarThreadId.isValid() ? qvarThreadId.toString() : QString("");

            if (!qstrMyNymId.isEmpty() && !qstrThreadId.isEmpty())
            {
                const std::string str_my_nym_id = qstrMyNymId.toStdString();
                const std::string str_thread_id = qstrThreadId.toStdString();
                // ----------------------------------------------
                // NOTE: Sometimes the "conversational thread id" is just the contact ID,
                // like when it's just you and the recipient in a private thread.
                // But otherwise, it might be a group conversation, in which case the
                // thread ID is the group ID (and thus NOT a contact ID). But it's okay
                // to pass it either way, because Opentxs handles it properly either way.
                //
                const opentxs::Identifier nymID(str_my_nym_id);
                const opentxs::Identifier threadID(str_thread_id);
                auto& thread = opentxs::OT::App().UI().ActivityThread(nymID, threadID);
                const auto loaded = thread.SetDraft(message);

                OT_ASSERT(loaded)
            }
        }
    }
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

void Activity::on_listWidgetConversations_customContextMenuRequested(const QPoint &pos)
{
//    conversationsPopupMenu(pos, ui->listWidgetConversations);
}



Activity::~Activity()
{
    delete ui;
}



void Activity::on_checkBoxSearchConversations_toggled(bool checked)
{
    if (checked)
    {
        ui->lineEditSearchConversations->setVisible(true);
        ui->pushButtonSearchConversations->setVisible(true);
    }
    else
    {
        ui->lineEditSearchConversations->setVisible(false);
        ui->pushButtonSearchConversations->setVisible(false);
    }
}




void Activity::on_checkBoxSearchPayments_toggled(bool checked)
{
    if (checked)
    {
        ui->labelMyIdentity2->setVisible(false);
        ui->toolButtonMyIdentity2->setVisible(false);
    }
    else
    {
        ui->labelMyIdentity2->setVisible(true);
        ui->toolButtonMyIdentity2->setVisible(true);
    }
}





















void Activity::on_tabWidgetTransactions_currentChanged(int index)
{
    if (ui->tableViewSent      && ui->tableViewReceived &&
        pPmntProxyModelOutbox_ && pPmntProxyModelInbox_)
    {
        pCurrentTabTableView_  = (0 == ui->tabWidgetTransactions->currentIndex()) ? ui->tableViewReceived    : ui->tableViewSent;
        pCurrentTabProxyModel_ = (0 == ui->tabWidgetTransactions->currentIndex()) ? &(*pPmntProxyModelInbox_) : &(*pPmntProxyModelOutbox_);
        // -------------------------------------------------
        QModelIndex the_index  = pCurrentTabTableView_->currentIndex();

        if (the_index.isValid())
            enableButtons();
        else
            disableButtons();
        // --------------------------------------
        RefreshPayments();
    }
    else
    {
        pCurrentTabTableView_  = nullptr;
        pCurrentTabProxyModel_ = nullptr;

        disableButtons();
    }
}


void Activity::enableButtons()
{
//    ui->toolButtonDelete ->setEnabled(true);
//    ui->toolButtonReply  ->setEnabled(true);
//    ui->toolButtonForward->setEnabled(true);
}

void Activity::disableButtons()
{
//    ui->toolButtonDelete ->setEnabled(false);
//    ui->toolButtonReply  ->setEnabled(false);
//    ui->toolButtonForward->setEnabled(false);
}

void Activity::on_MarkAsRead_timer()
{
    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

    if (!pModel)
        return;
    // ------------------------------
    bool bEditing = false;

    while (!listRecordsToMarkAsRead_.isEmpty())
    {
        QModelIndex index = listRecordsToMarkAsRead_.front();
        listRecordsToMarkAsRead_.pop_front();
        // ------------------------------------
        if (!index.isValid())
            continue;
        // ------------------------------------
        if (!bEditing)
        {
            bEditing = true;
            pModel->database().transaction();
        }
        // ------------------------------------
        pModel->setData(index, QVariant(1)); // 1 for "true" in sqlite. "Yes, we've now read this payment. Mark it as read."
    } // while
    // ------------------------------
    if (bEditing)
    {
        if (pModel->submitAll())
        {
            pModel->database().commit();
            // ------------------------------------
            QTimer::singleShot(0, this, SLOT(RefreshPayments()));
        }
        else
        {
            pModel->database().rollback();
            qDebug() << "Database Write Error" <<
                       "The database reported an error: " <<
                       pModel->lastError().text();
        }
    }
}

void Activity::on_MarkAsUnread_timer()
{
    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

    if (!pModel)
        return;
    // ------------------------------
    bool bEditing = false;

    while (!listRecordsToMarkAsUnread_.isEmpty())
    {
        QModelIndex index = listRecordsToMarkAsUnread_.front();
        listRecordsToMarkAsUnread_.pop_front();
        // ------------------------------------
        if (!index.isValid())
            continue;
        // ------------------------------------
        if (!bEditing)
        {
            bEditing = true;
            pModel->database().transaction();
        }
        // ------------------------------------
        pModel->setData(index, QVariant(0)); // 0 for "false" in sqlite. "This payment is now marked UNREAD."
    } // while
    // ------------------------------
    if (bEditing)
    {
        if (pModel->submitAll())
        {
            pModel->database().commit();
            // ------------------------------------
            QTimer::singleShot(0, this, SLOT(RefreshPayments()));
        }
        else
        {
            pModel->database().rollback();
            qDebug() << "Database Write Error" <<
                       "The database reported an error: " <<
                       pModel->lastError().text();
        }
    }
}

void Activity::on_MarkAsReplied_timer()
{
    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

    if (!pModel)
        return;
    // ------------------------------
    bool bEditing = false;

    while (!listRecordsToMarkAsReplied_.isEmpty())
    {
        QModelIndex index = listRecordsToMarkAsReplied_.front();
        listRecordsToMarkAsReplied_.pop_front();
        // ------------------------------------
        if (!index.isValid())
            continue;
        // ------------------------------------
        if (!bEditing)
        {
            bEditing = true;
            pModel->database().transaction();
        }
        // ------------------------------------
        pModel->setData(index, QVariant(1)); // 1 for "true" in sqlite. "Yes, we've now replied to this payment. Mark it as replied."
    } // while
    // ------------------------------
    if (bEditing)
    {
        if (pModel->submitAll())
        {
            pModel->database().commit();
            // ------------------------------------
            QTimer::singleShot(0, this, SLOT(RefreshPayments()));
        }
        else
        {
            pModel->database().rollback();
            qDebug() << "Database Write Error" <<
                       "The database reported an error: " <<
                       pModel->lastError().text();
        }
    }
}

void Activity::on_MarkAsForwarded_timer()
{
    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

    if (!pModel)
        return;
    // ------------------------------
    bool bEditing = false;

    while (!listRecordsToMarkAsForwarded_.isEmpty())
    {
        QModelIndex index = listRecordsToMarkAsForwarded_.front();
        listRecordsToMarkAsForwarded_.pop_front();
        // ------------------------------------
        if (!index.isValid())
            continue;
        // ------------------------------------
        if (!bEditing)
        {
            bEditing = true;
            pModel->database().transaction();
        }
        // ------------------------------------
        pModel->setData(index, QVariant(1)); // 1 for "true" in sqlite. "Yes, we've now forwarded this payment. Mark it as forwarded."
    } // while
    // ------------------------------
    if (bEditing)
    {
        if (pModel->submitAll())
        {
            pModel->database().commit();
            // ------------------------------------
            QTimer::singleShot(0, this, SLOT(RefreshPayments()));
        }
        else
        {
            pModel->database().rollback();
            qDebug() << "Database Write Error" <<
                       "The database reported an error: " <<
                       pModel->lastError().text();
        }
    }
}







void Activity::setAsCurrentPayment(int nSourceRow, int nFolder)
{
    if (-1 == nSourceRow || -1 == nFolder)
        return;

    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

    if (!pModel)
        return;
    // -------------------
    QTableView * pTableView = (0 == nFolder) ? ui->tableViewSent : ui->tableViewReceived;
    if (nullptr == pTableView) return; // should never happen.
    QPointer<ActivityPaymentsProxyModel> & pProxyModel = (0 == nFolder) ? pPmntProxyModelOutbox_ : pPmntProxyModelInbox_;
    // ----------------------------
    // If the table view we're switching to, is not the current one, then we
    // need to make it the current one.
    //
    if (pTableView != pCurrentTabTableView_)
    {
        ui->tabWidgetTransactions->setCurrentIndex((0 == nFolder) ? 1 : 0);
    }
    // ----------------------------
    QModelIndex sourceIndex = pModel->index(nSourceRow, PMNT_SOURCE_COL_PMNT_ID);
    QModelIndex proxyIndex;
    if (sourceIndex.isValid())
        proxyIndex = pProxyModel->mapFromSource(sourceIndex);
    if (proxyIndex.isValid())
        pTableView->setCurrentIndex(proxyIndex);
}




/*
    QString qstrCurrentTLA_;  // If the user clicks on "BTC" near the top, the current TLA is "BTC".
    QString qstrCurrentNotary_; // If the user clicks on "localhost" (hosted notary) then that Notary ID is set here.
//  QString qstrCurrentWallet_; // If the user clicks on his local BTC wallet, then we set something in here I guess to distinguish it from the local LTC wallet.
    QString qstrCurrentAccount_; // If the user clicks on one of his Opentxs accounts, or a notary with only 1 account under it, we put the acct ID here.
    QString qstrCurrentContact_; // If the user clicks on one of his contacts, we put the Opentxs Contact ID here.

#define ACTIVITY_TREE_HEADER_TOTALS   0
#define ACTIVITY_TREE_HEADER_LOCAL    1
#define ACTIVITY_TREE_HEADER_SNP      2
#define ACTIVITY_TREE_HEADER_HOSTED   3
#define ACTIVITY_TREE_HEADER_CONTACTS 4

#define AC_NODE_TYPE_HEADER          0
#define AC_NODE_TYPE_ASSET_TOTAL     1
#define AC_NODE_TYPE_CONTACT         2
#define AC_NODE_TYPE_LOCAL_WALLET    3
#define AC_NODE_TYPE_LOCAL_ACCOUNT   4
#define AC_NODE_TYPE_SNP_NOTARY      5
#define AC_NODE_TYPE_SNP_ACCOUNT     6
#define AC_NODE_TYPE_HOSTED_NOTARY   7
#define AC_NODE_TYPE_HOSTED_ACCOUNT  8
*/
void Activity::on_treeWidgetSummary_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(current);
    Q_UNUSED(previous);

//    if (nullptr != current)
//    {
//        const int nNodeType = current->data(0, Qt::UserRole+0).toInt();
//        const int nHeader   = current->data(0, Qt::UserRole+1).toInt();

//        switch (nNodeType) {
//            case AC_NODE_TYPE_HEADER: {
////                qstrCurrentTLA_      = QString("");
////                qstrCurrentNotary_   = QString("");
////                qstrCurrentAccount_  = QString("");
////                qstrCurrentContact_  = QString("");
//                ;
//            } break;

//            case AC_NODE_TYPE_ASSET_TOTAL: {
//                const QVariant qvarTLA = current->data(0, Qt::UserRole+2);

//                qstrCurrentTLA_      = qvarTLA.isValid() ? qvarTLA.toString() : QString("");
//                qstrCurrentNotary_   = QString("");
//                qstrCurrentAccount_  = QString("");
//                qstrCurrentContact_  = QString("");

////                qDebug() << "Setting qstrCurrentContact_ to empty string! 7\n";

//                pPmntProxyModelInbox_ ->setFilterTLA(qstrCurrentTLA_);
//                pPmntProxyModelOutbox_->setFilterTLA(qstrCurrentTLA_);
//            } break;

//            // Todo someday. Currently Moneychanger doesn't have its own local Bitcoin wallet.
//            case AC_NODE_TYPE_LOCAL_WALLET:
//            case AC_NODE_TYPE_LOCAL_ACCOUNT: {
//                qstrCurrentTLA_      = QString("");
//                qstrCurrentNotary_   = QString("");
//                qstrCurrentAccount_  = QString("");
//                qstrCurrentContact_  = QString("");

////                qDebug() << "Setting qstrCurrentContact_ to empty string! 8\n";

//                // Todo someday.
//                pPmntProxyModelInbox_ ->setFilterNone();
//                pPmntProxyModelOutbox_->setFilterNone();
//            } break;

//            case AC_NODE_TYPE_SNP_NOTARY:
//            case AC_NODE_TYPE_HOSTED_NOTARY: {
//                // If a Notary has a single account, it may display them on a single line.
//                // In which case, sometimes we'll want the Notary ID off that line, but
//                // sometimes we'll want the account ID. (But the account ID will sometimes
//                // not be there, if there are sub-accounts listed under the notary in the UI).
//                //
////              const QVariant qvarAccountId = current->data(0, Qt::UserRole+4);
//                const QVariant qvarNotaryId  = current->data(0, Qt::UserRole+5);

//                qstrCurrentTLA_      = QString("");
//                qstrCurrentAccount_  = QString("");
////              qstrCurrentAccount_  = qvarAccountId.isValid() ? qvarAccountId.toString() : QString("");
//                qstrCurrentNotary_   = qvarNotaryId.isValid()  ? qvarNotaryId.toString()  : QString("");
//                qstrCurrentContact_  = QString("");

////                qDebug() << "Setting qstrCurrentContact_ to empty string! 9\n";

//                pPmntProxyModelInbox_ ->setFilterNotary(qstrCurrentNotary_);
//                pPmntProxyModelOutbox_->setFilterNotary(qstrCurrentNotary_);
//            } break;

//            case AC_NODE_TYPE_SNP_ACCOUNT:
//            case AC_NODE_TYPE_HOSTED_ACCOUNT: {
//                const QVariant qvarAccountId = current->data(0, Qt::UserRole+4);

//                qstrCurrentTLA_      = QString("");
//                qstrCurrentNotary_   = QString("");
//                qstrCurrentAccount_  = qvarAccountId.isValid() ? qvarAccountId.toString() : QString("");
//                qstrCurrentContact_  = QString("");

////                qDebug() << "Setting qstrCurrentContact_ to empty string! 10\n";

//                pPmntProxyModelInbox_ ->setFilterAccount(qstrCurrentAccount_);
//                pPmntProxyModelOutbox_->setFilterAccount(qstrCurrentAccount_);
//            } break;

//            case AC_NODE_TYPE_CONTACT: {
//                const QVariant qvarContactId = current->data(0, Qt::UserRole+6);

//                qstrCurrentTLA_      = QString("");
//                qstrCurrentNotary_   = QString("");
//                qstrCurrentAccount_  = QString("");
//                qstrCurrentContact_  = qvarContactId.isValid() ? qvarContactId.toString() : QString("");

//                pPmntProxyModelInbox_ ->setFilterTopLevel(qstrCurrentContact_);
//                pPmntProxyModelOutbox_->setFilterTopLevel(qstrCurrentContact_);
//            } break;

//            default: {
//                qstrCurrentTLA_      = QString("");
//                qstrCurrentNotary_   = QString("");
//                qstrCurrentAccount_  = QString("");
//                qstrCurrentContact_  = QString("");

////                qDebug() << "Setting qstrCurrentContact_ to empty string! 11 default case\n";

////                qDebug() << "NODE TYPE: " << QString::number(nNodeType);

//                pPmntProxyModelInbox_ ->setFilterNone();
//                pPmntProxyModelOutbox_->setFilterNone();
//            } break;
//        }
//    }
//    else
//    {
//        // NOTE: This may have been the bug that caused the left-hand of
//        // activity page to lose selection after a few seconds delay.
//        // Commenting it out now to test that. May keep it that way, if it fixes it.
////        qstrCurrentTLA_      = QString("");
////        qstrCurrentNotary_   = QString("");
////        qstrCurrentAccount_  = QString("");
////        qstrCurrentContact_  = QString("");

////        qDebug() << "Setting qstrCurrentContact_ to empty string! 12 (it was nullptr)\n";

////        pPmntProxyModelInbox_ ->setFilterNone();
////        pPmntProxyModelOutbox_->setFilterNone();
//    }
//    // --------------------------------------
//    RefreshPayments();
}



void Activity::onClaimsUpdatedForNym(QString nymId)
{
    if (!bRefreshingAfterUpdatedClaims_)
    {
        bRefreshingAfterUpdatedClaims_ = true;
        QTimer::singleShot(500, this, SLOT(RefreshPayments()));
    }
}


void Activity::RefreshPayments()
{
    bRefreshingAfterUpdatedClaims_ = false;
    // -------------------------------------------
    ui->tableViewSent->reset();
    ui->tableViewReceived->reset();
    // -------------------------------------------
    ACTIVITY_TREE_ITEM theItem = make_tree_item(qstrCurrentTLA_, qstrCurrentNotary_, qstrCurrentAccount_, qstrCurrentContact_);

    bool bIsInbox = (0 == ui->tabWidgetTransactions->currentIndex());
    int  nPmntID   = bIsInbox ? get_inbox_pmntid_for_tree_item(theItem) : get_outbox_pmntid_for_tree_item(theItem);

    if (0 == nPmntID) // There's no "current selected payment ID" set for this tree item.
    {
        int nRowToSelect = -1;

        if (pCurrentTabProxyModel_->rowCount() > 0) // But there ARE rows for this tree item...
            nRowToSelect = 0;

        // So let's select the first one in the list!
        QModelIndex previous = pCurrentTabTableView_->currentIndex();
        pCurrentTabTableView_->blockSignals(true);
        pCurrentTabTableView_->selectRow(nRowToSelect);
        pCurrentTabTableView_->blockSignals(false);

        if (bIsInbox)
            on_tableViewReceivedSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
        else
            on_tableViewSentSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
    }
    else // There IS a "current selected payment ID" for the current tree item.
    {
        // So let's try to select that in the tree again! (If it's still there. Otherwise set it to row 0.)

        QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

        if (pModel)
        {
            bool bFoundIt = false;

            const int nRowCount = pCurrentTabProxyModel_->rowCount();

            for (int ii = 0; ii < nRowCount; ++ii)
            {
                QModelIndex indexProxy  = pCurrentTabProxyModel_->index(ii, 0);
                QModelIndex indexSource = pCurrentTabProxyModel_->mapToSource(indexProxy);

                QSqlRecord record = pModel->record(indexSource.row());

                if (!record.isEmpty())
                {
                    QVariant the_value = record.value(PMNT_SOURCE_COL_PMNT_ID);
                    const int nRecordpmntid = the_value.isValid() ? the_value.toInt() : 0;

                    if (nRecordpmntid == nPmntID)
                    {
                        bFoundIt = true;

                        QModelIndex previous = pCurrentTabTableView_->currentIndex();
                        pCurrentTabTableView_->blockSignals(true);
                        pCurrentTabTableView_->selectRow(ii);
                        pCurrentTabTableView_->blockSignals(false);

                        if (bIsInbox)
                            on_tableViewReceivedSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
                        else
                            on_tableViewSentSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
                        break;
                    }
                }
            }
            // ------------------------------------
            if (!bFoundIt)
            {
                int nRowToSelect = -1;

                if (nRowCount > 0)
                    nRowToSelect = 0;

                QModelIndex previous = pCurrentTabTableView_->currentIndex();
                pCurrentTabTableView_->blockSignals(true);
                pCurrentTabTableView_->selectRow(nRowToSelect);
                pCurrentTabTableView_->blockSignals(false);

                if (bIsInbox)
                    on_tableViewReceivedSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
                else
                    on_tableViewSentSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
            }
        }
    }
    // -------------------------------------------
    ui->tableViewSent->resizeColumnsToContents();
    ui->tableViewSent->resizeRowsToContents();
    ui->tableViewReceived->resizeColumnsToContents();
    ui->tableViewReceived->resizeRowsToContents();
    {
    int nWidthFirstColumn = ui->tableViewSent->columnWidth(0);
    int nNewWidth = static_cast<int>( static_cast<float>(nWidthFirstColumn) * 1.2 );
    ui->tableViewSent->setColumnWidth(0,nNewWidth);
    }{
    int nWidthFirstColumn = ui->tableViewReceived->columnWidth(0);
    int nNewWidth = static_cast<int>( static_cast<float>(nWidthFirstColumn) * 1.2 );
    ui->tableViewReceived->setColumnWidth(0,nNewWidth);
    }
    ui->tableViewSent->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewReceived->horizontalHeader()->setStretchLastSection(true);
}

//void Activity::RefreshTree()
//{
//    ClearTree();
//    // ----------------------------------------
//    ui->treeWidget->blockSignals(true);
//    // ----------------------------------------
//    QList<QTreeWidgetItem *> items;
//    // ------------------------------------
//    QTreeWidgetItem * pUnfilteredItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(tr("All Payments (Unfiltered)")));
//    pUnfilteredItem->setData(0, Qt::UserRole, QVariant(0));
//    items.append(pUnfilteredItem);
//    // ----------------------------------------
//    mapIDName mapContacts;

//    if (MTContactHandler::getInstance()->GetContacts(mapContacts))
//    {
//        for (mapIDName::iterator ii = mapContacts.begin(); ii != mapContacts.end(); ii++)
//        {
//            QString qstrContactID   = ii.key();
//            QString qstrContactName = ii.value();
//            int     nContactID      = qstrContactID.toInt();
//            // ------------------------------------
//            QTreeWidgetItem * pTopItem = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(qstrContactName));
//            pTopItem->setData(0, Qt::UserRole, QVariant(nContactID));
//            items.append(pTopItem);
//            // ------------------------------------
//            mapIDName mapMethodTypes;
//            // So we can look up these names quickly without having to repeatedly hit the database
//            // for the same names over and over again.
//            MTContactHandler::getInstance()->GetMsgMethodTypes(mapMethodTypes);
//            // ------------------------------------
//            mapIDName mapTransport;

//            if (MTContactHandler::getInstance()->GetMsgMethodTypesByContact(mapTransport, nContactID, true)) // True means to add the OT servers as well.
//            {
//                for (mapIDName::iterator it_transport = mapTransport.begin(); it_transport != mapTransport.end(); it_transport++)
//                {
//                    QString qstrID   = it_transport.key();
////                  QString qstrName = it_transport.value();

//                    QStringList stringlist = qstrID.split("|");

//                    if (stringlist.size() >= 2) // Should always be 2...
//                    {
//                        QString qstrViaTransport = stringlist.at(1);
//                        QString qstrTransportName = qstrViaTransport;
//                        QString qstrMethodType = stringlist.at(0);
//                        QString qstrMethodName = qstrMethodType;

//                        mapIDName::iterator it_mapMethodTypes = mapMethodTypes.find(qstrMethodType);

//                        if (mapMethodTypes.end() != it_mapMethodTypes)
//                        {
//                            QString qstrTemp = it_mapMethodTypes.value();
//                            if (!qstrTemp.isEmpty())
//                                qstrMethodName = qstrTemp;
//                        }
//                        // ------------------------------------------------------
//                        else if (0 == QString("otserver").compare(qstrMethodType))
//                        {
//                            qstrMethodName = tr("Notary");
//                            // ------------------------------
//                            QString qstrTemp = QString::fromStdString(opentxs::OT::App().API().Exec().GetServer_Name(qstrViaTransport.toStdString()));
//                            if (!qstrTemp.isEmpty())
//                                qstrTransportName = qstrTemp;
//                        }
//                        // ------------------------------------------------------
//                        QTreeWidgetItem * pAddressItem = new QTreeWidgetItem(pTopItem, QStringList(qstrContactName) << qstrMethodName << qstrTransportName);
//                        pAddressItem->setData(0, Qt::UserRole, QVariant(nContactID));
//                        pAddressItem->setData(1, Qt::UserRole, QVariant(qstrMethodType));
//                        pAddressItem->setData(2, Qt::UserRole, QVariant(qstrViaTransport));
//                        items.append(pAddressItem);
//                    }
//                }
//            }
//        }
//        if (items.count() > 0)
//        {
//            ui->treeWidget->insertTopLevelItems(0, items);
//            ui->treeWidget->resizeColumnToContents(0);
//        }
//    }
//    // ----------------------------------------
//    // Make sure the same item that was selected before, is selected again.
//    // (If it still exists, which it probably does.)

//    QTreeWidgetItem * previous = ui->treeWidget->currentItem();

//    // In this case, just select the first thing on the list.
//    if ( (qstrCurrentContact_.isEmpty()) && qstrMethodType_.isEmpty() && qstrViaTransport_.isEmpty() )
//    {
//        if (ui->treeWidget->topLevelItemCount() > 0)
//            ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(0));
//    }
//    else // Find the one that was selected before the refresh.
//    {
//        bool bFoundIt = false;

//        QTreeWidgetItemIterator it(ui->treeWidget);

//        while (*it)
//        {
//            QVariant qvarContactID    = (*it)->data(0, Qt::UserRole);
//            QVariant qvarMethodType   = (*it)->data(1, Qt::UserRole);
//            QVariant qvarViaTransport = (*it)->data(2, Qt::UserRole);

//            const int     nContactID       = qvarContactID   .isValid() ? qvarContactID   .toInt()    :  0;
//            const QString qstrMethodType   = qvarMethodType  .isValid() ? qvarMethodType  .toString() : "";
//            const QString qstrViaTransport = qvarViaTransport.isValid() ? qvarViaTransport.toString() : "";

//            if ( (nContactID == nCurrentContact_ ) &&
//                 (0 == qstrMethodType.compare(qstrMethodType_)) &&
//                 (0 == qstrViaTransport.compare(qstrViaTransport_)) )
//            {
//                bFoundIt = true;
//                ui->treeWidget->setCurrentItem(*it);
//                break;
//            }
//            // ------------------------
//            ++it;
//        } //while

//        if (!bFoundIt)
//        {
//            if (ui->treeWidget->topLevelItemCount() > 0)
//                ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(0));
//        }
//    }
//    // ----------------------------------------
//    ui->treeWidget->blockSignals(false);
//    // ----------------------------------------
//    on_treeWidget_currentItemChanged(ui->treeWidget->currentItem(), previous);
//}

// --------------------------------------------------

void Activity::on_tableViewReceived_customContextMenuRequested(const QPoint &pos)
{
    tableViewPayments_PopupMenu(pos, ui->tableViewReceived, &(*pPmntProxyModelInbox_));
}

void Activity::on_tableViewSent_customContextMenuRequested(const QPoint &pos)
{
    tableViewPayments_PopupMenu(pos, ui->tableViewSent, &(*pPmntProxyModelOutbox_));
}

// --------------------------------------------------
// TODO resume: payments::AcceptIncoming, CancelOutgoing, DiscardOutgoingCash, and DiscardIncoming:

void Activity::AcceptIncoming(QPointer<ModelPayments> & pModel, ActivityPaymentsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView)
{
    emit showDashboard();
}

void Activity::CancelOutgoing(QPointer<ModelPayments> & pModel, ActivityPaymentsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView)
{
    emit showDashboard();
}

void Activity::DiscardOutgoingCash(QPointer<ModelPayments> & pModel, ActivityPaymentsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView)
{
    emit showDashboard();
}

void Activity::DiscardIncoming(QPointer<ModelPayments> & pModel, ActivityPaymentsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView)
{
    emit showDashboard();
}
// --------------------------------------------------

void Activity::tableViewPayments_PopupMenu(const QPoint &pos, QTableView * pTableView, ActivityPaymentsProxyModel * pProxyModel)
{
    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

    if (!pModel)
        return;
    // ------------------------
    QModelIndex indexAtRightClick = pTableView->indexAt(pos);
    if (!indexAtRightClick.isValid())
        return;
    // I can't figure out how to ADD to the selection without UNSELECTING everything else.
    // The Qt docs indicate that the below options should do that -- but it doesn't work.
    // So this is commented out since it was deselecting everything.
    //pTableView->selectionModel()->select( indexAtRightClick, QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows );
    // ------------------------
    QModelIndex sourceIndexAtRightClick = pProxyModel->mapToSource(indexAtRightClick);
    const int nRow = sourceIndexAtRightClick.row();
    // ----------------------------------
    resetPopupMenus();
    popupMenuPayments_.reset(new QMenu(this));

    pActionOpenNewWindow = popupMenuPayments_->addAction(tr("View instrument(s)"));
    pActionReply = popupMenuPayments_->addAction(tr("Reply"));
    pActionForward = popupMenuPayments_->addAction(tr("Forward"));
    popupMenuPayments_->addSeparator();
    pActionDelete = popupMenuPayments_->addAction(tr("Delete"));
    popupMenuPayments_->addSeparator();
    pActionMarkRead = popupMenuPayments_->addAction(tr("Mark as read"));
    pActionMarkUnread = popupMenuPayments_->addAction(tr("Mark as unread"));
    // ----------------------------------
    QString qstrContactId("");

    QString qstrSenderNymId;
    QString qstrSenderAddr;
    QString qstrRecipientNymId;
    QString qstrRecipientAddr;
    QString qstrMsgNotaryId;
    QString qstrPmntNotaryId;
    QString qstrMethodType;
//  QString qstrSubject;

    // new-style
    QString qstrSenderContactByNym("");
    QString qstrRecipientContactByNym("");
    QString qstrSenderContactByAddress("");
    QString qstrRecipientContactByAddress("");

    ModelPayments::PaymentFlags flags = ModelPayments::NoFlags;
    // ----------------------------------------------
    // Look at the data for indexAtRightClick and see if I have a contact already in the
    // address book. If so, add the "View Contact" option to the menu. But if not, add the
    // "Create Contact" and "Add to Existing Contact" options to the menu instead.
    //
    // UPDATE: I've now also added similar functionality, for other actions specific
    // to certain payment records, based on their flags. (Pay this invoice, deposit
    // this cash, etc.)
    //
    if (nRow >= 0)
    {
        QModelIndex indexSenderNym     = pModel->index(nRow, PMNT_SOURCE_COL_SENDER_NYM);
        QModelIndex indexSenderAddr    = pModel->index(nRow, PMNT_SOURCE_COL_SENDER_ADDR);
        QModelIndex indexRecipientNym  = pModel->index(nRow, PMNT_SOURCE_COL_RECIP_NYM);
        QModelIndex indexRecipientAddr = pModel->index(nRow, PMNT_SOURCE_COL_RECIP_ADDR);
        QModelIndex indexMsgNotaryId   = pModel->index(nRow, PMNT_SOURCE_COL_MSG_NOTARY_ID);
        QModelIndex indexPmntNotaryId  = pModel->index(nRow, PMNT_SOURCE_COL_PMNT_NOTARY_ID);
        QModelIndex indexMethodType    = pModel->index(nRow, PMNT_SOURCE_COL_METHOD_TYPE);
        QModelIndex indexFlags         = pModel->index(nRow, PMNT_SOURCE_COL_FLAGS);
//      QModelIndex indexSubject       = pModel->index(nRow, PMNT_SOURCE_COL_MEMO);

        QVariant varSenderNym     = pModel->rawData(indexSenderNym);
        QVariant varSenderAddr    = pModel->rawData(indexSenderAddr);
        QVariant varRecipientNym  = pModel->rawData(indexRecipientNym);
        QVariant varRecipientAddr = pModel->rawData(indexRecipientAddr);
        QVariant varMsgNotaryId   = pModel->rawData(indexMsgNotaryId);
        QVariant varPmntNotaryId  = pModel->rawData(indexPmntNotaryId);
        QVariant varMethodType    = pModel->rawData(indexMethodType);
        QVariant varFlags         = pModel->rawData(indexFlags);
//      QVariant varSubject       = pModel->rawData(indexSubject);

        qint64 lFlags      = varFlags        .isValid() ? varFlags        .toLongLong() : 0;
        qstrSenderNymId    = varSenderNym    .isValid() ? varSenderNym    .toString()   : QString("");
        qstrSenderAddr     = varSenderAddr   .isValid() ? varSenderAddr   .toString()   : QString("");
        qstrRecipientNymId = varRecipientNym .isValid() ? varRecipientNym .toString()   : QString("");
        qstrRecipientAddr  = varRecipientAddr.isValid() ? varRecipientAddr.toString()   : QString("");
        qstrMsgNotaryId    = varMsgNotaryId  .isValid() ? varMsgNotaryId  .toString()   : QString("");
        qstrPmntNotaryId   = varPmntNotaryId .isValid() ? varPmntNotaryId .toString()   : QString("");
        qstrMethodType     = varMethodType   .isValid() ? varMethodType   .toString()   : QString("");
//      qstrSubject        = varSubject      .isValid() ? varSubject      .toString()   : QString("");

        // new-style
        const opentxs::Identifier senderContactId    = opentxs::OT::App().Contact().ContactID(opentxs::Identifier{qstrSenderNymId.toStdString()});
        const opentxs::Identifier recipientContactId = opentxs::OT::App().Contact().ContactID(opentxs::Identifier{qstrRecipientNymId.toStdString()});
        const opentxs::String     strSenderContactId(senderContactId);
        const opentxs::String     strRecipientContactId(recipientContactId);

        qstrSenderContactByNym     = senderContactId.empty()    ? "" : QString::fromStdString(std::string(strSenderContactId.Get()));
        qstrRecipientContactByNym  = recipientContactId.empty() ? "" : QString::fromStdString(std::string(strRecipientContactId.Get()));

        qstrContactId = (!qstrSenderContactByNym.isEmpty()) ? qstrSenderContactByNym : qstrRecipientContactByNym;

        flags = ModelPayments::PaymentFlag(static_cast<ModelPayments::PaymentFlag>(lFlags));
        // -------------------------------
        popupMenuPayments_->addSeparator();
        // -------------------------------
        if (!qstrContactId.isEmpty() ) // There's a known contact for this payment.
            pActionViewContact = popupMenuPayments_->addAction(tr("View contact in address book"));
        else // There is no known contact for this payment.
        {
            pActionCreateContact = popupMenuPayments_->addAction(tr("Create new contact"));
            pActionExistingContact = popupMenuPayments_->addAction(tr("Add to existing contact"));
        }
        // -------------------------------
        popupMenuPayments_->addSeparator();
        // -------------------------------
        pActionDownloadCredentials = popupMenuPayments_->addAction(tr("Download credentials"));
        // -------------------------------
        popupMenuPayments_->addSeparator();
        // -------------------------------
        if ( flags.testFlag(ModelPayments::CanAcceptIncoming))
        {
            QString nameString;
            QString actionString;

            if ( flags.testFlag(ModelPayments::IsTransfer) )
            {
                nameString = tr("Accept this Transfer");
                actionString = tr("Accepting...");
            }
            else if ( flags.testFlag(ModelPayments::IsReceipt) )
            {
                nameString = tr("Accept this Receipt");
                actionString = tr("Accepting...");
            }
            else if ( flags.testFlag(ModelPayments::IsInvoice) )
            {
                nameString = tr("Pay this Invoice");
                actionString = tr("Paying...");
            }
            else if ( flags.testFlag(ModelPayments::IsPaymentPlan) )
            {
                nameString = tr("Activate this Payment Plan");
                actionString = tr("Activating...");
            }
            else if ( flags.testFlag(ModelPayments::IsContract) )
            {
                nameString = tr("Sign this Smart Contract");
                actionString = tr("Signing...");
            }
            else if ( flags.testFlag(ModelPayments::IsCash) )
            {
                nameString = tr("Deposit this Cash");
                actionString = tr("Depositing...");
            }
            else if ( flags.testFlag(ModelPayments::IsCheque) )
            {
                nameString = tr("Deposit this Cheque");
                actionString = tr("Depositing...");
            }
            else if ( flags.testFlag(ModelPayments::IsVoucher) )
            {
                nameString = tr("Accept this Payment");
                actionString = tr("Accepting...");
            }
            else
            {
                nameString = tr("Deposit this Payment");
                actionString = tr("Depositing...");
            }

            pActionAcceptIncoming = popupMenuPayments_->addAction(nameString);
        }

        if (flags.testFlag(ModelPayments::CanCancelOutgoing))
        {
            QString cancelString;
            QString actionString = tr("Canceling...");
//          QString msg = tr("Cancellation Failed. Perhaps recipient had already accepted it?");

            if (flags.testFlag(ModelPayments::IsInvoice))
                cancelString = tr("Cancel this Invoice");
            else if (flags.testFlag(ModelPayments::IsPaymentPlan))
                cancelString = tr("Cancel this Payment Plan");
            else if (flags.testFlag(ModelPayments::IsContract))
                cancelString = tr("Cancel this Smart Contract");
            else if (flags.testFlag(ModelPayments::IsCash))
            {
                cancelString = tr("Recover this Cash");
                actionString = tr("Recovering...");
//              msg = tr("Recovery Failed. Perhaps recipient had already accepted it?");
            }
            else if (flags.testFlag(ModelPayments::IsCheque))
                cancelString = tr("Cancel this Cheque");
            else if (flags.testFlag(ModelPayments::IsVoucher))
                cancelString = tr("Cancel this Payment");
            else
                cancelString = tr("Cancel this Payment");

            pActionCancelOutgoing = popupMenuPayments_->addAction(cancelString);
        }

        if (flags.testFlag(ModelPayments::CanDiscardOutgoingCash))
        {
            QString discardString = tr("Discard this Sent Cash");

            pActionDiscardOutgoingCash = popupMenuPayments_->addAction(discardString);
        }

        if (flags.testFlag(ModelPayments::CanDiscardIncoming))
        {
            QString discardString;

            if (flags.testFlag(ModelPayments::IsInvoice))
                discardString = tr("Discard this Invoice");
            else if (flags.testFlag(ModelPayments::IsPaymentPlan))
                discardString = tr("Discard this Payment Plan");
            else if (flags.testFlag(ModelPayments::IsContract))
                discardString = tr("Discard this Smart Contract");
            else if (flags.testFlag(ModelPayments::IsCash))
                discardString = tr("Discard this Cash");
            else if (flags.testFlag(ModelPayments::IsCheque))
                discardString = tr("Discard this Cheque");
            else if (flags.testFlag(ModelPayments::IsVoucher))
                discardString = tr("Discard this Payment");
            else
                discardString = tr("Discard this Payment");

            pActionDiscardIncoming = popupMenuPayments_->addAction(discardString);
        }
    }
    // --------------------------------------------------
    QPoint globalPos = pTableView->mapToGlobal(pos);
    const QAction* selectedAction = popupMenuPayments_->exec(globalPos); // Here we popup the menu, and get the user's click.
    if (nullptr == selectedAction)
        return;
    // ----------------------------------
    if (selectedAction == pActionAcceptIncoming) // Only approves the current payment.
    {
        pTableView->setCurrentIndex(indexAtRightClick);
        AcceptIncoming(pModel, pProxyModel, nRow, pTableView);
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionCancelOutgoing) // Only cancels the current payment.
    {
        pTableView->setCurrentIndex(indexAtRightClick);
        CancelOutgoing(pModel, pProxyModel, nRow, pTableView);
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionDiscardOutgoingCash) // Only discards the current payment.
    {
        pTableView->setCurrentIndex(indexAtRightClick);
        DiscardOutgoingCash(pModel, pProxyModel, nRow, pTableView);
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionDiscardIncoming) // Only discards the current payment.
    {
        pTableView->setCurrentIndex(indexAtRightClick);
        DiscardIncoming(pModel, pProxyModel, nRow, pTableView);
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionReply) // Only replies to the current payment.
    {
        pTableView->setCurrentIndex(indexAtRightClick);
        on_toolButtonReply_clicked();
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionForward) // Only fowards the current payments.
    {
        pTableView->setCurrentIndex(indexAtRightClick);
        on_toolButtonForward_clicked();
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionDelete) // May delete many payments.
    {
        on_toolButtonDelete_clicked();
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionOpenNewWindow) // May open many payments.
    {
        pTableView->setCurrentIndex(indexAtRightClick);

        if (pTableView == ui->tableViewReceived)
            on_tableViewReceived_doubleClicked(indexAtRightClick); // just one for now. baby steps!
        else if (pTableView == ui->tableViewSent)
            on_tableViewSent_doubleClicked(indexAtRightClick); // just one for now. baby steps!
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionMarkRead) // May mark many payments.
    {
        if (!pTableView->selectionModel()->hasSelection())
            return;
        // ----------------------------------------------
        QItemSelection selection( pTableView->selectionModel()->selection() );
        QList<int> rows;
        foreach( const QModelIndex & index, selection.indexes() )
        {
            if (rows.indexOf(index.row()) != (-1)) // This row is already on the list, so skip it.
                continue;
            rows.append(index.row());
            // -----------------------
            QModelIndex sourceIndex = pProxyModel->mapToSource(index);
            QModelIndex sourceIndexHaveRead = pModel->sibling(sourceIndex.row(), PMNT_SOURCE_COL_HAVE_READ, sourceIndex);
            // --------------------------------
            if (sourceIndexHaveRead.isValid())
                listRecordsToMarkAsRead_.append(sourceIndexHaveRead);
        }
        if (listRecordsToMarkAsRead_.count() > 0)
            QTimer::singleShot(0, this, SLOT(on_MarkAsRead_timer()));
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionMarkUnread) // May mark many payments.
    {
        if (!pTableView->selectionModel()->hasSelection())
            return;
        // ----------------------------------------------
        QItemSelection selection( pTableView->selectionModel()->selection() );
        QList<int> rows;
        foreach( const QModelIndex & index, selection.indexes() )
        {
            if (rows.indexOf(index.row()) != (-1)) // This row is already on the list, so skip it.
                continue;
            rows.append(index.row());
            // -----------------------
            QModelIndex sourceIndex = pProxyModel->mapToSource(index);
            QModelIndex sourceIndexHaveRead = pModel->sibling(sourceIndex.row(), PMNT_SOURCE_COL_HAVE_READ, sourceIndex);
            // --------------------------------
            if (sourceIndexHaveRead.isValid())
                listRecordsToMarkAsUnread_.append(sourceIndexHaveRead);
        }
        if (listRecordsToMarkAsUnread_.count() > 0)
            QTimer::singleShot(0, this, SLOT(on_MarkAsUnread_timer()));
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionViewContact)
    {
        pTableView->setCurrentIndex(indexAtRightClick);

        if (!qstrContactId.isEmpty())
        {
            emit showContact(qstrContactId);
        }
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionCreateContact)
    {
        pTableView->setCurrentIndex(indexAtRightClick);

        MTGetStringDialog nameDlg(this, tr("Enter a name for the new contact"));

        if (QDialog::Accepted != nameDlg.exec())
            return;
        // --------------------------------------
        QString strNewContactName = nameDlg.GetOutputString();
        // --------------------------------------------------
        // NOTE:
        // if nSenderContactByNym > 0, then the sender Nym already has a contact.
        // else if nSenderContactByNym == 0 but qstrSenderNymId exists, that means it
        // contains a NymID that could be added to an existing contact, or used to
        // create a new contact. (And the same is true for the Sender Address.)
        //
        // (And the same is also true for the recipient nymID and address.)
        //
        if (qstrSenderContactByNym.isEmpty() && !qstrSenderNymId.isEmpty())
            qstrContactId = MTContactHandler::getInstance()->GetOrCreateOpentxsContactBasedOnNym(strNewContactName, qstrSenderNymId);
        else if (qstrRecipientContactByNym.isEmpty() && !qstrRecipientNymId.isEmpty())
            qstrContactId = MTContactHandler::getInstance()->GetOrCreateOpentxsContactBasedOnNym(strNewContactName, qstrRecipientNymId);
        // -----------------------------------------------------
        if (!qstrContactId.isEmpty())
        {
            emit showContactAndRefreshHome(qstrContactId);
        }
        return;
    }
    // ----------------------------------
    else if (selectedAction == pActionDownloadCredentials)
    {
        pTableView->setCurrentIndex(indexAtRightClick);

        const bool bHaveContact = !qstrContactId.isEmpty();
        mapIDName mapNymIds;

        if (bHaveContact)
        {
            MTContactHandler::getInstance()->GetNyms(mapNymIds, qstrContactId.toStdString());

            // Check to see if there is more than one Nym for this contact.
            // TODO: If so, get the user to select one of the Nyms, or give him the
            // option to do them all.
            // (Until then, we're just going to do them all.)
        }
        // ---------------------------------------------------
        QString qstrNymId;

        if      (!qstrSenderNymId.isEmpty())    qstrNymId   = qstrSenderNymId;
        else if (!qstrRecipientNymId.isEmpty()) qstrNymId   = qstrRecipientNymId;
        // ---------------------------------------------------
        // Might not have a contact. Even if we did, he might not have any NymIds.
        // Here, if there are no known NymIds, but there's one on the message,
        // then we add it to the map.
        if ( (0 == mapNymIds.size()) && (qstrNymId.size() > 0) )
        {
            mapNymIds.insert(qstrNymId, QString("Name not used here"));
        }
        // ---------------------------------------------------
        if (0 == mapNymIds.size())
        {
            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), tr("Unable to find a NymId for this message. (Unable to download credentials without Id.)"));
            qDebug() << "Unable to find a NymId for this message. (Unable to download credentials without Id.)";
            return;
        }
        // Below this point we're guaranteed that there's at least one NymID.
        // ---------------------------------------------------
        int nFound = 0;
        for (mapIDName::iterator
             it_nyms  = mapNymIds.begin();
             it_nyms != mapNymIds.end();
             ++it_nyms)
        {
            nFound++;
            emit needToCheckNym("", it_nyms.key(), qstrMsgNotaryId);
        }
    }
    // ----------------------------------
    else if (selectedAction == pActionExistingContact)
    {
        pTableView->setCurrentIndex(indexAtRightClick);

        // This should never happen since we wouldn't even have gotten this menu option
        // in the first place, unless contact ID had been 0.
        if (!qstrContactId.isEmpty())
            return;

        // (And that means no contact was found for ANY of the Nym IDs or Addresses on this payment.)
        // That means we can add the first one we find (which will probably be the only one as well.)
        // Because I'll EITHER have a SenderNymID OR SenderAddress,
        // ...OR I'll have a RecipientNymID OR RecipientAddress.
        // Thus, only one of the four IDs/Addresses will actually be found.
        // Therefore I don't care which one I find first:
        //
        QString qstrNymId;

        if      (!qstrSenderNymId.isEmpty())    qstrNymId   = qstrSenderNymId;
        else if (!qstrRecipientNymId.isEmpty()) qstrNymId   = qstrRecipientNymId;
        // ---------------------------------------------------
        if (qstrNymId.isEmpty()) // Should never happen.
            return;
        // Below this point we're guaranteed that there's a NymID.
        // ---------------------------------------------------
        const opentxs::Identifier contactId    = opentxs::OT::App().Contact().ContactID(opentxs::Identifier{qstrNymId.toStdString()});

        if (!contactId.empty())
        {
            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                 tr("Strange: NymID %1 already belongs to an existing contact.").arg(qstrNymId));
            return;
        }
        // ---------------------------------------------------
        // Pop up a Contact selection box. The user chooses an existing contact.
        // If OK (vs Cancel) then add the Nym / Acct to the existing contact selected.
        //
        DlgChooser theChooser(this);
        // -----------------------------------------------
        mapIDName & the_map = theChooser.m_map;
        MTContactHandler::getInstance()->GetOpentxsContacts(the_map);
        // -----------------------------------------------
        theChooser.setWindowTitle(tr("Choose an Existing Contact"));
        if (theChooser.exec() != QDialog::Accepted)
            return;
        // -----------------------------------------------
        QString strContactID = theChooser.GetCurrentID();

        if (!strContactID.isEmpty())
        {
            if (!qstrNymId.isEmpty()) // We're adding this NymID to the contact.
            {
                const bool bAdded = false;
                if (!bAdded) // Todo.
                    /*
                     * Justus:
                       - opentxs::Identifier ContactManager::ContactID(const opentxs::Identifier& nymID) const
                         That will tell you if a nym is associated with a contact

                       - bool Contact::AddNym(const opentxs::Identifier& nymID, const bool primary);
                         Will add it to an existing contact
                    */

//              if (!MTContactHandler::getInstance()->AddNymToExistingContact(nContactId, qstrNymId))
                {
                    QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                         tr("TODO: Tried to add NymID %1 to an existing contact but I don't know the API call to use.").arg(qstrNymId));
                    return;
                }
                else if (!qstrMsgNotaryId.isEmpty())
                    MTContactHandler::getInstance()->NotifyOfNymServerPair(qstrNymId, qstrMsgNotaryId);
            }
            // ---------------------------------
            // Display the normal contacts dialog, with the new contact
            // being the one selected.
            //
            emit showContactAndRefreshHome(strContactID);
            // ---------------------------------
        } // (!strContactID.isEmpty())
    }
}


void Activity::on_tableViewReceived_doubleClicked(const QModelIndex &index)
{
    tableViewPayments_DoubleClicked(index, &(*pPmntProxyModelInbox_));
}

void Activity::on_tableViewSent_doubleClicked(const QModelIndex &index)
{
    tableViewPayments_DoubleClicked(index, &(*pPmntProxyModelOutbox_));
}

void Activity::tableViewPayments_DoubleClicked(const QModelIndex &index, ActivityPaymentsProxyModel * pProxyModel)
{
    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

    if (!pModel)
        return;

    if (!index.isValid())
        return;

    QModelIndex sourceIndex = pProxyModel->mapToSource(index);

    if (!sourceIndex.isValid())
        return;
    // -------------------------------
    QModelIndex pmntidIndex   = pModel->sibling(sourceIndex.row(), PMNT_SOURCE_COL_PMNT_ID, sourceIndex);
    QModelIndex subjectIndex = pModel->sibling(sourceIndex.row(), PMNT_SOURCE_COL_MEMO, sourceIndex);

    QVariant qvarpmntid   = pModel->data(pmntidIndex);
    QVariant qvarSubject = pModel->data(subjectIndex);

    int     nPaymentID  = qvarpmntid.isValid() ? qvarpmntid.toInt() : 0;
    QString qstrSubject = qvarSubject.isValid() ? qvarSubject.toString() : "";
    // -------------------------------
    QString qstrPayment, qstrPending, qstrType, qstrSubtitle;
    // --------------------------------------------------
    if (nPaymentID > 0)
    {
        qstrPayment = MTContactHandler::getInstance()->GetPaymentBody(nPaymentID);
        qstrPending = MTContactHandler::getInstance()->GetPaymentPendingBody(nPaymentID);
    }
    // --------------------------------------------------
    QModelIndex myNymIndex        = pModel->sibling(sourceIndex.row(), PMNT_SOURCE_COL_MY_NYM, sourceIndex);
    QModelIndex senderNymIndex    = pModel->sibling(sourceIndex.row(), PMNT_SOURCE_COL_SENDER_NYM, sourceIndex);
    QModelIndex recipientNymIndex = pModel->sibling(sourceIndex.row(), PMNT_SOURCE_COL_RECIP_NYM, sourceIndex);

    QModelIndex myNymProxyIndex        = pProxyModel->mapFromSource(myNymIndex);
    QModelIndex senderNymProxyIndex    = pProxyModel->mapFromSource(senderNymIndex);
    QModelIndex recipientNymProxyIndex = pProxyModel->mapFromSource(recipientNymIndex);

    QVariant qvarMyNymName        = myNymProxyIndex.isValid()        ? pProxyModel->data(myNymProxyIndex) : QString("");
    QVariant qvarSenderNymName    = senderNymProxyIndex.isValid()    ? pProxyModel->data(senderNymProxyIndex) : QString("");
    QVariant qvarRecipientNymName = recipientNymProxyIndex.isValid() ? pProxyModel->data(recipientNymProxyIndex) : QString("");

    QString qstrMyNymName        = qvarMyNymName.isValid() ? qvarMyNymName.toString() : "";
    QString qstrSenderNymName    = qvarSenderNymName.isValid() ? qvarSenderNymName.toString() : "";
    QString qstrRecipientNymName = qvarRecipientNymName.isValid() ? qvarRecipientNymName.toString() : "";

    if (!qstrSenderNymName.isEmpty())
    {
        qstrType = QString("%1: %2").arg(tr("To")).arg(qstrMyNymName);
        qstrSubtitle = QString("%1: %2").arg(tr("From")).arg(qstrSenderNymName);
    }
    else if (!qstrRecipientNymName.isEmpty())
    {
        qstrType = QString("%1: %2").arg(tr("To")).arg(qstrRecipientNymName);
        qstrSubtitle = QString("%1: %2").arg(tr("From")).arg(qstrMyNymName);
    }
    else
    {
        qstrType = QString("Instrument:");
        qstrSubtitle = QString(" ");
    }
    // -----------
    // Pop up the result dialog.
    //
    if (qstrPayment.isEmpty() || qstrPending.isEmpty())
    {
        DlgExportedToPass dlgExported(this, qstrPayment.isEmpty() ? qstrPending : qstrPayment,
                                      qstrType,
                                      qstrSubtitle, false);
        dlgExported.setWindowTitle(QString("%1: %2").arg(tr("Memo")).arg(qstrSubject));
        dlgExported.exec();
    }
    else
    {
        DlgExportedCash dlgExported(this, qstrPending, qstrPayment,
                                    tr("Receipt:"), QString(" "),
                                    qstrType,
                                    qstrSubtitle, false);
        dlgExported.setWindowTitle(QString("%1: %2").arg(tr("Memo")).arg(qstrSubject));
        dlgExported.exec();
    }
}


void Activity::on_toolButtonReply_clicked()
{
    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

    if (!pModel)
        return;
    // ------------------------------------
    QModelIndex proxyIndex = pCurrentTabTableView_->currentIndex();

    if (!proxyIndex.isValid())
        return;
    // ------------------------------------
    QModelIndex sourceIndex = pCurrentTabProxyModel_->mapToSource(proxyIndex);

    if (!sourceIndex.isValid())
        return;

    QModelIndex haveRepliedIndex = pModel->sibling(sourceIndex.row(),
                                                   PMNT_SOURCE_COL_HAVE_REPLIED,
                                                   sourceIndex);
    // ------------------------------------
    QSqlRecord record = pModel->record(sourceIndex.row());

    if (record.isEmpty())
        return;
    // ------------------------------------
    const int nPaymentID = record.value(PMNT_SOURCE_COL_PMNT_ID).isValid() ? record.value(PMNT_SOURCE_COL_PMNT_ID).toInt() : 0;

    const bool bOutgoing = (0 == record.value(PMNT_SOURCE_COL_FOLDER).toInt());

    const QVariant qvar_method_type = record.value(PMNT_SOURCE_COL_METHOD_TYPE);
    const QString  methodType = qvar_method_type.isValid() ? qvar_method_type.toString() : "";

    const QVariant qvar_my_nym_id = record.value(PMNT_SOURCE_COL_MY_NYM);
    const QString  myNymID = qvar_my_nym_id.isValid() ? qvar_my_nym_id.toString() : "";

    const QVariant qvar_my_addr = record.value(PMNT_SOURCE_COL_MY_ADDR);
    const QString  myAddress = qvar_my_addr.isValid() ? qvar_my_addr.toString() : "";

    const QVariant qvar_sender_nym_id = record.value(PMNT_SOURCE_COL_SENDER_NYM);
    const QString  senderNymID = qvar_sender_nym_id.isValid() ? qvar_sender_nym_id.toString() : "";

    const QVariant qvar_recipient_nym_id = record.value(PMNT_SOURCE_COL_RECIP_NYM);
    const QString  recipientNymID = qvar_recipient_nym_id.isValid() ? qvar_recipient_nym_id.toString() : "";

    const QVariant qvar_msg_notary_id = record.value(PMNT_SOURCE_COL_MSG_NOTARY_ID);
    const QVariant qvar_pmnt_notary_id = record.value(PMNT_SOURCE_COL_PMNT_NOTARY_ID);
    const QString  MsgNotaryID = qvar_msg_notary_id.isValid() ? qvar_msg_notary_id.toString() : "";
    const QString  PmntNotaryID = qvar_pmnt_notary_id.isValid() ? qvar_pmnt_notary_id.toString() : "";

    const QVariant qvar_sender_addr = record.value(PMNT_SOURCE_COL_SENDER_ADDR);
    const QString  senderAddr = qvar_sender_addr.isValid() ? qvar_sender_addr.toString() : "";

    const QVariant qvar_recipient_addr = record.value(PMNT_SOURCE_COL_RECIP_ADDR);
    const QString  recipientAddr = qvar_recipient_addr.isValid() ? qvar_recipient_addr.toString() : "";

    const QVariant qvar_subject = record.value(PMNT_SOURCE_COL_MEMO);
    const QString  subject = qvar_subject.isValid() ? MTContactHandler::getInstance()->Decode(qvar_subject.toString()) : "";
    // --------------------------------------------------
    const QString& otherNymID = bOutgoing ? recipientNymID : senderNymID;
    const QString& otherAddress  = bOutgoing ? recipientAddr  : senderAddr;
    // --------------------------------------------------
    const bool bUsingNotary   = !MsgNotaryID.isEmpty();
    const bool bIsSpecialMail = !bUsingNotary;
    // --------------------------------------------------
    MTCompose * compose_window = new MTCompose;
    compose_window->setAttribute(Qt::WA_DeleteOnClose);
    // --------------------------------------------------
    if (!myNymID.isEmpty()) // If there's a nym ID.
    {
        if (!myAddress.isEmpty())
            compose_window->setInitialSenderNym(myNymID, myAddress);
        else
            compose_window->setInitialSenderNym(myNymID);
    }
    else if (!myAddress.isEmpty())
        compose_window->setInitialSenderAddress(myAddress);
    // ---------------------------------------
    if (!otherNymID.isEmpty()) // If there's an "other nym ID".
    {
        if (!otherAddress.isEmpty())
            compose_window->setInitialRecipientNym(otherNymID, otherAddress);
        else
            compose_window->setInitialRecipientNym(otherNymID);
    }
    else if (!otherAddress.isEmpty())
        compose_window->setInitialRecipientAddress(otherAddress);
    // --------------------------------------------------
    if (bUsingNotary)
        compose_window->setInitialServer(MsgNotaryID);
    // --------------------------------------------------
    compose_window->setInitialSubject(subject);
    // --------------------------------------------------
    if (nPaymentID > 0)
    {
        QString body        = MTContactHandler::getInstance()->GetPaymentBody(nPaymentID);
        QString pendingBody = MTContactHandler::getInstance()->GetPaymentPendingBody(nPaymentID);

        if (!body.isEmpty())
            compose_window->setInitialBody(body);
        else if (!pendingBody.isEmpty())
            compose_window->setInitialBody(pendingBody);
    }
    // --------------------------------------------------
    compose_window->setVariousIds(
                            bOutgoing ? myNymID : senderNymID,
                            bOutgoing ? recipientNymID : myNymID,
                            bOutgoing ? myAddress : senderAddr,
                            bOutgoing ? recipientAddr : myAddress);
    // --------------------------------------------------
    compose_window->dialog();
    Focuser f(compose_window);
    f.show();
    f.focus();
    // -----------------------------
    if (haveRepliedIndex.isValid())
        listRecordsToMarkAsReplied_.append(haveRepliedIndex);
    if (listRecordsToMarkAsReplied_.count() > 0)
        QTimer::singleShot(0, this, SLOT(on_MarkAsReplied_timer()));
}

void Activity::on_toolButtonForward_clicked()
{
    QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

    if (!pModel)
        return;
    // ------------------------------------
    QModelIndex proxyIndex = pCurrentTabTableView_->currentIndex();

    if (!proxyIndex.isValid())
        return;
    // ------------------------------------
    QModelIndex sourceIndex = pCurrentTabProxyModel_->mapToSource(proxyIndex);

    if (!sourceIndex.isValid())
        return;

    QModelIndex haveForwardedIndex = pModel->sibling(sourceIndex.row(),
                                                     PMNT_SOURCE_COL_HAVE_FORWARDED,
                                                     sourceIndex);
    // ------------------------------------
    QSqlRecord record = pModel->record(sourceIndex.row());

    if (record.isEmpty())
        return;
    // ------------------------------------
    const int nPaymentID = record.value(PMNT_SOURCE_COL_PMNT_ID).isValid() ? record.value(PMNT_SOURCE_COL_PMNT_ID).toInt() : 0;

    const bool bOutgoing = (0 == record.value(PMNT_SOURCE_COL_FOLDER).toInt());

    const QVariant qvar_method_type = record.value(PMNT_SOURCE_COL_METHOD_TYPE);
    const QString  methodType = qvar_method_type.isValid() ? qvar_method_type.toString() : "";

    const QVariant qvar_my_nym_id = record.value(PMNT_SOURCE_COL_MY_NYM);
    const QString  myNymID = qvar_my_nym_id.isValid() ? qvar_my_nym_id.toString() : "";

    const QVariant qvar_my_addr = record.value(PMNT_SOURCE_COL_MY_ADDR);
    const QString  myAddress = qvar_my_addr.isValid() ? qvar_my_addr.toString() : "";

    const QVariant qvar_sender_nym_id = record.value(PMNT_SOURCE_COL_SENDER_NYM);
    const QString  senderNymID = qvar_sender_nym_id.isValid() ? qvar_sender_nym_id.toString() : "";

    const QVariant qvar_recipient_nym_id = record.value(PMNT_SOURCE_COL_RECIP_NYM);
    const QString  recipientNymID = qvar_recipient_nym_id.isValid() ? qvar_recipient_nym_id.toString() : "";

    const QVariant qvar_msg_notary_id = record.value(PMNT_SOURCE_COL_MSG_NOTARY_ID);
    const QVariant qvar_pmnt_notary_id = record.value(PMNT_SOURCE_COL_PMNT_NOTARY_ID);
    const QString  MsgNotaryID = qvar_msg_notary_id.isValid() ? qvar_msg_notary_id.toString() : "";
    const QString  Pmnt_NotaryID = qvar_pmnt_notary_id.isValid() ? qvar_pmnt_notary_id.toString() : "";

    const QVariant qvar_sender_addr = record.value(PMNT_SOURCE_COL_SENDER_ADDR);
    const QString  senderAddr = qvar_sender_addr.isValid() ? qvar_sender_addr.toString() : "";

    const QVariant qvar_recipient_addr = record.value(PMNT_SOURCE_COL_RECIP_ADDR);
    const QString  recipientAddr = qvar_recipient_addr.isValid() ? qvar_recipient_addr.toString() : "";

    const QVariant qvar_subject = record.value(PMNT_SOURCE_COL_MEMO);
    const QString  subject = qvar_subject.isValid() ? MTContactHandler::getInstance()->Decode(qvar_subject.toString()) : "";
    // --------------------------------------------------
    const QString& otherNymID = bOutgoing ? recipientNymID : senderNymID;
    const QString& otherAddress  = bOutgoing ? recipientAddr  : senderAddr;
    // --------------------------------------------------
    const bool bUsingNotary   = !MsgNotaryID.isEmpty();
    const bool bIsSpecialMail = !bUsingNotary;
    // --------------------------------------------------
    MTCompose * compose_window = new MTCompose;
    compose_window->setAttribute(Qt::WA_DeleteOnClose);
    // --------------------------------------------------
    if (!myNymID.isEmpty()) // If there's a nym ID.
    {
        if (!myAddress.isEmpty())
            compose_window->setInitialSenderNym(myNymID, myAddress);
        else
            compose_window->setInitialSenderNym(myNymID);
    }
    else if (!myAddress.isEmpty())
        compose_window->setInitialSenderAddress(myAddress);
    // ---------------------------------------
//    if (!otherNymID.isEmpty()) // If there's an "other nym ID".
//    {
//        if (!otherAddress.isEmpty())
//            compose_window->setInitialRecipientNym(otherNymID, otherAddress);
//        else
//            compose_window->setInitialRecipientNym(otherNymID);
//    }
//    else if (!otherAddress.isEmpty())
//        compose_window->setInitialRecipientAddress(otherAddress);
    // --------------------------------------------------
    if (bUsingNotary)
        compose_window->setInitialServer(MsgNotaryID);
    // --------------------------------------------------
    compose_window->setInitialSubject(subject);
    // --------------------------------------------------
    if (nPaymentID > 0)
    {
        QString body        = MTContactHandler::getInstance()->GetPaymentBody(nPaymentID);
        QString pendingBody = MTContactHandler::getInstance()->GetPaymentPendingBody(nPaymentID);

        if (!body.isEmpty())
            compose_window->setInitialBody(body);
        else if (!pendingBody.isEmpty())
            compose_window->setInitialBody(pendingBody);
    }
    // --------------------------------------------------
    compose_window->setForwarded();
    // --------------------------------------------------
    compose_window->setVariousIds(
                            bOutgoing ? myNymID : senderNymID,
                            bOutgoing ? recipientNymID : myNymID,
                            bOutgoing ? myAddress : senderAddr,
                            bOutgoing ? recipientAddr : myAddress);
    // --------------------------------------------------
    compose_window->dialog();
    Focuser f(compose_window);
    f.show();
    f.focus();
    // -----------------------------
    if (haveForwardedIndex.isValid())
        listRecordsToMarkAsForwarded_.append(haveForwardedIndex);
    if (listRecordsToMarkAsForwarded_.count() > 0)
        QTimer::singleShot(0, this, SLOT(on_MarkAsForwarded_timer()));
}



void Activity::on_toolButtonDelete_clicked()
{
    if ( (nullptr != pCurrentTabTableView_) &&
         (nullptr != pCurrentTabProxyModel_) )
    {
        if (!pCurrentTabTableView_->selectionModel()->hasSelection())
            return;
        // ----------------------------------------------
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, tr(MONEYCHANGER_APP_NAME), QString("%1<br/><br/>%2").arg(tr("Are you sure you want to delete these receipts?")).
                                      arg(tr("WARNING: This is not reversible!")),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply != QMessageBox::Yes)
            return;
        // ----------------------------------------------
        QPointer<ModelPayments> pModel = DBHandler::getInstance()->getPaymentModel();

        if (pModel)
        {
            QItemSelection selection( pCurrentTabTableView_->selectionModel()->selection() );

            int nFirstProxyRowRemoved = -1;
            int nLastProxyRowRemoved  = -1;
            int nCountRowsRemoved     = 0;

            QList<int> rows, payment_ids;
            foreach( const QModelIndex & index, selection.indexes() ) {
                QModelIndex sourceIndex = pCurrentTabProxyModel_->mapToSource(index);
                rows.append( sourceIndex.row() );
                // --------------------------------
                nLastProxyRowRemoved = index.row();
                if ((-1) == nFirstProxyRowRemoved)
                    nFirstProxyRowRemoved = index.row();
            }

            qSort( rows );

            bool bRemoved = false;

            int prev = -1;

            for(int ii = rows.count() - 1; ii >= 0; ii -= 1 ) {
               int current = rows[ii];
               if( current != prev ) {
                   bRemoved = true;
                   QModelIndex sourceIndexpmntid = pModel->index(current, PMNT_SOURCE_COL_PMNT_ID);
                   if (sourceIndexpmntid.isValid())
                       payment_ids.append(pModel->data(sourceIndexpmntid).toInt());
                   pModel->removeRows( current, 1 );
                   prev = current;
                   nCountRowsRemoved++;
               }
            }

            if (bRemoved)
            {
                if (pModel->submitAll())
                {
                    pModel->database().commit();
                    // ------------------------
                    // Now we just deleted some receipts; let's delete also the corresponding
                    // receipt contents. (We saved the deleted IDs for this purpose.)
                    //
                    for (int ii = 0; ii < payment_ids.count(); ++ii)
                    {
                        const int nPmntID = payment_ids[ii];

                        if (nPmntID > 0)
                            if (!MTContactHandler::getInstance()->DeletePaymentBody(nPmntID))
                                qDebug() << "Payments::on_toolButtonDelete_clicked: Failed trying to delete payment body with payment_id: " << nPmntID << "\n";
                    }
                    // ------------------------
                    // We just deleted the selected rows.
                    // So now we need to choose another row to select.

                    int nRowToSelect = -1;

                    if ((nFirstProxyRowRemoved >= 0) && (nFirstProxyRowRemoved < pCurrentTabProxyModel_->rowCount()))
                        nRowToSelect = nFirstProxyRowRemoved;
                    else if (0 == nFirstProxyRowRemoved)
                        nRowToSelect = 0;
                    else if (nFirstProxyRowRemoved > 0)
                        nRowToSelect = pCurrentTabProxyModel_->rowCount() - 1;
                    else
                        nRowToSelect = 0;

                    if ((pCurrentTabProxyModel_->rowCount() > 0) && (nRowToSelect >= 0) &&
                            (nRowToSelect < pCurrentTabProxyModel_->rowCount()))
                    {
                        QModelIndex previous = pCurrentTabTableView_->currentIndex();
                        pCurrentTabTableView_->blockSignals(true);
                        pCurrentTabTableView_->selectRow(nRowToSelect);
                        pCurrentTabTableView_->blockSignals(false);

                        if (pCurrentTabTableView_ == ui->tableViewReceived)
                            on_tableViewReceivedSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
                        else
                            on_tableViewSentSelectionModel_currentRowChanged(pCurrentTabTableView_->currentIndex(), previous);
                    }
                }
                else
                {
                    pModel->database().rollback();
                    qDebug() << "Database Write Error" <<
                               "The database reported an error: " <<
                               pModel->lastError().text();
                }
            }
        }
    }
}

void Activity::on_toolButtonRefresh_clicked()
{
    emit needToDownloadAccountData();
}

void Activity::onRecordlistPopulated()
{
    RefreshAll();
}

void Activity::onBalancesChanged()
{
    RefreshUserBar();
}

// The balances hasn't necessarily changed.
// (Perhaps the default account was changed.)
void Activity::onNeedToRefreshUserBar()
{
    RefreshUserBar();
}

void Activity::onNeedToRefreshRecords()
{
    RefreshAll();
}


void Activity::on_pushButtonSearchConversations_clicked()
{
    QString qstrSearchText = ui->lineEditSearchConversations->text();

    this->doSearchConversations(qstrSearchText.simplified());
}


void Activity::doSearchConversations(QString qstrInput)
{
    if (pThreadItemsProxyModel_)
    {
        pThreadItemsProxyModel_->setFilterString(qstrInput);
    }

//  RefreshConversationsTab(); // NOTE: Possibly not necessary since setting the filter string might be all it takes in this case.
}

void Activity::doSearchPayments(QString qstrInput)
{
    if (pPmntProxyModelInbox_)
    {
        pPmntProxyModelInbox_ ->setFilterString(qstrInput);
    }
    if (pPmntProxyModelOutbox_)
    {
        pPmntProxyModelOutbox_->setFilterString(qstrInput);
    }

    RefreshPayments();
}

void Activity::on_lineEditSearchPayments_textChanged(const QString &arg1)
{
    // This means someone clicked the "clear" button on the search box.
    if (arg1.isEmpty())
        doSearchPayments(arg1);
}

void Activity::on_lineEditSearchConversations_textChanged(const QString &arg1)
{
    // This means someone clicked the "clear" button on the search box.
    if (arg1.isEmpty())
        doSearchConversations(arg1);
}

void Activity::on_lineEditSearchConversations_returnPressed()
{
    QString qstrSearchText = ui->lineEditSearchConversations->text();

    this->doSearchConversations(qstrSearchText.simplified());
}

void Activity::on_treeWidgetSummary_customContextMenuRequested(const QPoint &pos)
{
    treeWidgetSummary_PopupMenu(pos, ui->treeWidgetSummary);
}

void Activity::resetPopupMenus()
{
    if (popupMenuPayments_) {
        popupMenuPayments_->close();
    }
    if (popupMenuAccounts_) {
        popupMenuAccounts_->close();
    }

    popupMenuPayments_.reset();
    popupMenuAccounts_.reset();

    pActionViewContact         = nullptr;
    pActionCreateContact       = nullptr;
    pActionExistingContact     = nullptr;
    pActionAcceptIncoming      = nullptr;
    pActionCancelOutgoing      = nullptr;
    pActionDiscardOutgoingCash = nullptr;
    pActionDiscardIncoming     = nullptr;
    pActionDownloadCredentials = nullptr;
    pActionOpenNewWindow       = nullptr;
    pActionReply               = nullptr;
    pActionForward             = nullptr;
    pActionDelete              = nullptr;
    pActionMarkRead            = nullptr;
    pActionMarkUnread          = nullptr;
    pActionPairWithSNP         = nullptr;
    pActionManageNotaries      = nullptr;
    pActionManageAssets        = nullptr;
    pActionManageContacts      = nullptr;
    pActionManageAccounts      = nullptr;

    pActionContactMsg          = nullptr;
    pActionContactPay          = nullptr;
    pActionContactInvoice      = nullptr;
    pActionContactRecurring    = nullptr;

    pActionBailment            = nullptr;
    pActionOutbailment         = nullptr;
}

/// Returns true if it found ANY.
/// Otherwise returns false.
///
/// bigMapAccountsByServer contains all the account IDs in the wallet, mapped by server ID.
/// We want to loop through all the accounts for a particular server, qstrServerId, and nym (str_my_nym_id).
/// We want to get a map of the unit types used by those accounts, with the TLA of each.
/// We also want to get a map of all the TLAs used by those accounts, with a map of asset types for each.
/// Each of those maps of asset types gives us the issuer nym Id for each asset type.
///
bool Activity::GetUnitAndTLAMapForAccountsOnServer(mapIDName& mapUnitTLA, // output
                                                   mapOfMapIDName& bigMapAccountsByServer, // data being searched (input)
                                                   mapOfMapIDName& bigMapAssetsByTLA, // primary output
                                                   QString qstrServerId, // primary input
                                                   const std::string & str_my_nym_id)
{
    bool bFoundSome = false;

    // So let's see if there are any accounts for that server Id.
    mapOfMapIDName::const_iterator ci_paired = bigMapAccountsByServer.find(qstrServerId);

    if (ci_paired != bigMapAccountsByServer.end() && (0 == qstrServerId.compare(ci_paired.key()))) // Found some.
    {
        const mapIDName & mapAccounts = ci_paired.value();
        mapIDName::const_iterator ci_accounts = mapAccounts.begin();
        while (ci_accounts != mapAccounts.end())
        {
            const QString qstrAccountId = ci_accounts.key();
            const std::string str_acct_id = qstrAccountId.toStdString();
            const std::string str_current_nym_id = opentxs::OT::App().API().Exec().GetAccountWallet_NymID(str_acct_id);

            if (0 != str_current_nym_id.compare(str_my_nym_id)) {
                continue;
            }
            // ---------------------------------
            const std::string str_asset_id = opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID(str_acct_id);
            const QString qstrUnitTypeId = QString::fromStdString(str_asset_id);
            const QString qstrTLA = QString::fromStdString(opentxs::OT::App().API().Exec().GetCurrencyTLA(str_asset_id));
            // ---------------------------------
            mapIDName::iterator it_unit_type = mapUnitTLA.find(qstrUnitTypeId);

            if (it_unit_type != mapUnitTLA.end() && it_unit_type.key() == qstrUnitTypeId) {
                // TRhe unit type is already there. (Which is to be expected, since many
                // users will have multiple accounts of the same asset type).
                //
            }
            else { // It's not already there. Let's add it.
                // This map tracks the TLA for each unit type.
                mapUnitTLA.insert(qstrUnitTypeId, qstrTLA);
                bFoundSome = true;
            }
            // ---------------------------------
            const opentxs::Identifier asset_id(str_asset_id);
            opentxs::ConstUnitDefinition unit_definition = opentxs::OT::App().Wallet().UnitDefinition(asset_id);
            opentxs::ConstNym issuer_nym = unit_definition->Nym();
            const opentxs::Identifier & issuer_nym_id = issuer_nym->ID();
            const opentxs::String strIssuerNymId(issuer_nym_id);
            const std::string str_issuer_nym_id(strIssuerNymId.Get());
            const QString qstrIssuerNymId(QString::fromStdString(str_issuer_nym_id));

            // This map tracks a list of assets for each TLA.
            //
            mapOfMapIDName::iterator it_TLA = bigMapAssetsByTLA.find(qstrTLA);

            if (it_TLA != bigMapAssetsByTLA.end() && (0 == qstrTLA.compare(it_TLA.key()))) {
                // There's a map of assets existing for the TLA searched.
                mapIDName & mapAssets = it_TLA.value();
                // Let's see if our unit type is already on that map...
                mapIDName::iterator it_assets = mapAssets.find(qstrUnitTypeId);
                if (it_assets != mapAssets.end() && (0 == qstrUnitTypeId.compare(it_assets.key()))) {
                    // It's already there.
                }
                else { // Add our unit type to the map of unit types for that TLA.
                    // The map connects unit type to issuer nym id.
                    mapAssets.insert(qstrUnitTypeId, qstrIssuerNymId);
                }
            }
            else { // There's not (yet) a map of assets in existence for the TLA searched.
                // Let's create it, and add the unit/issuer pair to it.
                //
                bigMapAssetsByTLA[qstrTLA] = mapIDName{};
                it_TLA = bigMapAssetsByTLA.find(qstrTLA);
                mapIDName & mapAssets = it_TLA.value();
                mapAssets.insert(qstrUnitTypeId, qstrIssuerNymId);
            }
            // ---------------------------------
            ci_accounts++;
        }
    }
    return bFoundSome;
}

/*
#define ACTIVITY_TREE_HEADER_TOTALS   0
#define ACTIVITY_TREE_HEADER_LOCAL    1
#define ACTIVITY_TREE_HEADER_SNP      2
#define ACTIVITY_TREE_HEADER_HOSTED   3
#define ACTIVITY_TREE_HEADER_CONTACTS 4

#define AC_NODE_TYPE_HEADER          0
#define AC_NODE_TYPE_ASSET_TOTAL     1
#define AC_NODE_TYPE_CONTACT         2
#define AC_NODE_TYPE_LOCAL_WALLET    3
#define AC_NODE_TYPE_LOCAL_ACCOUNT   4
#define AC_NODE_TYPE_SNP_NOTARY      5
#define AC_NODE_TYPE_SNP_ACCOUNT     6
#define AC_NODE_TYPE_HOSTED_NOTARY   7
#define AC_NODE_TYPE_HOSTED_ACCOUNT  8
 *
 * So UserData(0) should be the NODE TYPE. For the first row, it will be a header (Totals)
 * and so the "node type" should be: AC_NODE_TYPE_HEADER and "header" will be ACTIVITY_TREE_HEADER_TOTALS.
 *
 * For the second row, it will be an asset total (probably BTC or USD) so the "node type"
 * would be: AC_NODE_TYPE_ASSET_TOTAL. Again, set in UserData(0).
 *
 * EVERY row in the tree is also under a "header", and so should have the header set.
 * Continuing the above example, the first row has "header" of ACTIVITY_TREE_HEADER_TOTALS
 * and the second row also has "header" of ACTIVITY_TREE_HEADER_TOTALS. This should probably
 * be UserData(1).
 *
 * IF the node type is AC_NODE_TYPE_ASSET_TOTAL, as seen just above for the second row,
 * we probably also want to have the TLA available. This should probably be UserData(2).
 *
 * IF the node type is instead, say, AC_NODE_TYPE_SNP_ACCOUNT or AC_NODE_TYPE_HOSTED_ACCOUNT,
 * tracking account instead of asset ID, then in that case UserData(2) should contain the TLA
 * as before (for consistency's sake). Therefore the account ID should be in UserData(4).
 *
 * That way I can put the Opentxs asset type ID (if one exists) into UserData(3).
 *
 * Let's say instead the "node type" is AC_NODE_TYPE_HOSTED_NOTARY -- in that case, the "header"
 * is ACTIVITY_TREE_HEADER_HOSTED. So that's user data 0 and 1.
 * Then UserData(2) would contain the Opentxs Notary ID, if one was available. (Which is possible,
 * at least in the case of AC_NODE_TYPE_SNP_NOTARY).
 * NO -- this should also contain the TLA in 2, the asset type in 3, and the account ID in 4,
 * IF APPLICABLE, since the notary MAY display a "rolled up" balance total as well, and I prefer
 * to have those values available if they are relevant, in a way consistent with the above.
 * (However, once again, I cannot assume they are present. They may not be.)
 *
 * Therefore the Notary ID must be in UserData(5)! IF it's present at all.
 *
 * Therefore if it turns out to be a Contact, "node type" AC_NODE_TYPE_CONTACT, the Contact ID
 * itself, a new-style string-based Opentxs Contact ID, will preferably be in UserData(6).
 */
void Activity::treeWidgetSummary_PopupMenu(const QPoint &pos, QTreeWidget * pTreeWidget)
{
//    const std::string str_my_nym_id = Moneychanger::It()->get_default_nym_id().toStdString();

//    int nTreeNodeType{-1};
//    int nUnderHeader {-1};

//    QString  qstrTLA;
//    QString  qstrAssetTypeId;
//    QString  qstrAccountId;
//    QString  qstrServerId;
//    QString  qstrContactId;
//    // ------------------------
//    QTreeWidgetItem * pTreeItem = pTreeWidget->itemAt(pos); // Might be Null...
//    // ------------------------
//    if ( nullptr != pTreeItem )
//    {
//        const QVariant qvarTreeNodeType = pTreeItem->data(0, Qt::UserRole+0);
//        nTreeNodeType = qvarTreeNodeType.isValid() ? qvarTreeNodeType.toInt() : -1;
//        // ------------------------
//        const QVariant qvarUnderHeader = pTreeItem->data(0, Qt::UserRole+1);
//        nUnderHeader = qvarUnderHeader.isValid() ? qvarUnderHeader.toInt() : -1;
//        // ------------------------
//        if(   (-1) != nTreeNodeType
//           && (-1) != nUnderHeader)
//        {
//            const QVariant qvarTLA = pTreeItem->data(0, Qt::UserRole+2);
//            const QVariant qvarAssetTypeId = pTreeItem->data(0, Qt::UserRole+3);
//            const QVariant qvarAccountId = pTreeItem->data(0, Qt::UserRole+4);
//            const QVariant qvarServerId = pTreeItem->data(0, Qt::UserRole+5);
//            const QVariant qvarContactId = pTreeItem->data(0, Qt::UserRole+6);
//            // ------------------------
//            qstrTLA = qvarTLA.isValid() ? qvarTLA.toString() : QString("");
//            qstrAssetTypeId = qvarAssetTypeId.isValid() ? qvarAssetTypeId.toString() : QString("");
//            qstrAccountId = qvarAccountId.isValid() ? qvarAccountId.toString() : QString("");
//            qstrServerId = qvarServerId.isValid() ? qvarServerId.toString() : QString("");
//            qstrContactId = qvarContactId.isValid() ? qvarContactId.toString() : QString("");
//        }
//    }
//    // ------------------------
//    /*
//    #define ACTIVITY_TREE_HEADER_TOTALS   0
//    #define ACTIVITY_TREE_HEADER_LOCAL    1
//    #define ACTIVITY_TREE_HEADER_SNP      2
//    #define ACTIVITY_TREE_HEADER_HOSTED   3
//    #define ACTIVITY_TREE_HEADER_CONTACTS 4

//    #define AC_NODE_TYPE_HEADER          0
//    #define AC_NODE_TYPE_ASSET_TOTAL     1
//    #define AC_NODE_TYPE_CONTACT         2
//    #define AC_NODE_TYPE_LOCAL_WALLET    3
//    #define AC_NODE_TYPE_LOCAL_ACCOUNT   4
//    #define AC_NODE_TYPE_SNP_NOTARY      5
//    #define AC_NODE_TYPE_SNP_ACCOUNT     6
//    #define AC_NODE_TYPE_HOSTED_NOTARY   7
//    #define AC_NODE_TYPE_HOSTED_ACCOUNT  8
//    */

//    // By this point we know for a fact that no matter what the user clicked on,
//    // the necessary data is available from that item to construct the appropriate menu.
//    //
//    resetPopupMenus();
//    popupMenuAccounts_.reset(new QMenu(this));
//    // --------------------------------------------------
//    const bool bSelectedSNP = (AC_NODE_TYPE_SNP_NOTARY  == nTreeNodeType)
//                           || (AC_NODE_TYPE_SNP_ACCOUNT == nTreeNodeType);
//    // --------------------------------------------------
//    const bool bSelectedHosted = (AC_NODE_TYPE_HOSTED_NOTARY  == nTreeNodeType)
//                              || (AC_NODE_TYPE_HOSTED_ACCOUNT == nTreeNodeType);
//    // --------------------------------------------------
//    const bool bSelectedNotary = !qstrServerId.isEmpty() &&
//                                 (   AC_NODE_TYPE_SNP_NOTARY    == nTreeNodeType
//                                  || AC_NODE_TYPE_HOSTED_NOTARY == nTreeNodeType );

//    const bool bSelectedAcctLowLevel = !qstrAccountId.isEmpty() &&
//                                        (   AC_NODE_TYPE_SNP_ACCOUNT    == nTreeNodeType
//                                         || AC_NODE_TYPE_HOSTED_ACCOUNT == nTreeNodeType);

//    // If the user has selected an actual account tree node, or he has selected
//    // a notary tree node that also happens to be rolled up from a single account ID,
//    // either way, there's an account ID and the user has "selected an account".
//    //
//    const bool bSelectedAccount =  (bSelectedAcctLowLevel ||
//                                    (bSelectedNotary && !qstrAccountId.isEmpty()) );
//    const bool bSelectedAsset   = !qstrAssetTypeId.isEmpty();
//    const bool bSelectedContact = (ACTIVITY_TREE_HEADER_CONTACTS == nUnderHeader) && !qstrContactId.isEmpty();
//    // --------------------------------------------------
//    if (AC_NODE_TYPE_ASSET_TOTAL == nTreeNodeType) {
//        pActionManageAssets = popupMenuAccounts_->addAction(tr("Manage asset types"));
//        popupMenuAccounts_->addSeparator();
//    }
//    // The user has right-clicked on a CONTACT.
//    // (That is, he's right-clicked on a valid non-header item that appears under the "Contacts" header).
//    //
//    else if (bSelectedContact) {
//        pActionViewContact = popupMenuAccounts_->addAction(tr("View this contact"));
//        if (opentxs::Messagability::READY == opentxs::OT::App().API().Sync().CanMessage(opentxs::Identifier(str_my_nym_id), opentxs::Identifier(qstrContactId.toStdString())))
//        {
//            pActionContactMsg = popupMenuAccounts_->addAction(tr("Message this contact"));
//            pActionContactPay = popupMenuAccounts_->addAction(tr("Pay this contact"));
//            pActionContactInvoice = popupMenuAccounts_->addAction(tr("Invoice this contact"));
//            pActionContactRecurring = popupMenuAccounts_->addAction(tr("Request recurring payments"));
//        }
//        //pActionExistingContact = popupMenuAccounts_->addAction(tr("Merge with existing Opentxs Contact"));
//        //pActionDownloadCredentials = popupMenuAccounts_->addAction(tr("Re-download contact"));
//        popupMenuAccounts_->addSeparator();
//    }
//    else if (bSelectedAccount) {
//        pActionManageAccounts = popupMenuAccounts_->addAction(tr("View account"));
//    }
//    // -----------------
//    if (bSelectedNotary) {
//        pActionManageNotaries = popupMenuAccounts_->addAction(tr("View notary"));
//    }
//    if (bSelectedAsset && (nullptr == pActionManageAssets)) {
//        pActionManageAssets = popupMenuAccounts_->addAction(tr("View asset type"));
//    }
//    popupMenuAccounts_->addSeparator();
//    // --------------------------------------------------
//    if (bSelectedNotary || bSelectedAccount) {
//        pActionBailment    = popupMenuAccounts_->addAction(tr("Deposit (from blockchain)"));
//        pActionOutbailment = popupMenuAccounts_->addAction(tr("Withdraw (to blockchain)"));
//        popupMenuAccounts_->addSeparator();
//    }
//    // --------------------------------------------------
//    if (nullptr == pActionManageContacts && nullptr == pActionViewContact) {
//        pActionManageContacts = popupMenuAccounts_->addAction(tr("Manage contacts"));
//    }
//    if (!bSelectedAccount && (nullptr == pActionManageAccounts)) {
//        pActionManageAccounts = popupMenuAccounts_->addAction(tr("Manage accounts"));
//    }
//    if (!bSelectedNotary && (nullptr == pActionManageNotaries)) {
//        pActionManageNotaries = popupMenuAccounts_->addAction(tr("Manage notaries"));
//    }
//    if (!bSelectedAsset && (nullptr == pActionManageAssets)) {
//        pActionManageAssets = popupMenuAccounts_->addAction(tr("Manage asset types"));
//    }
//    popupMenuAccounts_->addSeparator();
//    // --------------------------------------------------
//    if (!bSelectedContact)
//        pActionCreateContact = popupMenuAccounts_->addAction(tr("Create new contact"));
//    // --------------------------------------------------
//    popupMenuAccounts_->addSeparator();
//    // --------------------------------------------------
//    pActionPairWithSNP = popupMenuAccounts_->addAction(tr("Pair a Stash Node Pro"));

//    // ==============================================================================

//    QPoint globalPos = pTreeWidget->mapToGlobal(pos);
//    const QAction* selectedAction = popupMenuAccounts_->exec(globalPos); // Here we popup the menu, and get the user's click.
//    if (nullptr == selectedAction)
//        return;

//    // ==============================================================================

//    if (selectedAction == pActionViewContact)
//    {
//        QTimer::singleShot(0, this, SLOT(resetPopupMenus()));

//        if (nullptr != pTreeItem)
//            pTreeItem->setSelected(true);

//        if (!qstrContactId.isEmpty())
//        {
//            emit showContact(qstrContactId);
//        }
//    }
//    // ----------------------------------
//    // Merge 2 existing opentxs contacts together.
//    //
//    else if (selectedAction == pActionExistingContact)
//    {
//        QTimer::singleShot(0, this, SLOT(resetPopupMenus()));

//        if (nullptr != pTreeItem)
//            pTreeItem->setSelected(true);

//        // This should never happen since we wouldn't even have gotten this menu option
//        // in the first place, unless contact ID had been 0.
//        if (qstrContactId.isEmpty()) {
//            return;
//        }

//        // TODO:  Make sure there are 2 or more Contacts selected first?

//        // Then call Justus new Contact merging function.

//        // until then...
//        qDebug() << "TOD: ******** Merging contacts. *********";
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionDownloadCredentials)
//    {
//        QTimer::singleShot(0, this, SLOT(resetPopupMenus()));

//        if (nullptr != pTreeItem)
//            pTreeItem->setSelected(true);

//        if (qstrContactId.isEmpty()) {
//            return;
//        }

//        if (str_my_nym_id.empty()) {
//            qDebug() << "Unable to download credentials without a default Nym ID being set";
//            return;
//        }
//        opentxs::OT::App().API().Sync().CanMessage(opentxs::Identifier(str_my_nym_id), opentxs::Identifier(qstrContactId.toStdString()));

//        // Todo: emit some signal in a few seconds that an Opentxs Contact was just re-downloaded recently probably?

//    }
//    // ----------------------------------------------
//    else if (selectedAction == pActionCreateContact)
//    {
//        QTimer::singleShot(0, this, SLOT(resetPopupMenus()));

//        if (nullptr != pTreeItem)
//            pTreeItem->setSelected(true);

//        MTGetStringDialog nameDlg(this, tr("Enter a display label for the new contact"));

//        if (QDialog::Accepted != nameDlg.exec()) {
//            return;
//        }
//        // --------------------------------------
//        const QString strNewContactLabel = nameDlg.GetOutputString();
//        const std::string str_new_contact_label = strNewContactLabel.toStdString();
//        // --------------------------------------------------
//        auto pContact = opentxs::OT::App().Contact().NewContact(str_new_contact_label);

//        if (!pContact) {
//            qDebug() << "Error: Failed trying to create new Contact.";
//            return;
//        }
//        // -----------------------------------------------------
//        const opentxs::Identifier idContact{pContact->ID()};
//        const opentxs::String     strContact(idContact);
//        const std::string         str_contact(strContact.Get());
//        qstrContactId = QString::fromStdString(str_contact);
//        // -----------------------------------------------------
//        if (!qstrContactId.isEmpty())
//        {
//            emit showContactAndRefreshHome(qstrContactId);
//        }
//    }
//    // ----------------------------------------------
//    else if (selectedAction == pActionPairWithSNP)
//    {
//        QTimer::singleShot(0, this, SLOT(resetPopupMenus()));

//        emit needToPairStashNode();
//    }
//    // ----------------------------------------------
//    else if (selectedAction == pActionManageNotaries)
//    {
//        QTimer::singleShot(0, this, SLOT(resetPopupMenus()));

//        emit showServer(qstrServerId); // qstrServerId may be empty; that's okay.
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionManageAssets)
//    {
//        QTimer::singleShot(0, this, SLOT(resetPopupMenus()));

//        emit showAsset(qstrAssetTypeId); // asset ID may be empty, which is okay.
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionManageAccounts)
//    {
//        QTimer::singleShot(0, this, SLOT(resetPopupMenus()));

//        emit showAccount(qstrAccountId); // account ID may be empty, which is okay.
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionManageContacts)
//    {
//        QTimer::singleShot(0, this, SLOT(resetPopupMenus()));

//        emit showContacts();
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionContactMsg)
//    {
//        QTimer::singleShot(0, this, SLOT(resetPopupMenus()));

//        emit messageContact(QString::fromStdString(str_my_nym_id), qstrContactId);
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionContactPay)
//    {
//        QTimer::singleShot(0, this, SLOT(resetPopupMenus()));

//        // todo: qstrAccountId is empty here, so maybe should just have
//        // "pay to contact" signal.
//        emit payFromAccountToContact(qstrAccountId, qstrContactId);
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionContactInvoice)
//    {
//        QTimer::singleShot(0, this, SLOT(resetPopupMenus()));

//        // todo: qstrAccountId is empty here, so maybe should just have
//        // "request from contact" signal.
//        emit requestToAccountFromContact(qstrAccountId, qstrContactId);
//    }
//    // ----------------------------------
//    else if (selectedAction == pActionContactRecurring)
//    {
//        QTimer::singleShot(0, this, SLOT(resetPopupMenus()));

//        // todo: qstrAccountId is empty here, so maybe should just have
//        // "propose from contact" signal.
//        emit proposeToAccountFromContact(qstrAccountId, qstrContactId);
//    }
//    // *****************************************************************
//    else if (selectedAction == pActionBailment)
//    {
//        QTimer::singleShot(0, this, SLOT(resetPopupMenus()));

//        if (str_my_nym_id.empty()) {
//            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), tr("Please set your default Nym and then try again."));
//            qDebug() << "Unable to request bailment without a default Nym ID being set";
//            return;
//        }
//        if (qstrServerId.isEmpty()) {

//            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), tr("Unable to request bailment without a Notary ID being set. (Should never happen in this case...)"));
//            qDebug() << "Unable to request bailment without a Notary ID being set. (Should never happen in this case...)";
//            return;
//        }
//        // ---------------------------------------
//        // These two cases (both selected or neither selected) are theoretically impossible.
//        //
//        if (!bSelectedSNP && !bSelectedHosted) {

//            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), tr("Weird, this problem should never happen."));
//            qDebug() << "Somehow, neither a paired SNP or account, nor a hosted notary or account, "
//                        "is available from the point of the click, and so I am unable to continue the "
//                        "bailment attempt. I'd have to ask you at this point to pick a notary/SNP from a list.";
//            return;
//        }
//        if (bSelectedSNP && bSelectedHosted) {

//            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), tr("Weird, this problem should never happen."));
//            qDebug() << "Somehow, both a _paired_ SNP or account, AND a _hosted_ notary or account, "
//                        "is available from the point of the click, and so I am unable to continue the "
//                        "bailment attempt. Should never happen...";
//            return;
//        }
//        // -----------------------------------------------
//        // Grab the currently-selected default asset type ID (and its TLA),
//        // if there is one selected at all.
//        // We'll find it useful soon.
//        //
//        const QString qstrDefaultAssetId = Moneychanger::It()->get_default_asset_id();
//        const std::string default_asset_id = qstrDefaultAssetId.isEmpty()
//                ? ""
//                : qstrDefaultAssetId.toStdString();
//        const QString qstrDefaultTLA = qstrDefaultAssetId.isEmpty()
//                ? QString("")
//                : QString::fromStdString(opentxs::OT::App().API().Exec().GetCurrencyTLA( default_asset_id ));
//        // -----------------------------------------------
//        mapIDName mapUnitTLA;
//        mapOfMapIDName bigMapAccountsByServer, bigMapAssetsByTLA;
//        GetAccountIdMapsByServerId(bigMapAccountsByServer, bSelectedSNP); // true == paired, false == hosted.

//        const bool bFoundSome = GetUnitAndTLAMapForAccountsOnServer(
//                    mapUnitTLA,
//                    bigMapAccountsByServer,
//                    bigMapAssetsByTLA,
//                    qstrServerId,
//                    str_my_nym_id);

//        if (!bFoundSome) {

//            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), tr("Sorry, you don't have any accounts on this notary that can receive a deposit. Make sure you have a default nym selected."
//                                                              "TODO: Ask the user to choose an asset type and nym right here, and then create the appropriate account automatically."));
//            qDebug() << "No accounts available to do a bailment into, for the given notary and nym.";
//            return;
//        }
//        // -----------------------------------------------
//        // Below this point, we know we found at least one account for the given server and nym.
//        // (And thus, at least one "TLA/asset type ID/issuer nym ID" to use in order to initiate a bailment).
//        //
//        mapIDName * pMapAssets = nullptr;

//        // NOTE: TLA can be empty here. Maybe I'm clicked on a Stash Node Pro,
//        // and the TLAs available there are listed BELOW this line (not ON it).
//        // In the case where it's empty, just ask the user to choose which TLA
//        // he wants to deposit.
//        //
//        if (qstrTLA.isEmpty() && (1 == bigMapAssetsByTLA.size())) {
//            qstrTLA = bigMapAssetsByTLA.begin().key();
//            mapIDName & mapAssets = bigMapAssetsByTLA.begin().value();
//            pMapAssets = &mapAssets;
//        }
//        else if (qstrTLA.isEmpty()) { // There are more than one TLA available in the wallet.

//            // We can iterate bigMapAssetsByTLA to see all the available TLAs.
//            // We'll let the user choose which TLA he's bailing in.
//            //
//            DlgChooser theChooser(this);
//            mapIDName & the_map = theChooser.m_map;

//            mapOfMapIDName::iterator it_TLA = bigMapAssetsByTLA.begin();

//            // Set up the list of TLAs for the user to choose from.
//            //
//            while (it_TLA != bigMapAssetsByTLA.end()) {
//                const QString qstrTLA = it_TLA.key();
//                the_map.insert(qstrTLA, qstrTLA);
//                it_TLA++;
//            }
//            // -----------------------------------------------
//            // Pre-select the default TLA, if the application has a default asset type set already.
//            //
//            mapOfMapIDName::iterator it_default_TLA;

//            if (!qstrDefaultTLA.isEmpty()) {
//                it_default_TLA = bigMapAssetsByTLA.find(qstrDefaultTLA);
//            }
//            // If there's a default asset type ID already set in the application,
//            // and if the TLA for that asset type appears in the list we refined above
//            // for the set of accounts for a given notary and nym, then we pre-select it
//            // as the default TLA, when we ask the user to choose a TLA for the deposit.

//            if (!qstrDefaultTLA.isEmpty() && (it_default_TLA != bigMapAssetsByTLA.end()))
//            {
//                theChooser.SetPreSelected(qstrDefaultTLA);
//            }
//            // -----------------------------------------------
//            theChooser.setWindowTitle(tr("Choose asset type to deposit"));
//            // -----------------------------------------------

//            if (theChooser.exec() == QDialog::Accepted)
//            {
//                qstrTLA = theChooser.m_qstrCurrentID;    //  <============= qstrTLA is set.
//                mapOfMapIDName::iterator it_selected_TLA;
//                if (!qstrTLA.isEmpty()) {
//                    it_selected_TLA = bigMapAssetsByTLA.find(qstrTLA);
//                }
//                if (qstrTLA.isEmpty() || (it_selected_TLA == bigMapAssetsByTLA.end())) {

//                    QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), tr("Somehow failed to select a TLA. Should never happen."));
//                    qDebug() << "Somehow failed to select a TLA. Should never happen.";
//                    return;
//                }
//                mapIDName & mapAssets = it_selected_TLA.value();
//                pMapAssets = &mapAssets;     //  <============= pMapAssets is set.
//            }
//            else {
//                return;
//            }
//        }
//        else {
//            mapOfMapIDName::iterator it_selected_TLA;
//            it_selected_TLA = bigMapAssetsByTLA.find(qstrTLA);
//            if (qstrTLA.isEmpty() || (it_selected_TLA == bigMapAssetsByTLA.end())) {

//                QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), tr("Somehow failed to select a TLA. Should never happen."));
//                qDebug() << "Somehow failed to select a TLA. Should never happen.";
//                return;
//            }
//            mapIDName & mapAssets = it_selected_TLA.value();
//            pMapAssets = &mapAssets;     //  <============= pMapAssets is set.
//        }
//        // ----------------------------------------
//        if (qstrTLA.isEmpty() || (nullptr == pMapAssets) || (0 == pMapAssets->size())) {

////            qDebug() << "qstrTLA: " << qstrTLA;

////            qDebug() << "pMapAssets: " << ((nullptr == pMapAssets) ? QString("is null") : QString("not null"));

////            if (pMapAssets)
////                qDebug() << "pMapAssets size: " << QString::number(pMapAssets->size());

//            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), tr("Somehow failed to choose an asset type for the deposit. "
//                                                              "Make sure there is a default nym selected, "
//                                                              "so the application knows which accounts to filter for."));
//            qDebug() << "Somehow failed to select a TLA.";
//            return;
//        }
//        mapIDName & mapAssets = *pMapAssets;
//        // =================================================
////      QString  qstrTLA;
////      QString  qstrAssetTypeId;
////      QString  qstrServerId;
//        QString  qstrBailmentAssetTypeId;
//        QString  qstrIssuerNymId;

//        // Below this point, qstrTLA and its associated mapAssets are both guaranteed to exist.
//        // Next we need to find the exact unit type ID and its issuer Nym ID, so we can initiate
//        // the bailment request.
//        //
//        // In the case where there's only a single unit type ID for that TLA, then just use it.
//        //
//        if (1 == mapAssets.size()) {
//            const QString qstrSelectedAssetId = mapAssets.begin().key();
//            const QString qstrSelectedIssuerNymId = mapAssets.begin().value();

////            if (!qstrAssetTypeId.isEmpty() && (0 != qstrAssetTypeId.compare(qstrSelectedAssetId))) {
////                // NOTE: This is a case here where it's possible that the selected asset ID
////                // doesn't match the one the user clicked on. Perhaps, for example, the right-click
////                // happened to be on a gold account on Alice's SNP, but the default Nym is currently Bob,
////                // and Bob only owns a Bitcoin account on this SNP. (This means the user has more than one
////                // Nym in his desktop wallet, of course). So Bob's Bitcoin account's asset type of "Bitcoin"
////                // doesn't match the "Gold" asset type that was right-clicked on. But anyway, the user will
////                // have to confirm the bailment request before it goes out, and so we can deal with this case
////                // later on.
////            }

//            qstrBailmentAssetTypeId = qstrSelectedAssetId;
//            qstrIssuerNymId = qstrSelectedIssuerNymId;
//        }
//        // Otherwise pop up a list of the unit type IDs, perhaps showing the short name, and ask
//        // the user to double-click one of them.
//        //
//        // At some point in the future we will use verified claims to prove which issuer (and unit
//        // type IDs from that issuer) are the "official" ones owned by your stash-node-manager bot,
//        // versus some other currency that also happens to be issued on that box by some other issuer.
//        // (Which is possible).
//        //
//        else {
//            DlgChooser theChooser(this);
//            mapIDName & the_map = theChooser.m_map;

//            mapIDName::iterator it_assets = mapAssets.begin();

//            QString qstrDefaultKeyToPreselect;

//            // Set up the list of asset types for the user to choose from.
//            // (For a given TLA).
//            //
//            while (it_assets != mapAssets.end()) {
//                const QString qstrCurrentAssetTypeId = it_assets.key();
//                const QString qstrCurrentIssuerNymId = it_assets.value();
//                const QString qstrKey = QString("%1,%2").arg(qstrCurrentAssetTypeId).arg(qstrCurrentIssuerNymId);
//                // ------------------------------------------
//                MTNameLookupQT theLookup;
//                QString OT_issuer_name = QString::fromStdString(theLookup.GetNymName(qstrCurrentIssuerNymId.toStdString(), ""));
//                if (OT_issuer_name.isEmpty()) {
//                    OT_issuer_name = qstrCurrentIssuerNymId;
//                }
//                // ------------------------------------------
//                const opentxs::Identifier asset_id(qstrCurrentAssetTypeId.toStdString());
//                opentxs::ConstUnitDefinition unit_definition = opentxs::OT::App().Wallet().UnitDefinition(asset_id);
//                const QString OT_asset_name = QString::fromStdString(unit_definition->Alias());
////              const QString OT_asset_name = QString::fromStdString(opentxs::OT::App().API().Exec().GetAssetType_Name(qstrCurrentAssetTypeId.toStdString()));
//                const QString qstrValue = QString("'%1' %2: %3").arg(OT_asset_name).arg(tr("issued by")).arg(OT_issuer_name);
//                // ------------------------------------------
//                the_map.insert(qstrKey, qstrValue);

//                // So we can set the default selection in the chooser based on the application's
//                // default asset type ID -- if one is even set.
//                //
//                if (!qstrDefaultAssetId.isEmpty() && (0 == qstrDefaultAssetId.compare(qstrCurrentAssetTypeId))) {
//                    qstrDefaultKeyToPreselect = qstrKey;
//                }
//                it_assets++;
//            }
//            // -----------------------------------------------
//            // Preselect the application's default asset ID in the chooser dialog,
//            // if one is available.
//            //
//            if (!qstrDefaultKeyToPreselect.isEmpty())
//            {
//                theChooser.SetPreSelected(qstrDefaultKeyToPreselect);
//            }
//            // -----------------------------------------------
//            theChooser.setWindowTitle(tr("Choose exact asset type to deposit"));
//            // -----------------------------------------------

//            if (theChooser.exec() == QDialog::Accepted)
//            {
//                const QString qstrSelectedKey = theChooser.m_qstrCurrentID;
//                QStringList stringlist = qstrSelectedKey.split(",");
//                qstrBailmentAssetTypeId = stringlist.at(0);
//                qstrIssuerNymId = stringlist.at(1);
//            }
//            else {
//                return;
//            }
//        }
//        // ----------------------------------------
//        if (qstrBailmentAssetTypeId.isEmpty() || qstrIssuerNymId.isEmpty()) {

//            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), tr("Somehow failed to select an asset type or its associated issuer Nym ID. Should never happen."));
//            qDebug() << "Somehow failed to select an asset type or its associated issuer Nym ID. Should never happen.";
//            return;
//        }
//        // ----------------------------------------
////      QString  qstrTLA;
////      QString  qstrServerId;
////      QString  qstrBailmentAssetTypeId;
////      QString  qstrIssuerNymId;
////      std::string str_my_nym_id;

//        opentxs::Identifier bailmentId;
//        std::string bailmentInstructions;

//        const bool bGotAddress = get_deposit_address(
//            qstrServerId.toStdString(),
//            str_my_nym_id,
//            qstrIssuerNymId.toStdString(),
//            qstrBailmentAssetTypeId.toStdString(),
//            bailmentId,
//            bailmentInstructions);

//        if (bGotAddress)
//        {
//            const opentxs::Identifier my_nym_id{str_my_nym_id};
//            const opentxs::Identifier issuer_nym_id{opentxs::String{qstrIssuerNymId.toStdString()}};

//            auto editor = opentxs::OT::App().Wallet().mutable_Issuer(my_nym_id, issuer_nym_id);
//            auto& issuer = editor.It();

////          if (pIssuer)
//            {
//                issuer.SetUsed(opentxs::proto::PEERREQUEST_BAILMENT, bailmentId);

//                DlgInbailment * pDlgInbailment =
//                new DlgInbailment(this,
//                                  qstrServerId,
//                                  qstrIssuerNymId,
//                                  QString::fromStdString(str_my_nym_id),
//                                  qstrBailmentAssetTypeId,
//                                  QString::fromStdString(bailmentInstructions));

//                pDlgInbailment->setAttribute(Qt::WA_DeleteOnClose);
//                pDlgInbailment->show();
//            }
////            else {
////                QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
////                                     tr("Sorry, but the issuer information is not current available. Failure."));
////            }

////          QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME),
////            tr("Success requesting a deposit address. "
////               "(Soon, it should pop up on the screen in a separate window). "
////               "TODO: Reserve these addresses in advance, so the user doesn't have to wait."));
//        }
//        else {
//            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
//                                 tr("Sorry, but a deposit address is currently unavailable. A new one is already being requested (normally I keep a few laying around for these situations...) Please try again soon."));
//        }
//    }

//    // *********************************************************************

//    else if (selectedAction == pActionOutbailment)
//    {
//        QTimer::singleShot(0, this, SLOT(resetPopupMenus()));

//        if (str_my_nym_id.empty()) {
//            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), tr("Please set your default Nym and then try again."));
//            qDebug() << "Unable to request outbailment without a default Nym ID being set";
//            return;
//        }
//        if (qstrServerId.isEmpty()) {

//            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), tr("Unable to request a withdrawal without a Notary ID being set. (Should never happen in this case...)"));
//            qDebug() << "Unable to request outbailment without a Notary ID being set. (Should never happen in this case...)";
//            return;
//        }
//        // ---------------------------------------
//        // These two cases (both selected or neither selected) are theoretically impossible.
//        //
//        if (!bSelectedSNP && !bSelectedHosted) {

//            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), tr("Weird, this problem should never happen."));
//            qDebug() << "Somehow, neither a paired SNP or account, nor a hosted notary or account, "
//                        "is available from the point of the click, and so I am unable to continue the "
//                        "outbailment attempt. I'd have to ask you at this point to pick a notary/SNP from a list.";
//            return;
//        }
//        if (bSelectedSNP && bSelectedHosted) {

//            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), tr("Weird, this problem should never happen."));
//            qDebug() << "Somehow, both a _paired_ SNP or account, AND a _hosted_ notary or account, "
//                        "is available from the point of the click, and so I am unable to continue the "
//                        "outbailment attempt. Should never happen...";
//            return;
//        }
//        // -----------------------------------------------
//        // Grab the currently-selected default account ID,
//        // if there is one selected at all.
//        // We'll find it useful soon.
//        //
//        const QString qstrDefaultAccountId = Moneychanger::It()->get_default_account_id();
//        const std::string default_account_id = qstrDefaultAccountId.isEmpty()
//                ? ""
//                : qstrDefaultAccountId.toStdString();
//        // -----------------------------------------------
//        // When you WITHDRAW funds back to the blockchain, you cannot withdraw merely
//        // from an asset ID. You must have an account ID. (For the funds withdrawn are
//        // coming out of an opentxs account).
//        //
//        // Once the account ID is selected, we will have the correct asset type ID,
//        // and we will also know the maximum amount we can try to withdraw. (Based on
//        // the account's balance...)
//        //
//        // Interestingly, however, the application doesn't need the account ID to send
//        // the actual outbailment request. Because the outbailment results in an invoice
//        // being sent to the user, and only then does the user need to choose which account
//        // he will process the invoice with.
//        //
//        // However, a good GUI will still acquire the account from the right-click whenever
//        // possible, and that way it can automate the processing of the invoice when it comes
//        // in, by remembering which account to process the invoice with. In the meantime,
//        // the user will actually choose the account when he manually pays the invoice.
//        //
//        // But here we do still try to see if the user has right-clicked an account, so we
//        // can derive the asset ID and issuer ID in that way. Otherwise we still have to get
//        // the user to choose an asset ID, based on those available from filtering the user's
//        // accounts on that server.
//        //

//        const QString qstrMyNymId(QString::fromStdString(str_my_nym_id));
//        const auto& db = opentxs::OT::App().DB();
//        std::vector<opentxs::OTIdentifier> filteredAccounts;
//        const auto nymAccounts = db.AccountsByOwner(opentxs::Identifier::Factory(str_my_nym_id));
//        const auto serverAccounts = db.AccountsByServer(opentxs::Identifier::Factory(qstrServerId.toStdString()));
//        std::set_intersection(nymAccounts.begin(), nymAccounts.end(), serverAccounts.begin(), serverAccounts.end(), filteredAccounts.begin());

//        if (filteredAccounts.size() < 1)
//        {
//            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
//                                 tr("Sorry, you don't have any accounts on this notary that can withdraw to blockchain. "
//                                    "Make sure you have a default nym selected, so we know we're looking at the right accounts. "
//                                    "(Accounts are filtered by the owner Nym)."));
//            qDebug() << "No accounts available to do an outbailment from, for the given notary " << qstrServerId << "and nym " << qstrMyNymId << ".";
//            return;
//        }

//        mapIDName theAccountMap;

//        for (const auto& accountID : filteredAccounts) {
//            const auto str_current_acct_id = accountID->str();
//            const auto account_name = opentxs::OT::App().API().Exec().GetAccountWallet_Name(str_current_acct_id);

//            theAccountMap.insert(QString::fromStdString(str_current_acct_id),
//                                 QString::fromStdString(account_name));
//        }
//        // -----------------------------------------------
////      QString  qstrTLA;
////      QString  qstrAssetTypeId;
////      QString  qstrAccountId;
////      QString  qstrServerId;

//        std::string  strIssuerNymId;
//        std::string  strBailmentAssetTypeId;
//        QString      qstrBailmentAssetTypeId;
//        QString      qstrIssuerNymId;
//        // -----------------------------------------------
//        // TODO: Make sure the above four variables are set properly,
//        // so we can initiate the outbailment.
//        //
//        // Starting with strBailmentAssetTypeId...
//        // -----------------------------------------------
//        // If a given account was already selected (via the right-click), let's see if
//        // it's on the map for the current Nym. If so, we can use its asset type ID.
//        //
//        if (!qstrAccountId.isEmpty()) {
//            mapIDName::iterator it_accounts = theAccountMap.find(qstrAccountId);

//            if ((it_accounts != theAccountMap.end()) && (0 == qstrAccountId.compare(it_accounts.key()))) {
//                // Looks like the account the user right-clicked on, IS an available account based on
//                // filtering for the current nym and the selected notary.
//                //
//                strBailmentAssetTypeId = opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID(qstrAccountId.toStdString());

//                if (!strBailmentAssetTypeId.empty()) {
//                    const opentxs::Identifier asset_id(strBailmentAssetTypeId);
//                    if (!asset_id.empty()) {
//                        opentxs::ConstUnitDefinition unit_definition = opentxs::OT::App().Wallet().UnitDefinition(asset_id);
//                        if (unit_definition) {
//                            opentxs::ConstNym issuer_nym = unit_definition->Nym();
//                            if (issuer_nym) {
//                                const opentxs::Identifier & issuer_nym_id = issuer_nym->ID();
//                                const opentxs::String otstrIssuerNymId(issuer_nym_id);
//                                const std::string str_issuer_nym_id(otstrIssuerNymId.Get());
//                                strIssuerNymId = str_issuer_nym_id;
//                            }
//                        }
//                    }
//                    // ---------------------------------
//                    if (strIssuerNymId.empty()) {
//                        qDebug() << "Error: Found asset type ID based on selected account, "
//                                    "but couldn't find issuer Nym ID from there. Should never happen.";

//                        return;
//                    }
//                }
//            }
//        } // if (!qstrAccountId.isEmpty())
//        // -----------------------------------------------
//        if (!strBailmentAssetTypeId.empty() && !strIssuerNymId.empty()) {
//            qstrBailmentAssetTypeId = QString::fromStdString(strBailmentAssetTypeId);
//            qstrIssuerNymId = QString::fromStdString(strIssuerNymId);
//        }
//        // -----------------------------------------------
//        else {
//            // Grab the currently-selected default asset type ID (and its TLA),
//            // if there is one selected at all. We'll find it useful soon.
//            //
//            const QString qstrDefaultAssetId = Moneychanger::It()->get_default_asset_id();
//            const std::string default_asset_id = qstrDefaultAssetId.isEmpty()
//                    ? ""
//                    : qstrDefaultAssetId.toStdString();
//            const QString qstrDefaultTLA = qstrDefaultAssetId.isEmpty()
//                    ? QString("")
//                    : QString::fromStdString(opentxs::OT::App().API().Exec().GetCurrencyTLA( default_asset_id ));
//            // -----------------------------------------------
//            mapIDName mapUnitTLA;
//            mapOfMapIDName bigMapAccountsByServer, bigMapAssetsByTLA;
//            GetAccountIdMapsByServerId(bigMapAccountsByServer, bSelectedSNP); // true == paired, false == hosted.

//            const bool bFoundSome = GetUnitAndTLAMapForAccountsOnServer(
//                        mapUnitTLA,
//                        bigMapAccountsByServer,
//                        bigMapAssetsByTLA,
//                        qstrServerId,
//                        str_my_nym_id);

//            if (!bFoundSome) {

//                QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
//                    tr("Sorry, you don't have any accounts on this notary that can withdraw to blockchain. "
//                       "Make sure you have a default nym selected. (Accounts are filtered based on the owner Nym)."));
//                qDebug() << "No accounts available to do an outbailment from, for the given notary and nym.";
//                return;
//            }
//            // -----------------------------------------------
//            // Below this point, we know we found at least one account for the given server and nym.
//            // (And thus, at least one "TLA/asset type ID/issuer nym ID" to use in order to initiate an outbailment).
//            //
//            mapIDName * pMapAssets = nullptr;

//            // NOTE: TLA can be empty here. Maybe I'm clicked on a Stash Node Pro,
//            // and the TLAs available there are listed BELOW this line (not ON it).
//            // In the case where it's empty, just ask the user to choose which TLA
//            // he wants to withdraw.
//            //
//            if (qstrTLA.isEmpty() && (1 == bigMapAssetsByTLA.size())) {
//                // If there is only one TLA, we just select it automatically.
//                //
//                qstrTLA = bigMapAssetsByTLA.begin().key();
//                mapIDName & mapAssets = bigMapAssetsByTLA.begin().value();
//                pMapAssets = &mapAssets;
//            }
//            else if (qstrTLA.isEmpty()) { // There must be multiple TLAs to choose from, since we already know there's not zero. (Above, bFoundSome).

//                // We can iterate bigMapAssetsByTLA to see all the available TLAs.
//                // We'll let the user choose which TLA he's bailing in.
//                //
//                DlgChooser theChooser(this);
//                mapIDName & the_map = theChooser.m_map;

//                mapOfMapIDName::iterator it_TLA = bigMapAssetsByTLA.begin();

//                // Set up the list of TLAs for the user to choose from.
//                //
//                while (it_TLA != bigMapAssetsByTLA.end()) {
//                    const QString qstrTLA = it_TLA.key();
//                    the_map.insert(qstrTLA, qstrTLA);
//                    it_TLA++;
//                }
//                // -----------------------------------------------
//                // Pre-select the default TLA, if the application has a default asset type set already.
//                //
//                mapOfMapIDName::iterator it_default_TLA;
//                if (!qstrDefaultTLA.isEmpty()) {
//                    it_default_TLA = bigMapAssetsByTLA.find(qstrDefaultTLA);
//                }
//                // If there's a default asset type ID already set in the application,
//                // and if the TLA for that asset type appears in the list we refined above
//                // for the set of accounts for a given notary and nym, then we pre-select it
//                // as the default TLA, when we ask the user to choose a TLA for the withdrawal.
//                //
//                if (!qstrDefaultTLA.isEmpty() && (it_default_TLA != bigMapAssetsByTLA.end()))
//                {
//                    theChooser.SetPreSelected(qstrDefaultTLA);
//                }
//                // -----------------------------------------------
//                theChooser.setWindowTitle(tr("Choose asset type to withdraw"));
//                // -----------------------------------------------
//                if (theChooser.exec() == QDialog::Accepted)
//                {
//                    qstrTLA = theChooser.m_qstrCurrentID;    //  <============= qstrTLA is set.
//                    mapOfMapIDName::iterator it_selected_TLA;
//                    if (!qstrTLA.isEmpty()) {
//                        it_selected_TLA = bigMapAssetsByTLA.find(qstrTLA);
//                    }
//                    if (qstrTLA.isEmpty() || (it_selected_TLA == bigMapAssetsByTLA.end())) {

//                        QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), tr("Somehow failed to select a TLA. Should never happen."));
//                        qDebug() << "Somehow failed to select a TLA. Should never happen.";
//                        return;
//                    }
//                    mapIDName & mapAssets = it_selected_TLA.value();
//                    pMapAssets = &mapAssets;     //  <============= pMapAssets is set.
//                }
//                else {
//                    // User canceled.
//                    return;
//                }
//            }
//            else {
//                mapOfMapIDName::iterator it_selected_TLA;
//                it_selected_TLA = bigMapAssetsByTLA.find(qstrTLA);
//                if (qstrTLA.isEmpty() || (it_selected_TLA == bigMapAssetsByTLA.end())) {

//                    QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), tr("Somehow failed to select a TLA. Should never happen."));
//                    qDebug() << "Somehow failed to select a TLA. Should never happen.";
//                    return;
//                }
//                mapIDName & mapAssets = it_selected_TLA.value();
//                pMapAssets = &mapAssets;     //  <============= pMapAssets is set.
//            }
//            // ----------------------------------------
//            if (qstrTLA.isEmpty() || (nullptr == pMapAssets) || (0 == pMapAssets->size())) {

//                QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), tr("Somehow failed to choose an asset type for the withdrawal to blockchain. "
//                                                                  "Make sure there is a default nym selected, since the application "
//                                                                  "filters accounts based on owner Nym."));
//                qDebug() << "Somehow failed to select a TLA. Should never happen.";
//                return;
//            }
//            mapIDName & mapAssets = *pMapAssets;
//            // =================================================
//    //      QString  qstrTLA;
//    //      QString  qstrAssetTypeId;
//    //      QString  qstrServerId;
//    //      QString  qstrBailmentAssetTypeId;
//    //      QString  qstrIssuerNymId;

//            // Below this point, qstrTLA and its associated mapAssets are both guaranteed to exist.
//            // Next we need to find the exact unit type ID from that list (and its issuer Nym ID),
//            // so we can initiate the outbailment request.
//            //
//            // In the case where there's only a single unit type ID for that TLA, then just use it.
//            //
//            if (1 == mapAssets.size()) {
//                const QString qstrSelectedAssetId = mapAssets.begin().key();
//                const QString qstrSelectedIssuerNymId = mapAssets.begin().value();

//    //            if (!qstrAssetTypeId.isEmpty() && (0 != qstrAssetTypeId.compare(qstrSelectedAssetId))) {
//    //                // NOTE: This is a case here where it's possible that the selected asset ID
//    //                // doesn't match the one the user clicked on. Perhaps, for example, the right-click
//    //                // happened to be on a gold account on Alice's SNP, but the default Nym is currently Bob,
//    //                // and Bob only owns a Bitcoin account on this SNP. (This means the user has more than one
//    //                // Nym in his desktop wallet, of course). So Bob's Bitcoin account's asset type of "Bitcoin"
//    //                // doesn't match the "Gold" asset type that was right-clicked on. But anyway, the user will
//    //                // have to confirm the bailment request before it goes out, and so we can deal with this case
//    //                // later on. No point causing some weird unexplained failure on this spot when we can just
//    //                // ask later.
//    //            }

//                qstrBailmentAssetTypeId = qstrSelectedAssetId;
//                qstrIssuerNymId = qstrSelectedIssuerNymId;
//            }
//            // Otherwise pop up a list of the unit type IDs, perhaps showing the short name, and ask
//            // the user to double-click one of them.
//            //
//            // At some point in the future we will use verified claims to prove which issuer (and unit
//            // type IDs from that issuer) are the "official" ones owned by your stash-node-manager bot,
//            // versus some other currency that also happens to be issued on that box by some other issuer.
//            // (Which is possible).
//            //
//            else {
//                DlgChooser theChooser(this);
//                mapIDName & the_map = theChooser.m_map;
//                QString qstrDefaultKeyToPreselect;
//                mapIDName::iterator it_assets = mapAssets.begin();

//                // Set up the list of asset types for the user to choose from.
//                // (For a given TLA).
//                //
//                while (it_assets != mapAssets.end()) {
//                    const QString qstrCurrentAssetTypeId = it_assets.key();
//                    const QString qstrCurrentIssuerNymId = it_assets.value();
//                    const QString qstrKey = QString("%1,%2").arg(qstrCurrentAssetTypeId).arg(qstrCurrentIssuerNymId);
//                    // ------------------------------------------
//                    MTNameLookupQT theLookup;
//                    QString OT_issuer_name = QString::fromStdString(theLookup.GetNymName(qstrCurrentIssuerNymId.toStdString(), ""));
//                    if (OT_issuer_name.isEmpty()) {
//                        OT_issuer_name = qstrCurrentIssuerNymId;
//                    }
//                    // ------------------------------------------
//                    const opentxs::Identifier asset_id(qstrCurrentAssetTypeId.toStdString());
//                    opentxs::ConstUnitDefinition unit_definition = opentxs::OT::App().Wallet().UnitDefinition(asset_id);
//                    const QString OT_asset_name  = QString::fromStdString(unit_definition->Alias());
//    //              const QString OT_asset_name  = QString::fromStdString(opentxs::OT::App().API().Exec().GetAssetType_Name(qstrCurrentAssetTypeId.toStdString()));
//                    const QString qstrExtra = QString(" (%1: %2)").arg(tr("ID")).arg(qstrCurrentIssuerNymId);
//                    const QString qstrValue = QString("'%1' %2: %3%4").arg(OT_asset_name).arg(tr("issued by")).arg(OT_issuer_name)
//                            .arg( (0 == OT_issuer_name.compare(qstrCurrentIssuerNymId)) ? QString("") :  qstrExtra);
//                    // ------------------------------------------
//                    the_map.insert(qstrKey, qstrValue);

//                    // So we can set the default selection in the chooser based on the application's
//                    // default asset type ID -- if one is even set.
//                    //
//                    if (!qstrDefaultAssetId.isEmpty() && (0 == qstrDefaultAssetId.compare(qstrCurrentAssetTypeId))) {
//                        qstrDefaultKeyToPreselect = qstrKey;
//                    }
//                    it_assets++;
//                }
//                // -----------------------------------------------
//                // Preselect the application's default asset ID in the chooser dialog,
//                // if one is available.
//                //
//                if (!qstrDefaultKeyToPreselect.isEmpty())
//                {
//                    theChooser.SetPreSelected(qstrDefaultKeyToPreselect);
//                }
//                // -----------------------------------------------
//                theChooser.setWindowTitle(tr("Choose exact asset type to withdraw"));
//                // -----------------------------------------------
//                if (theChooser.exec() == QDialog::Accepted)
//                {
//                    const QString qstrSelectedKey = theChooser.m_qstrCurrentID;
//                    QStringList stringlist = qstrSelectedKey.split(",");
//                    qstrBailmentAssetTypeId = stringlist.at(0);
//                    qstrIssuerNymId = stringlist.at(1);
//                }
//                else {
//                    // User canceled.
//                    return;
//                }
//            } // dlgChooser
//            // ----------------------------------------
//            if (qstrBailmentAssetTypeId.isEmpty() || qstrIssuerNymId.isEmpty()) {

//                QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
//                                     tr("Somehow failed to select an asset type or its associated issuer Nym ID. Should never happen."));
//                qDebug() << "Somehow failed to select an asset type or its associated issuer Nym ID. Should never happen.";
//                return;
//            }
//            strBailmentAssetTypeId = qstrBailmentAssetTypeId.toStdString();
//            strIssuerNymId = qstrIssuerNymId.toStdString();
//        }
//        // ******************************************************************

//        OT_ASSERT(!strBailmentAssetTypeId.empty());
//        OT_ASSERT(!strIssuerNymId.empty());
//        OT_ASSERT(!qstrBailmentAssetTypeId.isEmpty());
//        OT_ASSERT(!qstrIssuerNymId.isEmpty());

//        // ******************************************************************

//        std::int64_t AMOUNT{0};
//        std::string  strToBlockchainAddress;
//        const std::string str_notary_id (qstrServerId.toStdString());
//        const std::string unit_type_id  (qstrBailmentAssetTypeId.toStdString());
//        // -----------------------------------------------
//        DlgOutbailment dlgOutbailment(this, AMOUNT, strToBlockchainAddress,
//                                      qstrBailmentAssetTypeId, qstrIssuerNymId, qstrServerId);

//        if (dlgOutbailment.exec() == QDialog::Accepted) {

//            const std::int64_t intZero{0};

//            if (AMOUNT <= intZero) {
//                QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
//                                     tr("Failure: Cannot withdraw a negative or zero amount."));
//                return;
//            }
//            if (strToBlockchainAddress.empty()) {
//                QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
//                                     tr("Failure: Blockchain address is empty."));
//                return;
//            }
//            // -----------------------------------------------


//            // TODO: Validate blockchain address here.


//            // -----------------------------------------------
//            const bool bRequestOutbailment = request_outbailment(
//                str_notary_id,
//                str_my_nym_id,
//                strIssuerNymId,
//                unit_type_id,
//                AMOUNT,
//                strToBlockchainAddress);
//        }
//    } // Outbailment
//    // -----------------------------
}


// EXPORT std::string initiate_outbailment(
//     const std::string& NOTARY_ID,
//     const std::string& NYM_ID,
//     const std::string& TARGET_NYM_ID,
//     const std::string& INSTRUMENT_DEFINITION_ID,
//     const std::int64_t& AMOUNT,
//     const std::string& THE_MESSAGE) const;

bool Activity::request_outbailment(
    const std::string str_notary_id,
    const std::string str_my_nym_id,
    const std::string str_issuer_nym_id,
    const std::string str_unit_type_id,
    const std::int64_t amount,
    const std::string str_blockchain_address)
{
    OT_ASSERT(!str_notary_id.empty());
    OT_ASSERT(!str_my_nym_id.empty());
    OT_ASSERT(!str_issuer_nym_id.empty());
    OT_ASSERT(!str_unit_type_id.empty());
    OT_ASSERT(!str_blockchain_address.empty());

    if (amount < 0) {
        qDebug() << "Failed attempt to request outbailment due to negative amount: " << QString::number(amount);
        return false;
    }

    opentxs::otErr << __FUNCTION__
                   << ": Requesting outbailment." << std::endl;
    std::string result{};

    try {
        auto action = opentxs::OT::App().API().ServerAction().InitiateOutbailment(
            opentxs::Identifier(str_my_nym_id),
			opentxs::Identifier(str_notary_id),
			opentxs::Identifier(str_issuer_nym_id),
			opentxs::Identifier(str_unit_type_id),
            amount,
            str_blockchain_address);
        result = action->Run();
    } catch (const std::runtime_error& e) {
        opentxs::otErr << __FUNCTION__ << ": " << e.what();
        return false;
    }

    if (result.empty()) {
        qDebug() << "Failed to receive a reply from the notary (while trying to initiate an outbailment).";
        QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
            tr("Failed to receive a reply from the notary while trying to initiate a withdrawal to blockchain."));
        return false;
    }
    // -----------------------------
    const bool output = (1 == opentxs::VerifyMessageSuccess(result));

    if (!output) {
        qDebug() << "Failed trying to request outbailment.";
        QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                             tr("Withdrawal request failed."));
    }
    else {
        opentxs::otErr << __FUNCTION__
                       << ": Initiate outbailment request sent.\n"
                       << "Address: " << str_blockchain_address << std::endl;
        const QString qstrMessage = QString("%1: %2. %3.")
                .arg(tr("Success requesting a withdrawal to blockchain address"))
                .arg(QString::fromStdString(str_blockchain_address))
                .arg(tr("You should receive an invoice soon; pay it to complete your withdrawal"));
        QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), qstrMessage);
    }
    return output;
}


//EXPORT std::string initiate_bailment(
//     const std::string& NOTARY_ID,
//     const std::string& NYM_ID,
//     const std::string& TARGET_NYM_ID,
//     const std::string& INSTRUMENT_DEFINITION_ID) const;

bool Activity::get_deposit_address(
    const std::string str_notary_id,
    const std::string str_my_nym_id,
    const std::string str_issuer_nym_id,
    const std::string str_unit_type_id,
    opentxs::Identifier & returnBailmentId,
    std::string & returnBailmentInstructions)
{
    OT_ASSERT(!str_notary_id.empty());
    OT_ASSERT(!str_my_nym_id.empty());
    OT_ASSERT(!str_issuer_nym_id.empty());
    OT_ASSERT(!str_unit_type_id.empty());

    /*
    Issuer::BailmentInstructions() is what you want
    13:13
     virtual std::vector<BailmentDetails> BailmentInstructions(
        const opentxs::Identifier& unitID,
        const bool onlyUnused = true) const = 0;

    That will give you a vector of opentxs::Identifier, proto::BailmentReply pairs.
    13:15
    Grab the address from the BailmentReply.
    13:15
    When you display one of those addresses as a QR code,
    call SetUsed() to cause the state machine to grab a new one.
     */

    const opentxs::Identifier nymId{str_my_nym_id};
    const opentxs::Identifier issuerNymId{str_issuer_nym_id};
    const opentxs::Identifier unitTypeId{str_unit_type_id};

    if (nymId.empty() || issuerNymId.empty() || unitTypeId.empty()) {
        qDebug() << "get_deposit_address: Error: Local Nym, Issuer Nym, or Unit Type was empty...";
        return false;
    }
    // ---------------------------------------------------------------
    auto pIssuer = opentxs::OT::App().Wallet().Issuer(nymId, issuerNymId);

    if (!pIssuer) {
        qDebug() << "get_deposit_address: Somehow failed to retrieve the issuer needed. (Failure).";
        return false;
    }
    // ---------------------------------------------------------------
    auto bailmentDetails = pIssuer->BailmentInstructions(unitTypeId);

    for (auto & [ bailmentId, bailmentReply ] : bailmentDetails)
    {
        if (bailmentId->empty() || !bailmentReply.has_instructions()) {
            qDebug() << "get_deposit_address: Error: missing bailment reply Id or instructions.";
            continue;
        }

        returnBailmentId = bailmentId;
        returnBailmentInstructions = bailmentReply.instructions();
        return true;
    }

    return false;


//    auto& me = opentxs::OT::App().API().OTME();
//    opentxs::otErr << __FUNCTION__
//                   << ": Requesting deposit address." << std::endl;
//    std::string result{};
//
//    try {
//        result = me.initiate_bailment(
//            str_notary_id,
//            str_my_nym_id,
//            str_issuer_nym_id,
//            str_unit_type_id);
//    } catch (const std::runtime_error& e) {
//        opentxs::otErr << __FUNCTION__ << ": " << e.what();
//        return false;
//    }
//
//    if (result.empty()) {
//        qDebug() << "Failed to receive a reply from the notary while trying to initiate bailment.";
//        return false;
//    }
//    // -----------------------------
//    const bool output = (1 == me.VerifyMessageSuccess(result));
//
//    if (!output) {
//        opentxs::otErr << __FUNCTION__
//                       << ": Initiate request bailment failed. Oh well, have to try again sometime soon."
//                       << std::endl;
//    }
//    return output;
}

void Activity::on_tabWidgetMain_currentChanged(int index)
{
    /*
     * 0 Accounts
     * 1 Conversations
     * 2 Messages
     * 3 Agreements
     * 4 Exchange
     * 5 Tools
     * 6 Settings
     */
    QWidget * pCurrentWidget = ui->tabWidgetMain->currentWidget();

    if (pCurrentWidget == pTab0_) {
        nSelectedTab_ = index;
    }
    else if (pCurrentWidget == pTab1_) {
        nSelectedTab_ = index;
    }
    else if (pCurrentWidget == pTab2_)  { // Messages
        ui->tabWidgetMain->blockSignals(true);
        ui->tabWidgetMain->setCurrentIndex(nSelectedTab_); // Set it back (don't allow the user to change to this tab)
        ui->tabWidgetMain->blockSignals(false);

        // Then pop up the messages dialog:
        emit showMessages();
        return;
    }
    else if (pCurrentWidget == pTab3_)  { // Active Agreements
        ui->tabWidgetMain->blockSignals(true);
        ui->tabWidgetMain->setCurrentIndex(nSelectedTab_); // Set it back (don't allow the user to change to this tab)
        ui->tabWidgetMain->blockSignals(false);

        // Then pop up the agreements dialog:
        emit showActiveAgreements();
        return;
    }
    else if (pCurrentWidget == pTab4_)  { // Exchange
        ui->tabWidgetMain->blockSignals(true);
        ui->tabWidgetMain->setCurrentIndex(nSelectedTab_); // Set it back (don't allow the user to change to this tab)
        ui->tabWidgetMain->blockSignals(false);

        // Then pop up the exchange dialog:
        emit showExchange();
        return;
    }
    else if (pCurrentWidget == pTab5_)  { // Tools
        nSelectedTab_ = index;
    }
    else if (pCurrentWidget == pTab6_)  { // Settings
        ui->tabWidgetMain->blockSignals(true);
        ui->tabWidgetMain->setCurrentIndex(nSelectedTab_); // Set it back (don't allow the user to change to this tab)
        ui->tabWidgetMain->blockSignals(false);

        // Then pop up the settings dialog:
        emit showSettings();
        return;
    }
    else { // should never happen
        qDebug() << QString("Activity::on_tabWidgetMain_currentChanged: Bad index: %1").arg(index);
        return;
    }
}

Activity::Activity(QWidget *parent) :
    QWidget(parent, Qt::Window),
    ui(new Ui::Activity)
{
    ui->setupUi(this);

    this->installEventFilter(this);

    connect(this, SIGNAL(showMessages()), Moneychanger::It(), SLOT(mc_messages_slot()));
    connect(this, SIGNAL(showActiveAgreements()), Moneychanger::It(), SLOT(mc_agreements_slot()));
    connect(this, SIGNAL(showExchange()), Moneychanger::It(), SLOT(mc_market_slot()));
    connect(this, SIGNAL(showSettings()), Moneychanger::It(), SLOT(mc_settings_slot()));
    connect(this, SIGNAL(needToCheckNym(QString, QString, QString)), Moneychanger::It(), SLOT(onNeedToCheckNym(QString, QString, QString)));
    connect(this, SIGNAL(showServer(QString)), Moneychanger::It(), SLOT(mc_show_server_slot(QString)));
    connect(this, SIGNAL(showAsset(QString)),  Moneychanger::It(), SLOT(mc_show_asset_slot(QString)));
    connect(this, SIGNAL(needToPairStashNode()), Moneychanger::It(), SLOT(mc_pair_node_slot()));
    connect(this, SIGNAL(showAccount(QString)),  Moneychanger::It(), SLOT(mc_show_account_slot(QString)));
    connect(this, SIGNAL(messageContact(QString,QString)), Moneychanger::It(), SLOT(mc_message_contact_slot(QString,QString)));
    connect(this, SIGNAL(payFromAccountToContact(QString,QString)), Moneychanger::It(), SLOT(mc_send_from_acct_to_contact(QString,QString)));
    connect(this, SIGNAL(requestToAccountFromContact(QString,QString)), Moneychanger::It(), SLOT(mc_request_to_acct_from_contact(QString,QString)));
    connect(this, SIGNAL(proposeToAccountFromContact(QString,QString)), Moneychanger::It(), SLOT(mc_proposeplan_to_acct_from_contact(QString,QString)));
    // --------------------------------------------------
    connect(this, SIGNAL(sig_on_toolButton_payments_clicked()), Moneychanger::It(), SLOT(mc_payments_slot()));
    connect(this, SIGNAL(sig_on_toolButton_pending_clicked()), Moneychanger::It(), SLOT(mc_overview_slot()));
    connect(this, SIGNAL(sig_on_toolButton_markets_clicked()), Moneychanger::It(), SLOT(mc_market_slot()));
    connect(this, SIGNAL(sig_on_toolButton_secrets_clicked()), Moneychanger::It(), SLOT(mc_passphrase_manager_slot()));
    connect(this, SIGNAL(sig_on_toolButton_importCash_clicked()), Moneychanger::It(), SLOT(mc_import_slot()));
    connect(this, SIGNAL(sig_on_toolButton_contacts_clicked()), Moneychanger::It(), SLOT(mc_addressbook_slot()));
    connect(this, SIGNAL(sig_on_toolButton_smartContracts_clicked()), Moneychanger::It(), SLOT(mc_smartcontract_slot()));
    connect(this, SIGNAL(sig_on_toolButton_Corporations_clicked()), Moneychanger::It(), SLOT(mc_corporation_slot()));
    connect(this, SIGNAL(sig_on_toolButton_transport_clicked()), Moneychanger::It(), SLOT(mc_transport_slot()));
    connect(this, SIGNAL(sig_on_toolButton_quit_clicked()), Moneychanger::It(), SLOT(mc_shutdown_slot()));
    connect(this, SIGNAL(sig_on_toolButton_encrypt_clicked()), Moneychanger::It(), SLOT(mc_crypto_encrypt_slot()));
    connect(this, SIGNAL(sig_on_toolButton_sign_clicked()), Moneychanger::It(), SLOT(mc_crypto_sign_slot()));
    connect(this, SIGNAL(sig_on_toolButton_decrypt_clicked()), Moneychanger::It(), SLOT(mc_crypto_decrypt_slot()));
    connect(this, SIGNAL(sig_on_toolButton_liveAgreements_clicked()), Moneychanger::It(), SLOT(mc_agreements_slot()));
    // --------------------------------------------------
//    connect(ui->toolButtonAddContact, SIGNAL(clicked()), this, SLOT(on_toolButtonAddContact_clicked()));
//    connect(ui->toolButtonPayContact, SIGNAL(clicked()), this, SLOT(on_toolButtonPayContact_clicked()));
//    connect(ui->toolButtonMsgContact, SIGNAL(clicked()), this, SLOT(on_toolButtonMsgContact_clicked()));
//    connect(ui->toolButtonInvoiceContact, SIGNAL(clicked()), this, SLOT(on_toolButtonInvoiceContact_clicked()));
//    connect(ui->toolButtonImportCash, SIGNAL(clicked()), Moneychanger::It(), SLOT(mc_import_slot()));

    icon_nym_ = QIcon(":/icons/icons/identity_BW.png");

}

void Activity::on_toolButtonAddContact_clicked()
{
    qDebug() << "on_toolButtonAddContact_clicked was successfully called.";

    MTDlgNewContact theNewContact(this);
    theNewContact.setWindowTitle(tr("Add Contact"));
    // -----------------------------------------------
    if (theNewContact.exec() != QDialog::Accepted)
        return;
    // -----------------------------------------------
    QString     rawId  = theNewContact.GetId();
    std::string raw_id = rawId.toStdString();
    std::string str_nym_id = raw_id;
    std::string payment_code;

    int nContactId{0};

    QString qstrNewContactLabel;
    QString nymID(rawId);
    //QString nymID = theNewContact.GetId();

    if (nymID.isEmpty())
        return;
    // --------------------------------------------------------
    // Might be a Payment Code.
    if (!opentxs::OT::App().API().Exec().IsValidID(raw_id))
    {
        nymID = "";
        str_nym_id = "";

        const std::string str_temp = opentxs::OT::App().API().Exec().NymIDFromPaymentCode(raw_id);

        if (!str_temp.empty())
        {
            payment_code = raw_id;
            str_nym_id = str_temp;
            nymID = QString::fromStdString(str_nym_id);
        }
    }
    // --------------------------------------------------------
    if (!opentxs::OT::App().API().Exec().IsValidID(str_nym_id))
    {
        QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                             tr("Sorry, that is not a valid Paycode or Nym ID."));
        return;
    }
    // Below this point we know we've got a valid Opentxs NymId for the Contact
    // we're adding. (Though a contact may already exist, we'll find out next...)
    // --------------------------------------------------------
    QString qstrContactId;
    const opentxs::Identifier existingContactId{opentxs::OT::App().Contact().ContactID(opentxs::Identifier{str_nym_id})};
    const bool bPreexistingOpentxsContact{!existingContactId.IsEmpty()};
    bool bCreatedOpentxsContactJustNow{false};

    if (bPreexistingOpentxsContact) // It already exists.
    {
        const opentxs::String strExistingContact(existingContactId);
        qstrContactId = QString::fromStdString(std::string(strExistingContact.Get()));
    }
    else
        //if (existingContactId.IsEmpty()) so it definitely doesn't exist yet in opentxs...
    { // Let's create one.
        MTGetStringDialog nameDlg(this, tr("Enter a display label for the new contact"));
        if (QDialog::Accepted != nameDlg.exec())
            return;
        qstrNewContactLabel = nameDlg.GetOutputString();
        // --------------------------------------
        qstrContactId = MTContactHandler::getInstance()->GetOrCreateOpentxsContactBasedOnNym(qstrNewContactLabel, nymID, QString::fromStdString(payment_code));
        bCreatedOpentxsContactJustNow = !qstrContactId.isEmpty();
    }
    // --------------------------------------------------------
    const bool bHaveAValidOpentxsContact = opentxs::OT::App().API().Exec().IsValidID(qstrContactId.toStdString());
    if (      !bHaveAValidOpentxsContact)
    {
        QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                             tr("Unable to get or create opentxs contact based on provided id: %1").arg(rawId));
        return;
    }
    // Below this point, we know for a fact that an Opentxs Contact now exists for
    // the provided NymId or PaymentCode. Next, let's make sure we've mirrored that
    // reality in our local Contacts database (the old style) so all the old dialogs
    // continue to work until we can switch them all over.
    // --------------------------------------------------------
    const int nExistingContactId = MTContactHandler::getInstance()->FindContactIDByNymID(nymID);
    const bool bPreexistingMnychrContact = (nExistingContactId > 0);

    if (bPreexistingMnychrContact)
    {
        nContactId = nExistingContactId;

        if (bCreatedOpentxsContactJustNow) {
//             We JUST NOW created the Opentxs contact, but the old-style
//             Moneychanger contact already existed. But since we are currently in the
//             new-style "Opentxs Contacts" dialog part of the code, we should therefore
//             still insert the entry into our list in the GUI on this dialog. (Since we
//             really did just create it, as far as this dialog is concerned).
//
//            m_pOwner->m_map.insert(qstrContactId, qstrNewContactLabel);
//            m_pOwner->SetPreSelected(qstrContactId);
            emit RefreshRecordsAndUpdateMenu();
        }
        else if (bPreexistingOpentxsContact) { // This block means the old-style Moneychanger Contact already existed, AND the new-style Opentxs Contact already existed as well!
            QString contactName = MTContactHandler::getInstance()->GetContactName(nExistingContactId);
            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                 tr("Contact '%1' already exists with NymID: %2").arg(contactName).arg(nymID));
//            // Since it already exists, we'll select it in the GUI, so the user can see what we're
//            // talking about when we tell him that it already exists.
//            m_pOwner->SetPreSelected(qstrContactId);
            emit RefreshRecordsAndUpdateMenu();
        }
        else {
            // This block means the old-style Moneychanger Contact already existed, and the
            // new-style Opentxs Contact did NOT already exist, but then we failed trying to
            // create that.
            QString contactName = MTContactHandler::getInstance()->GetContactName(nExistingContactId);
            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), // Should never happen.
                                 tr("Old-style contact '%1' already exists with NymID: %2. "
                                    "However, failed creating a new corresponding Opentxs Contact. (Should never happen).")
                                 .arg(contactName).arg(nymID));
        }
        return;
    }
    else // bHaveAValidOpentxsContact && !bPreexistingMnychrContact
    {
        // In this block, the old-style Moneychanger Contact does NOT already exist.
        // But we DO know (for a fact) that we have a valid Opentxs Contact.
        //
        // Therefore we need to create the old-style one.
        // --------------------------------------
        if (qstrNewContactLabel.isEmpty()) {
            MTGetStringDialog nameDlg(this, tr("Enter a display label for the new contact"));
            if (QDialog::Accepted != nameDlg.exec())
                return;
            qstrNewContactLabel = nameDlg.GetOutputString();
        }
        // --------------------------------------
        const int nNewContactId  = MTContactHandler::getInstance()->CreateContactBasedOnNym(nymID);
        if (      nNewContactId <= 0)
        {
            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                 tr("Failed trying to create old-style contact for NymID: %1, "
                                    "even though the new-style Opentxs Contact exists (%2).").arg(nymID).arg(qstrContactId));
//            m_pOwner->SetPreSelected(qstrContactId);
            emit RefreshRecordsAndUpdateMenu();
            return;
        }
        else {
            nContactId = nNewContactId;
            MTContactHandler::getInstance()->SetContactName(nNewContactId, qstrNewContactLabel);
        }
        // Below this point, we know that the old-style Contact exists as nContactId,
        // and we know that the new-style Opentxs Contact exists as qstrContactId.
        // We also know that at least some of that was created, since we would have
        // already returned by now if both had already existed. Therefore, success!
        // -------------------------------------------------------
//        // Now let's add this contact to the Map, and refresh the dialog,
//        // and then set the new contact as the current one.
//        //
//        m_pOwner->m_map.insert(qstrContactId, qstrNewContactLabel);
//        m_pOwner->SetPreSelected(qstrContactId);
        emit RefreshRecordsAndUpdateMenu();
    }
}



void Activity::RetrieveSelectedIds(
   QString  & qstrTLA,
   QString  & qstrAssetTypeId,
   QString  & qstrAccountId,
   QString  & qstrServerId,
   QString  & qstrContactId,
   bool     & bSelectedSNP,
   bool     & bSelectedHosted,
   bool     & bSelectedNotary,
   bool     & bSelectedAcctLowLevel,
   bool     & bSelectedAccount,
   bool     & bSelectedAsset,
   bool     & bSelectedContact
)
{
//    QTreeWidget * pTreeWidgetAccounts = ui->treeWidgetAccounts;
//    if (nullptr == pTreeWidgetAccounts) {
//        return;
//    }
//    // ----------------------------------------
//    int nTreeNodeType{-1};
//    int nUnderHeader {-1};

//    QTreeWidgetItem * pTreeItem = pTreeWidgetAccounts->currentItem(); // Might be Null...
//    // ------------------------
//    if ( nullptr != pTreeItem )
//    {
//        const QVariant qvarTreeNodeType = pTreeItem->data(0, Qt::UserRole+0);
//        nTreeNodeType = qvarTreeNodeType.isValid() ? qvarTreeNodeType.toInt() : -1;
//        // ------------------------
//        const QVariant qvarUnderHeader = pTreeItem->data(0, Qt::UserRole+1);
//        nUnderHeader = qvarUnderHeader.isValid() ? qvarUnderHeader.toInt() : -1;
//        // ------------------------
//        if(   (-1) != nTreeNodeType
//           && (-1) != nUnderHeader)
//        {
//            const QVariant qvarTLA = pTreeItem->data(0, Qt::UserRole+2);
//            const QVariant qvarAssetTypeId = pTreeItem->data(0, Qt::UserRole+3);
//            const QVariant qvarAccountId = pTreeItem->data(0, Qt::UserRole+4);
//            const QVariant qvarServerId = pTreeItem->data(0, Qt::UserRole+5);
//            const QVariant qvarContactId = pTreeItem->data(0, Qt::UserRole+6);
//            // ------------------------
//            qstrTLA = qvarTLA.isValid() ? qvarTLA.toString() : QString("");
//            qstrAssetTypeId = qvarAssetTypeId.isValid() ? qvarAssetTypeId.toString() : QString("");
//            qstrAccountId = qvarAccountId.isValid() ? qvarAccountId.toString() : QString("");
//            qstrServerId = qvarServerId.isValid() ? qvarServerId.toString() : QString("");
//            qstrContactId = qvarContactId.isValid() ? qvarContactId.toString() : QString("");
//        }
//    }
//    // ------------------------
//    /*
//     #define ACTIVITY_TREE_HEADER_TOTALS   0
//     #define ACTIVITY_TREE_HEADER_LOCAL    1
//     #define ACTIVITY_TREE_HEADER_SNP      2
//     #define ACTIVITY_TREE_HEADER_HOSTED   3
//     #define ACTIVITY_TREE_HEADER_CONTACTS 4

//     #define AC_NODE_TYPE_HEADER          0
//     #define AC_NODE_TYPE_ASSET_TOTAL     1
//     #define AC_NODE_TYPE_CONTACT         2
//     #define AC_NODE_TYPE_LOCAL_WALLET    3
//     #define AC_NODE_TYPE_LOCAL_ACCOUNT   4
//     #define AC_NODE_TYPE_SNP_NOTARY      5
//     #define AC_NODE_TYPE_SNP_ACCOUNT     6
//     #define AC_NODE_TYPE_HOSTED_NOTARY   7
//     #define AC_NODE_TYPE_HOSTED_ACCOUNT  8
//     */

//    bSelectedSNP = (AC_NODE_TYPE_SNP_NOTARY  == nTreeNodeType)
//    || (AC_NODE_TYPE_SNP_ACCOUNT == nTreeNodeType);
//    // --------------------------------------------------
//    bSelectedHosted = (AC_NODE_TYPE_HOSTED_NOTARY  == nTreeNodeType)
//    || (AC_NODE_TYPE_HOSTED_ACCOUNT == nTreeNodeType);
//    // --------------------------------------------------
//    bSelectedNotary = !qstrServerId.isEmpty() &&
//    (   AC_NODE_TYPE_SNP_NOTARY    == nTreeNodeType
//     || AC_NODE_TYPE_HOSTED_NOTARY == nTreeNodeType );

//    bSelectedAcctLowLevel = !qstrAccountId.isEmpty() &&
//    (   AC_NODE_TYPE_SNP_ACCOUNT    == nTreeNodeType
//     || AC_NODE_TYPE_HOSTED_ACCOUNT == nTreeNodeType);

//    // If the user has selected an actual account tree node, or he has selected
//    // a notary tree node that also happens to be rolled up from a single account ID,
//    // either way, there's an account ID and the user has "selected an account".
//    //
//    bSelectedAccount =  (bSelectedAcctLowLevel ||
//                                    (bSelectedNotary && !qstrAccountId.isEmpty()) );
//    bSelectedAsset   = !qstrAssetTypeId.isEmpty();
//    bSelectedContact = (ACTIVITY_TREE_HEADER_CONTACTS == nUnderHeader) && !qstrContactId.isEmpty();
//    // --------------------------------------------------
//    if (qstrAccountId.isEmpty()) {
//        qstrAccountId = Moneychanger::It()->get_default_account_id();
//    }
//    // --------------------------------------------------
//    if (qstrAccountId.isEmpty()) {
//        qstrAccountId = qstrCurrentAccount_;
//    }
//    // --------------------------------------------------
//    if (qstrContactId.isEmpty()) {
//        qstrContactId = qstrCurrentContact_;
//    }
}


void Activity::on_toolButtonPayContact_clicked()
{
    qDebug() << "on_toolButtonPayContact_clicked was successfully called.";

    QString  qstrTLA;
    QString  qstrAssetTypeId;
    QString  qstrAccountId;
    QString  qstrServerId;
    QString  qstrContactId;
    bool     bSelectedSNP;
    bool     bSelectedHosted;
    bool     bSelectedNotary;
    bool     bSelectedAcctLowLevel;
    bool     bSelectedAccount;
    bool     bSelectedAsset;
    bool     bSelectedContact;

    RetrieveSelectedIds(qstrTLA,
                        qstrAssetTypeId,
                        qstrAccountId,
                        qstrServerId,
                        qstrContactId,
                        bSelectedSNP,
                        bSelectedHosted,
                        bSelectedNotary,
                        bSelectedAcctLowLevel,
                        bSelectedAccount,
                        bSelectedAsset,
                        bSelectedContact
                        );
    // ---------------------------------------------------
    // If the qstrContactId is empty, need to make the user choose from a list.
    //
    if (qstrContactId.isEmpty())
    {
        // Pop up a Contact selection box. The user chooses an existing contact.
        //
        DlgChooser theChooser(this);
        // -----------------------------------------------
        mapIDName & the_map = theChooser.m_map;
        MTContactHandler::getInstance()->GetOpentxsContacts(the_map);
        // -----------------------------------------------
        theChooser.setWindowTitle(tr("Choose a contact"));
        if (theChooser.exec() != QDialog::Accepted)
            return;
        // -----------------------------------------------
        qstrContactId = theChooser.GetCurrentID();
    }
    // ---------------------------------------------------
    emit payFromAccountToContact(qstrAccountId, qstrContactId);
}

void Activity::on_toolButtonMsgContact_clicked()
{
    qDebug() << "on_toolButtonMsgContact_clicked was successfully called.";

    QString  qstrTLA;
    QString  qstrAssetTypeId;
    QString  qstrAccountId;
    QString  qstrServerId;
    QString  qstrContactId;
    bool     bSelectedSNP;
    bool     bSelectedHosted;
    bool     bSelectedNotary;
    bool     bSelectedAcctLowLevel;
    bool     bSelectedAccount;
    bool     bSelectedAsset;
    bool     bSelectedContact;

    RetrieveSelectedIds(qstrTLA,
                        qstrAssetTypeId,
                        qstrAccountId,
                        qstrServerId,
                        qstrContactId,
                        bSelectedSNP,
                        bSelectedHosted,
                        bSelectedNotary,
                        bSelectedAcctLowLevel,
                        bSelectedAccount,
                        bSelectedAsset,
                        bSelectedContact
                        );
    // ------------------------
    const QString qstrMyNymId = Moneychanger::It()->get_default_nym_id();
    // ---------------------------------------------------
    // If the qstrContactId is empty, need to make the user choose from a list.
    // That way we ALWAYS emit both IDs.
    //
    if (qstrContactId.isEmpty())
    {
        // Pop up a Contact selection box. The user chooses an existing contact.
        //
        DlgChooser theChooser(this);
        // -----------------------------------------------
        mapIDName & the_map = theChooser.m_map;
        MTContactHandler::getInstance()->GetOpentxsContacts(the_map);
        // -----------------------------------------------
        theChooser.setWindowTitle(tr("Choose a contact"));
        if (theChooser.exec() != QDialog::Accepted)
            return;
        // -----------------------------------------------
        qstrContactId = theChooser.GetCurrentID();
    }
    // ---------------------------------------------------
    emit messageContact(qstrMyNymId, qstrContactId);
}

void Activity::on_toolButtonInvoiceContact_clicked()
{
    qDebug() << "on_toolButtonInvoiceContact_clicked was successfully called.";

    QString  qstrTLA;
    QString  qstrAssetTypeId;
    QString  qstrAccountId;
    QString  qstrServerId;
    QString  qstrContactId;
    bool     bSelectedSNP;
    bool     bSelectedHosted;
    bool     bSelectedNotary;
    bool     bSelectedAcctLowLevel;
    bool     bSelectedAccount;
    bool     bSelectedAsset;
    bool     bSelectedContact;

    RetrieveSelectedIds(qstrTLA,
                        qstrAssetTypeId,
                        qstrAccountId,
                        qstrServerId,
                        qstrContactId,
                        bSelectedSNP,
                        bSelectedHosted,
                        bSelectedNotary,
                        bSelectedAcctLowLevel,
                        bSelectedAccount,
                        bSelectedAsset,
                        bSelectedContact
                        );
    // ---------------------------------------------------
    // If the qstrContactId is empty, need to make the user choose from a list.
    //
    if (qstrContactId.isEmpty())
    {
        // Pop up a Contact selection box. The user chooses an existing contact.
        //
        DlgChooser theChooser(this);
        // -----------------------------------------------
        mapIDName & the_map = theChooser.m_map;
        MTContactHandler::getInstance()->GetOpentxsContacts(the_map);
        // -----------------------------------------------
        theChooser.setWindowTitle(tr("Choose a contact"));
        if (theChooser.exec() != QDialog::Accepted)
            return;
        // -----------------------------------------------
        qstrContactId = theChooser.GetCurrentID();
    }
    // ---------------------------------------------------
    emit requestToAccountFromContact(qstrAccountId, qstrContactId);
}

void Activity::on_toolButtonImportCash_clicked()
{
    qDebug() << "on_toolButtonImportCash_clicked was successfully called.";

    emit sig_on_toolButton_importCash_clicked();
}

void Activity::on_toolButtonSettings_clicked()
{
    qDebug() << "on_toolButtonSettings_clicked was successfully called.";

    emit showSettings();
}

void Activity::on_toolButton_payments_clicked() { emit sig_on_toolButton_payments_clicked(); }
void Activity::on_toolButton_pending_clicked() { emit sig_on_toolButton_pending_clicked(); }
void Activity::on_toolButton_markets_clicked() { emit sig_on_toolButton_markets_clicked(); }
void Activity::on_toolButton_importCash_clicked() { emit sig_on_toolButton_importCash_clicked(); }
void Activity::on_toolButton_contacts_clicked() { emit sig_on_toolButton_contacts_clicked(); }
void Activity::on_toolButton_smartContracts_clicked() { emit sig_on_toolButton_smartContracts_clicked(); }
void Activity::on_toolButton_Corporations_clicked() { emit sig_on_toolButton_Corporations_clicked(); }
void Activity::on_toolButton_quit_clicked() { emit sig_on_toolButton_quit_clicked(); }
void Activity::on_toolButton_secrets_clicked() { emit sig_on_toolButton_secrets_clicked(); }
void Activity::on_toolButton_encrypt_clicked() { emit sig_on_toolButton_encrypt_clicked(); }
void Activity::on_toolButton_sign_clicked() { emit sig_on_toolButton_sign_clicked(); }
void Activity::on_toolButton_decrypt_clicked() { emit sig_on_toolButton_decrypt_clicked(); }
void Activity::on_toolButton_transport_clicked() { emit sig_on_toolButton_transport_clicked(); }
void Activity::on_toolButton_liveAgreements_clicked() { emit sig_on_toolButton_liveAgreements_clicked(); }


