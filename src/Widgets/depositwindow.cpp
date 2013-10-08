#include "depositwindow.h"

DepositWindow::DepositWindow(QWidget *parent) :
    QWidget(parent)
{
    already_init = false;
}

void DepositWindow::dialog()
{
    if (!already_init)
    {
        //Init deposit, then show.
        deposit_dialog = new QDialog(0);
        deposit_dialog->setWindowTitle("Deposit | Moneychanger");
        deposit_dialog->installEventFilter(this);
        //Gridlayout
        deposit_gridlayout = new QGridLayout(0);
        deposit_gridlayout->setColumnStretch(0, 1);
        deposit_gridlayout->setColumnStretch(1,0);
        deposit_dialog->setLayout(deposit_gridlayout);
        
        //Label (header)
        deposit_header_label = new QLabel("<h1>Deposit</h1>");
        deposit_header_label->setAlignment(Qt::AlignRight);
        deposit_gridlayout->addWidget(deposit_header_label, 0,1, 1,1);
        //Label ("Into Account") (subheader)
        deposit_account_header_label = new QLabel("<h3>Into Account</h3>");
        deposit_account_header_label->setAlignment(Qt::AlignRight);
        deposit_gridlayout->addWidget(deposit_account_header_label, 1,1, 1,1);
        
        //Label ("Into Purse") (subheader)
        deposit_purse_header_label = new QLabel("<h3>Into Purse</h3>");
        deposit_purse_header_label->setAlignment(Qt::AlignRight);
        deposit_gridlayout->addWidget(deposit_purse_header_label, 1,1, 1,1);
        deposit_purse_header_label->hide();
        
        
        //Combobox (choose deposit type)
        deposit_type = new QComboBox(0);
        deposit_type->setStyleSheet("QComboBox{padding:1em;}");
        deposit_gridlayout->addWidget(deposit_type, 0,0, 1,1, Qt::AlignHCenter);
        deposit_type->addItem("Deposit into your Account", QVariant(0));
        deposit_type->addItem("Deposit into your Purse", QVariant(1));
        //connect "update" to switching open depsoit account/purse screens.
        connect(deposit_type, SIGNAL(currentIndexChanged(int)), this, SLOT(deposit_type_changed_slot(int)));
        
        /** Deposit into Account **/
        deposit_account_widget = new QWidget(0);
        deposit_account_layout = new QHBoxLayout(0);
        deposit_account_widget->setLayout(deposit_account_layout);
        deposit_gridlayout->addWidget(deposit_account_widget, 1,0, 1,1);
        //Add to account screen
        
        /** Deposit into Purse **/
        deposit_purse_widget = new QWidget(0);
        deposit_purse_layout = new QHBoxLayout(0);
        deposit_purse_widget->setLayout(deposit_purse_layout);
        deposit_gridlayout->addWidget(deposit_purse_widget, 1,0, 1,1);
        //Add to purse screen
        
        
        //Hide by default
        deposit_purse_widget->hide();
        already_init = true;
    }
    //Resize
    deposit_dialog->resize(600, 300);
    //Show
    deposit_dialog->show();
}


void DepositWindow::deposit_type_changed_slot(int newIndex)
{
    /** 0 = Account; 1 = purse **/
    if(newIndex == 0){
        //Show account, hide purse.
        deposit_account_widget->show();
        deposit_account_header_label->show();
        
        deposit_purse_widget->hide();
        deposit_purse_header_label->hide();
        
    }else if(newIndex == 1){
        //Hide account, show purse.
        deposit_account_widget->hide();
        deposit_account_header_label->hide();
        
        deposit_purse_header_label->show();
        deposit_purse_widget->show();
        
    }
}


bool DepositWindow::eventFilter(QObject *obj, QEvent *event){
    
    if (event->type() == QEvent::Close) {
        ((Moneychanger *)parentWidget())->close_deposit_dialog();
        return true;
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape){
            deposit_dialog->close();
            return true;
        }
        return true;
    }else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}
