#ifndef SERVERDETAILS_H
#define SERVERDETAILS_H

#include "editdetails.h"

namespace Ui {
class MTServerDetails;
}

class MTServerDetails : public MTEditDetails
{
    Q_OBJECT
    
public:
    explicit MTServerDetails(QWidget *parent = 0);
    ~MTServerDetails();

    virtual void refresh(QString strID, QString strName);
    virtual void AddButtonClicked();
    virtual void DeleteButtonClicked();

private slots:
    void on_lineEditName_editingFinished();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void FavorLeftSideForIDs();

private:
    Ui::MTServerDetails *ui;
};

#endif // SERVERDETAILS_H
