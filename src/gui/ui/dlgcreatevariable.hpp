#ifndef DLGCREATEVARIABLE_H
#define DLGCREATEVARIABLE_H

#include <QDialog>

namespace Ui {
class DlgCreateVariable;
}

class DlgCreateVariable : public QDialog
{
    Q_OBJECT

public:
    explicit DlgCreateVariable(QWidget *parent = 0);
    ~DlgCreateVariable();

    bool m_bFirstRun;

    QString m_qstrName;
    QString m_qstrAccess;
    QString m_qstrType;
    QString m_qstrValue;

    int m_nAccessIndex;
    int m_nTypeIndex;

    bool m_bValue;

protected:
    virtual void showEvent(QShowEvent * event);
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_buttonBox_accepted();

    void on_comboBoxVariableType_currentIndexChanged(int index);

private:
    Ui::DlgCreateVariable *ui;
};

#endif // DLGCREATEVARIABLE_H
