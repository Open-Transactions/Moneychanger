#ifndef DLGEXPORTCASH_H
#define DLGEXPORTCASH_H

#include <QDialog>

namespace Ui {
class DlgExportCash;
}

class DlgExportCash : public QDialog
{
    Q_OBJECT

public:
    explicit DlgExportCash(QWidget *parent = 0);
    ~DlgExportCash();

    QString GetHisNymID()          { return m_hisNymId;            }
    QString GetHisName()           { return m_hisName;             }
    bool    IsExportToPassphrase() { return m_bExportToPassphrase; }

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_radioButton_clicked();

    void on_radioButton_2_clicked();

private:
    QString m_hisNymId;
    QString m_hisName;
    bool    m_bExportToPassphrase;

    Ui::DlgExportCash *ui;
};

#endif // DLGEXPORTCASH_H
