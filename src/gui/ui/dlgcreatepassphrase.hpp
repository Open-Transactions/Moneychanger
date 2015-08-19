#ifndef DLGCREATEPASSPHRASE_HPP
#define DLGCREATEPASSPHRASE_HPP

#include <QDialog>

namespace Ui {
class DlgCreatePassphrase;
}

class DlgCreatePassphrase : public QDialog
{
    Q_OBJECT

public:
    explicit DlgCreatePassphrase(QWidget *parent = 0);
    ~DlgCreatePassphrase();

private:
    Ui::DlgCreatePassphrase *ui;
};

#endif // DLGCREATEPASSPHRASE_HPP
