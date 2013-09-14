#include "assetmanagerwindow.h"

AssetManagerWindow::AssetManagerWindow(QWidget *parent) :
    QWidget(parent)
{
    
    //Asset Manager
    mc_assetmanager_already_init = 0;
    mc_assetmanager_refreshing = 0;
    mc_assetmanager_proccessing_dataChanged = 0;
    
    //"Add asset" dialog
    mc_assetmanager_addasset_dialog_already_init = 0;
    mc_assetmanager_addasset_dialog_advanced_showing = 0;
    
    //"Remove asset" dialog
    mc_assetmanager_removeasset_dialog_already_init = 0;
    
}


void AssetManagerWindow::dialog(){
    
    /** If the asset manager dialog has already been init,
     *  just show it, Other wise, init and show if this is the
     *  first time.
     **/
    if(mc_assetmanager_already_init == 0){
        
        //The asset Manager has not been init yet; Init, then show it.
        mc_asset_manager_dialog = new QDialog(0);
        
        /** window properties **/
        //Set window title
        mc_asset_manager_dialog->setWindowTitle("Asset Contracts | Moneychanger");
        
        //Set window on top
        //mc_asset_manager_dialog->setWindowFlags(Qt::WindowStaysOnTopHint);
        
        /** layout and content **/
        //Grid layout
        mc_asset_manager_gridlayout = new QGridLayout(0);
        mc_asset_manager_dialog->setLayout(mc_asset_manager_gridlayout);
        
        /* First Row in asset manager Grid */
        //Label (header)
        mc_asset_manager_label = new QLabel("<h3>Asset Contracts</h3>", 0);
        mc_asset_manager_label->setAlignment(Qt::AlignRight);
        mc_asset_manager_gridlayout->addWidget(mc_asset_manager_label, 0,0, 1,2, Qt::AlignRight);
        
        
        /* Second Row in asset manager Grid */
        /** First column in address book grid (left side) **/
        //Horizontal box (contains: List of assets, add/remove buttons)
        mc_asset_manager_holder = new QWidget(0);
        mc_asset_manager_hbox = new QHBoxLayout(0);
        mc_asset_manager_holder->setLayout(mc_asset_manager_hbox);
        mc_asset_manager_gridlayout->addWidget(mc_asset_manager_holder);
        
        //Table View (backend and visual init)
        mc_asset_manager_tableview_itemmodel = new QStandardItemModel(0,3,0);
        mc_asset_manager_tableview_itemmodel->setHorizontalHeaderItem(0, new QStandardItem(QString("Asset Display Name")));
        mc_asset_manager_tableview_itemmodel->setHorizontalHeaderItem(1, new QStandardItem(QString("Asset ID")));
        mc_asset_manager_tableview_itemmodel->setHorizontalHeaderItem(2, new QStandardItem(QString("Default")));
        
        //Connect tableviews' backend "dataChanged" signal to a re-action.
        connect(mc_asset_manager_tableview_itemmodel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(dataChanged_slot(QModelIndex,QModelIndex)));
        
        mc_asset_manager_tableview = new QTableView(0);
        mc_asset_manager_tableview->setSelectionMode(QAbstractItemView::SingleSelection);
        mc_asset_manager_tableview->setModel(mc_asset_manager_tableview_itemmodel);
        mc_asset_manager_tableview->setColumnWidth(0, 175);
        mc_asset_manager_tableview->setColumnWidth(1, 150);
        mc_asset_manager_tableview->setColumnWidth(2, 75);
        mc_asset_manager_gridlayout->addWidget(mc_asset_manager_tableview, 1,0, 1,1);
        
        
        /** Second column in asset manager grid (right side) **/
        //Vertical box (contains: add/remove buttons)
        mc_asset_manager_addremove_btngroup_holder = new QWidget(0);
        mc_asset_manager_addremove_btngroup_vbox = new QVBoxLayout(0);
        mc_asset_manager_addremove_btngroup_holder->setLayout(mc_asset_manager_addremove_btngroup_vbox);
        mc_asset_manager_addremove_btngroup_vbox->setAlignment(Qt::AlignTop);
        mc_asset_manager_gridlayout->addWidget(mc_asset_manager_addremove_btngroup_holder, 1,1, 1,1);
        
        //"Add asset" button
        mc_asset_manager_addremove_btngroup_addbtn = new QPushButton("Add Asset Contract", 0);
        mc_asset_manager_addremove_btngroup_addbtn->setStyleSheet("QPushButton{padding:0.5em;}");
        mc_asset_manager_addremove_btngroup_vbox->addWidget(mc_asset_manager_addremove_btngroup_addbtn, Qt::AlignTop);
        //Connect the add asset button with a re-action to it being "clicked"
        connect(mc_asset_manager_addremove_btngroup_addbtn, SIGNAL(clicked()), this, SLOT(addasset_slot()));
        
        //"Remove asset" button
        mc_asset_manager_addremove_btngroup_removebtn = new QPushButton("Remove Asset Contract", 0);
        mc_asset_manager_addremove_btngroup_removebtn->setStyleSheet("QPushButton{padding:0.5em;}");
        mc_asset_manager_addremove_btngroup_vbox->addWidget(mc_asset_manager_addremove_btngroup_removebtn, Qt::AlignTop);
        //Connect the remove asset button with a re-action to it being "clicked"
        connect(mc_asset_manager_addremove_btngroup_removebtn, SIGNAL(clicked()), this, SLOT(removeasset_slot()));
        
        /** Third column (Most revent error) **/
        //Label
        mc_asset_manager_most_recent_erorr = new QLabel("");
        mc_asset_manager_gridlayout->addWidget(mc_asset_manager_most_recent_erorr, 2,0, 1,2, Qt::AlignHCenter);
        /** Flag as init **/
        mc_assetmanager_already_init = 1;
    }
    /** ***
     ** Resize & Show
     **/
    mc_asset_manager_dialog->resize(600, 300);
    mc_asset_manager_dialog->show();
    
    /** ***
     ** Data Refresh/Fill Logic
     **/
    //Refresh asset manger list
    /** Flag Refreshing asset Manger **/
    mc_assetmanager_refreshing = 1;
    
    //remove all rows from the asset manager (so we can refresh any newly changed data)
    mc_asset_manager_tableview_itemmodel->removeRows(0, mc_asset_manager_tableview_itemmodel->rowCount());
    
    //Refresh the asset manager
    //Refresh asset list (can't be done, there is a glitch where if you open the asset manger dialog twice it does wierd things to the systray for asset menus )
    //mc_systrayMenu_reload_assetlist();
    
    //Add asset id and names to the manager list
    int total_assets = ((Moneychanger *)parentWidget())->get_asset_list_id_size();
    qDebug() << "total: " << total_assets;
    int row_index = 0;
    for(int a = 0; a < total_assets; a++){
        //Add asset account name and id to the list.
        
        
        //Extract stuff for this row
        QString asset_id = ((Moneychanger *)parentWidget())->get_asset_id_at(a);
        QString asset_name = ((Moneychanger *)parentWidget())->get_asset_name_at(a);
        qDebug() << "ADDING asset ID: " << asset_id;
        //Place extracted data into the table view
        QStandardItem * col_one = new QStandardItem(asset_name);
        QStandardItem * col_two = new QStandardItem(asset_id);
        //Column two is uneditable
        col_two->setEditable(0);
        
        QStandardItem * col_three = new QStandardItem();
        //Column three is a checkmark, we need to set some options in this case.
        col_three->setCheckable(1);
        
        //If this is the default asset type; if yes, mark as checked
        if(((Moneychanger *)parentWidget())->get_default_asset_id() == asset_id){
            col_three->setCheckState(Qt::Checked);
        }
        
        mc_asset_manager_tableview_itemmodel->setItem(row_index, 0, col_one);
        mc_asset_manager_tableview_itemmodel->setItem(row_index, 1, col_two);
        mc_asset_manager_tableview_itemmodel->setItem(row_index, 2, col_three);
        
        //Increment index
        row_index += 1;
    }
    /** Unflag as current refreshing **/
    mc_assetmanager_refreshing = 0;
}


void AssetManagerWindow::addasset_slot(){
    //Decide if we should init and show, or just show
    if(mc_assetmanager_addasset_dialog_already_init == 0){
        //Init, then show.
        mc_asset_manager_addasset_dialog = new QDialog(0);
        mc_asset_manager_addasset_dialog->setWindowTitle("Add Asset Contract | Moneychanger");
        mc_asset_manager_addasset_dialog->setModal(1);
        
        //Gridlayout
        mc_asset_manager_addasset_gridlayout = new QGridLayout(0);
        mc_asset_manager_addasset_dialog->setLayout(mc_asset_manager_addasset_gridlayout);
        
        //Label (header)
        mc_asset_manager_addasset_header = new QLabel("<h2>Add Asset Contract</h2>",0);
        mc_asset_manager_addasset_gridlayout->addWidget(mc_asset_manager_addasset_header, 0,0, 1,1, Qt::AlignRight);
        
        //Label (Show Advanced Option(s)) (Also a button/connection)
        mc_asset_manager_addasset_subheader_toggleadvanced_options_label = new QLabel("<a href='#'>Advanced Option(s)</a>", 0);
        mc_asset_manager_addasset_gridlayout->addWidget(mc_asset_manager_addasset_subheader_toggleadvanced_options_label, 1,0, 1,1, Qt::AlignRight);
        //Connect with a re-action
        connect(mc_asset_manager_addasset_subheader_toggleadvanced_options_label, SIGNAL(linkActivated(QString)), this, SLOT(dialog_showadvanced_slot(QString)));
        
        
        //Label (instructions)
        mc_asset_manager_addasset_subheader_instructions = new QLabel("Below are some options that will help determine how your Asset will be added.");
        mc_asset_manager_addasset_subheader_instructions->setWordWrap(1);
        mc_asset_manager_addasset_subheader_instructions->setStyleSheet("QLabel{padding:0.5em;}");
        mc_asset_manager_addasset_gridlayout->addWidget(mc_asset_manager_addasset_subheader_instructions, 2,0, 1,1);
        
        //Label (Choose Source Question)
        mc_asset_manager_addasset_choosesource_label = new QLabel("<h3>Enter the Asset Contract</h3>");
        mc_asset_manager_addasset_choosesource_label->setStyleSheet("QLabel{padding:1em;}");
        mc_asset_manager_addasset_gridlayout->addWidget(mc_asset_manager_addasset_choosesource_label, 3,0, 1,1);
        
        //Combobox (Dropdown box: Choose Source)
        mc_asset_manager_addasset_choosesource_answer_selection = new QComboBox(0);
        mc_asset_manager_addasset_choosesource_answer_selection->addItem("Namecoin");
        mc_asset_manager_addasset_choosesource_answer_selection->addItem("No-Source");
        mc_asset_manager_addasset_choosesource_answer_selection->setCurrentIndex(1);
        mc_asset_manager_addasset_choosesource_answer_selection->setStyleSheet("QComboBox{padding:0.5em;}");
        mc_asset_manager_addasset_gridlayout->addWidget(mc_asset_manager_addasset_choosesource_answer_selection, 4,0, 1,1);
        
        //Create asset (button)
        mc_asset_manager_addasset_create_asset_btn = new QPushButton("Add an Asset Contract", 0);
        mc_asset_manager_addasset_create_asset_btn->setStyleSheet("QPushButton{padding:1em;}");
        mc_asset_manager_addasset_gridlayout->addWidget(mc_asset_manager_addasset_create_asset_btn, 5,0, 1,1, Qt::AlignHCenter);
        //Connect create asset button with a re-action;
        connect(mc_asset_manager_addasset_create_asset_btn, SIGNAL(clicked()), this, SLOT(dialog_createasset_slot()));
        
        /** Flag as already init **/
        mc_assetmanager_addasset_dialog_already_init = 1;
    }
    //Resize
    mc_asset_manager_addasset_dialog->resize(400, 300);
    //Show
    mc_asset_manager_addasset_dialog->show();
}



void AssetManagerWindow::removeasset_slot(){
    //Init, then show; If already init, then just show
    if(mc_assetmanager_removeasset_dialog_already_init == 0){
        mc_asset_manager_removeasset_dialog = new QDialog(0);
        mc_asset_manager_removeasset_dialog->setWindowTitle("Remove Asset Contract | Moneychanger");
        mc_asset_manager_removeasset_dialog->setModal(1);
        //Grid layout
        mc_asset_manager_removeasset_gridlayout = new QGridLayout(0);
        mc_asset_manager_removeasset_dialog->setLayout(mc_asset_manager_removeasset_gridlayout);
    }
    mc_asset_manager_removeasset_dialog->resize(400, 290);
    mc_asset_manager_removeasset_dialog->show();
}




void AssetManagerWindow::dataChanged_slot(QModelIndex topLeft, QModelIndex bottomRight){
    //Ignore triggers while "refreshing" the asset manager.
    if(mc_assetmanager_refreshing == 0 && mc_assetmanager_proccessing_dataChanged == 0){
        /** Flag Proccessing dataChanged **/
        mc_assetmanager_proccessing_dataChanged = 1;
        
        /** Proccess the "Display Name" column **/
        if(topLeft.column() == 0){
            //Get the value (as std::string) of the asset id
            QStandardItem * asset_id_item = mc_asset_manager_tableview_itemmodel->item(topLeft.row(), 1);
            QString asset_id_string = asset_id_item->text();
            std::string asset_id = asset_id_string.toStdString();
            
            //Get the value (as std::string) of the newly set name of the asset id
            QVariant new_asset_name_variant = topLeft.data();
            QString new_asset_name_string = new_asset_name_variant.toString();
            std::string new_asset_name = new_asset_name_string.toStdString();
            qDebug() << asset_id_string;
            //Update the newly set display name for this asset in OT ( call ot for updating )
            bool setName_successfull = OTAPI_Wrap::SetAssetType_Name(asset_id, new_asset_name);
            if(setName_successfull == true){
                //Do nothing (There is nothing that needs to be done)
            }else{
                //Setting of the display name for this asset failed, revert value visually, display recent error
                mc_asset_manager_most_recent_erorr->setText("<span style='color:#A80000'><b>Renaming that asset failed. (Error Code: 100)</b></span>");
            }
        }
        
        /** Proccess the "Default" column (if triggered) **/
        if(topLeft.column() == 2){
            //The (default) 2 column has checked(or unchecked) a box, (uncheck all checkboxes, except the newly selected one)
            int total_checkboxes = mc_asset_manager_tableview_itemmodel->rowCount();
            
            //Uncheck all boxes (always checkMarked the triggered box)
            for(int a = 0; a < total_checkboxes; a++){
                
                //Check if this is a checkbox we should checkmark
                if(a != topLeft.row()){
                    
                    //Get checkbox item
                    QStandardItem * checkbox_model = mc_asset_manager_tableview_itemmodel->item(a, 2);
                    
                    //Update the checkbox item at the backend.
                    checkbox_model->setCheckState(Qt::Unchecked);
                    
                    //Update the checkbox item visually.
                    mc_asset_manager_tableview_itemmodel->setItem(a, 2, checkbox_model);
                }else if(a == topLeft.row()){
                    
                    //Get checkbox item
                    QStandardItem * checkbox_model = mc_asset_manager_tableview_itemmodel->item(a, 2);
                    
                    //Update the checkbox item at the backend.
                    //Get asset id we are targeting to update.
                    QStandardItem * asset_id = mc_asset_manager_tableview_itemmodel->item(a, 1);
                    QVariant asset_id_variant = asset_id->text();
                    QString asset_id_string = asset_id_variant.toString();
                    QString asset_name_string = QString::fromStdString(OTAPI_Wrap::GetAssetType_Name(asset_id_string.toStdString()));
                    //Update the checkbox item visually.
                    checkbox_model->setCheckState(Qt::Checked);
                    mc_asset_manager_tableview_itemmodel->setItem(a, 2, checkbox_model);
                    
                    //Update default asset at realtime memory backend
                    
                    ((Moneychanger *)parentWidget())->set_systrayMenu_asset_setDefaultAsset(asset_id_string, asset_name_string);
                    
                    
                }
                
            }
            
        }
        
        /** Unflag Proccessing Data Changed **/
        mc_assetmanager_proccessing_dataChanged = 0;
    }
}


/**** ****
 **** asset Manager -> Add asset Dialog (Private Slots)
 **** ****/
void AssetManagerWindow::dialog_showadvanced_slot(QString link_href){
    //If advanced options are already showing, hide, if they are hidden, show them.
    if(mc_assetmanager_addasset_dialog_advanced_showing == 0){
        //Show advanced options.
        //Show the Bits option
        
    }else if(mc_assetmanager_addasset_dialog_advanced_showing == 1){
        //Hide advanced options.
        //Hide the Bits option
    }
}



void AssetManagerWindow::request_remove_asset_slot(){
    //Extract the currently selected asset from the asset-list.
    QModelIndexList selected_indexes = mc_asset_manager_tableview->selectionModel()->selectedIndexes();
    
    for(int a = 0; a < selected_indexes.size(); a++){
        QModelIndex selected_index = selected_indexes.at(a);
        int selected_row = selected_index.row();
        
        //Get asset id
        QModelIndex asset_id_modelindex = mc_asset_manager_tableview_itemmodel->index(selected_row, 1, QModelIndex());
        QVariant asset_id_variant = asset_id_modelindex.data();
        QString asset_id_string = asset_id_variant.toString();
        bool can_remove = OTAPI_Wrap::Wallet_CanRemoveAssetType(asset_id_string.toStdString());
        
        if(can_remove == true){
            //Remove it
            OTAPI_Wrap::Wallet_RemoveAssetType(asset_id_string.toStdString());
        }else{
            //Find out why it can't be removed and alert the user the reasoning.
            //Loop through nyms
            //                        std::string asset_id_std = asset_id_string.toStdString();
            //                        int num_nyms_registered_at_asset = 0;
            //                        int num_nyms = OTAPI_Wrap::GetNymCount();
            //                        for(int b = 0; b < num_nyms; b++){
            //                            bool nym_index_at_asset = OTAPI_Wrap::IsNym_RegisteredAtAsset(OTAPI_Wrap::GetNym_ID(b), asset_id_std);
            //                            if(nym_index_at_asset == true){
            //                                num_nyms_registered_at_asset += 1;
            //                            }
            //                        }
        }
        
    }
    
}


void AssetManagerWindow::addasset_dialog_createasset_slot(){
    //            std::string pseudonym = OTAPI_Wrap::CreateNym(1024, "", "");
    //            QString new_pseudonym = QString::fromStdString(pseudonym);
    QString new_asset;
    
    //Success if non null
    if(new_asset != ""){
        
    }else{
        //Failed to create asset type
    }
}


