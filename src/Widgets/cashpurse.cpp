#include "cashpurse.h"
#include "ui_cashpurse.h"

#include "editdetails.h"

#include "home.h"


MTCashPurse::MTCashPurse(QWidget *parent) :
    QWidget(parent),
    m_pHeaderWidget(NULL),
    ui(new Ui::MTCashPurse)
{
    ui->setupUi(this);
    // ------------------------------------
    m_pHeaderWidget = new QWidget;
    ui->verticalLayoutPage->insertWidget(0, m_pHeaderWidget);
    // ------------------------------------
    ui->pushButtonDeposit ->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ui->pushButtonWithdraw->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->pushButtonDeposit ->setText(tr("Deposit Cash"));
    ui->pushButtonWithdraw->setText(tr("Withdraw Cash"));
    // ------------------------------------

    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);


    // ------------------------------------

    ui->tableWidget->setRowCount(10);
    ui->tableWidget->setColumnCount(3);

    // ------------------------------------

    int row = 1, column = 1;

    QTableWidgetItem *newItem = new QTableWidgetItem(tr("%1").arg(
          (row+1)*(column+1)));
      ui->tableWidget->setItem(row, column, newItem);
}


void MTCashPurse::refresh(QString strID, QString strName)
{
    if (NULL != ui)
    {
        QString qstrAmount = MTHome::shortAcctBalance(strID);

        QWidget * pHeaderWidget  = MTEditDetails::CreateDetailHeaderWidget(strID, strName, qstrAmount);

        if (NULL != m_pHeaderWidget)
        {
            ui->verticalLayoutPage->removeWidget(m_pHeaderWidget);
            delete m_pHeaderWidget;
            m_pHeaderWidget = NULL;
        }
        ui->verticalLayoutPage->insertWidget(0, pHeaderWidget);
        m_pHeaderWidget = pHeaderWidget;
        // ----------------------------------

        std::string str_acct_id     = strID.toStdString();
        std::string str_acct_nym    = OTAPI_Wrap::It()->GetAccountWallet_NymID(str_acct_id);
        std::string str_acct_server = OTAPI_Wrap::It()->GetAccountWallet_ServerID(str_acct_id);
        std::string str_acct_asset  = OTAPI_Wrap::It()->GetAccountWallet_AssetTypeID(str_acct_id);
        // -----------------------------------
        QString qstr_acct_nym    = QString::fromStdString(str_acct_nym);
        QString qstr_acct_server = QString::fromStdString(str_acct_server);
        QString qstr_acct_asset  = QString::fromStdString(str_acct_asset);
        // -----------------------------------
//      int64_t  raw_cash_balance = MTHome::rawCashBalance(qstr_acct_server, qstr_acct_asset, qstr_acct_nym);

        ui->labelCashBalance->setText(QString("<big><font color=grey>%1</font></big>").
                                      arg(MTHome::cashBalance(qstr_acct_server, qstr_acct_asset, qstr_acct_nym)));
        // -----------------------------------


    }
}

void MTCashPurse::on_pushButtonWithdraw_clicked()
{

}

void MTCashPurse::on_pushButtonDeposit_clicked()
{

}

void MTCashPurse::ClearContents()
{
    ui->tableWidget->clear();

    ui->labelCashBalance->setText("");
    // ----------------------------------
    if (NULL != m_pHeaderWidget)
    {
        ui->verticalLayoutPage->removeWidget(m_pHeaderWidget);
        delete m_pHeaderWidget;
        m_pHeaderWidget = NULL;
    }
    // ----------------------------------
    m_pHeaderWidget = new QWidget;
    ui->verticalLayoutPage->insertWidget(0, m_pHeaderWidget);
    // ----------------------------------

}


MTCashPurse::~MTCashPurse()
{
    delete ui;
}

