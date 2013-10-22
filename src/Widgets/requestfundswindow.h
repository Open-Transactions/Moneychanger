#ifndef REQUESTFUNDSWINDOW_H
#define REQUESTFUNDSWINDOW_H

#include <QWidget>
#include <QDialog>
#include <QGridLayout>
#include <QDebug>
#include <QComboBox>

#include "moneychanger.h"

class RequestFundsWindow : public QWidget
{
    Q_OBJECT
public:
    explicit RequestFundsWindow(QWidget *parent = 0);
    void dialog();
    
private:
    
    bool already_init;
    
    QDialog * requestfunds_dialog;
    QGridLayout * requestfunds_gridlayout;
    QComboBox * requestfunds_sendtype_combobox;

signals:

private slots:
    
protected:
    
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // REQUESTFUNDSWINDOW_H
