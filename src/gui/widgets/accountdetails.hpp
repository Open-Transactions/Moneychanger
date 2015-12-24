#ifndef ACCOUNTDETAILS_HPP
#define ACCOUNTDETAILS_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <gui/widgets/editdetails.hpp>


namespace Ui {
class MTAccountDetails;
}

class MTCashPurse;

class AccountRecordsProxyModel;
class ModelPayments;
class QTableView;
class QMenu;

class MTAccountDetails : public MTEditDetails
{
    Q_OBJECT
    
public:
    explicit MTAccountDetails(QWidget *parent, MTDetailEdit & theOwner);
    ~MTAccountDetails();
    
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

private:
    QPointer<QWidget>     m_pHeaderWidget;
    QPointer<MTCashPurse> m_pCashPurse; // Tab 3.

public slots:

signals:
    void showDashboard();
    void showContact(QString);
    void showContactAndRefreshHome(QString);
    void showPayment(int, int);

private slots:
    void on_lineEditName_editingFinished();
    void on_toolButtonAsset_clicked();
    void on_toolButtonNym_clicked();
    void on_toolButtonServer_clicked();
    void on_pushButtonSend_clicked();
    void on_pushButtonRequest_clicked();
    void on_pushButtonMakeDefault_clicked();

    void on_tableViewSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous);
    void on_tableView_doubleClicked(const QModelIndex &index);
    void on_tableView_customContextMenuRequested(const QPoint &pos);

    void on_toolButtonReply_clicked();
    void on_toolButtonForward_clicked();
    void on_toolButtonDelete_clicked();

    void on_MarkAsRead_timer();
    void on_MarkAsUnread_timer();
    void on_MarkAsReplied_timer();
    void on_MarkAsForwarded_timer();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void FavorLeftSideForIDs();

    void AcceptIncoming     (QPointer<ModelPayments> & pModel, AccountRecordsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView);
    void CancelOutgoing     (QPointer<ModelPayments> & pModel, AccountRecordsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView);
    void DiscardOutgoingCash(QPointer<ModelPayments> & pModel, AccountRecordsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView);
    void DiscardIncoming    (QPointer<ModelPayments> & pModel, AccountRecordsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView);

private:
    QString m_qstrID;
    Ui::MTAccountDetails *ui;

    QPointer<AccountRecordsProxyModel> pPmntProxyModel_;

    QScopedPointer<QMenu> popupMenu_;

    QAction * pActionViewInPayments      = nullptr;
    QAction * pActionOpenNewWindow       = nullptr;
    QAction * pActionReply               = nullptr;
    QAction * pActionForward             = nullptr;
    QAction * pActionDelete              = nullptr;
    QAction * pActionMarkRead            = nullptr;
    QAction * pActionMarkUnread          = nullptr;
    QAction * pActionViewContact         = nullptr;
    QAction * pActionCreateContact       = nullptr;
    QAction * pActionExistingContact     = nullptr;
    QAction * pActionAcceptIncoming      = nullptr;
    QAction * pActionCancelOutgoing      = nullptr;
    QAction * pActionDiscardOutgoingCash = nullptr;
    QAction * pActionDiscardIncoming     = nullptr;

    QList<QModelIndex> listRecordsToMarkAsRead_;
    QList<QModelIndex> listRecordsToMarkAsUnread_;

    QList<QModelIndex> listRecordsToMarkAsReplied_;
    QList<QModelIndex> listRecordsToMarkAsForwarded_;
};

#endif // ACCOUNTDETAILS_HPP
