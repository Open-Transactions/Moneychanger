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
    
    bool mc_sendfunds_already_init;
    
    QDialog * mc_sendfunds_dialog;
    QGridLayout * mc_sendfunds_gridlayout;
    QComboBox * mc_sendfunds_sendtype_combobox;


signals:

private slots:
    
protected:
    
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // SENDFUNDSWINDOW_H
