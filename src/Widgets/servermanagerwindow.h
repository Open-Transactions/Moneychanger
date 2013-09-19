#ifndef SERVERMANAGERWINDOW_H
#define SERVERMANAGERWINDOW_H

#include <QWidget>
#include <QDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QStandardItemModel>
#include <QTableView>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QComboBox>
#include <QEvent>
#include <QKeyEvent>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>
#include <opentxs/OTLog.h>

#include "moneychanger.h"

class ServerManagerWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ServerManagerWindow(QWidget *parent = 0);
public:
    void dialog();
    
private:
        
    /** Server Manager **/
    int mc_servermanager_already_init;
    int mc_servermanager_refreshing;
    
    QDialog * mc_server_manager_dialog;
    //Grid layout
    QGridLayout * mc_server_manager_gridlayout;
    
    /** First row **/
    //Label (server Manager Header)
    QLabel * mc_server_manager_label;
    
    /** Second Row **/
    //Horizontal holder (List of servers; Add/Remove server button)
    QWidget     * mc_server_manager_holder;
    QHBoxLayout * mc_server_manager_hbox;
    
    //Tableview/item model for server list.
    QStandardItemModel * mc_server_manager_tableview_itemmodel;
    QTableView         * mc_server_manager_tableview;
    
    //Vertical holder (add/remove server buttons)
    QWidget     * mc_server_manager_addremove_btngroup_holder;
    QVBoxLayout * mc_server_manager_addremove_btngroup_vbox;
    
    //Add server button
    QPushButton * mc_server_manager_addremove_btngroup_addbtn;
    
    //Remove server button
    QPushButton * mc_server_manager_addremove_btngroup_removebtn;
    
    /** Third Row (most recent error) **/
    QLabel * mc_server_manager_most_recent_erorr;
    // ------------------------------------------------
    /** "Add server" Dialog **/
    int mc_servermanager_addserver_dialog_already_init;
    int mc_servermanager_addserver_dialog_advanced_showing;
    
    QDialog * mc_server_manager_addserver_dialog;
    //Grid layout
    QGridLayout * mc_server_manager_addserver_gridlayout;
    //Label (header)
    QLabel * mc_server_manager_addserver_header;
    
    //Label (Toggle Advanced Options Label/Button)
    QLabel * mc_server_manager_addserver_subheader_toggleadvanced_options_label;
    
    //Label (instructions)
    QLabel * mc_server_manager_addserver_subheader_instructions;
    
    //Label (choose source)
    QLabel * mc_server_manager_addserver_choosesource_label;
    
    //Combobox (choose source)
    QComboBox * mc_server_manager_addserver_choosesource_answer_selection;
    
    //Button (create server)
    QPushButton * mc_server_manager_addserver_create_server_btn;
    // ------------------------------------------------
    /** "Remove server Dialog **/
    int mc_servermanager_removeserver_dialog_already_init;
    QDialog * mc_server_manager_removeserver_dialog;
    //Grid layout
    QGridLayout * mc_server_manager_removeserver_gridlayout;
    
    //Label (header)
    QLabel * mc_server_manager_removeserver_header;
    // ------------------------------------------------
    /** server Manger Slot locks **/
    int mc_servermanager_proccessing_dataChanged;
    // ------------------------------------------------

signals:

private slots:
    
    // ------------------------------------------------
    //Server Manager slots
    void addserver_slot();
    void removeserver_slot();
    void dataChanged_slot(QModelIndex,QModelIndex);
    
    //Add server Dialog slots
    void dialog_showadvanced_slot(QString);
    void dialog_createserver_slot();
    
    //request to remove a selected server from the serverlist manager
    void request_remove_server_slot();
    
protected:

    bool eventFilter(QObject *obj, QEvent *event);


};

#endif // SERVERMANAGERWINDOW_H
