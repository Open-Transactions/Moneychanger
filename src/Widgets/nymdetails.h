#ifndef NYMDETAILS_H
#define NYMDETAILS_H

#include <QPlainTextEdit>

#include "editdetails.h"

namespace Ui {
class MTNymDetails;
}

class MTNymDetails : public MTEditDetails
{
    Q_OBJECT
    
public:
    explicit MTNymDetails(QWidget *parent, MTDetailEdit & theOwner);
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

    virtual void ClearContents();

private:
    QPlainTextEdit * m_pPlainTextEdit;

private slots:
    void on_lineEditName_editingFinished();

    void on_toolButton_clicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void FavorLeftSideForIDs();

private:
    QWidget * m_pHeaderWidget;

    Ui::MTNymDetails *ui;
};

#endif // NYMDETAILS_H
