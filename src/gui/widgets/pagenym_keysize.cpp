#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/pagenym_keysize.hpp>
#include <ui_pagenym_keysize.h>


MTPageNym_Keysize::MTPageNym_Keysize(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::MTPageNym_Keysize)
{
    ui->setupUi(this);

    this->registerField("Keysize", ui->comboBoxKeysize);
}

MTPageNym_Keysize::~MTPageNym_Keysize()
{
    delete ui;
}

/*

  If it's rooted in Namecoin, then I need to know the Namecoin address.

  If it's rooted in Certificate Authority, then I need the DN info. (Or the Cert, to get the DN info from.)

  If it's Self-Signed, I don't need those things, since the public key will become the source.

  In ALL cases, I will have an Alternate Location.
*/
