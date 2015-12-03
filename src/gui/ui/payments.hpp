#ifndef PAYMENTS_HPP
#define PAYMENTS_HPP

#include <QWidget>

namespace Ui {
class Payments;
}

class Payments : public QWidget
{
    Q_OBJECT

public:
    explicit Payments(QWidget *parent = 0);
    ~Payments();

private:
    Ui::Payments *ui;
};

#endif // PAYMENTS_HPP
