#include <QMessageBox>
#include <QDebug>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>

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
