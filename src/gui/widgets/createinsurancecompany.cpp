#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/createinsurancecompany.hpp>
#include <ui_createinsurancecompany.h>

#include <QKeyEvent>

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

    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Escape)
        {
            close(); // This is caught by this same filter.
            return true;
        }
        return true;
    }
    else
    {
        // standard event processing
        return QWizard::eventFilter(obj, event);
    }
}
