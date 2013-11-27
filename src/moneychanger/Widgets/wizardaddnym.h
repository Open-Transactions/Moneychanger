#ifndef WIZARDADDNYM_H
#define WIZARDADDNYM_H

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
};

#endif // WIZARDADDNYM_H
