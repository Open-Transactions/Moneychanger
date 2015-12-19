#ifndef PAGENYM_ALTLOCATION_HPP
#define PAGENYM_ALTLOCATION_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <gui/widgets/groupboxcontactitems.hpp>

#include <QWizardPage>
#include <QPointer>
#include <QGroupBox>
#include <QTabWidget>
#include <QList>
#include <QMap>
#include <QWidget>

#include <string>
#include <tuple>
#include <list>

typedef std::tuple<uint32_t, uint32_t, std::string, bool> tupleContactDataItem;
typedef std::list<tupleContactDataItem> listContactDataTuples;

namespace Ui {
class MTPageNym_AltLocation;
}

class MTPageNym_AltLocation : public QWizardPage
{
    Q_OBJECT

public:
    explicit MTPageNym_AltLocation(QWidget *parent = 0);
    ~MTPageNym_AltLocation();

    void initializePage(); //virtual

    void SetFieldsBlank();

protected:
    QWidget * createSingleContactItem(GroupBoxContactItems * pGroupBox, int nComboIndex=0, const QString textValue="");
    bool      deleteSingleContactItem(GroupBoxContactItems * pGroupBox, QWidget * pWidget);
    bool      moveSingleContactItem(  GroupBoxContactItems * pGroupBoxOld,
                                      GroupBoxContactItems * pGroupBoxNew, QWidget * pWidget,
                                      int nComboIndex, const QString & textValue);

    // This function wipes the output data, then iterates through
    // the data items we have, and re-builds the output data based
    // on those.
    void PrepareOutputData();

private slots:
    void on_btnContactItemDelete_clicked();
    void on_lineEditItemValue_textChanged(const QString & text);
    void on_comboBox_currentIndexChanged(int nIndex);
    void on_btnPrimary_toggled(bool checked);

private:
    Ui::MTPageNym_AltLocation *ui;

    QList<QWidget *>  listTabs_;
    QMap<uint32_t, QList<GroupBoxContactItems *> * > mapGroupBoxLists_;
};

#endif // PAGENYM_ALTLOCATION_HPP
