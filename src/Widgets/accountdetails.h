#ifndef ACCOUNTDETAILS_H
#define ACCOUNTDETAILS_H

#include <QWidget>
#include <QLineEdit>

#include "editdetails.h"

namespace Ui {
class MTAccountDetails;
}

class MTAccountDetails : public MTEditDetails
{
    Q_OBJECT
    
public:
    explicit MTAccountDetails(QWidget *parent = 0);
    ~MTAccountDetails();
    
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
    // Tab page 2:
    //
    QLineEdit * m_pLineEdit_Acct_ID;
    QLineEdit * m_pLineEdit_Nym_ID;
    QLineEdit * m_pLineEdit_Server_ID;
    QLineEdit * m_pLineEdit_AssetType_ID;

    QLineEdit * m_pLineEdit_Acct_Name;
    QLineEdit * m_pLineEdit_Nym_Name;
    QLineEdit * m_pLineEdit_Server_Name;
    QLineEdit * m_pLineEdit_AssetType_Name;

private:
    QWidget * m_pHeaderWidget;

private slots:
    void on_lineEditName_editingFinished();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void FavorLeftSideForIDs();

private:
    Ui::MTAccountDetails *ui;
};

#endif // ACCOUNTDETAILS_H
