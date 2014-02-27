#ifndef ACCOUNTDETAILS_HPP
#define ACCOUNTDETAILS_HPP

#include <WinsockWrapper.h>
#include <ExportWrapper.h>

#include <gui/widgets/editdetails.hpp>


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

private:
    QPointer<QWidget>     m_pHeaderWidget;
    QPointer<MTCashPurse> m_pCashPurse; // Tab 3.

private slots:
    void on_lineEditName_editingFinished();

    void on_toolButtonAsset_clicked();

    void on_toolButtonNym_clicked();

    void on_toolButtonServer_clicked();

    void on_pushButtonSend_clicked();

    void on_pushButtonRequest_clicked();

    void on_pushButtonMakeDefault_clicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void FavorLeftSideForIDs();

private:
    QString m_qstrID;
    Ui::MTAccountDetails *ui;
};

#endif // ACCOUNTDETAILS_HPP
