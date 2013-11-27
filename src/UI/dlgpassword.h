#ifndef DLGPASSWORD_H
#define DLGPASSWORD_H

#include <QString>
#include <QDialog>


namespace Ui {
class MTDlgPassword;
}

class OTPassword;

class MTDlgPassword : public QDialog
{
    Q_OBJECT

public:
    explicit MTDlgPassword(QWidget *parent, OTPassword & thePassword);
    ~MTDlgPassword();

    void setDisplay(QString qstrDisplay);

    /**
     * Manually extract the input field's content into m_pPassword.  This is
     * necessary because in some situations the accepted-slot is not called
     * correctly for unknown reasons.
     */
    void extractPassword();

private slots:
    void on_MTDlgPassword_accepted();

private:
    OTPassword * m_pPassword;

    Ui::MTDlgPassword *ui;
};

#endif // DLGPASSWORD_H
