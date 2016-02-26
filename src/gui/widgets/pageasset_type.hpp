#ifndef PAGEASSET_TYPE_HPP
#define PAGEASSET_TYPE_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <QWizardPage>

namespace Ui {
class PageAsset_Type;
}

class PageAsset_Type : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageAsset_Type(QWidget *parent = 0);
    ~PageAsset_Type();

private:
    Ui::PageAsset_Type *ui;
};

#endif // PAGEASSET_TYPE_HPP
