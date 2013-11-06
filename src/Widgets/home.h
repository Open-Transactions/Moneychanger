#ifndef MTHOME_H
#define MTHOME_H

#include <QWidget>
#include <QLabel>
#include <QMutex>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QScrollArea>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>
#include <opentxs/OTLog.h>

#include "MTRecord.hpp"
#include "MTRecordList.hpp"


namespace Ui {
class MTHome;
}

class MTHomeDetail;

class MTHome : public QWidget
{
    Q_OBJECT
    
private:
    /** Functions **/

    void RefreshRecords();
    void setupRecordList();
    void RefreshUserBar();

    QWidget * CreateUserBarWidget();

    /** Overview **/
    bool already_init;
    QMutex mc_overview_refreshing_visuals_mutex;
    // ------------------------------------------------
    MTHomeDetail * m_pDetailPane;
    QVBoxLayout  * m_pDetailLayout;
    QGridLayout  * m_pHeaderLayout;
    // ------------------------------------------------
    MTRecordList   m_list;
    // ------------------------------------------------
    bool    m_bNeedRefresh;
    // ------------------------------------------------
public:
    explicit MTHome(QWidget *parent = 0);
    ~MTHome();
    
    void dialog();

    void SetNeedRefresh();
    void RefreshAll();

    void OnDeletedRecord();

    static QString shortAcctBalance(QString qstr_acct_id,   QString qstr_asset_id=QString(""));
    static QString cashBalance     (QString qstr_server_id, QString qstr_asset_id, QString qstr_nym_id);
    static int64_t rawCashBalance  (QString qstr_server_id, QString qstr_asset_id, QString qstr_nym_id);
    static int64_t rawAcctBalance(QString qstrAcctId);

    static QString FormDisplayLabelForAcctButton(QString qstr_acct_id, QString qstr_display_name);

private slots:
    void on_tableWidget_currentCellChanged(int row, int column, int previousRow, int previousColumn);

    void on_account_clicked();

    void on_refreshButton_clicked();
    void on_contactsButton_clicked();

    void on_sendButton_clicked();
    void on_requestButton_clicked();

private:
    Ui::MTHome *ui;

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // MTHOME_H
