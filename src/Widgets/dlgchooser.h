#ifndef DLGCHOOSER_H
#define DLGCHOOSER_H

#include <QDialog>

#include "Handlers/contacthandler.h"

namespace Ui {
class DlgChooser;
}

class DlgChooser : public QDialog
{
    Q_OBJECT
    
    bool m_bFirstRun;

public:
    explicit DlgChooser(QWidget *parent = 0);
    ~DlgChooser();
    
    int         m_nCurrentRow;
    QString     m_qstrCurrentID;
    QString     m_qstrCurrentName;
    mapIDName   m_map; // qstr/qstr for id/name

protected:
    virtual void showEvent(QShowEvent * event);
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_tableWidget_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void on_pushButton_clicked();
    void on_tableWidget_cellDoubleClicked(int row, int column);

private:
    Ui::DlgChooser *ui;
};

#endif // DLGCHOOSER_H
