#ifndef DLGPASSWORD_HPP
#define DLGPASSWORD_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <QString>
#include <QDialog>
#include <QCloseEvent>


namespace Ui {
class MTDlgPassword;
}

class OTPassword;

class MTOverrideCursor;

class MTDlgPassword : public QDialog
{
    Q_OBJECT

public:
    explicit MTDlgPassword(QWidget *parent, OTPassword & thePassword);
    ~MTDlgPassword();

    void setDisplay(QString qstrDisplay);

protected:
    void closeEvent(QCloseEvent * e);

private slots:
    void on_MTDlgPassword_accepted();

private:
    OTPassword * m_pPassword;

    MTOverrideCursor * m_pCursor; // Do not delete. Here for reference only.

    Ui::MTDlgPassword *ui;
};

#endif // DLGPASSWORD_HPP
