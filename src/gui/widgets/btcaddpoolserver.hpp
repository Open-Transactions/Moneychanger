#ifndef BTCADDPOOLSERVER_H
#define BTCADDPOOLSERVER_H

#include <QWidget>

namespace Ui {
class BtcAddPoolServer;
}

class BtcPoolManager;

class BtcAddPoolServer : public QWidget
{
    Q_OBJECT

public:
    explicit BtcAddPoolServer(QWidget *parent = 0);
    ~BtcAddPoolServer();

private slots:
    void on_buttonConnect_clicked();

    void on_buttonCreate_clicked();

    void on_buttonSimulate_clicked();

private:
    Ui::BtcAddPoolServer *ui;
};

#endif // BTCADDPOOLSERVER_H
