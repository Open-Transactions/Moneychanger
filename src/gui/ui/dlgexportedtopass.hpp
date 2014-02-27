#ifndef DLGEXPORTEDTOPASS_HPP
#define DLGEXPORTEDTOPASS_HPP

#include <WinsockWrapper.h>
#include <ExportWrapper.h>

#include <QDialog>

namespace Ui {
class DlgExportedToPass;
}

class DlgExportedToPass : public QDialog
{
    Q_OBJECT

public:
    explicit DlgExportedToPass(QWidget *parent, QString qstrTheCash);
    ~DlgExportedToPass();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_buttonBox_accepted();

    void on_pushButton_clicked();

private:
    Ui::DlgExportedToPass *ui;
};

#endif // DLGEXPORTEDTOPASS_HPP
