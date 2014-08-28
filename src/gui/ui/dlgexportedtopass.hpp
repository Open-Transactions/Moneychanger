#ifndef DLGEXPORTEDTOPASS_HPP
#define DLGEXPORTEDTOPASS_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <QDialog>

namespace Ui {
class DlgExportedToPass;
}

class DlgExportedToPass : public QDialog
{
    Q_OBJECT

public:
    explicit DlgExportedToPass(QWidget * parent,
                               QString   qstrTheCash,
                               QString   qstrLabelHeader1=QString(""),
                               QString   qstrLabelHeader2=QString(""),
                               bool      bShowWarning=true);
    ~DlgExportedToPass();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_buttonBox_accepted();

    void on_pushButton_clicked();

private:
    bool m_bShowWarning;
//  QString m_qstrLabelHeader1;
//  QString m_qstrLabelHeader2;
    Ui::DlgExportedToPass *ui;
};

#endif // DLGEXPORTEDTOPASS_HPP
