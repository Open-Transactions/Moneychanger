#ifndef CONTACTDETAILS_H
#define CONTACTDETAILS_H

#include "editdetails.h"

namespace Ui {
class MTContactDetails;
}

class MTContactDetails : public MTEditDetails
{
    Q_OBJECT

public:
    explicit MTContactDetails(QWidget *parent, MTDetailEdit & theOwner);
    virtual ~MTContactDetails();

    virtual void refresh(QString strID, QString strName);
    virtual void AddButtonClicked();
    virtual void DeleteButtonClicked();

    virtual void ClearContents();

private slots:
    void on_lineEditName_editingFinished();

private:
    QWidget * m_pHeaderWidget;

    Ui::MTContactDetails *ui;
};

#endif // CONTACTDETAILS_H
