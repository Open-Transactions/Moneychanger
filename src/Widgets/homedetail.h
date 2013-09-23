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

private:
    shared_ptr_MTRecord m_record;
    QGridLayout * m_pDetailLayout;

private:
    Ui::MTHomeDetail *ui;
};

#endif // MTHOMEDETAIL_H
