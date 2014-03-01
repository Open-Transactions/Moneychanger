#ifndef PAGEADDCONTRACT_HPP
#define PAGEADDCONTRACT_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <QWizardPage>

namespace Ui {
class MTPageAddContract;
}

class MTPageAddContract : public QWizardPage
{
    Q_OBJECT

public:
    explicit MTPageAddContract(QWidget *parent = 0);
    ~MTPageAddContract();

    //virtual
    int nextId() const;

private:
    Ui::MTPageAddContract *ui;
};

#endif // PAGEADDCONTRACT_HPP
