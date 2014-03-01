#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/corporationdetails.hpp>
#include <ui_corporationdetails.h>


MTCorporationDetails::MTCorporationDetails(QWidget *parent, MTDetailEdit & theOwner) :
    MTEditDetails(parent, theOwner),
    m_pHeaderWidget(NULL),
    ui(new Ui::MTCorporationDetails)
{
    ui->setupUi(this);
}

void MTCorporationDetails::refresh(QString strID, QString strName)
{

}

void MTCorporationDetails::AddButtonClicked()
{

}

void MTCorporationDetails::DeleteButtonClicked()
{

}

void MTCorporationDetails::ClearContents()
{

}

MTCorporationDetails::~MTCorporationDetails()
{
    delete ui;
}
