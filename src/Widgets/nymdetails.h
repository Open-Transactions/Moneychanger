#ifndef NYMDETAILS_H
#define NYMDETAILS_H

#include <QPlainTextEdit.h>

#include "editdetails.h"

namespace Ui {
class MTNymDetails;
}

class MTNymDetails : public MTEditDetails
{
    Q_OBJECT
    
public:
    explicit MTNymDetails(QWidget *parent = 0);
    ~MTNymDetails();
    
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

private:
    QPlainTextEdit * m_pPlainTextEdit;

private slots:
    void on_lineEditName_editingFinished();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void FavorLeftSideForIDs();

private:
    Ui::MTNymDetails *ui;
};

#endif // NYMDETAILS_H
