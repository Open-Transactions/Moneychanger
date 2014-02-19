#ifndef GETSTRINGDIALOG_H
#define GETSTRINGDIALOG_H

#include <QDialog>

namespace Ui {
class MTGetStringDialog;
}

class MTGetStringDialog : public QDialog
{
    Q_OBJECT

    QString m_Output;

public:
    explicit MTGetStringDialog(QWidget *parent = 0);
    ~MTGetStringDialog();

    QString GetOutputString() { return m_Output; }

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::MTGetStringDialog *ui;
};

#endif // GETSTRINGDIALOG_H
