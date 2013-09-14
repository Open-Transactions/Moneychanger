#include "addressbookwindow.h"



AddressBookWindow::AddressBookWindow(QWidget *parent) :
    QWidget(parent)
{
    already_init = false;
}


/* **
 * Address Book Related Calls
 */
/** Show Address Book **/
//Dummy Call
void AddressBookWindow::show(){
    //The caller dosen't wish to have the address book paste to anything (they just want to see/manage the address book), just call blank.
    show("");
}


//This will show the addressbook (and paste the selection accordingly if set)
void AddressBookWindow::show(QString paste_selection_to){
    /** Order of Operations
     *Initialize
     *Paste Selection Logic
     *Data Refresh/Fill Logic
     **/
    
    
    /** ***
     ** Initialize Address Book and/or just show
     **/
    //Check if address book has been init before.
    if(already_init == 0){
        //Init address book, then show
        dialog = new QDialog(0);
        
        //(Nice effect; Dims all windows except the address book and makes the address book on top upon showing
        dialog->setModal(1);
        //mc_addressbook_dialog->setWindowFlags(Qt::WindowStaysOnTopHint);
        dialog->setWindowTitle("Address Book | Moneychanger");
        
        //Set layout
        gridlayout = new QGridLayout(0);
        dialog->setLayout(gridlayout);
        
        /* First Row in Address Book Grid */
        //Label (Address Book)
        label = new QLabel("<h3>Address Book</h3>");
        label->setAlignment(Qt::AlignRight);
        gridlayout->addWidget(label, 0,0, 1,2);
        
        /* Second Row in Address Book Grid */
        /** First column in address book grid (left side) **/
        //Table View (backend and visual init)
        tableview_itemmodel = new QStandardItemModel(0,3,0);
        tableview_itemmodel->setHorizontalHeaderItem(0, new QStandardItem(QString("Display Nym")));
        tableview_itemmodel->setHorizontalHeaderItem(1, new QStandardItem(QString("Nym ID")));
        tableview_itemmodel->setHorizontalHeaderItem(2, new QStandardItem(QString("Backend DB ID")));
        //Connect tableviews' backend "dataChanged" signal to a re-action in the parent widget.
        // In this case, we can assume our parent widget is of type Moneychanger, and we'll pass the
        // message along accordingly. In the future I'll update this to be more generic.
        connect(tableview_itemmodel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(dataChanged_slot(QModelIndex,QModelIndex)));
        
        tableview = new QTableView(0);
        tableview->setSelectionMode(QAbstractItemView::SingleSelection);
        tableview->setModel(tableview_itemmodel);
        tableview->hideColumn(2);
        gridlayout->addWidget(tableview, 1,0, 1,1);
        
        
        /** Second column in address book grid (right side) **/
        //2 Buttons (Add/Remove)
        addremove_btngroup_widget = new QWidget(0);
        addremove_btngroup_holder = new QVBoxLayout(0);
        
        addremove_btngroup_widget->setLayout(addremove_btngroup_holder);
        gridlayout->addWidget(addremove_btngroup_widget, 1,1, 1,1, Qt::AlignTop);
        
        //Add button
        addremove_add_btn = new QPushButton("Add Contact", 0);
        addremove_add_btn->setStyleSheet("QPushButton{padding:0.5em;margin:0}");
        addremove_btngroup_holder->addWidget(addremove_add_btn, 0, Qt::AlignTop);
        //Connect the add contact button with a re-action
        connect(addremove_add_btn, SIGNAL(clicked()), this, SLOT(addblankrow_slot()));
        
        //Remove button
        addremove_remove_btn = new QPushButton("Remove Contact", 0);
        addremove_remove_btn->setStyleSheet("QPushButton{padding:0.5em;margin:0}");
        addremove_btngroup_holder->addWidget(addremove_remove_btn, 0, Qt::AlignTop);
        //Connect the remove contact button with a re-action
        connect(addremove_remove_btn, SIGNAL(clicked()), this, SLOT(confirm_remove_contact_slot()));
        
        /* Third row in Address Book Grid */
        /** Spans 2 columns **/
        //This "select" button will be shown if the address book was initiated with the intention of pasting the selection into a dialog/window
        select_nym_for_paste_btn = new QPushButton("Paste selected contact as Nym Id",0);
        select_nym_for_paste_btn->setStyleSheet("QPushButton{padding:0.5em;}");
        select_nym_for_paste_btn->hide();
        gridlayout->addWidget(select_nym_for_paste_btn, 2,0, 1,2, Qt::AlignHCenter);
        //Connect the "select" button with a re-action
        connect(select_nym_for_paste_btn, SIGNAL(clicked()), this, SLOT(paste_selected_slot()));
        //Show dialog
        dialog->show();
        dialog->activateWindow();
        
        /** Flag already init **/
        already_init = 1;
        
    }else{
        //Address book already init, show it!
        dialog->show();
        dialog->activateWindow();
    }
    
    /** ***
     ** Paste Selection Logic
     **/
    //If paste_selection_to is nothing then "hide" the select button
    if(paste_selection_to == ""){
        select_nym_for_paste_btn->hide();
    }else{
        //Set mc_addressbook_paste_into = paste_selection_to
        paste_into = paste_selection_to;
        
        //Show select button
        select_nym_for_paste_btn->show();
    }
    
    
    
    /** ***
     ** Data Refresh/Fill Logic
     **/
    //Refresh Addressbook with listing
    /** Flag Refreshing Address Book **/
    refreshing = 1;
    
    //remove all rows from the address book (so we can refresh any newly changed data)
    tableview_itemmodel->removeRows(0, tableview_itemmodel->rowCount());
    
    //Add Rows of data to the backend of the table view (QStandardItemModel)
    int row_index = 0;
    //while(mc_addressbook_query.next()){
    for(int x=0; x < DBHandler::getInstance()->querySize("SELECT `id`, `nym_display_name`, `nym_id` FROM `address_book`"); x++)
    {
        //Extract data
        QString row_id = DBHandler::getInstance()->queryString("SELECT `id`, `nym_display_name`, `nym_id` FROM `address_book`", 0, x);
        QString row_nym_display_name = DBHandler::getInstance()->queryString("SELECT `id`, `nym_display_name`, `nym_id` FROM `address_book`", 1, x);
        QString row_nym_id = DBHandler::getInstance()->queryString("SELECT `id`, `nym_display_name`, `nym_id` FROM `address_book`", 2, x);
        
        //Place extracted data into the table view
        QStandardItem * col_one = new QStandardItem(row_nym_display_name);
        QStandardItem * col_two = new QStandardItem(row_nym_id);
        QStandardItem * col_three = new QStandardItem(row_id);
        
        tableview_itemmodel->setItem(row_index, 0, col_one);
        tableview_itemmodel->setItem(row_index, 1, col_two);
        tableview_itemmodel->setItem(row_index, 2, col_three);
        
        //Increment index
        row_index += 1;
        
        //Clear address book variables
        row_id = "";
        row_nym_display_name = "";
        row_nym_id = "";
    }
    
    /** Un-Flag Refreshing Address Book **/
    refreshing = 0;
    
    //Resize
    dialog->resize(400, 300);
}



//"paste selected" button then we will detect here where to paste and what to paste.
void AddressBookWindow::paste_selected_slot(){
    //First validate if anything is selected, before continuing (this is a must).
    QModelIndexList indexList = tableview->selectionModel()->selectedIndexes();
    int total_selected = indexList.size();
    qDebug() << "TOTAL SELECTED" << total_selected;
    if(total_selected >= 1){
        //Paste the selection into the appropriate contact.
        if(paste_into == "withdraw_as_voucher"){
            //Extract only the first row of the column of Nym ID
            QModelIndex data_row_model = indexList.at(0);
            qDebug() << "ROW " << data_row_model.row();
            
            QModelIndex selected_nym_id = tableview_itemmodel->index(data_row_model.row(), 1, QModelIndex());
            QVariant selected_nym_id_variant = tableview_itemmodel->data(selected_nym_id);
            QString selected_nym_id_string = selected_nym_id_variant.toString();
            
            //Paste into the "Withdraw as voucher" window in the receipient nym area.
            ((Moneychanger *)parentWidget())->set_systrayMenu_withdraw_asvoucher_nym_input(selected_nym_id_string);
            
            //Hide address book now that the operator has selected a nym
           dialog->hide();
        }
    }else{
        qDebug() << "nothing was selected to paste into the target area";
        QMessageBox::information(this,"Moneychanger","Nothing selected to paste.");
    }
}




/* Address Book Slots */
//When "add contact" is clicked, Add a blank row to the address book so the user can edit it and save their changes.
void AddressBookWindow::addblankrow_slot(){
    //Get total rows from the table view
    int total_rows_in_table = 0;
    total_rows_in_table = tableview_itemmodel->rowCount();
    
    //Insert blank row
    int blank_row_target_index = total_rows_in_table;
    QStandardItem * blank_row_item = new QStandardItem("");
    tableview_itemmodel->setItem(blank_row_target_index,0,blank_row_item);
    
    
}

//Confirm "remove contact" from address book
void AddressBookWindow::confirm_remove_contact_slot(){
    //First validate if anything is selected, before continuing (this is a must).
    QModelIndexList indexList = tableview->selectionModel()->selectedIndexes();
    int total_selected = indexList.size();
    if(total_selected >= 1){
        //Loop through every selected menu and delete it visually as well as through the database/storage.
        int total_deleted = 0;
        while(total_deleted < total_selected){
            //Increment right away
            total_deleted += 1;
            
            //Extract the database index id.
            QModelIndex data_row_model = indexList.at(0);
            QModelIndex db_id_model = tableview_itemmodel->index(data_row_model.row(), 2, QModelIndex());
            QVariant db_id_variant = tableview_itemmodel->data(db_id_model);
            int db_id = db_id_variant.toInt();
            //Delete data from the database/storage.
            DBHandler::getInstance()->AddressBookRemoveID(db_id);
            
            //Delete data from the visuals.
            tableview_itemmodel->removeRow(data_row_model.row());
            
        }
        
    }else{
        qDebug() <<"nothing was selected to be removed";
        QMessageBox::information(this,"Moneychanger","Nothing selected to remove.");
    }
}

//When a row is edited/updated this will be triggered to sync the changes to the database.
void AddressBookWindow::dataChanged_slot(QModelIndex topLeft, QModelIndex bottomRight){
    
    //This slot will ignore everything while the address book is refreshing; If not refreshing, go about regular logic.
    if(!refreshing){
        
        //Get Database Index that is associated with the edited row.
        int target_index_row = topLeft.row();
        QModelIndex index = tableview_itemmodel->index(target_index_row, 2, QModelIndex());
        QVariant index_id_variant = tableview_itemmodel->data(index);
        QString index_id_string = index_id_variant.toString();
        int index_id = index_id_string.toInt();
        
        
        //Get Nym Display Name
        QModelIndex nym_display_name = tableview_itemmodel->index(target_index_row, 0, QModelIndex());
        QVariant nym_display_name_variant = tableview_itemmodel->data(nym_display_name);
        QString nym_display_name_string = nym_display_name_variant.toString();
        
        //Get Nym Id
        QModelIndex nym_id = tableview_itemmodel->index(target_index_row, 1, QModelIndex());
        QVariant nym_id_variant = tableview_itemmodel->data(nym_id);
        QString nym_id_string = nym_id_variant.toString();
        
        //If Index_id is not greater than zero, then insert, if greater then zero, update.
        if(index_id == 0){
            //Before inserting, check if any data has been entered in
            if(nym_display_name_string != "" || nym_id_string != ""){
                //Get last insert id (This is so we can attribute the visual with a DB id.
                //Set the associated visual data with the row id from the database/storage index.
                tableview_itemmodel->setData(index, DBHandler::getInstance()->AddressBookInsertNym(nym_id_string, nym_display_name_string));
            }
        }else{
            //Update
            DBHandler::getInstance()->AddressBookUpdateNym(nym_id_string, nym_display_name_string, index_id_string);
        }
    }
}