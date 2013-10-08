#ifndef DETAILEDIT_H
#define DETAILEDIT_H

#include <QWidget>
#include <QVBoxLayout>

#include "Handlers/contacthandler.h"

namespace Ui {
class MTDetailEdit;
}

class MTEditDetails;


class MTDetailEdit : public QWidget
{
    Q_OBJECT

    bool m_bFirstRun;

public:
    enum DetailEditType {
        DetailEditTypeError,
        DetailEditTypeContact,
        DetailEditTypeNym,
        DetailEditTypeServer,
        DetailEditTypeAsset,
        DetailEditTypeAccount
    };

    explicit MTDetailEdit(QWidget *parent = 0);
    ~MTDetailEdit();

    void SetPreSelected(QString strSelected);

    void dialog(DetailEditType theType);

    int         m_nCurrentRow;
    QString     m_qstrCurrentID;
    QString     m_qstrCurrentName;
    mapIDName   m_map; // qstr/qstr for id/name

protected:
    QString     m_PreSelected;

    MTEditDetails * m_pDetailPane;
    QVBoxLayout   * m_pDetailLayout;

//    virtual void showEvent(QShowEvent * event);
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_tableWidget_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

private:
    void RefreshRecords();

    DetailEditType m_Type;

    Ui::MTDetailEdit *ui;
};

#endif // DETAILEDIT_H
