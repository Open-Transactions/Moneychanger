#ifndef DLGGETAMOUNT_H
#define DLGGETAMOUNT_H

#include <QDialog>

namespace Ui {
class DlgGetAmount;
}

class DlgGetAmount : public QDialog
{
    Q_OBJECT

public:
    explicit DlgGetAmount(QWidget *parent = 0);
    ~DlgGetAmount();

private slots:
    void on_lineEdit_editingFinished();

private:
    Ui::DlgGetAmount *ui;
};

#endif // DLGGETAMOUNT_H
