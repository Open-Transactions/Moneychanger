#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/contactdetails.hpp>
#include <ui_contactdetails.h>

#include <gui/widgets/credentials.hpp>
#include <gui/ui/dlgnewcontact.hpp>
#include <gui/ui/getstringdialog.hpp>

#include <gui/widgets/compose.hpp>
#include <gui/widgets/senddlg.hpp>
#include <gui/widgets/dlgchooser.hpp>
#include <gui/widgets/overridecursor.hpp>

#include <core/moneychanger.hpp>
#include <core/handlers/DBHandler.hpp>
#include <core/handlers/contacthandler.hpp>
#include <core/handlers/modelclaims.hpp>
#include <core/handlers/modelverifications.hpp>

#include <opentxs/opentxs.hpp>

#include <QComboBox>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QMessageBox>
#include <QTreeView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMenu>
#include <QStringList>
#include <QTimer>
#include <QDebug>
#include <QRadioButton>
#include <QButtonGroup>

#include <map>
#include <tuple>
#include <string>


#define TREE_ITEM_TYPE_RELATIONSHIP    10
#define TREE_ITEM_TYPE_CLAIM           11
#define TREE_ITEM_TYPE_VERIFICATION    12


MTContactDetails::MTContactDetails(QWidget *parent, MTDetailEdit & theOwner) :
    MTEditDetails(parent, theOwner),
    ui(new Ui::MTContactDetails)
{
    ui->setupUi(this);
    this->setContentsMargins(0, 0, 0, 0);

    ui->lineEditID->setStyleSheet("QLineEdit { background-color: lightgray }");

    // ----------------------------------
    // Note: This is a placekeeper, so later on I can just erase
    // the widget at 0 and replace it with the real header widget.
    //
    m_pHeaderWidget  = new QWidget;
    ui->verticalLayout->insertWidget(0, m_pHeaderWidget);
    // ----------------------------------
    if (!Moneychanger::It()->expertMode())
    {
        ui->lineEditID->setVisible(false);
        ui->labelID->setVisible(false);
    }
}

MTContactDetails::~MTContactDetails()
{
    delete ui;
}


// ----------------------------------
//virtual
int MTContactDetails::GetCustomTabCount()
{
    return (Moneychanger::It()->expertMode()) ? 4 : 2;
}
// ----------------------------------
//virtual
QWidget * MTContactDetails::CreateCustomTab(int nTab)
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
    case 0: // "Notes" tab
        if (m_pOwner)
        {
            if (m_pPlainTextEditNotes)
            {
                m_pPlainTextEditNotes->setParent(NULL);
                m_pPlainTextEditNotes->disconnect();
                m_pPlainTextEditNotes->deleteLater();

                m_pPlainTextEditNotes = NULL;
            }
            m_pPlainTextEditNotes = new QPlainTextEdit;

            m_pPlainTextEditNotes->setReadOnly(false);
            m_pPlainTextEditNotes->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
            // -------------------------------
            QVBoxLayout * pvBox = new QVBoxLayout;

            QLabel * pLabelContents = new QLabel(tr("Notes (these don't save yet):"));

            pvBox->setAlignment(Qt::AlignTop);
            pvBox->addWidget   (pLabelContents);
            pvBox->addWidget   (m_pPlainTextEditNotes);
            // -------------------------------
            pReturnValue = new QWidget;
            pReturnValue->setContentsMargins(0, 0, 0, 0);
            pReturnValue->setLayout(pvBox);
        }
        break;

    case 1: // "Profile" tab
        if (m_pOwner)
        {
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
            QVBoxLayout * pvBox = new QVBoxLayout;

            QLabel * pLabel = new QLabel( QString("%1:").arg(tr("Profile")) );

            pvBox->setAlignment(Qt::AlignTop);
            pvBox->addWidget   (pLabel);
            pvBox->addWidget   (treeWidgetClaims_);
            // -------------------------------
            pReturnValue = new QWidget;
            pReturnValue->setContentsMargins(0, 0, 0, 0);
            pReturnValue->setLayout(pvBox);

            treeWidgetClaims_->setContextMenuPolicy(Qt::CustomContextMenu);

            connect(treeWidgetClaims_, SIGNAL(customContextMenuRequested(const QPoint &)),
                    this, SLOT(on_treeWidget_customContextMenuRequested(const QPoint &)));

        }
        break;

    case 2: // "Credentials" tab
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
            pReturnValue = m_pCredentials;
            pReturnValue->setContentsMargins(0, 0, 0, 0);
        }
        break;

    case 3: // "Known IDs" tab
    {
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
        QVBoxLayout * pvBox = new QVBoxLayout;

        QLabel * pLabelContents = new QLabel(tr("Known IDs:"));

        pvBox->setAlignment(Qt::AlignTop);
        pvBox->addWidget   (pLabelContents);
        pvBox->addWidget   (m_pPlainTextEdit);
        // -------------------------------
        pReturnValue = new QWidget;
        pReturnValue->setContentsMargins(0, 0, 0, 0);
        pReturnValue->setLayout(pvBox);
    }
        break;

    default:
        qDebug() << QString("Unexpected: MTContactDetails::CreateCustomTab was called with bad index: %1").arg(nTab);
        return NULL;
    }
    // -----------------------------
    return pReturnValue;
}
// ---------------------------------
//virtual
QString  MTContactDetails::GetCustomTabName(int nTab)
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
    case 0:  qstrReturnValue = "Notes";        break;
    case 1:  qstrReturnValue = "Profile";      break;
    case 2:  qstrReturnValue = "Credentials";  break;
    case 3:  qstrReturnValue = "Known IDs";    break;

    default:
        qDebug() << QString("Unexpected: MTContactDetails::GetCustomTabName was called with bad index: %1").arg(nTab);
        return QString("");
    }
    // -----------------------------
    return qstrReturnValue;
}
// ------------------------------------------------------



//virtual
void MTContactDetails::DeleteButtonClicked()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        bool bSuccess = MTContactHandler::getInstance()->DeleteContact(m_pOwner->m_qstrCurrentID.toInt());

        if (bSuccess)
        {
            m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
            // ------------------------------------------------
            emit RefreshRecordsAndUpdateMenu();
            // ------------------------------------------------
        }
    }
}

//virtual
void MTContactDetails::AddButtonClicked()
{
    MTDlgNewContact theNewContact(this);
    theNewContact.setWindowTitle(tr("Create New Contact"));
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
                             tr("Sorry, that is not a valid Open-Transactions Nym ID."));
        return;
    }
    // Below this point we know we've got a valid Opentxs NymId for the Contact
    // we're adding. (Though a contact may already exist, we'll find out next...)
    // --------------------------------------------------------
    QString qstrContactId;
    const auto existingContactId = opentxs::Identifier::Factory(opentxs::OT::App().Contact().ContactID(opentxs::Identifier::Factory(str_nym_id)));
    const bool bPreexistingOpentxsContact{!existingContactId->empty()};
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
            // The opentxs contact was just created, and so the opentxs contacts dialog may wish to refresh itself.
            //  After this block we'll make sure it's the one selected. (We know it already exists here).
        }
        else if (bPreexistingOpentxsContact) { // This block means the old-style Moneychanger Contact already existed, AND the new-style Opentxs Contact already existed as well!
            QString contactName = MTContactHandler::getInstance()->GetContactName(nExistingContactId);
            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                 tr("Contact '%1' already exists with NymID: %2").arg(contactName).arg(nymID));
            // Since it already exists, we'll select it in the GUI, so the user can see what we're
            // talking about when we tell him that it already exists.
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
        const QString qstrContactID = QString::number(nContactId);
        m_pOwner->SetPreSelected(qstrContactID);
        emit RefreshRecordsAndUpdateMenu();
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
        // Now let's add this contact to the Map, and refresh the dialog,
        // and then set the new contact as the current one.
        //
        const QString qstrContactID = QString::number(nContactId);
        m_pOwner->m_map.insert(qstrContactID, qstrNewContactLabel); // Since it wasn't pre-existing.
        m_pOwner->SetPreSelected(qstrContactID);
        emit RefreshRecordsAndUpdateMenu();
    }
}






void MTContactDetails::on_treeWidget_customContextMenuRequested(const QPoint &pos)
{
    if (m_pOwner->m_qstrCurrentID.isEmpty())
        return;

    const int nContactId = m_pOwner->m_qstrCurrentID.isEmpty() ? 0 : m_pOwner->m_qstrCurrentID.toInt();

    if (nContactId > 0)
    {
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
                const bool bIsMetInPerson  = (pItem == metInPerson_);
                // -----------------------------------------------------------------------------
                // Relationships are claims ABOUT a Contact's Nym made by OTHERS. (Including
                // possibly YOU.)
                // Thus for some of these, you can only view them. But for others (your own) you
                // can edit them as well.
                // Each of these relationship claims can be verified by the Contact himself. He
                // may sign someone's claim, thus confirming it (or refuting it.) So each of
                // these relationship claims may also display its "confirmation status" whenever
                // it's available.
                //
                if (bIsRelationship)
                {

                    // For now, the Contact is considered "Someone Else."
                    //
                    // And since the Contact is "someone else", we can see all the relationship claims made
                    // by OTHERS, ABOUT him. We cannot alter those. (And for now we'll set aside the case
                    // where the Contact also happens to be one of the Nyms in your wallet. In which case
                    // you can go to the Nym Details tab in the meantime, to access that functionality.)
                    //
                    // But ALSO since the Contact is "someone else", we can delete our OWN relationship claims
                    // about him here.


                    // Relationships are added as claims made by others, about the contact. His verifications
                    // of those claims appear on the claim itself. (No need for separate verifications to be
                    // displayed below the claim, since ONLY the contact can verify relationship claims about
                    // himself that were made by others.
                    //
//                    claim_item->setText(0, qstrClaimantLabel);      // "Alice" (some lady) from NymId
//                    claim_item->setText(1, qstrTypeName);           // "Has met" (or so she claimed)
//                    claim_item->setText(2, QString::fromStdString(str_nym_name));  // "Charlie" (one of the Nyms on this Contact.)

//                    claim_item->setData(0, Qt::UserRole+1, TREE_ITEM_TYPE_RELATIONSHIP);

//                    claim_item->setData(0, Qt::UserRole, qstrClaimId);
//                    claim_item->setData(2, Qt::UserRole, QString::fromStdString(claim_nym_id)); // For now this is Alice's Nym Id. Not sure how we'll use it.


                    // TODO here: Menu option to delete an existing relationship claim ABOUT this
                    // Contact's Nym, IF that claim was made by one of MY Nyms in this wallet.


                    //

                    QVariant qvarClaimId       = pItem->data(0, Qt::UserRole);
                    QVariant qvarClaimantNymId = pItem->data(2, Qt::UserRole);

                    QString qstrClaimId       = qvarClaimId      .isValid() ? qvarClaimId      .toString() : "";
                    QString qstrClaimantNymId = qvarClaimantNymId.isValid() ? qvarClaimantNymId.toString() : "";
                    // --------------------------------
                    if (qstrClaimantNymId.isEmpty())
                        return;
                    // --------------------------------
                    const std::string str_claimant_nym_id = qstrClaimantNymId.toStdString();

                    if (!opentxs::OT::App().API().Exec().VerifyUserPrivateKey(str_claimant_nym_id))
                        return;
                    // ----------------------------------
                    pActionConfirm_ = nullptr;
                    pActionRefute_ = nullptr;
                    pActionNoComment_ = nullptr;
                    pActionNewRelationship_ = nullptr;
                    pActionDeleteRelationship_ = nullptr;

                    popupMenuProfile_.reset(new QMenu(this));

                    pActionDeleteRelationship_ = popupMenuProfile_->addAction(tr("Delete relationship claim"));
                    // ------------------------
                    QPoint globalPos = treeWidgetClaims_->mapToGlobal(pos);
                    // ------------------------
                    const QAction* selectedAction = popupMenuProfile_->exec(globalPos); // Here we popup the menu, and get the user's click.
                    // ------------------------
                    if (nullptr == selectedAction)
                    {
                         // This space intentionally left blank.
                    }
                    else if (selectedAction == pActionDeleteRelationship_)
                    {
                        // ----------------------------------------------
                        // Get the Nym. Make sure we have the latest copy, since his credentials were apparently
                        // just downloaded and overwritten.
                        //
                        opentxs::OTPasswordData thePWData("Deleting relationship claim.");

                        // ------------------------------------------------
                        std::string str_claim_id(qstrClaimId.toStdString());
                        if (opentxs::OT::App().API().Exec().DeleteClaim(qstrClaimantNymId.toStdString(), str_claim_id))
                        {
                            emit nymWasJustChecked(qstrClaimantNymId);
                            return;
                        }
                    }
                    else
                    {
                        // This space intentionally left blank.
                    }

                } // if is relationship.
                // -----------------------------------------------
                // This is for relationships, like above. Except above, someone right-clicked on an
                // actual pre-existing relationship. Whereas here, someone right-clicked on the parent
                // node that contains all the relationships. (Probably because he wants to CREATE A
                // NEW RELATIONSHIP.)
                //
                else if (bIsMetInPerson)
                {
                    pActionConfirm_ = nullptr;
                    pActionRefute_ = nullptr;
                    pActionNoComment_ = nullptr;
                    pActionNewRelationship_ = nullptr;
                    pActionDeleteRelationship_ = nullptr;

                    popupMenuProfile_.reset(new QMenu(this));

                    pActionNewRelationship_ = popupMenuProfile_->addAction(tr("I've met this contact in real life..."));
                    // ------------------------
                    QPoint globalPos = treeWidgetClaims_->mapToGlobal(pos);
                    // ------------------------
                    const QAction* selectedAction = popupMenuProfile_->exec(globalPos); // Here we popup the menu, and get the user's click.
                    // ------------------------
                    if (nullptr == selectedAction)
                    {
                         // This space intentionally left blank.
                    }
                    else if (selectedAction == pActionNewRelationship_)
                    {
                        // Get the list of Nym Ids known for this contact.
                        //
                        // If there's only one Nym for this contact, select it automatically.
                        // Otherwise ask the user to choose the Nym where he wants to add the
                        // relationship.
                        //
                        mapIDName theNymMap;

                        if (!MTContactHandler::getInstance()->GetNyms(theNymMap, nContactId))
                        {
                            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                                 tr("Sorry, there are no opentxs NymIDs associated with this contact. "
                                                    "(Only Open-Transactions identities can create secure relationships.)"));
                            return;
                        }
                        QString qstrContactNymId;

                        if (theNymMap.size() == 1) // This contact has exactly one Nym, so we'll go with it.
                        {
                            mapIDName::iterator theNymIt = theNymMap.begin();

                            qstrContactNymId = theNymIt.key();
            //              QString qstrNymName = theNymIt.value();
                        }
                        else // There are multiple Nyms to choose from.
                        {
                            DlgChooser theNymChooser(this);
                            theNymChooser.m_map = theNymMap;
                            theNymChooser.setWindowTitle(tr("Contact has multiple Nyms. (Please choose one.)"));
                            // -----------------------------------------------
                            if (theNymChooser.exec() == QDialog::Accepted)
                                qstrContactNymId = theNymChooser.m_qstrCurrentID;
                            else // User must have canceled.
                                qstrContactNymId = QString("");
                        }
                        // --------------
                        if (qstrContactNymId.isEmpty())
                            return;
                        // ---------------------------------------
                        // Then for the claimant, just use the default Nym.
                        //
                        const QString qstrDefaultNymId = Moneychanger::It()->getDefaultNymID();

                        if (qstrDefaultNymId.isEmpty())
                        {
                            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                                 QString("%1").arg(tr("Your default identity is not set. Please do that and then try again. "
                                                                      "(It will be the signing identity used.)")));
                            return;
                        }
                        const QString & qstrClaimantNymId = qstrDefaultNymId;
                        // -----------------------------------------------
                        // Make sure the claimant nym is not the same as the contact's selected nym.
                        // (If so, advise the user to select a different default nym.)
                        if (0 == qstrClaimantNymId.compare(qstrContactNymId))
                        {
                            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                                 QString("%1").arg(tr("Sorry, an identity (a Nym) cannot make relationship claims about himself. "
                                                                      "You could try selecting a different default Nym for yourself, "
                                                                      "or select a different Nym for this contact, if one is available.")));
                            return;
                        }
                        // -----------------------------------------------
                        // Grab the various relationship type names.
                        //
                        DlgChooser theChooser(this);
                        mapIDName & the_map = theChooser.m_map;
                        // -----------------------------------------------
                        auto sectionTypes =
                                opentxs::OT::App().API().Exec().ContactSectionTypeList(
                                    opentxs::proto::CONTACTSECTION_RELATIONSHIP);
                        QMap<uint32_t, QString> mapTypeNames;

                        for (auto & indexSectionType: sectionTypes) {
                            auto typeName =
                                opentxs::OT::App().API().Exec().ContactTypeName(
                                    indexSectionType);
                            mapTypeNames.insert(indexSectionType, QString::fromStdString(typeName));

                            the_map.insert(QString::number(indexSectionType), QString::fromStdString(typeName));
                        }
                        // -----------------------------------------------
                        // Pop up a dialog with a drop-down and select the relationship
                        // being added.
                        //
                        theChooser.setWindowTitle(tr("Your relationship to this contact:"));
                        if (theChooser.exec() != QDialog::Accepted)
                            return;
                        // -----------------------------------------------
                        QString strSectionTypeId = theChooser.GetCurrentID();
                        uint32_t sectionType = strSectionTypeId.isEmpty() ? 0 : strSectionTypeId.toUInt();

                        if (0 == sectionType)
                            return;
                        // ----------------------------------------------
                        auto strClaimantNymId = opentxs::String(qstrClaimantNymId.toStdString());
                        auto claimant_nym_id = opentxs::Identifier::Factory(strClaimantNymId);
                        // ----------------------------------------------
                        // Get the Nym. Make sure we have the latest copy, since his credentials were apparently
                        // just downloaded and overwritten.
                        //
                        opentxs::OTPasswordData thePWData("Adding relationship claim.");
                        std::shared_ptr<const opentxs::Nym> pClaimantNym =
                        	opentxs::OT::App().Wallet().Nym(claimant_nym_id);

                        if (false == bool(pClaimantNym))
                        {
                            qDebug() << __FUNCTION__ << "Strange: failed trying to get reloadAndGetNym from the Wallet.";
                            return;
                        }
                        // ------------------------------------------------
                        opentxs::proto::ContactItem item;
                        item.set_version(CONTACT_CONTACT_DATA_VERSION);
                        item.set_type(
                            static_cast<opentxs::proto::ContactItemType>
                                (sectionType));
                        item.set_value(qstrContactNymId.toStdString());
                        item.set_start(0);
                        item.set_end(0);
                        item.add_attribute(opentxs::proto::CITEMATTR_ACTIVE);

                        // If true, that means OT had to CHANGE something in the
                        // Nym's data. (So we'll need to broadcast that, so
                        // Moneychanger can re-import the Nym.)
                        const bool set =
                            opentxs::OT::App().API().Exec().SetClaim(
                                qstrClaimantNymId.toStdString(),
                                opentxs::proto::CONTACTSECTION_RELATIONSHIP,
                                opentxs::proto::ProtoAsString(item));

                        if (set) {
                            emit nymWasJustChecked(qstrClaimantNymId);
                            return;
                        }
                    }
                    else
                    {
                        // This space intentionally left blank.
                    }
                }
                // Else if it's a normal claim made BY this Contact's Nym (versus relationship claims
                // above, made by others ABOUT him.)
                //
                else if (bIsClaim) // A claim this contact's Nym has made about himself.
                {
                    // Add ability to create a verification to confirm / refute the contact's selected claim.
                    // I'll use any of my Nyms in my wallet (the default one, to start) to confirm
                    // or refute the selected claim by the contact's Nym. (Except for that same Nym
                    // itself, of course, if it's in both places.)
                    //
                    //

                    // For reference purposes only:
//                    claim_item->setText(0, QString::fromStdString(claim_value)); // "james@blah.com"
//                    claim_item->setText(1, qstrTypeName);                        // "Personal"
//                    claim_item->setText(2, QString::fromStdString(nym_names[claim_nym_id]));

//                    claim_item->setData(0, Qt::UserRole+1, TREE_ITEM_TYPE_CLAIM);

//                    claim_item->setData(0, Qt::UserRole, qstrClaimId);
//                    claim_item->setData(1, Qt::UserRole, claim_type);
//                    claim_item->setData(2, Qt::UserRole, QString::fromStdString(claim_nym_id)); // Claimant
//                    claim_item->setData(3, Qt::UserRole, claim_section); // Section

                    QVariant qvarClaimId       = pItem->data(0, Qt::UserRole);
                    QVariant qvarClaimantNymId = pItem->data(2, Qt::UserRole);

                    QString qstrClaimId       = qvarClaimId      .isValid() ? qvarClaimId      .toString() : "";
                    QString qstrClaimantNymId = qvarClaimantNymId.isValid() ? qvarClaimantNymId.toString() : "";
                    // --------------------------------
                    const QString qstrDefaultNymId = Moneychanger::It()->getDefaultNymID();

                    if (qstrDefaultNymId.isEmpty())
                    {
                        QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                             QString("%1").arg(tr("Your default identity is not set. Please do that and then try again. "
                                                                  "(It will be the signing identity used.)")));
                        return;
                    }
                    const QString & qstrVerifierNymId = qstrDefaultNymId;
                    // -----------------------------------------------
                    if (qstrClaimId.isEmpty() || qstrClaimantNymId.isEmpty())
                    {
                        QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                             QString("%1").arg(tr("Strange - Either the ClaimId or ClaimantNymId was unexpectedly blank.")));
                        return;
                    }
                    // ----------------------------------
                    pActionConfirm_ = nullptr;
                    pActionRefute_ = nullptr;
                    pActionNoComment_ = nullptr;
                    pActionNewRelationship_ = nullptr;
                    pActionDeleteRelationship_ = nullptr;

                    popupMenuProfile_.reset(new QMenu(this));

                    pActionConfirm_   = popupMenuProfile_->addAction(tr("Confirm"));
                    pActionRefute_    = popupMenuProfile_->addAction(tr("Refute"));
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
                        if (0 == qstrClaimantNymId.compare(qstrVerifierNymId))
                        {
                            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                                 QString("%1").arg(tr("An identity can't confirm or refute its own claims. "
                                                                      "(You can verify using a different Nym - just change the "
                                                                      "default Nym and then try again.)")));
                            return;
                        }
                        // ----------------------------------
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
                        if (0 == qstrClaimantNymId.compare(qstrVerifierNymId))
                        {
                            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                                 QString("%1").arg(tr("An identity can't confirm or refute its own claims. "
                                                                      "(But you CAN verify this contact, by just using a different Nym - "
                                                                      "to do that, try selecting a different default Nym for your wallet, "
                                                                      "and then come back here and try again.)")));
                            return;
                        }
                        // ----------------------------------
                        if (MTContactHandler::getInstance()->claimVerificationRefute(qstrClaimId, qstrClaimantNymId, qstrVerifierNymId))
                        {
                            emit nymWasJustChecked(qstrVerifierNymId);
                            return;
                        }
                    }
                    // ------------------------
                    else if (selectedAction == pActionNoComment_)
                    {
                        if (0 == qstrClaimantNymId.compare(qstrVerifierNymId))
                        {
                            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                                 QString("%1").arg(tr("An identity can't confirm or refute its own claims. "
                                                                      "(You can verify using a different Nym - just change "
                                                                      "the default Nym and then try again.)")));
                            return;
                        }
                        // ----------------------------------
                        if (MTContactHandler::getInstance()->claimVerificationNoComment(qstrClaimId, qstrClaimantNymId, qstrVerifierNymId))
                        {
                            emit nymWasJustChecked(qstrVerifierNymId);
                            return;
                        }
                    }

                    // Also in that case where this Contact's Nym also happens to be one of my own private Nyms
                    // in  this wallet, we can eventually add here the ability to edit the profile and change the
                    // claims that I've made. But for NOW, that functionality is already available by clicking the
                    // "edit profile" button on the Nym details tab.


                } // if is claim.
                // -----------------------------------------------
                else if (bIsVerification) // verification
                {
                    // Else if it's a verification of a claim. I make my own claims, and
                    // other people create verifications for those claims. However, I
                    // have multiple Nyms in this wallet. So some of these verifications
                    // may have been created by MY Nyms -- which I have the power to
                    // edit -- versus other people's Nyms, which I may only view.


                    // Add ability to edit the selected verification to change its status
                    // to confirm or refute -- or delete the verification entirely.
                    // One of the Nyms in my wallet (specifically the one who originally created this
                    // verification) will be the same Nym to edit or delete the verification now.
                    //
                    // NOTE: This should ONLY be possible for Verifications made by MY Nyms (that are
                    // in my wallet). All other Verifications are read-only.


                    // For reference purposes only:
//                    verification_item->setText(0, qstrVerifierLabel); // "Jim Bob" (the Verifier on this claim verification.)
//                    verification_item->setText(1, qstrVerifierIdLabel); // with Verifier Nym Id...
//                    verification_item->setText(2, qstrClaimantIdLabel); // Verifies for Claimant Nym Id...
//                    verification_item->setText(3, qstrSignatureLabel);
//                    verification_item->setText(4, qstrSigVerifiedLabel);

//                    verification_item->setData(0, Qt::UserRole+1, TREE_ITEM_TYPE_VERIFICATION);

//                    verification_item->setData(0, Qt::UserRole, qstrVerId); // Verification ID.
//                    verification_item->setData(1, Qt::UserRole, qstrVerifierId); // Verifier Nym ID.
//                    verification_item->setData(2, Qt::UserRole, qstrClaimantId); // Claims have the claimant ID at index 2, so I'm matching that here.
//                    verification_item->setData(3, Qt::UserRole, qstrVerificationClaimId); // Claim ID stored here.
//                    verification_item->setData(5, Qt::UserRole, bPolarity); // Polarity

                    const QVariant qvarClaimId       = pItem->data(3, Qt::UserRole);
                    const QVariant qvarClaimantNymId = pItem->data(2, Qt::UserRole);
                    const QVariant qvarVerifierNymId = pItem->data(1, Qt::UserRole);
                    const QVariant qvarPolarity      = pItem->data(5, Qt::UserRole);

                    const QString qstrClaimId       = qvarClaimId      .isValid() ? qvarClaimId      .toString() : "";
                    const QString qstrClaimantNymId = qvarClaimantNymId.isValid() ? qvarClaimantNymId.toString() : "";
                    const QString qstrVerifierNymId = qvarVerifierNymId.isValid() ? qvarVerifierNymId.toString() : "";

                    const bool bPolarityValid = qvarPolarity.isValid();
                    const bool bPolarity = bPolarityValid ? qvarPolarity.toBool() : false;

                    const std::string verifier_nym_id = !qstrVerifierNymId.isEmpty() ? qstrVerifierNymId.toStdString() : "";
                    // -----------------------------------------------
                    if (qstrClaimId.isEmpty() || qstrClaimantNymId.isEmpty())
                    {
                        QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                             QString("%1").arg(tr("Strange - Either the ClaimId or ClaimantNymId was unexpectedly blank. ")));
                        return;
                    }

                    // If I'm the verifier, then I can change my verification.
                    // (Otherwise I can't.)
                    //
                    if (!opentxs::OT::App().API().Exec().VerifyUserPrivateKey(verifier_nym_id))
                        return;
                    // ----------------------------------
                    pActionConfirm_ = nullptr;
                    pActionRefute_ = nullptr;
                    pActionNoComment_ = nullptr;
                    pActionNewRelationship_ = nullptr;
                    pActionDeleteRelationship_ = nullptr;

                    popupMenuProfile_.reset(new QMenu(this));

                    if (!bPolarityValid || (bPolarityValid && (true  != bPolarity)) )
                        pActionConfirm_   = popupMenuProfile_->addAction(tr("Confirm"));
                    if (!bPolarityValid || (bPolarityValid && (false != bPolarity)) )
                        pActionRefute_    = popupMenuProfile_->addAction(tr("Refute"));
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




void MTContactDetails::ClearTree()
{
    metInPerson_ = nullptr;

    if (treeWidgetClaims_)
    {
        treeWidgetClaims_->blockSignals(true);
        treeWidgetClaims_->clear();
        treeWidgetClaims_->blockSignals(false);
    }
}


void MTContactDetails::on_pushButtonRefresh_clicked()
{
    if (m_pOwner->m_qstrCurrentID.isEmpty())
        return;
    int nContactId = m_pOwner->m_qstrCurrentID.toInt();
    if (nContactId <= 0)
        return;
    // --------------------------------
    const QString qstrDefaultNymId = Moneychanger::It()->getDefaultNymID();

    if (qstrDefaultNymId.isEmpty())
    {
        QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                             QString("%1").arg(tr("Your default identity is not set. Please do that and then try again. "
                                                  "(It's needed since an identity must be used to communicate with your "
                                                  "Contact's server in order to download the credentials.)")));
        return;
    }
    const std::string my_nym_id = qstrDefaultNymId.toStdString();
    // --------------------------------
    mapIDName mapNyms, mapServers;

    // TODO Optimize: We COULD just get the NymIDs here, not the map,
    // and thus save the time we waste ascertaining the display Name, which here,
    // we aren't even using.
    //
    const bool bGotNyms = MTContactHandler::getInstance()->GetNyms(mapNyms, nContactId);
    // --------------------------------
    if (!bGotNyms)
    {
        // TODO:
        // If bGotNyms is false, we can still potentially download the related Nym based on
        // the BIP47 payment code, if we have one. That would occur here, and then we could
        // re-try the GetNyms call.

        QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                             QString("%1").arg(tr("Sorry but there are no Nyms associated with this contact yet. "
                                                  "(This is where we will, in the future, just download those from the DHT.)")));
        return;
    }
    // --------------------------------
    bTimerFired_ = false; //reset it here just in case.
    // --------------------------------
    for (mapIDName::iterator it_nyms = mapNyms.begin(); it_nyms != mapNyms.end(); ++it_nyms)
    {
        QString qstrNymID   = it_nyms.key();
//      QString qstrNymName = it_nyms.value();

        if (qstrNymID.isEmpty()) // Weird, should never happen.
        {
            qDebug() << "MTContactDetails::on_pushButtonRefresh_clicked: Unexpected empty NymId, should not happen. (Returning.)";
            return;
        }
        const std::string str_nym_id = qstrNymID.toStdString();
        // ----------------------------------------------
        const bool bGotServers = MTContactHandler::getInstance()->GetServers(mapServers, qstrNymID);

        if (!bGotServers)
        {
            // Try the default server, then, since there are no known servers for that Nym.
            const QString qstrDefaultNotaryId = Moneychanger::It()->getDefaultNotaryID();

            if (!qstrDefaultNotaryId.isEmpty())
                mapServers.insert(qstrDefaultNotaryId, "(Leaving server name empty since it's unused.)");
        }
        // -------------------------------
        for (mapIDName::iterator it_servers = mapServers.begin(); it_servers != mapServers.end(); ++it_servers)
        {
            QString qstrNotaryID   = it_servers.key();
//          QString qstrNotaryName = it_servers.value();

            if (qstrNotaryID.isEmpty()) // Weird, should never happen.
            {
                qDebug() << "MTContactDetails::on_pushButtonRefresh_clicked: Unexpected empty NotaryID, should not happen. (Returning.)";
                return;
            }
            const std::string notary_id = qstrNotaryID.toStdString();

            // Todo note: May need to verify that I'M registered at this server, before I
            // try to download some guy's credentials from a "server he's known to use."
            // I may not even be registered there, in which case the check_nym call would fail.
            //
            // ------------------------------

            std::string response;
            {
                MTSpinner theSpinner;

                auto action = opentxs::OT::App().API().ServerAction().DownloadNym(
                        opentxs::Identifier::Factory(my_nym_id), opentxs::Identifier::Factory(notary_id), opentxs::Identifier::Factory(str_nym_id));
                response = action->Run();
            }

            int32_t nReturnVal = opentxs::VerifyMessageSuccess(response);

            if (1 == nReturnVal)
            {
                emit nymWasJustChecked(qstrNymID);
                break;
            }
        } // for (servers)
    }
}

void MTContactDetails::onClaimsUpdatedTimer()
{
    bTimerFired_ = false; //reset it here so it'll work again next time.

    // NOTE: By this point, the Nym's claims should be in our local DB.
    // So I'm not worried that we'll repopulate with old claim data
    // (Since the wallet keeps the Nyms cached in RAM) because I won't be
    // populating the GUI from those Nyms anyway, but from tables in the
    // Moneychanger DB.
    //
    emit RefreshRecordsAndUpdateMenu();
}

void MTContactDetails::onClaimsUpdatedForNym(QString nymId)
{
    if (!bTimerFired_)
    {
        bTimerFired_ = true;

        // I'm doing this because perhaps 5 Nyms may update and trigger this slot,
        // all in a row. So I want to make sure the GUI doesn't update 5 times in
        // a row as well! Right now I'm giving it 5 seconds, so presumably it won't
        // take that long (I hope) for all the Nyms to update, before we then go
        // ahead and update the GUI.
        QTimer::singleShot(2000, this, SLOT(onClaimsUpdatedTimer()));
    }
}

void MTContactDetails::RefreshTree(int nContactId, QStringList & qstrlistNymIDs)
{
    if (!treeWidgetClaims_ || (NULL == ui))
        return;
    // ----------------------------------------
    ClearTree();
    // ----------------------------------------
    if ( (0 == nContactId) || (0 == qstrlistNymIDs.size()) )
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

    bool bANymWasChecked = false;

    for (int ii = 0; ii < qstrlistNymIDs.size(); ++ii)
    {
        QString qstrNymID = qstrlistNymIDs.at(ii);
        // ---------------------------------------
        if (qstrNymID.isEmpty()) // should never happen.
            continue;
        // ---------------------------------------
        const std::string str_nym_id   = qstrNymID.toStdString();
        const std::string str_nym_name = opentxs::OT::App().API().Exec().GetNym_Name(str_nym_id);
        const auto id_nym = opentxs::Identifier::Factory(str_nym_id);

        if (!str_nym_id.empty())
        {
            auto pCurrentNym = opentxs::OT::App().Wallet().Nym(id_nym);
//          const opentxs::Nym * pCurrentNym =
//                opentxs::OT::App().API().OTAPI().GetOrLoadNym(id_nym);

            // check_nym if not already downloaded.
            if (!pCurrentNym)
            {
                const QString qstrDefaultNotaryId = Moneychanger::It()->getDefaultNotaryID();
                const QString qstrDefaultNymId    = Moneychanger::It()->getDefaultNymID();

                if (!qstrDefaultNotaryId.isEmpty() && !qstrDefaultNymId.isEmpty())
                {
                    const std::string my_nym_id = qstrDefaultNymId   .toStdString();
                    const std::string notary_id = qstrDefaultNotaryId.toStdString();


                    std::string response;
                    {
                        MTSpinner theSpinner;

                        auto action = opentxs::OT::App().API().ServerAction().DownloadNym(
                                opentxs::Identifier::Factory(my_nym_id), opentxs::Identifier::Factory(notary_id), opentxs::Identifier::Factory(str_nym_id));
                        response = action->Run();
                    }

                    int32_t nReturnVal = opentxs::VerifyMessageSuccess(response);

                    if (1 == nReturnVal)
                    {
                        pCurrentNym = opentxs::OT::App().Wallet().Nym(id_nym);
//                      pCurrentNym = opentxs::OT::App().API().OTAPI().reloadAndGetNym(id_nym);
                        bANymWasChecked = true;
                        emit nymWasJustChecked(qstrNymID);
                    }
                }
            }

            if (pCurrentNym && !bANymWasChecked)
            {
                nym_names.insert(std::pair<std::string, std::string>(str_nym_id, str_nym_name));
            }
        }
        // ---------------------------------------------
        // Since we're already looping here through all the Nyms known for this
        // contact, let's go ahead and grab all claims from anyone else, that they
        // "have met" that Nym, so later we can display those on the tree in the
        // appropriate place.


        // TODO



    }
    // -------------------------------------------------
    // NOTE: If a nymWasChecked, that means we emitted a signal which the Moneychanger singleton
    // received. That caused it to to upsert any claims from that Nym into the Moneychanger DB,
    // in the claim table.
    //
    // In that same function, Moneychanger then announces by emitting its own signal, that claims
    // were just updated in the database. And that ends up causing the detail edit to refresh itself,
    // which ends up calling RefreshTree() again (the function we're in right now.)
    //
    // So that's why it's okay for us to return here, since we know this function is just going to
    // get called again anyway, and there will be better data available when it does (because by
    // then, it'll have upserted the claims already.)
    //
    if (bANymWasChecked)
        return;
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
    pModelClaims_ = DBHandler::getInstance()->getClaimsModel(nContactId);

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

//    justusranvier
//    6:45 ok
//    6:46 well, when I update opentxs-proto to include the appropriate sections and types, you'll have a Relationships section
//    6:46 and there will be an item type called "met"
//    6:46 if you want to make this claim programmatically
//    6:47 set the indexSection to proto::CONTACTSECTION_RELATIONSHIPS
//    6:47 set the indexSectionType to CITEMTYPE_MET
//    6:47 and set the nymid of the person you met to textValue
//    6:47 (based on that code sample)
//    6:47 .
//    6:48 actually both of those enums should be in the opentxs::proto:: namespace

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
    QPointer<ModelClaims> pRelationships = DBHandler::getInstance()->getRelationshipClaims(nContactId);

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
            opentxs::OT::App().API().Exec().ContactSectionName(
                opentxs::proto::CONTACTSECTION_RELATIONSHIP);
        const auto sectionTypes =
            opentxs::OT::App().API().Exec().ContactSectionTypeList(
                opentxs::proto::CONTACTSECTION_RELATIONSHIP);

        for (const auto& indexSectionType: sectionTypes) {
            const std::string typeName =
                opentxs::OT::App().API().Exec().ContactTypeName(indexSectionType);
            mapTypeNames.insert(
                indexSectionType,
                QString::fromStdString(typeName));
        }
        // ---------------------------------------

        for (int nRelationshipCount = 0; nRelationshipCount < pProxyModelRelationships->rowCount(); nRelationshipCount++)
        {
//          QSqlRecord record = pRelationships->record(nRelationshipCount);

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
            const std::string str_claimant_name = opentxs::OT::App().API().Exec().GetNym_Name(claim_nym_id);

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
            claim_item->setText(2, QString::fromStdString(str_nym_name));  // "Charlie" (one of the Nyms on this Contact.)

            claim_item->setData(0, Qt::UserRole+1, TREE_ITEM_TYPE_RELATIONSHIP);

            claim_item->setData(0, Qt::UserRole, qstrClaimId);
            claim_item->setData(2, Qt::UserRole, QString::fromStdString(claim_nym_id)); // For now this is Alice's Nym Id. Not sure how we'll use it.

//            claim_item->setData(0, Qt::UserRole, QString::fromStdString(claim_nym_id)); // Alice's Nym Id. The person who made the claim. Claimant Nym Id.
//            claim_item->setData(1, Qt::UserRole, qstrClaimId);
//            claim_item->setData(2, Qt::UserRole, QString::fromStdString(claim_value)); // Verifier Nym Id. Alice made a claim about Charlie (this current Contact in the address book), who verifies her claim. So Charlie's ID goes in Alice's claim_value.
            // ----------------------------------------
            // Since this is someone else's claim about this contact, I should see if this contact has already confirmed or refuted it.
            bool bPolarity = false;
            const bool bGotPolarity = MTContactHandler::getInstance()->getPolarityIfAny(qstrClaimId,
                                                                                        qstrClaimValue, bPolarity);

//            qDebug() << "DEBUGGING CONTACT DETAILS: QString::fromStdString(claim_value): " << QString::fromStdString(claim_value);
//            qDebug() << "DEBUGGING CONTACT DETAILS: QString::fromStdString(str_nym_id): " << QString::fromStdString(str_nym_id);

            if (bGotPolarity)
            {
                claim_item->setText(5, bPolarity ? tr("Confirmed") : tr("Refuted") );
                claim_item->setBackgroundColor(5, bPolarity ? QColor("green") : QColor("red"));
            }
            else
                claim_item->setText(5, tr("No comment"));
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

        }

    } // Relationships
    // ------------------------------------------------


    //resume


    //        QString create_claim_table = "CREATE TABLE IF NOT EXISTS claim"
    //               "(claim_id TEXT PRIMARY KEY,"
    //               " claim_nym_id TEXT,"
    //               " claim_section INTEGER,"
    //               " claim_type INTEGER,"
    //               " claim_value TEXT,"
    //               " claim_start INTEGER,"
    //               " claim_end INTEGER,"
    //               " claim_attributes TEXT,"
    //               " claim_att_active INTEGER,"
    //               " claim_att_primary INTEGER"
    //               ")";

//    // TODO



//    // ------------------------------------------------
//    opentxs::proto::ContactData contactData;
//    contactData.set_version(1); // todo hardcoding.

//    for (auto& it: items) {
//        auto newSection = contactData.add_section();
//        newSection->set_version(1);
//        newSection->set_name(static_cast<opentxs::proto::ContactSectionName>(it.first));

//        for (auto& i: it.second) {
//            auto newItem = newSection->add_item();
//            newItem->set_version(1);
//            newItem->set_type(static_cast<opentxs::proto::ContactItemType>(std::get<0>(i)));
//            newItem->set_value(std::get<1>(i));
//            if (std::get<2>(i)) {
//                newItem->add_attribute(opentxs::proto::CITEMATTR_PRIMARY);
//            }
//            newItem->add_attribute(opentxs::proto::CITEMATTR_ACTIVE);
//        }
//    }
//    // ------------------------------------------------
//    if (!opentxs::OT::App().API().OTAPI().SetContactData(*pCurrentNym, contactData))
//        qDebug() << __FUNCTION__ << ": ERROR: Failed trying to Set Contact Data!";
////      else
////          qDebug() << __FUNCTION__ << "SetContactData SUCCESS. items.size(): " << items.size();
//    // ------------------------------------------------




    // ------------------------------------------
    // Now we loop through the sections, and for each, we populate its
    // itemwidgets by looping through the nym_claims we got above.
    //
    const auto sections = opentxs::OT::App().API().Exec().ContactSectionList();

    for (const auto& indexSection: sections) {
        if (opentxs::proto::CONTACTSECTION_RELATIONSHIP == indexSection) {
            continue;
        }
        // ----------------------------------------
        QMap<uint32_t, QString> mapTypeNames;

        const std::string sectionName =
            opentxs::OT::App().API().Exec().ContactSectionName(indexSection);
        const auto sectionTypes =
            opentxs::OT::App().API().Exec().ContactSectionTypeList(indexSection);

        for (const auto& indexSectionType: sectionTypes) {
            const std::string typeName =
                opentxs::OT::App().API().Exec().ContactTypeName(indexSectionType);
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
            QModelIndex sourceIndexClaimStart = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_START,       sourceIndexZero);
            QModelIndex sourceIndexClaimEnd   = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_END,         sourceIndexZero);
            QModelIndex sourceIndexAttributes = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_ATTRIBUTES,  sourceIndexZero);
            QModelIndex sourceIndexAttActive  = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_ATT_ACTIVE,  sourceIndexZero);
            QModelIndex sourceIndexAttPrimary = pModelClaims_->sibling(sourceIndexZero.row(), CLAIM_SOURCE_COL_ATT_PRIMARY, sourceIndexZero);
            // ----------------------------------------------------------------------------
            QModelIndex proxyIndexValue       = pProxyModelClaims_->mapFromSource(sourceIndexValue);
            QModelIndex proxyIndexClaimStart  = pProxyModelClaims_->mapFromSource(sourceIndexClaimStart);
            QModelIndex proxyIndexClaimEnd    = pProxyModelClaims_->mapFromSource(sourceIndexClaimEnd);
            // ----------------------------------------------------------------------------
            QVariant    qvarClaimId           = pModelClaims_->data(sourceIndexClaimId);
            QVariant    qvarNymId             = pModelClaims_->data(sourceIndexNymId);
            QVariant    qvarSection           = pModelClaims_->data(sourceIndexSection);
            QVariant    qvarType              = pModelClaims_->data(sourceIndexType);
            QVariant    qvarValue             = pProxyModelClaims_->data(proxyIndexValue); // Proxy here since the proxy model decodes this. UPDATE: no longer encoded.
            QVariant    qvarClaimStart        = pProxyModelClaims_->data(proxyIndexClaimStart); // Proxy for these two since it formats the
            QVariant    qvarClaimEnd          = pProxyModelClaims_->data(proxyIndexClaimEnd);   // timestamp as a human-readable string.
            QVariant    qvarAttributes        = pModelClaims_->data(sourceIndexAttributes);
            QVariant    qvarAttActive         = pModelClaims_->data(sourceIndexAttActive);
            QVariant    qvarAttPrimary        = pModelClaims_->data(sourceIndexAttPrimary);
            // ----------------------------------------------------------------------------
            const QString     qstrClaimId   =  qvarClaimId.isValid() ? qvarClaimId.toString() : "";
            const std::string claim_id      = !qstrClaimId.isEmpty() ? qstrClaimId.toStdString() : "";
            // ----------------------------------------------------------------------------
            const std::string claim_nym_id  =  qvarNymId  .isValid() ? qvarNymId.toString().toStdString() : "";
            const uint32_t    claim_section =  qvarSection.isValid() ? qvarSection.toUInt() : 0;
            const uint32_t    claim_type    =  qvarType   .isValid() ? qvarType.toUInt() : 0;
            const std::string claim_value   =  qvarValue  .isValid() ? qvarValue.toString().toStdString() : "";
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

            claim_item->setData(0, Qt::UserRole, qstrClaimId);
            claim_item->setData(1, Qt::UserRole, claim_type);
            claim_item->setData(2, Qt::UserRole, QString::fromStdString(claim_nym_id)); // Claimant
            claim_item->setData(3, Qt::UserRole, claim_section); // Section

            // ---------------------------------------
//          claim_item->setCheckState(3, claim_primary ? Qt::Checked : Qt::Unchecked); // Moved below (as radio button)
//          claim_item->setCheckState(4, claim_active  ? Qt::Checked : Qt::Unchecked); // Moved below (as radio button)
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
            // ----------------------------------------------------------------------------
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
            // ----------------------------------------------------------------------------
            { // "Primary"
            QRadioButton * pRadioBtn = new QRadioButton(treeWidgetClaims_);
            pButtonGroup->addButton(pRadioBtn);
            pRadioBtn->setChecked(claim_primary);
            pRadioBtn->setEnabled(false);
            // ---------
            treeWidgetClaims_->setItemWidget(claim_item, 3, pRadioBtn);
            }
            // ----------------------------------------------------------------------------
            { // "Active"
            QRadioButton * pRadioBtn = new QRadioButton(treeWidgetClaims_);
            pRadioBtn->setChecked(claim_active);
            pRadioBtn->setEnabled(false);
            // ---------
            treeWidgetClaims_->setItemWidget(claim_item, 4, pRadioBtn);
            }
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
                mapOfNymNames::iterator it_names = nym_names.find(verifier_id);
                std::string str_verifier_name;

                if (nym_names.end() != it_names)
                    str_verifier_name =  it_names->second;
                else
                    str_verifier_name = opentxs::OT::App().API().Exec().GetNym_Name(verifier_id);

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

void MTContactDetails::ClearContents()
{
    ui->lineEditID  ->setText("");
    ui->lineEditName->setText("");
    // ------------------------------------------
    if (m_pCredentials)
        m_pCredentials->ClearContents();
    // ------------------------------------------
    if (m_pPlainTextEdit)
        m_pPlainTextEdit->setPlainText("");
    // ------------------------------------------
    if (m_pPlainTextEditNotes)
        m_pPlainTextEditNotes->setPlainText("");
    // ------------------------------------------
    ClearTree();
    // ------------------------------------------
    ui->pushButtonMsg->setEnabled(false);
    ui->pushButtonPay->setEnabled(false);
    ui->pushButtonMsg->setProperty("contactid", 0);
    ui->pushButtonPay->setProperty("contactid", 0);
    // ------------------------------------------
}


void MTContactDetails::on_btnAddressAdd_clicked()
{
//    QObject * pqobjSender = QObject::sender();
//
//    if (NULL != pqobjSender)
//    {
//        QPushButton * pBtnAdd = dynamic_cast<QPushButton *>(pqobjSender);
//
//        if (m_pAddresses && (NULL != pBtnAdd))
//        {
//            QVariant    varContactID   = pBtnAdd->property("contactid");
//            QVariant    varMethodCombo = pBtnAdd->property("methodcombo");
//            QVariant    varAddressEdit = pBtnAdd->property("addressedit");
//            int         nContactID     = varContactID.toInt();
//            QComboBox * pCombo         = VPtr<QComboBox>::asPtr(varMethodCombo);
//            QLineEdit * pAddressEdit   = VPtr<QLineEdit>::asPtr(varAddressEdit);
//            QWidget   * pWidget        = VPtr<QWidget>::asPtr(pBtnAdd->property("methodwidget"));
//
//            if ((nContactID > 0) && (NULL != pCombo) && (NULL != pAddressEdit) && (NULL != pWidget))
//            {
//                QString qstrMethodType  = QString("");
//                QString qstrAddress     = QString("");
//                // --------------------------------------------------
//                if (pCombo->currentIndex() < 0)
//                    return;
//                // --------------------------------------------------
//                QVariant varMethodType = pCombo->itemData(pCombo->currentIndex());
//                qstrMethodType = varMethodType.toString();
//
//                if (qstrMethodType.isEmpty())
//                    return;
//                // --------------------------------------------------
//                qstrAddress = pAddressEdit->text();
//
//                if (qstrAddress.isEmpty())
//                    return;
//                // --------------------------------------------------
//                bool bAdded = MTContactHandler::getInstance()->AddMsgAddressToContact(nContactID, qstrMethodType, qstrAddress);
//
//                if (bAdded) // Let's add it to the GUI, too, then.
//                {
////                    QString qstrTypeDisplay = pCombo->currentText();
////                    // --------------------------------------------------
////                    QLayout     * pLayout = m_pAddresses->layout();
////                    QVBoxLayout * pVBox   = (NULL == pLayout) ? NULL : dynamic_cast<QVBoxLayout *>(pLayout);
////
////                    if (NULL != pVBox)
////                    {
////                        QWidget * pNewWidget = this->createSingleAddressWidget(nContactID, qstrMethodType, qstrTypeDisplay, qstrAddress);
////
////                        if (NULL != pNewWidget)
////                            pVBox->insertWidget(pVBox->count()-1, pNewWidget);
////                    }
//                }
//            }
//        }
//    }
}


void MTContactDetails::on_btnAddressDelete_clicked()
{
//    QObject * pqobjSender = QObject::sender();
//
//    if (NULL != pqobjSender)
//    {
//        QPushButton * pBtnDelete = dynamic_cast<QPushButton *>(pqobjSender);
//
//        if (m_pAddresses && (NULL != pBtnDelete))
//        {
//            QVariant  varContactID   = pBtnDelete->property("contactid");
//            QVariant  varMethodType  = pBtnDelete->property("methodtype");
//            QVariant  varMethodAddr  = pBtnDelete->property("methodaddr");
//            int       nContactID     = varContactID .toInt();
//            QString   qstrMethodType = varMethodType.toString();
//            QString   qstrAddress    = varMethodAddr.toString();
//            QWidget * pWidget        = VPtr<QWidget>::asPtr(pBtnDelete->property("methodwidget"));
//
//            if (NULL != pWidget)
//            {
//                bool bRemoved = MTContactHandler::getInstance()->RemoveMsgAddressFromContact(nContactID, qstrMethodType, qstrAddress);
//
//                if (bRemoved) // Let's remove it from the GUI, too, then.
//                {
//                    QLayout * pLayout = m_pAddresses->layout();
//
//                    if (NULL != pLayout)
//                    {
//                        pLayout->removeWidget(pWidget);
//
//                        pWidget->setParent(NULL);
//                        pWidget->disconnect();
//                        pWidget->deleteLater();
//
//                        pWidget = NULL;
//                    }
//                }
//            }
//        }
//    }
}

void MTContactDetails::on_pushButtonPay_clicked()
{
    QVariant varContactID = ui->pushButtonPay->property("contactid");
    int      nContactID   = varContactID.toInt();

    if (nContactID > 0)
    {
        // --------------------------------------------------
        MTSendDlg * send_window = new MTSendDlg(NULL);
        send_window->setAttribute(Qt::WA_DeleteOnClose);
        // --------------------------------------------------
        QString qstrAcct = Moneychanger::It()->get_default_account_id();
        QString qstr_acct_id = qstrAcct.isEmpty() ? QString("") : qstrAcct;

        if (!qstr_acct_id.isEmpty())
            send_window->setInitialMyAcct(qstr_acct_id);
        // ---------------------------------------
        mapIDName theNymMap;

        if (!MTContactHandler::getInstance()->GetNyms(theNymMap, nContactID))
        {
            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME), tr("Sorry, there are no NymIDs associated with this contact. Currently only Open-Transactions payments are supported."));
            return;
        }
        else
        {
            QString qstrHisNymId;

            if (theNymMap.size() == 1) // This contact has exactly one Nym, so we'll go with it.
            {
                mapIDName::iterator theNymIt = theNymMap.begin();

                qstrHisNymId = theNymIt.key();
//              QString qstrNymName = theNymIt.value();
            }
            else // There are multiple Nyms to choose from.
            {
                DlgChooser theNymChooser(this);
                theNymChooser.m_map = theNymMap;
                theNymChooser.setWindowTitle(tr("Recipient has multiple Nyms. (Please choose one.)"));
                // -----------------------------------------------
                if (theNymChooser.exec() == QDialog::Accepted)
                    qstrHisNymId = theNymChooser.m_qstrCurrentID;
                else // User must have cancelled.
                    qstrHisNymId = QString("");
            }

            send_window->setInitialHisNym(qstrHisNymId);
        }
        // ---------------------------------------
        send_window->dialog();
        // ---------------------------------------
        Focuser f(send_window);
        f.show();
        f.focus();
    }
}

void MTContactDetails::on_pushButtonMsg_clicked()
{
    QVariant varContactID = ui->pushButtonMsg->property("contactid");
    int      nContactID   = varContactID.toInt();

    //qDebug() << QString("nContactID: %1").arg(nContactID);

    if (nContactID > 0)
    {
        // --------------------------------------------------
        MTCompose * compose_window = new MTCompose;
        compose_window->setAttribute(Qt::WA_DeleteOnClose);
        // --------------------------------------------------
        // If Moneychanger has a default Nym set, we use that for the Sender.
        // (User can always change it.)
        //
        QString qstrDefaultNym = Moneychanger::It()->get_default_nym_id();

        if (!qstrDefaultNym.isEmpty())
            compose_window->setInitialSenderNym(qstrDefaultNym);

        compose_window->setInitialRecipientContactID(nContactID); // We definitely know this, since we're on the Contacts page.
        // --------------------------------------------------
        if (!qstrDefaultNym.isEmpty()) // Sender Nym is set.
        {
            QString qstrTransportType("");
            QString qstrDefaultServer = Moneychanger::It()->get_default_notary_id();

            if (!qstrDefaultServer.isEmpty() && compose_window->setRecipientNymBasedOnContact() &&
                compose_window->verifySenderAgainstServer   (false, qstrDefaultServer) &&
                compose_window->verifyRecipientAgainstServer(false, qstrDefaultServer)) // Notice the false? That's so it doesn't pop up a dialog asking questions.
                // ---------------------------------------------------
                compose_window->setInitialServer(qstrDefaultServer);
        }
        // --------------------------------------------------
        compose_window->dialog();

        Focuser f(compose_window);
        f.show();
        f.focus();
        // --------------------------------------------------
        // Recipient has just changed. Does Sender exist? If so, make sure he is
        // compatible with msgtype or find a new one that matches both. We call this
        // here to force the compose dialog to find a matching transport method
        // between sender and recipient, if one isn't already set by now.
        //
        compose_window->FindSenderMsgMethod();
    }
}

static void blah()
{
//resume
//todo

// OT_API.hpp
//EXPORT VerificationSet GetVerificationSet(const Nym& fromNym) const;
// EXPORT bool SetVerifications(Nym& onNym,
//                            const proto::VerificationSet&) const;

// Nym.hpp
//    std::shared_ptr<proto::VerificationSet> VerificationSet() const;
//    bool SetVerificationSet(const proto::VerificationSet& data);

//    proto::Verification Sign(
//        const std::string& claim,
//        const bool polarity,
//        const int64_t start = 0,
//        const int64_t end = 0,
//        const OTPasswordData* pPWData = nullptr) const;
//    bool Verify(const proto::Verification& item) const;

    // VerificationSet has 2 groups, internal and external.
    // Internal is for your signatures on other people's claims.
    // External is for other people's signatures on your claims.
    // When you find that in the external, you copy it to your own credential.
    // So external is for re-publishing other people's verifications of your claims.

    // If we've repudiated any claims, you can add their IDs to the repudiated field in the verification set.
}
//virtual
void MTContactDetails::refresh(QString strID, QString strName)
{
    if ((NULL == ui) || strID.isEmpty())
    {
        ui->pushButtonMsg->setEnabled(false);
        ui->pushButtonPay->setEnabled(false);
        ui->pushButtonMsg->setProperty("contactid", 0);
        ui->pushButtonPay->setProperty("contactid", 0);

        if (m_pPlainTextEdit)
            m_pPlainTextEdit->setPlainText("");

        if (m_pPlainTextEditNotes)
            m_pPlainTextEditNotes->setPlainText("");

        if (treeWidgetClaims_)
            ClearTree();

        return;
    }
    // -----------------------------
    QWidget * pHeaderWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, strID, strName, "", "", ":/icons/icons/rolodex_small", false);

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
    ui->lineEditID  ->setText(strID);
    ui->lineEditName->setText(strName);
    // --------------------------------------------
    QLayout   * pLayout    = nullptr;
    // ----------------------------------
    QString     strDetails;
    QStringList qstrlistNymIDs;
    // ------------------------------------------
    int nContactID = strID.toInt();
    // ------------------------------------------
    if (nContactID > 0)
    {
        ui->pushButtonMsg->setProperty("contactid", nContactID);
        ui->pushButtonPay->setProperty("contactid", nContactID);
        ui->pushButtonMsg->setEnabled(true);
        ui->pushButtonPay->setEnabled(true);
    }
    else
    {
        ui->pushButtonMsg->setProperty("contactid", 0);
        ui->pushButtonMsg->setEnabled(false);
        ui->pushButtonPay->setProperty("contactid", 0);
        ui->pushButtonPay->setEnabled(false);

        if (m_pPlainTextEdit)
            m_pPlainTextEdit->setPlainText("");

        if (m_pPlainTextEditNotes)
            m_pPlainTextEditNotes->setPlainText("");

        if (treeWidgetClaims_)
            ClearTree();
    }
    // ------------------------------------------
    {
        mapIDName theNymMap;

        if (MTContactHandler::getInstance()->GetNyms(theNymMap, nContactID))
        {
            strDetails += tr("Nyms:\n");

            for (mapIDName::iterator ii = theNymMap.begin(); ii != theNymMap.end(); ii++)
            {
                QString qstrNymID    = ii.key();
                QString qstrNymValue = ii.value();
                // -------------------------------------
                qstrlistNymIDs.append(qstrNymID);
                // -------------------------------------
                strDetails += QString("%1\n").arg(qstrNymID);
                // -------------------------------------
                mapIDName theServerMap;

                if (MTContactHandler::getInstance()->GetServers(theServerMap, qstrNymID))
                {
                    strDetails += tr("Found on servers:\n");

                    for (mapIDName::iterator iii = theServerMap.begin(); iii != theServerMap.end(); iii++)
                    {
                        QString qstrNotaryID    = iii.key();
                        QString qstrServerValue = iii.value();
                        // -------------------------------------
                        strDetails += QString("%1\n").arg(qstrNotaryID);
                        // -------------------------------------
                        mapIDName theAccountMap;

                        if (MTContactHandler::getInstance()->GetAccounts(theAccountMap, qstrNymID, qstrNotaryID, QString("")))
                        {
                            strDetails += tr("Where he owns the accounts:\n");

                            for (mapIDName::iterator iiii = theAccountMap.begin(); iiii != theAccountMap.end(); iiii++)
                            {
                                QString qstrAcctID    = iiii.key();
                                QString qstrAcctValue = iiii.value();
                                // -------------------------------------
                                strDetails += QString("%1\n").arg(qstrAcctID);
                                // -------------------------------------
                            } // for (accounts)
                        } // got accounts
                    } // for (servers)
                    strDetails += QString("\n");
                } // got servers
            } // for (nyms)
            strDetails += QString("\n");
        } // got nyms
    }
    // --------------------------------------------
    // TAB: "Profile"
    //
    RefreshTree(nContactID, qstrlistNymIDs);
    // --------------------------------------------
    // TAB: "Known IDs"
    //
    if (m_pPlainTextEdit)
        m_pPlainTextEdit->setPlainText(strDetails);
    // --------------------------------------------
    // TAB: "Notes"
    //
//    if (m_pPlainTextEditNotes)  //todo
//        m_pPlainTextEditNotes->setPlainText(strDetails);
    // -----------------------------------
    // TAB: "CREDENTIALS"
    //
    if (m_pCredentials)
        m_pCredentials->refresh(qstrlistNymIDs);
    // -----------------------------------------------------------------------
}


// Add a "delete contact" function that the owner dialog can use.
// It will use this to delete the contact from the SQL db:
//
//         bool MTContactHandler::DeleteContact(int nContactID);

void MTContactDetails::on_lineEditName_editingFinished()
{
    int nContactID = m_pOwner->m_qstrCurrentID.toInt();

    if (nContactID > 0)
    {
        bool bSuccess = MTContactHandler::getInstance()->SetContactName(nContactID, ui->lineEditName->text());

        if (bSuccess)
        {
            m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
            m_pOwner->m_map.insert(m_pOwner->m_qstrCurrentID, ui->lineEditName->text());

            m_pOwner->SetPreSelected(m_pOwner->m_qstrCurrentID);
            // ------------------------------------------------
            emit RefreshRecordsAndUpdateMenu();
            // ------------------------------------------------
        }
    }
}


