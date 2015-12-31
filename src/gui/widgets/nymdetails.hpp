#ifndef NYMDETAILS_HPP
#define NYMDETAILS_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <gui/widgets/editdetails.hpp>

#include <QScopedPointer>

namespace Ui {
class MTNymDetails;
}

class QGroupBox;
class QTableWidget;
class QMenu;
class QAction;
class QLineEdit;
class QToolButton;
class QLabel;
class QPlainTextEdit;
class MTCredentials;
class QTreeWidget;

class MTNymDetails : public MTEditDetails
{
    Q_OBJECT
    
public:
    explicit MTNymDetails(QWidget *parent, MTDetailEdit & theOwner);
    ~MTNymDetails();
    
    virtual void refresh(QString strID, QString strName);
    virtual void AddButtonClicked();
    virtual void DeleteButtonClicked();
    // ----------------------------------
    // Only used on construction (aka when dialog() is called for first time.)
    //
    virtual int       GetCustomTabCount();
    virtual QWidget * CreateCustomTab (int nTab);
    virtual QString   GetCustomTabName(int nTab);
    // ----------------------------------
    virtual void ClearContents();

    void ClearTree();
    void RefreshTree(QStringList & qstrlistNymIDs);

signals:
    void nymWasJustChecked(QString);

public slots:
    void onClaimsUpdatedForNym(QString nymId);

private:
    QPointer<QPlainTextEdit> m_pPlainTextEdit;
    QPointer<MTCredentials>  m_pCredentials;
    QPointer<QTreeWidget>    treeWidgetClaims_;

    QPointer<QLabel>       pLabelNymId_;
    QPointer<QLineEdit>    pLineEditNymId_;
    QPointer<QToolButton>  pToolButtonNymId_;
    QPointer<QLabel>       pLabelNotaries_;
    QPointer<QTableWidget> pTableWidgetNotaries_;

    QScopedPointer<QMenu> popupMenu_;

    QAction * pActionRegister_;
    QAction * pActionUnregister_;

private slots:
    void on_lineEditName_editingFinished();
    void on_toolButton_clicked();

    void on_btnAddressAdd_clicked();
    void on_btnAddressDelete_clicked();

    void on_tableWidget_customContextMenuRequested(const QPoint &pos);
    void on_toolButtonDescription_clicked();
    void on_toolButtonQrCode_clicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void FavorLeftSideForIDs();
    void clearNotaryTable();

    QGroupBox * createAddressGroupBox    (QString strNymID);
    QWidget   * createSingleAddressWidget(QString strNymID, int nMethodID, QString qstrAddress, QString qstrDisplayAddr);
    QWidget   * createNewAddressWidget   (QString strNymID);

private:
    QPointer<QWidget>   m_pHeaderWidget;
    QPointer<QGroupBox> m_pAddresses;

    Ui::MTNymDetails *ui;
};

#endif // NYMDETAILS_HPP
