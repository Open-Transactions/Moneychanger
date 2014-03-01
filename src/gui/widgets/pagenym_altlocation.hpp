#ifndef PAGENYM_ALTLOCATION_HPP
#define PAGENYM_ALTLOCATION_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <QWizardPage>

namespace Ui {
class MTPageNym_AltLocation;
}

class MTPageNym_AltLocation : public QWizardPage
{
    Q_OBJECT

public:
    explicit MTPageNym_AltLocation(QWidget *parent = 0);
    ~MTPageNym_AltLocation();

private:
    Ui::MTPageNym_AltLocation *ui;
};

#endif // PAGENYM_ALTLOCATION_HPP
