#include "requestfundswindow.h"

RequestFundsWindow::RequestFundsWindow(QWidget *parent) :
    QWidget(parent), already_init(false), requestfunds_dialog(NULL), requestfunds_gridlayout(NULL), requestfunds_sendtype_combobox(NULL)
{

}



void RequestFundsWindow::dialog()
{
    if (!already_init)
    {
        requestfunds_dialog = new QDialog(0);
        requestfunds_dialog->installEventFilter(this);
        requestfunds_gridlayout = new QGridLayout(0);
        requestfunds_dialog->setLayout(requestfunds_gridlayout);
        //Set window title
        requestfunds_dialog->setWindowTitle("Request Payment | Moneychanger");
        
        //Content
//        //Select requestfunds type
//        mc_requestfunds_requesttype_combobox = new QComboBox(0);
//        mc_requestfunds_requesttype_combobox->setStyleSheet("QComboBox{font-size:15pt;}");
//            //Add selection options
//            mc_requestfunds_requesttype_combobox->addItem("Write a Cheque");

//        requestfunds_gridlayout->addWidget(mc_requestfunds_requesttype_combobox, 0,0, 1,1, Qt::AlignHCenter);
    
        already_init = true;
    }
    
    //Resize
    requestfunds_dialog->resize(500, 300);
    
    //Show
    requestfunds_dialog->show();
    requestfunds_dialog->setFocus();
}


bool RequestFundsWindow::eventFilter(QObject *obj, QEvent *event){
    
    if (event->type() == QEvent::Close) {
        ((Moneychanger *)parentWidget())->close_requestfunds_dialog();
        return true;
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape){
            requestfunds_dialog->close();
            return true;
        }
        return true;
    }else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}
