#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/pagenym_altlocation.hpp>
#include <ui_pagenym_altlocation.h>

#include <gui/widgets/editdetails.hpp>
#include <gui/widgets/wizardaddnym.hpp>

#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVariant>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QDebug>

#include <opentxs/client/OTAPI.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>
#include <opentxs/client/OpenTransactions.hpp>

#include <set>


// Triggers whenever a contact Item is deleted on this wizard page.
void MTPageNym_AltLocation::on_btnContactItemDelete_clicked()
{
    QPushButton * pBtnDelete  = qobject_cast<QPushButton*>(sender());

    if (nullptr != pBtnDelete)
    {
        QWidget              * pItemWidget = VPtr<QWidget>::asPtr(pBtnDelete->property("contactitemwidget"));
        GroupBoxContactItems * pGroupBox   = nullptr;

        if (nullptr == pItemWidget) // Should never happen.
        {
            return;
        }
        // ----------------------------------------------------------
        pGroupBox = VPtr<GroupBoxContactItems>::asPtr(pItemWidget->property("groupbox"));

        if (nullptr == pGroupBox) // Should never happen. Could assert here.
        {
            return;
        }
        // ----------------------------------------------------------
        // const bool bDeleted =
        deleteSingleContactItem(pGroupBox, pItemWidget); // This function handles deleting the data.
        // ----------------------------------------------------------
        // Related task:
        // If that was the only one left on this tab, then we need to create a new
        // blank one.
        //
        QList<GroupBoxContactItems *> * pListGroupBoxes = pGroupBox->pListGroupBoxes_;

        if (nullptr == pListGroupBoxes) // should never be null. Could assert here.
        {
            return;
        }
        // -------------------------------------
        // Iterate the list and see if all of the groupboxes are all empty of contact data items.
        // And if so, create a blank one, so there's always at least one.
        //
        bool bAreAllEmpty = true;

        for (QList<GroupBoxContactItems *>::iterator it_boxes = pListGroupBoxes->begin();
             it_boxes != pListGroupBoxes->end();
             ++it_boxes)
        {
            GroupBoxContactItems * pCurrentGroupBox = *it_boxes;

            if (nullptr != pCurrentGroupBox) // should never be null.
            {
                if (pCurrentGroupBox->layout()->count() > 0)
                {
                    bAreAllEmpty = false;
                    break;
                }
            }
        }
        // -------------------------------------
        if (bAreAllEmpty)
        {
            QList<GroupBoxContactItems *>::iterator it_boxes = pListGroupBoxes->begin();

            if (it_boxes != pListGroupBoxes->end()) // Should never equal end() here.
            {
                GroupBoxContactItems * pFirstGroup = *it_boxes;

                QWidget * pNewItem = createSingleContactItem(pFirstGroup);

                if (nullptr != pNewItem)
                {
                    pFirstGroup->layout()->addWidget(pNewItem);
                }
            }
        }
    }
}


void MTPageNym_AltLocation::PrepareOutputData()
{
    // Step 1: wipe the old data.
    //
    static_cast<MTWizardAddNym *>(wizard())->listContactDataTuples_.clear();

    // Step 2: reconstruct it from the widgets.
    QMap<uint32_t, QList<GroupBoxContactItems *> * >::iterator it_mapGroupBoxLists;

    for (it_mapGroupBoxLists  = mapGroupBoxLists_.begin();
         it_mapGroupBoxLists != mapGroupBoxLists_.end();
         it_mapGroupBoxLists++)
    {
        const uint32_t indexSection = it_mapGroupBoxLists.key();
        QList<GroupBoxContactItems *> * pList = it_mapGroupBoxLists.value();

        if (nullptr != pList)
        {
            QList<GroupBoxContactItems *>::iterator it_list;

            for (it_list  = pList->begin();
                 it_list != pList->end();
                 it_list++)
            {
                GroupBoxContactItems * pGroupBox = *it_list;

                if (nullptr != pGroupBox)
                {
                    uint32_t indexSectionType = pGroupBox->indexSectionType_;

                    for (int nItemIndex = 0; nItemIndex < pGroupBox->layout()->count(); ++nItemIndex)
                    {
                        QWidget * pItemWidget = pGroupBox->layout()->itemAt(nItemIndex)->widget();
                        QLineEdit * pLineEdit = VPtr<QLineEdit>::asPtr(pItemWidget->property("lineedit"));
                        QRadioButton * pBtnRadio = VPtr<QRadioButton>::asPtr(pItemWidget->property("radiobtn"));

                        const QString & qstrText = pLineEdit->text();

                        if (!qstrText.isEmpty())
                            static_cast<MTWizardAddNym *>(wizard())->listContactDataTuples_.push_back(
                                    std::make_tuple(indexSection, indexSectionType,
                                        qstrText.toStdString(),
                                        pBtnRadio->isChecked()));
                    }
                }
            }
        }
    }
}

void MTPageNym_AltLocation::on_comboBox_currentIndexChanged(int nIndex)
{
    QComboBox * pComboBox = qobject_cast<QComboBox*>(sender());

    if (nullptr != pComboBox)
    {
        // Note: the internal data representation is updated at the
        // bottom, in the call to moveSingleContactItem, which calls
        // deleteSingleContractItem, which updates the data. (So no
        // need to do it in here.)
        // -------------------------------------------------------------
        // ALSO: MOVE the contact data item widget from the current group box,
        // to a different group box, since after all, its type has just changed.
        // (We have a different group box for each type.)

        uint32_t newSectionType = 0;

        QWidget              * pItemWidget  = VPtr<QWidget>::asPtr(pComboBox->property("contactitemwidget"));
        QLineEdit            * pLineEdit    = nullptr;
        GroupBoxContactItems * pGroupBoxOld = nullptr;
        GroupBoxContactItems * pGroupBoxNew = nullptr;

        if (nullptr == pItemWidget)
        {
            return;
        }
        // ----------------------------------------------------------
        pLineEdit = VPtr<QLineEdit>::asPtr(pItemWidget->property("lineedit"));

        if (nullptr == pLineEdit)
        {
            return;
        }
        // ----------------------------------------------------------
        pGroupBoxOld = VPtr<GroupBoxContactItems>::asPtr(pItemWidget->property("groupbox"));

        if (nullptr == pGroupBoxOld)
        {
            return;
        }
        // ----------------------------------------------------------
        // Let's get the section type of the NEW group box
        // (the one that was just selected in the combo box.)
        //
        QVariant qvarSectionType = pComboBox->itemData(nIndex);

        if (qvarSectionType.isValid()) // Always should be valid. We could assert here.
        {
            newSectionType = qvarSectionType.toUInt();
        }
        // -------------------------------------
        if (0 == newSectionType)
        {
            return;
        }
        // -------------------------------------
        // Both the old AND the new groupbox are both found on this list.
        QList<GroupBoxContactItems *> * pListGroupBoxes = pGroupBoxOld->pListGroupBoxes_;

        if (nullptr == pListGroupBoxes) // should never be null. Could assert here.
        {
            return;
        }
        // -------------------------------------
        // Iterate the list and find the groupbox that contains the new section type
        // we're looking for.
        //
        for (QList<GroupBoxContactItems *>::iterator it_boxes = pListGroupBoxes->begin();
             it_boxes != pListGroupBoxes->end();
             ++it_boxes)
        {
            GroupBoxContactItems * pGroupBox = *it_boxes;

            if (nullptr != pGroupBox) // should never be null.
            {
                if (pGroupBox->indexSectionType_ == newSectionType) // Found it!
                {
                    pGroupBoxNew = pGroupBox;
                    break;
                }
            }
        }
        // -------------------------------------
        if (nullptr == pGroupBoxNew)
        {
            return;
        }
        // -------------------------------------
        // By this point we know for sure that both group boxes, as well as the
        // contact data item widget, are all NOT null.
        // So now we can perform the move:
        //
        if (!moveSingleContactItem(pGroupBoxOld, pGroupBoxNew, pItemWidget, nIndex, pLineEdit->text()))
        {
            // qDebug() << "Failure...";
            return;
        }
    }
}

void MTPageNym_AltLocation::SetFieldsBlank()
{

    // TODO


//    setField("NymID",   "");
//    setField("NymName", QString("<%1>").arg(tr("Click to choose Nym")));


    //QList<QWidget *>  listTabs_;
    //QMap<int, QList<GroupBoxContactItems *> > mapGroupBoxLists_;

}


void MTPageNym_AltLocation::on_lineEditItemValue_textChanged(const QString & text)
{
    QLineEdit* edit = qobject_cast<QLineEdit*>(sender());

    if (edit)
    {
        // The data has just changed.

        // Update the internal data representation so the caller who
        // instantiated the wizard can get to it after the wizard is
        // closed.
        //
        PrepareOutputData();
        // ----------------------------------------------------------
        // Make sure there is at least one empty item in this group.
        //
        QWidget * pContactItemWidget = VPtr<QWidget>::asPtr(edit->property("contactitemwidget"));

        if (nullptr == pContactItemWidget)
        {
            return;
        }
        // ----------------------------------------------------------
        QComboBox * pComboBox = VPtr<QComboBox>::asPtr(pContactItemWidget->property("combo"));

        if (nullptr == pComboBox)
        {
            return;
        }
        // ----------------------------------------------------------
        GroupBoxContactItems * pGroupBox = VPtr<GroupBoxContactItems>::asPtr(pContactItemWidget->property("groupbox"));

        if (nullptr == pGroupBox)
        {
            return;
        }

        QLayout * pGroupBoxLayout = pGroupBox->layout();
        // ----------------------------------------------------------
        bool bAtLeastOneIsEmpty = false;

        for (int index_widget = 0; index_widget < pGroupBoxLayout->count(); ++index_widget)
        {
            QWidget *pItemWidget = pGroupBoxLayout->itemAt(index_widget)->widget();
            if (nullptr != pItemWidget)
            {
                const int nIndexLineEdit = 1; // todo hardcoding.
                QLayout * pItemLayout = pItemWidget->layout(); // Combo, Line Edit, Button
                QWidget *pLineEditWidget = pItemLayout->itemAt(nIndexLineEdit)->widget();

                if (nullptr != pLineEditWidget)
                {
                    QLineEdit * pLineEdit = dynamic_cast<QLineEdit *>(pLineEditWidget);
                    if (nullptr != pLineEdit)
                    {
                        const QString & qstrText = pLineEdit->text();

                        if (qstrText.isEmpty())
                        {
                            bAtLeastOneIsEmpty = true;
                            break;
                        }
                    }
                }
            }
        } // item widgets on group box.
        // -----------------------------------
        if (!bAtLeastOneIsEmpty)
        {
            QWidget * pNewItem = createSingleContactItem(pGroupBox, pComboBox->currentIndex());

            if (nullptr != pNewItem)
            {
                pGroupBoxLayout->addWidget(pNewItem);
            }
        }
    }
}


bool MTPageNym_AltLocation::moveSingleContactItem(GroupBoxContactItems * pGroupBoxOld,
                                                  GroupBoxContactItems * pGroupBoxNew, QWidget * pWidget,
                                                  int nComboIndex, const QString & textValue)
{
    if (nullptr == pGroupBoxOld || nullptr == pGroupBoxNew || nullptr == pWidget)
        return false;

    QWidget * pNewWidget = createSingleContactItem(pGroupBoxNew, nComboIndex, textValue);

    if (nullptr == pNewWidget)
        return false;
    // --------------------------------------------
    pGroupBoxNew->layout()->addWidget(pNewWidget);
    // --------------------------------------------
    return deleteSingleContactItem(pGroupBoxOld, pWidget);
}

bool MTPageNym_AltLocation::deleteSingleContactItem(GroupBoxContactItems * pGroupBox, QWidget * pWidget)
{
    if (nullptr == pGroupBox || nullptr == pWidget)
        return false;

    // We have to remove the radio button from the buttongroup.
    QLayout * pItemLayout = pWidget->layout(); // Combo, Line Edit, Radio, Delete

    if (nullptr != pItemLayout)
    {
        const int nIndexRadioBtn  = 2; // todo hardcoding.
        QWidget * pRadioBtnWidget = pItemLayout->itemAt(nIndexRadioBtn)->widget();

        if (nullptr != pRadioBtnWidget)
        {
            QRadioButton * pRadioBtn = dynamic_cast<QRadioButton *>(pRadioBtnWidget);
            if (nullptr != pRadioBtn)
            {
                pGroupBox->removeRadioButton(pRadioBtn);
            }
        }
    }
    // --------------------------------------
    pGroupBox->layout()->removeWidget(pWidget);

    pWidget->setParent(nullptr);
    pWidget->disconnect();
    pWidget->deleteLater();

    PrepareOutputData();

    return true;
}

QWidget * MTPageNym_AltLocation::createSingleContactItem(GroupBoxContactItems * pGroupBox, int nComboIndex/*=0*/, const QString textValue/*=""*/)
{
    QWidget      * pWidgetContactItem = new QWidget;
    // ----------------------------------------------------------
    QComboBox    * pComboType = new QComboBox(pWidgetContactItem);
    QLineEdit    * pLineEditItemValue = new QLineEdit(pWidgetContactItem);
    QPushButton  * pBtnDelete = new QPushButton(tr("Delete"), pWidgetContactItem);
    QRadioButton * pBtnRadio = new QRadioButton(tr("Primary"), pWidgetContactItem);

    pGroupBox->addRadioButton(pBtnRadio);
    // ----------------------------------------------------------
    pComboType->setMinimumWidth(60);
    pComboType->setMinimumHeight(25);
    pComboType->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    pBtnDelete->setMinimumHeight(25);
    pBtnDelete->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    pLineEditItemValue->setMinimumWidth(55);
    pLineEditItemValue->setMinimumHeight(25);
    pLineEditItemValue->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // ----------------------------------------------------------
    pWidgetContactItem->setProperty("groupbox",          VPtr<GroupBoxContactItems>::asQVariant(pGroupBox));
    pWidgetContactItem->setProperty("combo",             VPtr<QComboBox>::asQVariant(pComboType));
    pWidgetContactItem->setProperty("lineedit",          VPtr<QLineEdit>::asQVariant(pLineEditItemValue));
    pWidgetContactItem->setProperty("deletebtn",         VPtr<QPushButton>::asQVariant(pBtnDelete));
    pWidgetContactItem->setProperty("radiobtn",          VPtr<QRadioButton>::asQVariant(pBtnRadio));
    // ----------------------------------------------------------
    pComboType        ->setProperty("contactitemwidget", VPtr<QWidget>::asQVariant(pWidgetContactItem));
    pLineEditItemValue->setProperty("contactitemwidget", VPtr<QWidget>::asQVariant(pWidgetContactItem));
    pBtnDelete        ->setProperty("contactitemwidget", VPtr<QWidget>::asQVariant(pWidgetContactItem));
    pBtnRadio         ->setProperty("contactitemwidget", VPtr<QWidget>::asQVariant(pWidgetContactItem));
    // ---------------------------------------------------------
//  pBtnDelete->setMinimumWidth(60);
//  pBtnDelete->setProperty("contactSection",     contactSection);
//  pBtnDelete->setProperty("contactSectionType", contactSectionType);
    // ----------------------------------------------------------
    for (QMap<uint32_t, QString>::iterator it_types = pGroupBox->mapTypeNames_.begin();
         it_types != pGroupBox->mapTypeNames_.end();
         ++it_types)
    {
        const uint32_t  & key   = it_types.key();       // section type ID
        const QString   & value = it_types.value();     // section type name

        pComboType->addItem(value, QVariant::fromValue(key));
    }
    // ----------------------------------------------------------
    // We don't set these values until here, underneath the above loop.
    // After all, you can't set the combo box to a certain current index
    // if you haven't even populated it yet!
    //
    pComboType->setCurrentIndex(nComboIndex);
    pLineEditItemValue->setText(textValue);
    // ----------------------------------------------------------
    QHBoxLayout *layout = new QHBoxLayout(pWidgetContactItem);

    layout->setMargin(0);

    layout->addWidget(pComboType);
    layout->addWidget(pLineEditItemValue);
    layout->addWidget(pBtnRadio);
    layout->addWidget(pBtnDelete);
    // ----------------------------------------------------------
    pWidgetContactItem->setLayout(layout);

    connect(pComboType, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBox_currentIndexChanged(int)) );
    connect(pBtnDelete, SIGNAL(clicked()), this, SLOT(on_btnContactItemDelete_clicked()));
    connect(pLineEditItemValue, SIGNAL(textChanged(QString)), this, SLOT(on_lineEditItemValue_textChanged(QString)));
    connect(pBtnRadio, SIGNAL(toggled(bool)), this, SLOT(on_btnPrimary_toggled(bool)));
    // ----------------------------------------------------------
//    layout->setStretch(0,  0);
//    layout->setStretch(1, -1);
//    layout->setStretch(2,  0);
    // ----------------------------------------------------------
    return pWidgetContactItem;
}

void MTPageNym_AltLocation::on_btnPrimary_toggled(bool checked)
{
    PrepareOutputData();
}

/*
EXPORT proto::ContactData GetContactData(const Nym& fromNym) const;
EXPORT bool SetContactData(Nym& onNym,
                           const proto::ContactData&) const;
EXPORT std::set<uint32_t> GetContactSections (const uint32_t version = 1);
EXPORT std::set<uint32_t> GetContactSectionTypes (const uint32_t section, const uint32_t version = 1);
EXPORT std::string GetContactSectionName (const uint32_t section, std::string lang = "en");
EXPORT std::string GetContactTypeName (const uint32_t type, std::string lang = "en");
EXPORT std::string GetContactAttributeName (const uint32_t type, std::string lang = "en");
*/

MTPageNym_AltLocation::MTPageNym_AltLocation(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::MTPageNym_AltLocation)
{
    ui->setupUi(this);

//    this->registerField("Location", ui->lineEditLocation);

}

void MTPageNym_AltLocation::initializePage() //virtual
{
    //QList<QWidget *>  listTabs_;

    int nCurrentTab = 0;

    std::set<uint32_t> sections = opentxs::OTAPI_Wrap::OTAPI()->GetContactSections();

    for (auto & indexSection: sections)  //Names (for example)
    {
        nCurrentTab++;
        bool bAddedInitialItem = false;

        QMap<uint32_t, QString> mapTypeNames;
        // ----------------------------------------
        // Create a new (tab page) Widget.
        QWidget * pTab = new QWidget(this);
        std::string sectionName = opentxs::OTAPI_Wrap::OTAPI()->GetContactSectionName (indexSection);

//      pTab->setWindowTitle(QString::fromStdString(sectionName));
        // ----------------------------
        // Setup the widget

        //QMap<int, QList<GroupBoxContactItems *> * > mapGroupBoxLists_;

        QList<GroupBoxContactItems *> * pListGroupBoxes = new QList<GroupBoxContactItems *>;

        std::set<uint32_t> sectionTypes = opentxs::OTAPI_Wrap::OTAPI()->GetContactSectionTypes(indexSection);

        for (auto & indexSectionType: sectionTypes)
        {
            std::string typeName = opentxs::OTAPI_Wrap::OTAPI()->GetContactTypeName(indexSectionType);
            mapTypeNames.insert(indexSectionType, QString::fromStdString(typeName));
        }
        // -------------------------------
        QVBoxLayout * pvBox = new QVBoxLayout(pTab);
        pvBox->setAlignment(Qt::AlignTop);
        pvBox->setMargin(0);
        pTab->setContentsMargins(0,0,0,0);
        // -------------------------------
        int nContactItemsAdded = 0;

        for (auto & indexSectionType: sectionTypes) // BUSINESS Name (for example)
        {
            GroupBoxContactItems   * pGroupBox = new GroupBoxContactItems(pTab);
            QVBoxLayout            * layout    = new QVBoxLayout(pGroupBox);
            // -------------------------------
            pGroupBox->setFlat(true);
            pGroupBox->setContentsMargins(0,0,0,0);

            layout->setMargin(0);

            pGroupBox->indexSection_ = indexSection;
            pGroupBox->indexSectionType_ = indexSectionType;
            pGroupBox->mapTypeNames_ = mapTypeNames;

            if (!bAddedInitialItem &&
                    ( (1 != indexSection) ||     // hardcoded for CONTACTSECTION_NAME
                      (1 == indexSectionType ))  // hardcoded for CITEMTYPE_PERSONAL
                    )
            {
                bAddedInitialItem = true;
                QWidget * pInitialItem = nullptr;
                if (1 == nCurrentTab)
                {
                    // Pre-fill the name on the first tab.
                    QString qstrName = wizard()->field("Name").toString();
                    pInitialItem = createSingleContactItem(pGroupBox, 0, qstrName);
                }
                else
                    pInitialItem = createSingleContactItem(pGroupBox);

                if (nullptr != pInitialItem)
                {
                    layout->addWidget(pInitialItem);
                    nContactItemsAdded++;
                }

                pGroupBox->setLayout(layout);
            }
            // -------------------------------
            pGroupBox->pListGroupBoxes_ = pListGroupBoxes;
            pListGroupBoxes->append(pGroupBox);
            pvBox->addWidget(pGroupBox);

            if (0 == nContactItemsAdded)
                pGroupBox->setVisible(false);
            else
                pGroupBox->setVisible(true);
            // -------------------------------
        }
        pTab->setLayout(pvBox);
        // ----------------------------
        // Add new Widget to TabWidget
        //
        listTabs_.append(pTab);
        mapGroupBoxLists_.insert(indexSection, pListGroupBoxes);
        ui->tabWidget->addTab(pTab, QString::fromStdString(sectionName));
    }
}



MTPageNym_AltLocation::~MTPageNym_AltLocation()
{
    delete ui;
}
