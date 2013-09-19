#include "depositwindow.h"

DepositWindow::DepositWindow(QWidget *parent) :
    QWidget(parent)
{
    mc_deposit_already_init = false;
}

void DepositWindow::dialog(){
    if(mc_deposit_already_init == 0){
        //Init deposit, then show.
        mc_deposit_dialog = new QDialog(0);
        mc_deposit_dialog->setWindowTitle("Deposit | Moneychanger");
        mc_deposit_dialog->installEventFilter(this);
        //Gridlayout
        mc_deposit_gridlayout = new QGridLayout(0);
        mc_deposit_gridlayout->setColumnStretch(0, 1);
        mc_deposit_gridlayout->setColumnStretch(1,0);
        mc_deposit_dialog->setLayout(mc_deposit_gridlayout);
        
        //Label (header)
        mc_deposit_header_label = new QLabel("<h1>Deposit</h1>");
        mc_deposit_header_label->setAlignment(Qt::AlignRight);
        mc_deposit_gridlayout->addWidget(mc_deposit_header_label, 0,1, 1,1);
        //Label ("Into Account") (subheader)
        mc_deposit_account_header_label = new QLabel("<h3>Into Account</h3>");
        mc_deposit_account_header_label->setAlignment(Qt::AlignRight);
        mc_deposit_gridlayout->addWidget(mc_deposit_account_header_label, 1,1, 1,1);
        
        //Label ("Into Purse") (subheader)
        mc_deposit_purse_header_label = new QLabel("<h3>Into Purse</h3>");
        mc_deposit_purse_header_label->setAlignment(Qt::AlignRight);
        mc_deposit_gridlayout->addWidget(mc_deposit_purse_header_label, 1,1, 1,1);
        mc_deposit_purse_header_label->hide();
        
        
        //Combobox (choose deposit type)
        mc_deposit_deposit_type = new QComboBox(0);
        mc_deposit_deposit_type->setStyleSheet("QComboBox{padding:1em;}");
        mc_deposit_gridlayout->addWidget(mc_deposit_deposit_type, 0,0, 1,1, Qt::AlignHCenter);
        mc_deposit_deposit_type->addItem("Deposit into your Account", QVariant(0));
        mc_deposit_deposit_type->addItem("Deposit into your Purse", QVariant(1));
        //connect "update" to switching open depsoit account/purse screens.
        connect(mc_deposit_deposit_type, SIGNAL(currentIndexChanged(int)), this, SLOT(mc_deposit_type_changed_slot(int)));
        
        /** Deposit into Account **/
        mc_deposit_account_widget = new QWidget(0);
        mc_deposit_account_layout = new QHBoxLayout(0);
        mc_deposit_account_widget->setLayout(mc_deposit_account_layout);
        mc_deposit_gridlayout->addWidget(mc_deposit_account_widget, 1,0, 1,1);
        //Add to account screen
        
        /** Deposit into Purse **/
        mc_deposit_purse_widget = new QWidget(0);
        mc_deposit_purse_layout = new QHBoxLayout(0);
        mc_deposit_purse_widget->setLayout(mc_deposit_purse_layout);
        mc_deposit_gridlayout->addWidget(mc_deposit_purse_widget, 1,0, 1,1);
        //Add to purse screen
        
        
        //Hide by default
        mc_deposit_purse_widget->hide();
    }
    //Resize
    mc_deposit_dialog->resize(600, 300);
    //Show
    mc_deposit_dialog->show();
}


void DepositWindow::mc_deposit_type_changed_slot(int newIndex){
    /** 0 = Account; 1 = purse **/
    if(newIndex == 0){
        //Show account, hide purse.
        mc_deposit_account_widget->show();
        mc_deposit_account_header_label->show();
        
        mc_deposit_purse_widget->hide();
        mc_deposit_purse_header_label->hide();
        
    }else if(newIndex == 1){
        //Hide account, show purse.
        mc_deposit_account_widget->hide();
        mc_deposit_account_header_label->hide();
        
        mc_deposit_purse_header_label->show();
        mc_deposit_purse_widget->show();
        
    }
}


bool DepositWindow::eventFilter(QObject *obj, QEvent *event){
    
    if (event->type() == QEvent::Close) {
        ((Moneychanger *)parentWidget())->close_deposit_dialog();
        return true;
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape){
            mc_deposit_dialog->close();
            return true;
        }
        return true;
    }else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}