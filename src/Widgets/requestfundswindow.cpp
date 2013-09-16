#include "requestfundswindow.h"

RequestFundsWindow::RequestFundsWindow(QWidget *parent) :
    QWidget(parent)
{
    mc_requestfunds_already_init = false;
}


void RequestFundsWindow::dialog(){
    
    if(!mc_requestfunds_already_init){
        mc_requestfunds_dialog = new QDialog(0);
        mc_requestfunds_dialog->installEventFilter(this);
        mc_requestfunds_gridlayout = new QGridLayout(0);
        mc_requestfunds_dialog->setLayout(mc_requestfunds_gridlayout);
        //Set window title
        mc_requestfunds_dialog->setWindowTitle("Request Funds | Moneychanger");
        
        //Content
        //                                //Select requestfunds type
        //                                mc_requestfunds_requesttype_combobox = new QComboBox(0);
        //                                mc_requestfunds_requesttype_combobox->setStyleSheet("QComboBox{font-size:15pt;}");
        //                                    //Add selection options
        //                                    mc_requestfunds_requesttype_combobox->addItem("Write a Cheque");
        
        //                                mc_requestfunds_gridlayout->addWidget(mc_requestfunds_requesttype_combobox, 0,0, 1,1, Qt::AlignHCenter);
    }
    
    //Resize
    mc_requestfunds_dialog->resize(500, 300);
    
    //Show
    mc_requestfunds_dialog->show();
}


bool RequestFundsWindow::eventFilter(QObject *obj, QEvent *event){
    
    if (event->type() == QEvent::Close) {
        ((Moneychanger *)parentWidget())->close_requestfunds_dialog();
        return true;
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape){
            mc_requestfunds_dialog->close();
            return true;
        }
        return true;
    }else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}
