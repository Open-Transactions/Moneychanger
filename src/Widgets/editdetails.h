#ifndef EDITDETAILS_H
#define EDITDETAILS_H

#include <QPointer>
#include <QWidget>
#include <QGridLayout>

#include "Handlers/contacthandler.h"

namespace Ui {
class MTEditDetails;
}

class MTDetailEdit;
class Moneychanger;
// -----------------------------------------
class MTEditDetails : public QWidget
{
    Q_OBJECT

public:
    explicit MTEditDetails(QWidget *parent, MTDetailEdit & theOwner);
    virtual ~MTEditDetails();

    virtual void refresh(QString strID, QString strName)=0;
    virtual void AddButtonClicked()=0;
    virtual void DeleteButtonClicked()=0;

    virtual void ClearContents()=0;

    static QWidget * CreateDetailHeaderWidget(QString strID, QString strName,
                                              QString strAmount=QString(""),
                                              QString strStatus=QString(""),
                                              QString strPixmap=QString(""),
                                              bool bExternal=true);

    void SetOwnerPointer(MTDetailEdit & theOwner);

    // ----------------------------------
    // Only used on construction (aka when dialog() is called for first time.)
    //
    virtual int       GetCustomTabCount();
    virtual QWidget * CreateCustomTab (int nTab);
    virtual QString   GetCustomTabName(int nTab);
    // ----------------------------------

signals:
    void DefaultAccountChanged(QString qstrID, QString qstrName);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
//    void on_contactButton_clicked(bool checked = false);
//    void on_deleteButton_clicked(bool checked = false);
//    void on_acceptButton_clicked(bool checked = false);

protected:    
    QPointer<MTDetailEdit> m_pOwner;
    QPointer<Moneychanger> m_pMoneychanger;

private:
//    Ui::MTEditDetails *ui;
};

#endif // EDITDETAILS_H
