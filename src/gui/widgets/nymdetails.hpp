#ifndef NYMDETAILS_HPP
#define NYMDETAILS_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <gui/widgets/editdetails.hpp>

#include <QGroupBox>

namespace Ui {
class MTNymDetails;
}

class QPlainTextEdit;
class MTCredentials;

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
    QPointer<QPlainTextEdit> m_pPlainTextEdit;
    QPointer<MTCredentials>  m_pCredentials;

private slots:
    void on_lineEditName_editingFinished();
    void on_toolButton_clicked();

    void on_btnAddressAdd_clicked();
    void on_btnAddressDelete_clicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void FavorLeftSideForIDs();

    QGroupBox * createAddressGroupBox    (QString strNymID);
    QWidget   * createSingleAddressWidget(QString strNymID, int nMethodID, QString qstrAddress, QString qstrDisplayAddr);
    QWidget   * createNewAddressWidget   (QString strNymID);

private:
    QPointer<QWidget>   m_pHeaderWidget;
    QPointer<QGroupBox> m_pAddresses;

    Ui::MTNymDetails *ui;
};

#endif // NYMDETAILS_HPP
