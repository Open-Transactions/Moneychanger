#ifndef SERVERDETAILS_HPP
#define SERVERDETAILS_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <gui/widgets/editdetails.hpp>
#include <core/filedownloader.hpp>


namespace Ui {
class MTServerDetails;
}

class QPlainTextEdit;

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

    void on_toolButtonNotary_clicked();

    void on_toolButtonNym_clicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void FavorLeftSideForIDs();

private:
    QPointer<FileDownloader> m_pDownloader;
    QPointer<QWidget>        m_pHeaderWidget;

    Ui::MTServerDetails *ui;
};

#endif // SERVERDETAILS_HPP
