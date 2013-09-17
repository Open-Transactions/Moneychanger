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
    int mc_assetmanager_already_init;
    int mc_assetmanager_refreshing;
    
    QDialog * mc_asset_manager_dialog;
    //Grid layout
    QGridLayout * mc_asset_manager_gridlayout;
    
    /** First row **/
    //Label (Asset Manager Header)
    QLabel * mc_asset_manager_label;
    
    /** Second Row **/
    //Horizontal holder (List of asset contracts; Add/Remove asset button)
    QWidget     * mc_asset_manager_holder;
    QHBoxLayout * mc_asset_manager_hbox;
    
    //Tableview/item model for asset list.
    QStandardItemModel * mc_asset_manager_tableview_itemmodel;
    QTableView         * mc_asset_manager_tableview;
    
    //Vertical holder (add/remove asset buttons)
    QWidget * mc_asset_manager_addremove_btngroup_holder;
    QVBoxLayout * mc_asset_manager_addremove_btngroup_vbox;
    
    //Add asset button
    QPushButton * mc_asset_manager_addremove_btngroup_addbtn;
    
    //Remove asset button
    QPushButton * mc_asset_manager_addremove_btngroup_removebtn;
    
    /** Third Row (most recent error) **/
    QLabel * mc_asset_manager_most_recent_erorr;
    // ------------------------------------------------
    /** "Add Asset" Dialog **/
    bool mc_assetmanager_addasset_dialog_already_init;
    bool mc_assetmanager_addasset_dialog_advanced_showing;
    
    QDialog * mc_asset_manager_addasset_dialog;
    //Grid layout
    QGridLayout * mc_asset_manager_addasset_gridlayout;
    //Label (header)
    QLabel * mc_asset_manager_addasset_header;
    
    //Label (Toggle Advanced Options Label/Button)
    QLabel * mc_asset_manager_addasset_subheader_toggleadvanced_options_label;
    
    //Label (instructions)
    QLabel * mc_asset_manager_addasset_subheader_instructions;
    
    //Label (choose source)
    QLabel * mc_asset_manager_addasset_choosesource_label;
    
    //Combobox (choose source)
    QComboBox * mc_asset_manager_addasset_choosesource_answer_selection;
    
    //Button (create asset)
    QPushButton * mc_asset_manager_addasset_create_asset_btn;
    // ------------------------------------------------
    /** "Remove Asset Dialog **/
    bool mc_assetmanager_removeasset_dialog_already_init;
    QDialog * mc_asset_manager_removeasset_dialog;
    //Grid layout
    QGridLayout * mc_asset_manager_removeasset_gridlayout;
    
    //Label (header)
    QLabel * mc_asset_manager_removeasset_header;
    // ------------------------------------------------
    /** Asset Manger Slot locks **/
    bool mc_assetmanager_proccessing_dataChanged;
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
