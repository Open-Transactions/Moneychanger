#include "servermanagerwindow.h"

ServerManagerWindow::ServerManagerWindow(QWidget *parent) :
    QWidget(parent)
{
    
    //Server Manager
    mc_servermanager_already_init = 0;
    mc_servermanager_refreshing = 0;
    mc_servermanager_proccessing_dataChanged = 0;
    
    //"Add server" dialog
    mc_servermanager_addserver_dialog_already_init = 0;
    mc_servermanager_addserver_dialog_advanced_showing = 0;
    
    //"Remove server" dialog
    mc_servermanager_removeserver_dialog_already_init = 0;
}

void ServerManagerWindow::dialog(){
    /** If the server list manager dialog has already been init,
     *  just show it, Other wise, init and show if this is the
     *  first time.
     **/
    if(mc_servermanager_already_init == 0){
        //Init
        mc_server_manager_dialog = new QDialog(0);
        mc_server_manager_dialog->setWindowTitle("Server Manager | Moneychanger");
        mc_server_manager_dialog->installEventFilter(this);
        mc_server_manager_gridlayout = new QGridLayout(0);
        mc_server_manager_gridlayout->setColumnStretch(1, 0);
        mc_server_manager_dialog->setLayout(mc_server_manager_gridlayout);
        
        /** First Row (Takes up two columns) **/
        //Header (Server List Manager)
        mc_server_manager_label = new QLabel("<h2>Server Manager</h2>");
        mc_server_manager_gridlayout->addWidget(mc_server_manager_label, 0,0, 1,2, Qt::AlignRight);
        
        /** Second Row **/
        //Column One
        //Tableview (Server List)
        mc_server_manager_tableview_itemmodel = new QStandardItemModel(0);
        mc_server_manager_tableview = new QTableView(0);
        mc_server_manager_tableview->setSelectionMode(QAbstractItemView::SingleSelection);
        mc_server_manager_tableview->setModel(mc_server_manager_tableview_itemmodel);
        
        mc_server_manager_tableview_itemmodel->setColumnCount(3);
        mc_server_manager_tableview_itemmodel->setHorizontalHeaderItem(0, new QStandardItem(QString("Display Name")));
        mc_server_manager_tableview_itemmodel->setHorizontalHeaderItem(1, new QStandardItem(QString("Server ID")));
        mc_server_manager_tableview_itemmodel->setHorizontalHeaderItem(2, new QStandardItem(QString("Default")));
        
        //Add to grid
        mc_server_manager_gridlayout->addWidget(mc_server_manager_tableview, 1,0, 1,1);
        
        //Column Two
        mc_server_manager_addremove_btngroup_removebtn = new QPushButton("Remove Server", 0);
        mc_server_manager_addremove_btngroup_removebtn->setStyleSheet("QPushButton{padding:0.5em;}");
        //Make a "click" reaction to the remove server button
        //connect(mc_server_manager_addremove_btngroup_removebtn, SIGNAL(clicked()), this, SLOT(mc_server_manager_request_remove_server_slot()));
        
        //Add to grid
        mc_server_manager_gridlayout->addWidget(mc_server_manager_addremove_btngroup_removebtn, 1,1, 1,1, Qt::AlignTop);
        
        /** Flag already int **/
        mc_servermanager_already_init = 1;
    }
    
    //Resize
    mc_server_manager_dialog->resize(500,300);
    //Show
    mc_server_manager_dialog->show();
    
    /**
     ** Refresh server list data
     **/
    
    //Remove all servers in the list
    mc_server_manager_tableview_itemmodel->removeRows(0, mc_server_manager_tableview_itemmodel->rowCount(), QModelIndex());
    
    //Add/Append/Refresh server list.
    int row_index = 0;
    int32_t serverlist_count = OTAPI_Wrap::GetServerCount();
    for(int a = 0; a < serverlist_count; a++){
        std::string server_id =  OTAPI_Wrap::GetServer_ID(a);
        std::string server_name = OTAPI_Wrap::GetServer_Name(server_id);
        
        //Extract data
        QString server_name_string = QString::fromStdString(server_name);
        QString server_id_string = QString::fromStdString(server_id);
        
        //Place extracted data into the table view
        QStandardItem * col_one = new QStandardItem(server_name_string);
        QStandardItem * col_two = new QStandardItem(server_id_string);
        QStandardItem * col_three = new QStandardItem(0);
        //Set as checkbox
        col_three->setCheckable(1);
        
        //Check if this is the default server; If it is, then mark it as "Checked"
        if(((Moneychanger *)parentWidget())->get_default_server_id() == server_id_string){
            col_three->setCheckState(Qt::Checked);
        }
        mc_server_manager_tableview_itemmodel->setItem(row_index, 0, col_one);
        mc_server_manager_tableview_itemmodel->setItem(row_index, 1, col_two);
        mc_server_manager_tableview_itemmodel->setItem(row_index, 2, col_three);
    }
    
    
}



// ---------------------------------------------------------
// SERVER
void ServerManagerWindow::addserver_slot(){
    //Decide if we should init and show, or just show
    if(mc_servermanager_addserver_dialog_already_init == 0){
        //Init, then show.
        mc_server_manager_addserver_dialog = new QDialog(0);
        mc_server_manager_addserver_dialog->setWindowTitle("Add Server Contract | Moneychanger");
        mc_server_manager_addserver_dialog->setModal(1);
        
        //Gridlayout
        mc_server_manager_addserver_gridlayout = new QGridLayout(0);
        mc_server_manager_addserver_dialog->setLayout(mc_server_manager_addserver_gridlayout);
        
        //Label (header)
        mc_server_manager_addserver_header = new QLabel("<h2>Add Server Contract</h2>",0);
        mc_server_manager_addserver_gridlayout->addWidget(mc_server_manager_addserver_header, 0,0, 1,1, Qt::AlignRight);
        
        //Label (Show Advanced Option(s)) (Also a button/connection)
        mc_server_manager_addserver_subheader_toggleadvanced_options_label = new QLabel("<a href='#'>Advanced Option(s)</a>", 0);
        mc_server_manager_addserver_gridlayout->addWidget(mc_server_manager_addserver_subheader_toggleadvanced_options_label, 1,0, 1,1, Qt::AlignRight);
        //Connect with a re-action
        connect(mc_server_manager_addserver_subheader_toggleadvanced_options_label, SIGNAL(linkActivated(QString)), this, SLOT(dialog_showadvanced_slot(QString)));
        
        
        //Label (instructions)
        mc_server_manager_addserver_subheader_instructions = new QLabel("Below are some options that will help determine how your server will be added.");
        mc_server_manager_addserver_subheader_instructions->setWordWrap(1);
        mc_server_manager_addserver_subheader_instructions->setStyleSheet("QLabel{padding:0.5em;}");
        mc_server_manager_addserver_gridlayout->addWidget(mc_server_manager_addserver_subheader_instructions, 2,0, 1,1);
        
        //Label (Choose Source Question)
        //                    mc_server_manager_addserver_choosesource_label = new QLabel("<h3>Enter the Server Contract</h3>");
        //                    mc_server_manager_addserver_choosesource_label->setStyleSheet("QLabel{padding:1em;}");
        //                    mc_server_manager_addserver_gridlayout->addWidget(mc_server_manager_addserver_choosesource_label, 3,0, 1,1);
        
        //                    //Combobox (Dropdown box: Choose Source)
        //                    mc_server_manager_addserver_choosesource_answer_selection = new QComboBox(0);
        //                    mc_server_manager_addserver_choosesource_answer_selection->addItem("Namecoin");
        //                    mc_server_manager_addserver_choosesource_answer_selection->addItem("No-Source");
        //                    mc_server_manager_addserver_choosesource_answer_selection->setCurrentIndex(1);
        //                    mc_server_manager_addserver_choosesource_answer_selection->setStyleSheet("QComboBox{padding:0.5em;}");
        //                    mc_server_manager_addserver_gridlayout->addWidget(mc_server_manager_addserver_choosesource_answer_selection, 4,0, 1,1);
        
        //Create server (button)
        mc_server_manager_addserver_create_server_btn = new QPushButton("Add a Server Contract", 0);
        mc_server_manager_addserver_create_server_btn->setStyleSheet("QPushButton{padding:1em;}");
        mc_server_manager_addserver_gridlayout->addWidget(mc_server_manager_addserver_create_server_btn, 5,0, 1,1, Qt::AlignHCenter);
        //Connect create server button with a re-action;
        connect(mc_server_manager_addserver_create_server_btn, SIGNAL(clicked()), this, SLOT(dialog_createserver_slot()));
        
        /** Flag as already init **/
        mc_servermanager_addserver_dialog_already_init = 1;
    }
    //Resize
    mc_server_manager_addserver_dialog->resize(400, 290);
    //Show
    mc_server_manager_addserver_dialog->show();
}

void ServerManagerWindow::removeserver_slot(){
    //Init, then show; If already init, then just show
    if(mc_servermanager_removeserver_dialog_already_init == 0){
        mc_server_manager_removeserver_dialog = new QDialog(0);
        mc_server_manager_removeserver_dialog->setWindowTitle("Remove Server Contract | Moneychanger");
        mc_server_manager_removeserver_dialog->setModal(1);
        //Grid layout
        mc_server_manager_removeserver_gridlayout = new QGridLayout(0);
        mc_server_manager_removeserver_dialog->setLayout(mc_server_manager_removeserver_gridlayout);
    }
    mc_server_manager_removeserver_dialog->show();
}

void ServerManagerWindow::dataChanged_slot(QModelIndex topLeft, QModelIndex bottomRight){
    //Ignore triggers while "refreshing" the server manager.
    if(mc_servermanager_refreshing == 0 && mc_servermanager_proccessing_dataChanged == 0){
        /** Flag Proccessing dataChanged **/
        mc_servermanager_proccessing_dataChanged = 1;
        
        /** Proccess the "Display Name" column **/
        if(topLeft.column() == 0){
            //Get the value (as std::string) of the server id
            QStandardItem * server_id_item = mc_server_manager_tableview_itemmodel->item(topLeft.row(), 1);
            QString server_id_string = server_id_item->text();
            std::string server_id = server_id_string.toStdString();
            
            //Get the value (as std::string) of the newly set name of the server id
            QVariant new_server_name_variant = topLeft.data();
            QString new_server_name_string = new_server_name_variant.toString();
            std::string new_server_name = new_server_name_string.toStdString();
            qDebug() << server_id_string;
            //Update the newly set display name for this server in OT ( call ot for updating )
            bool setName_successfull = OTAPI_Wrap::SetServer_Name(server_id, new_server_name);
            if(setName_successfull == true){
                //Do nothing (There is nothing that needs to be done)
            }else{
                //Setting of the display name for this server failed, revert value visually, display recent error
                mc_server_manager_most_recent_erorr->setText("<span style='color:#A80000'><b>Renaming that server failed. (Error Code: 100)</b></span>");
            }
        }
        
        /** Proccess the "Default" column (if triggered) **/
        if(topLeft.column() == 2){
            //The (default) 2 column has checked(or unchecked) a box, (uncheck all checkboxes, except the newly selected one)
            int total_checkboxes = mc_server_manager_tableview_itemmodel->rowCount();
            
            //Uncheck all boxes (always checkMarked the triggered box)
            for(int a = 0; a < total_checkboxes; a++){
                
                //Check if this is a checkbox we should checkmark
                if(a != topLeft.row()){
                    
                    //Get checkbox item
                    QStandardItem * checkbox_model = mc_server_manager_tableview_itemmodel->item(a, 2);
                    
                    //Update the checkbox item at the backend.
                    checkbox_model->setCheckState(Qt::Unchecked);
                    
                    //Update the checkbox item visually.
                    mc_server_manager_tableview_itemmodel->setItem(a, 2, checkbox_model);
                }else if(a == topLeft.row()){
                    
                    //Get checkbox item
                    QStandardItem * checkbox_model = mc_server_manager_tableview_itemmodel->item(a, 2);
                    
                    //Update the checkbox item at the backend.
                    //Get server id we are targeting to update.
                    QStandardItem * server_id = mc_server_manager_tableview_itemmodel->item(a, 1);
                    QVariant server_id_variant = server_id->text();
                    QString server_id_string = server_id_variant.toString();
                    QString server_name_string = QString::fromStdString(OTAPI_Wrap::GetServer_Name(server_id_string.toStdString()));
                    //Update the checkbox item visually.
                    checkbox_model->setCheckState(Qt::Checked);
                    mc_server_manager_tableview_itemmodel->setItem(a, 2, checkbox_model);
                    
                    //Update default server at realtime memory backend
                    
                    ((Moneychanger *)parentWidget())->set_systrayMenu_server_setDefaultServer(server_id_string, server_name_string);
                    
                    
                }
                
            }
            
        }
        
        /** Unflag Proccessing Data Changed **/
        mc_servermanager_proccessing_dataChanged = 0;
    }
}

/**** ****
 **** server Manager -> Add server Dialog (Private Slots)
 **** ****/
void ServerManagerWindow::dialog_showadvanced_slot(QString link_href){
    //If advanced options are already showing, hide, if they are hidden, show them.
    if(mc_servermanager_addserver_dialog_advanced_showing == 0){
        //Show advanced options.
        //Show the Bits option
        
    }else if(mc_servermanager_addserver_dialog_advanced_showing == 1){
        //Hide advanced options.
        //Hide the Bits option
    }
}

void ServerManagerWindow::dialog_createserver_slot(){
    //            std::string pseudonym = OTAPI_Wrap::CreateNym(1024, "", "");
    //            QString new_pseudonym = QString::fromStdString(pseudonym);
    QString new_server;
    
    //Success if non null
    if(new_server != ""){
        
    }else{
        //Failed to create server type
    }
}



/**
 * @brief Moneychanger::mc_servermanager_request_remove_server_slot
 * @info This will attempt to remove the server from the loaded wallet,
 *       At the moment only "one" server can be selected but the for loop is there for
 *       future upgrades of such functionality.
 **/
void ServerManagerWindow::request_remove_server_slot(){
    //Extract the currently selected server from the server-list.
    QModelIndexList selected_indexes = mc_server_manager_tableview->selectionModel()->selectedIndexes();
    
    for(int a = 0; a < selected_indexes.size(); a++){
        QModelIndex selected_index = selected_indexes.at(a);
        int selected_row = selected_index.row();
        
        //Get server id
        QModelIndex server_id_modelindex = mc_server_manager_tableview_itemmodel->index(selected_row, 1, QModelIndex());
        QVariant server_id_variant = server_id_modelindex.data();
        QString server_id_string = server_id_variant.toString();
        bool can_remove = OTAPI_Wrap::Wallet_CanRemoveServer(server_id_string.toStdString());
        
        if(can_remove == true){
            //Remove it
            OTAPI_Wrap::Wallet_RemoveServer(server_id_string.toStdString());
        }else{
            //Find out why it can't be removed and alert the user the reasoning.
            //Loop through nyms
            std::string server_id_std = server_id_string.toStdString();
            int num_nyms_registered_at_server = 0;
            int num_nyms = OTAPI_Wrap::GetNymCount();
            for(int b = 0; b < num_nyms; b++){
                bool nym_index_at_server = OTAPI_Wrap::IsNym_RegisteredAtServer(OTAPI_Wrap::GetNym_ID(b), server_id_std);
                if(nym_index_at_server == true){
                    num_nyms_registered_at_server += 1;
                }
            }
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

bool ServerManagerWindow::eventFilter(QObject *obj, QEvent *event){
    
    if (event->type() == QEvent::Close) {
        ((Moneychanger *)parentWidget())->close_servermanager_dialog();
        return true;
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape){
            mc_server_manager_dialog->close();
            return true;
        }
        return true;
    }else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}
