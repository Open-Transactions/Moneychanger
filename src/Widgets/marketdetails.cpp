#include <QMessageBox>
#include <QDebug>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>

#include "marketdetails.h"
#include "ui_marketdetails.h"

#include "detailedit.h"
#include "moneychanger.h"
#include "overridecursor.h"

MTMarketDetails::MTMarketDetails(QWidget *parent, MTDetailEdit & theOwner) :
    MTEditDetails(parent, theOwner),
    m_pHeaderWidget(NULL),
    ui(new Ui::MTMarketDetails)
{
    ui->setupUi(this);
}

void MTMarketDetails::refresh(QString strID, QString strName)
{

}

void MTMarketDetails::AddButtonClicked()
{

}

void MTMarketDetails::DeleteButtonClicked()
{

}

void MTMarketDetails::ClearContents()
{

}

MTMarketDetails::~MTMarketDetails()
{
    delete ui;
}
