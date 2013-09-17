#include "sendfundswindow.h"

SendFundsWindow::SendFundsWindow(QWidget *parent) :
    QWidget(parent)
{
    
    mc_sendfunds_dialog = false;
}



void SendFundsWindow::dialog(){
    
    if(mc_sendfunds_already_init == 0){
        mc_sendfunds_dialog = new QDialog(0);
        mc_sendfunds_dialog->installEventFilter(this);
        mc_sendfunds_gridlayout = new QGridLayout(0);
        mc_sendfunds_dialog->setLayout(mc_sendfunds_gridlayout);
        //Set window title
        mc_sendfunds_dialog->setWindowTitle("Send Funds | Moneychanger");
        
        //Content
        //Select sendfunds type
        mc_sendfunds_sendtype_combobox = new QComboBox(0);
        mc_sendfunds_sendtype_combobox->setStyleSheet("QComboBox{font-size:15pt;}");
        //Add selection options
        mc_sendfunds_sendtype_combobox->addItem("Send a Payment");
        mc_sendfunds_sendtype_combobox->addItem("Send a Cheque");
        mc_sendfunds_sendtype_combobox->addItem("Send Cash");
        mc_sendfunds_sendtype_combobox->addItem("Send an Account Transfer");
        
        mc_sendfunds_gridlayout->addWidget(mc_sendfunds_sendtype_combobox, 0,0, 1,1, Qt::AlignHCenter);
    }
    
    //Resize
    mc_sendfunds_dialog->resize(500, 300);
    
    //Show
    mc_sendfunds_dialog->show();
}



bool SendFundsWindow::eventFilter(QObject *obj, QEvent *event){
    
    if (event->type() == QEvent::Close) {
        ((Moneychanger *)parentWidget())->close_sendfunds_dialog();
        return true;
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape){
            mc_sendfunds_dialog->close();
            return true;
        }
        return true;
    }else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}
