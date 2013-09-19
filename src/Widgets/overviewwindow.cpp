#include "overviewwindow.h"
#include "ot_worker.h"
#include "moneychanger.h"

OverviewWindow::OverviewWindow(QWidget *parent) :
    QWidget(parent)
{
    already_init = false;
}

void OverviewWindow::dialog()
{
/** Overview Dialog **/

    /** If the overview dialog has already been init
     *  just show it, Other wise, init and show if this is
     *  the first time.
     **/
    if(!already_init){
        //The overview dialog has not been init yet; Init, then show it.
        mc_overview_dialog_page = new QDialog(0);
        mc_overview_dialog_page->setWindowFlags(Qt::WindowStaysOnTopHint);
        mc_overview_dialog_page->setWindowTitle("Overview | Moneychanger");
        
        mc_overview_dialog_page->installEventFilter(this);

        //Grid Layout
        mc_overview_gridlayout = new QGridLayout(0);
        mc_overview_dialog_page->setLayout(mc_overview_gridlayout);

        //Label (header)
        mc_overview_header_label = new QLabel("<h3>Overview of Transactions</h3>", 0);
        mc_overview_gridlayout->addWidget(mc_overview_header_label, 0,0, 1,1, Qt::AlignRight);

        //in/Outgoing (Pane)
        mc_overview_inoutgoing_pane_holder = new QWidget(0);
        mc_overview_inoutgoing_pane = new QVBoxLayout(0);
        mc_overview_inoutgoing_pane_holder->setLayout(mc_overview_inoutgoing_pane);
        mc_overview_gridlayout->addWidget(mc_overview_inoutgoing_pane_holder, 1,0, 1,1);

        //Label (inOutgoing header)
        //                              mc_overview_inoutgoing_header_label = new QLabel("<b>Incoming & Outgoing Transactions</b>");
        mc_overview_inoutgoing_header_label = new QLabel;
        mc_overview_inoutgoing_pane->addWidget(mc_overview_inoutgoing_header_label);

        //GridView (Lists of TX)
        mc_overview_inoutgoing_scroll = new QScrollArea;
        mc_overview_inoutgoing_gridview_widget = new QWidget(0);
        mc_overview_inoutgoing_gridview = new QGridLayout(0);
        mc_overview_inoutgoing_gridview_widget->setLayout(mc_overview_inoutgoing_gridview);
        mc_overview_inoutgoing_gridview->setGeometry(QRect(100,100,100,100));
        mc_overview_inoutgoing_scroll->setWidgetResizable(true);
        mc_overview_inoutgoing_scroll->setBackgroundRole(QPalette::Light);
        mc_overview_inoutgoing_scroll->setWidget(mc_overview_inoutgoing_gridview_widget);
        mc_overview_inoutgoing_pane->addWidget(mc_overview_inoutgoing_scroll);


        /** Flag Already Init **/
        already_init = true;
    }
    //Resize
    mc_overview_dialog_page->resize(800, 400);
    //Show
    mc_overview_dialog_page->show();

    //Refresh visual data
    //Tell OT to repopulate, and refresh backend.
    ((Moneychanger*)parentWidget())->get_ot_worker_background()->mc_overview_ping();

    //Now refresh the repopulated data visually
    refresh();
}

//Overview refresh function
void OverviewWindow::refresh(){

    //(Lock the overview dialog refreshing mechinism until finished)
    QMutexLocker overview_refresh_locker(&mc_overview_refreshing_visuals_mutex);

    //Update the overview dialog visuals
    QList< QMap<QString,QVariant> > current_list_copy = ((Moneychanger*)parentWidget())->get_ot_worker_background()->mc_overview_get_currentlist();

    //Clear all records (In the future we should have a scan for updates records mechinism for now we will go for a browser "refresh" all mechinism)
    int items_in_inoutgoing_gridview = mc_overview_inoutgoing_gridview->rowCount();
    for(int a = 0; a < items_in_inoutgoing_gridview; a++){
        QLayoutItem * item = mc_overview_inoutgoing_gridview->takeAt(0);
        mc_overview_inoutgoing_gridview->removeItem(item);
        delete item;

    }

    int total_records_to_visualize = current_list_copy.size();
    for(int a = 0; a < total_records_to_visualize; a++){
        //Get map for this record
        QMap<QString, QVariant> record_map = current_list_copy.at(a);

        //Append to transactions list in overview dialog.
        QWidget * row_widget = new QWidget(0);
        QGridLayout * row_widget_layout = new QGridLayout(0);
        row_widget->setLayout(row_widget_layout);
        row_widget->setStyleSheet("QWidget{background-color:#c0cad4;}");

        //Render row.
        //Header of row
        QString tx_name = QString(record_map["name"].toString());
        if(tx_name.trimmed() == ""){
            //Tx has no name
            tx_name.clear();
            tx_name = "Transaction";
        }

        QLabel * header_of_row = new QLabel(0);
        QString header_of_row_string = QString();
        header_of_row_string.append(tx_name);

        header_of_row->setText(header_of_row_string);

        //Append header to layout
        row_widget_layout->addWidget(header_of_row, 0, 0, 1,1, Qt::AlignLeft);

        // Amount (with currency tla)
        QLabel * currency_amount_label = new QLabel(0);
        QString currency_amount = QString();


        if (record_map["ismail"].toBool())
        {
            if (record_map["isoutgoing"].toBool())
                currency_amount.append("sent message");
            else
                currency_amount.append("message");
        }
        else
            currency_amount.append(record_map["formatAmount"].toString());
        //                                    currency_amount.append(record_map["currencyTLA"].toString());
        //                                    currency_amount.append(QString(" %1"));
        //                                    currency_amount = currency_amount.arg(record_map["amount"].toString());

        if (record_map["isoutgoing"].toBool() || (record_map["amount"].toInt() < 0))
            currency_amount_label->setStyleSheet("QLabel { color : red; }");
        else
            currency_amount_label->setStyleSheet("QLabel { color : green; }");

        currency_amount_label->setText(currency_amount);
        row_widget_layout->addWidget(currency_amount_label, 0, 1, 1,1, Qt::AlignRight);

        //Sub-info
        QWidget * row_content_container = new QWidget(0);
        QGridLayout * row_content_grid = new QGridLayout(0);
        row_content_container->setLayout(row_content_grid);
        row_widget_layout->addWidget(row_content_container, 1,0, 1,2);

        /** Column one **/
        //Date (sub-info)
        //Calc/convert date/times
        QDateTime timestamp;
        timestamp.setTime_t(record_map["date"].toInt());

        QLabel * row_content_date_label = new QLabel(0);
        QString row_content_date_label_string = QString();
        row_content_date_label_string.append(QString(timestamp.toString(Qt::SystemLocaleShortDate)));
        row_content_date_label->setText(row_content_date_label_string);
        row_content_grid->addWidget(row_content_date_label, 0,0, 1,1, Qt::AlignLeft);

        /** Column two **/
        //Status
        QLabel * row_content_status_label = new QLabel(0);
        QString row_content_status_string = QString();

        //                                    if (record_map["ismail"].toBool())
        //                                        row_content_status_string.append(record_map["shortMail"].toString());
        //                                    else
        row_content_status_string.append(record_map["formatDescription"].toString());

        //add string to label
        row_content_status_label->setText(row_content_status_string);

        //add to row_content grid
        row_content_grid->addWidget(row_content_status_label, 0,1, 1,1, Qt::AlignRight);


        /** Append information to the grid/visuals. **/
        mc_overview_inoutgoing_gridview->addWidget(row_widget, a,0, 1,1);

    }

}



// This event filter catches Esc key and Close events
// So that they get cleaned up in the parentWidget appropriately.
// Note that this won't work if the parentWidget isn't of
// Type Moneychanger, but this can be modified so that Esc events are
// Discarded (Don't do this if the Widget is Modal!)
/*
 
 if(e->key() != Qt::Key_Escape)
 QDialog::keyPressEvent(e);
 else {;}
 
 */

bool OverviewWindow::eventFilter(QObject *obj, QEvent *event){
    
    if (event->type() == QEvent::Close) {
        ((Moneychanger *)parentWidget())->close_overview_dialog();
        return true;
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape){
            mc_overview_dialog_page->close(); // This is caught by this same filter.
            return true;
        }
        return true;
    }else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}
