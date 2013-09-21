#ifndef ACCOUNTMANAGERWINDOW_H
#define ACCOUNTMANAGERWINDOW_H

#include <QWidget>
#include <QGridLayout>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialog>
#include <QObject>
#include <QEvent>
#include <QKeyEvent>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>
#include <opentxs/OTLog.h>

#include "moneychanger.h"

class AccountManagerWindow : public QWidget
{
    Q_OBJECT
public:
    explicit AccountManagerWindow(QWidget *parent = 0);
    ~AccountManagerWindow(){};
    void dialog();
    
private:
    
    /** Account Manager **/
    bool already_init;
    bool refreshing;
    
    QDialog * account_manager_dialog;
    //Grid layout
    QGridLayout * account_manager_gridlayout;
    
    /** First row **/
    //Label (Account Manager Header)
    QLabel * account_manager_label;
    
    /** Second Row **/
    //Horizontal holder (List of accounts; Add/Remove account button)
    QWidget     * account_manager_holder;
    QHBoxLayout * account_manager_hbox;
    
    //Tableview/item model for account list.
    QStandardItemModel * account_manager_tableview_itemmodel;
    QTableView         * account_manager_tableview;
    
    //Vertical holder (add/remove account buttons)
    QWidget     * account_manager_addremove_btngroup_holder;
    QVBoxLayout * account_manager_addremove_btngroup_vbox;
    
    //Add account button
    QPushButton * account_manager_addremove_btngroup_addbtn;
    
    //Remove account button
    QPushButton * account_manager_addremove_btngroup_removebtn;
    
    /** Third Row (most recent error) **/
    QLabel * account_manager_most_recent_erorr;
    // ------------------------------------------------
    /** "Add Account" Dialog **/
    bool accountmanager_addaccount_dialog_already_init;
    bool accountmanager_addaccount_dialog_advanced_showing;
    
    QDialog * account_manager_addaccount_dialog;
    //Grid layout
    QGridLayout * account_manager_addaccount_gridlayout;
    //Label (header)
    QLabel * account_manager_addaccount_header;
    
    //Label (Toggle Advanced Options Label/Button)
    QLabel * account_manager_addaccount_subheader_toggleadvanced_options_label;
    
    //Label (instructions)
    QLabel * account_manager_addaccount_subheader_instructions;
    
    //Button (create account)
    QPushButton * account_manager_addaccount_create_account_btn;
    // ------------------------------------------------
    /** "Remove Account Dialog **/
    bool account_manager_removeaccount_dialog_already_init;
    QDialog * account_manager_removeaccount_dialog;
    //Grid layout
    QGridLayout * account_manager_removeaccount_gridlayout;
    
    //Label (header)
    QLabel * account_manager_removeaccount_header;
    // ------------------------------------------------
    /** Account Manger Slot locks **/
    bool account_manager_proccessing_dataChanged;
    // ------------------------------------------------


signals:

private slots:
    
    // ------------------------------------------------
    //Account Manager slots
    void addaccount_slot();
    void removeaccount_slot();
    void dataChanged_slot(QModelIndex,QModelIndex);
    
    //Add account Dialog slots
    void dialog_showadvanced_slot(QString);
    void dialog_createaccount_slot();
    //request to remove a selected account from the accountlist manager
    void request_remove_account_slot();

    
protected:
    
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // ACCOUNTMANAGERWINDOW_H
