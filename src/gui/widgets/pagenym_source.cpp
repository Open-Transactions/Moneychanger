#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/pagenym_source.hpp>
#include <ui_pagenym_source.h>

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

    // Default is no extra label and no button.
    ui->labelExtra->setText("");
    ui->getAddress->setVisible(false);

    if (2 == nAuthorityIndex) // Legacy CA
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
    }

  return true;
}

MTPageNym_Source::~MTPageNym_Source()
{
    delete ui;
}

void MTPageNym_Source::getAddressClicked()
{
//  NMC_Interface nmc;
//  nmcrpc::NamecoinInterface::Address addr;
//  addr = nmc.getNamecoin().createAddress();
//
//  ui->lineEditSource->setText(addr.getAddress().c_str());
}
