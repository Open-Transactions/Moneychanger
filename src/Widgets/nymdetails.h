#ifndef NYMDETAILS_H
#define NYMDETAILS_H

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
    
private slots:
    void on_lineEditName_editingFinished();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void FavorLeftSideForIDs();

private:
    Ui::MTNymDetails *ui;
};

#endif // NYMDETAILS_H
