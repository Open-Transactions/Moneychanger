#ifndef ASSETDETAILS_HPP
#define ASSETDETAILS_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <gui/widgets/editdetails.hpp>
#include <core/filedownloader.hpp>

namespace Ui {
class MTAssetDetails;
}

class QPlainTextEdit;

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

    // ----------------------------------
    // Only used on construction (aka when dialog() is called for first time.)
    //
    virtual int       GetCustomTabCount();
    virtual QWidget * CreateCustomTab (int nTab);
    virtual QString   GetCustomTabName(int nTab);
    // ----------------------------------

    void ImportContract(QString qstrContents);

private:
    QPointer<QPlainTextEdit> m_pPlainTextEdit;

private slots:
    void on_lineEditName_editingFinished();

    void DownloadedURL();

    void on_pushButton_clicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void FavorLeftSideForIDs();

    QPointer<FileDownloader> m_pDownloader;

private:
    QPointer<QWidget> m_pHeaderWidget;

    Ui::MTAssetDetails *ui;
};

#endif // ASSETDETAILS_HPP
