#include "pagenym_source.h"
#include "ui_pagenym_source.h"

#include "Namecoin.hpp"

MTPageNym_Source::MTPageNym_Source(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::MTPageNym_Source)
{
    ui->setupUi(this);

    this->registerField("Source*", ui->lineEditSource);
}

//virtual
void MTPageNym_Source::showEvent(QShowEvent * event)
{
    // -------------------------------
    // call inherited method
    //
    QWizardPage::showEvent(event);
    // -------------------------------
    int nAuthorityIndex = field("Authority").toInt();

    // Default is no extra label.
    ui->labelExtra->setText("");

    if (1 == nAuthorityIndex) // Namecoin
    {
        ui->label->setText(tr("Namecoin Address:"));
        ui->labelExplanation->setText(tr("Enter the Namecoin address that will be authoritative over this identity."));

        NMC_Interface nmc;
        std::string msg;
        const bool ok = nmc.getNamecoin ().testConnection (msg);
        ui->labelExtra->setText(msg.c_str ());
    }
    else if (2 == nAuthorityIndex) // Legacy CA
    {
        ui->label->setText(tr("DN info:"));
        ui->labelExplanation->setText(tr("Enter the DN info that uniquely identifies you on your x.509 certificate."));
    }
}

MTPageNym_Source::~MTPageNym_Source()
{
    delete ui;
}
