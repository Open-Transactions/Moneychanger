#ifndef MONEYCHANGER_H
#define MONEYCHANGER_H


#include <QMap>
#include <QtSql>
#include <QMenu>
#include <QList>
#include <QMutex>
#include <QDebug>
#include <QTimer>
#include <QLabel>
#include <QWidget>
#include <QDialog>
#include <QAction>
#include <QVariant>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QTableView>
#include <QScrollArea>
#include <QLayoutItem>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QMutexLocker>
#include <QRadioButton>
#include <QSystemTrayIcon>
#include <QCoreApplication>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QtSql/QtSql>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>
#include <opentxs/OTLog.h>

#include "ot_worker.h"

#include "MTRecordList.h"
#include "MTRecord.h"

#include "Widgets/accountmanagerwindow.h"

class AccountManagerWindow;


class Moneychanger : public QWidget
{
    Q_OBJECT
    
public:
    /** Constructor & Destructor **/
    Moneychanger(QWidget *parent = 0);
    ~Moneychanger();
    
    /** Start **/
    void bootTray();

    /** Interfaces **/
    ot_worker * get_ot_worker_background(){return ot_worker_background;};
    
    QString get_default_nym_id(){return default_nym_id;};
    int get_nym_list_id_size(){return nym_list_id->size();};
    QString get_nym_id_at(int a){return nym_list_id->at(a).toString();}
    QString get_nym_name_at(int a){return nym_list_name->at(a).toString();}
    
    QString get_default_asset_id(){return default_asset_id;};
    int get_asset_list_id_size(){return asset_list_id->size();};
    QString get_asset_id_at(int a){return asset_list_id->at(a).toString();}
    QString get_asset_name_at(int a){return asset_list_name->at(a).toString();}
    
    QString get_default_account_id(){return default_account_id;};
    int get_account_list_id_size(){return account_list_id->size();};
    QString get_account_id_at(int a){return account_list_id->at(a).toString();}
    QString get_account_name_at(int a){return account_list_name->at(a).toString();}
    
    void set_systrayMenu_withdraw_asvoucher_nym_input(QString input){mc_systrayMenu_withdraw_asvoucher_nym_input->setText(input);};
    // Set Systray Nym Value
    void set_systrayMenu_nym_setDefaultNym(QString, QString);
    
    // Set Systray Asset Value
    void set_systrayMenu_asset_setDefaultAsset(QString, QString);

    // Set Systray Account Value
    void set_systrayMenu_account_setDefaultAccount(QString, QString);
    void close_accountmanager_dialog();
    
private:
    // ------------------------------------------------
    /**           **
     ** Variables **
     **           **/
    //Open Transaction
    OT_ME * ot_me;
    
    ot_worker * ot_worker_background;
    
    // Already initialized bool's
    
    bool mc_market_window_already_init;
    bool mc_addressbook_already_init;
    bool mc_nymmanager_already_init;
    bool mc_assetmanager_already_init;
    bool mc_accountmanager_already_init;
    
    void nymmanager_dialog();
    
    
    AccountManagerWindow * accountmanagerwindow;

    // ------------------------------------------------
    //MC Systray icon
    QSystemTrayIcon * mc_systrayIcon;
    
    QIcon mc_systrayIcon_shutdown;
    
    QIcon mc_systrayIcon_overview;
    
    QIcon mc_systrayIcon_nym;
    QIcon mc_systrayIcon_server;
    
    QIcon mc_systrayIcon_goldaccount;
    QIcon mc_systrayIcon_purse;
    
    QIcon mc_systrayIcon_withdraw;
    QIcon mc_systrayIcon_deposit;
    
    QIcon mc_systrayIcon_sendfunds;
    QIcon mc_systrayIcon_requestfunds;
    
    QIcon mc_systrayIcon_advanced;
    QIcon mc_systrayIcon_advanced_agreements;
    QIcon mc_systrayIcon_advanced_markets;
    QIcon mc_systrayIcon_advanced_settings;
    // ------------------------------------------------
    //MC Systray menu
    QMenu * mc_systrayMenu;
    
    //Systray menu skeleton
    QAction * mc_systrayMenu_headertext;
    QAction * mc_systrayMenu_aboveBlank;
    
    QAction * mc_systrayMenu_shutdown;
    
    QAction * mc_systrayMenu_overview;
    // ---------------------------------------------------------
    QMenu * mc_systrayMenu_nym;
    //pseudonym list (backend) [For nym list in the qmenu and the nym manager]
    QList<QVariant> * nym_list_id;
    QList<QVariant> * nym_list_name;
    
    //pseudonym default selected (backend) [For saving the user supplied default, set from DB and user selections]
    QString default_nym_id;
    QString default_nym_name;
    // ---------------------------------------------------------
    QMenu * mc_systrayMenu_server;
    //server list (backend )
    QList<QVariant> * server_list_id;
    QList<QVariant> * server_list_name;
    
    //server default selected (backend)
    QString default_server_id;
    QString default_server_name;
    // ---------------------------------------------------------
    QMenu * mc_systrayMenu_asset;
    //asset type list (backend )
    QList<QVariant> * asset_list_id;
    QList<QVariant> * asset_list_name;
    
    //asset default selected (backend)
    QString default_asset_id;
    QString default_asset_name;
    // ---------------------------------------------------------
    QMenu * mc_systrayMenu_account;
    //account list (backend )
    QList<QVariant> * account_list_id;
    QList<QVariant> * account_list_name;
    
    //account default selected (backend)
    QString default_account_id;
    QString default_account_name;
    // ---------------------------------------------------------
    QAction * mc_systrayMenu_goldaccount;
    QAction * mc_systrayMenu_purse;
    // ---------------------------------------------------------
    QMenu * mc_systrayMenu_withdraw;
    //Withdraw submenu
    QAction * mc_systrayMenu_withdraw_ascash;
    QAction * mc_systrayMenu_withdraw_asvoucher;
    // ---------------------------------------------------------
    QAction * mc_systrayMenu_deposit;
    // ---------------------------------------------------------
    QAction * mc_systrayMenu_sendfunds;
    QAction * mc_systrayMenu_requestfunds;
    // ---------------------------------------------------------
    QMenu * mc_systrayMenu_advanced;
    //Advanced submenu
    QAction * mc_systrayMenu_advanced_agreements;
    QAction * mc_systrayMenu_advanced_markets;
    QAction * mc_systrayMenu_advanced_settings;
    // ---------------------------------------------------------
    QAction * mc_systrayMenu_bottomblank;
    // ---------------------------------------------------------
    //MC Systray Dialogs
    /** Overview **/
    bool mc_overview_already_init;
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
    /** Withdraw **/
    //As Cash
    int mc_withdraw_ascash_dialog_already_init;
    QDialog * mc_systrayMenu_withdraw_ascash_dialog;
    //Grid layout
    QGridLayout * mc_systrayMenu_withdraw_ascash_gridlayout;
    
    //Withdraw (as cash) header label
    QLabel * mc_systrayMenu_withdraw_ascash_header_label;
    
    //Account Id (label)
    QLabel * mc_systrayMenu_withdraw_ascash_accountid_label;
    
    //Account Name (Dropdown box)
    QComboBox * mc_systrayMenu_withdraw_ascash_account_dropdown;
    
    //Amount (in integer for now)
    QLineEdit * mc_systrayMenu_withdraw_ascash_amount_input;
    
    //Activate withdraw button
    QPushButton * mc_systrayMenu_withdraw_ascash_button;
    
    //Withdraw as cash confirmation
    int mc_withdraw_ascash_confirm_dialog_already_init;
    QDialog * mc_systrayMenu_withdraw_ascash_confirm_dialog;
    
    //Grid layout
    QGridLayout * mc_systrayMenu_withdraw_ascash_confirm_gridlayout;
    
    //Label
    QLabel * mc_systrayMenu_withdraw_ascash_confirm_label;
    
    //Label (Amount)
    QLabel * mc_systrayMenu_withdraw_ascash_confirm_amount_label;
    
    //Backend (Amount)
    int withdraw_ascash_confirm_amount_int;
    
    //Confirm/Cancel horizontal layout
    QWidget * mc_systrayMenu_withdraw_ascash_confirm_amount_confirm_cancel_widget;
    QHBoxLayout * mc_systrayMenu_withdraw_ascash_confirm_amount_confirm_cancel_layout;
    
    //Cancel amount (button)
    QPushButton * mc_systrayMenu_withdraw_ascash_confirm_amount_btn_cancel;
    
    //Confirm amount (button)
    QPushButton * mc_systrayMenu_withdraw_ascash_confirm_amount_btn_confirm;
    // ------------------------------------------------
    //As Voucher
    int mc_withdraw_asvoucher_dialog_already_init;
    QDialog * mc_systrayMenu_withdraw_asvoucher_dialog;
    //Grid layout
    QGridLayout * mc_systrayMenu_withdraw_asvoucher_gridlayout;
    //Withdraw (as voucher) header label
    QLabel * mc_systrayMenu_withdraw_asvoucher_header_label;
    
    //Account Id (label)
    QLabel * mc_systrayMenu_withdraw_asvoucher_accountid_label;
    
    //Account Name (Dropdown box)
    QComboBox * mc_systrayMenu_withdraw_asvoucher_account_dropdown;
    
    //Nym ID (input)
    //Horitzontal Layout for Nym ID Input
    QWidget * mc_systrayMenu_withdraw_asvoucher_nym_holder;
    QHBoxLayout * mc_systrayMenu_withdraw_asvoucher_nym_hbox;
    
    //Nym Id (type/paste input)
    QLineEdit * mc_systrayMenu_withdraw_asvoucher_nym_input;
    
    //Address book (button as Icon)
    QIcon mc_systrayMenu_withdraw_asvoucher_nym_addressbook_icon;
    QPushButton * mc_systrayMenu_withdraw_asvoucher_nym_addressbook_btn;
    
    //QR Code Scanner (button as Icon)
    //TODO ^^
    
    //Amount (in integer for now)
    QLineEdit * mc_systrayMenu_withdraw_asvoucher_amount_input;
    
    //Memo (Text box)
    QTextEdit * mc_systrayMenu_withdraw_asvoucher_memo_input;
    
    //Activate withdraw button
    QPushButton * mc_systrayMenu_withdraw_asvoucher_button;
    
    //Withdraw as Voucher confirmation
    int mc_withdraw_asvoucher_confirm_dialog_already_init;
    QDialog * mc_systrayMenu_withdraw_asvoucher_confirm_dialog;
    
    //Grid layout
    QGridLayout * mc_systrayMenu_withdraw_asvoucher_confirm_gridlayout;
    
    //Label
    QLabel * mc_systrayMenu_withdraw_asvoucher_confirm_label;
    
    //Label (Amount)
    QLabel * mc_systrayMenu_withdraw_asvoucher_confirm_amount_label;
    
    //Backend (Amount)
    int withdraw_asvoucher_confirm_amount_int;
    
    //Confirm/cancel horizontal layout
    QWidget * mc_systrayMenu_withdraw_asvoucher_confirm_amount_confirm_cancel_widget;
    QHBoxLayout * mc_systrayMenu_withdraw_asvoucher_confirm_amount_confirm_cancel_layout;
    
    //Cancel amount (button)
    QPushButton * mc_systrayMenu_withdraw_asvoucher_confirm_amount_btn_cancel;
    
    //Confirm amount (button)
    QPushButton * mc_systrayMenu_withdraw_asvoucher_confirm_amount_btn_confirm;
    // ------------------------------------------------
    /** Deposit **/
    int mc_deposit_already_init;
    QDialog * mc_deposit_dialog;
    //Gridlayout
    QGridLayout * mc_deposit_gridlayout;
    
    //header (label)
    QLabel * mc_deposit_header_label;
    
    //Dropdown box (combobox) (choose deposit type)
    QComboBox * mc_deposit_deposit_type;
    
    /** Deposit into account **/
    QWidget * mc_deposit_account_widget;
    QHBoxLayout * mc_deposit_account_layout;
    //(subheader) Deposit into account
    QLabel * mc_deposit_account_header_label;
    
    /** Deposit into purse **/
    QWidget * mc_deposit_purse_widget;
    QHBoxLayout * mc_deposit_purse_layout;
    //(header) Deposit into purse
    QLabel * mc_deposit_purse_header_label;
    // ------------------------------------------------
    /** Send Funds **/
    int mc_sendfunds_already_init;
    int mc_requestfunds_already_init;
    QDialog * mc_sendfunds_dialog;
    QDialog * mc_requestfunds_dialog;
    
    //Gridlayout
    QGridLayout * mc_sendfunds_gridlayout;
    QGridLayout * mc_requestfunds_gridlayout;
    
    //Send funds type selection (combobox)
    QComboBox * mc_sendfunds_sendtype_combobox;
    QComboBox * mc_requestfunds_sendtype_combobox;
    
    // ------------------------------------------------
    /**           **
     ** Functions **
     **           **/
    
    //Address Book Dialog
    //Show address book
    void mc_addressbook_show(QString text);
    
    //Add contact to address book
    void mc_addressbook_addblankrow(); //Adds a blank editable row for the user to add a contact with
    // ------------------------------------------------
    //Menu Dialog
    
    //Overview
    void mc_overview_dialog();
    //Refresh visual
    void mc_overview_dialog_refresh();
    // ------------------------------------------------
    //Default Nym
    void mc_nymmanager_dialog();
    
    //Reload nym list
    void mc_systrayMenu_reload_nymlist();
    // ------------------------------------------------
    //Default Server
    void mc_servermanager_dialog();
    //Load server
    void mc_systrayMenu_server_setDefaultServer(QString, QString);
    
    //Reload server list
    void mc_systrayMenu_reload_serverlist();
    // ------------------------------------------------
    //Default Asset
    void mc_assetmanager_dialog();
    
    //Reload asset list
    void mc_systrayMenu_reload_assetlist();
    // ------------------------------------------------
    //Default Account
    void mc_accountmanager_dialog();
    //Load account

    
    //Reload account list
    void mc_systrayMenu_reload_accountlist();
    // ------------------------------------------------
    //Withdraw
    //As Cash
    void mc_withdraw_ascash_dialog();
    
    //As Voucher
    void mc_withdraw_asvoucher_dialog();
    // ------------------------------------------------
    //Deposit
    void mc_deposit_show_dialog();
    // ------------------------------------------------
    //Send / Request funds
    void mc_sendfunds_show_dialog();
    void mc_requestfunds_show_dialog();
    // ------------------------------------------------
    
private slots:
    
    // Market Slot
    void mc_market_slot();

    // ------------------------------------------------
    //Server Manager slots
    void mc_servermanager_addserver_slot();
    void mc_servermanager_removeserver_slot();
    void mc_servermanager_dataChanged_slot(QModelIndex,QModelIndex);
    
    //Add server Dialog slots
    void mc_addserver_dialog_showadvanced_slot(QString);
    void mc_addserver_dialog_createserver_slot();

    // ------------------------------------------------
    //Systray Menu Slots
    //Shutdown
    void mc_shutdown_slot();
    
    //Overview
    void mc_overview_slot();
    // ------------------------------------------------
    //Nym
    void mc_defaultnym_slot();
    //new default nym selected
    void mc_nymselection_triggered(QAction*);
    
    // ------------------------------------------------
    //Server
    void mc_defaultserver_slot();
    //new default server selected
    void mc_serverselection_triggered(QAction*);
    
    //request to remove a selected server from the serverlist manager
    void mc_servermanager_request_remove_server_slot();
    // ------------------------------------------------
    //Asset
    void mc_defaultasset_slot();
    //new default asset selected
    void mc_assetselection_triggered(QAction*);
        // ------------------------------------------------
    //Account
    void mc_defaultaccount_slot();
    //new default account selected
    void mc_accountselection_triggered(QAction*);
    
    // ------------------------------------------------
    //Withdraw
    //As Cash
    void mc_withdraw_ascash_slot();
    void mc_withdraw_ascash_confirm_amount_dialog_slot();
    
    void mc_withdraw_ascash_account_dropdown_highlighted_slot(int);
    void mc_withdraw_ascash_confirm_amount_slot();
    void mc_withdraw_ascash_cancel_amount_slot();
    
    //As Voucher
    void mc_withdraw_asvoucher_slot();
    void mc_withdraw_asvoucher_show_addressbook_slot();
    void mc_withdraw_asvoucher_confirm_amount_dialog_slot();
    
    void mc_withdraw_asvoucher_account_dropdown_highlighted_slot(int);
    void mc_withdraw_asvoucher_confirm_amount_slot();
    void mc_withdraw_asvoucher_cancel_amount_slot();
    // ------------------------------------------------
    //Deposit
    void mc_deposit_slot();
    
    //The user changed the "deposit type" switch open/available menu
    void mc_deposit_type_changed_slot(int);
    // ------------------------------------------------
    //Send /Request funds
    void mc_sendfunds_slot();
    void mc_requestfunds_slot();
    
};

#endif // MONEYCHANGER_H
