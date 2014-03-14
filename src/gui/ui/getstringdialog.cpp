#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/ui/getstringdialog.hpp>
#include <ui_getstringdialog.h>

#include <QKeyEvent>


MTGetStringDialog::MTGetStringDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MTGetStringDialog)
{
    ui->setupUi(this);

    this->installEventFilter(this);
}

MTGetStringDialog::~MTGetStringDialog()
{
    delete ui;
}

bool MTGetStringDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape){
            close(); // This is caught by this same filter.
            return true;
        }
        return true;
    }else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

void MTGetStringDialog::on_buttonBox_accepted()
{
    m_Output = ui->lineEdit->displayText();

    QDialog::accept();
}

void MTGetStringDialog::on_buttonBox_rejected()
{
    QDialog::reject();
}

