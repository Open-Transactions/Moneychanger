#include "withdrawascashwindow.h"

WithdrawAsCashWindow::WithdrawAsCashWindow(QWidget *parent) :
    QWidget(parent)
{
    
    //Withdraw
    //As Cash
    already_init = false;
    withdraw_ascash_confirm_dialog_already_init = false;
    
}


void WithdrawAsCashWindow::dialog(){
    {
        /** If the withdraw as cash dialog has already been init,
         *  just show it, Other wise, init and show if this is the
         *  first time.
         **/
        if(!already_init){
            //Init, then show
            //Init
            withdraw_ascash_dialog = new QDialog(0);
            withdraw_ascash_dialog->installEventFilter(this);
            /** window properties **/
            //Set window title
            withdraw_ascash_dialog->setWindowTitle("Withdraw as Cash | Moneychanger");
            //withdraw_ascash_dialog->setWindowFlags(Qt::WindowStaysOnTopHint);
            
            /** layout and content **/
            //Grid layout
            withdraw_ascash_gridlayout = new QGridLayout(0);
            withdraw_ascash_dialog->setLayout(withdraw_ascash_gridlayout);
            
            //Withdraw As Cash (header label)
            withdraw_ascash_header_label = new QLabel("<h3>Withdraw as Cash</h3>", 0);
            withdraw_ascash_header_label->setAlignment(Qt::AlignRight);
            withdraw_ascash_gridlayout->addWidget(withdraw_ascash_header_label, 0, 0, 1, 1);
            
            //Account ID (label) Note: Value is set when the dropdown box is selected and/or highlighted
            withdraw_ascash_accountid_label = new QLabel("", 0);
            withdraw_ascash_accountid_label->setStyleSheet("QLabel{padding:0.5em;}");
            withdraw_ascash_accountid_label->setAlignment(Qt::AlignHCenter);
            withdraw_ascash_gridlayout->addWidget(withdraw_ascash_accountid_label, 1, 0, 1, 1);
            
            //Account Dropdown (combobox)
            withdraw_ascash_account_dropdown = new QComboBox(0);
            withdraw_ascash_account_dropdown->setStyleSheet("QComboBox{padding:0.5em;}");
            withdraw_ascash_gridlayout->addWidget(withdraw_ascash_account_dropdown, 2, 0, 1, 1);
            
            //Make connection to "hovering over items" to showing their IDs above the combobox (for user clarity and backend id indexing)
            connect(withdraw_ascash_account_dropdown, SIGNAL(highlighted(int)), this, SLOT(withdraw_ascash_account_dropdown_highlighted_slot(int)));
            
            //Amount Instructions
            //TODO ^^
            
            //Amount Input
            withdraw_ascash_amount_input = new QLineEdit;
            withdraw_ascash_amount_input->setPlaceholderText("Amount");
            withdraw_ascash_amount_input->setStyleSheet("QLineEdit{padding:0.5em;}");
            withdraw_ascash_gridlayout->addWidget(withdraw_ascash_amount_input, 3, 0, 1, 1);
            
            //Withdraw Button
            withdraw_ascash_button = new QPushButton("Withdraw as Cash");
            withdraw_ascash_button->setStyleSheet("QPushButton{padding:0.5em;}");
            withdraw_ascash_gridlayout->addWidget(withdraw_ascash_button, 4, 0, 1, 1);
            //Connect button with re-action
            connect(withdraw_ascash_button, SIGNAL(pressed()), this, SLOT(withdraw_ascash_confirm_amount_dialog_slot()));
            
            /** Flag already init **/
            already_init = true;
        }
        //Resize
        withdraw_ascash_dialog->resize(400, 120);
        //Show
        withdraw_ascash_dialog->show();
        withdraw_ascash_dialog->activateWindow();
        
        /** Refresh dynamic lists **/
        //remove all items from nym dropdown box
        while (withdraw_ascash_account_dropdown->count() > 0)
            withdraw_ascash_account_dropdown->removeItem(0);
        
        for(int a = 0; a < ((Moneychanger *)parentWidget())->get_nym_list_id_size(); a++){
            //Add to combobox
            //Get OT Account ID
            withdraw_ascash_account_dropdown->addItem(((Moneychanger *)parentWidget())->get_account_name_at(a), ((Moneychanger *)parentWidget())->get_account_id_at(a));
        }
    }
}


/**
 ** Button from dialog window has been activated;
 ** Confirm amount;
 ** Upon confirmation call OT withdraw_cash()
 **/
void WithdrawAsCashWindow::withdraw_ascash_confirm_amount_dialog_slot(){
    //Close the (withdraw as cash) dialog
    withdraw_ascash_dialog->hide();
    
    //First confirm this is the correct amount before calling OT
    //Has this dialog already been init before?
    if(!withdraw_ascash_confirm_dialog_already_init){
        //First time init
        withdraw_ascash_confirm_dialog = new QDialog(0);
        
        //Attach layout
        withdraw_ascash_confirm_gridlayout = new QGridLayout(0);
        withdraw_ascash_confirm_dialog->setLayout(withdraw_ascash_confirm_gridlayout);
        
        //Ask the operator to confirm the amount requested
        withdraw_ascash_confirm_label = new QLabel("Please confirm the amount to withdraw.");
        withdraw_ascash_confirm_gridlayout->addWidget(withdraw_ascash_confirm_label, 0,0, 1,1);
        
        //Label (Amount)
        QString confirm_amount_string = "<b>"+withdraw_ascash_amount_input->text()+"</b>";
        withdraw_ascash_confirm_amount_label = new QLabel(confirm_amount_string);
        withdraw_ascash_confirm_gridlayout->addWidget(withdraw_ascash_confirm_amount_label, 1,0, 1,1);
        
        //Set Withdraw as cash amount int
        QString confirm_amount_string_int = withdraw_ascash_amount_input->text();
        
        withdraw_ascash_confirm_amount_int = confirm_amount_string_int.toInt();
        
        //Spacer
        
        //Horizontal Box
        withdraw_ascash_confirm_amount_confirm_cancel_widget = new QWidget(0);
        withdraw_ascash_confirm_amount_confirm_cancel_layout = new QHBoxLayout(0);
        withdraw_ascash_confirm_amount_confirm_cancel_widget->setLayout(withdraw_ascash_confirm_amount_confirm_cancel_layout);
        withdraw_ascash_confirm_gridlayout->addWidget(withdraw_ascash_confirm_amount_confirm_cancel_widget, 3, 0, 1, 1);
        
        //Button (Cancel amount)
        withdraw_ascash_confirm_amount_btn_cancel = new QPushButton("Cancel Amount", 0);
        withdraw_ascash_confirm_amount_confirm_cancel_layout->addWidget(withdraw_ascash_confirm_amount_btn_cancel);
        //Connect the cancel button with a re-action
        connect(withdraw_ascash_confirm_amount_btn_cancel, SIGNAL(clicked()), this, SLOT(withdraw_ascash_cancel_amount_slot()));
        
        //Button (Confirm amount)
        withdraw_ascash_confirm_amount_btn_confirm = new QPushButton("Confirm Amount", 0);
        withdraw_ascash_confirm_amount_confirm_cancel_layout->addWidget(withdraw_ascash_confirm_amount_btn_confirm);
        //Connect the Confirm button with a re-action
        connect(withdraw_ascash_confirm_amount_btn_confirm, SIGNAL(clicked()), this, SLOT(withdraw_ascash_confirm_amount_slot()));
        
        /** Flag already init **/
        withdraw_ascash_confirm_dialog_already_init = true;
        
        //Show
        withdraw_ascash_confirm_dialog->show();
        
        
    }else{
        //Not first time init, just show the dialog.
        
        //Set Withdraw as cash amount int
        QString confirm_amount_string_int = withdraw_ascash_amount_input->text();
        withdraw_ascash_confirm_amount_int = confirm_amount_string_int.toInt();
        
        //Show dialog.
        withdraw_ascash_confirm_dialog->show();
    }
    
}



/**
 ** This will be triggered when the user click the "confirm amount" button from the withdraw/confirm dialog
 **/

void WithdrawAsCashWindow::withdraw_ascash_confirm_amount_slot(){
    //Close the dialog/window
    withdraw_ascash_confirm_dialog->hide();
    
    //Collect require information to call the OT_ME::withdraw_cash(?,?,?) function
    QString selected_account_id = withdraw_ascash_account_dropdown->itemData(withdraw_ascash_account_dropdown->currentIndex()).toString();
    std::string selected_account_id_string = selected_account_id.toStdString();
    
    QString amount_to_withdraw_string = withdraw_ascash_amount_input->text();
    int64_t amount_to_withdraw_int = amount_to_withdraw_string.toInt();
    
    //Get Nym ID
    std::string nym_id = OTAPI_Wrap::GetAccountWallet_NymID(selected_account_id_string);
    
    //Get Server ID
    std::string selected_server_id_string = OTAPI_Wrap::GetAccountWallet_ServerID(selected_account_id_string);
    
    //Call OTAPI Withdraw cash
    std::string withdraw_cash_response = ((Moneychanger *)parentWidget())->ot_withdraw_cash(selected_server_id_string, nym_id, selected_account_id_string, amount_to_withdraw_int);
    //qDebug() << QString::fromStdString(withdraw_cash_response);
    
    
}

/**
 ** This will be triggered when the user click the "cancel amount" button from the withdraw/confirm dialog
 **/
void WithdrawAsCashWindow::withdraw_ascash_cancel_amount_slot(){
    //Close the dialog/window
    withdraw_ascash_confirm_dialog->hide();
}



/**
 ** This will display the account id that the user has selected (for convience also for backend id tracking)
 **/
void WithdrawAsCashWindow::withdraw_ascash_account_dropdown_highlighted_slot(int dropdown_index){
    //Change Account ID label to the highlighted(bymouse) dropdown index.
    withdraw_ascash_accountid_label->setText(withdraw_ascash_account_dropdown->itemData(dropdown_index).toString());
}



bool WithdrawAsCashWindow::eventFilter(QObject *obj, QEvent *event){
    
    if (event->type() == QEvent::Close) {
        ((Moneychanger *)parentWidget())->close_withdrawascash_dialog();
        return true;
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape){
            withdraw_ascash_dialog->close();
            return true;
        }
        return true;
    }else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

