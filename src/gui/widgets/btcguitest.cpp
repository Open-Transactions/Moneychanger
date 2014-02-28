#include "widgets/btcguitest.h"
#include "ui_btcguitest.h"
#include "modules.h"
#include "btctest.h"
#include <OTLog.hpp>

BtcGuiTest::BtcGuiTest(QWidget *parent) :
    QWidget(parent, Qt::Window),
    ui(new Ui::BtcGuiTest)
{
    ui->setupUi(this);

    QObject::connect(this, SIGNAL(SetConfirmationsDeposit(int)), this->ui->confToEscrNumber, SLOT(display(int)));
    QObject::connect(this, SIGNAL(SetConfirmationsWithdrawal(int)), this->ui->confToWithdrNumber, SLOT(display(int)));
}

BtcGuiTest::~BtcGuiTest()
{
    delete ui;
}

void BtcGuiTest::on_testButton_clicked()
{
    if(BtcTest::TestBitcoinFunctions())
    {
        OTLog::Output(0, "Successfully tested bitcoin functionality.");
    }
    else
    {
        OTLog::Output(0, "Error testing bitcoin functionality.\nMaybe test environment is not set up properly?");
    }

    /*  deprecated:
    if(!Modules::btcInterface->TestBtcJson())
        OTLog::vOutput(0, "Error testing bitcoin integration. Maybe test environment is not set up.\n");
    else
        OTLog::vOutput(0, "Bitcoin integration successfully tested.\n");

    if(!Modules::btcInterface->TestBtcJsonEscrowTwoOfTwo())
        OTLog::vOutput(0, "Error testing bitcoin escrow functions. Maybe test environment is not set up.\n");
    else
        OTLog::vOutput(0, "Bitcoin escrow integration sucessfully tested.\n");
    */
}

void BtcGuiTest::on_simulateEscrSrvrsButton_clicked()
{
    Modules::sampleEscrowManager->OnSimulateEscrowServers();
}

void BtcGuiTest::on_sendToEscrButton_clicked()
{
    Modules::sampleEscrowManager->OnInitializeEscrow(this);
}

void BtcGuiTest::on_reqWithdrButton_clicked()
{
    Modules::sampleEscrowManager->OnRequestWithdrawal(this);
}

double BtcGuiTest::GetAmountToSend()
{
    return this->ui->amountToEscrSpinBox->value();
}

void BtcGuiTest::SetMultiSigAddress(const std::string &address)
{
    this->ui->depositAddrEdit->setText(QString::fromStdString(address));
}

void BtcGuiTest::SetTxIdDeposit(const std::string &txId)
{
    this->ui->txidToEscrEdit->setText(QString::fromStdString(txId));
}

void BtcGuiTest::OnSetConfirmationsDeposit(int confirms)
{
    this->SetConfirmationsDeposit(confirms);
}

void BtcGuiTest::SetStatusDeposit(SampleEscrowTransaction::SUCCESS status)
{
    QString statusText;
    switch(status)
    {
    case SampleEscrowTransaction::NotStarted:
        statusText = "Not started";
        break;
    case SampleEscrowTransaction::Pending:
        statusText = "Transaction pending";
        break;
    case SampleEscrowTransaction::Successfull:
        statusText = "Success";
        break;
    case SampleEscrowTransaction::Failed:
        statusText = "Failed";
        break;
    }
    this->ui->depositStatusLabel->setText(statusText);
}

void BtcGuiTest::SetWithdrawalAddress(const std::string &address)
{
    this->ui->withdrawAddrEdit->setText(QString::fromStdString(address));
}

void BtcGuiTest::SetTxIdWithdrawal(const std::string &txId)
{
    this->ui->txidWithdrEdit->setText(QString::fromStdString(txId));
}

void BtcGuiTest::OnSetConfirmationsWithdrawal(int confirms)
{
    this->SetConfirmationsWithdrawal(confirms);
}

void BtcGuiTest::SetStatusWithdrawal(SampleEscrowTransaction::SUCCESS status)
{
    QString statusText;
    switch(status)
    {
        case SampleEscrowTransaction::NotStarted:
            statusText = "Not started";
            break;
        case SampleEscrowTransaction::Pending:
            statusText = "Transaction pending";
            break;
        case SampleEscrowTransaction::Successfull:
            statusText = "Success";
            break;
        case SampleEscrowTransaction::Failed:
            statusText = "Failed";
            break;
    }
    this->ui->withdrawStatusLabel->setText(statusText);
}
