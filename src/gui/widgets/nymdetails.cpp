#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include "gui/widgets/nymdetails.hpp"
#include <ui_nymdetails.h>

#include <gui/widgets/credentials.hpp>
#include <gui/widgets/wizardaddnym.hpp>
#include <gui/widgets/wizardeditprofile.hpp>
#include <gui/widgets/overridecursor.hpp>

#include <gui/widgets/qrtoolbutton.hpp>

#include <core/handlers/DBHandler.hpp>
#include <core/handlers/contacthandler.hpp>
#include <core/handlers/modelclaims.hpp>
#include <core/handlers/modelverifications.hpp>
#include <core/mtcomms.h>
#include <core/moneychanger.hpp>

#include <opentxs/api/OT.hpp>
#include <opentxs/core/Nym.hpp>
#include <opentxs/client/OTAPI_Wrap.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>
#include <opentxs/client/OT_API.hpp>
#include <opentxs/client/OT_ME.hpp>
#include <opentxs/core/NumList.hpp>
#include <opentxs/core/Proto.hpp>
#include <opentxs/core/Types.hpp>

#include <namecoin/Namecoin.hpp>

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QStringList>
#include <QMessageBox>
#include <QStringList>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QPlainTextEdit>
#include <QClipboard>
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

#include <map>
#include <tuple>
#include <string>



#define TREE_ITEM_TYPE_RELATIONSHIP    10
#define TREE_ITEM_TYPE_CLAIM           11
#define TREE_ITEM_TYPE_VERIFICATION    12



void MTNymDetails::ClearTree()
{
    metInPerson_ = nullptr;

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
    default:                                       nReturnValue = 2; break;
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
    opentxs::String     strNymId    (str_nym_id);
    opentxs::Identifier id_nym      (strNymId);
    // -------------------------------------
    opentxs::Nym * pCurrentNym = opentxs::OTAPI_Wrap::OTAPI()->GetOrLoadPrivateNym(id_nym) ;

    if (nullptr == pCurrentNym)
        return;


//    qDebug() << "DEBUGGING: onClaimsUpdatedForNym 3 ";

    const int32_t server_count = opentxs::OTAPI_Wrap::Exec()->GetServerCount();
    // -----------------------------------------------
    // Loop through all the servers and for each, see if the Nym  is registered
    // there. For every server that he IS registered on, RE-register so it has
    // the latest copy of the credentials.
    //
    for (int32_t ii = 0; ii < server_count; ++ii)
    {
        QString notary_id = QString::fromStdString(opentxs::OTAPI_Wrap::Exec()->GetServer_ID(ii));
        // -----------------------------------------------
        if (!notary_id.isEmpty())
        {
            const bool isReg = opentxs::OTAPI_Wrap::Exec()->IsNym_RegisteredAtServer(str_nym_id, notary_id.toStdString());

            if (isReg) // We only RE-REGISTER at servers where we're ALREADY registered.
            {          // (To update their copy of the credentials we just edited.)
                std::string response;
                {
                    MTSpinner theSpinner;

                    response = opentxs::OTAPI_Wrap::Register_Nym_Public(str_nym_id, notary_id.toStdString());
                    if (response.empty() && !opentxs::OTAPI_Wrap::CheckConnection(notary_id.toStdString()))
                    {
                        QString qstrErrorMsg;
                        qstrErrorMsg = QString("%1: %2. %3.").
                                arg(tr("Failed trying to contact notary")).
                                arg(notary_id).arg(tr("Perhaps it is down, or there might be a network problem"));
                        emit appendToLog(qstrErrorMsg);
                        continue;
                    }
                }

                if (!opentxs::OT_ME::It().VerifyMessageSuccess(response)) {
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
    if (!treeWidgetClaims_ || (NULL == ui))
        return;
    // ----------------------------------------
    ClearTree();
    // ----------------------------------------
    if ( qstrNymId.isEmpty() )
        return;
    // ----------------------------------------
    treeWidgetClaims_->blockSignals(true);
    // ----------------------------------------
    // Before commencing with the main act, let's iterate all the Nyms once,
    // and construct a map, so we don't end up doing this multiple times below
    // unnecessarily.
    //
    typedef std::pair<std::string, opentxs::ClaimSet> NymClaims;
    typedef std::map <std::string, opentxs::ClaimSet> mapOfNymClaims;
    typedef std::map <std::string, std::string> mapOfNymNames;

//  mapOfNymClaims nym_claims; // Each pair in this map has a NymID and a ClaimSet.
    mapOfNymNames  nym_names;  // Each pair in this map has a NymID and a Nym Name.
    // ---------------------------------------
    MTNameLookupQT theLookup;
    const std::string str_nym_id   = qstrNymId.toStdString();
    const std::string str_nym_name = theLookup.GetNymName(qstrNymId.toStdString(), "");
    const opentxs::Identifier id_nym(str_nym_id);

    if (!str_nym_id.empty())
    {
        auto pCurrentNym = opentxs::OT::App().Contract().Nym(id_nym);
//        const opentxs::Nym * pCurrentNym =
//            opentxs::OTAPI_Wrap::OTAPI()->GetOrLoadNym(id_nym);

        if (pCurrentNym)
        {
//          opentxs::ClaimSet claims = opentxs::OTAPI_Wrap::OTAPI()->GetClaims(*pCurrentNym);
            // ---------------------------------------
//          nym_claims.insert( NymClaims(str_nym_id, claims) );
            nym_names.insert(std::pair<std::string, std::string>(str_nym_id, str_nym_name));
        }
    }

    // -------------------------------------------------
    // This means NONE of the contact's Nyms had any claims.
    // (So there's nothing to put on this tree. Done.)
//    if (nym_claims.empty())
//        return;
    // UPDATE: Even if Bob has no claims, ALICE might have made a relationship claim ABOUT
    // Bob, which would be displayed here (giving Bob the opportunity to confirm/refute.)
    // (So we can't just return here.)
    // -------------------------------------------------
//  QPointer<ModelClaims>      pModelClaims_;
//  QPointer<ClaimsProxyModel> pProxyModelClaims_;

    pProxyModelClaims_ = nullptr;
    pModelClaims_ = DBHandler::getInstance()->getClaimsModel(qstrNymId);

    if (!pModelClaims_)
        return;

    pProxyModelClaims_ = new ClaimsProxyModel;
    pProxyModelClaims_->setSourceModel(pModelClaims_);
    // -------------------------------------------------
    // We make a button group for each combination of Nym, Section, and Type.
    // Therefore I make a map with a KEY of: tuple<Nym, Section, Type>
    // The VALUE will be pointers to QButtonGroup.
    //
//    const std::string claim_nym_id  = qvarNymId  .isValid() ? qvarNymId.toString().toStdString() : "";
//    const uint32_t    claim_section = qvarSection.isValid() ? qvarSection.toUInt() : 0;
//    const uint32_t    claim_type    = qvarType   .isValid() ? qvarType.toUInt() : 0;

    typedef std::tuple<std::string, uint32_t, uint32_t> ButtonGroupKey;
    typedef std::map<ButtonGroupKey, QButtonGroup *> mapOfButtonGroups;

    mapOfButtonGroups mapButtonGroups;
    // -------------------------------------------------

    // Insert "Has Met" into Tree.
    //
    QString  qstrMetLabel = QString("<b>%1</b>").arg(tr("Relationship claims"));
    QLabel * label = new QLabel(qstrMetLabel, treeWidgetClaims_);

    metInPerson_ = new QTreeWidgetItem;

//  metInPerson_->setText(0, "Met in person");
    treeWidgetClaims_->addTopLevelItem(metInPerson_);
    treeWidgetClaims_->expandItem(metInPerson_);
    treeWidgetClaims_->setItemWidget(metInPerson_, 0, label);
    // ------------------------------------------
    // Earlier above, we made a list of all the claims from other Nyms
    // that they "have met" the Nyms represented by this Contact.
    // Now let's add those here as sub-items under the metInPerson_ top-level item.
    //
    QPointer<ModelClaims> pRelationships = DBHandler::getInstance()->getRelationshipClaims(qstrNymId);

    if (!pRelationships) // Should never happen, even if the result set is empty.
        return;

    QPointer<ClaimsProxyModel> pProxyModelRelationships = new ClaimsProxyModel;

    pProxyModelRelationships->setSourceModel(pRelationships);

    if (pRelationships->rowCount() > 0)
    {
        // First grab the various relationship type names:  (Parent of, have met, child of, etc.)
        QMap<uint32_t, QString> mapTypeNames;
        // ----------------------------------------
        const std::string sectionName =
            opentxs::OTAPI_Wrap::Exec()->ContactSectionName(
                opentxs::proto::CONTACTSECTION_RELATIONSHIP);
        const auto sectionTypes =
            opentxs::OTAPI_Wrap::Exec()->ContactSectionTypeList(
                opentxs::proto::CONTACTSECTION_RELATIONSHIP);

        for (const auto& indexSectionType: sectionTypes) {
            const std::string typeName =
                opentxs::OTAPI_Wrap::Exec()->ContactTypeName(indexSectionType);
            mapTypeNames.insert(
                indexSectionType,
                QString::fromStdString(typeName));
        }
        // ---------------------------------------

        for (int nRelationshipCount = 0; nRelationshipCount < pProxyModelRelationships->rowCount(); nRelationshipCount++)
        {
//            QSqlRecord record = pRelationships->record(nRelationshipCount);

            QModelIndex proxyIndexZero        = pProxyModelRelationships->index(nRelationshipCount, 0);
            QModelIndex sourceIndexZero       = pProxyModelRelationships->mapToSource(proxyIndexZero);
            // ----------------------------------------------------------------------------
            QModelIndex sourceIndexClaimId    = pRelationships->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_CLAIM_ID,    sourceIndexZero);
            QModelIndex sourceIndexNymId      = pRelationships->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_NYM_ID,      sourceIndexZero);
            QModelIndex sourceIndexSection    = pRelationships->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_SECTION,     sourceIndexZero);
            QModelIndex sourceIndexType       = pRelationships->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_TYPE,        sourceIndexZero);
            QModelIndex sourceIndexValue      = pRelationships->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_VALUE,       sourceIndexZero);
            QModelIndex sourceIndexStart      = pRelationships->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_START,       sourceIndexZero);
            QModelIndex sourceIndexEnd        = pRelationships->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_END,         sourceIndexZero);
            QModelIndex sourceIndexAttributes = pRelationships->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_ATTRIBUTES,  sourceIndexZero);
            QModelIndex sourceIndexAttActive  = pRelationships->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_ATT_ACTIVE,  sourceIndexZero);
            QModelIndex sourceIndexAttPrimary = pRelationships->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_ATT_PRIMARY, sourceIndexZero);
            // ----------------------------------------------------------------------------
            QModelIndex proxyIndexValue       = pProxyModelRelationships->mapFromSource(sourceIndexValue);
            QModelIndex proxyIndexStart       = pProxyModelRelationships->mapFromSource(sourceIndexStart);
            QModelIndex proxyIndexEnd         = pProxyModelRelationships->mapFromSource(sourceIndexEnd);
            // ----------------------------------------------------------------------------
            QVariant    qvarClaimId           = pRelationships->data(sourceIndexClaimId);
            QVariant    qvarNymId             = pRelationships->data(sourceIndexNymId);
            QVariant    qvarSection           = pRelationships->data(sourceIndexSection);
            QVariant    qvarType              = pRelationships->data(sourceIndexType);
            QVariant    qvarValue             = pProxyModelRelationships->data(proxyIndexValue); // Proxy here since the proxy model decodes this. UPDATE: no longer encoded.
            QVariant    qvarStart             = pProxyModelRelationships->data(proxyIndexStart); // Proxy for these two since it formats the
            QVariant    qvarEnd               = pProxyModelRelationships->data(proxyIndexEnd);   // timestamp as a human-readable string.
            QVariant    qvarAttributes        = pRelationships->data(sourceIndexAttributes);
            QVariant    qvarAttActive         = pRelationships->data(sourceIndexAttActive);
            QVariant    qvarAttPrimary        = pRelationships->data(sourceIndexAttPrimary);
            // ----------------------------------------------------------------------------
            const QString qstrClaimId       = qvarClaimId.isValid() ? qvarClaimId.toString() : "";
            const QString qstrClaimValue    = qvarValue  .isValid() ? qvarValue  .toString() : "";
            // ----------------------------------------------------------------------------
            const std::string claim_id      = qstrClaimId   .isEmpty() ? "" : qstrClaimId.toStdString();
            const std::string claim_value   = qstrClaimValue.isEmpty() ? "" : qstrClaimValue.toStdString();
            const std::string claim_nym_id  = qvarNymId     .isValid() ? qvarNymId.toString().toStdString() : "";
            const uint32_t    claim_section = qvarSection   .isValid() ? qvarSection.toUInt() : 0;
            const uint32_t    claim_type    = qvarType      .isValid() ? qvarType.toUInt() : 0;
            // ----------------------------------------------------------------------------
            const bool        claim_active  = qvarAttActive .isValid() ? qvarAttActive .toBool() : false;
            const bool        claim_primary = qvarAttPrimary.isValid() ? qvarAttPrimary.toBool() : false;
            // ----------------------------------------------------------------------------
            QMap<uint32_t, QString>::iterator it_typeNames = mapTypeNames.find(claim_type);
            QString qstrTypeName;

            if (it_typeNames != mapTypeNames.end())
                qstrTypeName = it_typeNames.value();
            // ---------------------------------------
            MTNameLookupQT theLookup;
            const std::string str_claimant_name = theLookup.GetNymName(claim_nym_id, "");

            // Add the claim to the tree.
            //
            QTreeWidgetItem * claim_item = new QTreeWidgetItem;
            // ---------------------------------------

            // ALICE claims she HAS MET *CHARLIE*.

            // str_claimant_name claims she qstrTypeName nym_names[claim_value]

            mapOfNymNames::iterator it_names = nym_names.find(claim_value);
            std::string str_nym_name;

            if (nym_names.end() != it_names)
                str_nym_name =  it_names->second;
            else
                str_nym_name = claim_value;

            const QString qstrClaimantLabel = QString("%1: %2").arg(tr("Claimant")).arg(QString::fromStdString(str_claimant_name));

            claim_item->setText(0, qstrClaimantLabel);      // "Alice" (some lady) from NymId
            claim_item->setText(1, qstrTypeName);           // "Has met" (or so she claimed)
            claim_item->setText(2, QString::fromStdString(str_nym_name)); // "Charlie" (the current Nym whose details we're viewing.)

            claim_item->setData(0, Qt::UserRole+1, TREE_ITEM_TYPE_RELATIONSHIP);

            claim_item->setData(0, Qt::UserRole, QString::fromStdString(claim_nym_id)); // Alice's Nym Id. The person who made the claim. Claimant Nym Id.
            claim_item->setData(1, Qt::UserRole, qstrClaimId);
            claim_item->setData(2, Qt::UserRole, qstrClaimValue); // Verifier Nym Id. Alice made a claim about Charlie (me), who verifies her claim. So Charlie's ID goes in Alice's claim_value.
            // ----------------------------------------
            // Since this is someone else's claim about me, I should see if I have already confirmed or refuted it.
            bool bPolarity = false;
            const bool bGotPolarity = MTContactHandler::getInstance()->getPolarityIfAny(qstrClaimId,
                                                                                        QString::fromStdString(str_nym_id), bPolarity);

//            qDebug() << "DEBUGGING NYM DETAILS: QString::fromStdString(claim_value): " << QString::fromStdString(claim_value);
//            qDebug() << "DEBUGGING NYM DETAILS: QString::fromStdString(str_nym_id): " << QString::fromStdString(str_nym_id);

            if (bGotPolarity)
            {
                claim_item->setText(5, bPolarity ? tr("Confirmed") : tr("Refuted") );
                claim_item->setBackgroundColor(5, bPolarity ? QColor("green") : QColor("red"));
            }
            else
                claim_item->setText(5, tr("No comment"));

            opentxs::ClaimPolarity claimPolarity = (!bGotPolarity ? opentxs::ClaimPolarity::NEUTRAL :
                (bPolarity ? opentxs::ClaimPolarity::POSITIVE : opentxs::ClaimPolarity::NEGATIVE));

            claim_item->setData(5, Qt::UserRole, QVariant::fromValue(claimPolarityToInt(claimPolarity))); // Confirmed or refuted. (Or none.) Polarity stored here.
            // ---------------------------------------
//          claim_item->setFlags(claim_item->flags() |     Qt::ItemIsEditable);
            claim_item->setFlags(claim_item->flags() & ~ ( Qt::ItemIsEditable | Qt::ItemIsUserCheckable) );
            // ---------------------------------------
            metInPerson_->addChild(claim_item);
            treeWidgetClaims_->expandItem(claim_item);
            // ----------------------------------------------------------------------------
            // Couldn't do this until now, when the claim_item has been added to the tree.
            //
//          typedef std::tuple<std::string, uint32_t, uint32_t> ButtonGroupKey;
//          typedef std::map<ButtonGroupKey, QButtonGroup *> mapOfButtonGroups;
//          mapOfButtonGroups mapButtonGroups;

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
//            { // "Primary"
//            QRadioButton * pRadioBtn = new QRadioButton(treeWidgetClaims_);
//            pButtonGroup->addButton(pRadioBtn);
//            pRadioBtn->setChecked(claim_primary);
//            pRadioBtn->setEnabled(false);
//            // ---------
//            treeWidgetClaims_->setItemWidget(claim_item, 3, pRadioBtn);
//            }
//            // ----------------------------------------------------------------------------
//            { // "Active"
//            QRadioButton * pRadioBtn = new QRadioButton(treeWidgetClaims_);
//            pRadioBtn->setChecked(claim_active);
//            pRadioBtn->setEnabled(false);
//            // ---------
//            treeWidgetClaims_->setItemWidget(claim_item, 4, pRadioBtn);
//            }

         } // for (relationships)
    }

    // Now we loop through the sections, and for each, we populate its
    // itemwidgets by looping through the nym_claims we got above.
    const auto sections = opentxs::OTAPI_Wrap::Exec()->ContactSectionList();

    for (const auto& indexSection: sections) {
        if (opentxs::proto::CONTACTSECTION_RELATIONSHIP == indexSection)
            continue;
        // ----------------------------------------
        QMap<uint32_t, QString> mapTypeNames;

        std::string sectionName =
            opentxs::OTAPI_Wrap::Exec()->ContactSectionName(indexSection);
        const auto sectionTypes =
            opentxs::OTAPI_Wrap::Exec()->ContactSectionTypeList(indexSection);

        for (auto& indexSectionType: sectionTypes) {
            const std::string typeName =
                opentxs::OTAPI_Wrap::Exec()->ContactTypeName(indexSectionType);
            mapTypeNames.insert(
                indexSectionType,
                QString::fromStdString(typeName));
        }
        // ---------------------------------------
        // Insert Section into Tree.
        //
        QTreeWidgetItem * topLevel = new QTreeWidgetItem;
        // ------------------------------------------
        topLevel->setText(0, QString::fromStdString(sectionName));
        // ------------------------------------------
        treeWidgetClaims_->addTopLevelItem(topLevel);
        treeWidgetClaims_->expandItem(topLevel);
        // ------------------------------------------
        for (int ii = 0; ii < pProxyModelClaims_->rowCount(); ++ii)
        {
            QModelIndex proxyIndexZero        = pProxyModelClaims_->index(ii, 0);
            QModelIndex sourceIndexZero       = pProxyModelClaims_->mapToSource(proxyIndexZero);
            // ----------------------------------------------------------------------------
            QModelIndex sourceIndexClaimId    = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_CLAIM_ID,    sourceIndexZero);
            QModelIndex sourceIndexNymId      = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_NYM_ID,      sourceIndexZero);
            QModelIndex sourceIndexSection    = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_SECTION,     sourceIndexZero);
            QModelIndex sourceIndexType       = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_TYPE,        sourceIndexZero);
            QModelIndex sourceIndexValue      = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_VALUE,       sourceIndexZero);
            QModelIndex sourceIndexStart      = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_START,       sourceIndexZero);
            QModelIndex sourceIndexEnd        = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_END,         sourceIndexZero);
            QModelIndex sourceIndexAttributes = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_ATTRIBUTES,  sourceIndexZero);
            QModelIndex sourceIndexAttActive  = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_ATT_ACTIVE,  sourceIndexZero);
            QModelIndex sourceIndexAttPrimary = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_ATT_PRIMARY, sourceIndexZero);
            // ----------------------------------------------------------------------------
            QModelIndex proxyIndexValue       = pProxyModelClaims_->mapFromSource(sourceIndexValue);
            QModelIndex proxyIndexStart       = pProxyModelClaims_->mapFromSource(sourceIndexStart);
            QModelIndex proxyIndexEnd         = pProxyModelClaims_->mapFromSource(sourceIndexEnd);
            // ----------------------------------------------------------------------------
            QVariant    qvarClaimId           = pModelClaims_->data(sourceIndexClaimId);
            QVariant    qvarNymId             = pModelClaims_->data(sourceIndexNymId);
            QVariant    qvarSection           = pModelClaims_->data(sourceIndexSection);
            QVariant    qvarType              = pModelClaims_->data(sourceIndexType);
            QVariant    qvarValue             = pProxyModelClaims_->data(proxyIndexValue); // Proxy here since the proxy model decodes this. UPDATE: No longer encoded!
            QVariant    qvarStart             = pProxyModelClaims_->data(proxyIndexStart); // Proxy for these two since it formats the
            QVariant    qvarEnd               = pProxyModelClaims_->data(proxyIndexEnd);   // timestamp as a human-readable string.
            QVariant    qvarAttributes        = pModelClaims_->data(sourceIndexAttributes);
            QVariant    qvarAttActive         = pModelClaims_->data(sourceIndexAttActive);
            QVariant    qvarAttPrimary        = pModelClaims_->data(sourceIndexAttPrimary);
            // ----------------------------------------------------------------------------
            const QString     qstrClaimId =  qvarClaimId.isValid() ? qvarClaimId.toString() : "";
            const std::string claim_id    = !qstrClaimId.isEmpty() ? qstrClaimId.toStdString() : "";
            // ----------------------------------------------------------------------------
            const std::string claim_nym_id  = qvarNymId  .isValid() ? qvarNymId.toString().toStdString() : "";
            const uint32_t    claim_section = qvarSection.isValid() ? qvarSection.toUInt() : 0;
            const uint32_t    claim_type    = qvarType   .isValid() ? qvarType.toUInt() : 0;
            const std::string claim_value   = qvarValue  .isValid() ? qvarValue.toString().toStdString() : "";
            // ----------------------------------------------------------------------------
            const bool        claim_active  = qvarAttActive .isValid() ? qvarAttActive .toBool() : false;
            const bool        claim_primary = qvarAttPrimary.isValid() ? qvarAttPrimary.toBool() : false;
            // ----------------------------------------------------------------------------
            if (claim_section != indexSection)
                continue;

            QMap<uint32_t, QString>::iterator it_typeNames = mapTypeNames.find(claim_type);
            QString qstrTypeName;

            if (it_typeNames != mapTypeNames.end())
                qstrTypeName = it_typeNames.value();
            // ---------------------------------------
            // Add the claim to the tree.
            //
            QTreeWidgetItem * claim_item = new QTreeWidgetItem;
            // ---------------------------------------
            claim_item->setText(0, QString::fromStdString(claim_value)); // "james@blah.com"
            claim_item->setText(1, qstrTypeName);                        // "Personal"
            claim_item->setText(2, QString::fromStdString(nym_names[claim_nym_id]));

            claim_item->setData(0, Qt::UserRole+1, TREE_ITEM_TYPE_CLAIM);

            claim_item->setData(2, Qt::UserRole, QString::fromStdString(claim_nym_id));  // Claimant aka Me.
            // ---------------------------------------
//          claim_item->setCheckState(3, claim_primary ? Qt::Checked : Qt::Unchecked); // Moved below (as radio button)
            claim_item->setCheckState(4, claim_active  ? Qt::Checked : Qt::Unchecked);
            // ---------------------------------------
            // NOTE: We'll do this for Nyms, not for Contacts.
            // At least, not for claims. (Contacts will be able to edit
            // their own verifications, though.)
            //
//          claim_item->setFlags(claim_item->flags() |     Qt::ItemIsEditable);
            claim_item->setFlags(claim_item->flags() & ~ ( Qt::ItemIsEditable | Qt::ItemIsUserCheckable) );
            // ---------------------------------------
            topLevel->addChild(claim_item);
            treeWidgetClaims_->expandItem(claim_item);
            // --------
            // Couldn't do this until now, when the claim_item has been added to the tree.
            //
//          typedef std::tuple<std::string, uint32_t, uint32_t> ButtonGroupKey;
//          typedef std::map<ButtonGroupKey, QButtonGroup *> mapOfButtonGroups;
//          mapOfButtonGroups mapButtonGroups;

            ButtonGroupKey keyBtnGroup{claim_nym_id, claim_section, claim_type};
            mapOfButtonGroups::iterator it_btn_group = mapButtonGroups.find(keyBtnGroup);
            QButtonGroup * pButtonGroup = nullptr;

            if (mapButtonGroups.end() != it_btn_group)
                pButtonGroup = it_btn_group->second;
            else
            {
                // The button group doesn't exist yet, for this tuple.
                // (So let's create it.)
                //
                pButtonGroup = new QButtonGroup(treeWidgetClaims_);
                mapButtonGroups.insert(std::pair<ButtonGroupKey, QButtonGroup *>(keyBtnGroup, pButtonGroup));
            }

            QRadioButton * pRadioBtn = new QRadioButton(treeWidgetClaims_);
            pButtonGroup->addButton(pRadioBtn);
            pRadioBtn->setChecked(claim_primary);
            pRadioBtn->setEnabled(false);
            // --------
            treeWidgetClaims_->setItemWidget(claim_item, 3, pRadioBtn);
            // ----------------------------------------------------------------------------
            // VERIFICATIONS
            // ----------------------------------------------------------------------------
            // So we want to get the verifications known in the database for the
            // current claim.
            //
            QPointer<ModelVerifications> pVerifications = DBHandler::getInstance()->getVerificationsModel(qstrClaimId);

            if (!pVerifications)
                continue;
            // ----------------------------------------------------------------------------
            // Here we now add the sub-widgets for the claim verifications.
            //
            QPointer<VerificationsProxyModel> pProxyModelVerifications = new VerificationsProxyModel;
            pProxyModelVerifications->setSourceModel(pVerifications);
            // ------------------------------------------
            for (int iii = 0; iii < pProxyModelVerifications->rowCount(); ++iii)
            {
                QModelIndex proxyIndexZero        = pProxyModelVerifications->index(iii, 0);
                QModelIndex sourceIndexZero       = pProxyModelVerifications->mapToSource(proxyIndexZero);
                // ----------------------------------------------------------------------------
                QModelIndex sourceIndexVerId       = pVerifications->sibling(sourceIndexZero.row(), VERIFY_SOURCE_COL_VERIFICATION_ID, sourceIndexZero);
                QModelIndex sourceIndexClaimantId  = pVerifications->sibling(sourceIndexZero.row(), VERIFY_SOURCE_COL_CLAIMANT_NYM_ID, sourceIndexZero);
                QModelIndex sourceIndexVerifierId  = pVerifications->sibling(sourceIndexZero.row(), VERIFY_SOURCE_COL_VERIFIER_NYM_ID, sourceIndexZero);
                QModelIndex sourceIndexClaimId     = pVerifications->sibling(sourceIndexZero.row(), VERIFY_SOURCE_COL_CLAIM_ID,        sourceIndexZero);
                QModelIndex sourceIndexPolarity    = pVerifications->sibling(sourceIndexZero.row(), VERIFY_SOURCE_COL_POLARITY,        sourceIndexZero);
                QModelIndex sourceIndexStart       = pVerifications->sibling(sourceIndexZero.row(), VERIFY_SOURCE_COL_START,           sourceIndexZero);
                QModelIndex sourceIndexEnd         = pVerifications->sibling(sourceIndexZero.row(), VERIFY_SOURCE_COL_END,             sourceIndexZero);
                QModelIndex sourceIndexSignature   = pVerifications->sibling(sourceIndexZero.row(), VERIFY_SOURCE_COL_SIGNATURE,       sourceIndexZero);
                QModelIndex sourceIndexSigVerified = pVerifications->sibling(sourceIndexZero.row(), VERIFY_SOURCE_COL_SIG_VERIFIED,    sourceIndexZero);
                // ----------------------------------------------------------------------------
                QModelIndex proxyIndexPolarity    = pProxyModelVerifications->mapFromSource(sourceIndexPolarity);
                QModelIndex proxyIndexStart       = pProxyModelVerifications->mapFromSource(sourceIndexStart);
                QModelIndex proxyIndexEnd         = pProxyModelVerifications->mapFromSource(sourceIndexEnd);
                // ----------------------------------------------------------------------------
                QVariant    qvarVerId             = pVerifications->data(sourceIndexVerId);
                QVariant    qvarClaimantId        = pVerifications->data(sourceIndexClaimantId);
                QVariant    qvarVerifierId        = pVerifications->data(sourceIndexVerifierId);
                QVariant    qvarClaimId           = pVerifications->data(sourceIndexClaimId);
                // ----------------------------------------------------------------------------
                QVariant    qvarPolarity          = pProxyModelVerifications->data(proxyIndexPolarity);
                QVariant    qvarStart             = pProxyModelVerifications->data(proxyIndexStart); // Proxy for these two since it formats the
                QVariant    qvarEnd               = pProxyModelVerifications->data(proxyIndexEnd);   // timestamp as a human-readable string.
                // ----------------------------------------------------------------------------
                QVariant    qvarSignature         = pVerifications->data(sourceIndexSignature);
                QVariant    qvarSigVerified       = pVerifications->data(sourceIndexSigVerified);
                // ----------------------------------------------------------------------------
                const QString     qstrVerId      =  qvarVerId  .isValid() ? qvarVerId.toString() : "";
                const std::string verif_id       = !qstrVerId  .isEmpty() ? qstrVerId.toStdString() : "";
                // ----------------------------------------------------------------------------
                const QString     qstrClaimantId =  qvarClaimantId.isValid() ? qvarClaimantId.toString() : "";
                const QString     qstrVerifierId =  qvarVerifierId.isValid() ? qvarVerifierId.toString() : "";

                const std::string claimant_id    = !qstrClaimantId.isEmpty() ? qstrClaimantId.toStdString() : "";
                const std::string verifier_id    = !qstrVerifierId.isEmpty() ? qstrVerifierId.toStdString() : "";
                // ----------------------------------------------------------------------------
                const QString     qstrVerificationClaimId =  qvarClaimId.isValid() ? qvarClaimId.toString() : "";
                const std::string verification_claim_id   = !qstrVerificationClaimId.isEmpty() ? qstrVerificationClaimId.toStdString() : "";
                // ----------------------------------------------------------------------------
                QString qstrPolarity(tr("No comment"));
                const int claim_polarity =  qvarPolarity.isValid() ? qvarPolarity.toInt() : 0;
                opentxs::ClaimPolarity claimPolarity = intToClaimPolarity(claim_polarity);

                if (opentxs::ClaimPolarity::NEUTRAL == claimPolarity)
                {
                    qDebug() << __FUNCTION__ << ": ERROR! A claim verification can't have neutral polarity, since that "
                                "means no verification exists. How did it get into the database this way?";
                    continue;
                }

                const bool bPolarity = (opentxs::ClaimPolarity::NEGATIVE == claimPolarity) ? false : true;
                qstrPolarity = bPolarity ? tr("Confirmed") : tr("Refuted");
                // ----------------------------------------------------------------------------
                const QString     qstrSignature   =  qvarSignature.isValid()   ? qvarSignature.toString() : "";
                const std::string verif_signature = !qstrSignature.isEmpty()   ? qstrSignature.toStdString() : "";
                // ---------------------------------------
                const bool        bSigVerified    = qvarSigVerified .isValid() ? qvarSigVerified.toBool() : false;
                // ----------------------------------------------------------------------------
                //#define VERIFY_SOURCE_COL_VERIFICATION_ID 0
                //#define VERIFY_SOURCE_COL_CLAIMANT_NYM_ID 1
                //#define VERIFY_SOURCE_COL_VERIFIER_NYM_ID 2
                //#define VERIFY_SOURCE_COL_CLAIM_ID 3
                //#define VERIFY_SOURCE_COL_POLARITY 4
                //#define VERIFY_SOURCE_COL_START 5
                //#define VERIFY_SOURCE_COL_END 6
                //#define VERIFY_SOURCE_COL_SIGNATURE 7
                //#define VERIFY_SOURCE_COL_SIG_VERIFIED 8

//                QStringList labels = {
//                      tr("Value")
//                    , tr("Type")
//                    , tr("Nym")
//                    , tr("Primary")
//                    , tr("Active")
//                    , tr("Polarity")
//                };
                // ---------------------------------------
                MTNameLookupQT theLookup;
                mapOfNymNames::iterator it_names = nym_names.find(verifier_id);
                std::string str_verifier_name;

                if (nym_names.end() != it_names)
                    str_verifier_name =  it_names->second;
                else
                    str_verifier_name = theLookup.GetNymName(verifier_id, "");

//              const QString qstrClaimIdLabel = QString("%1: %2").arg(tr("Claim Id")).arg(qstrVerificationClaimId);
                const QString qstrClaimantIdLabel = QString("%1: %2").arg(tr("Claimant")).arg(qstrClaimantId);
                const QString qstrVerifierIdLabel = QString("%1: %2").arg(tr("Nym Id")).arg(qstrVerifierId);
                const QString qstrVerifierLabel = QString("%1: %2").arg(tr("Verifier")).arg(QString::fromStdString(str_verifier_name));
                // ---------------------------------------
                const QString qstrSignatureLabel   = QString("%1").arg(qstrSignature.isEmpty() ? tr("missing signature") : tr("signature exists"));
                const QString qstrSigVerifiedLabel = QString("%1").arg(bSigVerified ? tr("signature verified") : tr("signature failed"));
                // ---------------------------------------
                // Add the verification to the tree.
                //
                QTreeWidgetItem * verification_item = new QTreeWidgetItem;
                // ---------------------------------------
                verification_item->setText(0, qstrVerifierLabel); // "Jim Bob" (the Verifier on this claim verification.)
                verification_item->setText(1, qstrVerifierIdLabel); // with Verifier Nym Id...
                verification_item->setText(2, qstrClaimantIdLabel); // Verifies for Claimant Nym Id...
                verification_item->setText(3, qstrSignatureLabel);
                verification_item->setText(4, qstrSigVerifiedLabel);

                verification_item->setData(0, Qt::UserRole+1, TREE_ITEM_TYPE_VERIFICATION);

                verification_item->setData(0, Qt::UserRole, qstrVerId); // Verification ID.
                verification_item->setData(1, Qt::UserRole, qstrVerifierId); // Verifier Nym ID.
                verification_item->setData(2, Qt::UserRole, qstrClaimantId); // Claims have the claimant ID at index 2, so I'm matching that here.
                verification_item->setData(3, Qt::UserRole, qstrVerificationClaimId); // Claim ID stored here.
                verification_item->setData(5, Qt::UserRole, bPolarity); // Polarity
                // ---------------------------------------
                verification_item->setFlags(verification_item->flags() & ~ ( Qt::ItemIsEditable | Qt::ItemIsUserCheckable) );
    //          verification_item->setFlags(verification_item->flags() |     Qt::ItemIsEditable);
                // ---------------------------------------
                verification_item->setBackgroundColor(5, bPolarity ? QColor("green") : QColor("red"));
                // ---------------------------------------
                claim_item->addChild(verification_item);
                treeWidgetClaims_->expandItem(verification_item);
                // ----------------------------------------------------------------------------
            } // for (verifications)
        } // for (claims)
    }

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
    ui(new Ui::MTNymDetails),
    pActionRegister_(nullptr),
    pActionUnregister_(nullptr)
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
}


void MTNymDetails::on_toolButtonDescription_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();

    if (nullptr != clipboard)
    {
        clipboard->setText(ui->lineEditDescription->text());

        QMessageBox::information(this, tr("Moneychanger"), QString("%1:<br/>%2").
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

        QMessageBox::information(this, tr("Moneychanger"), QString("%1:<br/>%2").
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
        return NULL; // out of bounds.
    // -----------------------------
    QWidget * pReturnValue = NULL;
    // -----------------------------
    switch (nTab)
    {
    case 0: // "Profile" tab
        if (m_pOwner)
        {
            if (pLabelNymId_)
            {
                pLabelNymId_->setParent(NULL);
                pLabelNymId_->disconnect();
                pLabelNymId_->deleteLater();

                pLabelNymId_ = NULL;
            }
            pLabelNymId_ = new QLabel(Moneychanger::It()->expertMode() ?
                        tr("Profile (below) for Nym ID: ") : tr("Profile: "));

            if (pToolButtonNymId_)
            {
                pToolButtonNymId_->setParent(NULL);
                pToolButtonNymId_->disconnect();
                pToolButtonNymId_->deleteLater();

                pToolButtonNymId_ = NULL;
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
                pLineEditNymId_->setParent(NULL);
                pLineEditNymId_->disconnect();
                pLineEditNymId_->deleteLater();

                pLineEditNymId_ = NULL;
            }

            pLineEditNymId_ = new QLineEdit;
            pLineEditNymId_->setReadOnly(true);
            pLineEditNymId_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            pLineEditNymId_->setStyleSheet("QLineEdit { background-color: lightgray }");
            // -------------------------------
            QHBoxLayout * phBoxNymId = new QHBoxLayout;
            QWidget * pWidgetNymId   = new QWidget;

//            QMargins margins = pWidgetNymId->contentsMargins();
//            QMargins newMargins(margins.left(), margins.top(), margins.right(), 0);
//            pWidgetNymId->setContentsMargins(newMargins);

            phBoxNymId->setContentsMargins(0, 0, 0, 0);

            phBoxNymId->addWidget(pLabelNymId_);
            phBoxNymId->addWidget(pLineEditNymId_);
            phBoxNymId->addWidget(pToolButtonNymId_);

            if (!Moneychanger::It()->expertMode())
            {
                pLineEditNymId_  ->setVisible(false);
                pToolButtonNymId_->setVisible(false);
            }

            pWidgetNymId->setLayout(phBoxNymId);
            // -------------------------------
            if (treeWidgetClaims_)
            {
                treeWidgetClaims_->setParent(NULL);
                treeWidgetClaims_->disconnect();
                treeWidgetClaims_->deleteLater();

                treeWidgetClaims_ = NULL;
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
            // -------------------------------
            QHBoxLayout * phBox        = new QHBoxLayout;
            QPushButton * pEditButton  = new QPushButton(tr("Edit Profile"));
            QSpacerItem * pSpacerItem1 = new QSpacerItem(0, 0, QSizePolicy::Expanding);
            QSpacerItem * pSpacerItem2 = new QSpacerItem(0, 0, QSizePolicy::Expanding);

            connect(pEditButton, SIGNAL(clicked(bool)), this, SLOT(on_btnEditProfile_clicked()));

            phBox->addSpacerItem(pSpacerItem1);
            phBox->addWidget(pEditButton);
            phBox->addSpacerItem(pSpacerItem2);
            // -------------------------------
            QVBoxLayout * pvBox = new QVBoxLayout;
            // -------------------------------
//            QFrame * line = new QFrame;
//            line->setFrameShape(QFrame::HLine);
//            line->setFrameShadow(QFrame::Sunken);
            // -------------------------------
//            QLabel * pLabel = new QLabel( QString("%1:").arg(tr("Profile")) );

            pvBox->setAlignment(Qt::AlignTop);
            pvBox->addWidget(pWidgetNymId);
//            pvBox->addWidget(line);
//            pvBox->addWidget   (pLabel);
            pvBox->addWidget   (treeWidgetClaims_);
            pvBox->addLayout   (phBox);
            // -------------------------------
            pReturnValue = new QWidget;
            pReturnValue->setContentsMargins(0, 0, 0, 0);
            pReturnValue->setLayout(pvBox);

            treeWidgetClaims_->setContextMenuPolicy(Qt::CustomContextMenu);

            connect(treeWidgetClaims_, SIGNAL(customContextMenuRequested(const QPoint &)),
                    this, SLOT(on_treeWidget_customContextMenuRequested(const QPoint &)));

        }
        break;

    case 1: // "Credentials" tab
        if (m_pOwner)
        {
            if (m_pCredentials)
            {
                m_pCredentials->setParent(NULL);
                m_pCredentials->disconnect();
                m_pCredentials->deleteLater();

                m_pCredentials = NULL;
            }
            m_pCredentials = new MTCredentials(NULL, *m_pOwner);
            m_pCredentials->setContentsMargins(0,0,0,0);

            QWidget * pWidgetCred = new QWidget;
            QVBoxLayout * pvBox = new QVBoxLayout;

            pvBox->setMargin(0);
            pvBox->addWidget(m_pCredentials);

            pWidgetCred->setLayout(pvBox);

            pReturnValue = pWidgetCred;
            pReturnValue->setContentsMargins(0, 0, 0, 0);
        }
        break;

    case 2: // "State of Nym" tab
    {
//        if (pToolButtonNymId_)
//        {
//            pToolButtonNymId_->setParent(NULL);
//            pToolButtonNymId_->disconnect();
//            pToolButtonNymId_->deleteLater();

//            pToolButtonNymId_ = NULL;
//        }

//        QPixmap pixmapCopy(":/icons/icons/Basic-Copy-icon.png");
//        QIcon   copyIcon  (pixmapCopy);
//        // ----------------------------------------------------------------
//        pToolButtonNymId_ = new QToolButton;
//        pToolButtonNymId_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//        pToolButtonNymId_->setIcon(copyIcon);

//        connect(pToolButtonNymId_.data(), SIGNAL(clicked()), this, SLOT(on_toolButton_clicked()));

//        if (pLineEditNymId_)
//        {
//            pLineEditNymId_->setParent(NULL);
//            pLineEditNymId_->disconnect();
//            pLineEditNymId_->deleteLater();

//            pLineEditNymId_ = NULL;
//        }

//        pLineEditNymId_ = new QLineEdit;
//        pLineEditNymId_->setReadOnly(true);
//        pLineEditNymId_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
//        pLineEditNymId_->setStyleSheet("QLineEdit { background-color: lightgray }");
//        // -------------------------------
//        QHBoxLayout * phBox = new QHBoxLayout;

//        phBox->addWidget(pLineEditNymId_);
//        phBox->addWidget(pToolButtonNymId_);
        // -------------------------------
        if (pLabelNotaries_)
        {
            pLabelNotaries_->setParent(NULL);
            pLabelNotaries_->disconnect();
            pLabelNotaries_->deleteLater();
            pLabelNotaries_ = nullptr;

        }

        pLabelNotaries_ = new QLabel(tr("Registered on notaries:"));

        if (pTableWidgetNotaries_)
        {
            pTableWidgetNotaries_->setParent(NULL);
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
            m_pPlainTextEdit->setParent(NULL);
            m_pPlainTextEdit->disconnect();
            m_pPlainTextEdit->deleteLater();

            m_pPlainTextEdit = NULL;
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
        return NULL;
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
    case 1:  qstrReturnValue = "Credentials";  break;
    case 2:  qstrReturnValue = "State";        break;

    default:
        qDebug() << QString("Unexpected: MTNymDetails::GetCustomTabName was called with bad index: %1").arg(nTab);
        return QString("");
    }
    // -----------------------------
    return qstrReturnValue;
}
// ------------------------------------------------------

QWidget * MTNymDetails::createNewAddressWidget(QString strNymID)
{
    QWidget     * pWidget = new QWidget;
    QPushButton * pBtnAdd = new QPushButton(tr("Add"));
    /*
    QString create_msg_method = "CREATE TABLE msg_method"
            " (method_id INTEGER PRIMARY KEY,"   // 1, 2, etc.
            "  method_display_name TEXT,"        // "Localhost"
            "  method_type TEXT,"                // "bitmessage"
            "  method_type_display TEXT,"        // "Bitmessage"
            "  method_connect TEXT)";            // "http://username:password@http://127.0.0.1:8332/"
    */
  //QString create_nym_method
  // = "CREATE TABLE nym_method(nym_id TEXT, method_id INTEGER, address TEXT, PRIMARY KEY(nym_id, method_id, address))";
  //QString create_contact_method
  // = "CREATE TABLE contact_method(contact_id INTEGER, method_type TEXT, address TEXT, PRIMARY KEY(contact_id, method_type, address))";

    QComboBox   * pCombo  = new QComboBox;
    mapIDName     mapMethods;
    MTContactHandler::getInstance()->GetMsgMethods(mapMethods);
    // -----------------------------------------------
    int nIndex = -1;
    for (mapIDName::iterator ii = mapMethods.begin(); ii != mapMethods.end(); ++ii)
    {
        ++nIndex; // 0 on first iteration.
        // ------------------------------
        QString method_id   = ii.key();
        QString method_name = ii.value();
        // ------------------------------
        pCombo->insertItem(nIndex, method_name, method_id);
    }
    // -----------------------------------------------
    if (mapMethods.size() > 0)
    {
        pWidget->setVisible(true);
        pCombo->setCurrentIndex(0);
    }
    else
    {
        pWidget->setVisible(false);
        pBtnAdd->setEnabled(false);
    }
    // -----------------------------------------------
    QLabel      * pLabel   = new QLabel(tr("Address:"));
    QLineEdit   * pAddress = new QLineEdit;
    QHBoxLayout * layout   = new QHBoxLayout;
    // -----------------------------------------------
    pCombo   ->setMinimumWidth(60);
    pLabel   ->setMinimumWidth(55);
    pLabel   ->setMaximumWidth(55);
    pAddress ->setMinimumWidth(60);

    pBtnAdd->setProperty("nymid",        strNymID);
    pBtnAdd->setProperty("methodcombo",  VPtr<QWidget>::asQVariant(pCombo));
    pBtnAdd->setProperty("addressedit",  VPtr<QWidget>::asQVariant(pAddress));
    pBtnAdd->setProperty("methodwidget", VPtr<QWidget>::asQVariant(pWidget));
    // -----------------------------------------------
    layout->addWidget(pCombo);
    layout->addWidget(pLabel);
    layout->addWidget(pAddress);
    layout->addWidget(pBtnAdd);
    // -----------------------------------------------
    pWidget->setLayout(layout);
    // -----------------------------------------------
    layout->setStretch(0,  1);
    layout->setStretch(1, -1);
    layout->setStretch(2,  3);
    layout->setStretch(3,  1);
    // -----------------------------------------------
    connect(pBtnAdd, SIGNAL(clicked()), this, SLOT(on_btnAddressAdd_clicked()));
    // -----------------------------------------------
    return pWidget;
}

void MTNymDetails::on_btnAddressAdd_clicked()
{
    QObject * pqobjSender = QObject::sender();

    if (NULL != pqobjSender)
    {
        QPushButton * pBtnAdd = dynamic_cast<QPushButton *>(pqobjSender);

        if (m_pAddresses && (NULL != pBtnAdd))
        {
            QVariant    varNymID       = pBtnAdd->property("nymid");
            QVariant    varMethodCombo = pBtnAdd->property("methodcombo");
            QVariant    varAddressEdit = pBtnAdd->property("addressedit");
            QString     qstrNymID      = varNymID.toString();
            QComboBox * pCombo         = VPtr<QComboBox>::asPtr(varMethodCombo);
            QLineEdit * pAddressEdit   = VPtr<QLineEdit>::asPtr(varAddressEdit);
            QWidget   * pWidget        = VPtr<QWidget>::asPtr(pBtnAdd->property("methodwidget"));

            if (!qstrNymID.isEmpty() && (NULL != pCombo) && (NULL != pAddressEdit) && (NULL != pWidget))
            {
                int     nMethodID       = 0;
                QString qstrAddress     = QString("");
                QString qstrDisplayAddr = QString("");
                // --------------------------------------------------
                if (pCombo->currentIndex() < 0)
                    return;
                // --------------------------------------------------
                QVariant varMethodID = pCombo->itemData(pCombo->currentIndex());
                nMethodID = varMethodID.toInt();

                if (nMethodID <= 0)
                    return;
                // --------------------------------------------------
                qstrAddress = pAddressEdit->text();

                if (qstrAddress.isEmpty())
                    return;
                // --------------------------------------------------
                bool bAdded = MTContactHandler::getInstance()->AddMsgAddressToNym(qstrNymID, nMethodID, qstrAddress);

                if (bAdded) // Let's add it to the GUI, too, then.
                {
                    QString qstrTypeDisplay = MTContactHandler::getInstance()->GetMethodTypeDisplay(nMethodID);
                    qstrDisplayAddr = QString("%1: %2").arg(qstrTypeDisplay).arg(qstrAddress);
                    // --------------------------------------------------
                    QLayout     * pLayout = m_pAddresses->layout();
                    QVBoxLayout * pVBox   = (NULL == pLayout) ? NULL : dynamic_cast<QVBoxLayout *>(pLayout);

                    if (NULL != pVBox)
                    {
                        QWidget * pNewWidget = this->createSingleAddressWidget(qstrNymID, nMethodID, qstrAddress, qstrAddress);

                        if (NULL != pNewWidget)
                            pVBox->insertWidget(pVBox->count()-1, pNewWidget);
                    }
                }
            }
        }
    }
}

void MTNymDetails::on_btnAddressDelete_clicked()
{
    QObject * pqobjSender = QObject::sender();

    if (NULL != pqobjSender)
    {
        QPushButton * pBtnDelete = dynamic_cast<QPushButton *>(pqobjSender);

        if (m_pAddresses && (NULL != pBtnDelete))
        {
            QVariant  varNymID      = pBtnDelete->property("nymid");
            QVariant  varMethodID   = pBtnDelete->property("methodid");
            QVariant  varMethodAddr = pBtnDelete->property("methodaddr");
            QString   qstrNymID     = varNymID     .toString();
            int       nMethodID     = varMethodID  .toInt();
            QString   qstrAddress   = varMethodAddr.toString();
            QWidget * pWidget       = VPtr<QWidget>::asPtr(pBtnDelete->property("methodwidget"));

            if (NULL != pWidget)
            {
                bool bRemoved = MTContactHandler::getInstance()->RemoveMsgAddressFromNym(qstrNymID, nMethodID, qstrAddress);

                if (bRemoved) // Let's remove it from the GUI, too, then.
                {
                    QLayout * pLayout = m_pAddresses->layout();

                    if (NULL != pLayout)
                    {
                        pLayout->removeWidget(pWidget);

                        pWidget->setParent(NULL);
                        pWidget->disconnect();
                        pWidget->deleteLater();

                        pWidget = NULL;
                    }
                }
            }
        }
    }
}


QWidget * MTNymDetails::createSingleAddressWidget(QString strNymID, int nMethodID, QString qstrAddress, QString qstrDisplayAddr)
{
    QString qstrMethodName = (nMethodID > 0) ? MTContactHandler::getInstance()->GetMethodDisplayName(nMethodID) : QString(tr("Unknown"));

    QWidget     * pWidget    = new QWidget;
    QLineEdit   * pMethod    = new QLineEdit(qstrMethodName);
    QLabel      * pLabel     = new QLabel(tr("Address:"));
    QLineEdit   * pAddress   = new QLineEdit(qstrDisplayAddr);
    QPushButton * pBtnDelete = new QPushButton(tr("Delete"));
    // ----------------------------------------------------------
    pMethod ->setMinimumWidth(60);
    pLabel  ->setMinimumWidth(55);
    pLabel  ->setMaximumWidth(55);
    pAddress->setMinimumWidth(60);

    pMethod ->setReadOnly(true);
    pAddress->setReadOnly(true);

    pMethod ->setStyleSheet("QLineEdit { background-color: lightgray }");
    pAddress->setStyleSheet("QLineEdit { background-color: lightgray }");

    pBtnDelete->setProperty("nymid",        strNymID);
    pBtnDelete->setProperty("methodid",     nMethodID);
    pBtnDelete->setProperty("methodaddr",   qstrAddress);
    pBtnDelete->setProperty("methodwidget", VPtr<QWidget>::asQVariant(pWidget));
    // ----------------------------------------------------------
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(pMethod);
    layout->addWidget(pLabel);
    layout->addWidget(pAddress);
    layout->addWidget(pBtnDelete);
    // ----------------------------------------------------------
    pWidget->setLayout(layout);

    connect(pBtnDelete, SIGNAL(clicked()), this, SLOT(on_btnAddressDelete_clicked()));
    // ----------------------------------------------------------
    layout->setStretch(0,  1);
    layout->setStretch(1, -1);
    layout->setStretch(2,  3);
    layout->setStretch(3,  1);
    // ----------------------------------------------------------
    pMethod ->home(false);
    pAddress->home(false);
    // ----------------------------------------------------------
    return pWidget;
}

QGroupBox * MTNymDetails::createAddressGroupBox(QString strNymID)
{
    QGroupBox   * pBox = new QGroupBox(tr("P2P Addresses"));
    QVBoxLayout * vbox = new QVBoxLayout;
    // -----------------------------------------------------------------
    int nAddressesAdded = 0;
    // -----------------------------------------------------------------
    // Loop through all known transport methods (communications addresses)
    // known for this Nym,
    mapIDName theMap;

    if (!strNymID.isEmpty() && MTContactHandler::getInstance()->GetMethodsAndAddrByNym(theMap, strNymID))
    {
        for (mapIDName::iterator it = theMap.begin(); it != theMap.end(); ++it)
        {
            QString qstrID          = it.key();   // QString("%1|%2").arg(qstrAddress).arg(nMethodID);
            QString qstrDisplayAddr = it.value(); // QString("%1: %2").arg(qstrTypeDisplay).arg(qstrAddress);

            QStringList stringlist = qstrID.split("|");

            if (stringlist.size() >= 2) // Should always be 2...
            {
                QString qstrAddress  = stringlist.at(0);
                QString qstrMethodID = stringlist.at(1);
                const int  nMethodID = qstrMethodID.isEmpty() ? 0 : qstrMethodID.toInt();
                // --------------------------------------
                QWidget * pWidget = this->createSingleAddressWidget(strNymID, nMethodID, qstrAddress, qstrAddress);

                if (NULL != pWidget)
                {
                    nAddressesAdded++;
                    vbox->addWidget(pWidget);
                }
            }
        }
    }
    // -----------------------------------------------------------------
    QWidget * pWidget = this->createNewAddressWidget(strNymID);

    if (NULL != pWidget)
        vbox->addWidget(pWidget);
    // -----------------------------------------------------------------
    pBox->setLayout(vbox);
    // -----------------------------------------------------------------
    if (pWidget->isHidden() && (0 == nAddressesAdded))
        pBox->hide();
    // -----------------------------------------------------------------
    return pBox;
}

//virtual
void MTNymDetails::refresh(QString strID, QString strName)
{
    if (treeWidgetClaims_)
        ClearTree();

    if ((NULL != ui) && !strID.isEmpty())
    {
        std::string nym_description = opentxs::OTAPI_Wrap::Exec()->GetNym_Description(strID.toStdString());
        ui->toolButtonQrCode->setString(QString::fromStdString(nym_description));
        ui->lineEditDescription->setText(QString::fromStdString(nym_description));

        QWidget * pHeaderWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, strID, strName,
                                                                           "", "", ":/icons/icons/identity_BW.png",
                                                                           false);

        pHeaderWidget->setObjectName(QString("DetailHeader")); // So the stylesheet doesn't get applied to all its sub-widgets.

        if (m_pHeaderWidget)
        {
            ui->verticalLayout->removeWidget(m_pHeaderWidget);

            m_pHeaderWidget->setParent(NULL);
            m_pHeaderWidget->disconnect();
            m_pHeaderWidget->deleteLater();

            m_pHeaderWidget = NULL;
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
            const int32_t serverCount = opentxs::OTAPI_Wrap::Exec()->GetServerCount();

            for (int32_t serverIndex = 0; serverIndex < serverCount; ++serverIndex)
            {
                std::string NotaryID   = opentxs::OTAPI_Wrap::Exec()->GetServer_ID(serverIndex);
                QString qstrNotaryID   = QString::fromStdString(NotaryID);
                QString qstrNotaryName = QString::fromStdString(opentxs::OTAPI_Wrap::Exec()->GetServer_Name(NotaryID));
                bool    bStatus        = opentxs::OTAPI_Wrap::Exec()->IsNym_RegisteredAtServer(nymId, NotaryID);
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
        QLayout   * pLayout    = nullptr;
        QGroupBox * pAddresses = this->createAddressGroupBox(strID);

        QWidget   * pTab = GetTab(1); // Tab 1 is the index (starting at 0) for tab 2. So this means tab 2.

        if (nullptr != pTab)
            pLayout = pTab->layout();

        if (m_pAddresses) // Delete the old one.
        {
            if (nullptr != pLayout)
                pLayout->removeWidget(m_pAddresses);

            m_pAddresses->setParent(NULL);
            m_pAddresses->disconnect();
            m_pAddresses->deleteLater();
            m_pAddresses = NULL;
        }

        if (nullptr != pLayout)
        {
            pLayout->addWidget(pAddresses);
            m_pAddresses = pAddresses;
        }
        else // Should never actually happen.
        {
            delete pAddresses;
            pAddresses = nullptr;
        }
        // ----------------------------------
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
            QString strContents = QString::fromStdString(opentxs::OTAPI_Wrap::Exec()->GetNym_Stats(strID.toStdString()));
            m_pPlainTextEdit->setPlainText(strContents);
        }
        // -----------------------------------
        // TAB: "CREDENTIALS"
        //
        if (m_pCredentials)
        {
            QStringList qstrlistNymIDs;
            qstrlistNymIDs.append(strID);

            m_pCredentials->refresh(qstrlistNymIDs);
        }
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
    if (m_pCredentials)
        m_pCredentials->ClearContents();
    // ------------------------------------------
    if (m_pPlainTextEdit)
        m_pPlainTextEdit->setPlainText("");
    // ------------------------------------------
    ClearTree();
    // ------------------------------------------
    clearNotaryTable();
    if (m_pAddresses)
    {
        QWidget * pTab = GetTab(1); // Tab 1 is the index (starting at 0) for tab 2. So this means tab 2.

        if (nullptr != pTab)
        {
            QLayout * pLayout = pTab->layout();

            if (nullptr != pLayout)
                pLayout->removeWidget(m_pAddresses);
        }

        m_pAddresses->setParent(NULL);
        m_pAddresses->disconnect();
        m_pAddresses->deleteLater();
        m_pAddresses = NULL;
    }
}

// ------------------------------------------------------

void MTNymDetails::FavorLeftSideForIDs()
{
    if (NULL != ui)
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
        bool bCanRemove = opentxs::OTAPI_Wrap::Exec()->Wallet_CanRemoveNym(m_pOwner->m_qstrCurrentID.toStdString());

        if (!bCanRemove)
        {
            QMessageBox::warning(this, tr("Moneychanger"),
                                 tr("For your protection, Nyms already registered on a notary cannot be summarily deleted. "
                                    "Please unregister first. (You may also delete need to any accounts you may have registered "
                                    "at that same notary using the same Nym.)"));
            return;
        }
        // ----------------------------------------------------
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, tr("Moneychanger"), tr("Are you sure you want to delete this Nym?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            bool bSuccess = opentxs::OTAPI_Wrap::Exec()->Wallet_RemoveNym(m_pOwner->m_qstrCurrentID.toStdString());

            if (bSuccess)
            {
                m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
                // ------------------------------------------------
                emit nymsChanged();
                // ------------------------------------------------
            }
            else
                QMessageBox::warning(this, tr("Moneychanger"),
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
        opentxs::OTAPI_Wrap::Exec()->GetContactData(str_nym_id);
    const auto claims =
        opentxs::proto::DataToProto<opentxs::proto::ContactData>
            (opentxs::Data(
                claims_data.c_str(),
                static_cast<uint32_t>(claims_data.length())));

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
            opentxs::proto::ProtoAsArmored(contactData, "CONTACT DATA");
        const bool set =
            opentxs::OTAPI_Wrap::Exec()->SetContactData(str_nym_id, armored.Get());
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
                str_id = opentxs::OTAPI_Wrap::CreateIndividualNym(qstrName.toStdString(), NYM_ID_SOURCE, 0);
                break;
            case 1: // 1024-bit RSA
                str_id = opentxs::OTAPI_Wrap::CreateNymLegacy(1024, NYM_ID_SOURCE);
                break;
//            case 2: // 2048-bit RSA
//                str_id = opentxs::OT_ME::It().create_nym_legacy(2048, NYM_ID_SOURCE);
//                break;
//            case 3: // 4096-bit RSA
//                str_id = opentxs::OT_ME::It().create_nym_legacy(4096, NYM_ID_SOURCE);
//                break;
//            case 4: // 8192-bit RSA
//                str_id = opentxs::OT_ME::It().create_nym_legacy(8192, NYM_ID_SOURCE);
//                break;
            default:
                QMessageBox::warning(this, tr("Moneychanger"),
                    tr("Unexpected key type."));
                return;
        }
        // --------------------------------------------------
        if (str_id.empty())
        {
            QMessageBox::warning(this, tr("Moneychanger"),
                tr("Failed trying to create Nym."));
            return;
        }
        // ------------------------------------------------------
        // Get the ID of the new nym.
        //
        QString qstrID = QString::fromStdString(str_id);
        // ------------------------------------------------------
        // Register the Namecoin name.
        if (nAuthorityIndex == 1)
        {
            const unsigned cnt = opentxs::OTAPI_Wrap::Exec()->GetNym_MasterCredentialCount (str_id);
            if (cnt != 1)
            {
                qDebug () << "Expected one master credential, got " << cnt
                          << ".  Skipping Namecoin registration.";
            }
            else
            {
                const std::string cred = opentxs::OTAPI_Wrap::Exec()->GetNym_MasterCredentialID (str_id, 0);
                const QString qCred = QString::fromStdString (cred);
                NMC_NameManager& nmc = NMC_NameManager::getInstance ();
                nmc.startRegistration (qstrID, qCred);
            }
        }
        // ------------------------------------------------------
        // Set the Name of the new Nym.
        //
        //bool bNameSet =
        //opentxs::OTAPI_Wrap::Exec()->SetNym_Name(qstrID.toStdString(), qstrID.toStdString(), qstrName.toStdString());
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
            opentxs::proto::ProtoAsArmored(contactData, "CONTACT DATA");

        if (!opentxs::OTAPI_Wrap::Exec()->SetContactData(str_id, armored.Get())) {
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
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        QString qstrNymID(m_pOwner->m_qstrCurrentID);
        QString qstrNymName(m_pOwner->m_qstrCurrentName);
        const std::string str_nym_id = qstrNymID.toStdString();
        // ----------------------------------------------------
        QTreeWidgetItem * pItem = treeWidgetClaims_->itemAt(pos);

        if (NULL != pItem)
        {
//          const int nRow = pItem->row();
//          if (nRow >= 0)
            {
                //#define TREE_ITEM_TYPE_RELATIONSHIP    10
                //#define TREE_ITEM_TYPE_CLAIM           11
                //#define TREE_ITEM_TYPE_VERIFICATION    12

                QVariant qvarTreeItemType = pItem->data(0, Qt::UserRole+1);
                const int nTreeItemType = qvarTreeItemType.isValid() ? qvarTreeItemType.toInt() : 0;

                const bool bIsRelationship = (TREE_ITEM_TYPE_RELATIONSHIP == nTreeItemType);
                const bool bIsClaim        = (TREE_ITEM_TYPE_CLAIM        == nTreeItemType);
                const bool bIsVerification = (TREE_ITEM_TYPE_VERIFICATION == nTreeItemType);
                // -----------------------------------------------------------------------------
                // Relationships, here, are claims ABOUT my Nym -- this Nym -- ("ME").
                // They are claims that were made by OTHERS. (But since my wallet contains multiple
                // Nyms, "others" may include MY Nyms in this wallet, in addition to outside Nyms
                // controlled by other users.)
                //
                // Thus for some of these, these records are read-only. Which is to say, a Nym can only
                // view the claims made about him by others, in a read-only fashion. He cannot delete
                // other people's claims. BUT! If the claim was made by ANOTHER Nym in the SAME WALLET,
                // then THAT Nym DOES have the right to delete this claim. (Though normally that sort
                // of thing is done in the Contact details tab, but we could make it available here as
                // well.)
                //
                // In either case and in any case, I ("Me" Nym) am here viewing relationship claims made
                // about me by "others", and I have the power to confirm or refute them by creating a
                // verification for that specific relationships claim.
                //
                // So I may sign someone's relationship claim, thus confirming it (or refuting it.) So
                // each of these relationship claims may also display its "confirmation status" whenever
                // it's available. And I (the current Nym) have the power to Confirm/Refute/NoComment
                // and thus change the confirmation status of any claim in this block.
                //
                if (bIsRelationship)
                {
                    QVariant qvarClaimId       = pItem->data(1, Qt::UserRole);
                    QVariant qvarClaimantNymId = pItem->data(0, Qt::UserRole);
                    QVariant qvarPolarity      = pItem->data(5, Qt::UserRole);

                    QString qstrClaimId       = qvarClaimId      .isValid() ? qvarClaimId      .toString() : "";
                    QString qstrClaimantNymId = qvarClaimantNymId.isValid() ? qvarClaimantNymId.toString() : "";

                    QString & qstrVerifierNymId = qstrNymID;

                    if (qstrClaimId.isEmpty() || qstrClaimantNymId.isEmpty())
                        return;

                    opentxs::ClaimPolarity claimPolarity = qvarPolarity.isValid() ?
                                intToClaimPolarity(static_cast<int>(qvarPolarity.toUInt())) :
                                opentxs::ClaimPolarity::NEUTRAL;
                    // ----------------------------------
                    pActionConfirm_ = nullptr;
                    pActionRefute_ = nullptr;
                    pActionNoComment_ = nullptr;

                    popupMenuProfile_.reset(new QMenu(this));

                    if (claimPolarity    != opentxs::ClaimPolarity::POSITIVE)
                        pActionConfirm_   = popupMenuProfile_->addAction(tr("Confirm"));
                    if (claimPolarity    != opentxs::ClaimPolarity::NEGATIVE)
                        pActionRefute_    = popupMenuProfile_->addAction(tr("Refute"));
                    if (claimPolarity    != opentxs::ClaimPolarity::NEUTRAL)
                        pActionNoComment_ = popupMenuProfile_->addAction(tr("No comment"));
                    // ------------------------
                    QPoint globalPos = treeWidgetClaims_->mapToGlobal(pos);
                    // ------------------------
                    const QAction* selectedAction = popupMenuProfile_->exec(globalPos); // Here we popup the menu, and get the user's click.
                    // ------------------------
                    if (nullptr == selectedAction)
                    {

                    }
                    else if (selectedAction == pActionConfirm_)
                    {
                        // If true, that means OT had to CHANGE something in the Nym's data.
                        // (So we'll need to broadcast that, so Moneychanger can re-import the Nym.)
                        //
                        if (MTContactHandler::getInstance()->claimVerificationConfirm(qstrClaimId, qstrClaimantNymId, qstrVerifierNymId))
                        {
                            emit nymWasJustChecked(qstrVerifierNymId);
                            return;
                        }
                    }
                    // ------------------------
                    else if (selectedAction == pActionRefute_)
                    {
                        if (MTContactHandler::getInstance()->claimVerificationRefute(qstrClaimId, qstrClaimantNymId, qstrVerifierNymId))
                        {
                            emit nymWasJustChecked(qstrVerifierNymId);
                            return;
                        }
                    }
                    // ------------------------
                    else if (selectedAction == pActionNoComment_)
                    {
                        if (MTContactHandler::getInstance()->claimVerificationNoComment(qstrClaimId, qstrClaimantNymId, qstrVerifierNymId))
                        {
                            emit nymWasJustChecked(qstrVerifierNymId);
                            return;
                        }
                    }
                } // if is relationship.
                // -----------------------------------------------
                else if (bIsClaim)
                {
                    // It's a normal claim (made by me, versus relationships above, made by others ABOUT me.)
                    //
                    // NOTE: To edit your own claims, just click the "edit profile" button.)

                } // if is claim.
                // -----------------------------------------------
                else if (bIsVerification) // verification
                {
                    // It's a verification of a claim. *I* make my own claims, and OTHER people
                    // create verifications for those claims. So normally I CAN'T edit other people's
                    // verifications.
                    //
                    // HOWEVER, there are multiple Nyms in this wallet. So some of these verifications,
                    // while they are not created by the "Me" Nym currently displayed in the Nym Details,
                    // nevertheless, some of them may have been created by Nyms under the control of this
                    // wallet -- Nyms which have the same power to edit the same verification now!
                    // So I have the power to edit SOME verifications, if they were created by other Nyms
                    // in the wallet. Otherwise, if they were created by other Nyms outside of the wallet,
                    // obviously I do NOT have the power to edit THOSE verifications.


                    // For now we won't put any verification editing code here, since normally only "others"
                    // can do this. (If you want to do this now, just edit the verification from the contact
                    // details page, instead of the Nym details page.)
                    //
                    // The more advanced functionality described in the larger above comment can come later.


                } // if is verification
                // -----------------------------------------------
                else
                {
                    // This space intentionally left blank.
                }
                // ------------------------
            } //nRow >= 0
        }
    }
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

        if (NULL != pItem)
        {
            int nRow = pItem->row();

            if (nRow >= 0)
            {
                QString qstrNotaryID = pTableWidgetNotaries_->item(nRow, 0)->data(Qt::UserRole).toString();
                std::string str_notary_id = qstrNotaryID.toStdString();
                QString qstrNotaryName = QString::fromStdString(opentxs::OTAPI_Wrap::Exec()->GetServer_Name(str_notary_id));
                // ------------------------
                QPoint globalPos = pTableWidgetNotaries_->mapToGlobal(pos);
                // ------------------------
                const QAction* selectedAction = popupMenu_->exec(globalPos); // Here we popup the menu, and get the user's click.
                // ------------------------
                if (selectedAction == pActionRegister_)
                {
                    if (opentxs::OTAPI_Wrap::Exec()->IsNym_RegisteredAtServer(str_nym_id, str_notary_id))
                    {
                        QMessageBox::information(this, tr("Moneychanger"), QString("%1 '%2' %3 '%4'.").arg(tr("The Nym")).
                                                 arg(qstrNymName).arg(tr("is already registered on notary")).arg(qstrNotaryName));
                        return;
                    }
                    else
                    {


                        int32_t nSuccess = 0;
                        bool    bRegistered = false;
                        {
                            MTSpinner theSpinner;

                            auto strResponse = opentxs::OTAPI_Wrap::Register_Nym_Public(str_nym_id, str_notary_id);

                            if (strResponse) {
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

                                QMessageBox::information(this, tr("Moneychanger"),
                                    tr("Success!"));
                                // ------------------------------------------------
                                emit nymsChanged();
                                // ------------------------------------------------
                                break; // SUCCESS
                            }
                        case (0):
                            {
                                QMessageBox::warning(this, tr("Moneychanger"),
                                    tr("Failed while trying to register nym on Server."));
                                break;
                            }
                        default:
                            {
                                QMessageBox::warning(this, tr("Moneychanger"),
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
                    if (!opentxs::OTAPI_Wrap::Exec()->IsNym_RegisteredAtServer(str_nym_id, str_notary_id))
                    {
                        QMessageBox::information(this, tr("Moneychanger"), QString("%1 '%2' %3 '%4'.").arg(tr("The Nym")).
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
                            QMessageBox::information(this, tr("Moneychanger"), QString("%1 '%2' %3 '%4'. %5.").arg(tr("The Nym")).
                                                     arg(qstrNymName).arg(tr("still has asset accounts on the notary")).arg(qstrNotaryName).arg(tr("Please delete those first")));
                            return;
                        }
                        // -----------------------------------
                        QMessageBox::StandardButton reply;
                        reply = QMessageBox::question(this, tr("Moneychanger"),
                                                      tr("Are you sure you want to unregister your Nym from this notary?"),
                                                      QMessageBox::Yes|QMessageBox::No);
                        if (reply == QMessageBox::Yes)
                        {


                            int32_t nSuccess = 0;
                            bool    bUnregistered = false;
                            {
                                MTSpinner theSpinner;

                                std::string strResponse = opentxs::OT_ME::It().unregister_nym(str_notary_id, str_nym_id);
                                nSuccess                = opentxs::OT_ME::It().VerifyMessageSuccess(strResponse);
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

                                    QMessageBox::information(this, tr("Moneychanger"),
                                        tr("Success!"));
                                    // ------------------------------------------------
                                    emit nymsChanged();
                                    // ------------------------------------------------
                                    break; // SUCCESS
                                }
                            case (0):
                                {
                                    QMessageBox::warning(this, tr("Moneychanger"),
                                        tr("Failed while trying to unregister nym from Server."));
                                    break;
                                }
                            default:
                                {
                                    QMessageBox::warning(this, tr("Moneychanger"),
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
        bool bSuccess = opentxs::OTAPI_Wrap::Exec()->Rename_Nym(m_pOwner->m_qstrCurrentID.toStdString(), // Nym
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









