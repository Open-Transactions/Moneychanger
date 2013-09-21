#include "sendfundswindow.h"

SendFundsWindow::SendFundsWindow(QWidget *parent) :
    QWidget(parent)
{
    
    already_init = false;

}



void SendFundsWindow::dialog(){
    
    if(!already_init){
        sendfunds_dialog = new QDialog(0);
        sendfunds_dialog->installEventFilter(this);
        sendfunds_gridlayout = new QGridLayout(0);
        sendfunds_dialog->setLayout(sendfunds_gridlayout);
        //Set window title
        sendfunds_dialog->setWindowTitle("Send Funds | Moneychanger");
        
        //Content
        //Select sendfunds type
        sendfunds_sendtype_combobox = new QComboBox(0);
        sendfunds_sendtype_combobox->setStyleSheet("QComboBox{font-size:15pt;}");
        //Add selection options
        sendfunds_sendtype_combobox->addItem("Send a Payment");
        sendfunds_sendtype_combobox->addItem("Send a Cheque");
        sendfunds_sendtype_combobox->addItem("Send Cash");
        sendfunds_sendtype_combobox->addItem("Send an Account Transfer");
        
        sendfunds_gridlayout->addWidget(sendfunds_sendtype_combobox, 0,0, 1,1, Qt::AlignHCenter);
        already_init = true;
    }
    
    //Resize
    sendfunds_dialog->resize(500, 300);
    
    //Show
    sendfunds_dialog->show();
}



bool SendFundsWindow::eventFilter(QObject *obj, QEvent *event){
    
    if (event->type() == QEvent::Close) {
        ((Moneychanger *)parentWidget())->close_sendfunds_dialog();
        return true;
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape){
            sendfunds_dialog->close();
            return true;
        }
        return true;
    }else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}
