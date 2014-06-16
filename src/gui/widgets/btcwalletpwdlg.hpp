#ifndef BTCWALLETPWDLG_H
#define BTCWALLETPWDLG_H

#include <QWidget>

#include <opentxs/TR1_Wrapper.hpp>

namespace Ui {
class BtcWalletPwDlg;
}

class QMutex;
class BtcJson;

class BtcWalletPwDlg : public QWidget
{
    Q_OBJECT

public:
    explicit BtcWalletPwDlg(QWidget *parent = 0);
    ~BtcWalletPwDlg();

    std::string WaitForPassword();

    std::string password;

private slots:
    void on_buttonUnlock_clicked();

private:
    Ui::BtcWalletPwDlg *ui;

    _SharedPtr<QMutex> mutex;
    _SharedPtr<BtcJson> btcJson;
    bool waitingForInput;
};

typedef _SharedPtr<BtcWalletPwDlg> BtcWalletPwDlgPtr;

#endif // BTCWALLETPWDLG_H
