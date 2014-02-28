#ifndef STABLE_H
#include <core/stable.h>
#endif

#include <QMessageBox>
#include <QDebug>

#ifdef _WIN32
#include <otapi/OTAPI.hpp>
#include <otapi/OT_ME.hpp>
#else
#include <opentxs/OTAPI.hpp>
#include <opentxs/OT_ME.hpp>
#endif

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
