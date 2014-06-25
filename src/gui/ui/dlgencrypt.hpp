#ifndef DLGENCRYPT_H
#define DLGENCRYPT_H

#include <QDialog>

namespace Ui {
class DlgEncrypt;
}

class DlgEncrypt : public QDialog
{
    Q_OBJECT

public:
    explicit DlgEncrypt(QWidget *parent = 0);
    ~DlgEncrypt();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::DlgEncrypt *ui;
};

#endif // DLGENCRYPT_H
