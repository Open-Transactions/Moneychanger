#ifndef PAGEADDCONTRACT_H
#define PAGEADDCONTRACT_H

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

#endif // PAGEADDCONTRACT_H
