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
    
    bool mc_deposit_already_init;
    
    /** Deposit **/
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
    
signals:

private slots:
    
    //The user changed the "deposit type" switch open/available menu
    void mc_deposit_type_changed_slot(int);
    
protected:
    
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // DEPOSITWINDOW_H
