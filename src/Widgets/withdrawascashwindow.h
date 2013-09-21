#ifndef WITHDRAWASCASHWINDOW_H
#define WITHDRAWASCASHWINDOW_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QGridLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>

#include "moneychanger.h"


class WithdrawAsCashWindow : public QWidget
{
    Q_OBJECT
public:
    explicit WithdrawAsCashWindow(QWidget *parent = 0);
    void dialog();
    
private:
    /** Withdraw **/
    //As Cash
    bool already_init;
    QDialog * withdraw_ascash_dialog;
    //Grid layout
    QGridLayout * withdraw_ascash_gridlayout;
    
    //Withdraw (as cash) header label
    QLabel * withdraw_ascash_header_label;
    
    //Account Id (label)
    QLabel * withdraw_ascash_accountid_label;
    
    //Account Name (Dropdown box)
    QComboBox * withdraw_ascash_account_dropdown;
    
    //Amount (in integer for now)
    QLineEdit * withdraw_ascash_amount_input;
    
    //Activate withdraw button
    QPushButton * withdraw_ascash_button;
    
    //Withdraw as cash confirmation
    bool withdraw_ascash_confirm_dialog_already_init;
    QDialog * withdraw_ascash_confirm_dialog;
    
    //Grid layout
    QGridLayout * withdraw_ascash_confirm_gridlayout;
    
    //Label
    QLabel * withdraw_ascash_confirm_label;
    
    //Label (Amount)
    QLabel * withdraw_ascash_confirm_amount_label;
    
    //Backend (Amount)
    bool withdraw_ascash_confirm_amount_int;
    
    //Confirm/Cancel horizontal layout
    QWidget * withdraw_ascash_confirm_amount_confirm_cancel_widget;
    QHBoxLayout * withdraw_ascash_confirm_amount_confirm_cancel_layout;
    
    //Cancel amount (button)
    QPushButton * withdraw_ascash_confirm_amount_btn_cancel;
    
    //Confirm amount (button)
    QPushButton * withdraw_ascash_confirm_amount_btn_confirm;
    // ------------------------------------------------


signals:

private slots:
    
    void withdraw_ascash_confirm_amount_dialog_slot();
    void withdraw_ascash_confirm_amount_slot();
    void withdraw_ascash_cancel_amount_slot();
    void withdraw_ascash_account_dropdown_highlighted_slot(int);

protected:
    
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // WITHDRAWASCASHWINDOW_H
