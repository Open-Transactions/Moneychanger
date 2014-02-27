#ifndef CONTACTDETAILS_HPP
#define CONTACTDETAILS_HPP

#include <WinsockWrapper.h>
#include <ExportWrapper.h>

#include <gui/widgets/editdetails.hpp>


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

private:
    QPointer<QPlainTextEdit> m_pPlainTextEdit;

    QPointer<MTCredentials> m_pCredentials;

private slots:
    void on_lineEditName_editingFinished();

private:
    QPointer<QWidget> m_pHeaderWidget;

    Ui::MTContactDetails *ui;
};

#endif // CONTACTDETAILS_HPP
