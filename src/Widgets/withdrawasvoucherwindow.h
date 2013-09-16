#ifndef WITHDRAWASVOUCHERWINDOW_H
#define WITHDRAWASVOUCHERWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QComboBox>
#include <QHBoxLayout>
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QIcon>
#include <QDebug>
#include <QString>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>
#include <opentxs/OTLog.h>

#include "moneychanger.h"


class WithdrawAsVoucherWindow : public QWidget
{
    Q_OBJECT
public:
    explicit WithdrawAsVoucherWindow(QWidget *parent = 0);
    
    void dialog();
    
    void set_systrayMenu_withdraw_asvoucher_nym_input_text(QString input){mc_systrayMenu_withdraw_asvoucher_nym_input->setText(input);};

private:
    
    bool mc_withdraw_asvoucher_dialog_already_init;
    bool mc_withdraw_asvoucher_confirm_dialog_already_init;
    
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
    
    //Nym Id (type/paste input)
    QLineEdit * mc_systrayMenu_withdraw_asvoucher_nym_input;
    
    //Withdraw as Voucher confirmation

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

    
signals:

private slots:
    
    void mc_withdraw_asvoucher_show_addressbook_slot();
    void mc_withdraw_asvoucher_confirm_amount_dialog_slot();
    void set_withdraw_asvoucher_account_dropdown_highlighted_slot(int);
    void mc_withdraw_asvoucher_confirm_amount_slot();
    void mc_withdraw_asvoucher_cancel_amount_slot();

};

#endif // WITHDRAWASVOUCHERWINDOW_H
