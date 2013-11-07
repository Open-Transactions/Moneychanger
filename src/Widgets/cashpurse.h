#ifndef CASHPURSE_H
#define CASHPURSE_H

#include <QTableWidgetItem>
#include <QWidget>
#include <QList>
#include <QString>

namespace Ui {
class MTCashPurse;
}


class MTCashPurse : public QWidget
{
    Q_OBJECT

public:
    explicit MTCashPurse(QWidget *parent = 0);
    ~MTCashPurse();

    void ClearContents();

    void refresh(QString strID, QString strName);

    int TallySelections(QList<QString> & selectedIDs, int64_t & lAmount);

private slots:
    void on_pushButtonWithdraw_clicked();

    void on_pushButtonDeposit_clicked();

    void checkboxClicked(int state);

private:
    QString   m_qstrAcctId;
    QString   m_qstrAssetId;
    QWidget * m_pHeaderWidget;

    Ui::MTCashPurse *ui;
};

#endif // CASHPURSE_H
