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

    /** Overview **/
    bool already_init;
    QMutex mc_overview_refreshing_visuals_mutex;
    // ------------------------------------------------
    MTHomeDetail * m_pDetailPane;
    QVBoxLayout  * m_pDetailLayout;
    // ------------------------------------------------
    MTRecordList   m_list;

public:
    explicit MTHome(QWidget *parent = 0);
    ~MTHome();
    
    void dialog();

private slots:
    void on_tableWidget_cellClicked(int row, int column);

private:
    Ui::MTHome *ui;

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // MTHOME_H
