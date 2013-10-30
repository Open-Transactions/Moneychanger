#ifndef ACCOUNTDETAILS_H
#define ACCOUNTDETAILS_H

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

private slots:
    void on_lineEditName_editingFinished();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void FavorLeftSideForIDs();

private:
    Ui::MTAccountDetails *ui;
};

#endif // ACCOUNTDETAILS_H
