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

#include "Widgets/MarketWindow.h"
#include "Widgets/overviewwindow.h"
#include "Widgets/addressbookwindow.h"
#include "Widgets/nymmanagerwindow.h"
#include "Widgets/assetmanagerwindow.h"
#include "Widgets/accountmanagerwindow.h"
#include "Widgets/servermanagerwindow.h"
#include "Widgets/withdrawascashwindow.h"
#include "Widgets/withdrawasvoucherwindow.h"
#include "Widgets/depositwindow.h"
#include "Widgets/requestfundswindow.h"
#include "Widgets/sendfundswindow.h"

class AddressBookWindow;
class AccountManagerWindow;
class ServerManagerWindow;
class WithdrawAsCashWindow;
class WithdrawAsVoucherWindow;
class DepositWindow;
class RequestFundsWindow;
class SendFundsWindow;


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
    
    //Address Book Dialog
    //Show address book
    void mc_addressbook_show(QString text);

    
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
    
    QString get_default_server_id(){return default_server_id;};
    int get_server_list_id_size(){return server_list_id->size();};
    QString get_server_id_at(int a){return server_list_id->at(a).toString();}
    QString get_server_name_at(int a){return server_list_name->at(a).toString();}
    
    void set_systrayMenu_withdraw_asvoucher_nym_input(QString input);
    // Set Systray Nym Value
    void set_systrayMenu_nym_setDefaultNym(QString, QString);
    
    // Set Systray Asset Value
    void set_systrayMenu_asset_setDefaultAsset(QString, QString);

    // Set Systray Account Value
    void set_systrayMenu_account_setDefaultAccount(QString, QString);
    void close_accountmanager_dialog();
    
    // Set Systray Server Value
    void set_systrayMenu_server_setDefaultServer(QString, QString);
    void close_servermanager_dialog();
    


    
    // OT Interface Functions
    // These should probably be moved to the main class file.
    std::string ot_withdraw_cash(std::string selected_server_id_string, std::string nym_id, std::string selected_account_id_string, int amount_to_withdraw_int){return ot_me->withdraw_cash(selected_server_id_string, nym_id, selected_account_id_string, amount_to_withdraw_int);};
    std::string ot_withdraw_voucher(std::string selected_server_id_string, std::string nym_id, std::string selected_account_id_string, std::string recip_nym_string, std::string memo_string, int amount_to_withdraw_int){return ot_me->withdraw_voucher(selected_server_id_string, nym_id, selected_account_id_string, recip_nym_string, memo_string, amount_to_withdraw_int);};
    
private:
    // ------------------------------------------------
    /**           **
     ** Variables **
     **           **/
    //Open Transaction
    OT_ME * ot_me;
    
    ot_worker * ot_worker_background;
    
    // Already initialized bool's
    
    bool mc_overview_already_init;
    bool mc_market_window_already_init;
    bool mc_addressbook_already_init;
    bool mc_nymmanager_already_init;
    bool mc_assetmanager_already_init;
    bool mc_accountmanager_already_init;
    bool mc_servermanager_already_init;
    bool mc_withdraw_ascash_already_init;
    bool mc_withdraw_asvoucher_already_init;
    
    void nymmanager_dialog();
    
    AddressBookWindow * addressbookwindow;
    AccountManagerWindow * accountmanagerwindow;
    ServerManagerWindow * servermanagerwindow;
    WithdrawAsCashWindow * withdrawascashwindow;
    WithdrawAsVoucherWindow * withdrawasvoucherwindow;
    DepositWindow * depositwindow;
    RequestFundsWindow * requestfundswindow;
    SendFundsWindow * sendfundswindow;

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
    
    //As Voucher
    void mc_withdraw_asvoucher_slot();

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
