#ifndef ASSETDETAILS_H
#define ASSETDETAILS_H

#include "editdetails.h"

namespace Ui {
class MTAssetDetails;
}

class MTAssetDetails : public MTEditDetails
{
    Q_OBJECT
    
public:
    explicit MTAssetDetails(QWidget *parent = 0);
    ~MTAssetDetails();

    virtual void refresh(QString strID, QString strName);
    virtual void AddButtonClicked();
    virtual void DeleteButtonClicked();

private slots:
    void on_lineEditName_editingFinished();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void FavorLeftSideForIDs();

private:
    Ui::MTAssetDetails *ui;
};

#endif // ASSETDETAILS_H
