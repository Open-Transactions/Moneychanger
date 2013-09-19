#include "createinsurancecompany.h"
#include "ui_createinsurancecompany.h"

createinsurancecompany::createinsurancecompany(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::createinsurancecompany)
{
    ui->setupUi(this);
}

createinsurancecompany::~createinsurancecompany()
{
    delete ui;
}

bool createinsurancecompany::eventFilter(QObject *obj, QEvent *event){

    if (event->type() == QEvent::Close) {
        ((createinsurancecompany *)parentWidget())->close_createinsurancecompany_dialog();
        return true;
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape){
            close(); // This is caught by this same filter.
            return true;
        }
        return true;
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}
