#ifndef DLGPASSWORDCONFIRM_H
#define DLGPASSWORDCONFIRM_H

#include <QString>
#include <QDialog>
#include <QCloseEvent>

namespace Ui {
class MTDlgPasswordConfirm;
}

class OTPassword;

class MTOverrideCursor;

class MTDlgPasswordConfirm : public QDialog
{
    Q_OBJECT

public:
    explicit MTDlgPasswordConfirm(QWidget *parent, OTPassword & thePassword);
    ~MTDlgPasswordConfirm();

    void setDisplay(QString qstrDisplay);

protected:
    void closeEvent(QCloseEvent * e);

private slots:
    void on_MTDlgPasswordConfirm_accepted();

private:
    OTPassword * m_pPassword;

    MTOverrideCursor * m_pCursor; // Do not delete. Here for reference only.

    Ui::MTDlgPasswordConfirm *ui;
};

#endif // DLGPASSWORDCONFIRM_H
