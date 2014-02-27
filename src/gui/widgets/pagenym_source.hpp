#ifndef PAGENYM_SOURCE_HPP
#define PAGENYM_SOURCE_HPP

#include <WinsockWrapper.h>
#include <ExportWrapper.h>

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

private:
    Ui::MTPageNym_Source *ui;
};

#endif // PAGENYM_SOURCE_HPP
