#ifndef CREATEINSURANCECOMPANY_H
#define CREATEINSURANCECOMPANY_H

#include <QWizard>

namespace Ui {
class createinsurancecompany;
}

class createinsurancecompany : public QWizard
{
    Q_OBJECT

public:
    explicit createinsurancecompany(QWidget *parent = 0);
    ~createinsurancecompany();

private:
    Ui::createinsurancecompany *ui;
};

#endif // CREATEINSURANCECOMPANY_H
