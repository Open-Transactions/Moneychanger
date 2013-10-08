#ifndef COMPOSE_H
#define COMPOSE_H

#include <QWidget>

namespace Ui {
class MTCompose;
}

class MTCompose : public QWidget
{
    Q_OBJECT
    
public:
    explicit MTCompose(QWidget *parent = 0);
    ~MTCompose();
    
    void dialog();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void closeEvent(QCloseEvent *event);

private slots:
    void on_toButton_clicked();

    void on_fromButton_clicked();

    void on_serverButton_clicked();

    void on_sendButton_clicked();

    void on_subjectEdit_textChanged(const QString &arg1);

private:
    bool already_init;

    Ui::MTCompose *ui;
};

#endif // COMPOSE_H
