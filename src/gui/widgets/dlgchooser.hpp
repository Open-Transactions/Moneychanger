#ifndef DLGCHOOSER_HPP
#define DLGCHOOSER_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <core/handlers/contacthandler.hpp>

#include <QDialog>



namespace Ui {
class DlgChooser;
}

class DlgChooser : public QDialog
{
    Q_OBJECT
    
    bool m_bFirstRun;
    bool m_bIsAccounts; // If we're showing a list of accounts, we display the balances.

    QString m_qstrInstructions; // This is optional, and will be displayed above the list.

public:
    explicit DlgChooser(QWidget *parent, QString qstrInstructions=QString(""));
    ~DlgChooser();
    
    QString GetCurrentID() const { return m_qstrCurrentID; }

    void SetIsAccounts(bool bAcct=true) { m_bIsAccounts = bAcct; }
    void SetPreSelected(QString strSelected);

    int         m_nCurrentRow;
    QString     m_qstrCurrentID;
    QString     m_qstrCurrentName;
    mapIDName   m_map; // qstr/qstr for id/name

protected:

    QString     m_PreSelected;

    virtual void showEvent(QShowEvent * event);
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_pushButton_clicked();
    void on_cancelButton_clicked();

    void on_tableWidget_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void on_tableWidget_cellDoubleClicked(int row, int column);

private:
    Ui::DlgChooser *ui;
};

#endif // DLGCHOOSER_HPP
