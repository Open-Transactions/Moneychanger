#ifndef PAYMENTS_HPP
#define PAYMENTS_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <QWidget>
#include <QMenu>
#include <QScopedPointer>
#include <QPointer>
#include <QList>
#include <QSqlRecord>

#include <tuple>
#include <map>

// These are used for re-selecting the row when you return
// to a certain filter. Like if you clicked on Alice, then back
// to Bob, the same row would be selected for you that was
// selected before.
typedef std::tuple<int, std::string, std::string> PMNT_TREE_ITEM;
typedef std::map< PMNT_TREE_ITEM, int> mapOfPmntTreeItems;

namespace Ui {
class Payments;
}

class QTreeWidgetItem;
class PaymentsProxyModel;
class ModelPayments;
//class QFrame;

class QTableView;

class Payments : public QWidget
{
    Q_OBJECT

private:
    bool already_init = false;

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void doSearch(QString qstrInput);

    void RefreshAll();
    void ClearTree();
    void RefreshTree();

    void enableButtons();
    void disableButtons();

    PMNT_TREE_ITEM make_tree_item(int nCurrentContact, QString qstrMethodType, QString qstrViaTransport);
    void set_inbox_pmntid_for_tree_item(PMNT_TREE_ITEM & theItem, int nPmntID);
    void set_outbox_pmntid_for_tree_item(PMNT_TREE_ITEM & theItem, int nPmntID);
    int get_inbox_pmntid_for_tree_item(PMNT_TREE_ITEM & theItem); // returns payment ID
    int get_outbox_pmntid_for_tree_item(PMNT_TREE_ITEM & theItem); // returns payment ID

    void tableViewPopupMenu(const QPoint &pos, QTableView * pTableView, PaymentsProxyModel * pProxyModel);
    void tableViewDoubleClicked(const QModelIndex &index, PaymentsProxyModel * pProxyModel);

    void setAsCurrentPayment(int nSourceRow, int nFolder);

public:
    explicit Payments(QWidget *parent = 0);
    ~Payments();

    void dialog(int nSourceRow=-1, int nFolder=-1);

    void RefreshUserBar();
    QWidget * CreateUserBarWidget();

    void AcceptIncoming     (QPointer<ModelPayments> & pModel, PaymentsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView);
    void CancelOutgoing     (QPointer<ModelPayments> & pModel, PaymentsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView);
    void DiscardOutgoingCash(QPointer<ModelPayments> & pModel, PaymentsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView);
    void DiscardIncoming    (QPointer<ModelPayments> & pModel, PaymentsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView);

public slots:
    void onRecordlistPopulated();
    void onBalancesChanged();
    void onNeedToRefreshUserBar();
    void onNeedToRefreshRecords();

signals:
    void showDashboard();
    void needToPopulateRecordlist();
    void needToDownloadAccountData();
    void showContact(QString);
    void showContactAndRefreshHome(QString);

private slots:
    void on_pushButtonSearch_clicked();
    void on_lineEdit_textChanged(const QString &arg1);
    void on_lineEdit_returnPressed();
    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_tableViewSentSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous);
    void on_tableViewReceivedSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous);

    void on_toolButtonReply_clicked();
    void on_toolButtonForward_clicked();
    void on_toolButtonDelete_clicked();
    void on_toolButtonRefresh_clicked();

    void on_tabWidget_currentChanged(int index);

    void on_MarkAsRead_timer();
    void on_MarkAsUnread_timer();
    void on_MarkAsReplied_timer();
    void on_MarkAsForwarded_timer();

    void RefreshPayments();

    void on_tableViewReceived_customContextMenuRequested(const QPoint &pos);
    void on_tableViewSent_customContextMenuRequested(const QPoint &pos);

    void on_tableViewSent_doubleClicked(const QModelIndex &index);
    void on_tableViewReceived_doubleClicked(const QModelIndex &index);

private:
    Ui::Payments *ui;

    QPointer<QWidget>     m_pHeaderFrame;
//  QPointer<QFrame>      m_pHeaderFrame;

    QScopedPointer<QMenu> popupMenu_;

    QAction * pActionDelete              = nullptr;
    QAction * pActionOpenNewWindow       = nullptr;
    QAction * pActionReply               = nullptr;
    QAction * pActionForward             = nullptr;
    QAction * pActionMarkRead            = nullptr;
    QAction * pActionMarkUnread          = nullptr;
    QAction * pActionViewContact         = nullptr;
    QAction * pActionCreateContact       = nullptr;
    QAction * pActionExistingContact     = nullptr;
    QAction * pActionAcceptIncoming      = nullptr;
    QAction * pActionCancelOutgoing      = nullptr;
    QAction * pActionDiscardOutgoingCash = nullptr;
    QAction * pActionDiscardIncoming     = nullptr;

    int nCurrentContact_ = 0;
    QString qstrMethodType_;
    QString qstrViaTransport_;

    QPointer<PaymentsProxyModel> pPmntProxyModelInbox_;
    QPointer<PaymentsProxyModel> pPmntProxyModelOutbox_;

    QTableView         * pCurrentTabTableView_  = nullptr;
    PaymentsProxyModel * pCurrentTabProxyModel_ = nullptr;

    mapOfPmntTreeItems    mapCurrentRows_inbox; // For each tree item, we store a "currently selected" row so we can re-select it when that tree item is clicked.
    mapOfPmntTreeItems    mapCurrentRows_outbox;

    QList<QModelIndex> listRecordsToMarkAsRead_;
    QList<QModelIndex> listRecordsToMarkAsUnread_;

    QList<QModelIndex> listRecordsToMarkAsReplied_;
    QList<QModelIndex> listRecordsToMarkAsForwarded_;
};

#endif // PAYMENTS_HPP



