#ifndef PAGEADDCONTRACT_HPP
#define PAGEADDCONTRACT_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

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

    //virtual
    bool validatePage();
    void initializePage();

private:
    Ui::MTPageAddContract *ui;
};

#endif // PAGEADDCONTRACT_HPP
