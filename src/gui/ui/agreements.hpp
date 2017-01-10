#ifndef AGREEMENTS_HPP
#define AGREEMENTS_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <core/handlers/modelagreements.hpp>

#include <QWidget>
#include <QMenu>
#include <QScopedPointer>
#include <QPointer>
#include <QList>
#include <QSqlRecord>
#include <QTimer>
#include <QDebug>

#include <tuple>
#include <map>

namespace Ui {
class Agreements;
}


class AgreementsProxyModel;
class AgreementReceiptsProxyModel;
//class ModelAgreements;
//class ModelAgreementReceipts;

class QTableView;

//source model row, agreement_id, txnIdDisplay, notaryId, newestState
typedef std::tuple<int, int, int64_t, std::string, int> LiveAgreement;

enum class agreement_status { ERROR = 0,
                              OUTGOING = 1,
                              INCOMING = 2,
                              ACTIVATED = 3,
                              PAID = 4,
                              PAYMENT_FAILED = 5,    // LIVE agreement here and above this line.
                              FAILED_ACTIVATING = 6, // DEAD agreement here and below this line.
                              CANCELED = 7,
                              EXPIRED = 8,
                              NO_LONGER_ACTIVE = 9,
                              KILLED = 10
                            };

class Agreements : public QWidget
{
    Q_OBJECT

private:
    bool already_init = false;

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void RefreshAll();

    void enableButtons();
    void disableButtons();

    void tableViewAgreementsPopupMenu(const QPoint &pos, QTableView * pTableView, AgreementsProxyModel * pProxyModel);
    void tableViewAgreementsDoubleClicked(const QModelIndex &index, AgreementsProxyModel * pProxyModel);

    void tableViewReceiptsPopupMenu(const QPoint &pos, QTableView * pTableView, AgreementReceiptsProxyModel * pProxyModel);
    void tableViewReceiptsDoubleClicked(const QModelIndex &index, AgreementReceiptsProxyModel * pProxyModel);

    bool setupCurrentPointers();
    void setAsCurrentAgreement(int nSourceRow, int nFolder); // The smart contract itself, or payment plan, that's selected.
    void setAsCurrentParty(int nSourceRow); // You may have multiple signer Nyms on the same agreement, in the same wallet.
    void setAsCurrentPayment(int nSourceRow, int nFolder); // the recurring paymentReceipts for each smart contract or payment plan.

    template <typename T>
    void timer_MarkAsRead(T & pModel, QList<QModelIndex> & listRecordsToMarkAsRead);
    template <typename T>
    void timer_MarkAsUnread(T & pModel, QList<QModelIndex> & listRecordsToMarkAsUnread);

    void killSelectedAgreement();

    void updateFilters(int nAgreementId);

    // --------------------------------------------------------------------
    bool rowAndIdForCurrentAgreement(int & nRow, int & nAgreementId);

    bool agreementIdAndRow_Recurring(int & nRow, int & nAgreementId, // output params
                                     const QModelIndex * pProxyIndex=nullptr); // defaults to current.
    bool agreementIdAndRow_SmartContract(int & nRow, int & nAgreementId, // output params
                                         const QModelIndex * pProxyIndex=nullptr); // defaults to current.
    bool agreementIdAndRow(int & nRow, int & nAgreementId, // output params
                           QTableView & agreementTableView,
                           AgreementsProxyModel & agreementProxyModel,
                           const QModelIndex * pProxyIndex=nullptr); // defaults to current.
    // --------------------------------------------------------------------
    bool agreementIdAndReceiptKeyForCurrentReceipt(int & nRow, int & nAgreementId, int & nReceiptKey); // output params

    bool agreementIdAndReceiptKeyForInbox (int & nRow, int & nAgreementId, int & nReceiptKey,// output params
                                           const QModelIndex * pModelIndex=nullptr); // defaults to current index.
    bool agreementIdAndReceiptKeyForOutbox(int & nRow, int & nAgreementId, int & nReceiptKey,// output params
                                           const QModelIndex * pModelIndex=nullptr); // defaults to current index.
    bool agreementIdAndReceiptKey(int & nRow, int & nAgreementId, int & nReceiptKey, // output params on top
                                  std::map<int, int> & mapLastSelectedReceiptKey, // will be the right one for the inbox or outbox.
                                  AgreementReceiptsProxyModel & receiptProxyModel, // will be the right one for the above table view.
                                  const QModelIndex & proxyIndex); // will be a valid index for the above tableview/proxymodel
    // --------------------------------------------------------------------
public:
    explicit Agreements(QWidget *parent = 0);
    ~Agreements();

    void dialog(int nSourceRow=-1, int nFolder=-1);

    void RefreshUserBar();
    QWidget * CreateUserBarWidget();

    void AcceptIncomingReceipt (QPointer<ModelAgreementReceipts> & pModel, AgreementReceiptsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView);
    void CancelOutgoingReceipt (QPointer<ModelAgreementReceipts> & pModel, AgreementReceiptsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView);
    void DiscardIncomingReceipt(QPointer<ModelAgreementReceipts> & pModel, AgreementReceiptsProxyModel * pProxyModel, const int nSourceRow, QTableView * pTableView);

public slots:
    void onRecordlistPopulated();
    void onBalancesChanged();
    void onNeedToRefreshUserBar();
    void onNeedToRefreshRecords();
    void onClaimsUpdatedForNym(QString nymId);

signals:
    void showDashboard();
    void needToDownloadAccountData();
    void needToPopulateRecordlist();
    void showContact(QString);
    void showContactAndRefreshHome(QString);
    void needToCheckNym(QString, QString, QString);
    void needToRefreshAgreements();
    void needToRefreshReceipts();

private slots:
    void on_tableViewRecurringSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous);
    void on_tableViewSmartContractsSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous);
    // ----------------------------------------
    void on_tableViewSentSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous);
    void on_tableViewReceivedSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous);
    // ----------------------------------------
    void on_tableViewPartiesSelectionModel_currentRowChanged(const QModelIndex & current, const QModelIndex & previous);
    // ----------------------------------------
    void on_toolButtonDelete_clicked();
    void on_toolButtonRefresh_clicked();
    // ----------------------------------------
    void on_tabWidgetAgreements_currentChanged(int index); // recurring, smart contracts, entities.
    void on_tabWidgetReceipts_currentChanged(int index); // Received, Sent.

    void on_MarkAgreementsAsRead_timer();
    void on_MarkAgreementsAsUnread_timer();
    void on_MarkReceiptsAsRead_timer();
    void on_MarkReceiptsAsUnread_timer();

    void RefreshAgreements();
//    void RefreshParties();
    void RefreshReceipts();

    void on_tableViewSent_doubleClicked(const QModelIndex &index);
    void on_tableViewSent_customContextMenuRequested(const QPoint &pos);

    void on_tableViewReceived_doubleClicked(const QModelIndex &index);
    void on_tableViewReceived_customContextMenuRequested(const QPoint &pos);

    void on_tableViewRecurring_doubleClicked(const QModelIndex &index);
    void on_tableViewRecurring_customContextMenuRequested(const QPoint &pos);

    void on_tableViewSmartContracts_doubleClicked(const QModelIndex &index);
    void on_tableViewSmartContracts_customContextMenuRequested(const QPoint &pos);

private:
    Ui::Agreements *ui;

    QPointer<QWidget>     m_pHeaderFrame;
    QScopedPointer<QMenu> popupMenu_;

    QAction * pActionReply               = nullptr;
    QAction * pActionForward             = nullptr;
    QAction * pActionAcceptIncoming      = nullptr;
    QAction * pActionCancelOutgoing      = nullptr;
    QAction * pActionDiscardIncoming     = nullptr;
    QAction * pActionDelete              = nullptr;
    QAction * pActionKill                = nullptr;
    QAction * pActionOpenNewWindow       = nullptr;
    QAction * pActionMarkRead            = nullptr;
    QAction * pActionMarkUnread          = nullptr;
    QAction * pActionViewContact         = nullptr;
    QAction * pActionCreateContact       = nullptr;
    QAction * pActionExistingContact     = nullptr;
    QAction * pActionDownloadCredentials = nullptr;

    QPointer<AgreementsProxyModel> pProxyModelRecurring_;
    QPointer<AgreementsProxyModel> pProxyModelSmartContracts_;

    QPointer<AgreementReceiptsProxyModel> pReceiptProxyModelInbox_;
    QPointer<AgreementReceiptsProxyModel> pReceiptProxyModelOutbox_;

    QTableView         * pCurrentAgreementTableView_ = nullptr; // Recurring payments or Smart contracts.
    QTableView         * pCurrentPartyTableView_     = nullptr; // Might remove this. (I don't see how it would ever change.)
    QTableView         * pCurrentReceiptTableView_   = nullptr; // Sent or Received.

    AgreementsProxyModel        * pCurrentAgreementProxyModel_ = nullptr;
    AgreementReceiptsProxyModel * pCurrentReceiptProxyModel_   = nullptr;

    QList<QModelIndex> listAgreementRecordsToMarkAsRead_;
    QList<QModelIndex> listAgreementRecordsToMarkAsUnread_;

    QList<QModelIndex> listReceiptRecordsToMarkAsRead_;
    QList<QModelIndex> listReceiptRecordsToMarkAsUnread_;

    bool bRefreshingAfterUpdatedClaims_=false;

    int nLastSelectedRecurringIndex_{0};
    int nLastSelectedContractIndex_{0};
    int nLastSelectedRecurringAgreementId_{0};
    int nLastSelectedContractAgreementId_{0};
    std::map<int, int> mapLastSelectedInboxReceiptKey_; // int agreement_id, int agreement_receipt_key
    std::map<int, int> mapLastSelectedOutboxReceiptKey_; // int agreement_id, int agreement_receipt_key
    std::map<int, int> * pMapLastSelectedReceiptKey_ = nullptr; // int agreement_id, int agreement_receipt_key
};


template <typename T>
void Agreements::timer_MarkAsRead(T & pModel, QList<QModelIndex> & listRecordsToMarkAsRead)
{
    bool bEditing = false;

    while (!listRecordsToMarkAsRead.isEmpty())
    {
        QModelIndex index = listRecordsToMarkAsRead.front();
        listRecordsToMarkAsRead.pop_front();
        // ------------------------------------
        if (!index.isValid())
            continue;
        // ------------------------------------
        if (!bEditing)
        {
            bEditing = true;
            pModel->database().transaction();
        }
        // ------------------------------------
        pModel->setData(index, QVariant(1)); // 1 for "true" in sqlite. "Yes, we've now read this receipt. Mark it as read."
    } // while
    // ------------------------------
    if (bEditing)
    {
        if (pModel->submitAll())
        {
            pModel->database().commit();
            // ------------------------------------
            if (&listRecordsToMarkAsRead == &listAgreementRecordsToMarkAsRead_)
                QTimer::singleShot(0, this, SLOT(RefreshAgreements()));
            else if (&listRecordsToMarkAsRead == &listReceiptRecordsToMarkAsRead_)
                QTimer::singleShot(0, this, SLOT(RefreshReceipts()));
        }
        else
        {
            pModel->database().rollback();
            qDebug() << "Database Write Error" <<
                       "The database reported an error: " <<
                       pModel->lastError().text();
        }
    }
}

template <typename T>
void Agreements::timer_MarkAsUnread(T & pModel, QList<QModelIndex> & listRecordsToMarkAsUnread)
{
    bool bEditing = false;

    while (!listRecordsToMarkAsUnread.isEmpty())
    {
        QModelIndex index = listRecordsToMarkAsUnread.front();
        listRecordsToMarkAsUnread.pop_front();
        // ------------------------------------
        if (!index.isValid())
            continue;
        // ------------------------------------
        if (!bEditing)
        {
            bEditing = true;
            pModel->database().transaction();
        }
        // ------------------------------------
        pModel->setData(index, QVariant(0)); // 0 for "false" in sqlite. "This receipt is now marked UNREAD."
    } // while
    // ------------------------------
    if (bEditing)
    {
        if (pModel->submitAll())
        {
            pModel->database().commit();
            // ------------------------------------
            if (&listRecordsToMarkAsUnread == &listAgreementRecordsToMarkAsUnread_)
                QTimer::singleShot(0, this, SLOT(RefreshAgreements()));
            else if (&listRecordsToMarkAsUnread == &listReceiptRecordsToMarkAsUnread_)
                QTimer::singleShot(0, this, SLOT(RefreshReceipts()));
        }
        else
        {
            pModel->database().rollback();
            qDebug() << "Database Write Error" <<
                       "The database reported an error: " <<
                       pModel->lastError().text();
        }
    }
}

#endif // AGREEMENTS_HPP

