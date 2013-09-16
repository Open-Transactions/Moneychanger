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
    bool mc_accountmanager_already_init;
    bool mc_accountmanager_refreshing;
    
    QDialog * mc_account_manager_dialog;
    //Grid layout
    QGridLayout * mc_account_manager_gridlayout;
    
    /** First row **/
    //Label (Account Manager Header)
    QLabel * mc_account_manager_label;
    
    /** Second Row **/
    //Horizontal holder (List of accounts; Add/Remove account button)
    QWidget     * mc_account_manager_holder;
    QHBoxLayout * mc_account_manager_hbox;
    
    //Tableview/item model for account list.
    QStandardItemModel * mc_account_manager_tableview_itemmodel;
    QTableView         * mc_account_manager_tableview;
    
    //Vertical holder (add/remove account buttons)
    QWidget     * mc_account_manager_addremove_btngroup_holder;
    QVBoxLayout * mc_account_manager_addremove_btngroup_vbox;
    
    //Add account button
    QPushButton * mc_account_manager_addremove_btngroup_addbtn;
    
    //Remove account button
    QPushButton * mc_account_manager_addremove_btngroup_removebtn;
    
    /** Third Row (most recent error) **/
    QLabel * mc_account_manager_most_recent_erorr;
    // ------------------------------------------------
    /** "Add Account" Dialog **/
    int mc_accountmanager_addaccount_dialog_already_init;
    int mc_accountmanager_addaccount_dialog_advanced_showing;
    
    QDialog * mc_account_manager_addaccount_dialog;
    //Grid layout
    QGridLayout * mc_account_manager_addaccount_gridlayout;
    //Label (header)
    QLabel * mc_account_manager_addaccount_header;
    
    //Label (Toggle Advanced Options Label/Button)
    QLabel * mc_account_manager_addaccount_subheader_toggleadvanced_options_label;
    
    //Label (instructions)
    QLabel * mc_account_manager_addaccount_subheader_instructions;
    
    //Button (create account)
    QPushButton * mc_account_manager_addaccount_create_account_btn;
    // ------------------------------------------------
    /** "Remove Account Dialog **/
    int mc_accountmanager_removeaccount_dialog_already_init;
    QDialog * mc_account_manager_removeaccount_dialog;
    //Grid layout
    QGridLayout * mc_account_manager_removeaccount_gridlayout;
    
    //Label (header)
    QLabel * mc_account_manager_removeaccount_header;
    // ------------------------------------------------
    /** Account Manger Slot locks **/
    int mc_accountmanager_proccessing_dataChanged;
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
