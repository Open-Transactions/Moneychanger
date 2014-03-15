#ifndef CASHPURSE_HPP
#define CASHPURSE_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/TR1_Wrapper.hpp>

#include _CINTTYPES
#include _MEMORY

#include <QPointer>
#include <QTableWidgetItem>
#include <QWidget>
#include <QString>


namespace Ui {
class MTCashPurse;
}

class QStringList;
class MTDetailEdit;

class MTCashPurse : public QWidget
{
    Q_OBJECT

public:
    explicit MTCashPurse(QWidget *parent, MTDetailEdit & theOwner);
    ~MTCashPurse();

    void ClearContents();

    void refresh(QString strID, QString strName);

    int TallySelections(QStringList & selectedIndices, int64_t & lAmount);

signals:
    void balancesChanged(QString qstrAcct);

private slots:
    void on_pushButtonWithdraw_clicked();

    void on_pushButtonDeposit_clicked();

    void checkboxClicked(int state);

    void on_pushButtonExport_clicked();

private:
    QString   m_qstrAcctId;
    QString   m_qstrAssetId;
    QString   m_qstrAcctName;

    QPointer<MTDetailEdit> m_pOwner;
    QPointer<QWidget>      m_pHeaderWidget;

    Ui::MTCashPurse *ui;
};

#endif // CASHPURSE_HPP
