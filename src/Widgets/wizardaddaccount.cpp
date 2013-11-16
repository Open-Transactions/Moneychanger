#include "wizardaddaccount.h"
#include "ui_wizardaddaccount.h"

#include "moneychanger.h"

MTWizardAddAccount::MTWizardAddAccount(QWidget *parent, Moneychanger & theMC) :
    QWizard(parent),
    m_pMoneychanger(&theMC),
    ui(new Ui::MTWizardAddAccount)
{
    ui->setupUi(this);

    setOption(QWizard::NoCancelButton,     false);
    setOption(QWizard::CancelButtonOnLeft, true);
}


Moneychanger * MTWizardAddAccount::GetMoneychanger() { return m_pMoneychanger ? m_pMoneychanger.data() : NULL; }

MTWizardAddAccount::~MTWizardAddAccount()
{
    delete ui;
}
