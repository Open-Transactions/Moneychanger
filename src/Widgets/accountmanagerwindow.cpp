#include "accountmanagerwindow.h"

AccountManagerWindow::AccountManagerWindow(QWidget *parent) :
QWidget(parent)
{
    
    
    //Account Manager
    mc_accountmanager_already_init = 0;
    mc_accountmanager_refreshing = 0;
    mc_accountmanager_proccessing_dataChanged = 0;
    
    //"Add account" dialog
    mc_accountmanager_addaccount_dialog_already_init = 0;
    mc_accountmanager_addaccount_dialog_advanced_showing = 0;
    
    //"Remove account" dialog
    mc_accountmanager_removeaccount_dialog_already_init = 0;
    
}

void AccountManagerWindow::dialog(){
    
    /** If the account manager dialog has already been init,
     *  just show it, Other wise, init and show if this is the
     *  first time.
     **/
    if(mc_accountmanager_already_init == 0){
        
        //The account Manager has not been init yet; Init, then show it.
        mc_account_manager_dialog = new QDialog(0);
        mc_account_manager_dialog->installEventFilter(this);
        
        /** window properties **/
        //Set window title
        mc_account_manager_dialog->setWindowTitle("Account Manager | Moneychanger");
        
        //Set window on top
        //mc_account_manager_dialog->setWindowFlags(Qt::WindowStaysOnTopHint);
        
        /** layout and content **/
        //Grid layout
        mc_account_manager_gridlayout = new QGridLayout(0);
        mc_account_manager_dialog->setLayout(mc_account_manager_gridlayout);
        
        /* First Row in account manager Grid */
        //Label (header)
        mc_account_manager_label = new QLabel("<h3>Account Manager</h3>", 0);
        mc_account_manager_label->setAlignment(Qt::AlignRight);
        mc_account_manager_gridlayout->addWidget(mc_account_manager_label, 0,0, 1,2, Qt::AlignRight);
        
        
        /* Second Row in account manager Grid */
        /** First column in address book grid (left side) **/
        //Horizontal box (contains: List of accounts, add/remove buttons)
        mc_account_manager_holder = new QWidget(0);
        mc_account_manager_hbox = new QHBoxLayout(0);
        mc_account_manager_holder->setLayout(mc_account_manager_hbox);
        mc_account_manager_gridlayout->addWidget(mc_account_manager_holder);
        
        //Table View (backend and visual init)
        mc_account_manager_tableview_itemmodel = new QStandardItemModel(0,3,0);
        mc_account_manager_tableview_itemmodel->setHorizontalHeaderItem(0, new QStandardItem(QString("Account Display Name")));
        mc_account_manager_tableview_itemmodel->setHorizontalHeaderItem(1, new QStandardItem(QString("Account ID")));
        mc_account_manager_tableview_itemmodel->setHorizontalHeaderItem(2, new QStandardItem(QString("Default")));
        
        //Connect tableviews' backend "dataChanged" signal to a re-action.
        connect(mc_account_manager_tableview_itemmodel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(dataChanged_slot(QModelIndex,QModelIndex)));
        
        mc_account_manager_tableview = new QTableView(0);
        mc_account_manager_tableview->setSelectionMode(QAbstractItemView::SingleSelection);
        mc_account_manager_tableview->setModel(mc_account_manager_tableview_itemmodel);
        mc_account_manager_tableview->setColumnWidth(0, 175);
        mc_account_manager_tableview->setColumnWidth(1, 150);
        mc_account_manager_tableview->setColumnWidth(2, 75);
        mc_account_manager_gridlayout->addWidget(mc_account_manager_tableview, 1,0, 1,1);
        
        
        /** Second column in account manager grid (right side) **/
        //Vertical box (contains: add/remove buttons)
        mc_account_manager_addremove_btngroup_holder = new QWidget(0);
        mc_account_manager_addremove_btngroup_vbox = new QVBoxLayout(0);
        mc_account_manager_addremove_btngroup_holder->setLayout(mc_account_manager_addremove_btngroup_vbox);
        mc_account_manager_addremove_btngroup_vbox->setAlignment(Qt::AlignTop);
        mc_account_manager_gridlayout->addWidget(mc_account_manager_addremove_btngroup_holder, 1,1, 1,1);
        
        //"Add account" button
        mc_account_manager_addremove_btngroup_addbtn = new QPushButton("Add Account", 0);
        mc_account_manager_addremove_btngroup_addbtn->setStyleSheet("QPushButton{padding:0.5em;}");
        mc_account_manager_addremove_btngroup_vbox->addWidget(mc_account_manager_addremove_btngroup_addbtn, Qt::AlignTop);
        //Connect the add account button with a re-action to it being "clicked"
        connect(mc_account_manager_addremove_btngroup_addbtn, SIGNAL(clicked()), this, SLOT(addaccount_slot()));
        
        //"Remove account" button
        mc_account_manager_addremove_btngroup_removebtn = new QPushButton("Remove Account", 0);
        mc_account_manager_addremove_btngroup_removebtn->setStyleSheet("QPushButton{padding:0.5em;}");
        mc_account_manager_addremove_btngroup_vbox->addWidget(mc_account_manager_addremove_btngroup_removebtn, Qt::AlignTop);
        //Connect the remove account button with a re-action to it being "clicked"
        connect(mc_account_manager_addremove_btngroup_removebtn, SIGNAL(clicked()), this, SLOT(removeaccount_slot()));
        
        /** Third column (Most revent error) **/
        //Label
        mc_account_manager_most_recent_erorr = new QLabel("");
        mc_account_manager_gridlayout->addWidget(mc_account_manager_most_recent_erorr, 2,0, 1,2, Qt::AlignHCenter);
        /** Flag as init **/
        mc_accountmanager_already_init = 1;
    }
    /** ***
     ** Resize & Show
     **/
    mc_account_manager_dialog->resize(600, 300);
    mc_account_manager_dialog->show();
    
    /** ***
     ** Data Refresh/Fill Logic
     **/
    //Refresh account manger list
    /** Flag Refreshing account Manger **/
    mc_accountmanager_refreshing = 1;
    
    //remove all rows from the account manager (so we can refresh any newly changed data)
    mc_account_manager_tableview_itemmodel->removeRows(0, mc_account_manager_tableview_itemmodel->rowCount());
    
    //Refresh the account manager
    //Refresh account list (can't be done, there is a glitch where if you open the account manger dialog twice it does wierd things to the systray for account menus )
    //mc_systrayMenu_reload_accountlist();
    
    //Add account id and names to the manager list
    int total_accounts = ((Moneychanger *)parentWidget())->get_account_list_id_size();
    qDebug() << "total: " << total_accounts;
    int row_index = 0;
    for(int a = 0; a < total_accounts; a++){
        //Add account name and id to the list.
        
        
        //Extract stuff for this row
        QString account_id = ((Moneychanger *)parentWidget())->get_account_id_at(a);
        QString account_name = ((Moneychanger *)parentWidget())->get_account_name_at(a);
        qDebug() << "ADDING account ID: " << account_id;
        //Place extracted data into the table view
        QStandardItem * col_one = new QStandardItem(account_name);
        QStandardItem * col_two = new QStandardItem(account_id);
        //Column two is uneditable
        col_two->setEditable(0);
        
        QStandardItem * col_three = new QStandardItem();
        //Column three is a checkmark, we need to set some options in this case.
        col_three->setCheckable(1);
        
        //If this is the default account; if yes, mark as checked
        if(((Moneychanger *)parentWidget())->get_default_account_id() == account_id){
            col_three->setCheckState(Qt::Checked);
        }
        
        mc_account_manager_tableview_itemmodel->setItem(row_index, 0, col_one);
        mc_account_manager_tableview_itemmodel->setItem(row_index, 1, col_two);
        mc_account_manager_tableview_itemmodel->setItem(row_index, 2, col_three);
        
        //Increment index
        row_index += 1;
    }
    /** Unflag as current refreshing **/
    mc_accountmanager_refreshing = 0;
}



// ---------------------------------------------------------
// ACCOUNTS
void AccountManagerWindow::addaccount_slot(){
    //Decide if we should init and show, or just show
    if(mc_accountmanager_addaccount_dialog_already_init == 0){
        //Init, then show.
        mc_account_manager_addaccount_dialog = new QDialog(0);
        mc_account_manager_addaccount_dialog->setWindowTitle("Add Account | Moneychanger");
        mc_account_manager_addaccount_dialog->setModal(1);
        
        //Gridlayout
        mc_account_manager_addaccount_gridlayout = new QGridLayout(0);
        mc_account_manager_addaccount_dialog->setLayout(mc_account_manager_addaccount_gridlayout);
        
        //Label (header)
        mc_account_manager_addaccount_header = new QLabel("<h2>Add Account</h2>",0);
        mc_account_manager_addaccount_gridlayout->addWidget(mc_account_manager_addaccount_header, 0,0, 1,1, Qt::AlignRight);
        
        //Label (Show Advanced Option(s)) (Also a button/connection)
        mc_account_manager_addaccount_subheader_toggleadvanced_options_label = new QLabel("<a href='#'>Advanced Option(s)</a>", 0);
        mc_account_manager_addaccount_gridlayout->addWidget(mc_account_manager_addaccount_subheader_toggleadvanced_options_label, 1,0, 1,1, Qt::AlignRight);
        //Connect with a re-action
        connect(mc_account_manager_addaccount_subheader_toggleadvanced_options_label, SIGNAL(linkActivated(QString)), this, SLOT(dialog_showadvanced_slot(QString)));
        
        
        //Label (instructions)
        mc_account_manager_addaccount_subheader_instructions = new QLabel("Below are some options that will help determine how your Account will be added.");
        mc_account_manager_addaccount_subheader_instructions->setWordWrap(1);
        mc_account_manager_addaccount_subheader_instructions->setStyleSheet("QLabel{padding:0.5em;}");
        mc_account_manager_addaccount_gridlayout->addWidget(mc_account_manager_addaccount_subheader_instructions, 2,0, 1,1);
        
        //Label (Choose Source Question)
        //                    mc_account_manager_addaccount_choosesource_label = new QLabel("<h3>Choose the source of the Account</h3>");
        //                    mc_account_manager_addaccount_choosesource_label->setStyleSheet("QLabel{padding:1em;}");
        //                    mc_account_manager_addaccount_gridlayout->addWidget(mc_account_manager_addaccount_choosesource_label, 3,0, 1,1);
        
        //                    //Combobox (Dropdown box: Choose Source)
        //                    mc_account_manager_addaccount_choosesource_answer_selection = new QComboBox(0);
        //                    mc_account_manager_addaccount_choosesource_answer_selection->addItem("Namecoin");
        //                    mc_account_manager_addaccount_choosesource_answer_selection->addItem("No-Source");
        //                    mc_account_manager_addaccount_choosesource_answer_selection->setCurrentIndex(1);
        //                    mc_account_manager_addaccount_choosesource_answer_selection->setStyleSheet("QComboBox{padding:0.5em;}");
        //                    mc_account_manager_addaccount_gridlayout->addWidget(mc_account_manager_addaccount_choosesource_answer_selection, 4,0, 1,1);
        
        //Create account (button)
        mc_account_manager_addaccount_create_account_btn = new QPushButton("Create a new Account", 0);
        mc_account_manager_addaccount_create_account_btn->setStyleSheet("QPushButton{padding:1em;}");
        mc_account_manager_addaccount_gridlayout->addWidget(mc_account_manager_addaccount_create_account_btn, 5,0, 1,1, Qt::AlignHCenter);
        //Connect create account button with a re-action;
        connect(mc_account_manager_addaccount_create_account_btn, SIGNAL(clicked()), this, SLOT(dialog_createaccount_slot()));
        
        /** Flag as already init **/
        mc_accountmanager_addaccount_dialog_already_init = 1;
    }
    //Resize
    mc_account_manager_addaccount_dialog->resize(400, 290);
    //Show
    mc_account_manager_addaccount_dialog->show();
}

void AccountManagerWindow::removeaccount_slot(){
    //Init, then show; If already init, then just show
    if(mc_accountmanager_removeaccount_dialog_already_init == 0){
        mc_account_manager_removeaccount_dialog = new QDialog(0);
        mc_account_manager_removeaccount_dialog->setWindowTitle("Remove Account | Moneychanger");
        mc_account_manager_removeaccount_dialog->setModal(1);
        //Grid layout
        mc_account_manager_removeaccount_gridlayout = new QGridLayout(0);
        mc_account_manager_removeaccount_dialog->setLayout(mc_account_manager_removeaccount_gridlayout);
    }
    mc_account_manager_removeaccount_dialog->resize(400,250);
    mc_account_manager_removeaccount_dialog->show();
}

void AccountManagerWindow::dataChanged_slot(QModelIndex topLeft, QModelIndex bottomRight){
    //Ignore triggers while "refreshing" the account manager.
    if(mc_accountmanager_refreshing == 0 && mc_accountmanager_proccessing_dataChanged == 0){
        /** Flag Proccessing dataChanged **/
        mc_accountmanager_proccessing_dataChanged = 1;
        
        /** Proccess the "Display Name" column **/
        if(topLeft.column() == 0){
            //Get the value (as std::string) of the account id
            QStandardItem * account_id_item = mc_account_manager_tableview_itemmodel->item(topLeft.row(), 1);
            QString account_id_string = account_id_item->text();
            std::string account_id = account_id_string.toStdString();
            
            //Get the value (as std::string) of the newly set name of the account id
            QVariant new_account_name_variant = topLeft.data();
            QString new_account_name_string = new_account_name_variant.toString();
            std::string new_account_name = new_account_name_string.toStdString();
            qDebug() << account_id_string;
            //Update the newly set display name for this account in OT ( call ot for updating )
            bool setName_successfull = OTAPI_Wrap::SetAccountWallet_Name(account_id, account_id, new_account_name);
            if(setName_successfull == true){
                //Do nothing (There is nothing that needs to be done)
            }else{
                //Setting of the display name for this account failed, revert value visually, display recent error
                mc_account_manager_most_recent_erorr->setText("<span style='color:#A80000'><b>Renaming that account failed. (Error Code: 100)</b></span>");
            }
        }
        
        /** Proccess the "Default" column (if triggered) **/
        if(topLeft.column() == 2){
            //The (default) 2 column has checked(or unchecked) a box, (uncheck all checkboxes, except the newly selected one)
            int total_checkboxes = mc_account_manager_tableview_itemmodel->rowCount();
            
            //Uncheck all boxes (always checkMarked the triggered box)
            for(int a = 0; a < total_checkboxes; a++){
                
                //Check if this is a checkbox we should checkmark
                if(a != topLeft.row()){
                    
                    //Get checkbox item
                    QStandardItem * checkbox_model = mc_account_manager_tableview_itemmodel->item(a, 2);
                    
                    //Update the checkbox item at the backend.
                    checkbox_model->setCheckState(Qt::Unchecked);
                    
                    //Update the checkbox item visually.
                    mc_account_manager_tableview_itemmodel->setItem(a, 2, checkbox_model);
                }else if(a == topLeft.row()){
                    
                    //Get checkbox item
                    QStandardItem * checkbox_model = mc_account_manager_tableview_itemmodel->item(a, 2);
                    
                    //Update the checkbox item at the backend.
                    //Get account id we are targeting to update.
                    QStandardItem * account_id = mc_account_manager_tableview_itemmodel->item(a, 1);
                    QVariant account_id_variant = account_id->text();
                    QString account_id_string = account_id_variant.toString();
                    QString account_name_string = QString::fromStdString(OTAPI_Wrap::GetAccountWallet_Name(account_id_string.toStdString()));
                    //Update the checkbox item visually.
                    checkbox_model->setCheckState(Qt::Checked);
                    mc_account_manager_tableview_itemmodel->setItem(a, 2, checkbox_model);
                    
                    //Update default account at realtime memory backend
                    
                    ((Moneychanger *)parentWidget())->set_systrayMenu_account_setDefaultAccount(account_id_string, account_name_string);
                    
                    
                }
                
            }
            
        }
        
        /** Unflag Proccessing Data Changed **/
        mc_accountmanager_proccessing_dataChanged = 0;
    }
}


/**** ****
 **** account Manager -> Add account Dialog (Private Slots)
 **** ****/
void AccountManagerWindow::dialog_showadvanced_slot(QString link_href){
    //If advanced options are already showing, hide, if they are hidden, show them.
    if(mc_accountmanager_addaccount_dialog_advanced_showing == 0){
        //Show advanced options.
        //Show the Bits option
        
    }else if(mc_accountmanager_addaccount_dialog_advanced_showing == 1){
        //Hide advanced options.
        //Hide the Bits option
    }
}

void AccountManagerWindow::dialog_createaccount_slot(){
    //            std::string pseudonym = OTAPI_Wrap::CreateNym(1024, "", "");
    //            QString new_pseudonym = QString::fromStdString(pseudonym);
    QString new_account;
    
    //Success if non null
    if(new_account != ""){
        
    }else{
        //Failed to create account
    }
}


void AccountManagerWindow::request_remove_account_slot(){
    //Extract the currently selected account from the account-list.
    QModelIndexList selected_indexes = mc_account_manager_tableview->selectionModel()->selectedIndexes();
    
    for(int a = 0; a < selected_indexes.size(); a++){
        QModelIndex selected_index = selected_indexes.at(a);
        int selected_row = selected_index.row();
        
        //Get account id
        QModelIndex account_id_modelindex = mc_account_manager_tableview_itemmodel->index(selected_row, 1, QModelIndex());
        QVariant account_id_variant = account_id_modelindex.data();
        QString account_id_string = account_id_variant.toString();
        bool can_remove = OTAPI_Wrap::Wallet_CanRemoveAccount(account_id_string.toStdString());
        
        if(can_remove == true){
            //Remove it
            //                    OTAPI_Wrap::Wallet_RemoveAccount(account_id_string.toStdString());
        }else{
            //                    //Find out why it can't be removed and alert the user the reasoning.
            //                        //Loop through nyms
            //                        std::string account_id_std = account_id_string.toStdString();
            //                        int num_nyms_registered_at_account = 0;
            //                        int num_nyms = OTAPI_Wrap::GetNymCount();
            //                        for(int b = 0; b < num_nyms; b++){
            //                            bool nym_index_at_account = OTAPI_Wrap::IsNym_RegisteredAtaccount(OTAPI_Wrap::GetNym_ID(b), account_id_std);
            //                            if(nym_index_at_account == true){
            //                                num_nyms_registered_at_account += 1;
            //                            }
            //                        }
        }
        
    }
    
}



// This event filter catches Esc key and Close events
// So that they get cleaned up in the parentWidget appropriately.
// Note that this won't work if the parentWidget isn't of
// Type Moneychanger, but this can be modified so that Esc events are
// Discarded (Don't do this if the Widget is Modal!)
/*
 
 if(e->key() != Qt::Key_Escape)
 QDialog::keyPressEvent(e);
 else {;}

 */

bool AccountManagerWindow::eventFilter(QObject *obj, QEvent *event){
    
    if (event->type() == QEvent::Close) {
        ((Moneychanger *)parentWidget())->close_accountmanager_dialog();
        return true;
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape){
            mc_account_manager_dialog->close();
            return true;
        }
        return true;
    }else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}
