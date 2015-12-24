#ifndef DLGEXPORTEDCASH_HPP
#define DLGEXPORTEDCASH_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <QDialog>

namespace Ui {
class DlgExportedCash;
}

class DlgExportedCash : public QDialog
{
    Q_OBJECT

public:
    explicit DlgExportedCash(QWidget *parent, QString qstrHisCopy, QString qstrMyCopy,
                             QString   qstrLabelHeader1=QString(""),
                             QString   qstrLabelHeader2=QString(""),
                             QString   qstrLabelHeader3=QString(""),
                             QString   qstrLabelHeader4=QString(""),
                             bool      bShowWarning=true);
    ~DlgExportedCash();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_pushButton_clicked();
    void on_pushButtonCopy_clicked();
    void on_buttonBox_accepted();

private:
    bool m_bShowWarning=true;
    Ui::DlgExportedCash *ui;
};

#endif // DLGEXPORTEDCASH_HPP
