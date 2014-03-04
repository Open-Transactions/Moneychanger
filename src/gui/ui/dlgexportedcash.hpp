#ifndef DLGEXPORTEDCASH_HPP
#define DLGEXPORTEDCASH_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <QDialog>

namespace Ui {
class DlgExportedCash;
}

class DlgExportedCash : public QDialog
{
    Q_OBJECT

public:
    explicit DlgExportedCash(QWidget *parent, QString qstrHisCopy, QString qstrMyCopy);
    ~DlgExportedCash();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_pushButton_clicked();

    void on_pushButtonCopy_clicked();

    void on_buttonBox_accepted();

private:
    Ui::DlgExportedCash *ui;
};

#endif // DLGEXPORTEDCASH_HPP
