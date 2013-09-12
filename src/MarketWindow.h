#ifndef MARKETWINDOW_H
#define MARKETWINDOW_H

#include <QDialog>
#include <QAbstractButton>


namespace Ui {
class MarketWindow;
}

class MarketWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MarketWindow(QWidget *parent = 0);
    ~MarketWindow();

private slots:

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MarketWindow *ui;
};

#endif // MARKETWINDOW_H
