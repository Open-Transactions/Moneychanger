#ifndef EDITDETAILS_H
#define EDITDETAILS_H

#include <QWidget>
#include <QGridLayout>

#include "Handlers/contacthandler.h"

namespace Ui {
class MTEditDetails;
}

class MTDetailEdit;
// -----------------------------------------
class MTEditDetails : public QWidget
{
    Q_OBJECT

public:
    explicit MTEditDetails(QWidget *parent = 0);
    virtual ~MTEditDetails();

    virtual void refresh(QString strID, QString strName)=0;
    virtual void AddButtonClicked()=0;
    virtual void DeleteButtonClicked()=0;

    static QWidget * CreateDetailHeaderWidget(QString strID, QString strName,
                                              QString strAmount=QString(""),
                                              QString strStatus=QString(""),
                                              bool bExternal=true);

    void SetOwnerPointer(MTDetailEdit & theOwner);

    // ----------------------------------
    // Only used on construction (aka when dialog() is called for first time.)
    //
    virtual int       GetCustomTabCount();
    virtual QWidget * CreateCustomTab (int nTab);
    virtual QString   GetCustomTabName(int nTab);
    // ----------------------------------

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
//    void on_contactButton_clicked(bool checked = false);
//    void on_deleteButton_clicked(bool checked = false);
//    void on_acceptButton_clicked(bool checked = false);

protected:
    MTDetailEdit * m_pOwner;

private:
//    Ui::MTEditDetails *ui;
};

#endif // EDITDETAILS_H
