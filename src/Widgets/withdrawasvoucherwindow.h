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
    
    void set_systrayMenu_withdraw_asvoucher_nym_input_text(QString input){withdraw_asvoucher_nym_input->setText(input);};

private:
    
    bool already_init;
    bool confirm_dialog_already_init;
    
    QDialog * withdraw_asvoucher_dialog;
    //Grid layout
    QGridLayout * withdraw_asvoucher_gridlayout;
    //Withdraw (as voucher) header label
    QLabel * withdraw_asvoucher_header_label;
    
    //Account Id (label)
    QLabel * withdraw_asvoucher_accountid_label;
    
    //Account Name (Dropdown box)
    QComboBox * withdraw_asvoucher_account_dropdown;
    
    //Nym ID (input)
    //Horitzontal Layout for Nym ID Input
    QWidget * withdraw_asvoucher_nym_holder;
    QHBoxLayout * withdraw_asvoucher_nym_hbox;
    
    //Address book (button as Icon)
    QIcon withdraw_asvoucher_nym_addressbook_icon;
    QPushButton * withdraw_asvoucher_nym_addressbook_btn;
    
    //QR Code Scanner (button as Icon)
    //TODO ^^
    
    //Amount (in integer for now)
    QLineEdit * withdraw_asvoucher_amount_input;
    
    //Memo (Text box)
    QTextEdit * withdraw_asvoucher_memo_input;
    
    //Activate withdraw button
    QPushButton * withdraw_asvoucher_button;
    
    //Nym Id (type/paste input)
    QLineEdit * withdraw_asvoucher_nym_input;
    
    //Withdraw as Voucher confirmation

    QDialog * withdraw_asvoucher_confirm_dialog;
    
    //Grid layout
    QGridLayout * withdraw_asvoucher_confirm_gridlayout;
    
    //Label
    QLabel * withdraw_asvoucher_confirm_label;
    
    //Label (Amount)
    QLabel * withdraw_asvoucher_confirm_amount_label;
    
    //Backend (Amount)
    int withdraw_asvoucher_confirm_amount_int;
    
    //Confirm/cancel horizontal layout
    QWidget * withdraw_asvoucher_confirm_amount_confirm_cancel_widget;
    QHBoxLayout * withdraw_asvoucher_confirm_amount_confirm_cancel_layout;
    
    //Cancel amount (button)
    QPushButton * withdraw_asvoucher_confirm_amount_btn_cancel;
    
    //Confirm amount (button)
    QPushButton * withdraw_asvoucher_confirm_amount_btn_confirm;

    
signals:

private slots:
    
    void show_addressbook_slot();
    void confirm_amount_dialog_slot();
    void account_dropdown_highlighted_slot(int);
    void confirm_amount_slot();
    void cancel_amount_slot();
    
protected:
    
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // WITHDRAWASVOUCHERWINDOW_H
