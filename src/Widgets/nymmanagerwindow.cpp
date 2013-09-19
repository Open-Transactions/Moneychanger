#include "nymmanagerwindow.h"

NymManagerWindow::NymManagerWindow(QWidget *parent) :
    QWidget(parent)
{
    already_init = false;
    
    refreshing = false;
    mc_nymmanager_proccessing_dataChanged = false;
    
    //"Add Nym" dialog
    mc_nymmanager_addnym_dialog_already_init = false;
    mc_nymmanager_addnym_dialog_advanced_showing = false;
    
    //"Remove Nym" dialog
    mc_nymmanager_removenym_dialog_already_init = false;
    
}

/** Nym Manager Dialog **/
void NymManagerWindow::dialog(){
    
    /** If the nym managerh dialog has already been init,
     *  just show it, Other wise, init and show if this is the
     *  first time.
     **/
    if(already_init == 0){
        
        //The Nym Manager has not been init yet; Init, then show it.
        mc_nym_manager_dialog = new QDialog(0);
        
        mc_nym_manager_dialog->installEventFilter(this);
        
        /** window properties **/
        //Set window title
        mc_nym_manager_dialog->setWindowTitle("Nym Manager | Moneychanger");
        
        //Set window on top
        //mc_nym_manager_dialog->setWindowFlags(Qt::WindowStaysOnTopHint);
        
        /** layout and content **/
        //Grid layout
        mc_nym_manager_gridlayout = new QGridLayout(0);
        mc_nym_manager_dialog->setLayout(mc_nym_manager_gridlayout);
        
        /* First Row in nym manager Grid */
        //Label (header)
        mc_nym_manager_label = new QLabel("<h3>Pseudonyms</h3>", 0);
        mc_nym_manager_label->setAlignment(Qt::AlignRight);
        mc_nym_manager_gridlayout->addWidget(mc_nym_manager_label, 0,0, 1,2, Qt::AlignRight);
        
        
        /* Second Row in nym manager Grid */
        /** First column in address book grid (left side) **/
        //Horizontal box (contains: List of pseudo nyms, add/remove buttons)
        mc_nym_manager_holder = new QWidget(0);
        mc_nym_manager_hbox = new QHBoxLayout(0);
        mc_nym_manager_holder->setLayout(mc_nym_manager_hbox);
        mc_nym_manager_gridlayout->addWidget(mc_nym_manager_holder);
        
        //Table View (backend and visual init)
        mc_nym_manager_tableview_itemmodel = new QStandardItemModel(0,3,0);
        mc_nym_manager_tableview_itemmodel->setHorizontalHeaderItem(0, new QStandardItem(QString("Pseudonym Display Name")));
        mc_nym_manager_tableview_itemmodel->setHorizontalHeaderItem(1, new QStandardItem(QString("Pseudonym ID")));
        mc_nym_manager_tableview_itemmodel->setHorizontalHeaderItem(2, new QStandardItem(QString("Default")));
        
        //Connect tableviews' backend "dataChanged" signal to a re-action.
        connect(mc_nym_manager_tableview_itemmodel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(nymmanager_dataChanged_slot(QModelIndex,QModelIndex)));
        
        mc_nym_manager_tableview = new QTableView(0);
        mc_nym_manager_tableview->setSelectionMode(QAbstractItemView::SingleSelection);
        mc_nym_manager_tableview->setModel(mc_nym_manager_tableview_itemmodel);
        mc_nym_manager_tableview->setColumnWidth(0, 175);
        mc_nym_manager_tableview->setColumnWidth(1, 150);
        mc_nym_manager_tableview->setColumnWidth(2, 75);
        mc_nym_manager_gridlayout->addWidget(mc_nym_manager_tableview, 1,0, 1,1);
        
        
        /** Second column in nym manager grid (right side) **/
        //Vertical box (contains: add/remove buttons)
        mc_nym_manager_addremove_btngroup_holder = new QWidget(0);
        mc_nym_manager_addremove_btngroup_vbox = new QVBoxLayout(0);
        mc_nym_manager_addremove_btngroup_holder->setLayout(mc_nym_manager_addremove_btngroup_vbox);
        mc_nym_manager_addremove_btngroup_vbox->setAlignment(Qt::AlignTop);
        mc_nym_manager_gridlayout->addWidget(mc_nym_manager_addremove_btngroup_holder, 1,1, 1,1);
        
        //"Add Nym" button
        mc_nym_manager_addremove_btngroup_addbtn = new QPushButton("Add Nym", 0);
        mc_nym_manager_addremove_btngroup_addbtn->setStyleSheet("QPushButton{padding:0.5em;}");
        mc_nym_manager_addremove_btngroup_vbox->addWidget(mc_nym_manager_addremove_btngroup_addbtn, Qt::AlignTop);
        //Connect the add nym button with a re-action to it being "clicked"
        connect(mc_nym_manager_addremove_btngroup_addbtn, SIGNAL(clicked()), this, SLOT(nymmanager_addnym_slot()));
        
        //"Remove Nym" button
        mc_nym_manager_addremove_btngroup_removebtn = new QPushButton("Remove Nym", 0);
        mc_nym_manager_addremove_btngroup_removebtn->setStyleSheet("QPushButton{padding:0.5em;}");
        mc_nym_manager_addremove_btngroup_vbox->addWidget(mc_nym_manager_addremove_btngroup_removebtn, Qt::AlignTop);
        //Connect the remove nym button with a re-action to it being "clicked"
        connect(mc_nym_manager_addremove_btngroup_removebtn, SIGNAL(clicked()), this, SLOT(nymmanager_removenym_slot()));
        
        /** Third column (Most revent error) **/
        //Label
        mc_nym_manager_most_recent_erorr = new QLabel("");
        mc_nym_manager_gridlayout->addWidget(mc_nym_manager_most_recent_erorr, 2,0, 1,2, Qt::AlignHCenter);
        /** Flag as init **/
        mc_nymmanager_already_init = 1;
    }
    /** ***
     ** Resize & Show
     **/
    mc_nym_manager_dialog->resize(600, 300);
    mc_nym_manager_dialog->show();
    
    /** ***
     ** Data Refresh/Fill Logic
     **/
    //Refresh Nym manger list
    /** Flag Refreshing Nym Manger **/
    mc_nymmanager_refreshing = 1;
    
    //remove all rows from the nym manager (so we can refresh any newly changed data)
    mc_nym_manager_tableview_itemmodel->removeRows(0, mc_nym_manager_tableview_itemmodel->rowCount());
    
    //Refresh the nym manager
    //Refresh nym list (can't be done, there is a glitch where if you open the nym manger dialog twice it does wierd things to the systray for nym menus )
    //mc_systrayMenu_reload_nymlist();
    
    //Add nym id and names to the manager list
    int total_nym_accounts = ((Moneychanger *)parentWidget())->get_nym_list_id_size();
    qDebug() << "total: " << total_nym_accounts;
    int row_index = 0;
    for(int a = 0; a < total_nym_accounts; a++){
        //Add nym account name and id to the list.
        
        
        //Extract stuff for this row
        QString nym_id = ((Moneychanger *)parentWidget())->get_nym_id_at(a);//nym_list_id->at(a).toString();
        QString nym_name = ((Moneychanger *)parentWidget())->get_nym_name_at(a);//nym_list_name->at(a).toString();
        qDebug() << "ADDING NYM ID: " << nym_id;
        //Place extracted data into the table view
        QStandardItem * col_one = new QStandardItem(nym_name);
        QStandardItem * col_two = new QStandardItem(nym_id);
        //Column two is uneditable
        col_two->setEditable(0);
        
        QStandardItem * col_three = new QStandardItem();
        //Column three is a checkmark, we need to set some options in this case.
        col_three->setCheckable(1);
        
        //If this is the default pseudonym; if yes, mark as checked
        if(((Moneychanger *)parentWidget())->get_default_nym_id() == nym_id){
            col_three->setCheckState(Qt::Checked);
        }
        
        mc_nym_manager_tableview_itemmodel->setItem(row_index, 0, col_one);
        mc_nym_manager_tableview_itemmodel->setItem(row_index, 1, col_two);
        mc_nym_manager_tableview_itemmodel->setItem(row_index, 2, col_three);
        
        //Increment index
        row_index += 1;
    }
    /** Unflag as current refreshing **/
    mc_nymmanager_refreshing = 0;
}


// ---------------------------------------------------------
/* Nym Slots */
void NymManagerWindow::nymmanager_addnym_slot(){
    //Decide if we should init and show, or just show
    if(mc_nymmanager_addnym_dialog_already_init == 0){
        //Init, then show.
        mc_nym_manager_addnym_dialog = new QDialog(0);
        //mc_nym_manager_addnym_dialog->setWindowFlags(Qt::WindowStaysOnTopHint);
        mc_nym_manager_addnym_dialog->setWindowTitle("Add Pseudonym | Moneychanger");
        mc_nym_manager_addnym_dialog->setModal(1);
        
        //Gridlayout
        mc_nym_manager_addnym_gridlayout = new QGridLayout(0);
        mc_nym_manager_addnym_dialog->setLayout(mc_nym_manager_addnym_gridlayout);
        
        //Label (header)
        mc_nym_manager_addnym_header = new QLabel("<h2>Add Pseudonym</h2>",0);
        mc_nym_manager_addnym_gridlayout->addWidget(mc_nym_manager_addnym_header, 0,0, 1,1, Qt::AlignRight);
        
        //Label (Show Advanced Option(s)) (Also a button/connection)
        mc_nym_manager_addnym_subheader_toggleadvanced_options_label = new QLabel("<a href='#'>Advanced Option(s)</a>", 0);
        mc_nym_manager_addnym_gridlayout->addWidget(mc_nym_manager_addnym_subheader_toggleadvanced_options_label, 1,0, 1,1, Qt::AlignRight);
        //Connect with a re-action
        connect(mc_nym_manager_addnym_subheader_toggleadvanced_options_label, SIGNAL(linkActivated(QString)), this, SLOT(addnym_dialog_showadvanced_slot(QString)));
        
        
        //Label (instructions)
        mc_nym_manager_addnym_subheader_instructions = new QLabel("Below are some options that will help determine how your Pseudonym will be added. Selecting \"No-Source\" is for self-signed Pseudonyms.");
        mc_nym_manager_addnym_subheader_instructions->setWordWrap(1);
        mc_nym_manager_addnym_subheader_instructions->setStyleSheet("QLabel{padding:0.5em;}");
        mc_nym_manager_addnym_gridlayout->addWidget(mc_nym_manager_addnym_subheader_instructions, 2,0, 1,1);
        
        //Label (Choose Source Question)
        mc_nym_manager_addnym_choosesource_label = new QLabel("<h3>Choose the source of the Pseudonym</h3>");
        mc_nym_manager_addnym_choosesource_label->setStyleSheet("QLabel{padding:1em;}");
        mc_nym_manager_addnym_gridlayout->addWidget(mc_nym_manager_addnym_choosesource_label, 3,0, 1,1);
        
        //Combobox (Dropdown box: Choose Source)
        mc_nym_manager_addnym_choosesource_answer_selection = new QComboBox(0);
        mc_nym_manager_addnym_choosesource_answer_selection->addItem("Namecoin");
        mc_nym_manager_addnym_choosesource_answer_selection->addItem("No-Source");
        mc_nym_manager_addnym_choosesource_answer_selection->setCurrentIndex(1);
        mc_nym_manager_addnym_choosesource_answer_selection->setStyleSheet("QComboBox{padding:0.5em;}");
        mc_nym_manager_addnym_gridlayout->addWidget(mc_nym_manager_addnym_choosesource_answer_selection, 4,0, 1,1);
        
        //Create Nym (button)
        mc_nym_manager_addnym_create_nym_btn = new QPushButton("Create a new Pseudonym", 0);
        mc_nym_manager_addnym_create_nym_btn->setStyleSheet("QPushButton{padding:1em;}");
        mc_nym_manager_addnym_gridlayout->addWidget(mc_nym_manager_addnym_create_nym_btn, 5,0, 1,1, Qt::AlignHCenter);
        //Connect create nym button with a re-action;
        connect(mc_nym_manager_addnym_create_nym_btn, SIGNAL(clicked()), this, SLOT(addnym_dialog_createnym_slot()));
        
        /** Flag as already init **/
        mc_nymmanager_addnym_dialog_already_init = 1;
    }
    //Resize
    mc_nym_manager_addnym_dialog->resize(400, 290);
    //Show
    mc_nym_manager_addnym_dialog->show();
}

void NymManagerWindow::nymmanager_removenym_slot(){
    //Init, then show; If already init, then just show
    if(mc_nymmanager_removenym_dialog_already_init == 0){
        mc_nym_manager_removenym_dialog = new QDialog(0);
        mc_nym_manager_removenym_dialog->setWindowTitle("Remove Pseudonym | Moneychanger");
        mc_nym_manager_removenym_dialog->setModal(1);
        //Grid layout
        mc_nym_manager_removenym_gridlayout = new QGridLayout(0);
        mc_nym_manager_removenym_dialog->setLayout(mc_nym_manager_removenym_gridlayout);
    }
    mc_nym_manager_removenym_dialog->resize(400, 290);
    mc_nym_manager_removenym_dialog->show();
}

void NymManagerWindow::nymmanager_dataChanged_slot(QModelIndex topLeft, QModelIndex bottomRight){
    //Ignore triggers while "refreshing" the nym manager.
    if(mc_nymmanager_refreshing == 0 && mc_nymmanager_proccessing_dataChanged == 0){
        /** Flag Proccessing dataChanged **/
        mc_nymmanager_proccessing_dataChanged = 1;
        
        /** Proccess the "Display Name" column **/
        if(topLeft.column() == 0){
            //Get the value (as std::string) of the nym id
            QStandardItem * nym_id_item = mc_nym_manager_tableview_itemmodel->item(topLeft.row(), 1);
            QString nym_id_string = nym_id_item->text();
            std::string nym_id = nym_id_string.toStdString();
            
            //Get the value (as std::string) of the newly set name of the nym id
            QVariant new_nym_name_variant = topLeft.data();
            QString new_nym_name_string = new_nym_name_variant.toString();
            std::string new_nym_name = new_nym_name_string.toStdString();
            qDebug() << nym_id_string;
            //Update the newly set display name for this nym in OT ( call ot for updating )
            bool setName_successfull = OTAPI_Wrap::SetNym_Name(nym_id, nym_id, new_nym_name);
            if(setName_successfull == true){
                //Do nothing (There is nothing that needs to be done)
            }else{
                //Setting of the display name for this nym failed, revert value visually, display recent error
                mc_nym_manager_most_recent_erorr->setText("<span style='color:#A80000'><b>Renaming that nym failed. (Error Code: 100)</b></span>");
            }
        }
        
        /** Proccess the "Default" column (if triggered) **/
        if(topLeft.column() == 2){
            //The (default) 2 column has checked(or unchecked) a box, (uncheck all checkboxes, except the newly selected one)
            int total_checkboxes = mc_nym_manager_tableview_itemmodel->rowCount();
            
            //Uncheck all boxes (always checkMarked the triggered box)
            for(int a = 0; a < total_checkboxes; a++){
                
                //Check if this is a checkbox we should checkmark
                if(a != topLeft.row()){
                    
                    //Get checkbox item
                    QStandardItem * checkbox_model = mc_nym_manager_tableview_itemmodel->item(a, 2);
                    
                    //Update the checkbox item at the backend.
                    checkbox_model->setCheckState(Qt::Unchecked);
                    
                    //Update the checkbox item visually.
                    mc_nym_manager_tableview_itemmodel->setItem(a, 2, checkbox_model);
                }else if(a == topLeft.row()){
                    
                    //Get checkbox item
                    QStandardItem * checkbox_model = mc_nym_manager_tableview_itemmodel->item(a, 2);
                    
                    //Update the checkbox item at the backend.
                    //Get nym id we are targeting to update.
                    QStandardItem * nym_id = mc_nym_manager_tableview_itemmodel->item(a, 1);
                    QVariant nym_id_variant = nym_id->text();
                    QString nym_id_string = nym_id_variant.toString();
                    QString nym_name_string = QString::fromStdString(OTAPI_Wrap::GetNym_Name(nym_id_string.toStdString()));
                    //Update the checkbox item visually.
                    checkbox_model->setCheckState(Qt::Checked);
                    mc_nym_manager_tableview_itemmodel->setItem(a, 2, checkbox_model);
                    
                    //Update default nym at realtime memory backend
                    
                    ((Moneychanger *)parentWidget())->set_systrayMenu_nym_setDefaultNym(nym_id_string, nym_name_string);
                    
                    
                }
                
            }
            
        }
        
        /** Unflag Proccessing Data Changed **/
        mc_nymmanager_proccessing_dataChanged = 0;
    }
}


/**** ****
 **** Nym Manager -> Add Nym Dialog (Private Slots)
 **** ****/
void NymManagerWindow::addnym_dialog_showadvanced_slot(QString link_href){
    //If advanced options are already showing, hide, if they are hidden, show them.
    if(mc_nymmanager_addnym_dialog_advanced_showing == 0){
        //Show advanced options.
        //Show the Bits option
        
    }else if(mc_nymmanager_addnym_dialog_advanced_showing == 1){
        //Hide advanced options.
        //Hide the Bits option
    }
}


void NymManagerWindow::addnym_dialog_createnym_slot(){
    std::string pseudonym = OTAPI_Wrap::CreateNym(1024, "", "");
    QString new_pseudonym = QString::fromStdString(pseudonym);
    
    //Success if non null
    if(new_pseudonym != ""){
        
    }else{
        //Failed to create pseudonym
    }
}


void NymManagerWindow::request_remove_nym_slot(){
    //Extract the currently selected nym from the nym-list.
    QModelIndexList selected_indexes = mc_nym_manager_tableview->selectionModel()->selectedIndexes();
    
    for(int a = 0; a < selected_indexes.size(); a++){
        QModelIndex selected_index = selected_indexes.at(a);
        int selected_row = selected_index.row();
        
        //Get nym id
        QModelIndex nym_id_modelindex = mc_nym_manager_tableview_itemmodel->index(selected_row, 1, QModelIndex());
        QVariant nym_id_variant = nym_id_modelindex.data();
        QString nym_id_string = nym_id_variant.toString();
        bool can_remove = OTAPI_Wrap::Wallet_CanRemoveNym(nym_id_string.toStdString());
        
        if(can_remove == true){
            //Remove it
            OTAPI_Wrap::Wallet_RemoveNym(nym_id_string.toStdString());
        }else{
            //Find out why it can't be removed and alert the user the reasoning.
            //                        //Loop through nyms
            //                        std::string nym_id_std = nym_id_string.toStdString();
            //                        int num_nyms_registered_at_nym = 0;
            //                        int num_nyms = OTAPI_Wrap::GetNymCount();
            //                        for(int b = 0; b < num_nyms; b++){
            //                            bool nym_index_at_nym = OTAPI_Wrap::IsNym_RegisteredAtnym(OTAPI_Wrap::GetNym_ID(b), nym_id_std);
            //                            if(nym_index_at_nym == true){
            //                                num_nyms_registered_at_nym += 1;
            //                            }
            //                        }
        }
        
    }
    
}


bool NymManagerWindow::eventFilter(QObject *obj, QEvent *event){
    
    if (event->type() == QEvent::Close) {
        ((Moneychanger *)parentWidget())->close_nymmanager_dialog();
        return true;
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape){
            mc_nym_manager_dialog->close();
            return true;
        }
        return true;
    }else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

