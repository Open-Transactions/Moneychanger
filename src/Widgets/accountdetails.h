#ifndef ACCOUNTDETAILS_H
#define ACCOUNTDETAILS_H

#include <QWidget>
#include <QLineEdit>
#include <QGridLayout>

#include "editdetails.h"

namespace Ui {
class MTAccountDetails;
}

class MTCashPurse;

class MTAccountDetails : public MTEditDetails
{
    Q_OBJECT
    
public:
    explicit MTAccountDetails(QWidget *parent, MTDetailEdit & theOwner);
    ~MTAccountDetails();
    
    virtual void refresh(QString strID, QString strName);
    virtual void AddButtonClicked();
    virtual void DeleteButtonClicked();

    virtual void ClearContents();
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
    QWidget * m_pHeaderWidget2;

    QGridLayout * m_pGridLayout; // Used for tab 2.

    MTCashPurse * m_pCashPurse; // Tab 3.

private slots:
    void on_lineEditName_editingFinished();

    void on_toolButtonAsset_clicked();

    void on_toolButtonNym_clicked();

    void on_toolButtonServer_clicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void FavorLeftSideForIDs();

private:
    Ui::MTAccountDetails *ui;
};

#endif // ACCOUNTDETAILS_H
