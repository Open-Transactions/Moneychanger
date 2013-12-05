#include "stdafx.h"

#include <QMessageBox>
#include <QDebug>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>

#include "corporationdetails.h"
#include "ui_corporationdetails.h"

#include "detailedit.h"
#include "moneychanger.h"
#include "overridecursor.h"


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
