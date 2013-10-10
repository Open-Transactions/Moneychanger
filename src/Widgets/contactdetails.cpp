
#include <QDebug>

#include "contactdetails.h"
#include "ui_contactdetails.h"

#include "Handlers/contacthandler.h"

#include "detailedit.h"

MTContactDetails::MTContactDetails(QWidget *parent) :
    MTEditDetails(parent),
    ui(new Ui::MTContactDetails)
{
    ui->setupUi(this);
    this->setContentsMargins(0, 0, 0, 0);
}

MTContactDetails::~MTContactDetails()
{
    delete ui;
}

//virtual
void MTContactDetails::DeleteButtonClicked()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        bool bSuccess = MTContactHandler::getInstance()->DeleteContact(m_pOwner->m_qstrCurrentID.toInt());

        if (bSuccess)
        {
            m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
            m_pOwner->RefreshRecords();
        }
    }
}

//virtual
void MTContactDetails::refresh(QString strID, QString strName)
{
    qDebug() << "MTContactDetails::refresh";

    ui->lineEditID  ->setText(strID);
    ui->lineEditName->setText(strName);

}


// Add a "delete contact" function that the owner dialog can use.
// It will use this to delete the contact from the SQL db:
//
//         bool MTContactHandler::DeleteContact(int nContactID);

void MTContactDetails::on_lineEditName_editingFinished()
{
    int nContactID = m_pOwner->m_qstrCurrentID.toInt();

    if (nContactID > 0)
    {
        bool bSuccess = MTContactHandler::getInstance()->SetContactName(nContactID, ui->lineEditName->text());

        if (bSuccess)
        {
            m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
            m_pOwner->m_map.insert(m_pOwner->m_qstrCurrentID, ui->lineEditName->text());

            m_pOwner->RefreshRecords();
        }
    }
}
