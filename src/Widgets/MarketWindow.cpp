#include "marketwindow.h"
#include "ui_marketwindow.h"

MarketWindow::MarketWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MarketWindow)
{
    ui->setupUi(this);
}

MarketWindow::~MarketWindow()
{
    delete ui;
}

void MarketWindow::on_pushButton_clicked()
{
    ui->textEdit->setText("Test Success!");
}

void MarketWindow::on_pushButton_2_clicked()
{
    close();
}
