#ifndef ASSETMANAGERWINDOW_H
#define ASSETMANAGERWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QHBoxLayout>
#include <QTableView>
#include <QComboBox>
#include <QDialog>
#include <QDebug>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>
#include <opentxs/OTLog.h>

#include "moneychanger.h"

class AssetManagerWindow : public QWidget
{
    Q_OBJECT
public:
    explicit AssetManagerWindow(QWidget *parent = 0);
    
    void dialog();
    
    
private:
    
    /** Asset Manager **/
    bool already_init;
    bool refreshing;
    
    QDialog * asset_manager_dialog;
    //Grid layout
    QGridLayout * asset_manager_gridlayout;
    
    /** First row **/
    //Label (Asset Manager Header)
    QLabel * asset_manager_label;
    
    /** Second Row **/
    //Horizontal holder (List of asset contracts; Add/Remove asset button)
    QWidget     * asset_manager_holder;
    QHBoxLayout * asset_manager_hbox;
    
    //Tableview/item model for asset list.
    QStandardItemModel * asset_manager_tableview_itemmodel;
    QTableView         * asset_manager_tableview;
    
    //Vertical holder (add/remove asset buttons)
    QWidget * asset_manager_addremove_btngroup_holder;
    QVBoxLayout * asset_manager_addremove_btngroup_vbox;
    
    //Add asset button
    QPushButton * asset_manager_addremove_btngroup_addbtn;
    
    //Remove asset button
    QPushButton * asset_manager_addremove_btngroup_removebtn;
    
    /** Third Row (most recent error) **/
    QLabel * asset_manager_most_recent_erorr;
    // ------------------------------------------------
    /** "Add Asset" Dialog **/
    bool addasset_dialog_already_init;
    bool addasset_dialog_advanced_showing;
    
    QDialog * asset_manager_addasset_dialog;
    //Grid layout
    QGridLayout * asset_manager_addasset_gridlayout;
    //Label (header)
    QLabel * asset_manager_addasset_header;
    
    //Label (Toggle Advanced Options Label/Button)
    QLabel * asset_manager_addasset_subheader_toggleadvanced_options_label;
    
    //Label (instructions)
    QLabel * asset_manager_addasset_subheader_instructions;
    
    //Label (choose source)
    QLabel * asset_manager_addasset_choosesource_label;
    
    //Combobox (choose source)
    QComboBox * asset_manager_addasset_choosesource_answer_selection;
    
    //Button (create asset)
    QPushButton * asset_manager_addasset_create_asset_btn;
    // ------------------------------------------------
    /** "Remove Asset Dialog **/
    bool assetmanager_removeasset_dialog_already_init;
    QDialog * asset_manager_removeasset_dialog;
    //Grid layout
    QGridLayout * asset_manager_removeasset_gridlayout;
    
    //Label (header)
    QLabel * asset_manager_removeasset_header;
    // ------------------------------------------------
    /** Asset Manger Slot locks **/
    bool assetmanager_proccessing_dataChanged;
    // ------------------------------------------------


signals:

private slots:
    
    void addasset_slot();
    void removeasset_slot();
    void dataChanged_slot(QModelIndex topLeft, QModelIndex bottomRight);
    void dialog_showadvanced_slot(QString link_href);
    void request_remove_asset_slot();
    void addasset_dialog_createasset_slot();
    
protected:
    
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // ASSETMANAGERWINDOW_H
