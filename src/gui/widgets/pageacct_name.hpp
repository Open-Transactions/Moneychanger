#ifndef PAGEACCT_NAME_H
#define PAGEACCT_NAME_H

#include <QWizardPage>

namespace Ui {
class MTPageAcct_Name;
}

class MTPageAcct_Name : public QWizardPage
{
    Q_OBJECT

public:
    explicit MTPageAcct_Name(QWidget *parent = 0);
    ~MTPageAcct_Name();

private:
    Ui::MTPageAcct_Name *ui;
};

#endif // PAGEACCT_NAME_H
