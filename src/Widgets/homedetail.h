#ifndef MTHOMEDETAIL_H
#define MTHOMEDETAIL_H

#include <QWidget>

#include "MTRecord.hpp"
#include "MTRecordList.hpp"

class QGraphicsLayout;
class QLayout;
class QGridLayout;

namespace Ui {
class MTHomeDetail;
}

class MTHome;

enum TransactionTableViewCellType {
    TransactionTableViewCellTypeSent,
    TransactionTableViewCellTypeOutgoing,
    TransactionTableViewCellTypeReceived,
    TransactionTableViewCellTypeIncoming
};

class MTHomeDetail : public QWidget
{
    Q_OBJECT
    
public:
    explicit MTHomeDetail(QWidget *parent = 0);
    ~MTHomeDetail();
    
    void refresh(int nRow, MTRecordList & theList);

    static void clearLayout(QLayout* pLayout);

    static QWidget * CreateDetailHeaderWidget(MTRecord & recordmt, bool bExternal=true);

    void SetHomePointer(MTHome & theHome);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_contactButton_clicked(bool checked = false);
    void on_deleteButton_clicked(bool checked = false);
    void on_acceptButton_clicked(bool checked = false);
    void on_cancelButton_clicked(bool checked = false);
    void on_discardOutgoingButton_clicked(bool checked = false);
    void on_discardIncomingButton_clicked(bool checked = false);
    void on_msgButton_clicked(bool checked = false);

private:
    shared_ptr_MTRecord m_record;
    QGridLayout * m_pDetailLayout;
    MTHome * m_pHome;

private:
    Ui::MTHomeDetail *ui;
};

#endif // MTHOMEDETAIL_H
