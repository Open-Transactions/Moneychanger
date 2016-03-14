#ifndef PAGECONTRACT_TERMS_HPP
#define PAGECONTRACT_TERMS_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <QWizardPage>

namespace Ui {
class PageContract_Terms;
}

class PageContract_Terms : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageContract_Terms(QWidget *parent = 0);
    ~PageContract_Terms();

    //virtual
    int nextId() const;

private:
    Ui::PageContract_Terms *ui;
};

#endif // PAGECONTRACT_TERMS_HPP
