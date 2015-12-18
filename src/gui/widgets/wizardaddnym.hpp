#ifndef WIZARDADDNYM_HPP
#define WIZARDADDNYM_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <gui/widgets/pagenym_altlocation.hpp>

#include <QWizard>

namespace Ui {
class MTWizardAddNym;
}

class MTWizardAddNym : public QWizard
{
    Q_OBJECT

public:
    explicit MTWizardAddNym(QWidget *parent = 0);
    ~MTWizardAddNym();

private:
    Ui::MTWizardAddNym *ui;

public:
    listContactDataTuples listContactDataTuples_;
};

#endif // WIZARDADDNYM_HPP
