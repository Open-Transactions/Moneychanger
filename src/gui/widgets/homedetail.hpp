#ifndef MTHOMEDETAIL_HPP
#define MTHOMEDETAIL_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <core/MTRecordList.hpp>

#include <QPointer>
#include <QWidget>
#include <QLineEdit>


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
    
    static QWidget * CreateDetailHeaderWidget(MTRecord & recordmt, bool bExternal=true);

    void SetHomePointer(MTHome & theHome);

signals:
    void balanceChanged();
    void accountDataDownloaded();
    void setRefreshBtnRed();
    void refreshUserBar();
    void recordDeleted(bool bNeedToRefreshUserBar);
    void recordDeletedBalanceChanged(bool bNeedToRefreshUserBar);

    void showContact(QString qstrContactID);
    void showContactAndRefreshHome(QString qstrContactID);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

public slots:
    void onRefresh(int nRow, MTRecordList & theList);

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
    void refresh(MTRecord & recordmt);
    void refresh(int nRow, MTRecordList & theList);

    int m_nContactID; // If there's a known Contact ID for this record, it will be set here when discovered, for later use.
    shared_ptr_MTRecord m_record;

    QPointer<QGridLayout> m_pDetailLayout;
    QPointer<MTHome> m_pHome;

    QPointer<QLineEdit> m_pLineEdit_Nym_ID;
    QPointer<QLineEdit> m_pLineEdit_OtherNym_ID;
    QPointer<QLineEdit> m_pLineEdit_Acct_ID;
    QPointer<QLineEdit> m_pLineEdit_OtherAcct_ID;
    QPointer<QLineEdit> m_pLineEdit_Server_ID;
    QPointer<QLineEdit> m_pLineEdit_AssetType_ID;

    QPointer<QLineEdit> m_pLineEdit_Nym_Name;
    QPointer<QLineEdit> m_pLineEdit_OtherNym_Name;
    QPointer<QLineEdit> m_pLineEdit_Acct_Name;
    QPointer<QLineEdit> m_pLineEdit_OtherAcct_Name;
    QPointer<QLineEdit> m_pLineEdit_Server_Name;
    QPointer<QLineEdit> m_pLineEdit_AssetType_Name;

    void FavorLeftSideForIDs();

    QString FindAppropriateDepositAccount(MTRecord & recordmt);
    void    RecreateLayout();

private:
    Ui::MTHomeDetail *ui;
};

#endif // MTHOMEDETAIL_HPP
