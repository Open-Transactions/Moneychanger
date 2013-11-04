#ifndef DLGPASSWORDCONFIRM_H
#define DLGPASSWORDCONFIRM_H

#include <QString>
#include <QDialog>

namespace Ui {
class MTDlgPasswordConfirm;
}

class OTPassword;

class MTDlgPasswordConfirm : public QDialog
{
    Q_OBJECT

public:
    explicit MTDlgPasswordConfirm(QWidget *parent, OTPassword & thePassword);
    ~MTDlgPasswordConfirm();

    void setDisplay(QString qstrDisplay);

private slots:
    void on_MTDlgPasswordConfirm_accepted();

private:
    OTPassword * m_pPassword;

    Ui::MTDlgPasswordConfirm *ui;
};

#endif // DLGPASSWORDCONFIRM_H
