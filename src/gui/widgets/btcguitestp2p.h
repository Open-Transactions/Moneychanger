#ifndef BTCGUITESTP2P_H
#define BTCGUITESTP2P_H

#include <QWidget>

namespace Ui {
class BtcGuiTestp2p;
}

class BtcGuiTestp2p : public QWidget
{
    Q_OBJECT

public:
    explicit BtcGuiTestp2p(QWidget *parent = 0);
    ~BtcGuiTestp2p();

private slots:
    void on_simulatePoolButton_clicked();

private:
    Ui::BtcGuiTestp2p *ui;
};

#endif // BTCGUITESTP2P_H
