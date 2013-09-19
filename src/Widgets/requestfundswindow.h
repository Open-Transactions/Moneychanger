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
    
    bool mc_requestfunds_already_init;
    
    QDialog * mc_requestfunds_dialog;
    
    //Gridlayout
    QGridLayout * mc_requestfunds_gridlayout;
    
    //Send funds type selection (combobox)
    QComboBox * mc_requestfunds_sendtype_combobox;

signals:

private slots:
    
protected:
    
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // REQUESTFUNDSWINDOW_H
