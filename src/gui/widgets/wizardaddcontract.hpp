#ifndef WIZARDADDCONTRACT_HPP
#define WIZARDADDCONTRACT_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

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

#endif // WIZARDADDCONTRACT_HPP
