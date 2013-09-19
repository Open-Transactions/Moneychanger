#include "createinsurancecompany.h"
#include "ui_createinsurancecompany.h"

CreateInsuranceCompany::CreateInsuranceCompany(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::CreateInsuranceCompany)
{
    ui->setupUi(this);
}

CreateInsuranceCompany::~CreateInsuranceCompany()
{
    delete ui;
}

bool CreateInsuranceCompany::eventFilter(QObject *obj, QEvent *event){

    if (event->type() == QEvent::Close) {
        ((Moneychanger *)parentWidget())->close_createinsurancecompany_dialog();
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
