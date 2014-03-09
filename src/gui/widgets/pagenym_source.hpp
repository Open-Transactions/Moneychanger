#ifndef PAGENYM_SOURCE_HPP
#define PAGENYM_SOURCE_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <QWizardPage>

namespace Ui {
class MTPageNym_Source;
}

class MTPageNym_Source : public QWizardPage
{
    Q_OBJECT

public:
    explicit MTPageNym_Source(QWidget *parent = 0);
    ~MTPageNym_Source();

protected:
    virtual void showEvent(QShowEvent * event);
    virtual bool validatePage();

private:
    Ui::MTPageNym_Source *ui;

private slots:
    void getAddressClicked();
};

#endif // PAGENYM_SOURCE_HPP
