#ifndef WIZARDADDCONTRACT_H
#define WIZARDADDCONTRACT_H

#include <QWizard>

namespace Ui {
class MTWizardAddContract;
}

class MTWizardAddContract : public QWizard
{
    Q_OBJECT

public:
    explicit MTWizardAddContract(QWidget *parent = 0);
    ~MTWizardAddContract();

    QString getContents() const;

private:
    Ui::MTWizardAddContract *ui;
};

#endif // WIZARDADDCONTRACT_H
