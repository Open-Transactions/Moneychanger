#include "pagenym_source.h"
#include "ui_pagenym_source.h"

#include "Namecoin.hpp"

#include <string>

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
        nmc.getNamecoin ().testConnection (msg);
        ui->labelExtra->setText(msg.c_str ());
    }
    else if (2 == nAuthorityIndex) // Legacy CA
    {
        ui->label->setText(tr("DN info:"));
        ui->labelExplanation->setText(tr("Enter the DN info that uniquely identifies you on your x.509 certificate."));
    }
}

//virtual
bool MTPageNym_Source::validatePage()
{
  const int nAuthorityIndex = field("Authority").toInt();

  /* In the case of Namecoin source, check that the Namecoin address
     entered is valid and owned by the user.  */
  if (nAuthorityIndex == 1)
    {
      const std::string addrStr = ui->lineEditSource->text().toStdString();

      NMC_Interface nmc;
      nmcrpc::NamecoinInterface& nc = nmc.getNamecoin();
      const bool ok = nc.testConnection ();

      if (!ok)
        {
          // The connection error message should still be shown.
          return false;
        }

      const nmcrpc::NamecoinInterface::Address addr = nc.queryAddress(addrStr);
      if (!addr.isValid ())
        {
          ui->labelExtra->setText(tr("Please enter a valid Namecoin address."));
          return false;
        }
      if (!addr.isMine ())
        {
          ui->labelExtra->setText(tr("The provided Namecoin address is not in your wallet."));
          return false;
        }
    }

  return true;
}

MTPageNym_Source::~MTPageNym_Source()
{
    delete ui;
}
