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
class QTreeWidgetItem;

class ClaimsProxyModel;
class ModelClaims;

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
    void RefreshTree(const QString & qstrNymId);

signals:
    void nymWasJustChecked(QString);
    void appendToLog(QString);

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

    QPointer<ModelClaims>      pModelClaims_;
    QPointer<ClaimsProxyModel> pProxyModelClaims_;

    QScopedPointer<QMenu> popupMenu_; // For the grid showing notaries the Nym is registered on.
    QScopedPointer<QMenu> popupMenuProfile_; // For the tree showing the Nym's profile claims/verifications.

    QAction * pActionRegister_ = nullptr;
    QAction * pActionUnregister_ = nullptr;

    QAction * pActionConfirm_ = nullptr;
    QAction * pActionRefute_ = nullptr;
    QAction * pActionNoComment_ = nullptr;

    QTreeWidgetItem * metInPerson_ = nullptr;

private slots:
    void on_lineEditName_editingFinished();
    void on_toolButton_clicked();

    void on_btnAddressAdd_clicked();
    void on_btnAddressDelete_clicked();

    void on_tableWidget_customContextMenuRequested(const QPoint &pos);
    void on_treeWidget_customContextMenuRequested(const QPoint &pos);
    void on_toolButtonDescription_clicked();
    void on_toolButtonQrCode_clicked();

    void on_btnEditProfile_clicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void FavorLeftSideForIDs();
    void clearNotaryTable();

private:
    QPointer<QWidget>   m_pHeaderWidget;
//    QPointer<QGroupBox> m_pAddresses;

    Ui::MTNymDetails *ui;
};

#endif // NYMDETAILS_HPP
