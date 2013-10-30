#include "withdrawasvoucherwindow.h"

WithdrawAsVoucherWindow::WithdrawAsVoucherWindow(QWidget *parent) :
    QWidget(parent)
{
    
    //As Voucher
    already_init = false;
    confirm_dialog_already_init = false;    
    
}


void WithdrawAsVoucherWindow::dialog()
{
    /** If the withdraw as voucher dialog has already been init,
     *  just show it, Other wise, init and show if this is the
     *  first time.
     **/
    if (!already_init)
    {
        //Init, then show
        //Init
        withdraw_asvoucher_dialog = new QDialog(0);
        withdraw_asvoucher_dialog->installEventFilter(this);
        /** window properties **/
        //Set window title
        withdraw_asvoucher_dialog->setWindowTitle(tr("Withdraw as Voucher | Moneychanger"));
        //withdraw_asvoucher_dialog->setWindowFlags(Qt::WindowStaysOnTopHint);
        
        /** layout and content **/
        //Grid layout Input
        withdraw_asvoucher_gridlayout = new QGridLayout(0);
        withdraw_asvoucher_dialog->setLayout(withdraw_asvoucher_gridlayout);
        
        //Label (withdraw as voucher)
        withdraw_asvoucher_header_label = new QLabel(QString("<h3>%1</h3>").arg(tr("Withdraw as Voucher")), 0);
        withdraw_asvoucher_header_label->setAlignment(Qt::AlignRight);
        withdraw_asvoucher_gridlayout->addWidget(withdraw_asvoucher_header_label, 0,0, 1,1);
        
        //Account ID (label) Note: Value is set when the dropdown box is selected and/or highlighted
        withdraw_asvoucher_accountid_label = new QLabel("", 0);
        withdraw_asvoucher_accountid_label->setAlignment(Qt::AlignHCenter);
        withdraw_asvoucher_gridlayout->addWidget(withdraw_asvoucher_accountid_label, 1,0, 1,1);
        
        //Account Dropdown (combobox)
        withdraw_asvoucher_account_dropdown = new QComboBox(0);
        
        withdraw_asvoucher_account_dropdown->setStyleSheet("QComboBox{padding:0.5em;}");
        withdraw_asvoucher_gridlayout->addWidget(withdraw_asvoucher_account_dropdown, 2,0, 1,1);
        
        //Make connection to "hovering over items" to showing their IDs above the combobox (for user clarity and backend id indexing)
        connect(withdraw_asvoucher_account_dropdown, SIGNAL(highlighted(int)), this, SLOT(account_dropdown_highlighted_slot(int)));
        
        //To Nym ID
        //Horizontal Box (to hold Nym Id input/Address Box Icon/QR Code Scanner Icon)
        withdraw_asvoucher_nym_holder = new QWidget(0);
        withdraw_asvoucher_nym_hbox = new QHBoxLayout(0);
        withdraw_asvoucher_nym_hbox->setMargin(0);
        withdraw_asvoucher_nym_holder->setLayout(withdraw_asvoucher_nym_hbox);
        withdraw_asvoucher_gridlayout->addWidget(withdraw_asvoucher_nym_holder, 3,0, 1,1);
        
        //Nym ID (Paste input)
        withdraw_asvoucher_nym_input = new QLineEdit;
        withdraw_asvoucher_nym_input->setPlaceholderText(tr("Recipient Nym Id"));
        withdraw_asvoucher_nym_input->setStyleSheet("QLineEdit{padding:0.5em;}");
        withdraw_asvoucher_nym_hbox->addWidget(withdraw_asvoucher_nym_input);
        
        
        //Address Book (button)
        withdraw_asvoucher_nym_addressbook_icon = QIcon(":/icons/addressbook");
        withdraw_asvoucher_nym_addressbook_btn = new QPushButton(withdraw_asvoucher_nym_addressbook_icon, "", 0);
        withdraw_asvoucher_nym_addressbook_btn->setStyleSheet("QPushButton{padding:0.5em;}");
        withdraw_asvoucher_nym_hbox->addWidget(withdraw_asvoucher_nym_addressbook_btn);
        //Connect Address book button with a re-action
        connect(withdraw_asvoucher_nym_addressbook_btn, SIGNAL(clicked()), this, SLOT(show_addressbook_slot()));
        
        //QR Code scanner (button)
        //TO DO^^
        
        
        //Amount input
        withdraw_asvoucher_amount_input = new QLineEdit;
        withdraw_asvoucher_amount_input->setPlaceholderText(tr("Amount as Integer"));
        withdraw_asvoucher_amount_input->setStyleSheet("QLineEdit{padding:0.5em;}");
        withdraw_asvoucher_gridlayout->addWidget(withdraw_asvoucher_amount_input, 4,0, 1,1);
        
        //Memo input box
        withdraw_asvoucher_memo_input = new QTextEdit(tr("Memo"), 0);
        withdraw_asvoucher_gridlayout->addWidget(withdraw_asvoucher_memo_input, 5,0, 1,1);
        
        //Withdraw Button
        withdraw_asvoucher_button = new QPushButton(tr("Withdraw as Voucher"));
        withdraw_asvoucher_button->setStyleSheet("QPushButton{padding:1em;}");
        withdraw_asvoucher_gridlayout->addWidget(withdraw_asvoucher_button, 6,0, 1,1);
        //Connect button with re-action
        connect(withdraw_asvoucher_button, SIGNAL(clicked()), this, SLOT(confirm_amount_dialog_slot()));
        
        /** Flag as init **/
        already_init = true;
    }
    //Resize & Show
    withdraw_asvoucher_dialog->resize(400, 120);
    withdraw_asvoucher_dialog->show();
    withdraw_asvoucher_dialog->setFocus();
    
    /** Refresh dynamic lists **/
    //remove all items from nym dropdown box
    while (withdraw_asvoucher_account_dropdown->count() > 0)
        withdraw_asvoucher_account_dropdown->removeItem(0);
    
    for(int a = 0; a < ((Moneychanger *)parentWidget())->get_nym_list_id_size(); a++){
        //Add to combobox
        //Get OT Account ID
        withdraw_asvoucher_account_dropdown->addItem( ((Moneychanger *)parentWidget())->get_account_name_at(a), ((Moneychanger *)parentWidget())->get_account_id_at(a));
    }
}

//Withdraw Slots

//This will show the address book, the opened address book will be set to paste in recipient nym ids if/when selecting a nymid in the addressbook.
void WithdrawAsVoucherWindow::show_addressbook_slot(){
    //Show address book
    ((Moneychanger *)parentWidget())->mc_addressbook_show(QString(""));
}

/**
 ** Button from dialog window has been activated;
 ** Confirm amount;
 ** Upon confirmation call OT withdraw_voucher()
 **/
void WithdrawAsVoucherWindow::confirm_amount_dialog_slot(){
    //Close the (withdraw as voucher) dialog
    withdraw_asvoucher_dialog->hide();
    
    //First confirm this is the correct amount before calling OT
    //Has this dialog already been init before?
    if(!confirm_dialog_already_init){
        //First time init
        withdraw_asvoucher_confirm_dialog = new QDialog(0);
        //Set window properties
        withdraw_asvoucher_confirm_dialog->setWindowTitle("Confirm Amount | Withdraw as Voucher | Moneychanger");
        //withdraw_asvoucher_confirm_dialog->setWindowFlags(Qt::WindowStaysOnTopHint);
        
        //Attach layout
        withdraw_asvoucher_confirm_gridlayout = new QGridLayout(0);
        withdraw_asvoucher_confirm_dialog->setLayout(withdraw_asvoucher_confirm_gridlayout);
        
        
        //Ask the operator to confirm the amount
        //Ask Label
        withdraw_asvoucher_confirm_label = new QLabel(QString("<h3>%1</h3>").arg(tr("Please confirm the amount to withdraw.")), 0);
        withdraw_asvoucher_confirm_label->setAlignment(Qt::AlignRight);
        withdraw_asvoucher_confirm_gridlayout->addWidget(withdraw_asvoucher_confirm_label, 0,0, 1,1);
        
        //Label (Amount)
        QString confirm_amount_string = "<b>"+withdraw_asvoucher_amount_input->text()+"</b>";
        withdraw_asvoucher_confirm_amount_label = new QLabel(confirm_amount_string);
        withdraw_asvoucher_confirm_amount_label->setAlignment(Qt::AlignHCenter);
        withdraw_asvoucher_confirm_gridlayout->addWidget(withdraw_asvoucher_confirm_amount_label, 1,0, 1,1);
        
        
        //Set Withdraw as voucher amount int
        QString confirm_amount_string_int = withdraw_asvoucher_amount_input->text();
        withdraw_asvoucher_confirm_amount_int = confirm_amount_string_int.toInt();
        
        
        //Spacer
        
        //Horizontal Box
        withdraw_asvoucher_confirm_amount_confirm_cancel_widget = new QWidget(0);
        withdraw_asvoucher_confirm_amount_confirm_cancel_layout = new QHBoxLayout(0);
        withdraw_asvoucher_confirm_amount_confirm_cancel_widget->setLayout(withdraw_asvoucher_confirm_amount_confirm_cancel_layout);
        withdraw_asvoucher_confirm_gridlayout->addWidget(withdraw_asvoucher_confirm_amount_confirm_cancel_widget, 3, 0, 1, 1);
        
        
        //Button (Cancel amount)
        withdraw_asvoucher_confirm_amount_btn_cancel = new QPushButton(tr("Cancel Amount"), 0);
        withdraw_asvoucher_confirm_amount_confirm_cancel_layout->addWidget(withdraw_asvoucher_confirm_amount_btn_cancel);
        //Connect the cancel button with a re-action
        connect(withdraw_asvoucher_confirm_amount_btn_cancel, SIGNAL(clicked()), this, SLOT(cancel_amount_slot()));
        
        //Button (Confirm amount)
        withdraw_asvoucher_confirm_amount_btn_confirm = new QPushButton(tr("Confirm Amount"), 0);
        withdraw_asvoucher_confirm_amount_confirm_cancel_layout->addWidget(withdraw_asvoucher_confirm_amount_btn_confirm);
        //Connect the Confirm button with a re-action
        connect(withdraw_asvoucher_confirm_amount_btn_confirm, SIGNAL(clicked()), this, SLOT(confirm_amount_slot()));
        
        
        /** Flag already init **/
        confirm_dialog_already_init = true;
    }

    //Show
    withdraw_asvoucher_confirm_dialog->show();
    withdraw_asvoucher_confirm_dialog->setFocus();
}

//This is activated when the user clicks "Confirm amount"
void WithdrawAsVoucherWindow::confirm_amount_slot(){
    //Close the dialog/window
    withdraw_asvoucher_confirm_dialog->hide();
    
    //Collect require information to call the OT_ME::withdraw_cash(?,?,?) function
    QString selected_account_id = withdraw_asvoucher_account_dropdown->itemData(withdraw_asvoucher_account_dropdown->currentIndex()).toString();
    std::string selected_account_id_string = selected_account_id.toStdString();
    
    QString amount_to_withdraw_string = withdraw_asvoucher_amount_input->text();
    int64_t amount_to_withdraw_int = amount_to_withdraw_string.toInt();
    
    //Get Nym ID
    std::string nym_id = OTAPI_Wrap::GetAccountWallet_NymID(selected_account_id_string);
    
    //Get Server ID
    std::string selected_server_id_string = OTAPI_Wrap::GetAccountWallet_ServerID(selected_account_id_string);
    
    //Get receipent nym id
    std::string recip_nym_string = QString(withdraw_asvoucher_nym_input->text()).toStdString();
    
    //Get memo string
    std::string memo_string = QString(withdraw_asvoucher_memo_input->toPlainText()).toStdString();
    
    //Call OTAPI Withdraw voucher
    std::string withdraw_voucher_response = ((Moneychanger *)parentWidget())->ot_withdraw_voucher(selected_server_id_string, nym_id, selected_account_id_string, recip_nym_string, memo_string, amount_to_withdraw_int);
    qDebug() << QString::fromStdString(withdraw_voucher_response);
    
}

//This is activated when the user clicks "cancel confirmation amount"
void WithdrawAsVoucherWindow::cancel_amount_slot(){
    //Close the dialog/window
    withdraw_asvoucher_confirm_dialog->hide();
}


/**
 ** This will be triggered when the user hovers over a dropdown (combobox) item in the withdraw as voucher account selection
 **/
void WithdrawAsVoucherWindow::account_dropdown_highlighted_slot(int dropdown_index){
    //Change Account ID label to the highlighted(bymouse) dropdown index.
    withdraw_asvoucher_accountid_label->setText(withdraw_asvoucher_account_dropdown->itemData(dropdown_index).toString());
}



bool WithdrawAsVoucherWindow::eventFilter(QObject *obj, QEvent *event){
    
    if (event->type() == QEvent::Close) {
        ((Moneychanger *)parentWidget())->close_withdrawasvoucher_dialog();
        return true;
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape){
            withdraw_asvoucher_dialog->close();
            return true;
        }
        return true;
    }else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}
