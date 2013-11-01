#include "sendfundswindow.h"

SendFundsWindow::SendFundsWindow(QWidget *parent) :
    QWidget(parent), already_init(false), sendfunds_dialog(NULL), sendfunds_gridlayout(NULL), sendfunds_sendtype_combobox(NULL)
{
    
}



void SendFundsWindow::dialog()
{
    if (!already_init)
    {
        sendfunds_dialog = new QDialog(0);
        sendfunds_dialog->installEventFilter(this);
        sendfunds_gridlayout = new QGridLayout(0);
        sendfunds_dialog->setLayout(sendfunds_gridlayout);
        //Set window title
        sendfunds_dialog->setWindowTitle(tr("Send Funds | Moneychanger"));
        
        //Content
        //Select sendfunds type
        sendfunds_sendtype_combobox = new QComboBox(0);
        sendfunds_sendtype_combobox->setStyleSheet("QComboBox{font-size:15pt;}");
        //Add selection options
        sendfunds_sendtype_combobox->addItem(tr("Send a Payment"));
        sendfunds_sendtype_combobox->addItem(tr("Send a Cheque"));
        sendfunds_sendtype_combobox->addItem(tr("Send Cash"));
        sendfunds_sendtype_combobox->addItem(tr("Send an Account Transfer"));
        
        sendfunds_gridlayout->addWidget(sendfunds_sendtype_combobox, 0,0, 1,1, Qt::AlignHCenter);
        already_init = true;
    }
    
    //Resize
    sendfunds_dialog->resize(500, 300);
    
    //Show
    sendfunds_dialog->show();
    sendfunds_dialog->setFocus();
}



bool SendFundsWindow::eventFilter(QObject *obj, QEvent *event)
{
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
