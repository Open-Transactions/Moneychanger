#ifndef MTHOME_H
#define MTHOME_H

#include <QPointer>
#include <QWidget>
#include <QFrame>
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
    // ------------------------------------------------
    QPointer<MTHomeDetail>  m_pDetailPane;
    // ------------------------------------------------
    QPointer<QFrame>        m_pHeaderFrame;
    // ------------------------------------------------
    MTRecordList   m_list;
    // ------------------------------------------------
    bool    m_bTurnRefreshBtnRed;
    // ------------------------------------------------
public:
    explicit MTHome(QWidget *parent = 0);
    ~MTHome();
    
    void dialog();

    void SetRefreshBtnRed();
    void RefreshAll();

    void OnDeletedRecord();

    static QString shortAcctBalance(QString qstr_acct_id,   QString qstr_asset_id=QString(""));
    static QString cashBalance     (QString qstr_server_id, QString qstr_asset_id, QString qstr_nym_id);
    static int64_t rawCashBalance  (QString qstr_server_id, QString qstr_asset_id, QString qstr_nym_id);
    static int64_t rawAcctBalance  (QString qstrAcctId);

    static QString FormDisplayLabelForAcctButton(QString qstr_acct_id, QString qstr_display_name);

signals:
    void needToDownloadAccountData();
    void needToRefreshDetails(int nRow, MTRecordList & theList);

public slots:
    void onAccountDataDownloaded();
    void onBalancesChanged();
    void onSetRefreshBtnRed();
    void onNeedToRefreshUserBar();
    void onNeedToRefreshRecords();
    void onRecordDeleted(bool bNeedToRefreshUserBar);

private slots:
    void on_tableWidget_currentCellChanged(int row, int column, int previousRow, int previousColumn);

    void on_refreshButton_clicked();

private:
    Ui::MTHome *ui;

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // MTHOME_H
