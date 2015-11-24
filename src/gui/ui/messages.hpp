#ifndef MESSAGES_HPP
#define MESSAGES_HPP

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
typedef std::tuple<int, std::string, std::string> MSG_TREE_ITEM;
typedef std::map< MSG_TREE_ITEM, int> mapOfMsgTreeItems;

namespace Ui {
class Messages;
}

class QTreeWidgetItem;
class MessagesProxyModel;

class QTableView;

class Messages : public QWidget
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

    MSG_TREE_ITEM make_tree_item(int nCurrentContact, QString qstrMethodType, QString qstrViaTransport);
    void set_inbox_msgid_for_tree_item(MSG_TREE_ITEM & theItem, int nMsgID);
    void set_outbox_msgid_for_tree_item(MSG_TREE_ITEM & theItem, int nMsgID);
    int get_inbox_msgid_for_tree_item(MSG_TREE_ITEM & theItem); // returns message ID
    int get_outbox_msgid_for_tree_item(MSG_TREE_ITEM & theItem); // returns message ID

public:
    explicit Messages(QWidget *parent = 0);
    ~Messages();

    void dialog();

public slots:
    void onRecordlistPopulated();

signals:
    void needToDownloadMail();

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

    void RefreshMessages();

private:
    Ui::Messages *ui;

    QScopedPointer<QMenu> popupMenu_;

    QAction * pActionDelete = nullptr;
    QAction * pActionOpenNewWindow = nullptr;
    QAction * pActionReply = nullptr;
    QAction * pActionForward = nullptr;

    int nCurrentContact_ = 0;
    QString qstrMethodType_;
    QString qstrViaTransport_;

    QPointer<MessagesProxyModel> pMsgProxyModelInbox_;
    QPointer<MessagesProxyModel> pMsgProxyModelOutbox_;

    QTableView         * pCurrentTabTableView_  = nullptr;
    MessagesProxyModel * pCurrentTabProxyModel_ = nullptr;

    mapOfMsgTreeItems    mapCurrentRows_inbox; // For each tree item, we store a "currently selected" row so we can re-select it when that tree item is clicked.
    mapOfMsgTreeItems    mapCurrentRows_outbox;

    QList<QModelIndex> listRecordsToMarkAsRead_;
};

#endif // MESSAGES_HPP



