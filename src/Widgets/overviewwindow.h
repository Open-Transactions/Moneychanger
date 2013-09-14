#ifndef OVERVIEWWINDOW_H
#define OVERVIEWWINDOW_H

#include <QWidget>
#include <QMutex>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QDateTime>
#include <QMap>
#include <QVariant>
#include <QDialog>


class OverviewWindow : public QWidget
{
    Q_OBJECT
public:
    explicit OverviewWindow(QWidget *parent = 0);
    void dialog();

private:

    /** Functions **/
    void refresh();

    /** Overview **/
    bool already_init;
    QMutex mc_overview_refreshing_visuals_mutex;
    QDialog * mc_overview_dialog_page;
    //Grid layout
    QGridLayout * mc_overview_gridlayout;
    //Header (label)
    QLabel * mc_overview_header_label;

    //In/out going pane (Table View)
    QWidget * mc_overview_inoutgoing_pane_holder;
    QVBoxLayout * mc_overview_inoutgoing_pane;
    //Header (label)
    QLabel * mc_overview_inoutgoing_header_label;

    //Gridview of Transactionslist
    QScrollArea * mc_overview_inoutgoing_scroll;
    QWidget * mc_overview_inoutgoing_gridview_widget;
    QGridLayout * mc_overview_inoutgoing_gridview;

    //Tracking index <> MTRecordlist index
    QList<QVariant> mc_overview_index_of_tx;
    // ------------------------------------------------

signals:

public slots:

};

#endif // OVERVIEWWINDOW_H
