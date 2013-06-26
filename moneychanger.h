#ifndef MONEYCHANGER_H
#define MONEYCHANGER_H


#include <QMap>
#include <QtSql>
#include <QMenu>
#include <QList>
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
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QSystemTrayIcon>
#include <QCoreApplication>
#include <QStandardItemModel>
#include <QtSql/QtSql>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>
#include <opentxs/OTLog.h>

#include "ot_worker.h"

#include "MTRecordList.h"
#include "MTRecord.h"

class Moneychanger : public QWidget
{
    Q_OBJECT

public:
    /** Constructor & Destructor **/
    Moneychanger(QWidget *parent = 0);
    ~Moneychanger();

    /** Start **/
    void bootTray();

private:

    /**           **
     ** Variables **
     **           **/
        //Open Transaction
        OT_ME * ot_me;

        ot_worker * ot_worker_background;
        QTimer * ot_worker_timer;

        //Sqlite database(s)
        QSqlDatabase addressbook_db;

        //MC Address book
        int mc_addressbook_already_init;
        int mc_addressbook_refreshing;
        QString mc_addressbook_paste_into;

            //Dialog
            QDialog * mc_addressbook_dialog;
            QGridLayout * mc_addressbook_gridlayout;

                /** Top (Spans 2 columns) **/
                //Label
                QLabel * mc_addressbook_label;

                /** Left side (column 1) **/
                    //Table view
                    QStandardItemModel * mc_addressbook_tableview_itemmodel;
                    QTableView * mc_addressbook_tableview;

                /** Right side (column 2) **/
                    //Button Group
                    QWidget * mc_addressbook_addremove_btngroup_widget;
                    QVBoxLayout * mc_addressbook_addremove_btngroup_holder;

                        //Add contact
                        QPushButton * mc_addressbook_addremove_add_btn;

                        //Remove contact
                        QPushButton * mc_addressbook_addremove_remove_btn;

                /** Bottom (Spans 2 columns) **/
                //Button
                QPushButton * mc_addressbook_select_nym_for_paste_btn;

        //MC Systray icon
        QSystemTrayIcon * mc_systrayIcon;

            QIcon mc_systrayIcon_shutdown;

            QIcon mc_systrayIcon_overview;

            QIcon mc_systrayIcon_nym;
            QIcon mc_systrayIcon_server;

            QIcon mc_systrayIcon_goldaccount;
            QIcon mc_systrayIcon_goldcashpurse;

            QIcon mc_systrayIcon_withdraw;
            QIcon mc_systrayIcon_deposit;

            QIcon mc_systrayIcon_sendfunds;
            QIcon mc_systrayIcon_requestpayment;

            QIcon mc_systrayIcon_advanced;




        //MC Systray menu
        QMenu * mc_systrayMenu;

            //Systray menu skeleton
            QAction * mc_systrayMenu_headertext;
            QAction * mc_systrayMenu_aboveBlank;

            QAction * mc_systrayMenu_shutdown;

            QAction * mc_systrayMenu_overview;

            QAction * mc_systrayMenu_nym;
            QAction * mc_systrayMenu_server;

            QAction * mc_systrayMenu_goldaccount;
            QAction * mc_systrayMenu_goldcashpurse;

            QMenu * mc_systrayMenu_withdraw;
                //Withdraw submenu
                QAction * mc_systrayMenu_withdraw_ascash;
                QAction * mc_systrayMenu_withdraw_asvoucher;

            QAction * mc_systrayMenu_deposit;

            QAction * mc_systrayMenu_sendfunds;
            QAction * mc_systrayMenu_requestpayment;

            QAction * mc_systrayMenu_advanced;

            QAction * mc_systrayMenu_bottomblank;

        //MC Systray Dialogs
            /** Overview **/
            int mc_overview_already_init;

            QDialog * mc_overview_dialog_page;
                //Grid layout
                QGridLayout * mc_overview_gridlayout;
                    //Header (label)
                    QLabel * mc_overview_header_label;

                    //Horizontal Two Pane(Incomming/Outgoing) layout
                    QWidget * mc_overview_hbox_twopane_holder;
                    QHBoxLayout * mc_overview_hbox_twopane;
                        //Incomming (pane)
                        QWidget * mc_overview_incomming_pane_holder;
                        QVBoxLayout * mc_overview_incomming_pane;
                            //Header (label)
                            QLabel * mc_overview_incomming_header_label;

                            //Table view
                            QTableView * mc_overview_incomming_tableview;

                        //Outgoing Table View
                        QWidget * mc_overview_outgoing_pane_holder;
                        QVBoxLayout * mc_overview_outgoing_pane;
                            //Header (label)
                            QLabel * mc_overview_outgoing_header_label;

                            //Tabel view
                            QTableView * mc_overview_outgoing_tableview;


            /** Nym Manager **/
            int mc_nymmanager_already_init;
            int mc_nymmanager_refreshing;

            QDialog * mc_nym_manager_dialog;
                //Grid layout
                QGridLayout * mc_nym_manager_gridlayout;

                    /** First row **/
                    //Label (Nym Manager Header)
                    QLabel * mc_nym_manager_label;

                    /** Second Row **/
                    //Horizontal holder (List of nyms; Add/Remove nym button)
                    QWidget * mc_nym_manager_holder;
                    QHBoxLayout * mc_nym_manager_hbox;

                        //Tableview/item model for pseudo nym list.
                        QStandardItemModel * mc_nym_manager_tableview_itemmodel;
                        QTableView * mc_nym_manager_tableview;

                        //Vertical holder (add/remove nym buttons)
                        QWidget * mc_nym_manager_addremove_btngroup_holder;
                        QVBoxLayout * mc_nym_manager_addremove_btngroup_vbox;

                            //Add nym button
                            QPushButton * mc_nym_manager_addremove_btngroup_addbtn;

                            //Remove nym button
                            QPushButton * mc_nym_manager_addremove_btngroup_removebtn;

                /** "Add Nym" Dialog **/
                int mc_nymmanager_addnym_dialog_already_init;
                int mc_nymmanager_addnym_dialog_advanced_showing;
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


                /** Nym Manger Slot locks **/
                    int mc_nymmanager_proccessing_dataChanged;

            //Withdraw
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


    /**           **
     ** Functions **
     **           **/

        //Address Book Dialog
            //Show address book
            void mc_addressbook_show();
            void mc_addressbook_show(QString);

            //Add contact to address book
            void mc_addressbook_addblankrow(); //Adds a blank editable row for the user to add a contact with


        //Menu Dialog

            //Overview
            void mc_overview_dialog();

            //Default Nym
            void mc_nymmanager_dialog();

            //Withdraw
                //As Cash
                void mc_withdraw_ascash_dialog();

                //As Voucher
                void mc_withdraw_asvoucher_dialog();

private slots:
        //Overview Slots
                //Thread slots
                void mc_worker_overview_ping_slot();

        //Nym Manager slots
            void mc_nymmanager_addnym_slot();
            void mc_nymmanager_dataChanged_slot(QModelIndex,QModelIndex);

                //Add Nym Dialog slots
                void mc_addnym_dialog_showadvanced_slot(QString);
                void mc_addnym_dialog_createnym_slot();


        //Address Book slots
            //Create a new blank editable address book row
            void mc_addressbook_addblankrow_slot();

            //Remove contact from address book
            void mc_addressbook_confirm_remove_contact_slot();

            //When the operator is done editing a data row, sync it with the database
            void mc_addressbook_dataChanged_slot(QModelIndex,QModelIndex);

            //When the operator has clicked the "Select and paste" button, we will detect what to paste and where to paste it into.
            void mc_addressbook_paste_selected_slot();

        //Systray Menu Slots
            //Shutdown
            void mc_shutdown_slot();

            //Overview
            void mc_overview_slot();

            //Default Nym
            void mc_defaultnym_slot();

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
};

#endif // MONEYCHANGER_H
