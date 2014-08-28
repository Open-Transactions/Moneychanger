#ifndef GETSTRINGDIALOG_HPP
#define GETSTRINGDIALOG_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <QDialog>

namespace Ui {
class MTGetStringDialog;
}

class MTGetStringDialog : public QDialog
{
    Q_OBJECT

    QString m_Output;

public:
    explicit MTGetStringDialog(QWidget *parent = 0, QString qstrLabel=QString(""), QString qstrPlaceholder=QString(""));
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

#endif // GETSTRINGDIALOG_HPP
