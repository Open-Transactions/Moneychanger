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

private slots:
    void on_MTDlgPassword_accepted();

private:
    OTPassword * m_pPassword;

    Ui::MTDlgPassword *ui;
};

#endif // DLGPASSWORD_H
