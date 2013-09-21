#ifndef SENDFUNDSWINDOW_H
#define SENDFUNDSWINDOW_H

#include <QWidget>
#include <QDialog>
#include <QComboBox>
#include <QGridLayout>
#include <QDialog>

#include "moneychanger.h"

class SendFundsWindow : public QWidget
{
    Q_OBJECT
public:
    explicit SendFundsWindow(QWidget *parent = 0);
    void dialog();
    
private:
    
    bool already_init;
    
    QDialog * sendfunds_dialog;
    QGridLayout * sendfunds_gridlayout;
    QComboBox * sendfunds_sendtype_combobox;


signals:

private slots:
    
protected:
    
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // SENDFUNDSWINDOW_H
