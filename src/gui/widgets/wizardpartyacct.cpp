#include "wizardpartyacct.hpp"
#include "ui_wizardpartyacct.h"

WizardPartyAcct::WizardPartyAcct(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::WizardPartyAcct)
{
    ui->setupUi(this);
}

WizardPartyAcct::~WizardPartyAcct()
{
    delete ui;
}

// m_mapConfirmed

