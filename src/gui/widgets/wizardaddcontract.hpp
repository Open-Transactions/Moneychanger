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
    void setServerMode();

    bool assetMode()  const { return bAssetMode_;  }
    bool serverMode() const { return bServerMode_; }

private:
    Ui::MTWizardAddContract *ui;

    bool bAssetMode_ = false;
    bool bServerMode_ = false;
};

#endif // WIZARDADDCONTRACT_HPP
