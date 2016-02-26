#ifndef WIZARDADDCONTRACT_HPP
#define WIZARDADDCONTRACT_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

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

    void setAssetMode();

private:
    Ui::MTWizardAddContract *ui;

    bool bIsAssetMode_ = false;
};

#endif // WIZARDADDCONTRACT_HPP
