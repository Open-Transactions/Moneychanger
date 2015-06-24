#ifndef CONTACTDETAILS_HPP
#define CONTACTDETAILS_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <gui/widgets/editdetails.hpp>

#include <QGroupBox>

namespace Ui {
class MTContactDetails;
}

class QPlainTextEdit;
class MTCredentials;

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

protected:
    QGroupBox * createAddressGroupBox    (QString strContactID);
    QWidget   * createSingleAddressWidget(int nContactID, QString qstrType, QString qstrTypeDisplay, QString qstrAddress);
    QWidget   * createNewAddressWidget   (int nContactID);

private:
    QPointer<QPlainTextEdit> m_pPlainTextEdit;
    QPointer<MTCredentials> m_pCredentials;

private slots:
    void on_lineEditName_editingFinished();

    void on_btnAddressAdd_clicked();
    void on_btnAddressDelete_clicked();

    void on_pushButtonMsg_clicked();

private:
    QPointer<QWidget>   m_pHeaderWidget;
    QPointer<QGroupBox> m_pAddresses;

    Ui::MTContactDetails *ui;
};

#endif // CONTACTDETAILS_HPP
