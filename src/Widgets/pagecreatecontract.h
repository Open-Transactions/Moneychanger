#ifndef PAGECREATECONTRACT_H
#define PAGECREATECONTRACT_H

#include <QWizardPage>

namespace Ui {
class MTPageCreateContract;
}

class MTPageCreateContract : public QWizardPage
{
    Q_OBJECT

public:
    explicit MTPageCreateContract(QWidget *parent = 0);
    ~MTPageCreateContract();

    //virtual
    int nextId() const;

private:
    Ui::MTPageCreateContract *ui;
};

#endif // PAGECREATECONTRACT_H
