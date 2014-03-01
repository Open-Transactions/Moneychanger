#ifndef DLGIMPORT_HPP
#define DLGIMPORT_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <QDialog>

namespace Ui {
class DlgImport;
}

class DlgImport : public QDialog
{
    Q_OBJECT

public:
    explicit DlgImport(QWidget *parent = 0);
    ~DlgImport();
    // ---------------------------------------------
    bool    IsPasted   () { return  m_bPasted;     }
    bool    IsFilename () { return !m_bPasted;     }
    // ---------------------------------------------
    QString GetFilename() { return m_qstrFilename; }
    QString GetPasted  () { return m_qstrPaste;    }
    // ---------------------------------------------
protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_radioButton_clicked();

    void on_radioButton_2_clicked();

    void on_toolButton_clicked();

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    bool    m_bPasted;
    QString m_qstrFilename;
    QString m_qstrPaste;

    Ui::DlgImport *ui;
};

#endif // DLGIMPORT_HPP
