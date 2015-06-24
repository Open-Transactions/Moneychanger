#ifndef TRANSPORTDETAILS_H
#define TRANSPORTDETAILS_H

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <gui/widgets/editdetails.hpp>

namespace Ui {
class TransportDetails;
}

class TransportDetails : public MTEditDetails
{
    Q_OBJECT

public:
    explicit TransportDetails(QWidget *parent, MTDetailEdit & theOwner);
    ~TransportDetails();

    virtual void refresh(QString strID, QString strName);
    virtual void AddButtonClicked();
    virtual void DeleteButtonClicked();
    
    virtual void ClearContents();
    
private slots:
    void on_lineEditName_editingFinished();
    void on_lineEditConnect_editingFinished();

private:
    QWidget * m_pHeaderWidget;
    
    Ui::TransportDetails *ui;
};

#endif // TRANSPORTDETAILS_H
