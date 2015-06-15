#ifndef DLGLOG_H
#define DLGLOG_H

#include <QDialog>

namespace Ui {
class DlgLog;
}

class DlgLog : public QDialog
{
    Q_OBJECT

public:
    explicit DlgLog(QWidget *parent = 0);
    ~DlgLog();

    void FirstRun();

    void dialog();

    void appendToLog(QString qstrAppend);

private slots:
    void on_pushButtonClear_clicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    bool m_bFirstRun;

    Ui::DlgLog *ui;
};

#endif // DLGLOG_H
