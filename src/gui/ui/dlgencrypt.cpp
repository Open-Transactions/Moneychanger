#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/ui/dlgencrypt.hpp>
#include <ui_dlgencrypt.h>

#include <QKeyEvent>

DlgEncrypt::DlgEncrypt(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgEncrypt)
{
    ui->setupUi(this);
    
    this->installEventFilter(this);
}

DlgEncrypt::~DlgEncrypt()
{
    delete ui;
}

bool DlgEncrypt::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        
        if(keyEvent->key() == Qt::Key_Escape)
        {
            close(); // This is caught by this same filter.
            return true;
        }
        return true;
    }
    else
    {
        // standard event processing
        return QDialog::eventFilter(obj, event);
    }
}
