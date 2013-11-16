#include <QMessageBox>
#include <QDebug>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>

#include "offerdetails.h"
#include "ui_offerdetails.h"

#include "detailedit.h"
#include "moneychanger.h"
#include "overridecursor.h"

MTOfferDetails::MTOfferDetails(QWidget *parent, MTDetailEdit & theOwner) :
    MTEditDetails(parent, theOwner),
    m_pHeaderWidget(NULL),
    ui(new Ui::MTOfferDetails)
{
    ui->setupUi(this);
}


void MTOfferDetails::refresh(QString strID, QString strName)
{

}

void MTOfferDetails::AddButtonClicked()
{

}

void MTOfferDetails::DeleteButtonClicked()
{

}

void MTOfferDetails::ClearContents()
{

}

MTOfferDetails::~MTOfferDetails()
{
    delete ui;
}
