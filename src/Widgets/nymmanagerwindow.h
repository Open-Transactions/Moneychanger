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
    
    bool already_init;
    bool refreshing;
    bool proccessing_dataChanged;
    
    //Nym Manager
    bool mc_nymmanager_already_init;
    bool mc_nymmanager_refreshing;
    
    /** Nym Manger Slot locks **/
    bool mc_nymmanager_proccessing_dataChanged;
    
    //"Add Nym" dialog
    bool mc_nymmanager_addnym_dialog_already_init;
    bool mc_nymmanager_addnym_dialog_advanced_showing;
    
    //"Remove Nym" dialog
    bool mc_nymmanager_removenym_dialog_already_init;
    
    QDialog * mc_nym_manager_dialog;
    QGridLayout * mc_nym_manager_gridlayout;
    QLabel * mc_nym_manager_label;
    QWidget * mc_nym_manager_holder;
    QHBoxLayout * mc_nym_manager_hbox;
    QStandardItemModel * mc_nym_manager_tableview_itemmodel;
    QTableView * mc_nym_manager_tableview;
    QWidget * mc_nym_manager_addremove_btngroup_holder;
    QVBoxLayout * mc_nym_manager_addremove_btngroup_vbox;
    QPushButton * mc_nym_manager_addremove_btngroup_addbtn;
    QPushButton * mc_nym_manager_addremove_btngroup_removebtn;
    QLabel * mc_nym_manager_most_recent_erorr;
    
    /** "Add Nym" Dialog **/

    QDialog * mc_nym_manager_addnym_dialog;
    //Grid layout
    QGridLayout * mc_nym_manager_addnym_gridlayout;
    //Label (header)
    QLabel * mc_nym_manager_addnym_header;
    
    //Label (Toggle Advanced Options Label/Button)
    QLabel * mc_nym_manager_addnym_subheader_toggleadvanced_options_label;
    
    //Label (instructions)
    QLabel * mc_nym_manager_addnym_subheader_instructions;
    
    //Label (choose source)
    QLabel * mc_nym_manager_addnym_choosesource_label;
    
    //Combobox (choose source)
    QComboBox * mc_nym_manager_addnym_choosesource_answer_selection;
    
    //Button (create nym)
    QPushButton * mc_nym_manager_addnym_create_nym_btn;
    // ------------------------------------------------
    QDialog * mc_nym_manager_removenym_dialog;
    //Grid layout
    QGridLayout * mc_nym_manager_removenym_gridlayout;
    
    //Label (header)
    QLabel * mc_nym_manager_removenym_header;

    // ------------------------------------------------

signals:

private slots:
    
    void nymmanager_addnym_slot();
    void nymmanager_removenym_slot();
    void nymmanager_dataChanged_slot(QModelIndex topLeft, QModelIndex bottomRight);
    void addnym_dialog_showadvanced_slot(QString link_href);
    void addnym_dialog_createnym_slot();
    void request_remove_nym_slot();
    
protected:
    
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // NYMMANAGERWINDOW_H
