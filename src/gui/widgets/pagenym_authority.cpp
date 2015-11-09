#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/pagenym_authority.hpp>
#include <ui_pagenym_authority.h>


MTPageNym_Authority::MTPageNym_Authority(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::MTPageNym_Authority)
{
    ui->setupUi(this);

    this->setCommitPage(true);

    this->registerField("Authority", ui->comboBoxAuthority);
    this->registerField("Algorithm", ui->comboBoxAlgorithm);
}


//virtual
int MTPageNym_Authority::nextId() const
{
    if (0 == ui->comboBoxAuthority->currentIndex()) // Self-Signed
        return 3; // Skip source and go straight to alternate location.

    return 2; // Get the source and then the alternate location.
}

MTPageNym_Authority::~MTPageNym_Authority()
{
    delete ui;
}
