#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include "gui/widgets/nymdetails.hpp"
#include <ui_nymdetails.h>

#include <gui/widgets/credentials.hpp>
#include <gui/widgets/wizardaddnym.hpp>
#include <gui/widgets/wizardeditprofile.hpp>
#include <gui/widgets/overridecursor.hpp>

#include <opentxs/opentxs.hpp>

#include <gui/widgets/qrtoolbutton.hpp>

#include <core/handlers/DBHandler.hpp>
#include <core/handlers/contacthandler.hpp>
#include <core/handlers/modelclaims.hpp>
#include <core/handlers/modelverifications.hpp>
//#include <core/mtcomms.h>
#include <core/moneychanger.hpp>


// #include <opentxs/api/client/ServerAction.hpp>
// #include <opentxs/api/client/Sync.hpp>
// #include <opentxs/api/Api.hpp>
// #include <opentxs/api/Native.hpp>
// #include <opentxs/client/OT_API.hpp>
// #include <opentxs/client/OTAPI_Exec.hpp>
// #include <opentxs/client/ServerAction.hpp>
// #include <opentxs/client/Utility.hpp>
// #include <opentxs/core/NumList.hpp>
// #include <opentxs/core/Identifier.hpp>
// #include <opentxs/core/Nym.hpp>
// #include <opentxs/OT.hpp>
// #include <opentxs/Proto.hpp>
// #include <opentxs/Types.hpp>

//#include <namecoin/Namecoin.hpp>

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QStringList>
#include <QMessageBox>
#include <QClipboard>
#include <QStringList>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QPlainTextEdit>
#include <QDebug>
#include <QGroupBox>
#include <QTableWidget>
#include <QMenu>
#include <QAction>
#include <QLineEdit>
#include <QToolButton>
#include <QScopedPointer>
#include <QLabel>
#include <QSpacerItem>
#include <QHeaderView>

#include <map>
#include <tuple>
#include <string>



#define TREE_ITEM_TYPE_RELATIONSHIP    10
#define TREE_ITEM_TYPE_CLAIM           11
#define TREE_ITEM_TYPE_VERIFICATION    12



void MTNymDetails::ClearTree()
{
//    metInPerson_ = nullptr;

    if (treeWidgetClaims_)
    {
        treeWidgetClaims_->blockSignals(true);
        treeWidgetClaims_->clear();
        treeWidgetClaims_->blockSignals(false);
    }
}

int claimPolarityToInt(opentxs::ClaimPolarity polarity)
{
    int nReturnValue = 2;

    switch(polarity)
    {
    case opentxs::ClaimPolarity::NEUTRAL:  nReturnValue = 0; break;
    case opentxs::ClaimPolarity::POSITIVE: nReturnValue = 1; break;
    case opentxs::ClaimPolarity::NEGATIVE: nReturnValue = 2; break;
    default:                               nReturnValue = 2; break;
    }
    return nReturnValue;
}

opentxs::ClaimPolarity intToClaimPolarity(int polarity)
{
    opentxs::ClaimPolarity returnValue = opentxs::ClaimPolarity::NEUTRAL;

    switch(polarity)
    {
    case 0:  returnValue = opentxs::ClaimPolarity::NEUTRAL;  break;
    case 1:  returnValue = opentxs::ClaimPolarity::POSITIVE; break;
    case 2:  returnValue = opentxs::ClaimPolarity::NEGATIVE; break;
    default: returnValue = opentxs::ClaimPolarity::NEUTRAL;  break;
    }
    return returnValue;
}


//resume
void MTNymDetails::onClaimsUpdatedForNym(QString nymId)
{
    // Get the Nym as private, to see if we definitely own this as a private Nym.
    // If so, then we have just finished editing the claims on one of our own Nyms.
    // (That's every single case inside this file, but we may have received the signal
    // here from some other screen emitting it, like ContactDetails.)
    // At this point we want to re-register the Nym on all its servers, in order to
    // update each server's copy of the Nym's credentials containing the updated
    // claims.



//    qDebug() << "DEBUGGING: onClaimsUpdatedForNym 1 ";



    if (nymId.isEmpty())
        return;

//    qDebug() << "DEBUGGING: onClaimsUpdatedForNym 2 ";

    // -------------------------------------
    std::string         str_nym_id  (nymId.toStdString());
    auto                strNymId =  opentxs::String::Factory(str_nym_id);
    auto   id_nym      = opentxs::Identifier::Factory(strNymId);
    // -------------------------------------
    std::shared_ptr<const opentxs::Nym> pCurrentNym = Moneychanger::It()->OT().Wallet().Nym(id_nym) ;

    if (false == bool(pCurrentNym))
        return;


//    qDebug() << "DEBUGGING: onClaimsUpdatedForNym 3 ";

    const int32_t server_count = Moneychanger::It()->OT().Exec().GetServerCount();
    // -----------------------------------------------
    // Loop through all the servers and for each, see if the Nym  is registered
    // there. For every server that he IS registered on, RE-register so it has
    // the latest copy of the credentials.
    //
    for (int32_t ii = 0; ii < server_count; ++ii)
    {
        QString notary_id = QString::fromStdString(Moneychanger::It()->OT().Exec().GetServer_ID(ii));
        // -----------------------------------------------
        if (!notary_id.isEmpty())
        {
            const bool isReg = Moneychanger::It()->OT().Exec().IsNym_RegisteredAtServer(str_nym_id, notary_id.toStdString());

            if (isReg) // We only RE-REGISTER at servers where we're ALREADY registered.
            {          // (To update their copy of the credentials we just edited.)
                std::string response;
                {
                    MTSpinner theSpinner;

                    response = opentxs::String::Factory(Moneychanger::It()->OT().Sync().RegisterNym(opentxs::Identifier::Factory(str_nym_id),
                                                                                           opentxs::Identifier::Factory(notary_id.toStdString()), true))->Get();
                    if (response.empty() && !Moneychanger::It()->OT().Exec().CheckConnection(notary_id.toStdString()))
                    {
                        QString qstrErrorMsg;
                        qstrErrorMsg = QString("%1: %2. %3.").
                                arg(tr("Failed trying to contact notary")).
                                arg(notary_id).arg(tr("Perhaps it is down, or there might be a network problem"));
                        emit appendToLog(qstrErrorMsg);
                        continue;
                    }
                }

                if (!opentxs::VerifyMessageSuccess(Moneychanger::It()->OT(), response)) {
                    Moneychanger::It()->HasUsageCredits(notary_id, nymId);
                    continue;
                }
                else
                    MTContactHandler::getInstance()->NotifyOfNymServerPair(nymId, notary_id);
            }
        }
    }
    // -----------------------------------------------
    emit RefreshRecordsAndUpdateMenu();
}



void MTNymDetails::RefreshTree(const QString & qstrNymId)
{
    if ( (nullptr == ui) || !treeWidgetClaims_ )
        return;
    // ----------------------------------------
//    ClearTree();
    m_pPlainTextEditNotes->setPlainText(QString(""));
    // ----------------------------------------
    if ( qstrNymId.isEmpty() )
        return;
    // ----------------------------------------
    treeWidgetClaims_->blockSignals(true);
    // ---------------------------------------
    const std::string str_nym_id   = qstrNymId.toStdString();
    const auto id_nym = opentxs::Identifier::Factory(str_nym_id);
    // ----------------------------------------
    auto pNym = Moneychanger::It()->OT().Wallet().Nym(id_nym);

    if (!pNym) {
        qDebug() << "MTNymDetails::RefreshTree: No Nym found in wallet for ID: " << qstrNymId;
        return;
    }
    const std::string str_nym_name = pNym->Alias();
    // ----------------------------------------
    std::string str_nym_contact_data;
    auto claims = Moneychanger::It()->OT().OTAPI().GetContactData(id_nym);
    if (claims) {
        str_nym_contact_data = opentxs::ContactData::PrintContactData(*claims);
    }

    const QString qstrNymContactData = QString::fromStdString(str_nym_contact_data);

    m_pPlainTextEditNotes->setPlainText(qstrNymContactData);

//    // Before commencing with the main act, let's iterate all the Nyms once,
//    // and construct a map, so we don't end up doing this multiple times below
//    // unnecessarily.
//    //
//    typedef std::pair<std::string, opentxs::ClaimSet> NymClaims;
//    typedef std::map <std::string, opentxs::ClaimSet> mapOfNymClaims;
//    typedef std::map <std::string, std::string> mapOfNymNames;
//
////  mapOfNymClaims nym_claims; // Each pair in this map has a NymID and a ClaimSet.
//    mapOfNymNames  nym_names;  // Each pair in this map has a NymID and a Nym Name.
//
//    if (!str_nym_id.empty())
//    {
//        auto pCurrentNym = Moneychanger::It()->OT().Wallet().Nym(id_nym);
//
//        if (pCurrentNym)
//        {
//            opentxs::ClaimSet claims = Moneychanger::It()->OT().OTAPI().GetClaims(*pCurrentNym);
//            // ---------------------------------------
////          nym_claims.insert( NymClaims(str_nym_id, claims) );
//            nym_names.insert(std::pair<std::string, std::string>(str_nym_id, str_nym_name));
//        }
//    }
//
//    // -------------------------------------------------
//    // This means NONE of the contact's Nyms had any claims.
//    // (So there's nothing to put on this tree. Done.)
////    if (nym_claims.empty())
////        return;
//    // UPDATE: Even if Bob has no claims, ALICE might have made a relationship claim ABOUT
//    // Bob, which would be displayed here (giving Bob the opportunity to confirm/refute.)
//    // (So we can't just return here.)
//    // -------------------------------------------------
////  QPointer<ModelClaims>      pModelClaims_;
////  QPointer<ClaimsProxyModel> pProxyModelClaims_;

//    pProxyModelClaims_ = nullptr;
//    pModelClaims_ = DBHandler::getInstance()->getClaimsModel(qstrNymId);

//    if (!pModelClaims_)
//        return;

//    pProxyModelClaims_ = new ClaimsProxyModel;
//    pProxyModelClaims_->setSourceModel(pModelClaims_);
//    // -------------------------------------------------
//    // We make a button group for each combination of Nym, Section, and Type.
//    // Therefore I make a map with a KEY of: tuple<Nym, Section, Type>
//    // The VALUE will be pointers to QButtonGroup.
//    //
////    const std::string claim_nym_id  = qvarNymId  .isValid() ? qvarNymId.toString().toStdString() : "";
////    const uint32_t    claim_section = qvarSection.isValid() ? qvarSection.toUInt() : 0;
////    const uint32_t    claim_type    = qvarType   .isValid() ? qvarType.toUInt() : 0;

//    typedef std::tuple<std::string, uint32_t, uint32_t> ButtonGroupKey;
//    typedef std::map<ButtonGroupKey, QButtonGroup *> mapOfButtonGroups;

//    mapOfButtonGroups mapButtonGroups;
//    // -------------------------------------------------

//    // Insert "Has Met" into Tree.
//    //
//    QString  qstrMetLabel = QString("<b>%1</b>").arg(tr("Relationship claims"));
//    QLabel * label = new QLabel(qstrMetLabel, treeWidgetClaims_);

//    metInPerson_ = new QTreeWidgetItem;

////  metInPerson_->setText(0, "Met in person");
//    treeWidgetClaims_->addTopLevelItem(metInPerson_);
//    treeWidgetClaims_->expandItem(metInPerson_);
//    treeWidgetClaims_->setItemWidget(metInPerson_, 0, label);
//    // ------------------------------------------
//    // Earlier above, we made a list of all the claims from other Nyms
//    // that they "have met" the Nyms represented by this Contact.
//    // Now let's add those here as sub-items under the metInPerson_ top-level item.
//    //
//    QPointer<ModelClaims> pRelationships = DBHandler::getInstance()->getRelationshipClaims(qstrNymId);

//    if (!pRelationships) // Should never happen, even if the result set is empty.
//        return;

//    QPointer<ClaimsProxyModel> pProxyModelRelationships = new ClaimsProxyModel;

//    pProxyModelRelationships->setSourceModel(pRelationships);

//    if (pRelationships->rowCount() > 0)
//    {
//        // First grab the various relationship type names:  (Parent of, have met, child of, etc.)
//        QMap<uint32_t, QString> mapTypeNames;
//        // ----------------------------------------
//        const std::string sectionName =
//            Moneychanger::It()->OT().Exec().ContactSectionName(
//                opentxs::proto::CONTACTSECTION_RELATIONSHIP);
//        const auto sectionTypes =
//            Moneychanger::It()->OT().Exec().ContactSectionTypeList(
//                opentxs::proto::CONTACTSECTION_RELATIONSHIP);

//        for (const auto& indexSectionType: sectionTypes) {
//            const std::string typeName =
//                Moneychanger::It()->OT().Exec().ContactTypeName(indexSectionType);
//            mapTypeNames.insert(
//                indexSectionType,
//                QString::fromStdString(typeName));
//        }
//        // ---------------------------------------

//        for (int nRelationshipCount = 0; nRelationshipCount < pProxyModelRelationships->rowCount(); nRelationshipCount++)
//        {
////            QSqlRecord record = pRelationships->record(nRelationshipCount);

//            QModelIndex proxyIndexZero        = pProxyModelRelationships->index(nRelationshipCount, 0);
//            QModelIndex sourceIndexZero       = pProxyModelRelationships->mapToSource(proxyIndexZero);
//            // ----------------------------------------------------------------------------
//            QModelIndex sourceIndexClaimId    = pRelationships->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_CLAIM_ID,    sourceIndexZero);
//            QModelIndex sourceIndexNymId      = pRelationships->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_NYM_ID,      sourceIndexZero);
//            QModelIndex sourceIndexSection    = pRelationships->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_SECTION,     sourceIndexZero);
//            QModelIndex sourceIndexType       = pRelationships->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_TYPE,        sourceIndexZero);
//            QModelIndex sourceIndexValue      = pRelationships->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_VALUE,       sourceIndexZero);
//            QModelIndex sourceIndexStart      = pRelationships->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_START,       sourceIndexZero);
//            QModelIndex sourceIndexEnd        = pRelationships->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_END,         sourceIndexZero);
//            QModelIndex sourceIndexAttributes = pRelationships->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_ATTRIBUTES,  sourceIndexZero);
//            QModelIndex sourceIndexAttActive  = pRelationships->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_ATT_ACTIVE,  sourceIndexZero);
//            QModelIndex sourceIndexAttPrimary = pRelationships->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_ATT_PRIMARY, sourceIndexZero);
//            // ----------------------------------------------------------------------------
//            QModelIndex proxyIndexValue       = pProxyModelRelationships->mapFromSource(sourceIndexValue);
//            QModelIndex proxyIndexStart       = pProxyModelRelationships->mapFromSource(sourceIndexStart);
//            QModelIndex proxyIndexEnd         = pProxyModelRelationships->mapFromSource(sourceIndexEnd);
//            // ----------------------------------------------------------------------------
//            QVariant    qvarClaimId           = pRelationships->data(sourceIndexClaimId);
//            QVariant    qvarNymId             = pRelationships->data(sourceIndexNymId);
//            QVariant    qvarSection           = pRelationships->data(sourceIndexSection);
//            QVariant    qvarType              = pRelationships->data(sourceIndexType);
//            QVariant    qvarValue             = pProxyModelRelationships->data(proxyIndexValue); // Proxy here since the proxy model decodes this. UPDATE: no longer encoded.
//            QVariant    qvarStart             = pProxyModelRelationships->data(proxyIndexStart); // Proxy for these two since it formats the
//            QVariant    qvarEnd               = pProxyModelRelationships->data(proxyIndexEnd);   // timestamp as a human-readable string.
//            QVariant    qvarAttributes        = pRelationships->data(sourceIndexAttributes);
//            QVariant    qvarAttActive         = pRelationships->data(sourceIndexAttActive);
//            QVariant    qvarAttPrimary        = pRelationships->data(sourceIndexAttPrimary);
//            // ----------------------------------------------------------------------------
//            const QString qstrClaimId       = qvarClaimId.isValid() ? qvarClaimId.toString() : "";
//            const QString qstrClaimValue    = qvarValue  .isValid() ? qvarValue  .toString() : "";
//            // ----------------------------------------------------------------------------
//            const std::string claim_id      = qstrClaimId   .isEmpty() ? "" : qstrClaimId.toStdString();
//            const std::string claim_value   = qstrClaimValue.isEmpty() ? "" : qstrClaimValue.toStdString();
//            const std::string claim_nym_id  = qvarNymId     .isValid() ? qvarNymId.toString().toStdString() : "";
//            const uint32_t    claim_section = qvarSection   .isValid() ? qvarSection.toUInt() : 0;
//            const uint32_t    claim_type    = qvarType      .isValid() ? qvarType.toUInt() : 0;
//            // ----------------------------------------------------------------------------
//            const bool        claim_active  = qvarAttActive .isValid() ? qvarAttActive .toBool() : false;
//            const bool        claim_primary = qvarAttPrimary.isValid() ? qvarAttPrimary.toBool() : false;
//            // ----------------------------------------------------------------------------
//            QMap<uint32_t, QString>::iterator it_typeNames = mapTypeNames.find(claim_type);
//            QString qstrTypeName;

//            if (it_typeNames != mapTypeNames.end())
//                qstrTypeName = it_typeNames.value();
//            // ---------------------------------------
//            const std::string str_claimant_name = Moneychanger::It()->OT().Exec().GetNym_Name(claim_nym_id);

//            // Add the claim to the tree.
//            //
//            QTreeWidgetItem * claim_item = new QTreeWidgetItem;
//            // ---------------------------------------

//            // ALICE claims she HAS MET *CHARLIE*.

//            // str_claimant_name claims she qstrTypeName nym_names[claim_value]

//            mapOfNymNames::iterator it_names = nym_names.find(claim_value);
//            std::string str_nym_name;

//            if (nym_names.end() != it_names)
//                str_nym_name =  it_names->second;
//            else
//                str_nym_name = claim_value;

//            const QString qstrClaimantLabel = QString("%1: %2").arg(tr("Claimant")).arg(QString::fromStdString(str_claimant_name));

//            claim_item->setText(0, qstrClaimantLabel);      // "Alice" (some lady) from NymId
//            claim_item->setText(1, qstrTypeName);           // "Has met" (or so she claimed)
//            claim_item->setText(2, QString::fromStdString(str_nym_name)); // "Charlie" (the current Nym whose details we're viewing.)

//            claim_item->setData(0, Qt::UserRole+1, TREE_ITEM_TYPE_RELATIONSHIP);

//            claim_item->setData(0, Qt::UserRole, QString::fromStdString(claim_nym_id)); // Alice's Nym Id. The person who made the claim. Claimant Nym Id.
//            claim_item->setData(1, Qt::UserRole, qstrClaimId);
//            claim_item->setData(2, Qt::UserRole, qstrClaimValue); // Verifier Nym Id. Alice made a claim about Charlie (me), who verifies her claim. So Charlie's ID goes in Alice's claim_value.
//            // ----------------------------------------
//            // Since this is someone else's claim about me, I should see if I have already confirmed or refuted it.
//            bool bPolarity = false;
//            const bool bGotPolarity = MTContactHandler::getInstance()->getPolarityIfAny(qstrClaimId,
//                                                                                        QString::fromStdString(str_nym_id), bPolarity);

////            qDebug() << "DEBUGGING NYM DETAILS: QString::fromStdString(claim_value): " << QString::fromStdString(claim_value);
////            qDebug() << "DEBUGGING NYM DETAILS: QString::fromStdString(str_nym_id): " << QString::fromStdString(str_nym_id);

//            if (bGotPolarity)
//            {
//                claim_item->setText(5, bPolarity ? tr("Confirmed") : tr("Refuted") );
//                claim_item->setBackgroundColor(5, bPolarity ? QColor("green") : QColor("red"));
//            }
//            else
//                claim_item->setText(5, tr("No comment"));

//            opentxs::ClaimPolarity claimPolarity = (!bGotPolarity ? opentxs::ClaimPolarity::NEUTRAL :
//                (bPolarity ? opentxs::ClaimPolarity::POSITIVE : opentxs::ClaimPolarity::NEGATIVE));

//            claim_item->setData(5, Qt::UserRole, QVariant::fromValue(claimPolarityToInt(claimPolarity))); // Confirmed or refuted. (Or none.) Polarity stored here.
//            // ---------------------------------------
////          claim_item->setFlags(claim_item->flags() |     Qt::ItemIsEditable);
//            claim_item->setFlags(claim_item->flags() & ~ ( Qt::ItemIsEditable | Qt::ItemIsUserCheckable) );
//            // ---------------------------------------
//            metInPerson_->addChild(claim_item);
//            treeWidgetClaims_->expandItem(claim_item);
//            // ----------------------------------------------------------------------------
//            // Couldn't do this until now, when the claim_item has been added to the tree.
//            //
////          typedef std::tuple<std::string, uint32_t, uint32_t> ButtonGroupKey;
////          typedef std::map<ButtonGroupKey, QButtonGroup *> mapOfButtonGroups;
////          mapOfButtonGroups mapButtonGroups;

////            ButtonGroupKey keyBtnGroup{claim_nym_id, claim_section, claim_type};
////            mapOfButtonGroups::iterator it_btn_group = mapButtonGroups.find(keyBtnGroup);
////            QButtonGroup * pButtonGroup = nullptr;

////            if (mapButtonGroups.end() != it_btn_group)
////                pButtonGroup = it_btn_group->second;
////            else
////            {
////                // The button group doesn't exist yet, for this tuple.
////                // (So let's create it.)
////                //
////                pButtonGroup = new QButtonGroup(treeWidgetClaims_);
////                mapButtonGroups.insert(std::pair<ButtonGroupKey, QButtonGroup *>(keyBtnGroup, pButtonGroup));
////            }
////            { // "Primary"
////            QRadioButton * pRadioBtn = new QRadioButton(treeWidgetClaims_);
////            pButtonGroup->addButton(pRadioBtn);
////            pRadioBtn->setChecked(claim_primary);
////            pRadioBtn->setEnabled(false);
////            // ---------
////            treeWidgetClaims_->setItemWidget(claim_item, 3, pRadioBtn);
////            }
////            // ----------------------------------------------------------------------------
////            { // "Active"
////            QRadioButton * pRadioBtn = new QRadioButton(treeWidgetClaims_);
////            pRadioBtn->setChecked(claim_active);
////            pRadioBtn->setEnabled(false);
////            // ---------
////            treeWidgetClaims_->setItemWidget(claim_item, 4, pRadioBtn);
////            }

//         } // for (relationships)
//    }

//    // Now we loop through the sections, and for each, we populate its
//    // itemwidgets by looping through the nym_claims we got above.
//    const auto sections = Moneychanger::It()->OT().Exec().ContactSectionList();

//    for (const auto& indexSection: sections) {
//        if (opentxs::proto::CONTACTSECTION_RELATIONSHIP == indexSection)
//            continue;
//        // ----------------------------------------
//        QMap<uint32_t, QString> mapTypeNames;

//        std::string sectionName =
//            Moneychanger::It()->OT().Exec().ContactSectionName(indexSection);
//        const auto sectionTypes =
//            Moneychanger::It()->OT().Exec().ContactSectionTypeList(indexSection);

//        for (auto& indexSectionType: sectionTypes) {
//            const std::string typeName =
//                Moneychanger::It()->OT().Exec().ContactTypeName(indexSectionType);
//            mapTypeNames.insert(
//                indexSectionType,
//                QString::fromStdString(typeName));
//        }
//        // ---------------------------------------
//        // Insert Section into Tree.
//        //
//        QTreeWidgetItem * topLevel = new QTreeWidgetItem;
//        // ------------------------------------------
//        topLevel->setText(0, QString::fromStdString(sectionName));
//        // ------------------------------------------
//        treeWidgetClaims_->addTopLevelItem(topLevel);
//        treeWidgetClaims_->expandItem(topLevel);
//        // ------------------------------------------
//        for (int ii = 0; ii < pProxyModelClaims_->rowCount(); ++ii)
//        {
//            QModelIndex proxyIndexZero        = pProxyModelClaims_->index(ii, 0);
//            QModelIndex sourceIndexZero       = pProxyModelClaims_->mapToSource(proxyIndexZero);
//            // ----------------------------------------------------------------------------
//            QModelIndex sourceIndexClaimId    = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_CLAIM_ID,    sourceIndexZero);
//            QModelIndex sourceIndexNymId      = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_NYM_ID,      sourceIndexZero);
//            QModelIndex sourceIndexSection    = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_SECTION,     sourceIndexZero);
//            QModelIndex sourceIndexType       = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_TYPE,        sourceIndexZero);
//            QModelIndex sourceIndexValue      = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_VALUE,       sourceIndexZero);
//            QModelIndex sourceIndexStart      = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_START,       sourceIndexZero);
//            QModelIndex sourceIndexEnd        = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_END,         sourceIndexZero);
//            QModelIndex sourceIndexAttributes = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_ATTRIBUTES,  sourceIndexZero);
//            QModelIndex sourceIndexAttActive  = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_ATT_ACTIVE,  sourceIndexZero);
//            QModelIndex sourceIndexAttPrimary = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_ATT_PRIMARY, sourceIndexZero);
//            // ----------------------------------------------------------------------------
//            QModelIndex proxyIndexValue       = pProxyModelClaims_->mapFromSource(sourceIndexValue);
//            QModelIndex proxyIndexStart       = pProxyModelClaims_->mapFromSource(sourceIndexStart);
//            QModelIndex proxyIndexEnd         = pProxyModelClaims_->mapFromSource(sourceIndexEnd);
//            // ----------------------------------------------------------------------------
//            QVariant    qvarClaimId           = pModelClaims_->data(sourceIndexClaimId);
//            QVariant    qvarNymId             = pModelClaims_->data(sourceIndexNymId);
//            QVariant    qvarSection           = pModelClaims_->data(sourceIndexSection);
//            QVariant    qvarType              = pModelClaims_->data(sourceIndexType);
//            QVariant    qvarValue             = pProxyModelClaims_->data(proxyIndexValue); // Proxy here since the proxy model decodes this. UPDATE: No longer encoded!
//            QVariant    qvarStart             = pProxyModelClaims_->data(proxyIndexStart); // Proxy for these two since it formats the
//            QVariant    qvarEnd               = pProxyModelClaims_->data(proxyIndexEnd);   // timestamp as a human-readable string.
//            QVariant    qvarAttributes        = pModelClaims_->data(sourceIndexAttributes);
//            QVariant    qvarAttActive         = pModelClaims_->data(sourceIndexAttActive);
//            QVariant    qvarAttPrimary        = pModelClaims_->data(sourceIndexAttPrimary);
//            // ----------------------------------------------------------------------------
//            const QString     qstrClaimId =  qvarClaimId.isValid() ? qvarClaimId.toString() : "";
//            const std::string claim_id    = !qstrClaimId.isEmpty() ? qstrClaimId.toStdString() : "";
//            // ----------------------------------------------------------------------------
//            const std::string claim_nym_id  = qvarNymId  .isValid() ? qvarNymId.toString().toStdString() : "";
//            const uint32_t    claim_section = qvarSection.isValid() ? qvarSection.toUInt() : 0;
//            const uint32_t    claim_type    = qvarType   .isValid() ? qvarType.toUInt() : 0;
//            const std::string claim_value   = qvarValue  .isValid() ? qvarValue.toString().toStdString() : "";
//            // ----------------------------------------------------------------------------
//            const bool        claim_active  = qvarAttActive .isValid() ? qvarAttActive .toBool() : false;
//            const bool        claim_primary = qvarAttPrimary.isValid() ? qvarAttPrimary.toBool() : false;
//            // ----------------------------------------------------------------------------
//            if (claim_section != indexSection)
//                continue;

//            QMap<uint32_t, QString>::iterator it_typeNames = mapTypeNames.find(claim_type);
//            QString qstrTypeName;

//            if (it_typeNames != mapTypeNames.end())
//                qstrTypeName = it_typeNames.value();
//            // ---------------------------------------
//            // Add the claim to the tree.
//            //
//            QTreeWidgetItem * claim_item = new QTreeWidgetItem;
//            // ---------------------------------------
//            claim_item->setText(0, QString::fromStdString(claim_value)); // "james@blah.com"
//            claim_item->setText(1, qstrTypeName);                        // "Personal"
//            claim_item->setText(2, QString::fromStdString(nym_names[claim_nym_id]));

//            claim_item->setData(0, Qt::UserRole+1, TREE_ITEM_TYPE_CLAIM);

//            claim_item->setData(2, Qt::UserRole, QString::fromStdString(claim_nym_id));  // Claimant aka Me.
//            // ---------------------------------------
////          claim_item->setCheckState(3, claim_primary ? Qt::Checked : Qt::Unchecked); // Moved below (as radio button)
//            claim_item->setCheckState(4, claim_active  ? Qt::Checked : Qt::Unchecked);
//            // ---------------------------------------
//            // NOTE: We'll do this for Nyms, not for Contacts.
//            // At least, not for claims. (Contacts will be able to edit
//            // their own verifications, though.)
//            //
////          claim_item->setFlags(claim_item->flags() |     Qt::ItemIsEditable);
//            claim_item->setFlags(claim_item->flags() & ~ ( Qt::ItemIsEditable | Qt::ItemIsUserCheckable) );
//            // ---------------------------------------
//            topLevel->addChild(claim_item);
//            treeWidgetClaims_->expandItem(claim_item);
//            // --------
//            // Couldn't do this until now, when the claim_item has been added to the tree.
//            //
////          typedef std::tuple<std::string, uint32_t, uint32_t> ButtonGroupKey;
////          typedef std::map<ButtonGroupKey, QButtonGroup *> mapOfButtonGroups;
////          mapOfButtonGroups mapButtonGroups;

//            ButtonGroupKey keyBtnGroup{claim_nym_id, claim_section, claim_type};
//            mapOfButtonGroups::iterator it_btn_group = mapButtonGroups.find(keyBtnGroup);
//            QButtonGroup * pButtonGroup = nullptr;

//            if (mapButtonGroups.end() != it_btn_group)
//                pButtonGroup = it_btn_group->second;
//            else
//            {
//                // The button group doesn't exist yet, for this tuple.
//                // (So let's create it.)
//                //
//                pButtonGroup = new QButtonGroup(treeWidgetClaims_);
//                mapButtonGroups.insert(std::pair<ButtonGroupKey, QButtonGroup *>(keyBtnGroup, pButtonGroup));
//            }

//            QRadioButton * pRadioBtn = new QRadioButton(treeWidgetClaims_);
//            pButtonGroup->addButton(pRadioBtn);
//            pRadioBtn->setChecked(claim_primary);
//            pRadioBtn->setEnabled(false);
//            // --------
//            treeWidgetClaims_->setItemWidget(claim_item, 3, pRadioBtn);
//            // ----------------------------------------------------------------------------
//            // VERIFICATIONS
//            // ----------------------------------------------------------------------------
//            // So we want to get the verifications known in the database for the
//            // current claim.
//            //
//            QPointer<ModelVerifications> pVerifications = DBHandler::getInstance()->getVerificationsModel(qstrClaimId);

//            if (!pVerifications)
//                continue;
//            // ----------------------------------------------------------------------------
//            // Here we now add the sub-widgets for the claim verifications.
//            //
//            QPointer<VerificationsProxyModel> pProxyModelVerifications = new VerificationsProxyModel;
//            pProxyModelVerifications->setSourceModel(pVerifications);
//            // ------------------------------------------
//            for (int iii = 0; iii < pProxyModelVerifications->rowCount(); ++iii)
//            {
//                QModelIndex proxyIndexZero        = pProxyModelVerifications->index(iii, 0);
//                QModelIndex sourceIndexZero       = pProxyModelVerifications->mapToSource(proxyIndexZero);
//                // ----------------------------------------------------------------------------
//                QModelIndex sourceIndexVerId       = pVerifications->sibling(sourceIndexZero.row(), VERIFY_SOURCE_COL_VERIFICATION_ID, sourceIndexZero);
//                QModelIndex sourceIndexClaimantId  = pVerifications->sibling(sourceIndexZero.row(), VERIFY_SOURCE_COL_CLAIMANT_NYM_ID, sourceIndexZero);
//                QModelIndex sourceIndexVerifierId  = pVerifications->sibling(sourceIndexZero.row(), VERIFY_SOURCE_COL_VERIFIER_NYM_ID, sourceIndexZero);
//                QModelIndex sourceIndexClaimId     = pVerifications->sibling(sourceIndexZero.row(), VERIFY_SOURCE_COL_CLAIM_ID,        sourceIndexZero);
//                QModelIndex sourceIndexPolarity    = pVerifications->sibling(sourceIndexZero.row(), VERIFY_SOURCE_COL_POLARITY,        sourceIndexZero);
//                QModelIndex sourceIndexStart       = pVerifications->sibling(sourceIndexZero.row(), VERIFY_SOURCE_COL_START,           sourceIndexZero);
//                QModelIndex sourceIndexEnd         = pVerifications->sibling(sourceIndexZero.row(), VERIFY_SOURCE_COL_END,             sourceIndexZero);
//                QModelIndex sourceIndexSignature   = pVerifications->sibling(sourceIndexZero.row(), VERIFY_SOURCE_COL_SIGNATURE,       sourceIndexZero);
//                QModelIndex sourceIndexSigVerified = pVerifications->sibling(sourceIndexZero.row(), VERIFY_SOURCE_COL_SIG_VERIFIED,    sourceIndexZero);
//                // ----------------------------------------------------------------------------
//                QModelIndex proxyIndexPolarity    = pProxyModelVerifications->mapFromSource(sourceIndexPolarity);
//                QModelIndex proxyIndexStart       = pProxyModelVerifications->mapFromSource(sourceIndexStart);
//                QModelIndex proxyIndexEnd         = pProxyModelVerifications->mapFromSource(sourceIndexEnd);
//                // ----------------------------------------------------------------------------
//                QVariant    qvarVerId             = pVerifications->data(sourceIndexVerId);
//                QVariant    qvarClaimantId        = pVerifications->data(sourceIndexClaimantId);
//                QVariant    qvarVerifierId        = pVerifications->data(sourceIndexVerifierId);
//                QVariant    qvarClaimId           = pVerifications->data(sourceIndexClaimId);
//                // ----------------------------------------------------------------------------
//                QVariant    qvarPolarity          = pProxyModelVerifications->data(proxyIndexPolarity);
//                QVariant    qvarStart             = pProxyModelVerifications->data(proxyIndexStart); // Proxy for these two since it formats the
//                QVariant    qvarEnd               = pProxyModelVerifications->data(proxyIndexEnd);   // timestamp as a human-readable string.
//                // ----------------------------------------------------------------------------
//                QVariant    qvarSignature         = pVerifications->data(sourceIndexSignature);
//                QVariant    qvarSigVerified       = pVerifications->data(sourceIndexSigVerified);
//                // ----------------------------------------------------------------------------
//                const QString     qstrVerId      =  qvarVerId  .isValid() ? qvarVerId.toString() : "";
//                const std::string verif_id       = !qstrVerId  .isEmpty() ? qstrVerId.toStdString() : "";
//                // ----------------------------------------------------------------------------
//                const QString     qstrClaimantId =  qvarClaimantId.isValid() ? qvarClaimantId.toString() : "";
//                const QString     qstrVerifierId =  qvarVerifierId.isValid() ? qvarVerifierId.toString() : "";

//                const std::string claimant_id    = !qstrClaimantId.isEmpty() ? qstrClaimantId.toStdString() : "";
//                const std::string verifier_id    = !qstrVerifierId.isEmpty() ? qstrVerifierId.toStdString() : "";
//                // ----------------------------------------------------------------------------
//                const QString     qstrVerificationClaimId =  qvarClaimId.isValid() ? qvarClaimId.toString() : "";
//                const std::string verification_claim_id   = !qstrVerificationClaimId.isEmpty() ? qstrVerificationClaimId.toStdString() : "";
//                // ----------------------------------------------------------------------------
//                QString qstrPolarity(tr("No comment"));
//                const int claim_polarity =  qvarPolarity.isValid() ? qvarPolarity.toInt() : 0;
//                opentxs::ClaimPolarity claimPolarity = intToClaimPolarity(claim_polarity);

//                if (opentxs::ClaimPolarity::NEUTRAL == claimPolarity)
//                {
//                    qDebug() << __FUNCTION__ << ": ERROR! A claim verification can't have neutral polarity, since that "
//                                "means no verification exists. How did it get into the database this way?";
//                    continue;
//                }

//                const bool bPolarity = (opentxs::ClaimPolarity::NEGATIVE == claimPolarity) ? false : true;
//                qstrPolarity = bPolarity ? tr("Confirmed") : tr("Refuted");
//                // ----------------------------------------------------------------------------
//                const QString     qstrSignature   =  qvarSignature.isValid()   ? qvarSignature.toString() : "";
//                const std::string verif_signature = !qstrSignature.isEmpty()   ? qstrSignature.toStdString() : "";
//                // ---------------------------------------
//                const bool        bSigVerified    = qvarSigVerified .isValid() ? qvarSigVerified.toBool() : false;
//                // ----------------------------------------------------------------------------
//                //#define VERIFY_SOURCE_COL_VERIFICATION_ID 0
//                //#define VERIFY_SOURCE_COL_CLAIMANT_NYM_ID 1
//                //#define VERIFY_SOURCE_COL_VERIFIER_NYM_ID 2
//                //#define VERIFY_SOURCE_COL_CLAIM_ID 3
//                //#define VERIFY_SOURCE_COL_POLARITY 4
//                //#define VERIFY_SOURCE_COL_START 5
//                //#define VERIFY_SOURCE_COL_END 6
//                //#define VERIFY_SOURCE_COL_SIGNATURE 7
//                //#define VERIFY_SOURCE_COL_SIG_VERIFIED 8

////                QStringList labels = {
////                      tr("Value")
////                    , tr("Type")
////                    , tr("Nym")
////                    , tr("Primary")
////                    , tr("Active")
////                    , tr("Polarity")
////                };
//                // ---------------------------------------
//                mapOfNymNames::iterator it_names = nym_names.find(verifier_id);
//                std::string str_verifier_name;

//                if (nym_names.end() != it_names)
//                    str_verifier_name =  it_names->second;
//                else
//                    str_verifier_name = Moneychanger::It()->OT().Exec().GetNym_Name(verifier_id);

////              const QString qstrClaimIdLabel = QString("%1: %2").arg(tr("Claim Id")).arg(qstrVerificationClaimId);
//                const QString qstrClaimantIdLabel = QString("%1: %2").arg(tr("Claimant")).arg(qstrClaimantId);
//                const QString qstrVerifierIdLabel = QString("%1: %2").arg(tr("Nym Id")).arg(qstrVerifierId);
//                const QString qstrVerifierLabel = QString("%1: %2").arg(tr("Verifier")).arg(QString::fromStdString(str_verifier_name));
//                // ---------------------------------------
//                const QString qstrSignatureLabel   = QString("%1").arg(qstrSignature.isEmpty() ? tr("missing signature") : tr("signature exists"));
//                const QString qstrSigVerifiedLabel = QString("%1").arg(bSigVerified ? tr("signature verified") : tr("signature failed"));
//                // ---------------------------------------
//                // Add the verification to the tree.
//                //
//                QTreeWidgetItem * verification_item = new QTreeWidgetItem;
//                // ---------------------------------------
//                verification_item->setText(0, qstrVerifierLabel); // "Jim Bob" (the Verifier on this claim verification.)
//                verification_item->setText(1, qstrVerifierIdLabel); // with Verifier Nym Id...
//                verification_item->setText(2, qstrClaimantIdLabel); // Verifies for Claimant Nym Id...
//                verification_item->setText(3, qstrSignatureLabel);
//                verification_item->setText(4, qstrSigVerifiedLabel);

//                verification_item->setData(0, Qt::UserRole+1, TREE_ITEM_TYPE_VERIFICATION);

//                verification_item->setData(0, Qt::UserRole, qstrVerId); // Verification ID.
//                verification_item->setData(1, Qt::UserRole, qstrVerifierId); // Verifier Nym ID.
//                verification_item->setData(2, Qt::UserRole, qstrClaimantId); // Claims have the claimant ID at index 2, so I'm matching that here.
//                verification_item->setData(3, Qt::UserRole, qstrVerificationClaimId); // Claim ID stored here.
//                verification_item->setData(5, Qt::UserRole, bPolarity); // Polarity
//                // ---------------------------------------
//                verification_item->setFlags(verification_item->flags() & ~ ( Qt::ItemIsEditable | Qt::ItemIsUserCheckable) );
//    //          verification_item->setFlags(verification_item->flags() |     Qt::ItemIsEditable);
//                // ---------------------------------------
//                verification_item->setBackgroundColor(5, bPolarity ? QColor("green") : QColor("red"));
//                // ---------------------------------------
//                claim_item->addChild(verification_item);
//                treeWidgetClaims_->expandItem(verification_item);
//                // ----------------------------------------------------------------------------
//            } // for (verifications)
//        } // for (claims)
//    }

    treeWidgetClaims_->blockSignals(false);
    treeWidgetClaims_->resizeColumnToContents(0);
    treeWidgetClaims_->resizeColumnToContents(1);
    treeWidgetClaims_->resizeColumnToContents(2);
    treeWidgetClaims_->resizeColumnToContents(3);
    treeWidgetClaims_->resizeColumnToContents(4);
}

// ------------------------------------------------------
MTNymDetails::MTNymDetails(QWidget *parent, MTDetailEdit & theOwner) :
    MTEditDetails(parent, theOwner),
    ui(new Ui::MTNymDetails)
{
    ui->setupUi(this);
    this->setContentsMargins(0, 0, 0, 0);
//  this->installEventFilter(this); // NOTE: Successfully tested theory that the base class has already installed this.

    ui->lineEditDescription->setStyleSheet("QLineEdit { background-color: lightgray }");
    // ----------------------------------
//    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
//    // ----------------------------------
//    ui->tableWidget->verticalHeader()->hide();
//    // ----------------------------------
//    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
//    ui->tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
//    // ----------------------------------
//    ui->tableWidget->setSelectionMode    (QAbstractItemView::SingleSelection);
//    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
//    // ----------------------------------
//    ui->tableWidget->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
//    ui->tableWidget->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
    // ----------------------------------
    // Note: This is a placekeeper, so later on I can just erase
    // the widget at 0 and replace it with the real header widget.
    //
    m_pHeaderWidget  = new QWidget;
    ui->verticalLayout->insertWidget(0, m_pHeaderWidget);
    // ----------------------------------
    popupMenu_.reset(new QMenu(this));

    pActionRegister_ = popupMenu_->addAction(tr("Register"));
    pActionUnregister_ = popupMenu_->addAction(tr("Unregister"));
    // ----------------------------------
}

// ------------------------------------------------------

MTNymDetails::~MTNymDetails()
{
    delete ui;
    ui = nullptr;
}


void MTNymDetails::on_toolButtonDescription_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();

    if (nullptr != clipboard)
    {
        clipboard->setText(ui->lineEditDescription->text());

        QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), QString("%1:<br/>%2").
                                 arg(tr("Copied payment address to the clipboard")).
                                 arg(ui->lineEditDescription->text()));
    }
}


void MTNymDetails::on_toolButtonQrCode_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();

    if (nullptr != clipboard)
    {
        QImage image;
        ui->toolButtonQrCode->asImage(image, 200);
        QPixmap pixmapQR = QPixmap::fromImage(image);

        clipboard->setPixmap(pixmapQR);

        QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), QString("%1:<br/>%2").
                                 arg(tr("Copied QR CODE IMAGE to the clipboard, of payment address")).
                                 arg(ui->lineEditDescription->text()));
    }
}

void MTNymDetails::on_toolButton_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();

    if (pLineEditNymId_ && (nullptr != clipboard))
    {
        clipboard->setText(pLineEditNymId_->text());

        QMessageBox::information(this, tr("ID copied"), QString("%1:<br/>%2").
                                 arg(tr("Copied Nym ID to the clipboard")).
                                 arg(pLineEditNymId_->text()));
    }
}


// ----------------------------------
//virtual
int MTNymDetails::GetCustomTabCount()
{
    return Moneychanger::It()->expertMode() ? 3 : 1;
}
// ----------------------------------
//virtual
QWidget * MTNymDetails::CreateCustomTab(int nTab)
{
    const int nCustomTabCount = this->GetCustomTabCount();
    // -----------------------------
    if ((nTab < 0) || (nTab >= nCustomTabCount))
        return nullptr; // out of bounds.
    // -----------------------------
    QWidget * pReturnValue = nullptr;
    // -----------------------------
    switch (nTab)
    {
    case 0: // "Profile" tab
        if (m_pOwner)
        {
            if (pLabelNymId_)
            {
                pLabelNymId_->setParent(nullptr);
                pLabelNymId_->disconnect();
                pLabelNymId_->deleteLater();

                pLabelNymId_ = nullptr;
            }
            QString label_string = QString("%1: ").arg(tr("Nym id"));
            pLabelNymId_ = new QLabel(label_string);

//            pLabelNymId_->setVisible(false);

            if (pToolButtonNymId_)
            {
                pToolButtonNymId_->setParent(nullptr);
                pToolButtonNymId_->disconnect();
                pToolButtonNymId_->deleteLater();

                pToolButtonNymId_ = nullptr;
            }

            QPixmap pixmapCopy(":/icons/icons/Basic-Copy-icon.png");
            QIcon   copyIcon  (pixmapCopy);
            // ----------------------------------------------------------------
            pToolButtonNymId_ = new QToolButton;
            pToolButtonNymId_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            pToolButtonNymId_->setIcon(copyIcon);

            connect(pToolButtonNymId_.data(), SIGNAL(clicked()), this, SLOT(on_toolButton_clicked()));

            if (pLineEditNymId_)
            {
                pLineEditNymId_->setParent(nullptr);
                pLineEditNymId_->disconnect();
                pLineEditNymId_->deleteLater();

                pLineEditNymId_ = nullptr;
            }

            pLineEditNymId_ = new QLineEdit;
            pLineEditNymId_->setReadOnly(true);
            pLineEditNymId_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            pLineEditNymId_->setStyleSheet("QLineEdit { background-color: gray }");
            // -------------------------------
            QHBoxLayout * phBoxNymId = new QHBoxLayout;
            QWidget * pWidgetNymId   = new QWidget;

//          QMargins margins = pWidgetNymId->contentsMargins();
//          QMargins newMargins(margins.left(), margins.top(), margins.right(), 0);
//          pWidgetNymId->setContentsMargins(newMargins);

            phBoxNymId->setContentsMargins(0, 0, 0, 0);

            phBoxNymId->addWidget(pLabelNymId_);
            phBoxNymId->addWidget(pLineEditNymId_);
            phBoxNymId->addWidget(pToolButtonNymId_);

            pWidgetNymId->setLayout(phBoxNymId);
            // -------------------------------
            if (m_pPlainTextEditNotes)
            {
                m_pPlainTextEditNotes->setParent(nullptr);
                m_pPlainTextEditNotes->disconnect();
                m_pPlainTextEditNotes->deleteLater();

                m_pPlainTextEditNotes = nullptr;
            }
            m_pPlainTextEditNotes = new QPlainTextEdit;

            m_pPlainTextEditNotes->setReadOnly(true);
            m_pPlainTextEditNotes->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
            // -------------------------------
            QVBoxLayout * pvBox = new QVBoxLayout;

            QLabel * pLabelContents = new QLabel(tr("Profile (text dump):"));

            pvBox->setAlignment(Qt::AlignTop);
            pvBox->addWidget   (pLabelContents);
            pvBox->addWidget   (m_pPlainTextEditNotes);
            // -------------------------------
            pReturnValue = new QWidget;
            pReturnValue->setContentsMargins(0, 0, 0, 0);
            pReturnValue->setLayout(pvBox);
            // -------------------------------
            if (treeWidgetClaims_)
            {
                treeWidgetClaims_->setParent(nullptr);
                treeWidgetClaims_->disconnect();
                treeWidgetClaims_->deleteLater();

                treeWidgetClaims_ = nullptr;
            }

            treeWidgetClaims_ = new QTreeWidget;

            treeWidgetClaims_->setEditTriggers(QAbstractItemView::NoEditTriggers);
            treeWidgetClaims_->setAlternatingRowColors(true);
            treeWidgetClaims_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
            treeWidgetClaims_->setColumnCount(5);
            // ---------------------------------------
            QStringList labels = {
                  tr("Value")
                , tr("Type")
                , tr("Nym")
                , tr("Primary")
                , tr("Active")
                , tr("Polarity")
            };
            treeWidgetClaims_->setHeaderLabels(labels);
            treeWidgetClaims_->setVisible(false);
            // -------------------------------
            QHBoxLayout * phBox        = new QHBoxLayout;
            QPushButton * pEditButton  = new QPushButton(tr("Edit Profile"));
            QSpacerItem * pSpacerItem1 = new QSpacerItem(0, 0, QSizePolicy::Expanding);
            QSpacerItem * pSpacerItem2 = new QSpacerItem(0, 0, QSizePolicy::Expanding);

            pEditButton->setVisible(false);

            connect(pEditButton, SIGNAL(clicked(bool)), this, SLOT(on_btnEditProfile_clicked()));

            phBox->addSpacerItem(pSpacerItem1);
            phBox->addWidget(pEditButton);
            phBox->addSpacerItem(pSpacerItem2);
            // -------------------------------
//          QFrame * line = new QFrame;
//          line->setFrameShape(QFrame::HLine);
//          line->setFrameShadow(QFrame::Sunken);
            // -------------------------------
//          QLabel * pLabel = new QLabel( QString("%1:").arg(tr("Profile")) );

            pvBox->setAlignment(Qt::AlignTop);
            pvBox->addWidget(pWidgetNymId);
//          pvBox->addWidget(line);
//          pvBox->addWidget(pLabel);
            pvBox->addWidget(treeWidgetClaims_);
            pvBox->addLayout(phBox);
            // -------------------------------
            pReturnValue = new QWidget;
            pReturnValue->setContentsMargins(0, 0, 0, 0);
            pReturnValue->setLayout(pvBox);

            treeWidgetClaims_->setContextMenuPolicy(Qt::CustomContextMenu);

            connect(treeWidgetClaims_, SIGNAL(customContextMenuRequested(const QPoint &)),
                    this, SLOT(on_treeWidget_customContextMenuRequested(const QPoint &)));
        }
        break;

//    case 1: // "Credentials" tab
//        if (m_pOwner)
//        {
//            if (m_pCredentials)
//            {
//                m_pCredentials->setParent(nullptr);
//                m_pCredentials->disconnect();
//                m_pCredentials->deleteLater();

//                m_pCredentials = nullptr;
//            }
//            m_pCredentials = new MTCredentials(nullptr, *m_pOwner);
//            m_pCredentials->setContentsMargins(0,0,0,0);

//            QWidget * pWidgetCred = new QWidget;
//            QVBoxLayout * pvBox = new QVBoxLayout;

//            pvBox->setMargin(0);
//            pvBox->addWidget(m_pCredentials);

//            pWidgetCred->setLayout(pvBox);

//            pReturnValue = pWidgetCred;
//            pReturnValue->setContentsMargins(0, 0, 0, 0);
//        }
//        break;

    case 1: // "State of Nym" tab
    {
        // -------------------------------
        if (pLabelNotaries_)
        {
            pLabelNotaries_->setParent(nullptr);
            pLabelNotaries_->disconnect();
            pLabelNotaries_->deleteLater();
            pLabelNotaries_ = nullptr;

        }

        pLabelNotaries_ = new QLabel(tr("Registered on notaries:"));

        if (pTableWidgetNotaries_)
        {
            pTableWidgetNotaries_->setParent(nullptr);
            pTableWidgetNotaries_->disconnect();
            pTableWidgetNotaries_->deleteLater();
            pTableWidgetNotaries_ = nullptr;
        }

        pTableWidgetNotaries_ = new QTableWidget;
        pTableWidgetNotaries_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        pTableWidgetNotaries_->setColumnCount(3);

        pTableWidgetNotaries_->setHorizontalHeaderLabels(
                    QStringList() << tr("Notary") << tr("Status") << tr("Notary ID"));
        // ----------------------------------
        pTableWidgetNotaries_->setContextMenuPolicy(Qt::CustomContextMenu);
        // ----------------------------------
        pTableWidgetNotaries_->verticalHeader()->hide();
        // ----------------------------------
        pTableWidgetNotaries_->horizontalHeader()->setStretchLastSection(true);
        pTableWidgetNotaries_->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
        // ----------------------------------
        pTableWidgetNotaries_->setSelectionMode    (QAbstractItemView::SingleSelection);
        pTableWidgetNotaries_->setSelectionBehavior(QAbstractItemView::SelectRows);
        // ----------------------------------
        pTableWidgetNotaries_->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
        pTableWidgetNotaries_->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
        // ----------------------------------
        connect(pTableWidgetNotaries_, SIGNAL(customContextMenuRequested(const QPoint &)),
                this, SLOT(on_tableWidget_customContextMenuRequested(const QPoint &)));
        // -------------------------------
        if (m_pPlainTextEdit)
        {
            m_pPlainTextEdit->setParent(nullptr);
            m_pPlainTextEdit->disconnect();
            m_pPlainTextEdit->deleteLater();

            m_pPlainTextEdit = nullptr;
        }

        m_pPlainTextEdit = new QPlainTextEdit;

        m_pPlainTextEdit->setReadOnly(true);
        m_pPlainTextEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        // -------------------------------



        // -------------------------------
        QVBoxLayout * pvBox = new QVBoxLayout;

        QLabel * pLabelContents = new QLabel(tr("Raw State of Nym:"));

        pvBox->setAlignment(Qt::AlignTop);
        pvBox->addWidget   (pLabelNotaries_);
        pvBox->addWidget   (pTableWidgetNotaries_);
        pvBox->addWidget   (pLabelContents);
        pvBox->addWidget   (m_pPlainTextEdit);
        // -------------------------------
        pReturnValue = new QWidget;
        pReturnValue->setContentsMargins(0, 0, 0, 0);
        pReturnValue->setLayout(pvBox);
    }
        break;

    default:
        qDebug() << QString("Unexpected: MTNymDetails::CreateCustomTab was called with bad index: %1").arg(nTab);
        return nullptr;
    }
    // -----------------------------
    return pReturnValue;
}
// ---------------------------------
//virtual
QString  MTNymDetails::GetCustomTabName(int nTab)
{
    const int nCustomTabCount = this->GetCustomTabCount();
    // -----------------------------
    if ((nTab < 0) || (nTab >= nCustomTabCount))
        return QString(""); // out of bounds.
    // -----------------------------
    QString qstrReturnValue("");
    // -----------------------------
    switch (nTab)
    {
    case 0:  qstrReturnValue = "Profile";      break;
//  case 1:  qstrReturnValue = "Credentials";  break;
    case 1:  qstrReturnValue = "State";        break;

    default:
        qDebug() << QString("Unexpected: MTNymDetails::GetCustomTabName was called with bad index: %1").arg(nTab);
        return QString("");
    }
    // -----------------------------
    return qstrReturnValue;
}
// ------------------------------------------------------

void MTNymDetails::on_btnAddressAdd_clicked()
{
//    QObject * pqobjSender = QObject::sender();
//
//    if (nullptr != pqobjSender)
//    {
//        QPushButton * pBtnAdd = dynamic_cast<QPushButton *>(pqobjSender);
//
//        if (m_pAddresses && (nullptr != pBtnAdd))
//        {
//            QVariant    varNymID       = pBtnAdd->property("nymid");
//            QVariant    varMethodCombo = pBtnAdd->property("methodcombo");
//            QVariant    varAddressEdit = pBtnAdd->property("addressedit");
//            QString     qstrNymID      = varNymID.toString();
//            QComboBox * pCombo         = VPtr<QComboBox>::asPtr(varMethodCombo);
//            QLineEdit * pAddressEdit   = VPtr<QLineEdit>::asPtr(varAddressEdit);
//            QWidget   * pWidget        = VPtr<QWidget>::asPtr(pBtnAdd->property("methodwidget"));
//
//            if (!qstrNymID.isEmpty() && (nullptr != pCombo) && (nullptr != pAddressEdit) && (nullptr != pWidget))
//            {
//                int     nMethodID       = 0;
//                QString qstrAddress     = QString("");
//                QString qstrDisplayAddr = QString("");
//                // --------------------------------------------------
//                if (pCombo->currentIndex() < 0)
//                    return;
//                // --------------------------------------------------
//                QVariant varMethodID = pCombo->itemData(pCombo->currentIndex());
//                nMethodID = varMethodID.toInt();
//
//                if (nMethodID <= 0)
//                    return;
//                // --------------------------------------------------
//                qstrAddress = pAddressEdit->text();
//
//                if (qstrAddress.isEmpty())
//                    return;
//                // --------------------------------------------------
//                bool bAdded = MTContactHandler::getInstance()->AddMsgAddressToNym(qstrNymID, nMethodID, qstrAddress);
//
//                if (bAdded) // Let's add it to the GUI, too, then.
//                {
//                    QString qstrTypeDisplay = MTContactHandler::getInstance()->GetMethodTypeDisplay(nMethodID);
//                    qstrDisplayAddr = QString("%1: %2").arg(qstrTypeDisplay).arg(qstrAddress);
//                    // --------------------------------------------------
//                    QLayout     * pLayout = m_pAddresses->layout();
//                    QVBoxLayout * pVBox   = (nullptr == pLayout) ? nullptr : dynamic_cast<QVBoxLayout *>(pLayout);
//
//                    if (nullptr != pVBox)
//                    {
//                        QWidget * pNewWidget = this->createSingleAddressWidget(qstrNymID, nMethodID, qstrAddress, qstrAddress);
//
//                        if (nullptr != pNewWidget)
//                            pVBox->insertWidget(pVBox->count()-1, pNewWidget);
//                    }
//                }
//            }
//        }
//    }
}

void MTNymDetails::on_btnAddressDelete_clicked()
{
//    QObject * pqobjSender = QObject::sender();
//
//    if (nullptr != pqobjSender)
//    {
//        QPushButton * pBtnDelete = dynamic_cast<QPushButton *>(pqobjSender);
//
//        if (m_pAddresses && (nullptr != pBtnDelete))
//        {
//            QVariant  varNymID      = pBtnDelete->property("nymid");
//            QVariant  varMethodID   = pBtnDelete->property("methodid");
//            QVariant  varMethodAddr = pBtnDelete->property("methodaddr");
//            QString   qstrNymID     = varNymID     .toString();
//            int       nMethodID     = varMethodID  .toInt();
//            QString   qstrAddress   = varMethodAddr.toString();
//            QWidget * pWidget       = VPtr<QWidget>::asPtr(pBtnDelete->property("methodwidget"));
//
//            if (nullptr != pWidget)
//            {
//                bool bRemoved = MTContactHandler::getInstance()->RemoveMsgAddressFromNym(qstrNymID, nMethodID, qstrAddress);
//
//                if (bRemoved) // Let's remove it from the GUI, too, then.
//                {
//                    QLayout * pLayout = m_pAddresses->layout();
//
//                    if (nullptr != pLayout)
//                    {
//                        pLayout->removeWidget(pWidget);
//
//                        pWidget->setParent(nullptr);
//                        pWidget->disconnect();
//                        pWidget->deleteLater();
//
//                        pWidget = nullptr;
//                    }
//                }
//            }
//        }
//    }
}



//virtual
void MTNymDetails::refresh(QString strID, QString strName)
{
    if (treeWidgetClaims_)
        ClearTree();

    if ((nullptr != ui) && !strID.isEmpty())
    {
        auto pNym = Moneychanger::It()->OT().Wallet().Nym(opentxs::Identifier::Factory(strID.toStdString()));
        std::string nym_paycode = pNym->PaymentCode();
        const QString qstrPaymentCode = QString::fromStdString(nym_paycode);
        ui->toolButtonQrCode->setString(qstrPaymentCode);
        ui->lineEditDescription->setText(qstrPaymentCode);

        //qDebug() << "DEBUGGING:  SETTING PAYCODE AND QR CODE: " << qstrPaymentCode;

        QWidget * pHeaderWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, strID, strName,
                                                                           "", "", ":/icons/icons/identity_BW.png",
                                                                           false);

        pHeaderWidget->setObjectName(QString("DetailHeader")); // So the stylesheet doesn't get applied to all its sub-widgets.

        if (m_pHeaderWidget)
        {
            ui->verticalLayout->removeWidget(m_pHeaderWidget);

            m_pHeaderWidget->setParent(nullptr);
            m_pHeaderWidget->disconnect();
            m_pHeaderWidget->deleteLater();

            m_pHeaderWidget = nullptr;
        }
        ui->verticalLayout->insertWidget(0, pHeaderWidget);
        m_pHeaderWidget = pHeaderWidget;
        // ----------------------------------
        if (pLineEditNymId_)
            pLineEditNymId_->setText(strID);

        ui->lineEditName->setText(strName);

        FavorLeftSideForIDs();
        // ----------------------------------------------------------------
        clearNotaryTable();
        // ----------------------------------------------------------------
        if (pTableWidgetNotaries_)
        {
            pTableWidgetNotaries_->blockSignals(true);
            // ----------------------------
            std::string nymId   = strID.toStdString();
            const int32_t serverCount = Moneychanger::It()->OT().Exec().GetServerCount();

            for (int32_t serverIndex = 0; serverIndex < serverCount; ++serverIndex)
            {
                std::string NotaryID   = Moneychanger::It()->OT().Exec().GetServer_ID(serverIndex);
                QString qstrNotaryID   = QString::fromStdString(NotaryID);
                QString qstrNotaryName = QString::fromStdString(Moneychanger::It()->OT().Exec().GetServer_Name(NotaryID));
                bool    bStatus        = Moneychanger::It()->OT().Exec().IsNym_RegisteredAtServer(nymId, NotaryID);
                QString qstrStatus     = bStatus ? tr("Registered") : tr("Not registered");
                // ----------------------------------
                int column = 0;
                // ----------------------------------
                pTableWidgetNotaries_->insertRow(0);
                QTableWidgetItem * item = nullptr;
                // ----------------------------------
                item = new QTableWidgetItem(qstrNotaryName);
                item->setFlags(item->flags() &  ~Qt::ItemIsEditable);
                pTableWidgetNotaries_->setItem(0, column++, item);
                // ----------------------------------
                item = new QTableWidgetItem(qstrStatus);
                item->setFlags(item->flags() &  ~Qt::ItemIsEditable);
                pTableWidgetNotaries_->setItem(0, column++, item);
                // ----------------------------------
                item = new QTableWidgetItem(qstrNotaryID);
                item->setFlags(item->flags() &  ~Qt::ItemIsEditable);
                pTableWidgetNotaries_->setItem(0, column++, item);
                // ----------------------------------
                pTableWidgetNotaries_->item(0,0)->setData(Qt::UserRole, QVariant(qstrNotaryID));
            }
            if (serverCount < 1)
                pTableWidgetNotaries_->setCurrentCell(-1,0);
            else
                pTableWidgetNotaries_->setCurrentCell(0,0);
            // ----------------------------
            pTableWidgetNotaries_->blockSignals(false);
        }
        // --------------------------------------------
        // TAB: "Profile"
        //
        if (treeWidgetClaims_)
        {
            RefreshTree(strID);
        }
        // ----------------------------------
        // TAB: "Nym State"
        //
        if (m_pPlainTextEdit)
        {
            QString strContents = QString::fromStdString(Moneychanger::It()->OT().Exec().GetNym_Stats(strID.toStdString()));
            m_pPlainTextEdit->setPlainText(strContents);
        }
        // -----------------------------------
        // TAB: "CREDENTIALS"
        //
//        if (m_pCredentials)
//        {
//            QStringList qstrlistNymIDs;
//            qstrlistNymIDs.append(strID);

//            m_pCredentials->refresh(qstrlistNymIDs);
//        }
        // -----------------------------------------------------------------------
    }
}

void MTNymDetails::clearNotaryTable()
{
    if (!pTableWidgetNotaries_)
        return;

    pTableWidgetNotaries_->blockSignals(true);
    // -----------------------------------
    pTableWidgetNotaries_->clearContents();
    pTableWidgetNotaries_->setRowCount (0);
    // -----------------------------------
    pTableWidgetNotaries_->setCurrentCell(-1, 0);
    pTableWidgetNotaries_->blockSignals(false);
}

void MTNymDetails::ClearContents()
{
    ui->toolButtonQrCode->setString("");
    ui->lineEditDescription->setText("");

    if (pLineEditNymId_)
        pLineEditNymId_->setText("");
    ui->lineEditName->setText("");
    // ------------------------------------------
//    if (m_pCredentials)
//        m_pCredentials->ClearContents();
    // ------------------------------------------
    if (m_pPlainTextEdit)
        m_pPlainTextEdit->setPlainText("");
    // ------------------------------------------
    ClearTree();
    // ------------------------------------------
    clearNotaryTable();
}

// ------------------------------------------------------

void MTNymDetails::FavorLeftSideForIDs()
{
    if (nullptr != ui)
    {
        if (pLineEditNymId_)
            pLineEditNymId_->home(false);
        ui->lineEditDescription->home(false);
        ui->lineEditName->home(false);
    }
}

// ------------------------------------------------------

bool MTNymDetails::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Resize)
    {
        // This insures that the left-most part of the IDs and Names
        // remains visible during all resize events.
        //
        FavorLeftSideForIDs();
    }
//    else
//    {
        // standard event processing
//        return QWidget::eventFilter(obj, event);
        return MTEditDetails::eventFilter(obj, event);

        // NOTE: Since the base class has definitely already installed this
        // function as the event filter, I must assume that this version
        // is overriding the version in the base class.
        //
        // Therefore I call the base class version here, since as it's overridden,
        // I don't expect it will otherwise ever get called.
//    }
}


// ------------------------------------------------------


//virtual
void MTNymDetails::DeleteButtonClicked()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        // ----------------------------------------------------
        bool bCanRemove = Moneychanger::It()->OT().Exec().Wallet_CanRemoveNym(m_pOwner->m_qstrCurrentID.toStdString());

        if (!bCanRemove)
        {
            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                 tr("For your protection, Nyms already registered on a notary cannot be summarily deleted. "
                                    "Please unregister first. (You may also delete need to any accounts you may have registered "
                                    "at that same notary using the same Nym.)"));
            return;
        }
        // ----------------------------------------------------
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, tr(MONEYCHANGER_APP_NAME), tr("Are you sure you want to delete this Nym?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            bool bSuccess = Moneychanger::It()->OT().Exec().Wallet_RemoveNym(m_pOwner->m_qstrCurrentID.toStdString());

            if (bSuccess)
            {
                m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
                // ------------------------------------------------
                emit nymsChanged();
                // ------------------------------------------------
            }
            else
                QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                     tr("Failed trying to delete this Nym."));
        }
    }
}


// ------------------------------------------------------


void MTNymDetails::on_btnEditProfile_clicked()
{
    if (!pLineEditNymId_)
        return;

    const QString qstrNymId = pLineEditNymId_->text();

    if (qstrNymId.isEmpty())
        return;
    // -------------------------------------
    std::string         str_nym_id  (qstrNymId.toStdString());
    // -------------------------------------
    WizardEditProfile theWizard(this);
    theWizard.setWindowTitle(tr("Edit Profile (Public information)"));
    // -------------------------------------
    // Put the existing profile data in the wizard, so it doesn't load up blank.
    // (Not much point in editing your profile data if the data comes up blank...)
    //
    theWizard.listContactDataTuples_.clear();

    const auto claims_data =
        Moneychanger::It()->OT().Exec().GetContactData(str_nym_id);
    const auto claims =
        opentxs::proto::DataToProto<opentxs::proto::ContactData>
            (opentxs::Data::Factory(
                claims_data.c_str(),
                claims_data.length()));

    for (auto& section: claims.section()) {
        for (auto& claim: section.item()) {
            const QString claim_id = QString::fromStdString(claim.id());
            const uint32_t claim_section = section.name();
            const uint32_t claim_type = claim.type();
            const QString claim_value = QString::fromStdString(claim.value());
            const int64_t claim_start = claim.start();
            const int64_t claim_end = claim.end();

            bool claim_att_active  = false;
            bool claim_att_primary = false;

            for (const auto& attribute: claim.attribute()) {
                if (opentxs::proto::CITEMATTR_ACTIVE  == attribute) {
                    claim_att_active  = true;
                }

                if (opentxs::proto::CITEMATTR_PRIMARY == attribute) {
                    claim_att_primary = true;
                }
            }

            std::string str_value(claim_value.toStdString());
            tupleContactDataItem item
                {claim_section, claim_type, str_value, claim_att_primary};

            theWizard.listContactDataTuples_.push_front(std::move(item));
        }
    }
    // -------------------------------------
    theWizard.setOption(QWizard::IndependentPages);
    // -------------------------------------
    if (QDialog::Accepted == theWizard.exec())
    {
        // Set the updated profile data on the Nym
        //
        std::map<uint32_t, std::list<std::tuple<uint32_t, std::string, bool>>> items;

        for (const auto & contactDataItem: theWizard.listContactDataTuples_)
        {
            uint32_t     indexSection     = std::get<0>(contactDataItem);
            uint32_t     indexSectionType = std::get<1>(contactDataItem);
            std::string  textValue        = std::get<2>(contactDataItem);
            bool         bIsPrimary       = std::get<3>(contactDataItem);

            std::tuple<uint32_t, std::string, bool> item{indexSectionType, textValue, bIsPrimary};

            if (items.count(indexSection) > 0) {
                items[indexSection].push_back(item);
            } else {
                items.insert({indexSection, { item }});
            }
        }
        // ------------------------------------------------
        opentxs::proto::ContactData contactData;
        contactData.set_version(NYM_CONTACT_DATA_VERSION);

        for (auto& it: items) {
            auto newSection = contactData.add_section();
            newSection->set_version(NYM_CONTACT_DATA_VERSION);
            newSection->set_name(static_cast<opentxs::proto::ContactSectionName>(it.first));

            for (auto& i: it.second) {
                auto newItem = newSection->add_item();
                newItem->set_version(NYM_CONTACT_DATA_VERSION);
                newItem->set_type(static_cast<opentxs::proto::ContactItemType>(std::get<0>(i)));
                newItem->set_value(std::get<1>(i));
                if (std::get<2>(i)) {
                    newItem->add_attribute(opentxs::proto::CITEMATTR_PRIMARY);
                }
                newItem->add_attribute(opentxs::proto::CITEMATTR_ACTIVE);
            }
        }
        // ------------------------------------------------
        const auto armored =
            opentxs::proto::ProtoAsArmored(contactData, opentxs::String::Factory("CONTACT DATA"));
        const bool set =
            Moneychanger::It()->OT().Exec().SetContactData(str_nym_id, armored->Get());
        if (!set) {
            qDebug() << __FUNCTION__ << ": ERROR: Failed trying to Set Contact "
                     << "Data!";
        }
        // ------------------------------------------------
        // Update the local database by re-importing the claims.
        emit nymWasJustChecked(qstrNymId);
    }
}




//virtual
void MTNymDetails::AddButtonClicked()
{
    MTWizardAddNym theWizard(this);


    // Page Nym Authority can go. Default authority index=0

    // Change the Alt Location page to do the claims.


    theWizard.setWindowTitle(tr("Create Nym (a.k.a. Create Identity)"));

    if (QDialog::Accepted == theWizard.exec())
    {
        QString qstrName        = theWizard.field("Name")     .toString();
        int     nAuthorityIndex = theWizard.field("Authority").toInt();
        int     nAlgorithmIndex = theWizard.field("Algorithm").toInt();
        QString qstrSource      = theWizard.field("Source")   .toString();
        // ---------------------------------------------------
        // NOTE: theWizard won't allow each page to finish unless the data is provided.
        // (Therefore we don't have to check here to see if any of the data is empty.)

        // -------------------------------------------
        std::string NYM_ID_SOURCE("");

        if (0 != nAuthorityIndex) // Zero would be Self-Signed, which needs no source.
            NYM_ID_SOURCE = qstrSource.toStdString();
        // -------------------------------------------
        // Create Nym here...
        //

        std::string    str_id;

        switch (nAlgorithmIndex)
        {
            case 0:  // ECDSA
                str_id = Moneychanger::It()->OT().Exec().CreateNymHD(opentxs::proto::CITEMTYPE_INDIVIDUAL, qstrName.toStdString(), NYM_ID_SOURCE, -1);
                break;
            case 1: // 1024-bit RSA
                str_id = Moneychanger::It()->OT().Exec().CreateNymLegacy(1024, NYM_ID_SOURCE);
                break;
//            case 2: // 2048-bit RSA
//                str_id = Moneychanger::It()->OT().OTME().create_nym_legacy(2048, NYM_ID_SOURCE);
//                break;
//            case 3: // 4096-bit RSA
//                str_id = Moneychanger::It()->OT().OTME().create_nym_legacy(4096, NYM_ID_SOURCE);
//                break;
//            case 4: // 8192-bit RSA
//                str_id = Moneychanger::It()->OT().OTME().create_nym_legacy(8192, NYM_ID_SOURCE);
//                break;
            default:
                QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                    tr("Unexpected key type."));
                return;
        }
        // --------------------------------------------------
        if (str_id.empty())
        {
            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                tr("Failed trying to create Nym."));
            return;
        }
        // ------------------------------------------------------
        // Get the ID of the new nym.
        //
        QString qstrID = QString::fromStdString(str_id);
        // ------------------------------------------------------
        // Set the Name of the new Nym.
        //
        //bool bNameSet =
        //Moneychanger::It()->OT().Exec().SetNym_Name(qstrID.toStdString(), qstrID.toStdString(), qstrName.toStdString());
        // ------------------------------------------------------
        std::map<uint32_t, std::list<std::tuple<uint32_t, std::string, bool>>> items;

        for (const auto & contactDataItem: theWizard.listContactDataTuples_)
        {
            uint32_t     indexSection     = std::get<0>(contactDataItem);
            uint32_t     indexSectionType = std::get<1>(contactDataItem);
            std::string  textValue        = std::get<2>(contactDataItem);
            bool         bIsPrimary       = std::get<3>(contactDataItem);

            std::tuple<uint32_t, std::string, bool> item{indexSectionType, textValue, bIsPrimary};

            if (items.count(indexSection) > 0) {
                items[indexSection].push_back(item);
            } else {
                items.insert({indexSection, { item }});
            }
        }

        opentxs::proto::ContactData contactData;
        contactData.set_version(NYM_CONTACT_DATA_VERSION);

        for (auto& it: items) {
            auto newSection = contactData.add_section();
            newSection->set_version(NYM_CONTACT_DATA_VERSION);
            newSection->set_name(static_cast<opentxs::proto::ContactSectionName>(it.first));

            for (auto& i: it.second) {
                auto newItem = newSection->add_item();
                newItem->set_version(NYM_CONTACT_DATA_VERSION);
                newItem->set_type(static_cast<opentxs::proto::ContactItemType>(std::get<0>(i)));
                newItem->set_value(std::get<1>(i));
                if (std::get<2>(i)) {
                    newItem->add_attribute(opentxs::proto::CITEMATTR_PRIMARY);
                }
                newItem->add_attribute(opentxs::proto::CITEMATTR_ACTIVE);
            }
        }

        auto armored =
            opentxs::proto::ProtoAsArmored(contactData, opentxs::String::Factory("CONTACT DATA"));

        if (!Moneychanger::It()->OT().Exec().SetContactData(str_id, armored->Get())) {
            qDebug() << __FUNCTION__ << ": ERROR: Failed trying to Set Contact "
                     << "Data!";
        } else {
            qDebug() << __FUNCTION__ << "SetContactData SUCCESS. items.size(): "
                     << items.size();
        }

        m_pOwner->m_map.insert(qstrID, qstrName);
        m_pOwner->SetPreSelected(qstrID);
        // ------------------------------------------------
        emit newNymAdded(qstrID);
        // -----------------------------------------------
//      QMessageBox::information(this, tr("Success!"), QString("%1: '%2' %3: %4").arg(tr("Success Creating Nym! Name")).
//                               arg(qstrName).arg(tr("ID")).arg(qstrID));
        // ----------
    }
}

// ------------------------------------------------------


void MTNymDetails::on_treeWidget_customContextMenuRequested(const QPoint &pos)
{
//    if (!m_pOwner->m_qstrCurrentID.isEmpty())
//    {
//        QString qstrNymID(m_pOwner->m_qstrCurrentID);
//        QString qstrNymName(m_pOwner->m_qstrCurrentName);
//        const std::string str_nym_id = qstrNymID.toStdString();
//        // ----------------------------------------------------
//        QTreeWidgetItem * pItem = treeWidgetClaims_->itemAt(pos);

//        if (nullptr != pItem)
//        {
////          const int nRow = pItem->row();
////          if (nRow >= 0)
//            {
//                //#define TREE_ITEM_TYPE_RELATIONSHIP    10
//                //#define TREE_ITEM_TYPE_CLAIM           11
//                //#define TREE_ITEM_TYPE_VERIFICATION    12

//                QVariant qvarTreeItemType = pItem->data(0, Qt::UserRole+1);
//                const int nTreeItemType = qvarTreeItemType.isValid() ? qvarTreeItemType.toInt() : 0;

//                const bool bIsRelationship = (TREE_ITEM_TYPE_RELATIONSHIP == nTreeItemType);
//                const bool bIsClaim        = (TREE_ITEM_TYPE_CLAIM        == nTreeItemType);
//                const bool bIsVerification = (TREE_ITEM_TYPE_VERIFICATION == nTreeItemType);
//                // -----------------------------------------------------------------------------
//                // Relationships, here, are claims ABOUT my Nym -- this Nym -- ("ME").
//                // They are claims that were made by OTHERS. (But since my wallet contains multiple
//                // Nyms, "others" may include MY Nyms in this wallet, in addition to outside Nyms
//                // controlled by other users.)
//                //
//                // Thus for some of these, these records are read-only. Which is to say, a Nym can only
//                // view the claims made about him by others, in a read-only fashion. He cannot delete
//                // other people's claims. BUT! If the claim was made by ANOTHER Nym in the SAME WALLET,
//                // then THAT Nym DOES have the right to delete this claim. (Though normally that sort
//                // of thing is done in the Contact details tab, but we could make it available here as
//                // well.)
//                //
//                // In either case and in any case, I ("Me" Nym) am here viewing relationship claims made
//                // about me by "others", and I have the power to confirm or refute them by creating a
//                // verification for that specific relationships claim.
//                //
//                // So I may sign someone's relationship claim, thus confirming it (or refuting it.) So
//                // each of these relationship claims may also display its "confirmation status" whenever
//                // it's available. And I (the current Nym) have the power to Confirm/Refute/NoComment
//                // and thus change the confirmation status of any claim in this block.
//                //
//                if (bIsRelationship)
//                {
//                    QVariant qvarClaimId       = pItem->data(1, Qt::UserRole);
//                    QVariant qvarClaimantNymId = pItem->data(0, Qt::UserRole);
//                    QVariant qvarPolarity      = pItem->data(5, Qt::UserRole);

//                    QString qstrClaimId       = qvarClaimId      .isValid() ? qvarClaimId      .toString() : "";
//                    QString qstrClaimantNymId = qvarClaimantNymId.isValid() ? qvarClaimantNymId.toString() : "";

//                    QString & qstrVerifierNymId = qstrNymID;

//                    if (qstrClaimId.isEmpty() || qstrClaimantNymId.isEmpty())
//                        return;

//                    opentxs::ClaimPolarity claimPolarity = qvarPolarity.isValid() ?
//                                intToClaimPolarity(static_cast<int>(qvarPolarity.toUInt())) :
//                                opentxs::ClaimPolarity::NEUTRAL;
//                    // ----------------------------------
//                    pActionConfirm_ = nullptr;
//                    pActionRefute_ = nullptr;
//                    pActionNoComment_ = nullptr;

//                    popupMenuProfile_.reset(new QMenu(this));

//                    if (claimPolarity    != opentxs::ClaimPolarity::POSITIVE)
//                        pActionConfirm_   = popupMenuProfile_->addAction(tr("Confirm"));
//                    if (claimPolarity    != opentxs::ClaimPolarity::NEGATIVE)
//                        pActionRefute_    = popupMenuProfile_->addAction(tr("Refute"));
//                    if (claimPolarity    != opentxs::ClaimPolarity::NEUTRAL)
//                        pActionNoComment_ = popupMenuProfile_->addAction(tr("No comment"));
//                    // ------------------------
//                    QPoint globalPos = treeWidgetClaims_->mapToGlobal(pos);
//                    // ------------------------
//                    const QAction* selectedAction = popupMenuProfile_->exec(globalPos); // Here we popup the menu, and get the user's click.
//                    // ------------------------
//                    if (nullptr == selectedAction)
//                    {

//                    }
//                    else if (selectedAction == pActionConfirm_)
//                    {
//                        // If true, that means OT had to CHANGE something in the Nym's data.
//                        // (So we'll need to broadcast that, so Moneychanger can re-import the Nym.)
//                        //
//                        if (MTContactHandler::getInstance()->claimVerificationConfirm(qstrClaimId, qstrClaimantNymId, qstrVerifierNymId))
//                        {
//                            emit nymWasJustChecked(qstrVerifierNymId);
//                            return;
//                        }
//                    }
//                    // ------------------------
//                    else if (selectedAction == pActionRefute_)
//                    {
//                        if (MTContactHandler::getInstance()->claimVerificationRefute(qstrClaimId, qstrClaimantNymId, qstrVerifierNymId))
//                        {
//                            emit nymWasJustChecked(qstrVerifierNymId);
//                            return;
//                        }
//                    }
//                    // ------------------------
//                    else if (selectedAction == pActionNoComment_)
//                    {
//                        if (MTContactHandler::getInstance()->claimVerificationNoComment(qstrClaimId, qstrClaimantNymId, qstrVerifierNymId))
//                        {
//                            emit nymWasJustChecked(qstrVerifierNymId);
//                            return;
//                        }
//                    }
//                } // if is relationship.
//                // -----------------------------------------------
//                else if (bIsClaim)
//                {
//                    // It's a normal claim (made by me, versus relationships above, made by others ABOUT me.)
//                    //
//                    // NOTE: To edit your own claims, just click the "edit profile" button.)

//                } // if is claim.
//                // -----------------------------------------------
//                else if (bIsVerification) // verification
//                {
//                    // It's a verification of a claim. *I* make my own claims, and OTHER people
//                    // create verifications for those claims. So normally I CAN'T edit other people's
//                    // verifications.
//                    //
//                    // HOWEVER, there are multiple Nyms in this wallet. So some of these verifications,
//                    // while they are not created by the "Me" Nym currently displayed in the Nym Details,
//                    // nevertheless, some of them may have been created by Nyms under the control of this
//                    // wallet -- Nyms which have the same power to edit the same verification now!
//                    // So I have the power to edit SOME verifications, if they were created by other Nyms
//                    // in the wallet. Otherwise, if they were created by other Nyms outside of the wallet,
//                    // obviously I do NOT have the power to edit THOSE verifications.


//                    // For now we won't put any verification editing code here, since normally only "others"
//                    // can do this. (If you want to do this now, just edit the verification from the contact
//                    // details page, instead of the Nym details page.)
//                    //
//                    // The more advanced functionality described in the larger above comment can come later.


//                } // if is verification
//                // -----------------------------------------------
//                else
//                {
//                    // This space intentionally left blank.
//                }
//                // ------------------------
//            } //nRow >= 0
//        }
//    }
}

void MTNymDetails::on_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        QString qstrNymID(m_pOwner->m_qstrCurrentID);
        QString qstrNymName(m_pOwner->m_qstrCurrentName);
        const std::string str_nym_id = qstrNymID.toStdString();
        // ----------------------------------------------------
        QTableWidgetItem * pItem = pTableWidgetNotaries_->itemAt(pos);

        if (nullptr != pItem)
        {
            int nRow = pItem->row();

            if (nRow >= 0)
            {
                QString qstrNotaryID = pTableWidgetNotaries_->item(nRow, 0)->data(Qt::UserRole).toString();
                std::string str_notary_id = qstrNotaryID.toStdString();
                QString qstrNotaryName = QString::fromStdString(Moneychanger::It()->OT().Exec().GetServer_Name(str_notary_id));
                // ------------------------
                QPoint globalPos = pTableWidgetNotaries_->mapToGlobal(pos);
                // ------------------------
                const QAction* selectedAction = popupMenu_->exec(globalPos); // Here we popup the menu, and get the user's click.
                // ------------------------
                if (selectedAction == pActionRegister_)
                {
                    if (Moneychanger::It()->OT().Exec().IsNym_RegisteredAtServer(str_nym_id, str_notary_id))
                    {
                        QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), QString("%1 '%2' %3 '%4'.").arg(tr("The Nym")).
                                                 arg(qstrNymName).arg(tr("is already registered on notary")).arg(qstrNotaryName));
                        return;
                    }
                    else
                    {


                        int32_t nSuccess = 0;
                        bool    bRegistered = false;
                        {
                            MTSpinner theSpinner;

                            auto strResponse = Moneychanger::It()->OT().Sync().RegisterNym(opentxs::Identifier::Factory(str_nym_id),
                                                                                           opentxs::Identifier::Factory(str_notary_id), true);

                            if (false == strResponse->empty()) {
                                nSuccess = 1;
                            } else {
                                nSuccess = 0;
                            }
                        }
                        // -1 is error,
                        //  0 is reply received: failure
                        //  1 is reply received: success
                        //
                        switch (nSuccess)
                        {
                        case (1):
                            {
                                bRegistered = true;

                                MTContactHandler::getInstance()->NotifyOfNymServerPair(QString::fromStdString(str_nym_id),
                                                                                       QString::fromStdString(str_notary_id));

                                QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME),
                                    tr("Success!"));
                                // ------------------------------------------------
                                emit nymsChanged();
                                // ------------------------------------------------
                                break; // SUCCESS
                            }
                        case (0):
                            {
                                QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                    tr("Failed while trying to register nym on Server."));
                                break;
                            }
                        default:
                            {
                                QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                    tr("Error while trying to register nym on Server."));
                                break;
                            }
                        } // switch
                        // --------------------------
                        if (1 != nSuccess)
                        {
                            Moneychanger::It()->HasUsageCredits(QString::fromStdString(str_notary_id), QString::fromStdString(str_nym_id));
                            return;
                        }
                    }
                }
                // ------------------------
                else if (selectedAction == pActionUnregister_)
                {
                    if (!Moneychanger::It()->OT().Exec().IsNym_RegisteredAtServer(str_nym_id, str_notary_id))
                    {
                        QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), QString("%1 '%2' %3 '%4'.").arg(tr("The Nym")).
                                                 arg(qstrNymName).arg(tr("is already not registered on notary")).arg(qstrNotaryName));
                        return;
                    }
                    else
                    {
                        // See if Nym even CAN unregister.
                        // (Not if he has any accounts there -- he must delete those accounts first.)
                        //
                        mapIDName accountMap;
                        if (MTContactHandler::getInstance()->GetAccounts(accountMap, qstrNymID, qstrNotaryID, QString("")))
                        {
                            QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), QString("%1 '%2' %3 '%4'. %5.").arg(tr("The Nym")).
                                                     arg(qstrNymName).arg(tr("still has asset accounts on the notary")).arg(qstrNotaryName).arg(tr("Please delete those first")));
                            return;
                        }
                        // -----------------------------------
                        QMessageBox::StandardButton reply;
                        reply = QMessageBox::question(this, tr(MONEYCHANGER_APP_NAME),
                                                      tr("Are you sure you want to unregister your Nym from this notary?"),
                                                      QMessageBox::Yes|QMessageBox::No);
                        if (reply == QMessageBox::Yes)
                        {


                            int32_t nSuccess = 0;
                            bool    bUnregistered = false;
                            {
                                MTSpinner theSpinner;

                                auto action = Moneychanger::It()->OT().ServerAction().UnregisterNym(
                                        opentxs::Identifier::Factory(str_nym_id), opentxs::Identifier::Factory(str_notary_id));
                                std::string strResponse = action->Run();
                                nSuccess                = opentxs::VerifyMessageSuccess(Moneychanger::It()->OT(), strResponse);
                            }
                            // -1 is error,
                            //  0 is reply received: failure
                            //  1 is reply received: success
                            //
                            switch (nSuccess)
                            {
                            case (1):
                                {
                                    bUnregistered = true;

                                    MTContactHandler::getInstance()->NotifyOfNymServerUnpair(QString::fromStdString(str_nym_id),
                                                                                             QString::fromStdString(str_notary_id));

                                    QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME),
                                        tr("Success!"));
                                    // ------------------------------------------------
                                    emit nymsChanged();
                                    // ------------------------------------------------
                                    break; // SUCCESS
                                }
                            case (0):
                                {
                                    QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                        tr("Failed while trying to unregister nym from Server."));
                                    break;
                                }
                            default:
                                {
                                    QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                        tr("Error while trying to unregister nym from Server."));
                                    break;
                                }
                            } // switch
                            // --------------------------
                            if (1 != nSuccess)
                            {
                                Moneychanger::It()->HasUsageCredits(QString::fromStdString(str_notary_id), QString::fromStdString(str_nym_id));
                                return;
                            }
                        }
                    }
                }
            } // nRow >= 0
        } // pItem not NULL.
    }
}

// ------------------------------------------------------

void MTNymDetails::on_lineEditName_editingFinished()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        bool bSuccess = Moneychanger::It()->OT().Exec().Rename_Nym(m_pOwner->m_qstrCurrentID.toStdString(), // Nym
                                                ui->lineEditName->text(). toStdString()); // New Name
        if (bSuccess)
        {
            m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
            m_pOwner->m_map.insert(m_pOwner->m_qstrCurrentID, ui->lineEditName->text());

            m_pOwner->SetPreSelected(m_pOwner->m_qstrCurrentID);
            // ------------------------------------------------
            emit nymsChanged();
            // ------------------------------------------------
        }
    }
}

// ------------------------------------------------------









