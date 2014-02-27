#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <QKeyEvent>

#include <QMessageBox>

#include "dlgmenu.h"
#include "ui_dlgmenu.h"

DlgMenu::DlgMenu(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgMenu)
{
    ui->setupUi(this);

    this->installEventFilter(this);
}

bool DlgMenu::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Escape)
        {
//          close(); // This is caught by this same filter.
            return true;
        }
    }

    // standard event processing
    return QDialog::eventFilter(obj, event);
}


void DlgMenu::closeEvent(QCloseEvent *event)
{
    QMessageBox::information(this, tr("Quit?"),
                            tr("If you want to close the application, click the 'Quit' button"));
    event->ignore();
//  QDialog::closeEvent(event);
}


DlgMenu::~DlgMenu()
{
    delete ui;
}


void DlgMenu::dialog()
{
    show();
    setFocus();
}


void DlgMenu::on_toolButton_main_clicked()
{
    emit sig_on_toolButton_main_clicked();
}

void DlgMenu::on_toolButton_markets_clicked()
{
    emit sig_on_toolButton_markets_clicked();
}

void DlgMenu::on_toolButton_importCash_clicked()
{
    emit sig_on_toolButton_importCash_clicked();
}

void DlgMenu::on_toolButton_manageAccounts_clicked()
{
    emit sig_on_toolButton_manageAccounts_clicked();
}

void DlgMenu::on_toolButton_manageAssets_clicked()
{
    emit sig_on_toolButton_manageAssets_clicked();
}

void DlgMenu::on_toolButton_manageNyms_clicked()
{
    emit sig_on_toolButton_manageNyms_clicked();
}

void DlgMenu::on_toolButton_manageServers_clicked()
{
    emit sig_on_toolButton_manageServers_clicked();
}

void DlgMenu::on_toolButton_smartContracts_clicked()
{
    emit sig_on_toolButton_smartContracts_clicked();
}

void DlgMenu::on_toolButton_Corporations_clicked()
{
    emit sig_on_toolButton_Corporations_clicked();
}

void DlgMenu::on_toolButton_settings_clicked()
{
    emit sig_on_toolButton_settings_clicked();
}

void DlgMenu::on_toolButton_quit_clicked()
{
    emit sig_on_toolButton_quit_clicked();
}




