#ifndef SERVERDETAILS_H
#define SERVERDETAILS_H

#include "editdetails.h"

#include "filedownloader.h"

namespace Ui {
class MTServerDetails;
}

class MTServerDetails : public MTEditDetails
{
    Q_OBJECT
    
public:
    explicit MTServerDetails(QWidget *parent, MTDetailEdit & theOwner);
    ~MTServerDetails();

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

private:
    FileDownloader * m_pDownloader;

    QWidget * m_pHeaderWidget;

    Ui::MTServerDetails *ui;
};

#endif // SERVERDETAILS_H
