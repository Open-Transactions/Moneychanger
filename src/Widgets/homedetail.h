#ifndef MTHOMEDETAIL_H
#define MTHOMEDETAIL_H

#include <QWidget>
#include <QLineEdit>

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
    
    void refresh(MTRecord & recordmt);
    void refresh(int nRow, MTRecordList & theList);

    static void      clearLayout(QLayout* pLayout);
    static QWidget * CreateDetailHeaderWidget(MTRecord & recordmt, bool bExternal=true);

    void SetHomePointer(MTHome & theHome);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_viewContactButton_clicked(bool checked = false);
    void on_addContactButton_clicked(bool checked = false);
    void on_existingContactButton_clicked(bool checked = false);

    void on_deleteButton_clicked(bool checked = false);
    void on_acceptButton_clicked(bool checked = false);
    void on_cancelButton_clicked(bool checked = false);
    void on_discardOutgoingButton_clicked(bool checked = false);
    void on_discardIncomingButton_clicked(bool checked = false);
    void on_msgButton_clicked(bool checked = false);

private:
    int m_nContactID; // If there's a known Contact ID for this record, it will be set here when discovered, for later use.
    shared_ptr_MTRecord m_record;
    QGridLayout * m_pDetailLayout;
    MTHome * m_pHome;

    QLineEdit * m_pLineEdit_Nym_ID;
    QLineEdit * m_pLineEdit_OtherNym_ID;
    QLineEdit * m_pLineEdit_Acct_ID;
    QLineEdit * m_pLineEdit_OtherAcct_ID;
    QLineEdit * m_pLineEdit_Server_ID;
    QLineEdit * m_pLineEdit_AssetType_ID;

    QLineEdit * m_pLineEdit_Nym_Name;
    QLineEdit * m_pLineEdit_OtherNym_Name;
    QLineEdit * m_pLineEdit_Acct_Name;
    QLineEdit * m_pLineEdit_OtherAcct_Name;
    QLineEdit * m_pLineEdit_Server_Name;
    QLineEdit * m_pLineEdit_AssetType_Name;

    void FavorLeftSideForIDs();

    QString FindAppropriateDepositAccount(MTRecord & recordmt);
    void    RecreateLayout();

private:
    Ui::MTHomeDetail *ui;
};

#endif // MTHOMEDETAIL_H
