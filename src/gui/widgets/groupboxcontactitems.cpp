#include <gui/widgets/groupboxcontactitems.hpp>
#include <ui_groupboxcontactitems.h>

GroupBoxContactItems::GroupBoxContactItems(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::GroupBoxContactItems),
    radioBtnGroup_(this)
{
    ui->setupUi(this);
}

void GroupBoxContactItems::showRadioButtons(bool bShow)
{
    QList<QAbstractButton *> listButtons = radioBtnGroup_.buttons();

    bool bAnyAreChecked = false;

    QAbstractButton * pFirstButton = nullptr;

    foreach (QAbstractButton * pButton, listButtons)
    {
        if (nullptr == pFirstButton)
            pFirstButton = pButton;

        pButton->setVisible(bShow);

        if (pButton->isChecked())
            bAnyAreChecked = true;
    }

    // Make sure at least 1 is checked.
    if (!bAnyAreChecked && nullptr != pFirstButton)
    {
        pFirstButton->toggle();
    }
}

void GroupBoxContactItems::addRadioButton(QRadioButton * pButton)
{
    radioBtnGroup_.addButton(pButton);

    // NOTE: We want to show/hide the radio buttons in this group,
    // based on whether there are more than 1 in the group.

    showRadioButtons(radioBtnGroup_.buttons().size() > 1);
}

void GroupBoxContactItems::removeRadioButton(QRadioButton * pButton)
{
    radioBtnGroup_.removeButton(pButton);

    // NOTE: We want to show/hide the radio buttons in this group,
    // based on whether there are more than 1 in the group.

    showRadioButtons(radioBtnGroup_.buttons().size() > 1);
}

GroupBoxContactItems::~GroupBoxContactItems()
{
    delete ui;
}
