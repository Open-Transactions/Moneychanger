#ifndef CASHPURSE_H
#define CASHPURSE_H

#include <QWidget>

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

private slots:
    void on_pushButtonWithdraw_clicked();

    void on_pushButtonDeposit_clicked();

private:
    QWidget * m_pHeaderWidget;

    Ui::MTCashPurse *ui;
};

#endif // CASHPURSE_H
