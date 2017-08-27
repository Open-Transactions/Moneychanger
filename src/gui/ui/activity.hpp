#ifndef ACTIVITY_HPP
#define ACTIVITY_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include "core/mapidname.hpp"

#include <QWidget>
#include <QMenu>
#include <QScopedPointer>
#include <QPointer>
#include <QList>
#include <QSqlRecord>
#include <QTimer>
#include <QDebug>

#include <tuple>
#include <map>


namespace Ui {
class Activity;
}

class QListWidget;

class Activity : public QWidget
{
    Q_OBJECT

private:
    bool already_init{false};

public:
    explicit Activity(QWidget *parent = 0);
    ~Activity();

    void dialog();

    // Remove these once I put the actual filter in place:
    int nCurrentContact_ = 0;
    QString qstrMethodType_;
    QString qstrViaTransport_;


protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void RefreshAll();
    void RefreshAccountTab();
    void ClearAccountTree();
    void RefreshAccountTree();
    void RefreshConversationsTab();
    void RefreshConversationDetails(int nRow);
    void ClearListWidgetConversations();

    void setupCurrentPointers();

    void PopulateConversationsForNym(QListWidget * pListWidgetConversations, int & nIndex, const std::string & str_my_nym_id);

    bool AddItemToConversation(int nAtIndex, const QString & qstrContents, bool bIncoming, bool bAddedByHand);

    mapIDName & GetOrCreateAssetIdMapByCurrencyCode(QString qstrTLA, mapOfMapIDName & bigMap);
    void GetAssetIdMapsByCurrencyCode(mapOfMapIDName & bigMap);
    int64_t GetAccountBalancesTotaledForUnitTypes(const mapIDName & mapUnitTypeIds);

    mapIDName & GetOrCreateAccountIdMapByServerId(QString qstrServerId, mapOfMapIDName & bigMap);
    void GetAccountIdMapsByServerId(mapOfMapIDName & bigMap, bool bPairedOrHosted); // true == paired, false == hosted.

private slots:
    void on_listWidgetConversations_currentRowChanged(int currentRow);
    void on_listWidgetConversations_customContextMenuRequested(const QPoint &pos);

    void on_pushButtonSendMsg_clicked();

    void on_checkBoxSearch_toggled(bool checked);

private:
    Ui::Activity *ui;
};

#endif // ACTIVITY_HPP
