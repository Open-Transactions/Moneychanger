#ifndef CONTACTDETAILS_H
#define CONTACTDETAILS_H

#include "editdetails.h"

namespace Ui {
class MTContactDetails;
}

class QPlainTextEdit;

class MTContactDetails : public MTEditDetails
{
    Q_OBJECT

public:
    explicit MTContactDetails(QWidget *parent, MTDetailEdit & theOwner);
    virtual ~MTContactDetails();

    virtual void refresh(QString strID, QString strName);
    virtual void AddButtonClicked();
    virtual void DeleteButtonClicked();

    // ----------------------------------
    // Only used on construction (aka when dialog() is called for first time.)
    //
    virtual int       GetCustomTabCount();
    virtual QWidget * CreateCustomTab (int nTab);
    virtual QString   GetCustomTabName(int nTab);
    // ----------------------------------

    virtual void ClearContents();

private:
    QPlainTextEdit * m_pPlainTextEdit;

private slots:
    void on_lineEditName_editingFinished();

private:
    QWidget * m_pHeaderWidget;

    Ui::MTContactDetails *ui;
};

#endif // CONTACTDETAILS_H
