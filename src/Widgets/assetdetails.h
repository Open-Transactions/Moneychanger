#ifndef ASSETDETAILS_H
#define ASSETDETAILS_H

#include "editdetails.h"

#include "filedownloader.h"

namespace Ui {
class MTAssetDetails;
}

class MTAssetDetails : public MTEditDetails
{
    Q_OBJECT
    
public:
    explicit MTAssetDetails(QWidget *parent, MTDetailEdit & theOwner);
    ~MTAssetDetails();

    virtual void refresh(QString strID, QString strName);
    virtual void AddButtonClicked();
    virtual void DeleteButtonClicked();

    virtual void ClearContents();

    void ImportContract(QString qstrContents);

private slots:
    void on_lineEditName_editingFinished();

    void DownloadedURL();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void FavorLeftSideForIDs();

    FileDownloader * m_pDownloader;

private:
    Ui::MTAssetDetails *ui;
};

#endif // ASSETDETAILS_H
