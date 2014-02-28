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

#include "agreementdetails.h"
#include "ui_agreementdetails.h"

#include "detailedit.h"
#include "moneychanger.h"
#include "overridecursor.h"


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
