#ifndef CreateInsuranceCompany_HPP
#define CreateInsuranceCompany_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <QWizard>


namespace Ui {
class CreateInsuranceCompany;
}

class CreateInsuranceCompany : public QWizard
{
    Q_OBJECT

public:
    explicit CreateInsuranceCompany(QWidget *parent = 0);
    ~CreateInsuranceCompany();

private:
    Ui::CreateInsuranceCompany *ui;

protected:

    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // CreateInsuranceCompany_HPP
