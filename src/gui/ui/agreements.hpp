#ifndef AGREEMENTS_HPP
#define AGREEMENTS_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <QWidget>
#include <QMenu>
#include <QScopedPointer>
#include <QPointer>
#include <QList>
#include <QSqlRecord>

#include <tuple>
#include <map>

namespace Ui {
class Agreements;
}

class AgreementsProxyModel;
class AgreementReceiptsProxyModel;

class QTableView;

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

    void setAsCurrentAgreement(int nSourceRow, int nFolder); // The smart contract itself, or payment plan, that's selected.
    void setAsCurrentParty(int nSourceRow); // You may have multiple signer Nyms on the same agreement, in the same wallet.
    void setAsCurrentPayment(int nSourceRow, int nFolder); // the recurring paymentReceipts for each smart contract or payment plan.

public:
    explicit Agreements(QWidget *parent = 0);
    ~Agreements();

    void dialog(int nSourceRow=-1, int nFolder=-1);

    void RefreshUserBar();
    QWidget * CreateUserBarWidget();

public slots:
    void onRecordlistPopulated();
    void onBalancesChanged();
    void onNeedToRefreshUserBar();
    void onNeedToRefreshRecords();
    void onClaimsUpdatedForNym(QString nymId);

signals:
    void needToDownloadAccountData();
    void needToPopulateRecordlist();
    void showContact(QString);
    void showContactAndRefreshHome(QString);
    void needToCheckNym(QString, QString, QString);

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

    void on_MarkAsRead_timer();
    void on_MarkAsUnread_timer();

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

    QAction * pActionDelete              = nullptr;
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

    QList<QModelIndex> listRecordsToMarkAsRead_;
    QList<QModelIndex> listRecordsToMarkAsUnread_;

    bool bRefreshingAfterUpdatedClaims_=false;

    int nLastSelectedRecurringIndex_ = -1;
    int nLastSelectedContractIndex_  = -1;
};

#endif // AGREEMENTS_HPP

