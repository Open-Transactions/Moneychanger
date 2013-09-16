#include "MarketWindow.h"
#include "ui_marketwindow.h"

MarketWindow::MarketWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MarketWindow)
{
    ui->setupUi(this);
    this->installEventFilter(this);
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


bool MarketWindow::eventFilter(QObject *obj, QEvent *event){
    
    if (event->type() == QEvent::Close) {
        ((Moneychanger *)parentWidget())->close_market_dialog();
        return true;
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape){
            close(); // This is caught by this same filter.
            return true;
        }
        return true;
    }else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}