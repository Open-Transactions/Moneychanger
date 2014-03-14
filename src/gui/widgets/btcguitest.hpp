#ifndef BTCGUITEST_HPP
#define BTCGUITEST_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <bitcoin/sampleescrowtransaction.hpp>

#include <QWidget>


namespace Ui
{
    class BtcGuiTest;
}


class BtcGuiTest : public QWidget
{
public:
    explicit BtcGuiTest(QWidget *parent = 0);
    ~BtcGuiTest();

    double GetAmountToSend();

private:
    Q_OBJECT
    Ui::BtcGuiTest *ui;

private slots:
    void on_testButton_clicked();
    void on_simulateEscrSrvrsButton_clicked();
    void on_sendToEscrButton_clicked();
    void on_reqWithdrButton_clicked();

    // for updating the GUI:
    void SetMultiSigAddress(const std::string& address);
    void SetTxIdDeposit(const std::string& txId);
    void OnSetConfirmationsDeposit(int confirms);
    void SetStatusDeposit(SampleEscrowTransaction::SUCCESS status);

    void SetWithdrawalAddress(const std::string& address);
    void SetTxIdWithdrawal(const std::string& txId);
    void OnSetConfirmationsWithdrawal(int confirms);
    void SetStatusWithdrawal(SampleEscrowTransaction::SUCCESS status);

signals:
    void SetConfirmationsDeposit(int confirms);
    void SetConfirmationsWithdrawal(int confirms);
};

#endif // BTCGUITEST_HPP
