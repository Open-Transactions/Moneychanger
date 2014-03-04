#ifndef BTCGUITESTP2P_HPP
#define BTCGUITESTP2P_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

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

#endif // BTCGUITESTP2P_HPP
