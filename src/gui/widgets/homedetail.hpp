#ifndef MTHOMEDETAIL_HPP
#define MTHOMEDETAIL_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <opentxs/client/OTRecordList.hpp>

#include <QPointer>
#include <QWidget>
#include <QLineEdit>
#include <QToolButton>


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
    
    static QWidget * CreateDetailHeaderWidget(opentxs::OTRecord& recordmt, bool bExternal=true);

    void SetHomePointer(MTHome & theHome);

signals:
    void balanceChanged();
    void accountDataDownloaded();
    void setRefreshBtnRed();
    void refreshUserBar();
    void recordDeleted();
    void recordDeletedBalanceChanged();

    void showContact(QString qstrContactID);
    void showContactAndRefreshHome(QString qstrContactID);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

public slots:
    void onRefresh(int nRow, opentxs::OTRecordList & theList);

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

    void copyNymClicked();
    void copyOtherNymClicked();
    void copyAddressClicked();
    void copyOtherAddressClicked();
    void copyAcctClicked();
    void copyOtherAcctClicked();
    void copyServerClicked();
    void copyAssetTypeClicked();

private:
    void refresh(opentxs::OTRecord& recordmt);
    void refresh(int nRow, opentxs::OTRecordList & theList);

    void copyIDToClipboard(const QString qstr_field, const QString & text);

    int m_nContactID; // If there's a known Contact ID for this record, it will be set here when discovered, for later use.
    opentxs::shared_ptr_OTRecord m_record;

    QPointer<QGridLayout> m_pDetailLayout;
    QPointer<MTHome> m_pHome;

    QPointer<QLineEdit> m_pLineEdit_Nym_ID;
    QPointer<QLineEdit> m_pLineEdit_OtherNym_ID;
    QPointer<QLineEdit> m_pLineEdit_Address;
    QPointer<QLineEdit> m_pLineEdit_OtherAddress;
    QPointer<QLineEdit> m_pLineEdit_Acct_ID;
    QPointer<QLineEdit> m_pLineEdit_OtherAcct_ID;
    QPointer<QLineEdit> m_pLineEdit_notary_id;
    QPointer<QLineEdit> m_pLineEdit_AssetType_ID;

    QPointer<QLineEdit> m_pLineEdit_Nym_Name;
    QPointer<QLineEdit> m_pLineEdit_OtherNym_Name;
    QPointer<QLineEdit> m_pLineEdit_Address_Name;
    QPointer<QLineEdit> m_pLineEdit_OtherAddress_Name;
    QPointer<QLineEdit> m_pLineEdit_Acct_Name;
    QPointer<QLineEdit> m_pLineEdit_OtherAcct_Name;
    QPointer<QLineEdit> m_pLineEdit_Server_Name;
    QPointer<QLineEdit> m_pLineEdit_AssetType_Name;

    QPointer<QToolButton> m_pToolbutton_Nym_Name;
    QPointer<QToolButton> m_pToolbutton_OtherNym_Name;
    QPointer<QToolButton> m_pToolbutton_Address_Name;
    QPointer<QToolButton> m_pToolbutton_OtherAddress_Name;
    QPointer<QToolButton> m_pToolbutton_Acct_Name;
    QPointer<QToolButton> m_pToolbutton_OtherAcct_Name;
    QPointer<QToolButton> m_pToolbutton_Server_Name;
    QPointer<QToolButton> m_pToolbutton_AssetType_Name;

    void FavorLeftSideForIDs();

    QString FindAppropriateDepositAccount(opentxs::OTRecord& recordmt);
    void    RecreateLayout();

private:
    Ui::MTHomeDetail *ui;
};

#endif // MTHOMEDETAIL_HPP
