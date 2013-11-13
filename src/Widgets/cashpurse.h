#ifndef CASHPURSE_H
#define CASHPURSE_H

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

private slots:
    void on_pushButtonWithdraw_clicked();

    void on_pushButtonDeposit_clicked();

    void checkboxClicked(int state);

    void on_pushButtonExport_clicked();

private:
    QString   m_qstrAcctId;
    QString   m_qstrAssetId;
    QString   m_qstrAcctName;

    MTDetailEdit * m_pOwner;
    QWidget      * m_pHeaderWidget;

    Ui::MTCashPurse *ui;
};

#endif // CASHPURSE_H
