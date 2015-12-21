#ifndef GROUPBOXCONTACTITEMS_HPP
#define GROUPBOXCONTACTITEMS_HPP

#include <QGroupBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QMap>

namespace Ui {
class GroupBoxContactItems;
}

class GroupBoxContactItems : public QGroupBox
{
    Q_OBJECT

public:
    explicit GroupBoxContactItems(QWidget *parent = 0);
    ~GroupBoxContactItems();

    void addRadioButton(QRadioButton * pButton);
    void removeRadioButton(QRadioButton * pButton);

protected:
    void showRadioButtons(bool bShow);

private:
    Ui::GroupBoxContactItems *ui;

public:
    uint32_t indexSection_=0;
    uint32_t indexSectionType_=0;
    QMap<uint32_t, QString> mapTypeNames_;

    QList<GroupBoxContactItems *> * pListGroupBoxes_=nullptr;

    QButtonGroup radioBtnGroup_;
};

#endif // GROUPBOXCONTACTITEMS_HPP
