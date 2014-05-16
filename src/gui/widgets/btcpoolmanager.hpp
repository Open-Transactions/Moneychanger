#ifndef BTCPOOLMANAGER_H
#define BTCPOOLMANAGER_H

#include <QWidget>

namespace Ui {
class BtcPoolManager;
}

class BtcPoolManager : public QWidget
{
    Q_OBJECT

public:
    explicit BtcPoolManager(QWidget *parent = 0);
    ~BtcPoolManager();

private:
    Ui::BtcPoolManager *ui;
};

#endif // BTCPOOLMANAGER_H
