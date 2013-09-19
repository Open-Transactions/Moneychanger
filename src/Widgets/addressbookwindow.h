#ifndef ADDRESSBOOKWINDOW_H
#define ADDRESSBOOKWINDOW_H

#include <QWidget>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QString>

#include "moneychanger.h"
#include "Handlers/DBHandler.h"


class AddressBookWindow : public QWidget
{
    Q_OBJECT
public:
    explicit AddressBookWindow(QWidget *parent = 0);
    
    void show();
    void show(QString paste_selection_to);
    
private:
    
    bool already_init;
    bool refreshing;
    
    QString paste_into;
    
    QDialog * dialog;
    QGridLayout * gridlayout;
    QLabel * label;
    QStandardItemModel * tableview_itemmodel;
    QTableView * tableview;
    
    QWidget * addremove_btngroup_widget;
    QVBoxLayout * addremove_btngroup_holder;
    QPushButton * addremove_add_btn;
    QPushButton * addremove_remove_btn;
    QPushButton * select_nym_for_paste_btn;
    
signals:
    
public slots:
    
    void paste_selected_slot();
    void addblankrow_slot();
    void confirm_remove_contact_slot();
    void dataChanged_slot(QModelIndex topLeft, QModelIndex bottomRight);
    
    
protected:
    
    bool eventFilter(QObject *obj, QEvent *event);


    
};

#endif // ADDRESSBOOKWINDOW_H
