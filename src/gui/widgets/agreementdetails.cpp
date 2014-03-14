#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/agreementdetails.hpp>
#include <ui_agreementdetails.h>


MTAgreementDetails::MTAgreementDetails(QWidget *parent, MTDetailEdit & theOwner) :
    MTEditDetails(parent, theOwner),
    m_pHeaderWidget(NULL),
    ui(new Ui::MTAgreementDetails)
{
    ui->setupUi(this);
}

void MTAgreementDetails::refresh(QString strID, QString strName)
{

}

void MTAgreementDetails::AddButtonClicked()
{

}

void MTAgreementDetails::DeleteButtonClicked()
{

}

void MTAgreementDetails::ClearContents()
{

}

MTAgreementDetails::~MTAgreementDetails()
{
    delete ui;
}
