#ifndef DLGDECRYPT_HPP
#define DLGDECRYPT_HPP

#include <QDialog>

namespace Ui {
class DlgDecrypt;
}

class DlgDecrypt : public QDialog
{
    Q_OBJECT

public:
    explicit DlgDecrypt(QWidget *parent = 0);
    ~DlgDecrypt();

    void dialog();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_pushButtonClipboard_clicked();

    void on_pushButtonDecrypt_clicked();

private:
    Ui::DlgDecrypt *ui;
    bool already_init;
};

#endif // DLGDECRYPT_HPP
