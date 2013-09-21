#ifndef NYMMANAGERWINDOW_H
#define NYMMANAGERWINDOW_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QDialog>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QDebug>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>
#include <opentxs/OTLog.h>

#include "moneychanger.h"


class NymManagerWindow : public QWidget
{
    Q_OBJECT
public:
    explicit NymManagerWindow(QWidget *parent = 0);
    void dialog();
    
private:
        
    //Nym Manager
    bool already_init;
    bool refreshing;
    
    /** Nym Manger Slot locks **/
    bool proccessing_dataChanged;
    
    //"Add Nym" dialog
    bool addnym_dialog_already_init;
    bool addnym_dialog_advanced_showing;
    
    //"Remove Nym" dialog
    bool removenym_dialog_already_init;
    
    QDialog * nym_manager_dialog;
    QGridLayout * nym_manager_gridlayout;
    QLabel * nym_manager_label;
    QWidget * nym_manager_holder;
    QHBoxLayout * nym_manager_hbox;
    QStandardItemModel * nym_manager_tableview_itemmodel;
    QTableView * nym_manager_tableview;
    QWidget * nym_manager_addremove_btngroup_holder;
    QVBoxLayout * nym_manager_addremove_btngroup_vbox;
    QPushButton * nym_manager_addremove_btngroup_addbtn;
    QPushButton * nym_manager_addremove_btngroup_removebtn;
    QLabel * mc_nym_manager_most_recent_erorr;
    
    /** "Add Nym" Dialog **/

    QDialog * nym_manager_addnym_dialog;
    //Grid layout
    QGridLayout * nym_manager_addnym_gridlayout;
    //Label (header)
    QLabel * nym_manager_addnym_header;
    
    //Label (Toggle Advanced Options Label/Button)
    QLabel * nym_manager_addnym_subheader_toggleadvanced_options_label;
    
    //Label (instructions)
    QLabel * nym_manager_addnym_subheader_instructions;
    
    //Label (choose source)
    QLabel * nym_manager_addnym_choosesource_label;
    
    //Combobox (choose source)
    QComboBox * nym_manager_addnym_choosesource_answer_selection;
    
    //Button (create nym)
    QPushButton * nym_manager_addnym_create_nym_btn;
    // ------------------------------------------------
    QDialog * nym_manager_removenym_dialog;
    //Grid layout
    QGridLayout * nym_manager_removenym_gridlayout;
    
    //Label (header)
    QLabel * nym_manager_removenym_header;

    // ------------------------------------------------

signals:

private slots:
    
    void addnym_slot();
    void removenym_slot();
    void dataChanged_slot(QModelIndex topLeft, QModelIndex bottomRight);
    void addnym_dialog_showadvanced_slot(QString link_href);
    void addnym_dialog_createnym_slot();
    void request_remove_nym_slot();
    
protected:
    
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // NYMMANAGERWINDOW_H
