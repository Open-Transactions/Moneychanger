#ifndef DEPOSITWINDOW_H
#define DEPOSITWINDOW_H

#include <QWidget>
#include <QDialog>
#include <QDebug>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QHBoxLayout>

#include "moneychanger.h"

class DepositWindow : public QWidget
{
    Q_OBJECT
public:
    explicit DepositWindow(QWidget *parent = 0);

    void dialog();
    
private:
    
    bool already_init;
    
    /** Deposit **/
    QDialog * deposit_dialog;
    //Gridlayout
    QGridLayout * deposit_gridlayout;
    
    //header (label)
    QLabel * deposit_header_label;
    
    //Dropdown box (combobox) (choose deposit type)
    QComboBox * deposit_type;
    
    /** Deposit into account **/
    QWidget * deposit_account_widget;
    QHBoxLayout * deposit_account_layout;
    //(subheader) Deposit into account
    QLabel * deposit_account_header_label;
    
    /** Deposit into purse **/
    QWidget * deposit_purse_widget;
    QHBoxLayout * deposit_purse_layout;
    //(header) Deposit into purse
    QLabel * deposit_purse_header_label;
    
signals:

private slots:
    
    //The user changed the "deposit type" switch open/available menu
    void deposit_type_changed_slot(int);
    
protected:
    
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // DEPOSITWINDOW_H
